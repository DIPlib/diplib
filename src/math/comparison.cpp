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
class dip__Equal : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] ==
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Equal(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__Equal, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


//
template< typename TPI >
class dip__NotEqual : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] !=
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void NotEqual(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__NotEqual, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


//
template< typename TPI >
class dip__Lesser : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] <
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Lesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__Lesser, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


//
template< typename TPI >
class dip__Greater : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] >
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Greater(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__Greater, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


//
template< typename TPI >
class dip__NotGreater : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] <=
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void NotGreater(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__NotGreater, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


//
template< typename TPI >
class dip__NotLesser : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
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
               out[ jj * outTensorStride ] = lhs[ jj * lhsTensorStride ] >=
                                             rhs[ jj * rhsTensorStride ];
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void NotLesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDiadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_NONCOMPLEX( scanLineFilter, dip__NotLesser, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, scanLineFilter.get(), opts );
}


} // namespace dip
