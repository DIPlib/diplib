/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016-2019, Cris Luengo.
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

#include <limits>

#include "diplib.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/boundary.h"
#include "diplib/saturated_arithmetic.h"

namespace dip {
namespace detail {


//
// CopyBuffer()
//

namespace {

template< class inT, class outT >
static inline void cast_copy( ConstSampleIterator< inT > in, ConstSampleIterator< inT > end, SampleIterator< outT > out ) {
   for( ; in != end; ++in, ++out ) {
      *out = clamp_cast< outT >( *in );
   }
}

template< typename inT, typename outT >
static inline void CopyBufferFromTo(
      inT const* inBuffer,
      dip::sint inStride,
      dip::sint inTensorStride,
      outT* outBuffer,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      std::vector< dip::sint > const& lookUpTable // it this is empty, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
) {
   //std::cout << "CopyBufferFromTo<" << typeid(inT).name() << "," << typeid(outT).name() << ">\n";
   if( tensorElements == 1 ) {
      if( inStride == 0 ) {
         //std::cout << "CopyBufferFromTo<inT,outT>, mode 1\n";
         FillBufferFromTo( outBuffer, outStride, 1, pixels, 1, clamp_cast< outT >( *inBuffer ) );
      } else {
         //std::cout << "CopyBufferFromTo<inT,outT>, mode 2\n";
         auto inIt = ConstSampleIterator< inT >( inBuffer, inStride );
         auto outIt = SampleIterator< outT >( outBuffer, outStride );
         cast_copy( inIt, inIt + pixels, outIt );
      }
   } else if( lookUpTable.empty() ) {
      // TODO: tensorElements * {in|out}TensorStride == {in|out}Stride -> do a single loop. Is that worth it?
      if( inStride == 0 ) {
         if( inTensorStride == 0 ) {
            //std::cout << "CopyBufferFromTo<inT,outT>, mode 3\n";
            FillBufferFromTo( outBuffer, outStride, outTensorStride, pixels, tensorElements, clamp_cast< outT >( *inBuffer ) );
         } else {
            //std::cout << "CopyBufferFromTo<inT,outT>, mode 4\n";
            for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
               FillBufferFromTo( outBuffer, outStride, 1, pixels, 1, clamp_cast< outT >( *inBuffer ) );
               inBuffer += inTensorStride;
               outBuffer += outTensorStride;
            }
         }
      } else {
         if( inTensorStride == 0 ) {
            //std::cout << "CopyBufferFromTo<inT,outT>, mode 5\n";
            for( dip::uint pp = 0; pp < pixels; ++pp ) {
               FillBufferFromTo( outBuffer, 1, outTensorStride, 1, tensorElements, clamp_cast< outT >( *inBuffer ) );
               inBuffer += inStride;
               outBuffer += outStride;
            }
         } else {
            //std::cout << "CopyBufferFromTo<inT,outT>, mode 6\n";
            for( dip::uint pp = 0; pp < pixels; ++pp ) {
               auto inIt = ConstSampleIterator< inT >( inBuffer, inTensorStride );
               auto outIt = SampleIterator< outT >( outBuffer, outTensorStride );
               cast_copy( inIt, inIt + tensorElements, outIt );
               inBuffer += inStride;
               outBuffer += outStride;
            }
         }
      }
   } else {
      //std::cout << ( inStride == 0 ? "CopyBufferFromTo<inT,outT>, mode 7\n" : "CopyBufferFromTo<inT,outT>, mode 8\n" );
      for( dip::uint tt = 0; tt < lookUpTable.size(); ++tt ) {
         dip::sint index = lookUpTable[ tt ];
         auto outIt = SampleIterator< outT >( outBuffer, outStride );
         if( index < 0 ) {
            std::fill( outIt, outIt + pixels, outT( 0 ) );
         } else {
            if( inStride == 0 ) {
               // mode 7
               std::fill( outIt, outIt + pixels, clamp_cast< outT >( *( inBuffer + index * inTensorStride ) ) );
            } else {
               // mode 8
               auto inIt = ConstSampleIterator< inT >( inBuffer + index * inTensorStride, inStride );
               cast_copy( inIt, inIt + pixels, outIt );
            }
         }
         outBuffer += outTensorStride;
      }
   }
   //std::cout << "/CopyBufferFromTo\n";
}

template< typename T >
static inline void CopyBufferFromTo(
      T const* inBuffer,
      dip::sint inStride,
      dip::sint inTensorStride,
      T* outBuffer,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      std::vector< dip::sint > const& lookUpTable // it this is empty, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
) {
   if( tensorElements == 1 ) {
      if( inStride == 0 ) {
         //std::cout << "CopyBufferFromTo<T>, mode 1\n";
         FillBufferFromTo( outBuffer, outStride, 1, pixels, 1, *inBuffer );
      } else if(( inStride == 1 ) && ( outStride == 1 )) {
         //std::cout << "CopyBufferFromTo<T>, mode 2\n";
         std::copy( inBuffer, inBuffer + pixels, outBuffer );
      } else {
         //std::cout << "CopyBufferFromTo<T>, mode 3\n";
         auto inIt = ConstSampleIterator< T >( inBuffer, inStride );
         auto outIt = SampleIterator< T >( outBuffer, outStride );
         std::copy( inIt, inIt + pixels, outIt );
      }
   } else if( lookUpTable.empty() ) {
      if( inStride == 0 ) {
         if( inTensorStride == 0 ) {
            //std::cout << "CopyBufferFromTo<T>, mode 4\n";
            FillBufferFromTo( outBuffer, outStride, outTensorStride, pixels, tensorElements, *inBuffer );
         } else {
            //std::cout << "CopyBufferFromTo<T>, mode 5\n";
            for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
               FillBufferFromTo( outBuffer, outStride, 1, pixels, 1, *inBuffer );
               inBuffer += inTensorStride;
               outBuffer += outTensorStride;
            }
         }
      } else if(( inTensorStride == 1 ) && ( outTensorStride == 1 )) {
         if(( inStride == static_cast< dip::sint >( tensorElements )) && ( outStride == static_cast< dip::sint >( tensorElements ))) {
            //std::cout << "CopyBufferFromTo<T>, mode 6\n";
            std::copy( inBuffer, inBuffer + pixels * tensorElements, outBuffer );
         } else {
            //std::cout << "CopyBufferFromTo<T>, mode 7\n";
            for( dip::uint pp = 0; pp < pixels; ++pp ) {
               std::copy( inBuffer, inBuffer + tensorElements, outBuffer );
               inBuffer += inStride;
               outBuffer += outStride;
            }
         }
      } else if(( inStride == 1 ) && ( outStride == 1 )) {
         if(( inTensorStride == static_cast< dip::sint >( pixels )) && ( outTensorStride == static_cast< dip::sint >( pixels ))) {
            //std::cout << "CopyBufferFromTo<T>, mode 8\n";
            std::copy( inBuffer, inBuffer + pixels * tensorElements, outBuffer );
         } else {
            //std::cout << "CopyBufferFromTo<T>, mode 9\n";
            for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
               std::copy( inBuffer, inBuffer + pixels, outBuffer );
               inBuffer += inTensorStride;
               outBuffer += outTensorStride;
            }
         }
      } else {
         if( inTensorStride == 0 ) {
            //std::cout << "CopyBufferFromTo<T>, mode 10\n";
            for( dip::uint pp = 0; pp < pixels; ++pp ) {
               FillBufferFromTo( outBuffer, 1, outTensorStride, 1, tensorElements, *inBuffer );
               inBuffer += inStride;
               outBuffer += outStride;
            }
         } else {
            //std::cout << "CopyBufferFromTo<T>, mode 11\n";
            for( dip::uint pp = 0; pp < pixels; ++pp ) {
               auto inIt = ConstSampleIterator< T >( inBuffer, inTensorStride );
               auto outIt = SampleIterator< T >( outBuffer, outTensorStride );
               std::copy( inIt, inIt + tensorElements, outIt );
               inBuffer += inStride;
               outBuffer += outStride;
            }
         }
      }
   } else {
      //std::cout << ( inStride == 0 ? "CopyBufferFromTo<T>, mode 12\n" : "CopyBufferFromTo<T>, mode 13\n" );
      for( dip::uint tt = 0; tt < lookUpTable.size(); ++tt ) {
         dip::sint index = lookUpTable[ tt ];
         auto outIt = SampleIterator< T >( outBuffer, outStride );
         if( index < 0 ) {
            std::fill( outIt, outIt + pixels, T( 0 ) );
         } else {
            if( inStride == 0 ) {
               std::fill( outIt, outIt + pixels, *( inBuffer + index * inTensorStride ) );
            } else {
               auto inIt = ConstSampleIterator< T >( inBuffer + index * inTensorStride, inStride );
               std::copy( inIt, inIt + pixels, outIt );
            }
         }
         outBuffer += outTensorStride;
      }
   }
}

