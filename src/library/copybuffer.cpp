/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <limits>

#include "copybuffer.h"
#include "diplib/clamp_cast.h"

//#include <iostream>

namespace dip {

// CopyBuffer()

template< typename inT, typename outT >
static inline void CopyBufferFromTo(
      inT* inBuffer,
      dip::sint inStride,
      dip::sint inTensorStride,
      outT* outBuffer,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      std::vector< dip::sint > const& lookUpTable // it this is null, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
) {
   //std::cout << "CopyBuffer: ";
   for( dip::uint pp = 0; pp < pixels; ++pp ) {
      inT* in = inBuffer;
      outT* out = outBuffer;
      if( lookUpTable.empty() ) {
         for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
            *out = clamp_cast< outT >( *in );
            //std::cout << *out << " ";
            in += inTensorStride;
            out += outTensorStride;
         }
      } else {
         for( dip::uint tt = 0; tt < lookUpTable.size(); ++tt ) {
            *out = lookUpTable[ tt ] < 0 ? outT( 0 ) : clamp_cast< outT >( in[ lookUpTable[ tt ] ] );
            //std::cout << *out << " ";
            out += outTensorStride;
         }
      }
      inBuffer += inStride;
      outBuffer += outStride;
   }
   //std::cout << std::endl;
}

template< typename inT >
static inline void CopyBufferFrom(
      inT* inBuffer,
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
      case dip::DT_SINT8:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint8* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT16:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint16* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT32:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, static_cast< sint32* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, lookUpTable );
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
   }
}

void CopyBuffer(
      void* inBuffer,
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
   switch( inType ) {
      case dip::DT_BIN:
         CopyBufferFrom( static_cast< bin* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT8:
         CopyBufferFrom( static_cast< uint8* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT16:
         CopyBufferFrom( static_cast< uint16* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_UINT32:
         CopyBufferFrom( static_cast< uint32* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT8:
         CopyBufferFrom( static_cast< sint8* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT16:
         CopyBufferFrom( static_cast< sint16* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT32:
         CopyBufferFrom( static_cast< sint32* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SFLOAT:
         CopyBufferFrom( static_cast< sfloat* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DFLOAT:
         CopyBufferFrom( static_cast< dfloat* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SCOMPLEX:
         CopyBufferFrom( static_cast< scomplex* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_DCOMPLEX:
         CopyBufferFrom( static_cast< dcomplex* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
   }
}


// FillBuffer()

template< typename inT, typename outT >
static inline void FillBufferFromTo(
      outT* outBuffer,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      inT value
) {
   outT v = clamp_cast< outT >( value );
   for( dip::uint pp = 0; pp < pixels; ++pp ) {
      outT* out = outBuffer;
      for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
         * out = v;
         out += outTensorStride;
      }
      outBuffer += outStride;
   }
}

template< typename inT >
static inline void FillBufferFrom(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      inT value
) {
   switch( outType ) {
      case dip::DT_BIN:
         FillBufferFromTo( static_cast< bin* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT8:
         FillBufferFromTo( static_cast< uint8* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT16:
         FillBufferFromTo( static_cast< uint16* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT32:
         FillBufferFromTo( static_cast< uint32* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT8:
         FillBufferFromTo( static_cast< sint8* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT16:
         FillBufferFromTo( static_cast< sint16* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT32:
         FillBufferFromTo( static_cast< sint32* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SFLOAT:
         FillBufferFromTo( static_cast< sfloat* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_DFLOAT:
         FillBufferFromTo( static_cast< dfloat* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SCOMPLEX:
         FillBufferFromTo( static_cast< scomplex* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_DCOMPLEX:
         FillBufferFromTo( static_cast< dcomplex* >( outBuffer ), outStride, outTensorStride, pixels, tensorElements, value );
         break;
   }
}

void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dip::sint value
) {
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value );
}

void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dfloat value
) {
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value );
}

void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dcomplex value
) {
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value );
}

} // namespace dip
