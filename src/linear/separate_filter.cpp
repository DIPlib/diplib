/*
 * DIPlib 3.0
 * This file contains the definition of SeparateFilter().
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib.h"
#include "diplib/linear.h"

#if defined(__GNUG__) || defined(__clang__)
// For this file, turn off -Wsign-conversion, Eigen is really bad at this!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

namespace dip {

OneDimensionalFilterArray SeparateFilter( Image const& c_in ) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint ndims = c_in.Dimensionality();
   DIP_THROW_IF( ndims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   OneDimensionalFilterArray out( ndims );
   Image filter = Convert( c_in, DT_DFLOAT ); // Filter is DFLOAT and has normal strides
   DIP_ASSERT( filter.HasNormalStrides() );
   dfloat* data = static_cast< dfloat* >( filter.Origin() );
   UnsignedArray sizes = filter.Sizes();
   dip::uint nPixels = sizes.product();
   // Shave dimensions off the filter from the end
   while( ndims > 1 ) {
      dip::uint dim = ndims - 1; // Current dimension
      dip::uint length = sizes[ dim ]; // Number of pixels in 1D filter for this dimension
      nPixels /= length; // Number of pixels in remainder
      if( length > 1 ) {
         // Make a matrix out of `filter` that has `nPixel` rows and `length` columns
         Eigen::Map< Eigen::MatrixXd > matrix( data, static_cast< Eigen::Index >( nPixels ),
                                                     static_cast< Eigen::Index >( length ));
         // Compute SVD
         Eigen::JacobiSVD< Eigen::MatrixXd > svd( matrix, Eigen::ComputeThinU | Eigen::ComputeThinV );
         // Expect all but first singular value to be close to 0. If not, it's not separable, and we return {}.
         auto S = svd.singularValues();
         dfloat s1 = S( 0 );
         dfloat s2 = S( 1 );
         dfloat tolerance = 1e-7 * static_cast< dfloat >( std::max( nPixels, length )) * std::abs( s1 );
         //std::cout << "s1 = " << s1 << ", s2 = " << s2 << ", tol = " << tolerance << std::endl;
         if( s2 > tolerance ) {
            // Not separable!
            return {};
         }
         // 1D filter is first column of V -- write V into 1D filter buffer
         out[ dim ].filter.resize( length );
         Eigen::Map< Eigen::VectorXd > oneDFilter( out[ dim ].filter.data(), static_cast< Eigen::Index >( length ));
         oneDFilter = svd.matrixV().col( 0 );
         // The ndims-1--dimensional remainder is the first column of U * singular value --
         // write U * s1 back into input image, we'll use fewer pixels than before
         Eigen::Map< Eigen::VectorXd > remainder( data, static_cast< Eigen::Index >( nPixels ));
         remainder = svd.matrixU().col( 0 ) * s1;
      }
      // else: the output filter will have size 0, and we continue as usual.
      --ndims;
   }
   out[ 0 ].filter.resize( nPixels );
   std::copy( data, data + nPixels, out[ 0 ].filter.data() );
   return out;
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"

DOCTEST_TEST_CASE("[DIPlib] testing the filter separation") {
   dip::Image delta3D( { 30, 30, 30 }, 1, dip::DT_SFLOAT );
   delta3D.Fill( 0 );
   delta3D.At( 15, 15, 15 ) = 1;

   auto rectPt = dip::PixelTable( "rectangular", { 10, 11, 5 } );
   dip::Image rect = dip::Convert( rectPt.AsImage(), dip::DT_UINT8 );
   //auto clock = std::chrono::steady_clock::now();
   dip::OneDimensionalFilterArray rect1 = dip::SeparateFilter( rect );
   //std::cout << "Separating filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   DOCTEST_REQUIRE( rect1.size() == 3 );
   DOCTEST_CHECK( rect1[ 0 ].filter.size() == 10 );
   DOCTEST_CHECK( rect1[ 1 ].filter.size() == 11 );
   DOCTEST_CHECK( rect1[ 2 ].filter.size() == 5 );
   //clock = std::chrono::steady_clock::now();
   //dip::Image tmp1 = dip::SeparableConvolution( delta3D, rect1 );
   //std::cout << "Apply separated filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   //clock = std::chrono::steady_clock::now();
   //dip::Image tmp2 = dip::GeneralConvolution( delta3D, rect );
   //std::cout << "Apply full filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   //DOCTEST_CHECK( dip::All( tmp1 == tmp2 ).As< bool >() );
   bool correct = true;
   for( dip::uint ii = 0; ii < 3; ++ii ) {
      for( auto v : rect1[ ii ].filter ) {
         if( doctest::Approx( v ) != rect1[ ii ].filter[ 0 ] ) {
            correct = false;
         }
      }
   }
   DOCTEST_CHECK( correct );
   dip::dfloat product = rect1[ 0 ].filter[ 0 ] * rect1[ 1 ].filter[ 0 ] * rect1[ 2 ].filter[ 0 ];
   DOCTEST_CHECK( product == doctest::Approx( 1.0 ));

   auto circPt = dip::PixelTable( "elliptic", { 10, 11, 5 } );
   dip::Image circ = dip::Convert( circPt.AsImage(), dip::DT_UINT8 );
   //clock = std::chrono::steady_clock::now();
   dip::OneDimensionalFilterArray circ1 = dip::SeparateFilter( circ );
   DOCTEST_CHECK( circ1.size() == 0 );

   //clock = std::chrono::steady_clock::now();
   dip::Image gaussRes = dip::GaussFIR( delta3D, { 3, 2, 3 }, { 1, 0, 2 } );
   //std::cout << "Compute Gaussian: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   dip::Image gauss = gaussRes.Crop( { 3*6+1, 2*6+1, 3*6+1 } );
   //clock = std::chrono::steady_clock::now();
   dip::OneDimensionalFilterArray gauss1 = dip::SeparateFilter( gauss );
   //std::cout << "Separating filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   DOCTEST_REQUIRE( gauss1.size() == 3 );
   //clock = std::chrono::steady_clock::now();
   dip::Image tmp3 = dip::SeparableConvolution( delta3D, gauss1 );
   //std::cout << "Apply separated filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   //clock = std::chrono::steady_clock::now();
   //tmp2 = dip::GeneralConvolution( delta3D, gauss );
   //std::cout << "Apply full filter: " << std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - clock ).count() << std::endl;
   tmp3 -= gaussRes;
   auto m = dip::GetMaximumAndMinimum( tmp3 );
   DOCTEST_CHECK( m.Minimum() > -1e-5 );
   DOCTEST_CHECK( m.Maximum() < 1e-5 );
}

#endif // DIP__ENABLE_DOCTEST
