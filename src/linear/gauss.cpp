/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the FIR and FT Gaussian filter.
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
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/transform.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

inline dip::uint HalfGaussianSize(
      dfloat sigma,
      dip::uint order,
      dfloat truncation
) {
   return clamp_cast< dip::uint >( std::ceil(( truncation + 0.5 * static_cast< dfloat >( order )) * sigma ));
}

// Creates a half Gaussian kernel, with the x=0 at the right end (last element) of the output array.
FloatArray MakeHalfGaussian(
      dfloat sigma,
      dip::uint order,
      dip::uint length // the length of the array
) {
   FloatArray filter( length );
   dip::uint r0 = length - 1;
   switch( order ) {
      case 0: {
         dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
         //dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma ); // There's no point in computing this if we normalize later!
         dfloat normalization = 0;
         filter[ r0 ] = 1.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat g = std::exp( factor * ( rad * rad ));
            filter[ r0 - rr ] = g;
            normalization += g;
         }
         normalization = 1.0 / ( normalization * 2 + 1 );
         for( dip::uint rr = 0; rr < length; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 1: {
         dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
         dfloat moment = 0.0;
         filter[ r0 ] = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat g = rad * std::exp( factor * ( rad * rad ));
            filter[ r0 - rr ] = g;
            moment += rad * g;
         }
         dfloat normalization = 1.0 / ( 2.0 * moment );
         for( dip::uint rr = 0; rr < length - 1; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 2: {
         dfloat sigma2 = sigma * sigma;
         dfloat sigma4 = sigma2 * sigma2;
         dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma );
         dfloat mean = 0.0;
         filter[ r0 ] = ( -1.0 / sigma2 ) * norm;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat rr2 = rad * rad;
            dfloat g = (( -1.0 / sigma2 ) + ( rr2 ) / sigma4 ) * norm * std::exp( -( rr2 ) / ( 2.0 * sigma2 ));
            filter[ r0 - rr ] = g;
            mean += g;
         }
         mean = ( mean * 2.0 + filter[ r0 ] ) / ( static_cast< dfloat >( r0 ) * 2.0 + 1.0 );
         filter[ r0 ] -= mean;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            filter[ r0 - rr ] -= mean;
            moment += rad * rad * filter[ r0 - rr ];
         }
         dfloat normalization = 1.0 / moment;
         for( dip::uint rr = 0; rr < length; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      case 3: {
         dfloat sigma2 = sigma * sigma;
         dfloat sigma4 = sigma2 * sigma2;
         dfloat sigma6 = sigma4 * sigma2;
         dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma );
         filter[ r0 ] = 0.0;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat r2 = rad * rad;
            dfloat g = norm * std::exp( -r2 / ( 2.0 * sigma2 )) * ( rad * ( 3.0 * sigma2 - r2 ) / sigma6 );
            filter[ r0 - rr ] = g;
            moment += g * r2 * rad;
         }
         dfloat normalization = 3.0 / moment;
         for( dip::uint rr = 0; rr < length; rr++ ) {
            filter[ rr ] *= normalization;
         }
         break;
      }
      default:
         DIP_THROW( E::NOT_IMPLEMENTED );
   }
   return filter;
}

// Create 1D full Gaussian
FloatArray MakeGaussian(
   dfloat sigma,
   dip::uint order,
   dfloat truncation,
   bool roundSize
) {
   FloatArray gaussian;
   // Handle sigma == 0.0
   if( sigma == 0.0 ) {
      gaussian.resize( 1, 1.0 );
   } else {
      // Determine filter size
      dip::uint halfFilterSize = 1 + clamp_cast<dip::uint>((truncation + 0.5 * static_cast<dfloat>(order)) * sigma + 0.5); // Differs from DIPlib v2: HalfGaussianSize( sigma, order, truncation );
      // Create actual Gauss
      if( halfFilterSize < 1 ) {
         halfFilterSize = 1;
      }
      if( order > 2 && halfFilterSize < 2 ) {
         halfFilterSize = 2;
      }

      FloatArray halfGaussian = MakeHalfGaussian( sigma, order, halfFilterSize );
      dfloat symmetrySign;
      switch( order ) {
      case 0:
      case 2:
         // Even symmetry
         symmetrySign = 1.0;
         break;
      case 1:
      case 3:
         // Odd symmetry
         symmetrySign = -1.0;
         break;
      default:
         DIP_THROW( "Gaussian FIR filter not implemented for order > 3" );
      }
      // Complete the gaussian
      gaussian = halfGaussian;
      for( dip::uint iHG = halfGaussian.size() - 1; iHG > 0; ) {
         --iHG;
         gaussian.push_back( symmetrySign * halfGaussian[ iHG ] );
      }
   }

   return gaussian;
}

} // namespace

