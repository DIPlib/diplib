/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

//
template< typename TPI >
static void dip__And(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      const void* functionParameters,
      void* functionVariables
) {
   TPI const* lhs = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   TPI const* rhs = static_cast< TPI const* >( inBuffer[ 1 ].buffer );
   TPI* out = static_cast< TPI* >( outBuffer[ 0 ].buffer );
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[ 0 ].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[ jj * outBuffer[ 0 ].tensorStride ] = lhs[ jj * inBuffer[ 0 ].tensorStride ] &
                                                   rhs[ jj * inBuffer[ 1 ].tensorStride ];
      }
      lhs += inBuffer[ 0 ].stride;
      rhs += inBuffer[ 1 ].stride;
      out += outBuffer[ 0 ].stride;
   }
}

void And(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__And, dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Or(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      const void* functionParameters,
      void* functionVariables
) {
   TPI const* lhs = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   TPI const* rhs = static_cast< TPI const* >( inBuffer[ 1 ].buffer );
   TPI* out = static_cast< TPI* >( outBuffer[ 0 ].buffer );
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[ 0 ].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[ jj * outBuffer[ 0 ].tensorStride ] = lhs[ jj * inBuffer[ 0 ].tensorStride ] |
                                                   rhs[ jj * inBuffer[ 1 ].tensorStride ];
      }
      lhs += inBuffer[ 0 ].stride;
      rhs += inBuffer[ 1 ].stride;
      out += outBuffer[ 0 ].stride;
   }
}

void Or(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__Or, dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Xor(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      const void* functionParameters,
      void* functionVariables
) {
   TPI const* lhs = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   TPI const* rhs = static_cast< TPI const* >( inBuffer[ 1 ].buffer );
   TPI* out = static_cast< TPI* >( outBuffer[ 0 ].buffer );
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      for( dip::uint jj = 0; jj < outBuffer[ 0 ].tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
         out[ jj * outBuffer[ 0 ].tensorStride ] = lhs[ jj * inBuffer[ 0 ].tensorStride ] ^
                                                   rhs[ jj * inBuffer[ 1 ].tensorStride ];
      }
      lhs += inBuffer[ 0 ].stride;
      rhs += inBuffer[ 1 ].stride;
      out += outBuffer[ 0 ].stride;
   }
}

void Xor(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__Xor, dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, filter, nullptr, vars, opts );
}

//
template< typename TPI >
static void dip__Not(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      const void* functionParameters,
      void* functionVariables
) {
   TPI const* in = static_cast< TPI const* >( inBuffer[ 0 ].buffer );
   TPI* out = static_cast< TPI* >( outBuffer[ 0 ].buffer );
   for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
      // Tensor dimension is 1 because we request `Scan_TensorAsSpatialDim`
      * out = ~* in;
      in += inBuffer[ 0 ].stride;
      out += outBuffer[ 0 ].stride;
   }
}

void Not(
      Image const& in,
      Image& out
) {
   DataType dt = in.DataType();
   Framework::ScanFilter filter;
   DIP_OVL_ASSIGN_INT_OR_BIN( filter, dip__Not, dt );
   std::vector< void* > vars;
   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   Framework::ScanMonadic( in, out, dt, dt, 1, filter, nullptr, vars, opts );
}


} // namespace dip
