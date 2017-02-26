/*
 * DIPlib 3.0
 * This file contains the definition of the ImageDisplay function
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
#include "diplib/display.h"
#include "diplib/math.h"
#include "diplib/overload.h"

namespace dip {

namespace {

enum class ComplexToReal {
      Magnitude,
      Phase
};

enum class Mapping {
      Linear,
      Logarithmic
};

template< typename T >
dfloat convert( T v, ComplexToReal /*method*/ ) {
   return v;
}
template<>
dfloat convert( dcomplex v, ComplexToReal method ) {
   if( method == ComplexToReal::Magnitude ) {
      return std::abs( v );
   } else {
      return std::arg( v );
   }
}
template<>
dfloat convert( scomplex v, ComplexToReal method ) {
   if( method == ComplexToReal::Magnitude ) {
      return std::abs( v );
   } else {
      return std::arg( v );
   }
}

template< typename TPI >
void dip__ImageDisplay(
      Image const& slice,
      Image& out,
      ComplexToReal complexToReal,
      Mapping mapping,
      dip::dfloat offset,
      dip::dfloat scale
) {
   dip::uint width = slice.Size( 0 );
   dip::uint height = slice.Size( 1 );
   dip::sint sliceStride0 = slice.Stride( 0 );
   dip::sint sliceStride1 = slice.Stride( 1 );
   dip::sint outStride0 = out.Stride( 0 );
   dip::sint outStride1 = out.Stride( 1 );
   dip::uint telems = slice.TensorElements();
   dip::sint sliceStrideT = slice.TensorStride();
   dip::sint outStrideT = out.TensorStride();
   for( dip::uint kk = 0; kk < telems; ++kk ) {
      TPI* slicePtr = static_cast< TPI* >( slice.Pointer( sliceStrideT * kk ) );
      uint8* outPtr = static_cast< uint8* >( out.Pointer( outStrideT * kk ) );
      for( dip::uint jj = 0; jj < height; ++jj ) {
         TPI* iPtr = slicePtr;
         uint8* oPtr = outPtr;
         if( mapping == Mapping::Linear ) {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = clamp_cast< uint8 >( ( convert( *iPtr, complexToReal ) - offset ) * scale );
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         } else {
            for( dip::uint ii = 0; ii < width; ++ii ) {
               *oPtr = clamp_cast< uint8 >( std::log( convert( *iPtr, complexToReal ) - offset ) * scale );
               iPtr += sliceStride0;
               oPtr += outStride0;
            }
         }
         slicePtr += sliceStride1;
         outPtr += outStride1;
      }
   }
}

} // namespace

void ImageDisplay(
      Image const& in,
      Image& out,
      UnsignedArray const& coordinates,
      dip::uint dim1,
      dip::uint dim2,
      ImageDisplayParams const& params
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.IsScalar() && !in.IsColor(), E::NOT_SCALAR );

   // Compute projection
   Image slice = in.QuickCopy();
   if( nDims > 2 ) {
      DIP_THROW_IF( !coordinates.empty() && coordinates.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
      DIP_THROW_IF(( dim1 >= nDims ) || ( dim2 >= nDims ), E::PARAMETER_OUT_OF_RANGE );
      DIP_THROW_IF( dim1 == dim2, E::INVALID_PARAMETER );
      if( params.projection == "slice" ) {
         RangeArray rangeArray( nDims ); // By default, covers all image pixels
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if(( ii != dim1 ) && ( ii != dim2 )) {
               rangeArray[ ii ] = Range( coordinates[ ii ] );
            }
         }
         slice = slice.At( rangeArray );
      } else if( params.projection == "max" ) {
         BooleanArray process( nDims, true );
         process[ dim1 ] = false;
         process[ dim2 ] = false;
         Maximum( slice, {}, slice, process );
      } else if( params.projection == "mean" ) {
         BooleanArray process( nDims, true );
         process[ dim1 ] = false;
         process[ dim2 ] = false;
         Mean( slice, {}, slice, "", process );
      } else {
         DIP_THROW( E::INVALID_FLAG );
      }
      slice.PermuteDimensions( { dim1, dim2 } );
   }

   // Convert color image to RGB
   if( in.IsColor() ) {
      // TODO
   }

   // How do we convert from complex to real?
   ComplexToReal complexToReal = ComplexToReal::Magnitude;
   if( slice.DataType().IsComplex() ) {
      if( params.complex == "mag" || params.complex == "abs" ) {
         // nothing to do
      } else if( params.complex == "phase" ) {
         complexToReal = ComplexToReal::Phase;
      } else if( params.complex == "real" ) {
         slice = slice.Real();
      } else if( params.complex == "imag" ) {
         slice = slice.Imaginary();
      } else {
         DIP_THROW( E::INVALID_FLAG );
      }
   }

   // How do we stretch the values to the uint8 range?
   DIP_ASSERT( params.upperBound > params.lowerBound );
   Mapping mapping = Mapping::Linear;
   dfloat offset;
   dfloat scale;
   if( params.mode == "lin" ) {
      offset = params.lowerBound;
      scale = 255.0 / ( params.upperBound - params.lowerBound );
   } else if( params.mode == "based" ) {
      dfloat bound = std::max( std::abs( params.lowerBound ), std::abs( params.upperBound ) );
      scale = 255.0 / bound / 2.0;
      offset = -bound;
   } else if( params.mode == "log" ) {
      mapping = Mapping::Logarithmic;
      offset = params.lowerBound + 1.0;
      scale = 255.0 / std::log( params.upperBound - offset );
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }

   // Create ouput
   DIP_ASSERT( slice.Dimensionality() == 2 );
   out.ReForge( slice.Sizes(), slice.TensorElements(), DT_UINT8 );

   // Stretch and convert the data
   DIP_OVL_CALL_ALL( dip__ImageDisplay, ( slice, out, complexToReal, mapping, offset, scale ), slice.DataType() );
}


} // namespace dip