template< typename inT >
static inline void CopyBufferFrom(
      inT const* inBuffer,
      dip::sint inStride,
      dip::sint inTensorStride,
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      std::vector< dip::sint > const& lookUpTable
) {
   switch( outType ) {
      case dip::DT_BIN:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< bin* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT8:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< uint8* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT16:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< uint16* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT32:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< uint32* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT64:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< uint64* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT8:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint8* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT16:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint16* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT32:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint32* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT64:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint64* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SFLOAT:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sfloat* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DFLOAT:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< dfloat* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SCOMPLEX:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< scomplex* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DCOMPLEX:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< dcomplex* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}

} // namespace

void CopyBuffer(
      void const* inBuffer,
      DataType inType,
      dip::sint inStride,
      dip::sint inTensorStride,
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      std::vector< dip::sint > const& lookUpTable
) {
   // If output strides are zero, we only have one sample to fill. As is now, we fill it with the first input sample.
   // TODO: should we throw instead?
   if( outStride == 0 ) {
      pixels = 1;
      outStride = 1;
   }
   if( outTensorStride == 0 ) {
      tensorElements = 1;
      outTensorStride = 1;
   }
   switch( inType ) {
      case dip::DT_BIN:
         CopyBufferFrom( static_cast< bin const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT8:
         CopyBufferFrom( static_cast< uint8 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT16:
         CopyBufferFrom( static_cast< uint16 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT32:
         CopyBufferFrom( static_cast< uint32 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT64:
         CopyBufferFrom( static_cast< uint64 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT8:
         CopyBufferFrom( static_cast< sint8 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT16:
         CopyBufferFrom( static_cast< sint16 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT32:
         CopyBufferFrom( static_cast< sint32 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT64:
         CopyBufferFrom( static_cast< sint64 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SFLOAT:
         CopyBufferFrom( static_cast< sfloat const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DFLOAT:
         CopyBufferFrom( static_cast< dfloat const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SCOMPLEX:
         CopyBufferFrom( static_cast< scomplex const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DCOMPLEX:
         CopyBufferFrom( static_cast< dcomplex const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}


//
// ExpandBuffer()
//

namespace {

template< typename DataType >
static inline void ExpandBufferConstant(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels,
      dip::uint left,
      dip::uint right,
      DataType leftValue,
      DataType rightValue
) {
   DataType* out = buffer - stride;
   for( dip::sint ii = 0; ii < dip::sint( left ); ++ii ) {
      *out = leftValue;
      out -= stride;
   }
   out = buffer + static_cast< dip::sint >( pixels ) * stride;
   for( dip::sint ii = 0; ii < dip::sint( right ); ++ii ) {
      *out = rightValue;
      out += stride;
   }
}

// First order extrapolation: instead of constructing a 1st order polynomial based on the 2 samples at the edge of
// the image, which is not very useful, we construct a 1st order polynomial that connects to the image edge
// (i.e. function value matches the sample at the edge of the image line), and reaches 0 at the end of the expanded
// boundary. This imposes a sort of windowing around the image.
template< typename DataType >
static inline void ExpandBufferFirstOrder(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 1
      dip::uint left,
      dip::uint right
) {
   if( left > 0 ) {
      // Left side
      DataType* in = buffer;
      DataType* out = buffer - stride;
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat d1 = d0 / static_cast< dfloat >( left + 1 );
      for( dip::uint ii = 0; ii < left; ii++ ) {
         d0 -= d1;
         *out = clamp_cast< DataType >( d0 );
         out -= stride;
      }
   }
   if( right > 0 ) {
      // Right side
      DataType* in = buffer + ( static_cast< dip::sint >( pixels ) - 1 ) * stride;
      DataType* out = buffer + static_cast< dip::sint >( pixels ) * stride;
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat d1 = d0 / static_cast< dfloat >( right + 1 );
      for( dip::uint ii = 0; ii < right; ii++ ) {
         d0 -= d1;
         *out = clamp_cast< DataType >( d0 );
         out += stride;
      }
   }
}

// Second order extrapolation: instead of constructing a 2nd order polynomial based on the 3 samples at the edge of
// the image, which would very quickly explode to infinity, we construct a 2nd order polynomial that smoothly connects
// to the image edge (i.e. function value matches the 2 samples at the edge of the image line), and reaches 0 at the
// end of the expanded boundary. This imposes a sort of windowing around the image.
template< typename DataType >
static inline void ExpandBufferSecondOrder(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 1
      dip::uint left,
      dip::uint right
) {
   if( left > 0 ) {
      // Left side
      DataType* in = buffer;
      DataType* out = buffer - stride;
      dfloat b = static_cast< dfloat >( left ) + 1.0;
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat f1 = static_cast< dfloat >( *( in + stride ));
      dfloat d1 = ( b - 1.0 ) / b * d0 - b / ( b + 1.0 ) * f1;
      dfloat d2 = -1.0 / b * d0 + 1.0 / ( b + 1.0 ) * f1;
      for( dip::uint ii = 1; ii <= left; ii++ ) {
         dfloat x = static_cast< dfloat >( ii );
         *out = clamp_cast< DataType >( d0 + x * d1 + x * x * d2 );
         out -= stride;
      }
   }
   if( right > 0 ) {
      // Right side
      DataType* in = buffer + ( static_cast< dip::sint >( pixels ) - 1 ) * stride;
      DataType* out = buffer + static_cast< dip::sint >( pixels ) * stride;
      dfloat b = static_cast< dfloat >( right ) + 1.0;
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat f1 = static_cast< dfloat >( *( in - stride ));
      dfloat d1 = ( b - 1.0 ) / b * d0 - b / ( b + 1.0 ) * f1;
      dfloat d2 = -1.0 / b * d0 + 1.0 / ( b + 1.0 ) * f1;
      for( dip::uint ii = 1; ii <= right; ii++ ) {
         dfloat x = static_cast< dfloat >( ii );
         *out = clamp_cast< DataType >( d0 + x * d1 + x * x * d2 );
         out += stride;
      }
   }
}

// Third order extrapolation: similarly to the 2nd order one, we construct a 3rd order polynomial that smoothly
// connects to the image edge (i.e. function value matches the 2 samples at the edge of the image line), and
// reaches 0 smoothly at the end of the expanded boundary (i.e. function value and first derivative are 0).
// Wolfram Alpha: solve[{c = a - x + y - z , 0 = a + b x + b^2 y + b^3 z , 0 = x + 2 b y + 3 b^2 z },{x,y,z}]
// (with c = the 2nd pixel value from the edge, a = the value at the edge, and b = border)
// (below we set d0 = a, d1 = x, d2 = y, d3 = z, f1 = c)
// x = (a b^3-3 a b-2 a-b^3 c)/(b (b+1)^2)   and   y = (-2 a b^3-3 a b^2+a+2 b^3 c)/(b^2 (b+1)^2)   and   z = (a b^2+2 a b+a-b^2 c)/(b^2 (b+1)^2)
// x = -(2 a)/b + a - (b^2 c)/(b+1)^2   and   y = (2 b c)/(b+1)^2 - (a (2 b-1))/b^2   and   z = a/b^2 - c/(b+1)^2
template< typename DataType >
static inline void ExpandBufferThirdOrder(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 2
      dip::uint left,
      dip::uint right
) {
   if( left > 0 ) {
      // Left side
      DataType* in = buffer;
      DataType* out = buffer - stride;
      dfloat b = static_cast< dfloat >( left ) + 1.0;
      dfloat b12 = ( b + 1 ) * ( b + 1 );
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat f1 = static_cast< dfloat >( *( in + stride ));
      dfloat d1 = -( 2.0 * d0 ) / b + d0 - ( b * b * f1 ) / b12;
      dfloat d2 = ( 2.0 * b * f1 ) / b12 - ( d0 * ( 2.0 * b - 1.0 )) / ( b * b );
      dfloat d3 = d0 / ( b * b ) - f1 / b12;
      for( dip::uint ii = 1; ii <= left; ii++ ) {
         dfloat x = static_cast< dfloat >( ii );
         *out = clamp_cast< DataType >( d0 + x * d1 + x * x * d2 + x * x * x * d3 );
         out -= stride;
      }
   }
   if( right > 0 ) {
      // Right side
      DataType* in = buffer + ( static_cast< dip::sint >( pixels ) - 1 ) * stride;
      DataType* out = buffer + static_cast< dip::sint >( pixels ) * stride;
      dfloat b = static_cast< dfloat >( right ) + 1.0;
      dfloat b12 = ( b + 1 ) * ( b + 1 );
      dfloat d0 = static_cast< dfloat >( *in );
      dfloat f1 = static_cast< dfloat >( *( in - stride ));
      dfloat d1 = -( 2.0 * d0 ) / b + d0 - ( b * b * f1 ) / b12;
      dfloat d2 = ( 2.0 * b * f1 ) / b12 - ( d0 * ( 2.0 * b - 1.0 )) / ( b * b );
      dfloat d3 = d0 / ( b * b ) - f1 / b12;
      for( dip::uint ii = 1; ii <= right; ii++ ) {
         dfloat x = static_cast< dfloat >( ii );
         *out = clamp_cast< DataType >( d0 + x * d1 + x * x * d2 + x * x * x * d3 );
         out += stride;
      }
   }
}

template< typename DataType, bool asymmetric >
static inline void ExpandBufferMirror(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 2
      dip::uint left,
      dip::uint right
) {
   dip::uint steps = pixels - 1;
   // Left side
   DataType* in = buffer;
   DataType* out = buffer;
   for( dip::uint ii = 0; ii < left; ii++ ) {
      bool dir = (( ii / steps ) & 1 ) == 1;
      in -= dir ? stride : -stride;
      out -= stride;
      *out = asymmetric ? ( dir ? *in : saturated_inv( *in )) : *in;
   }
   // Right side
   in = buffer + ( static_cast< dip::sint >( pixels ) - 1 ) * stride;
   out = in;
   for( dip::uint ii = 0; ii < right; ii++ ) {
      bool dir = (( ii / steps ) & 1 ) == 1;
      in += dir ? stride : -stride;
      out += stride;
      *out = asymmetric ? ( dir ? *in : saturated_inv( *in )) : *in;
   }
}

template< typename DataType, bool asymmetric >
static inline void ExpandBufferPeriodic(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 2
      dip::uint left,
      dip::uint right
) {
   // Left side
   DataType* in = buffer; // value not used, set during first iteration of loop
   DataType* out = buffer - stride;
   bool invert = false;   // value modified during first loop iteration (and hopefully optimized out if `!asymmetric`)
   for( dip::uint ii = 0; ii < left; ii++ ) {
      if( !( ii % pixels )) {
         in = buffer + ( static_cast< dip::sint >( pixels ) - 1 ) * stride;
         invert = !invert;
      }
      *out = asymmetric ? ( invert ? saturated_inv( *in ) : *in ) : *in;
      in -= stride;
      out -= stride;
   }
   // Right side
   in = buffer;
   out = buffer + static_cast< dip::sint >( pixels ) * stride;
   invert = false;
   for( dip::uint ii = 0; ii < right; ii++ ) {
      if( !( ii % pixels )) {
         in = buffer;
         invert = !invert;
      }
      *out = asymmetric ? ( invert ? saturated_inv( *in ) : *in ) : *in;
      in += stride;
      out += stride;
   }
}

template< typename DataType >
static inline void ExpandBufferFromTo(
      DataType* buffer,
      dip::sint stride,
      dip::sint tensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dip::uint left,
      dip::uint right,
      BoundaryCondition bc
) {
   switch( bc ) {

      case BoundaryCondition::SYMMETRIC_MIRROR:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, left, right, buffer[ 0 ], buffer[ 0 ] );
            } else {
               ExpandBufferMirror< DataType, false >( buffer, stride, pixels, left, right );
            }
         }
         break;

      case BoundaryCondition::ASYMMETRIC_MIRROR:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, left, right, saturated_inv( buffer[ 0 ] ), saturated_inv( buffer[ 0 ] ));
            } else {
               ExpandBufferMirror< DataType, true >( buffer, stride, pixels, left, right );
            }
         }
         break;

      case BoundaryCondition::PERIODIC:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, left, right, buffer[ 0 ], buffer[ 0 ] );
            } else {
               ExpandBufferPeriodic< DataType, false >( buffer, stride, pixels, left, right );
            }
         }
         break;

      case BoundaryCondition::ASYMMETRIC_PERIODIC:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, left, right, saturated_inv( buffer[ 0 ] ), saturated_inv( buffer[ 0 ] ));
            } else {
               ExpandBufferPeriodic< DataType, true >( buffer, stride, pixels, left, right );
            }
         }
         break;

      case BoundaryCondition::ADD_ZEROS:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, left, right, DataType( 0 ), DataType( 0 ));
         }
         break;

      case BoundaryCondition::ADD_MAX_VALUE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, left, right, std::numeric_limits< DataType >::max(), std::numeric_limits< DataType >::max() );
         }
         break;

      case BoundaryCondition::ADD_MIN_VALUE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, left, right, std::numeric_limits< DataType >::lowest(), std::numeric_limits< DataType >::lowest() );
         }
         break;

      case BoundaryCondition::THIRD_ORDER_EXTRAPOLATE:
         if( pixels > 2 ) {
            for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
               ExpandBufferThirdOrder( buffer, stride, pixels, left, right );
            }
            break;
         }
         // Else: falls through to do second order extrapolation
         // fallthrough

      case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE:
         if( pixels > 1 ) {
            for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
               ExpandBufferSecondOrder( buffer, stride, pixels, left, right );
            }
            break;
         }
         // Else: falls through to do first order extrapolation
         // fallthrough

      case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferFirstOrder( buffer, stride, pixels, left, right );
         }
         break;

      case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, left, right, buffer[ 0 ], buffer[ ( static_cast< dip::sint >( pixels ) - 1 ) * stride ] );
         }
         break;

      default:
         DIP_THROW( E::NOT_IMPLEMENTED );
   }
}

} // namespace

