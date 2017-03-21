/*
 * DIPlib 3.0
 * This file contains the definition the operators that work with tensor images.
 *
 * (c)2017, Cris Luengo.
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
#include <diplib/iterators.h>

#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI, typename TPO, typename F >
class TensorMonadicScanLineFilter : public Framework::ScanLineFilter {
   public:
      TensorMonadicScanLineFilter( F func ) : func_( func ) {}
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         ConstLineIterator< TPI > in(
               static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
               bufferLength,
               params.inBuffer[ 0 ].stride,
               params.outBuffer[ 0 ].tensorLength,
               params.inBuffer[ 0 ].tensorStride
         );
         LineIterator< TPO > out(
               static_cast< TPO* >( params.outBuffer[ 0 ].buffer ),
               bufferLength,
               params.outBuffer[ 0 ].stride,
               params.outBuffer[ 0 ].tensorLength,
               params.outBuffer[ 0 ].tensorStride
         );
         do {
            func_( in.begin(), out.begin() );
         } while( ++in, ++out );
      }
   private:
      F const& func_;
};

template< typename TPI, typename TPO, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewTensorMonadicScanLineFilter( F func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new TensorMonadicScanLineFilter< TPI, TPO, F >( func ));
}

} // namespace

void DotProduct( Image const& lhs, Image const& rhs, Image& out ) {
   DIP_THROW_IF( !lhs.IsForged() || !rhs.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !lhs.IsVector() || !rhs.IsVector(), "The dot product is only defined for vector images" );
   DIP_THROW_IF( lhs.TensorElements() != rhs.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   Image a = lhs.QuickCopy();
   a.ReshapeTensor( 1, a.TensorElements() );
   Image b = rhs.QuickCopy();
   b.ReshapeTensor( b.TensorElements(), 1 );
   Multiply( a, b, out, DataType::SuggestArithmetic( a.DataType(), b.DataType() ));
}

void CrossProduct( Image const& lhs, Image const& rhs, Image& out ) {
   DIP_THROW_IF( !lhs.IsForged() || !rhs.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( lhs.TensorElements() != rhs.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( !lhs.IsVector() || !rhs.IsVector(), "The cross product is only defined for 2D and 3D vector images" );
   Image a = lhs.QuickCopy();
   Image b = rhs.QuickCopy();
   DataType dtype = DataType::SuggestArithmetic( a.DataType(), b.DataType() );
   if( lhs.TensorElements() == 2 ) {
      // We could write `out = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ]` if we didn't care about potentially copying pixels to `out`.
      MultiplySampleWise( a[ 0 ], b[ 1 ], out, dtype );
      out -= a[ 1 ] * b[ 0 ];
   } else if( lhs.TensorElements() == 3 ) {
      UnsignedArray sizes = a.Sizes();
      Framework::SingletonExpandedSize( sizes, b.Sizes() );
      out.ReForge( sizes, 3, dtype, Option::AcceptDataTypeChange::DO_ALLOW );
      //
      Image tmp = out[ 0 ];
      tmp.Protect();
      MultiplySampleWise( a[ 1 ], b[ 2 ], tmp, dtype );
      tmp -= a[ 2 ] * b[ 1 ];
      //
      tmp = out[ 1 ];
      tmp.Protect();
      MultiplySampleWise( a[ 2 ], b[ 0 ], tmp, dtype );
      tmp -= a[ 0 ] * b[ 2 ];
      //
      tmp = out[ 2 ];
      tmp.Protect();
      MultiplySampleWise( a[ 0 ], b[ 1 ], tmp, dtype );
      tmp -= a[ 1 ] * b[ 0 ];
   } else {
      DIP_THROW( "The cross product is only defined for 2D and 3D vector images" );
   }
}

void Determinant( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.Tensor().IsSquare(), "The determinant can only be computed from square matrices" );
   dip::uint n = in.TensorElements();
   if( n == 1 ) {
      out.Copy( in );
   } else {
      DataType outtype = DataType::SuggestFlex( in.DataType() );
      DataType buffertype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
         if( outtype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { * pout = DeterminantDiagonal( n, pin ); }
            );
            buffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                  [ n ]( auto const& pin, auto const& pout ) { * pout = DeterminantDiagonal( n, pin ); }
            );
            buffertype = DT_DFLOAT;
         }
         Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter );
      } else {
         if( outtype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = Determinant( n, pin ); }
            );
            buffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = Determinant( n, pin ); }
            );
            buffertype = DT_DFLOAT;
         }
         Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter, Framework::Scan_ExpandTensorInBuffer );
      }
   }
}

void Norm( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsVector(), "Norm only defined for vector images" );
   dip::uint n = in.TensorElements();
   if( n == 1 ) {
      Abs( in, out );
   } else {
      DataType outtype = DataType::SuggestFloat( in.DataType() );
      DataType intype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if( in.DataType().IsComplex() ) {
         scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = Norm( n, pin ); }
         );
         intype = DT_DCOMPLEX;
      } else {
         scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = Norm( n, pin ); }
         );
         intype = DT_DFLOAT;
      }
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { intype }, { DT_DFLOAT }, { outtype }, { 1 }, *scanLineFilter );
   }
}

void Trace( Image const& in, Image& out );

void Rank( Image const& in, Image& out );

void EigenValues( Image const& in, Image& out );

void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors );

void Inverse( Image const& in, Image& out );

void PseudoInverse( Image const& in, Image& out );

void SingularValueDecomposition( Image const& A, Image& U, Image& S, Image& V );

void SingularValueDecomposition( Image const& in, Image& out );

} // namespace dip
