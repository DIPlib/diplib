/*
 * DIPlib 3.0
 * This file contains the definition the arithmetic operators.
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

#include <array>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"

namespace dip {

//
void Add(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_add( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
void Subtract(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_sub( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
namespace {
template< typename TPI >
class MultiplyLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // This function is only called for two non-scalar images.
         DIP_ASSERT( params.inBuffer.size() == 2 );
         DIP_ASSERT( params.outBuffer.size() == 1 );
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint const rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::sint const lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint const rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint const outTensorStride = params.outBuffer[ 0 ].tensorStride;
         DIP_ASSERT( params.inBuffer[ 0 ].tensorLength == nRows * nInner );
         DIP_ASSERT( params.inBuffer[ 1 ].tensorLength == nInner * nColumns );
         DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == nRows * nColumns );
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            TPI const* rhsT = rhs;
            TPI* outT = out;
            for( dip::uint col = 0; col < nColumns; ++col ) {
               TPI const* lhsT = lhs;
               for( dip::uint row = 0; row < nRows; ++row ) {
                  TPI const* lhsTT = lhsT;
                  TPI const* rhsTT = rhsT;
                  TPI v = 0;
                  for( dip::uint jj = 0; jj < nInner; ++jj ) {
                     v = saturated_add( v, saturated_mul( *lhsTT, *rhsTT ));
                     lhsTT += nRows * lhsTensorStride;
                     rhsTT += rhsTensorStride;
                  }
                  *outT = v;
                  lhsT += lhsTensorStride;
                  outT += outTensorStride;
               }
               rhsT += nInner * rhsTensorStride;
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
      MultiplyLineFilter( dip::uint nRows, dip::uint nColumns, dip::uint nInner )
            : nRows( nRows ), nColumns( nColumns ), nInner( nInner ) {}
   private:
      dip::uint nRows;     // == lhs.TensorRows
      dip::uint nColumns;  // == rhs.TensorColumns
      dip::uint nInner;    // == lhs.TensorColumns == rhs.TensorRows
};
} // namespace

void Multiply(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   // TODO: If lhs == rhs.Transpose(), we should create a symmetric matrix! lhs.IsIdenticalView(rhs) && lhs.Tensor() == rhs.Tensor().Transpose()
   //       Special cases of these are the dot product and the inner product of vectors
   // TODO: Another special case could be for diagonal matrices, where each row/column of the other matrix is multiplied by the same value.
   if( lhs.IsScalar() || rhs.IsScalar() ) {
      MultiplySampleWise( lhs, rhs, out, dt );
      return;
   } else if( lhs.TensorColumns() != rhs.TensorRows() ) {
      DIP_THROW( "Inner tensor dimensions must match in multiplication" );
   }
   Tensor outTensor = Tensor( lhs.TensorRows(), rhs.TensorColumns() );
   ImageConstRefArray inar{ lhs, rhs };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ dt, dt };
   DataTypeArray outBufT{ dt };
   DataTypeArray outImT{ dt };
   UnsignedArray nElem{ outTensor.Elements() };
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, MultiplyLineFilter, ( lhs.TensorRows(), rhs.TensorColumns(), lhs.TensorColumns() ), dt );
   Framework::Scan( inar, outar, inBufT, outBufT, outImT, nElem, *scanLineFilter, Framework::Scan_ExpandTensorInBuffer );
   out.ReshapeTensor( outTensor );
}

void MultiplySampleWise(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_mul( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
void Divide(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_div( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
void Modulo(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   if( dt.IsFloat() ) {
      DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewDyadicScanLineFilter, (
            []( auto its ) { return std::fmod( *its[ 0 ], *its[ 1 ] ); }
      ), dt );
   } else {
      DIP_OVL_CALL_ASSIGN_INTEGER( scanLineFilter, Framework::NewDyadicScanLineFilter, (
            []( auto its ) { return *its[ 0 ] % *its[ 1 ]; }
      ), dt );
   }
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
void Power(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   dt = DataType::SuggestFlex( dt );
   DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return std::pow( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, *scanLineFilter );
}

//
template< typename TPI >
class dip__Invert : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const inStride = params.inBuffer[ 0 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            // Tensor dimension is 1 because we request `Scan_TensorAsSpatialDim`
            *out = saturated_inv( *in );
            in += inStride;
            out += outStride;
         }
      }
};

void Invert(
      Image const& in,
      Image& out
) {
   DataType dt = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_CALL_ASSIGN_ALL( scanLineFilter, Framework::NewMonadicScanLineFilter, (
         []( auto its ) { return saturated_inv( *its[ 0 ] ); }
   ), dt );
   Framework::ScanMonadic( in, out, dt, dt, 1, *scanLineFilter, Framework::Scan_TensorAsSpatialDim );
}


} // namespace dip