void ExpandBuffer(
      void* buffer,
      DataType type,
      dip::sint stride,
      dip::sint tensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dip::uint left,
      dip::uint right,
      BoundaryCondition bc
) {
   if(( left == 0 ) && ( right == 0 )) {
      // We've got nothing to do
      return;
   }
   DIP_ASSERT( pixels > 0 );
   switch( type ) {
      case dip::DT_BIN:
         ExpandBufferFromTo( static_cast< bin* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_UINT8:
         ExpandBufferFromTo( static_cast< uint8* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_UINT16:
         ExpandBufferFromTo( static_cast< uint16* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_UINT32:
         ExpandBufferFromTo( static_cast< uint32* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_UINT64:
         ExpandBufferFromTo( static_cast< uint64* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SINT8:
         ExpandBufferFromTo( static_cast< sint8* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SINT16:
         ExpandBufferFromTo( static_cast< sint16* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SINT32:
         ExpandBufferFromTo( static_cast< sint32* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SINT64:
         ExpandBufferFromTo( static_cast< sint64* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SFLOAT:
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_DFLOAT:
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ), stride, tensorStride, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_SCOMPLEX:
         // For complex values, we treat real and imaginary components separately
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ),     stride * 2, tensorStride * 2, pixels, tensorElements, left, right, bc );
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ) + 1, stride * 2, tensorStride * 2, pixels, tensorElements, left, right, bc );
         break;
      case dip::DT_DCOMPLEX:
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ),     stride * 2, tensorStride * 2, pixels, tensorElements, left, right, bc );
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ) + 1, stride * 2, tensorStride * 2, pixels, tensorElements, left, right, bc );
         break;
      default:
         DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}


} // namespace detail
} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <numeric>

