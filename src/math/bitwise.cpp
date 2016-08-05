/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"
#include "dip_overload.h"

namespace dip {

//
template< typename TPI >
static void dip__And(
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
      *out = *lhs & *rhs;
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void And(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__And, dt ); // NOTE: binary and integer.
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Or(
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
      *out = *lhs | *rhs;
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Or(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__Or, dt ); // NOTE: binary and integer.
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Xor(
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
      *out = *lhs ^ *rhs;
      lhs += inBuffer[0].stride;
      rhs += inBuffer[1].stride;
      out += outBuffer[0].stride;
   }
}

void Xor(
      const Image& lhs,
      const Image& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   ImageConstRefArray inar { lhs, rhs };
   ImageRefArray outar { out };
   DataTypeArray inBufferTypes { dt, dt };
   DataTypeArray outBufferTypes { dt };
   DataTypeArray outImageTypes { dt };

   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   UnsignedArray nTensorOut{ 1 };
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__Xor, dt ); // NOTE: binary and integer.
   std::vector<void*> vars;
   Framework::Scan( inar, outar, inBufferTypes, outBufferTypes, outImageTypes,
                    nTensorOut, filter, nullptr, vars, opts );
}

} // namespace dip
