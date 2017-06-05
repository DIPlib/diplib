/*
 * DIPlib 3.0
 * This file contains implements dyadic operators.
 *
 * (c)2017, Cris Luengo.
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
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

void Atan2( Image const& y, Image const& x, Image& out ) {
   DIP_THROW_IF( !y.IsScalar() || !x.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( y.DataType(), x.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::atan2( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   ImageRefArray outar{ out };
   Framework::Scan( { y, x }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

void Hypot( Image const& a, Image const& b, Image& out ) {
   DIP_THROW_IF( !a.IsScalar() || !b.IsScalar(), E::IMAGE_NOT_SCALAR );
   DataType dt = DataType::SuggestArithmetic( a.DataType(), b.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::hypot( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   ImageRefArray outar{ out };
   Framework::Scan( { a, b }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

namespace {

template< typename TPI, typename F >
class DIP_EXPORT MultiScanLineFilter : public Framework::ScanLineFilter {
   public:
      MultiScanLineFilter( F const& func ) : func_( func ) {}
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint N = params.inBuffer.size();
         dip::uint const bufferLength = params.bufferLength;
         std::vector< TPI const* > in( N );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            in[ ii ] = static_cast< TPI const* >( params.inBuffer[ ii ].buffer );
         }
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            TPI res = *in[ 0 ];
            in[ 0 ] += params.inBuffer[ 0 ].stride;
            for( dip::uint ii = 1; ii < N; ++ii ) {
               res = func_( res, *in[ ii ] );
               in[ ii ] += params.inBuffer[ ii ].stride;
            }
            *out = res;
            out += outStride;
         }
      }
   private:
      F const& func_;
};

template< typename TPI, typename F >
inline std::unique_ptr< Framework::ScanLineFilter > NewMultiScanLineFilter( F const& func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new MultiScanLineFilter< TPI, F >( func ));
}

} // namespace

void Supremum( ImageConstRefArray const& in, Image& out ) {
   DIP_THROW_IF( in.size() < 2, "Need at least two input images" );
   DataType dt = in[ 0 ].get().DataType();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      dt = DataType::SuggestDyadicOperation( dt, in[ ii ].get().DataType() );
   }
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewMultiScanLineFilter, (
         []( auto a, auto b ) { return std::max( a, b ); }
   ), dt );
   ImageRefArray outar{ out };
   DataTypeArray buftypes( in.size(), dt );
   Framework::Scan( in, outar, buftypes, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

void Infimum( ImageConstRefArray const& in, Image& out ) {
   DIP_THROW_IF( in.size() < 2, "Need at least two input images" );
   DataType dt = in[ 0 ].get().DataType();
   for( dip::uint ii = 1; ii < in.size(); ++ii ) {
      dt = DataType::SuggestDyadicOperation( dt, in[ ii ].get().DataType() );
   }
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_NONCOMPLEX( scanLineFilter, NewMultiScanLineFilter, (
         []( auto a, auto b ) { return std::min( a, b ); }
   ), dt );
   ImageRefArray outar{ out };
   DataTypeArray buftypes( in.size(), dt );
   Framework::Scan( in, outar, buftypes, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

void SignedMinimum ( Image const& a, Image const& b, Image& out ) {
   DataType dt = DataType::SuggestSigned( a.DataType() );
   dt = DataType::SuggestDyadicOperation( dt, b.DataType() );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return *its[ 0 ] > *its[ 1 ] ? static_cast< decltype( *its[ 1 ] ) >( -( *its[ 1 ] )) : *its[ 0 ]; }
   ), dt );
   ImageRefArray outar{ out };
   Framework::Scan( { a, b }, outar, { dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}

} // namespace dip
