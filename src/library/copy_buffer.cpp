/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <limits>

#include "copy_buffer.h"
#include "diplib/saturated_arithmetic.h"

//#include <iostream>

namespace dip {

//
// CopyBuffer()
//


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
      std::vector< dip::sint > const& lookUpTable // it this is null, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
) {
   //std::cout << "CopyBuffer: ";
   if( tensorElements == 1 ) {
      for( dip::uint pp = 0; pp < pixels; ++pp ) {
         *outBuffer = clamp_cast< outT >( *inBuffer );
         inBuffer += inStride;
         outBuffer += outStride;
      }
   } else {
      for( dip::uint pp = 0; pp < pixels; ++pp ) {
         inT const* in = inBuffer;
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
   }
   //std::cout << std::endl;
}
// TODO: make a template specialization where inT == outT, using std::memcpy when there's multiple tensor elements and tstride == 1.

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
      case dip::DT_SINT8:
         CopyBufferFrom( static_cast< sint8 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT16:
         CopyBufferFrom( static_cast< sint16 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
         break;
      case dip::DT_SINT32:
         CopyBufferFrom( static_cast< sint32 const* >( inBuffer ), inStride, inTensorStride, outBuffer, outType, outStride, outTensorStride, pixels, tensorElements, lookUpTable );
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
   }
}


//
// ExpandBuffer()
//


template< typename DataType >
static inline void ExpandBufferConstant(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels,
      dip::uint border, // guaranteed larger than 0
      DataType leftValue,
      DataType rightValue
) {
   DataType* out = buffer - stride;
   for( dip::sint ii = 0; ii < dip::sint( border ); ++ii ) {
      *out = leftValue;
      out -= stride;
   }
   out = buffer + pixels * stride;
   for( dip::sint ii = 0; ii < dip::sint( border ); ++ii ) {
      *out = rightValue;
      out += stride;
   }
}

// First order extrapolation: we construct a 1st order polynomial based on the 2 samples at the edge of the
// image line. This polynomial is used to fill out the expanded border.
template< typename DataType >
static inline void ExpandBufferFirstOrder(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 1
      dip::uint border  // guaranteed larger than 0
) {
   // Left side
   DataType* in = buffer;
   DataType* out = buffer - stride;
   dfloat d0 = *in;
   dfloat d1 = dfloat( *in ) - dfloat( *( in + stride ));
   for( dip::uint ii = 0; ii < border; ii++ ) {
      d0 += d1;
      *out = clamp_cast< DataType >( d0 );
      out -= stride;
   }
   // Right side
   in = buffer + ( pixels - 1 ) * stride;
   out = buffer + pixels * stride;
   d0 = *in;
   d1 = dfloat( *in ) - dfloat( *( in - stride ));
   for( dip::uint ii = 0; ii < border; ii++ ) {
      d0 += d1;
      *out = clamp_cast< DataType >( d0 );
      out += stride;
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
      dip::uint border  // guaranteed larger than 0
) {
   dfloat b = border + 1;
   // Left side
   DataType* in = buffer;
   DataType* out = buffer - stride;
   dfloat d0 = *in;
   dfloat f1 = *( in + stride );
   dfloat d1 = ( b - 1.0 ) / b * d0 - b / ( b + 1.0 ) * f1;
   dfloat d2 = -1.0 / b * d0 + 1.0 / ( b + 1.0 ) * f1;
   for( dip::uint ii = 1; ii <= border; ii++ ) {
      *out = clamp_cast< DataType >( d0 + ii * d1 + ii * ii * d2 );
      out -= stride;
   }
   // Right side
   in = buffer + ( pixels - 1 ) * stride;
   out = buffer + pixels * stride;
   d0 = *in;
   f1 = *( in - stride );
   d1 = ( b - 1.0 ) / b * d0 - b / ( b + 1.0 ) * f1;
   d2 = -1.0 / b * d0 + 1.0 / ( b + 1.0 ) * f1;
   for( dip::uint ii = 1; ii <= border; ii++ ) {
      *out = clamp_cast< DataType >( d0 + ii * d1 + ii * ii * d2 );
      out += stride;
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
      dip::uint border  // guaranteed larger than 0
) {
   dfloat b = border + 1;
   dfloat b12 = ( b + 1 ) * ( b + 1 );
   // Left side
   DataType* in = buffer;
   DataType* out = buffer - stride;
   dfloat d0 = *in;
   dfloat f1 = *( in + stride );
   dfloat d1 = -( 2.0 * d0 ) / b + d0 - ( b * b * f1 ) / b12;
   dfloat d2 = ( 2.0 * b * f1 ) / b12 - ( d0 * ( 2.0 * b - 1.0 ) ) / ( b * b );
   dfloat d3 = d0 / ( b * b ) - f1 / b12;
   for( dip::uint ii = 1; ii <= border; ii++ ) {
      *out = clamp_cast< DataType >( d0 + ii * d1 + ii * ii * d2 + ii * ii * ii * d3 );
      out -= stride;
   }
   // Right side
   in = buffer + ( pixels - 1 ) * stride;
   out = buffer + pixels * stride;
   d0 = *in;
   f1 = *( in - stride );
   d1 = -( 2.0 * d0 ) / b + d0 - ( b * b * f1 ) / b12;
   d2 = ( 2.0 * b * f1 ) / b12 - ( d0 * ( 2.0 * b - 1.0 ) ) / ( b * b );
   d3 = d0 / ( b * b ) - f1 / b12;
   for( dip::uint ii = 1; ii <= border; ii++ ) {
      *out = clamp_cast< DataType >( d0 + ii * d1 + ii * ii * d2 + ii * ii * ii * d3 );
      out += stride;
   }
}

template< typename DataType, bool invert >
static inline void ExpandBufferMirror(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 2
      dip::uint border  // guaranteed larger than 0
) {
   // Left side
   DataType* in = buffer;
   DataType* out = buffer - stride;
   for( dip::uint ii = 0; ii < border; ii++ ) {
      *out = invert ? saturated_inv( *in ) : *in;
      in -= (( ii / pixels ) & 1 ) ? stride : -stride;
      out -= stride;
   }
   // Right side
   in = buffer + ( pixels - 1 ) * stride;
   out = buffer + pixels * stride;
   for( dip::uint ii = 0; ii < border; ii++ ) {
      *out = invert ? saturated_inv( *in ) : *in;
      in += (( ii / pixels ) & 1 ) ? stride : -stride;
      out += stride;
   }
}

template< typename DataType, bool invert >
static inline void ExpandBufferPeriodic(
      DataType* buffer,
      dip::sint stride,
      dip::uint pixels, // guaranteed larger than 2
      dip::uint border  // guaranteed larger than 0
) {
// Left side
   DataType* in = buffer + ( pixels - 1 ) * stride;
   DataType* out = buffer - stride;
   for( dip::uint ii = 0; ii < border; ii++ ) {
      *out = invert ? saturated_inv( *in ) : *in;
      if( !( ii % pixels ) ) {
         in = buffer + ( pixels - 1 ) * stride;
      }
      in -= stride;
      out -= stride;
   }
   // Right side
   in = buffer;
   out = buffer + pixels * stride;
   for( dip::uint ii = 0; ii < border; ii++ ) {
      *out = invert ? saturated_inv( *in ) : *in;
      if( !( ii % pixels ) ) {
         in = buffer;
      }
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
      dip::uint border,
      BoundaryCondition bc
) {
   switch( bc ) {

      case BoundaryCondition::SYMMETRIC_MIRROR:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, border, buffer[ 0 ], buffer[ 0 ] );
            } else {
               ExpandBufferMirror< DataType, false >( buffer, stride, pixels, border );
            }
         }
         break;

      case BoundaryCondition::ASYMMETRIC_MIRROR:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, border, saturated_inv( buffer[ 0 ] ), saturated_inv( buffer[ 0 ] ));
            } else {
               ExpandBufferMirror< DataType, true >( buffer, stride, pixels, border );
            }
         }
         break;

      case BoundaryCondition::PERIODIC:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, border, buffer[ 0 ], buffer[ 0 ] );
            } else {
               ExpandBufferPeriodic< DataType, false >( buffer, stride, pixels, border );
            }
         }
         break;

      case BoundaryCondition::ASYMMETRIC_PERIODIC:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            if( pixels == 1 ) {
               ExpandBufferConstant( buffer, stride, pixels, border, saturated_inv( buffer[ 0 ] ), saturated_inv( buffer[ 0 ] ));
            } else {
               ExpandBufferPeriodic< DataType, true >( buffer, stride, pixels, border );
            }
         }
         break;

      case BoundaryCondition::ADD_ZEROS:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, border, DataType( 0 ), DataType( 0 ));
         }
         break;

      case BoundaryCondition::ADD_MAX_VALUE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, border, std::numeric_limits< DataType >::max(), std::numeric_limits< DataType >::max() );
         }
         break;

      case BoundaryCondition::ADD_MIN_VALUE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, border, std::numeric_limits< DataType >::lowest(), std::numeric_limits< DataType >::lowest() );
         }
         break;

      case BoundaryCondition::THIRD_ORDER_EXTRAPOLATE:
         if( pixels > 2 ) {
            for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
               ExpandBufferThirdOrder( buffer, stride, pixels, border );
            }
            break;
         }
         // Else: fall-through to do second order extrapolation

      case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE:
         if( pixels > 1 ) {
            for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
               ExpandBufferSecondOrder( buffer, stride, pixels, border );
            }
            break;
         }
         // Else: fall-through (twice) to do zero order extrapolation

      case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:
         if( pixels > 1 ) {
            for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
               ExpandBufferFirstOrder( buffer, stride, pixels, border );
            }
            break;
         }
         // Else: fall-through to do zero order extrapolation

      case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
         for( dip::uint jj = 0; jj < tensorElements; ++jj, buffer += tensorStride ) {
            ExpandBufferConstant( buffer, stride, pixels, border, buffer[ 0 ], buffer[ ( pixels - 1 ) * stride ] );
         }
         break;
   }
}

