/*
 * DIPlib 3.0
 * This file contains the definition the arithmetic operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <iostream>

#include "diplib.h"
#include "dip_framework.h"
#include "dip_overload.h"
#include "dip_saturated_arithmetic.h"

namespace dip {

//
template< typename TPI >
static void dip__Add(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   TPI* out = (TPI*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      // Note: we don't loop over tensor elements because we use the Scan_TensorAsSpatialDim option.
      *out = saturated_add( *lhs, *rhs);
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Add(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__Add, dt );
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Sub(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   TPI* out = (TPI*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      // Note: we don't loop over tensor elements because we use the Scan_TensorAsSpatialDim option.
      *out = saturated_sub( *lhs, *rhs );
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Sub(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__Sub, dt );
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Mul(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   TPI* out = (TPI*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      // TODO: do vector/matrix multiplication
      *out = saturated_mul( *lhs, *rhs );
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Mul(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   dip_ThrowIf( lhs.TensorColumns() != rhs.TensorRows(), "Inner tensor dimensions must match in multiplication" );

   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts;
   UnsignedArray nTensorOut{ lhs.TensorRows() * rhs.TensorColumns() };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__Mul, dt );
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
   out.ReshapeTensor( lhs.TensorRows(), rhs.TensorColumns() );
}

//
template< typename TPI >
static void dip__Div(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   TPI* out = (TPI*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      *out = saturated_div( *lhs, *rhs );
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Div(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   dip_ThrowIf( rhs.TensorElements() != 1, "Divisor must be scalar image" );
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_ALL( filter, dip__Div, dt );
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Mod(
      const std::vector<Framework::ScanBuffer>&   inBuffer,
      std::vector<Framework::ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
) {
   const TPI* lhs = (const TPI*)inBuffer[0].buffer;
   const TPI* rhs = (const TPI*)inBuffer[1].buffer;
   TPI* out = (TPI*)outBuffer[0].buffer;
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      //*out = *lhs % *rhs; // this only works for integer types.
      *out = static_cast<TPI>( std::fmod( *lhs, *rhs ));
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Mod(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   dip_ThrowIf( rhs.TensorElements() != 1, "Divisor must be scalar image" );
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_REAL( filter, dip__Mod, dt ); // NOTE: non-binary and non-complex.
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

} // namespace dip
