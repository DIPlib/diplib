/*
 * DIPlib 3.0
 * This file contains definitions for cross correlation
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
#include "diplib/analysis.h"
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/linear.h"
#include "diplib/geometry.h"

namespace dip {

void CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& in1Representation,
      String const& in2Representation,
      String const& outRepresentation,
      String const& normalize
) {
   DIP_THROW_IF( !in1.IsForged() || !in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in1.IsScalar() || !in2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in1.Sizes() != in2.Sizes(), E::SIZES_DONT_MATCH );
   Image in1FT;
   if( BooleanFromString( in1Representation, "spatial", "frequency" )) {
      DIP_START_STACK_TRACE
         FourierTransform( in1, in1FT );
      DIP_END_STACK_TRACE
   } else {
      in1FT = in1.QuickCopy();
   }
   Image in2FT;
   if( BooleanFromString( in2Representation, "spatial", "frequency" )) {
      DIP_START_STACK_TRACE
         FourierTransform( in2, in2FT );
      DIP_END_STACK_TRACE
   } else {
      in2FT = in2.QuickCopy();
   }
   DataType dt = in1FT.DataType();
   DIP_START_STACK_TRACE
      MultiplyConjugate( in1FT, in2FT, out, dt );
   DIP_END_STACK_TRACE
   if( BooleanFromString( normalize, "normalize", "don't normalize" )) {
      if( in2FT.IsShared() ) {
         in2FT.Strip(); // make sure we don't write in any input data segments. Otherwise, we re-use the data segment.
      }
      SquareModulus( in1FT, in2FT );
      out /= in2FT; // Normalize by the square modulus of in1.
      // TODO: prevent division by 0, if that is even possible...
   }
   if( BooleanFromString( outRepresentation, "spatial", "frequency" )) {
      DIP_START_STACK_TRACE
         StringSet options{ "inverse", "real" };
         FourierTransform( out, out, options );
      DIP_END_STACK_TRACE
   }
}

namespace {

FloatArray FindShift_CPF( Image const& in1, Image const& in2, dfloat maxFrequency ) {
   DIP_THROW_IF( in1.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   if( maxFrequency <= 0.0 ) {
      maxFrequency = 0.2;
   }
   // Do cross-correlation for sub-pixel shift, forcing dcomplex output
   Image cross{ in1.Sizes(), 1, DT_DCOMPLEX };
   cross.Protect();
   CrossCorrelationFT( in1, in2, cross, "spatial", "spatial", "frequency", "normalize" );
   DIP_ASSERT( cross.DataType() == DT_DCOMPLEX );
   DIP_ASSERT( cross.Stride( 0 ) == 1 );
   DIP_ASSERT( cross.Stride( 1 ) == 1 );
   // Do the least squares fit
   dip::sint center_x = static_cast< dip::sint >( cross.Size( 0 ) / 2 );
   dip::sint center_y = static_cast< dip::sint >( cross.Size( 1 ) / 2 );
   dip::uint amount = 0;
   dfloat sumuv = 0;
   dfloat sumuu = 0;
   dfloat sumvv = 0;
   dfloat sumAv = 0;
   dfloat sumAu = 0;
   dfloat radius = maxFrequency * maxFrequency;
   dfloat du = 2.0 * pi / static_cast< dfloat >( cross.Size( 0 ));
   dfloat dv = 2.0 * pi / static_cast< dfloat >( cross.Size( 1 ));
   dfloat uStart = static_cast< dfloat >( 0 - center_x ) * du;
   dfloat v = static_cast< dfloat >( 0 - center_y ) * dv;
   // For the sake of simplicity, we forgo the Framework
   for( dip::sint jj = 0; jj < static_cast< dip::sint >( cross.Size( 1 )); ++jj ) {
      dcomplex* ptr = static_cast< dcomplex* >( cross.Origin() ) + cross.Stride( 1 ) * jj; // just in case stride != width, which should not happen.
      dfloat vv = v * v;
      if( vv < radius ) {
         dfloat u = uStart;
         for( dip::sint ii = 0; ii < static_cast< dip::sint >( cross.Size( 0 )); ii++ ) {
            dfloat uu = u * u;
            if( uu + vv < radius ) {
               dfloat amplitude = std::abs( *ptr );
               if( std::abs( amplitude - 1 ) < 0.1 ) {
                  // Use this point
                  dfloat angle = std::arg( *ptr );
                  sumuv += u * v;
                  sumuu += uu;
                  sumvv += vv;
                  sumAv += angle * v;
                  sumAu += angle * u;
                  ++amount;
               }
            }
            ++ptr;
            u += du;
         }
      }
      v += dv;
   }
   DIP_THROW_IF( amount < 3, "Too few valid data points to do calculation" );
   dfloat value = sumuv * sumuv - sumvv * sumuu;
   DIP_THROW_IF ( value == 0, "Parameter 'value' is zero" );
   return FloatArray{
         ( sumAu * sumvv - sumAv * sumuv ) / value,
         ( sumAv * sumuu - sumAu * sumuv ) / value
   };
   // TODO: Add removal of outliers as an option to this function.
}

FloatArray FindShift_MTS( Image const& in1, Image const& in2, dip::uint iterations, dfloat accuracy, dfloat sigma ) {
   dip::uint nDims = in1.Dimensionality();
   FloatArray shift( nDims, 1e9 ); // Something large...
   FloatArray previousShift = shift;
   FloatArray previousPreviousShift = shift;

   // Solve: sum( gradient * gradient' ) * shift = sum(( in2 - in1 ) * gradient )
   // Solve: M * shift = V
   Image in1g = Gauss( in1, { sigma } );
   Image in2g = Gauss( in2, { sigma } );
   Image gradient = Gradient( in1, { sigma } );
   Image M = Sum( Multiply( gradient, Transpose( gradient )));
   M.Convert( DT_DFLOAT );
   M.ExpandTensor(); // Multiply yields a symmetric tensor, here we force the storage to be normal

   // iterative Taylor with early break if accuracy is achieved
   dip::uint ii;
   for( ii = 0; ii < iterations; ii++ ) {
      Image tmp = in2g.QuickCopy();
      // If ii > 0, we shift in2
      if(( ii == 1 ) || ( ii == 2 )) {
         Shift( in2g, tmp, shift, "3-cubic" );
      } else if( ii > 2 ) {
         // Use non-smoothed image for iterations after the 3rd one.
         Shift( in2, tmp, shift, "3-cubic" );
         in1g = in1.QuickCopy();
      }
      tmp -= in1;
      Image V = Sum( tmp * gradient );
      V.Convert( DT_DFLOAT );
      previousPreviousShift = previousShift;
      previousShift = shift;
      Solve( nDims, nDims, { static_cast< dfloat* >( M.Origin() ), M.TensorStride() },
                           { static_cast< dfloat* >( V.Origin() ), V.TensorStride() }, shift.begin() );
      // break if desired accuracy achieved or good condition for bias correction
      if(( ii >= 2 ) && ( ii < iterations - 1 )) { // if ii == maxIter-1 => loop would end anyway
         bool done = false;
         bool small = true;
         for( dip::uint kk = 0; kk < nDims; ++kk ) {
            if( std::abs( shift[ kk ] ) > std::abs( previousShift[ kk ] )) {
               done = true;
               break;
            }
            if( accuracy > 0.0 ) {
               if( std::abs( shift[ kk ] ) > accuracy ) {
                  small = false;
               }
            } else {
               if( std::abs( previousPreviousShift[ kk ] * shift[ kk ] / previousShift[ kk ] / previousShift[ kk ] ) <= 1.05 ) {
                  small = false;
               }
            }
         }
         if( done || small ) {
            ++ii;
            break;
         }
      }
   }
   if( ii > 2 ) {
      // shortcut bias formula
      for( dip::uint kk = 0; kk < nDims; ++kk ) {
         dfloat bias = previousShift[ kk ] * previousShift[ kk ] /
                       ( previousPreviousShift[ kk ] - previousShift[ kk ] ); // beware /0 instability
         if( std::abs( bias ) < std::abs( previousShift[ kk ] )) { // bias should be smaller than last incremented shift
            shift[ kk ] += bias;
         }
      }
   }
   return shift;
}

FloatArray FindShift_PROJ( Image const& in1, Image const& in2, dip::uint iterations, dfloat accuracy, dfloat sigma ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   dip::uint nDims = in1.Dimensionality();
   FloatArray shift( nDims, 0.0 );
   BooleanArray ps( nDims, true );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_START_STACK_TRACE
         ps[ ii ] = false;
         Image line1 = Sum( in1, {}, ps );
         Image line2 = Sum( in2, {}, ps );
         ps[ ii ] = true;
         line1.Squeeze();
         line2.Squeeze();
         FloatArray res = FindShift_MTS( line1, line2, iterations, accuracy, sigma );
         shift[ ii ] = res[ 0 ];
      DIP_END_STACK_TRACE
   }
}

FloatArray FindShift_CC(
      Image const& in1,
      Image const& in2,
      dip::uint maxShift,
      String const& normalize = "don't normalize",
      bool subpixelPrecision = false
) {
   dip::uint nDims = in1.Dimensionality();
   Image cross;
   DIP_START_STACK_TRACE
      CrossCorrelationFT( in1, in2, cross, "spatial", "spatial", "spatial", normalize );
   DIP_END_STACK_TRACE
   UnsignedArray sizes = cross.Sizes();
   bool crop = false;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( sizes[ ii ] > maxShift ) {
         sizes[ ii ] = maxShift;
         crop = true;
      }
   }
   if( crop ) {
      DIP_START_STACK_TRACE
         cross = cross.Crop( sizes );
      DIP_END_STACK_TRACE
   }
   UnsignedArray maxPixel;
   DIP_START_STACK_TRACE
      maxPixel = MaximumPixel( cross, {} );
   DIP_END_STACK_TRACE
   FloatArray shift;
   if( subpixelPrecision ) {
      SubpixelLocationResult loc = SubpixelLocation( cross, maxPixel );
      shift = loc.coordinates;
   } else {
      shift = FloatArray{ maxPixel };
   }
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dfloat centerpixel = static_cast< dfloat >( sizes[ ii ] / 2 );
      shift[ ii ] = centerpixel - shift[ ii ]; // Reverse sign of shift
   }
   return shift;
}

FloatArray CorrectIntegerShift(
      Image& in1,
      Image& in2,
      dip::uint maxShift
) {
   dip::uint nDims = in1.Dimensionality();
   FloatArray shift;
   DIP_START_STACK_TRACE
      shift = FindShift_CC( in1, in2, maxShift );
   DIP_END_STACK_TRACE
   if( shift.any() ) {
      // Shift is non-zero along at least one dimension
      // Correct for this integer shift by cropping both images
      UnsignedArray sizes = in1.Sizes();
      UnsignedArray origin( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = shift[ ii ] > 0 ? static_cast< dip::uint >( shift[ ii ] ) : 0;
         sizes[ ii ] -= static_cast< dip::uint >( std::abs( shift[ ii ] ));
      }
      in1.dip__SetSizes( sizes );
      in1.dip__SetOrigin( in1.Pointer( origin ));
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = shift[ ii ] < 0 ? static_cast< dip::uint >( -shift[ ii ] ) : 0;
      }
      in2.dip__SetSizes( sizes );
      in2.dip__SetOrigin( in2.Pointer( origin ));
   }
   return shift;
}

}

FloatArray FindShift(
      Image const& c_in1,
      Image const& c_in2,
      String const& method,
      dfloat parameter,
      dip::uint maxShift
) {
   DIP_THROW_IF( !c_in1.IsForged() || !c_in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in1.IsScalar() || !c_in2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in1.DataType().IsReal() || !c_in2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( c_in1.Sizes() != c_in2.Sizes(), E::SIZES_DONT_MATCH );
   dip::uint nDims = c_in1.Dimensionality();
   FloatArray shift( nDims, 0.0 );
   if( method == "integer only" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, "don't normalize", false ));
   } else if( method == "CC" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, "don't normalize", true ));
   } else if( method == "NCC" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, "normalize", true ));
   } else {
      Image in1 = c_in1.QuickCopy();
      Image in2 = c_in2.QuickCopy();
      DIP_STACK_TRACE_THIS( shift = CorrectIntegerShift( in1, in2, maxShift )); // modifies in1 and in2
      if( method == "CPF" ) {
         DIP_STACK_TRACE_THIS( shift += FindShift_CPF( in1, in2, parameter ));
      } else if( method == "MTS" ) {
         if( parameter <= 0.0 ) {
            parameter = 1.0;
         }
         DIP_STACK_TRACE_THIS( shift += FindShift_MTS( in1, in2, 1, 0.0, parameter ));
      } else {
         dip::uint maxIter = 5;  // default number of iteration => accuracy ~ 1e-4
         dfloat accuracy = 0.0;  // signals early break if bias correction is possible
         if( parameter < 0.0 ) {
            maxIter = std::max( dip::uint{ 1 }, static_cast< dip::uint >( std::round( -parameter )));
            accuracy = 1e-10;    // so small that maxIter would play its role
         } else if(( parameter > 0.0 ) && ( parameter <= 0.1 )) {
            maxIter = 20;        // NOTE: more iteration solution may end up very far from truth
            accuracy = parameter;
         }
         if( method == "ITER" ) {
            DIP_STACK_TRACE_THIS( shift += FindShift_MTS( in1, in2, maxIter, accuracy, 1.0 ));
         } else if( method == "PROJ" ) {
            DIP_STACK_TRACE_THIS( shift += FindShift_PROJ( in1, in2, maxIter, accuracy, 1.0 )); // calls FindShift_MTS
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }
   }
   return shift;
}

}
