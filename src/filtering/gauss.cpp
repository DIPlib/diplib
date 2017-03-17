/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the Gaussian filter.
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

namespace dip {

namespace {

inline dip::uint HalfGaussianSize(
      dfloat sigma,
      dip::uint order,
      dfloat truncation
) {
   return static_cast< dip::uint >( std::ceil( ( truncation + 0.5 * order ) * sigma ) + 1 );
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
         //dfloat norm = 1.0 / ( sqrt( 2.0 * pi ) * sigma ); // There's no point in computing this if we normalize later!
         dfloat normalization = 0;
         filter[ r0 ] = 1.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            dfloat g = exp( factor * ( rr * rr ) );
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
            dfloat g = rr * exp( factor * ( rr * rr ) );
            filter[ r0 - rr ] = g;
            moment += rr * g;
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
            dfloat rr2 = rr * rr;
            dfloat g = ( ( -1.0 / sigma2 ) + ( rr2 ) / sigma4 ) * norm * exp( -( rr2 ) / ( 2.0 * sigma2 ) );
            filter[ r0 - rr ] = g;
            mean += g;
         }
         mean = ( mean * 2.0 + filter[ r0 ] ) / ( r0 * 2.0 + 1.0 );
         filter[ r0 ] -= mean;
         dfloat moment = 0.0;
         for( dip::uint rr = 1; rr < length; rr++ ) {
            filter[ r0 - rr ] -= mean;
            moment += rr * rr * filter[ r0 - rr ];
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
            dfloat r2 = rr * rr;
            dfloat g = norm * exp( -r2 / ( 2.0 * sigma2 ) ) * ( rr * ( 3.0 * sigma2 - r2 ) / sigma6 );
            filter[ r0 - rr ] = g;
            moment += g * r2 * rr;
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


void Gauss(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& order,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      String const& method // "FIR","IIR","FT","optimal"
) {

}

void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      BooleanArray process,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( order, nDims, dip::uint( 0 ));
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
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
         filter[ ii ].filter = MakeHalfGaussian( sigmas[ ii ], order[ ii ],
                                                 HalfGaussianSize( sigmas[ ii ], order[ ii ], truncation ));
         // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
      }
   }
   SeparableConvolution( in, out, filter, boundaryCondition, process );
}

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      dfloat truncation
) {

}

void GaussIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      UnsignedArray order,
      StringArray const& boundaryCondition,
      BooleanArray process,
      IntegerArray filterOrder,
      dip::uint designMethod, // should be a string
      dfloat truncation
) {

}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the Gaussian filter") {
   dip::Image img{ dip::UnsignedArray{ 256, 256 }, 1, dip::DT_UINT8 };
   img.SwapDimensions( 0, 1 );
   {
      DIP_THROW_IF( img.DataType() != dip::DT_UINT8, "Expecting 32-bit signed integer image" );
      std::random_device rd;
      std::mt19937 gen( rd() );
      std::normal_distribution< dip::dfloat > normDist( 54.64, 6535.41 );
      dip::ImageIterator< dip::uint8 > it( img );
      do {
         *it = dip::clamp_cast< dip::uint8 >( normDist( gen ) );
      } while( ++it );
   }
   dip::Image out;
   dip::GaussFIR( img, out, { 1 }, { 0 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( dip::Sum( out )) == doctest::Approx( static_cast< dip::dfloat >( dip::Sum( img ))));
   dip::GaussFIR( img, out, { 5, 1 }, { 0 } );
   DOCTEST_CHECK( static_cast< dip::dfloat >( dip::Sum( out )) == doctest::Approx( static_cast< dip::dfloat >( dip::Sum( img ))));
}

#endif // DIP__ENABLE_DOCTEST
