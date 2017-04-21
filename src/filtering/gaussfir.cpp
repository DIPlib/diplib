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
#include "gauss.h"

namespace dip {

namespace {

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
         //dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma ); // There's no point in computing this if we normalize later!
         dfloat normalization = 0;
         filter[ r0 ] = 1.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat g = exp( factor * ( rad * rad ) );
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
            dfloat g = rad * exp( factor * ( rad * rad ) );
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
         dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma );
         dfloat mean = 0.0;
         filter[ r0 ] = ( -1.0 / sigma2 ) * norm;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat rr2 = rad * rad;
            dfloat g = ( ( -1.0 / sigma2 ) + ( rr2 ) / sigma4 ) * norm * exp( -( rr2 ) / ( 2.0 * sigma2 ) );
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
         dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma );
         filter[ r0 ] = 0.0;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat rad = static_cast< dfloat >( rr );
            dfloat r2 = rad * rad;
            dfloat g = norm * exp( -r2 / ( 2.0 * sigma2 ) ) * ( rad * ( 3.0 * sigma2 - r2 ) / sigma6 );
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

} // namespace

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
                  filter[ ii ].symmetry = "even";
                  break;
               case 1:
               case 3:
                  filter[ ii ].symmetry = "odd";
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

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the Gaussian filters") {

   // Test smoothing for the 3 filters
   dip::Image img{ dip::UnsignedArray{ 256 }, 1, dip::DT_DFLOAT };
   img.Fill( 0.0 );
   img.At( 128 ) = 1.0;
   dip::dfloat sigma = 5.0;
   dip::dfloat amplitude = 1.0 / ( std::sqrt( 2.0 * dip::pi ) * sigma );
   dip::Image ft = dip::GaussFT( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( ft.SampleAt< dip::dfloat >( 128 ) - amplitude ) < 0.00015 );
   DOCTEST_CHECK( static_cast< dip::dfloat >( dip::Sum( ft )) == doctest::Approx( 1.0 ));
   dip::Image fir = dip::GaussFIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( dip::Maximum( fir - ft ))) < 0.0003 );
   dip::Image iir = dip::GaussIIR( img, { sigma }, { 0 } );
   DOCTEST_CHECK( std::abs( static_cast< dip::dfloat >( dip::Maximum( iir - ft ))) < 0.0015 );

   // Test first derivative for the 3 filters
   dip::ImageIterator< dip::dfloat > it( img );
   for( dip::dfloat x = -128; it; ++it, ++x ) {
      *it = x;
   }
   ft = dip::GaussFT( img, { sigma }, { 1 } );
   DOCTEST_CHECK( std::abs( ft.SampleAt< dip::dfloat >( 128 ) - 1.0 ) < 0.0015 ); // Affected by edge effects?
   fir = dip::GaussFIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( fir.SampleAt< dip::dfloat >( 128 ) == doctest::Approx( 1.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 1 } );
   DOCTEST_CHECK( iir.SampleAt< dip::dfloat >( 128 ) == doctest::Approx( 1.0 ));

   // Test second derivative for the 3 filters
   img = img * img;
   ft = dip::GaussFT( img, { sigma }, { 2 } );
   DOCTEST_CHECK( std::abs( ft.SampleAt< dip::dfloat >( 128 ) - 2.0 ) < 0.0005 );
   fir = dip::GaussFIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( fir.SampleAt< dip::dfloat >( 128 ) == doctest::Approx( 2.0 ));
   iir = dip::GaussIIR( img, { sigma }, { 2 } );
   DOCTEST_CHECK( iir.SampleAt< dip::dfloat >( 128 ) == doctest::Approx( 2.0 ));
}

#endif // DIP__ENABLE_DOCTEST
