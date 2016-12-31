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
class dip__And : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] &
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void And(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_INT_OR_BIN( scanLineFilter, dip__And, (), dt ); // NOTE: binary and integer.
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Or : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] |
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Or(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_INT_OR_BIN( scanLineFilter, dip__Or, (), dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Xor : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] ^
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Xor(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = lhs.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_INT_OR_BIN( scanLineFilter, dip__Xor, (), dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Not : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            // Tensor dimension is 1 because we request `Scan_TensorAsSpatialDim`
            *out = ~*in;
            in += inStride;
            out += outStride;
         }
      }
};

void Not(
      Image const& in,
      Image& out
) {
   DataType dt = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_INT_OR_BIN( scanLineFilter, dip__Not, (), dt ); // NOTE: binary and integer.
   std::vector< void* > vars;
   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   Framework::ScanMonadic( in, out, dt, dt, 1, scanLineFilter.get(), opts );
}


} // namespace dip
