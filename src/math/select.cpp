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
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {


namespace {

template< typename TPI, typename F >
class Select1ScanLineFilter : public Framework::ScanLineFilter {
   public:
      Select1ScanLineFilter( F func ) : func_( func ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) { return 4; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dfloat const* in1 = static_cast< dfloat const* >( params.inBuffer[ 0 ].buffer );
         dfloat const* in2 = static_cast< dfloat const* >( params.inBuffer[ 1 ].buffer );
         TPI const* in3 = static_cast< TPI const* >( params.inBuffer[ 2 ].buffer );
         TPI const* in4 = static_cast< TPI const* >( params.inBuffer[ 3 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const in1Stride = params.inBuffer[ 0 ].stride;
         dip::sint const in2Stride = params.inBuffer[ 1 ].stride;
         dip::sint const in3Stride = params.inBuffer[ 2 ].stride;
         dip::sint const in4Stride = params.inBuffer[ 3 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = func_( *in1, *in2 ) ? *in3 : *in4;
            in1 += in1Stride;
            in2 += in2Stride;
            in3 += in3Stride;
            in4 += in4Stride;
            out += outStride;
         }
      }
   private:
      F func_;
};

template< typename TPI, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewSelect1ScanLineFilter( F func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new Select1ScanLineFilter< TPI, F >( func ));
}

} // namespace

void Select(
      Image const& in1,
      Image const& in2,
      Image const& in3,
      Image const& in4,
      Image& out,
      String const& selector
) {
   DIP_THROW_IF( in1.DataType().IsComplex() || in2.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType dataType = dip::DataType::SuggestDyadicOperation( in3.DataType(), in4.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   if( selector == "==" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 == in2; } ), dataType );
   } else if( selector == "!=" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 != in2; } ), dataType );
   } else if( selector == ">" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 > in2; } ), dataType );
   } else if( selector == "<" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 < in2; } ), dataType );
   } else if( selector == ">=" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 >= in2; } ), dataType );
   } else if( selector == "<=" ) {
      DIP_OVL_CALL_ASSIGN_ALL( lineFilter, NewSelect1ScanLineFilter, ( []( dfloat in1, dfloat in2 ) { return in1 <= in2; } ), dataType );
   } else {
      DIP_THROW( "Illegal selector string" );
   }
   ImageConstRefArray inar{ in1, in2, in3, in4 };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ DT_DFLOAT, DT_DFLOAT, dataType, dataType };
   Framework::Scan( inar, outar, inBufT, { dataType }, { dataType }, { 0 }, *lineFilter, Framework::Scan_TensorAsSpatialDim );
}


namespace {

template< typename TPI >
class Select2ScanLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in1 = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* in2 = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         bin const* mask = static_cast< bin const* >( params.inBuffer[ 2 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const in1Stride = params.inBuffer[ 0 ].stride;
         dip::sint const in2Stride = params.inBuffer[ 1 ].stride;
         dip::sint const maskStride = params.inBuffer[ 2 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = *mask ? *in1 : *in2;
            in1 += in1Stride;
            in2 += in2Stride;
            mask += maskStride;
            out += outStride;
         }
      }
};

} // namespace

void Select(
      Image const& in1,
      Image const& in2,
      Image const& mask,
      Image& out
) {
   DIP_START_STACK_TRACE
      UnsignedArray size = in1.Sizes();
      Framework::SingletonExpandedSize( size, in2.Sizes() );
      mask.CheckIsMask( size, Option::AllowSingletonExpansion::DO_ALLOW );
   DIP_END_STACK_TRACE
   DataType dataType = dip::DataType::SuggestDyadicOperation( in1.DataType(), in2.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, Select2ScanLineFilter, (), dataType );
   ImageConstRefArray inar{ in1, in2, mask };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ dataType, dataType, DT_BIN };
   Framework::Scan( inar, outar, inBufT, { dataType }, { dataType }, { 0 }, *lineFilter, Framework::Scan_TensorAsSpatialDim );
}


} // namespace dip
