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
                  FlexType< TPI > v = 0;
                  for( dip::uint jj = 0; jj < nInner; ++jj ) {
                     v += static_cast< FlexType< TPI >>( *lhsTT ) * static_cast< FlexType< TPI >>( *rhsTT );
                     lhsTT += static_cast< dip::sint >( nRows ) * lhsTensorStride;
                     rhsTT += rhsTensorStride;
                  }
                  *outT = clamp_cast< TPI >( v );
                  lhsT += lhsTensorStride;
                  outT += outTensorStride;
               }
               rhsT += static_cast< dip::sint >( nInner ) * rhsTensorStride;
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

template< typename TPI >
class MultiplySymmetricLineFilter : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // This function is only called for one non-scalar image.
         DIP_ASSERT( params.inBuffer.size() == 1 ); // RHS matrix, meaning the inner dimension is the columns
         DIP_ASSERT( params.outBuffer.size() == 1 );
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const inStride = params.inBuffer[ 0 ].stride;
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::sint const inTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint const outTensorStride = params.outBuffer[ 0 ].tensorStride;
         DIP_ASSERT( params.inBuffer[ 0 ].tensorLength == nOuter * nInner );
         DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == ( nOuter * ( nOuter + 1 )) / 2 );
         dip::uint const bufferLength = params.bufferLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            TPI const* inT = in;
            TPI* outT = out;
            // Compute diagonal elements first
            for( dip::uint col = 0; col < nOuter; ++col ) {
               FlexType< TPI > v = 0;
               for( dip::uint jj = 0; jj < nInner; ++jj ) {
                  v += static_cast< FlexType< TPI >>( *inT ) * static_cast< FlexType< TPI >>( *inT );
                  inT += inTensorStride;
               }
               *outT = clamp_cast< TPI >( v );
               outT += outTensorStride;
            }
            // Elements above diagonal are stored column-wise
            dip::sint colSkip = static_cast< dip::sint >( nInner ) * inTensorStride;
            TPI const* rhsT = in + colSkip;
            for( dip::uint col = 1; col < nOuter; ++col ) { // Elements above diagonal stored column-wise
               TPI const* lhsT = in;
               for( dip::uint row = 0; row < col; ++row ) {
                  TPI const* lhsTT = lhsT;
                  TPI const* rhsTT = rhsT;
                  FlexType< TPI > v = 0;
                  for( dip::uint jj = 0; jj < nInner; ++jj ) {
                     v += static_cast< FlexType< TPI >>( *lhsTT ) * static_cast< FlexType< TPI >>( *rhsTT );
                     lhsTT += inTensorStride;
                     rhsTT += inTensorStride;
                  }
                  *outT = clamp_cast< TPI >( v );
                  lhsT += colSkip;
                  outT += outTensorStride;
               }
               rhsT += colSkip;
            }
            in += inStride;
            out += outStride;
         }
      }
      MultiplySymmetricLineFilter( dip::uint nOuter, dip::uint nInner )
            : nOuter( nOuter ), nInner( nInner ) {}
   private:
      dip::uint nOuter;    // == lhs.TensorRows == rhs.TensorColumns
      dip::uint nInner;    // == lhs.TensorColumns == rhs.TensorRows
};

} // namespace

