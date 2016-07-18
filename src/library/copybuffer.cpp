/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <limits>

#include "copybuffer.h"
#include "dip_clamp_cast.h"

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
      dip::uint tensorElements
) {
   for( dip::uint pp = 0; pp < pixels; ++pp ) {
      inT* in = inBuffer;
      outT* out = outBuffer;
      for( dip::uint tt = 0; tt < tensorElements; ++tt ) {
         *out = clamp_cast< outT >( *in );
         in += inTensorStride;
         out += outTensorStride;
      }
      inBuffer += inStride;
      outBuffer += outStride;
   }
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
      dip::uint tensorElements
) {
   switch( outType ) {
      case dip::DT_BIN:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (bin*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT8:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (uint8*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT16:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (uint16*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT32:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (uint32*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT8:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (sint8*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT16:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (sint16*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT32:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (sint32*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SFLOAT:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (sfloat*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_DFLOAT:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (dfloat*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SCOMPLEX:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (scomplex*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_DCOMPLEX:
         CopyBufferFromTo( inBuffer, inStride, inTensorStride, (dcomplex*)outBuffer, outStride, outTensorStride, pixels, tensorElements );
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
      dip::uint tensorElements
) {
   switch( inType ) {
      case dip::DT_BIN:
         CopyBufferFrom( (bin*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT8:
         CopyBufferFrom( (uint8*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT16:
         CopyBufferFrom( (uint16*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_UINT32:
         CopyBufferFrom( (uint32*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT8:
         CopyBufferFrom( (sint8*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT16:
         CopyBufferFrom( (sint16*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SINT32:
         CopyBufferFrom( (sint32*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SFLOAT:
         CopyBufferFrom( (sfloat*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_DFLOAT:
         CopyBufferFrom( (dfloat*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_SCOMPLEX:
         CopyBufferFrom( (scomplex*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
         break;
      case dip::DT_DCOMPLEX:
         CopyBufferFrom( (dcomplex*)inBuffer, inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements );
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
         *out = v;
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
         FillBufferFromTo( (bin*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT8:
         FillBufferFromTo( (uint8*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT16:
         FillBufferFromTo( (uint16*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_UINT32:
         FillBufferFromTo( (uint32*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT8:
         FillBufferFromTo( (sint8*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT16:
         FillBufferFromTo( (sint16*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SINT32:
         FillBufferFromTo( (sint32*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SFLOAT:
         FillBufferFromTo( (sfloat*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_DFLOAT:
         FillBufferFromTo( (dfloat*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_SCOMPLEX:
         FillBufferFromTo( (scomplex*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
         break;
      case dip::DT_DCOMPLEX:
         FillBufferFromTo( (dcomplex*)outBuffer, outStride, outTensorStride, pixels, tensorElements, value );
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
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value);
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
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value);
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
   FillBufferFrom( outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, value);
}

} // namespace dip
