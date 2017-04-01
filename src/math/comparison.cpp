/*
 * DIPlib 3.0
 * This file contains the definition the bit-wise operators.
 *
 * (c)2016-2017, Cris Luengo.
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
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

// This are the same class as Framework::NadicScanLineFilter, but with a binary output.
template< dip::uint N, typename TPI, typename F >
class NadicScanLineFilterBinOut : public Framework::ScanLineFilter {
   // Note that N is a compile-time constant, and consequently the compiler should be able to optimize all the loops
   // over N.
   public:
      static_assert( N > 0, "NadicScanLineFilterBinOut does not work without input images" );
      NadicScanLineFilterBinOut( F const& func ) : func_( func ) {}
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         DIP_ASSERT( params.inBuffer.size() == N );
         DIP_ASSERT( params.outBuffer.size() == 1 );
         std::array< TPI const*, N > in;
         std::array< dip::sint, N > inStride;
         std::array< dip::sint, N > inTensorStride;
         dip::uint const bufferLength = params.bufferLength;
         dip::uint const tensorLength = params.outBuffer[ 0 ].tensorLength; // all buffers have same number of tensor elements
         for( dip::uint ii = 0; ii < N; ++ii ) {
            in[ ii ] = static_cast< TPI const* >( params.inBuffer[ ii ].buffer );
            inStride[ ii ] = params.inBuffer[ ii ].stride;
            if( tensorLength > 1 ) {
               inTensorStride[ ii ] = params.inBuffer[ ii ].tensorStride;
            }
            DIP_ASSERT( params.inBuffer[ ii ].tensorLength == tensorLength );
         }
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::sint const outTensorStride = params.outBuffer[ 0 ].tensorStride;
         if( tensorLength > 1 ) {
            for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
               std::array< TPI const*, N > inT = in;
               bin* outT = out;
               for( dip::uint jj = 0; jj < tensorLength; ++jj ) {
                  *outT = func_( inT );
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     inT[ ii ] += inTensorStride[ ii ];
                  }
                  outT += outTensorStride;
               }
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  in[ ii ] += inStride[ ii ];
               }
               out += outStride;
            }
         } else {
            for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
               *out = func_( in );
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  in[ ii ] += inStride[ ii ];
               }
               out += outStride;
            }
         }
      }
   private:
      F const& func_;
};

template< typename TPI, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewDyadicScanLineFilterBinOut( F const& func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new NadicScanLineFilterBinOut< 2, TPI, F >( func ));
}

} // namespace

//
void Equal(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] == *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


//
void NotEqual(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] != *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


//
void Lesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] < *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


//
void Greater(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] > *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


//
void NotGreater(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] <= *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


//
void NotLesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
) {
   DataType dt = DataType::SuggestDyadicOperation( lhs.DataType(), rhs.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewDyadicScanLineFilterBinOut, (
         []( auto its ) { return *its[ 0 ] >= *its[ 1 ]; }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, DT_BIN, *scanLineFilter );
}


} // namespace dip