void Multiply(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   // TODO: Another special case could be for diagonal matrices, where each row/column of the other matrix is multiplied by the same value.
   if( lhs.IsScalar() || rhs.IsScalar() ) {
      MultiplySampleWise( lhs, rhs, out, dt );
      return;
   } else if( lhs.TensorColumns() != rhs.TensorRows() ) {
      DIP_THROW( "Inner tensor dimensions must match in multiplication" );
   }
   Tensor lhsTensorTransposed = lhs.Tensor();
   lhsTensorTransposed.Transpose();
   if(( lhsTensorTransposed == rhs.Tensor() ) && lhs.IsIdenticalView( rhs )) {
      // a' * a  or  a * a' : produces a symmetric matrix
      dip::uint nOuter = lhs.TensorRows();
      dip::uint nInner = lhs.TensorColumns();
      Tensor outTensor = Tensor( Tensor::Shape::SYMMETRIC_MATRIX, nOuter, nOuter );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_ALL( scanLineFilter, MultiplySymmetricLineFilter,
                       ( nOuter, nInner ), dt );
      ImageRefArray outar{ out };
      Framework::Scan( { rhs }, outar, { dt }, { dt }, { dt }, { outTensor.Elements() }, * scanLineFilter,
                       Framework::Scan_ExpandTensorInBuffer );
      out.ReshapeTensor( outTensor );
   } else {
      // General case
      Tensor outTensor = Tensor( lhs.TensorRows(), rhs.TensorColumns() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_ALL( scanLineFilter, MultiplyLineFilter,
                       ( lhs.TensorRows(), rhs.TensorColumns(), lhs.TensorColumns() ), dt );
      ImageRefArray outar{ out };
      Framework::Scan( { lhs, rhs }, outar, { dt, dt }, { dt }, { dt }, { outTensor.Elements() }, * scanLineFilter,
                       Framework::Scan_ExpandTensorInBuffer );
      out.ReshapeTensor( outTensor );
   }
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
            []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( *its[ 0 ] % *its[ 1 ] ); }
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

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the matrix multiplication operation") {
   try {
   dip::Image lhs( dip::Image::Pixel{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } );
   lhs.ReshapeTensor( 2, 3 );
   dip::Image rhs( dip::Image::Pixel{ 1.0, 10.0, 100.0 } );
   rhs.ReshapeTensorAsDiagonal();
   dip::Image out = lhs * rhs;
   DOCTEST_REQUIRE( out.TensorElements() == 6 );
   DOCTEST_CHECK( out.TensorShape() == dip::Tensor::Shape::COL_MAJOR_MATRIX );
   auto it = out.At( 0 );
   DOCTEST_CHECK( double( it[ 0 ] ) == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( double( it[ 1 ] ) == doctest::Approx( 2.0 ));
   DOCTEST_CHECK( double( it[ 2 ] ) == doctest::Approx( 30.0 ));
   DOCTEST_CHECK( double( it[ 3 ] ) == doctest::Approx( 40.0 ));
   DOCTEST_CHECK( double( it[ 4 ] ) == doctest::Approx( 500.0 ));
   DOCTEST_CHECK( double( it[ 5 ] ) == doctest::Approx( 600.0 ));
   DOCTEST_CHECK_THROWS( lhs * lhs );
   out = lhs * Transpose( lhs );
   DOCTEST_REQUIRE( out.TensorElements() == 3 );
   DOCTEST_CHECK( out.TensorShape() == dip::Tensor::Shape::SYMMETRIC_MATRIX );
   it = out.At( 0 );
   DOCTEST_CHECK( double( it[ 0 ] ) == doctest::Approx( 35.0 ));
   DOCTEST_CHECK( double( it[ 1 ] ) == doctest::Approx( 56.0 ));
   DOCTEST_CHECK( double( it[ 2 ] ) == doctest::Approx( 44.0 ));
   out = Transpose( lhs ) * lhs;
   DOCTEST_REQUIRE( out.TensorElements() == 6 );
   DOCTEST_CHECK( out.TensorShape() == dip::Tensor::Shape::SYMMETRIC_MATRIX );
   it = out.At( 0 );
   DOCTEST_CHECK( double( it[ 0 ] ) == doctest::Approx( 5.0 ));
   DOCTEST_CHECK( double( it[ 1 ] ) == doctest::Approx( 25.0 ));
   DOCTEST_CHECK( double( it[ 2 ] ) == doctest::Approx( 61.0 ));
   DOCTEST_CHECK( double( it[ 3 ] ) == doctest::Approx( 11.0 ));
   DOCTEST_CHECK( double( it[ 4 ] ) == doctest::Approx( 17.0 ));
   DOCTEST_CHECK( double( it[ 5 ] ) == doctest::Approx( 39.0 ));
   } catch ( dip::Error& e ){
      std::cout << e.what() << std::endl;
   }
}

#endif // DIP__ENABLE_DOCTEST