void CreateGauss(
   Image& out,
   FloatArray const& sigmas,
   UnsignedArray orders,
   dfloat truncation,
   UnsignedArray expos
) {
   // Verify dimensionality
   dip::uint nDims = sigmas.size();
   std::vector< FloatArray > gaussians;

   // Adjust truncation to default if needed
   if( truncation <= 0.0 ) {
      truncation = 3.0;
   }

   UnsignedArray outSizes;
   UnsignedArray centers;
   // Create 1D gaussian for each dimension
   for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
      const bool roundSize = true;
      gaussians.emplace_back( MakeGaussian( sigmas[ iDim ], orders[ iDim ], truncation, roundSize ) );
      dip::uint gaussianLength = gaussians.back().size();
      outSizes.push_back( gaussianLength );
      centers.push_back( (gaussianLength - 1) / 2 );
   }

   // Create output image
   out = Image( outSizes, 1, DT_DFLOAT );
   ImageIterator< dfloat > itOut( out );
   do {
      const UnsignedArray& coords = itOut.Coordinates();
      // Multiply gaussians
      *itOut = 1.0;
      for( dip::uint iDim = 0; iDim < nDims; ++iDim ) {
         *itOut *= gaussians[ iDim ][ coords[ iDim ] ];
      }
      // Add moments
      for( dip::uint iM = 0; iM < nDims; ++iM) {
         *itOut *= std::pow( static_cast<dfloat>(coords[ iM ]) - centers[ iM ], expos[ iM ] );
      }
   } while( ++itOut );
}

void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   BooleanArray process( nDims, true );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( sigmas[ ii ] > 0.0 ) && ( in.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
               filter[ ii ] = filter[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            switch( order[ ii ] ) {
               case 0:
               case 2:
                  filter[ ii ].symmetry = S::EVEN;
                  break;
               case 1:
               case 3:
                  filter[ ii ].symmetry = S::ODD;
                  break;
               default:
                  DIP_THROW( "Gaussian FIR filter not implemented for order > 3" );
            }
            filter[ ii ].filter = MakeHalfGaussian(
                  sigmas[ ii ], order[ ii ],
                  HalfGaussianSize( sigmas[ ii ], order[ ii ], truncation ) + 1 );
            // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
         }
      } else {
         process[ ii ] = false;
      }
   }
   SeparableConvolution( in, out, filter, boundaryCondition, process );
}