DOCTEST_TEST_CASE("[DIPlib] testing the CopyBuffer function") {

   std::vector< dip::uint8 > input( 100 );
   std::vector< dip::uint8 > output( 237 ); // part 1, mode 7 and part 2, mode 2 use this many elements
   std::iota( input.begin(), input.end(), 0 );
   bool error;
   dip::uint kk;

   // 1- Copying with identical types

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 1
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_UINT8, 1, 1,
         20, 1 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii ] != 0;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 2
         input.data(), dip::DT_UINT8, 1, 1,
         output.data(), dip::DT_UINT8, 1, 1,
         20, 1 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii ] != kk++;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 3
         input.data(), dip::DT_UINT8, 1, 1,
         output.data(), dip::DT_UINT8, 3, 1,
         20, 1 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 3 ] != kk++;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 4
         input.data(), dip::DT_UINT8, 0, 0,
         output.data(), dip::DT_UINT8, 5, 1,
         20, 5 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 5 + jj ] != 0;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 5
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_UINT8, 5, 1,
         20, 5 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      kk = 0;
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 5 + jj ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 6
         input.data(), dip::DT_UINT8, 5, 1,
         output.data(), dip::DT_UINT8, 5, 1,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 5 + jj ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 7
         input.data(), dip::DT_UINT8, 5, 1,
         output.data(), dip::DT_UINT8, 6, 1,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 6 + jj ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 8
         input.data(), dip::DT_UINT8, 1, 20,
         output.data(), dip::DT_UINT8, 1, 20,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint jj = 0; jj < 5; ++jj ) {
      for( dip::uint ii = 0; ii < 20; ++ii ) {
         error |= output[ ii + jj * 20 ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 9
         input.data(), dip::DT_UINT8, 1, 20,
         output.data(), dip::DT_UINT8, 1, 30,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint jj = 0; jj < 5; ++jj ) {
      for( dip::uint ii = 0; ii < 20; ++ii ) {
         error |= output[ ii + jj * 30 ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 10
         input.data(), dip::DT_UINT8, 1, 0,
         output.data(), dip::DT_UINT8, 12, 2,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != kk;
      }
      ++kk;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 11
         input.data(), dip::DT_UINT8, 5, 1,
         output.data(), dip::DT_UINT8, 12, 2,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 12
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_UINT8, 4, 1,
         20, 2, dip::Tensor{ dip::Tensor::Shape::DIAGONAL_MATRIX, 2, 2 }.LookUpTable() );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 4 + 0 ] != 0;
      error |= output[ ii * 4 + 1 ] != 0;
      error |= output[ ii * 4 + 2 ] != 0;
      error |= output[ ii * 4 + 3 ] != 1;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 13
         input.data(), dip::DT_UINT8, 2, 1,
         output.data(), dip::DT_UINT8, 4, 1,
         20, 2, dip::Tensor{ dip::Tensor::Shape::DIAGONAL_MATRIX, 2, 2 }.LookUpTable() );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 4 + 0 ] != kk++;
      error |= output[ ii * 4 + 1 ] != 0;
      error |= output[ ii * 4 + 2 ] != 0;
      error |= output[ ii * 4 + 3 ] != kk++;
   }
   DOCTEST_CHECK_FALSE( error );

   // 2- Copying with different types

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 1
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_SINT8, 3, 1,
         20, 1 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 3 ] != 0;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 2
         input.data(), dip::DT_UINT8, 1, 1,
         output.data(), dip::DT_SINT8, 3, 1,
         20, 1 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 3 ] != kk++;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 3
         input.data(), dip::DT_UINT8, 0, 0,
         output.data(), dip::DT_SINT8, 12, 2,
         20, 5 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != 0;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 4
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_SINT8, 12, 2,
         20, 5 );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      kk = 0;
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 5
         input.data(), dip::DT_UINT8, 1, 0,
         output.data(), dip::DT_SINT8, 12, 2,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != kk;
      }
      ++kk;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 6
         input.data(), dip::DT_UINT8, 5, 1,
         output.data(), dip::DT_SINT8, 12, 2,
         20, 5 );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      for( dip::uint jj = 0; jj < 5; ++jj ) {
         error |= output[ ii * 12 + jj * 2 ] != kk++;
      }
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 7
         input.data(), dip::DT_UINT8, 0, 1,
         output.data(), dip::DT_SINT8, 5, 1,
         20, 2, dip::Tensor{ dip::Tensor::Shape::DIAGONAL_MATRIX, 2, 2 }.LookUpTable() );
   error = false;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 5 + 0 ] != 0;
      error |= output[ ii * 5 + 1 ] != 0;
      error |= output[ ii * 5 + 2 ] != 0;
      error |= output[ ii * 5 + 3 ] != 1;
   }
   DOCTEST_CHECK_FALSE( error );

   std::fill( output.begin(), output.end(), 101 );
   dip::detail::CopyBuffer( // mode 8
         input.data(), dip::DT_UINT8, 2, 1,
         output.data(), dip::DT_SINT8, 5, 1,
         20, 2, dip::Tensor{ dip::Tensor::Shape::DIAGONAL_MATRIX, 2, 2 }.LookUpTable() );
   error = false;
   kk = 0;
   for( dip::uint ii = 0; ii < 20; ++ii ) {
      error |= output[ ii * 5 + 0 ] != kk++;
      error |= output[ ii * 5 + 1 ] != 0;
      error |= output[ ii * 5 + 2 ] != 0;
      error |= output[ ii * 5 + 3 ] != kk++;
   }
   DOCTEST_CHECK_FALSE( error );
}

#endif // DIP__ENABLE_DOCTEST