void ExpandBuffer(
      void* buffer,
      DataType type,
      dip::sint stride,
      dip::sint tensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dip::uint border,
      BoundaryCondition bc
) {
   if( border == 0) {
      // We've got nothing to do
      return;
   }
   switch( type ) {
      case dip::DT_BIN:
         ExpandBufferFromTo( static_cast< bin* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_UINT8:
         ExpandBufferFromTo( static_cast< uint8* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_UINT16:
         ExpandBufferFromTo( static_cast< uint16* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_UINT32:
         ExpandBufferFromTo( static_cast< uint32* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_SINT8:
         ExpandBufferFromTo( static_cast< sint8* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_SINT16:
         ExpandBufferFromTo( static_cast< sint16* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_SINT32:
         ExpandBufferFromTo( static_cast< sint32* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_SFLOAT:
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_DFLOAT:
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ), stride, tensorStride, pixels, tensorElements, border, bc );
         break;
      case dip::DT_SCOMPLEX:
         // For complex values, we treat real and imaginary components separately
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ),     stride * 2, tensorStride * 2, pixels, tensorElements, border, bc );
         ExpandBufferFromTo( static_cast< sfloat* >( buffer ) + 1, stride * 2, tensorStride * 2, pixels, tensorElements, border, bc );
         break;
      case dip::DT_DCOMPLEX:
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ),     stride * 2, tensorStride * 2, pixels, tensorElements, border, bc );
         ExpandBufferFromTo( static_cast< dfloat* >( buffer ) + 1, stride * 2, tensorStride * 2, pixels, tensorElements, border, bc );
         break;
   }
}


//
// FillBuffer()
//


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