namespace {

template< typename TPI > // TPI is always complex (either scomplex or dcomplex)
class GaussFTLineFilter : public Framework::ScanLineFilter {
   public:
      using TPIf = FloatType< TPI >;
      GaussFTLineFilter( UnsignedArray const& sizes, FloatArray const& sigmas, UnsignedArray const& order, dfloat truncation ) {
         dip::uint nDims = sizes.size();
         gaussLUTs_.resize( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            bool found = false;
            for( dip::uint jj = 0; jj < ii; ++jj ) {
               if(( sizes[ jj ] == sizes[ ii ] ) && ( sigmas[ jj ] == sigmas[ ii ] ) && ( order[ jj ] == order[ ii ] )) {
                  gaussLUTs_[ ii ] = gaussLUTs_[ jj ];
                  found = true;
                  break;
               }
            }
            if( !found ) {
               gaussLUTs_[ ii ].resize( sizes[ ii ], TPI( 0 ));
               TPI* lut = gaussLUTs_[ ii ].data();
               // (( i*2*pi ) * x / size )^o * exp( -0.5 * (( 2*pi * sigma ) * x / size )^2 ) == a * x^o * exp( b * x^2 )
               dip::sint origin = static_cast< dip::sint >( sizes[ ii ] ) / 2;
               TPIf b = static_cast< TPIf >( 2.0 * pi * sigmas[ ii ] ) / static_cast< TPIf >( sizes[ ii ] );
               b = -TPIf( 0.5 ) * b * b;
               dip::uint N = b == 0 ? sizes[ ii ] : HalfGaussianSize( static_cast< dfloat >( sizes[ ii ] ) / ( 2.0 * pi * sigmas[ ii ] ), order[ ii ], truncation );
               dip::sint begin = std::max( dip::sint( 0 ), origin - static_cast< dip::sint >( N ));
               dip::sint end = std::min( static_cast< dip::sint >( sizes[ ii ] ), origin + static_cast< dip::sint >( N ) + 1 );
               if( order[ ii ] > 0 ) {
                  TPIf o = static_cast< TPIf >( order[ ii ] );
                  TPI a = { 0, static_cast< TPIf >( 2.0 * pi ) / static_cast< TPIf >( sizes[ ii ] ) };
                  a = std::pow( a, o );
                  if( b != 0 ) {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = a * std::pow( x, o ) * std::exp( b * x * x );
                        ++lut;
                     }
                  } else {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = a * std::pow( x, o );
                        ++lut;
                     }
                  }
               } else {
                  if( b != 0 ) {
                     lut += begin;
                     for( dip::sint jj = begin; jj < end; ++jj ) {
                        TPIf x = static_cast< TPIf >( jj - origin ); // narrowing conversion!
                        *lut = std::exp( b * x * x );
                        ++lut;
                     }
                  } else {
                     std::fill( lut, lut + sizes[ ii ], TPI( 1 ));
                  }
               }
            }
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 3; } // not counting initialization
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         auto bufferLength = params.bufferLength;
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         auto outStride = params.outBuffer[ 0 ].stride;
         TPI weight = 1;
         dip::uint procDim = params.dimension;
         for( dip::uint ii = 0; ii < gaussLUTs_.size(); ++ii ) {
            if( ii != procDim ) {
               weight *= gaussLUTs_[ ii ][ params.position[ ii ] ];
            }
         }
         TPI const* lut = gaussLUTs_[ procDim ].data();
         lut += params.position[ procDim ];
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            *out = *in * weight * *lut;
            in += inStride;
            out += outStride;
            ++lut;
         }
      }
   private:
      std::vector< std::vector< TPI >> gaussLUTs_;
};

} // namespace

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
   DIP_END_STACK_TRACE
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( in.Size( ii ) == 1 ) {
         sigmas[ ii ] = 0;
         order[ ii ] = 0;
      } else {
         if( sigmas[ ii ] < 0 ) {
            sigmas[ ii ] = 0; // no smoothing
         }
      }
   }
   if( sigmas.any() || order.any() ) {
      bool isreal = !in.DataType().IsComplex();
      Image ft = FourierTransform( in );
      DataType dtype = DataType::SuggestComplex( ft.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_COMPLEX( scanLineFilter, GaussFTLineFilter, ( in.Sizes(), sigmas, order, truncation ), dtype );
      Framework::ScanMonadic(
            ft, ft, dtype, dtype, 1, *scanLineFilter,
            Framework::ScanOption::TensorAsSpatialDim + Framework::ScanOption::NeedCoordinates );
      StringSet opts = { S::INVERSE };
      if( isreal ) {
         opts.emplace( S::REAL );
      }
      FourierTransform( ft, out, opts );
   } else {
      out = in;
   }
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/iterators.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing the Gaussian filters") {

   // Test smoothing for the 3 filters
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::dfloat sigma = 5.0;
   dip::dfloat amplitude = 1.0 / ( std::sqrt( 2.0 * dip::pi ) * sigma );
   dip::Image ft = dip::GaussFT( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - amplitude ) < 0.00015 );
   DOCTEST_CHECK( dip::Sum( ft ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   dip::Image fir = dip::GaussFIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( dip::testing::CompareImages( fir, ft, 0.0006 ));
   dip::Image iir = dip::GaussIIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( dip::testing::CompareImages( iir, ft, 0.0015 ));

   // Test first derivative for the 3 filters
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   ft = dip::GaussFT( img, { sigma }, { 1 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - 1.0 ) < 0.0015 ); // Affected by edge effects?
   fir = dip::GaussFIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( fir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( iir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 1.0 ));

   // Test second derivative for the 3 filters
   img = img * img;
   ft = dip::GaussFT( img, { sigma }, { 2 } );
   DOCTEST_CHECK( std::abs( ft.At( 128 ).As< dip::dfloat >() - 2.0 ) < 0.0005 );
   fir = dip::GaussFIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( fir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( iir.At( 128 ).As< dip::dfloat >() == doctest::Approx( 2.0 ));
}

#endif // DIP__ENABLE_DOCTEST
