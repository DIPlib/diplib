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
      TensorMonadicScanLineFilter( F const& func, dip::uint cost ) : func_( func ), cost_( cost ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) { return cost_; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         ConstLineIterator< TPI > in(
               static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
               bufferLength,
               params.inBuffer[ 0 ].stride,
               params.inBuffer[ 0 ].tensorLength,
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
      F func_;
      dip::uint cost_;
};

template< typename TPI, typename TPO = TPI, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewTensorMonadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new TensorMonadicScanLineFilter< TPI, TPO, F >( func, cost ));
}

template< typename TPI, typename TPO, typename F >
class TensorDyadicScanLineFilter : public Framework::ScanLineFilter {
   public:
      TensorDyadicScanLineFilter( F const& func, dip::uint cost ) : func_( func ), cost_( cost ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) { return cost_; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         ConstLineIterator< TPI > in(
               static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
               bufferLength,
               params.inBuffer[ 0 ].stride,
               params.inBuffer[ 0 ].tensorLength,
               params.inBuffer[ 0 ].tensorStride
         );
         LineIterator< TPO > out1(
               static_cast< TPO* >( params.outBuffer[ 0 ].buffer ),
               bufferLength,
               params.outBuffer[ 0 ].stride,
               params.outBuffer[ 0 ].tensorLength,
               params.outBuffer[ 0 ].tensorStride
         );
         LineIterator< TPO > out2(
               static_cast< TPO* >( params.outBuffer[ 1 ].buffer ),
               bufferLength,
               params.outBuffer[ 1 ].stride,
               params.outBuffer[ 1 ].tensorLength,
               params.outBuffer[ 1 ].tensorStride
         );
         do {
            func_( in.begin(), out1.begin(), out2.begin() );
         } while( ++in, ++out1, ++out2 );
      }
   private:
      F func_;
      dip::uint cost_;
};

template< typename TPI, typename TPO = TPI, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewTensorDyadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new TensorDyadicScanLineFilter< TPI, TPO, F >( func, cost ));
}

template< typename TPI, typename TPO, typename F >
class TensorTriadicScanLineFilter : public Framework::ScanLineFilter {
   public:
      TensorTriadicScanLineFilter( F const& func, dip::uint cost ) : func_( func ), cost_( cost ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) { return cost_; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         ConstLineIterator< TPI > in(
               static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
               bufferLength,
               params.inBuffer[ 0 ].stride,
               params.inBuffer[ 0 ].tensorLength,
               params.inBuffer[ 0 ].tensorStride
         );
         LineIterator< TPO > out1(
               static_cast< TPO* >( params.outBuffer[ 0 ].buffer ),
               bufferLength,
               params.outBuffer[ 0 ].stride,
               params.outBuffer[ 0 ].tensorLength,
               params.outBuffer[ 0 ].tensorStride
         );
         LineIterator< TPO > out2(
               static_cast< TPO* >( params.outBuffer[ 1 ].buffer ),
               bufferLength,
               params.outBuffer[ 1 ].stride,
               params.outBuffer[ 1 ].tensorLength,
               params.outBuffer[ 1 ].tensorStride
         );
         LineIterator< TPO > out3(
               static_cast< TPO* >( params.outBuffer[ 2 ].buffer ),
               bufferLength,
               params.outBuffer[ 2 ].stride,
               params.outBuffer[ 2 ].tensorLength,
               params.outBuffer[ 2 ].tensorStride
         );
         do {
            func_( in.begin(), out1.begin(), out2.begin(), out3.begin() );
         } while( ++in, ++out1, ++out2, ++out3 );
      }
   private:
      F func_;
      dip::uint cost_;
};

template< typename TPI, typename TPO = TPI, typename F >
std::unique_ptr< Framework::ScanLineFilter > NewTensorTriadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new TensorTriadicScanLineFilter< TPI, TPO, F >( func, cost ));
}


} // namespace

void DotProduct( Image const& lhs, Image const& rhs, Image& out ) {
   DIP_THROW_IF( !lhs.IsForged() || !rhs.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !lhs.IsVector() || !rhs.IsVector(), E::IMAGE_NOT_VECTOR );
   DIP_THROW_IF( lhs.TensorElements() != rhs.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   Image a = lhs.QuickCopy();
   a.SetPixelSize( lhs.PixelSize() ); // Copy pixel size so `Multiply` below can properly set the output pixel size.
   a.ReshapeTensor( 1, a.TensorElements() );
   Image b = rhs.QuickCopy();
   b.SetPixelSize( rhs.PixelSize() ); // Copy pixel size so `Multiply` below can properly set the output pixel size.
   b.ReshapeTensor( b.TensorElements(), 1 );
   // TODO: should use MultiplyConjugate, except that one does sample-wise multiplication at the moment
   if( b.DataType().IsComplex() ) {
      b = Conjugate( b ); // If we use `Conjugate( b, b )`, it would write into `rhs`, which we don't want to do.
   }
   Multiply( a, b, out );
}

//
namespace {
template< typename TPI >
class CrossProductLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint tensorElements ) {
         return tensorElements == 2 ? 2 : 6;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::uint const bufferLength = params.bufferLength;
         ConstLineIterator< TPI > lhs(
               static_cast< TPI const* >( params.inBuffer[ 0 ].buffer ),
               bufferLength,
               params.inBuffer[ 0 ].stride,
               params.inBuffer[ 0 ].tensorLength,
               params.inBuffer[ 0 ].tensorStride
         );
         ConstLineIterator< TPI > rhs(
               static_cast< TPI const* >( params.inBuffer[ 1 ].buffer ),
               bufferLength,
               params.inBuffer[ 1 ].stride,
               params.inBuffer[ 1 ].tensorLength,
               params.inBuffer[ 1 ].tensorStride
         );
         LineIterator< TPI > out(
               static_cast< TPI* >( params.outBuffer[ 0 ].buffer ),
               bufferLength,
               params.outBuffer[ 0 ].stride,
               params.outBuffer[ 0 ].tensorLength,
               params.outBuffer[ 0 ].tensorStride
         );
         DIP_ASSERT( params.inBuffer[ 0 ].tensorLength == params.inBuffer[ 1 ].tensorLength );
         switch( params.inBuffer[ 0 ].tensorLength ) {
            case 2:
            DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == 1 );
               do {
                  out[ 0 ] = lhs[ 0 ] * rhs[ 1 ] - lhs[ 1 ] * rhs[ 0 ];
               } while( ++lhs, ++rhs, ++out );
               break;
            case 3:
               DIP_ASSERT( params.outBuffer[ 0 ].tensorLength == 3 );
               do {
                  out[ 0 ] = lhs[ 1 ] * rhs[ 2 ] - lhs[ 2 ] * rhs[ 1 ];
                  out[ 1 ] = lhs[ 2 ] * rhs[ 0 ] - lhs[ 0 ] * rhs[ 2 ];
                  out[ 2 ] = lhs[ 0 ] * rhs[ 1 ] - lhs[ 1 ] * rhs[ 0 ];
               } while( ++lhs, ++rhs, ++out );
               break;
            default:
               DIP_THROW_ASSERTION( "This should not happen" );
         }
      }
};
} // namespace

void CrossProduct( Image const& lhs, Image const& rhs, Image& out ) {
   DIP_THROW_IF( !lhs.IsForged() || !rhs.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( lhs.TensorElements() != rhs.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( !lhs.IsVector() || !rhs.IsVector(), E::TENSOR_NOT_2_OR_3 );
   DataType dtype = DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() );
   dip::uint nElem;
   if( lhs.TensorElements() == 2 ) {
      nElem = 1;
   } else if( lhs.TensorElements() == 3 ) {
      nElem = 3;
   } else {
      DIP_THROW( E::TENSOR_NOT_2_OR_3 );
   }
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_FLEXBIN( scanLineFilter, CrossProductLineFilter, (), dtype );
   ImageConstRefArray inar{ lhs, rhs };
   ImageRefArray outar{ out };
   Framework::Scan( inar, outar, { dtype, dtype }, { dtype }, { dtype }, { nElem }, *scanLineFilter );
}

void Norm( Image const& in, Image& out ) {
   if( in.IsScalar() ) {
      Abs( in, out );
   } else {
      DIP_THROW_IF( !in.IsVector(), E::IMAGE_NOT_VECTOR );
      dip::uint n = in.TensorElements();
      DataType outtype = DataType::SuggestFloat( in.DataType() );
      DataType intype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if( in.DataType().IsComplex() ) {
         scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = Norm( n, pin ); }, 20 + 2 * n
         );
         intype = DT_DCOMPLEX;
      } else {
         scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = Norm( n, pin ); }, 20 + 2 * n
         );
         intype = DT_DFLOAT;
      }
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { intype }, { DT_DFLOAT }, { outtype }, { 1 }, *scanLineFilter );
   }
}

void Angle( Image const& in, Image& out ) {
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   dip::uint outTensorElem;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) { *pout = std::atan2( pin[ 1 ], pin[ 0 ] ); }, 50
      );
      outTensorElem = 1;
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               dfloat norm = Norm( 3, pin );
               pout[ 0 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
               pout[ 1 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
            }, 26 + 50 + 21
      );
      outTensorElem = 2;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { outTensorElem }, *scanLineFilter );
}

void Orientation( Image const& in, Image& out ) {
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   dip::uint outTensorElem;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = pin[ 0 ] == 0 ? 0 : std::atan( pin[ 1 ] / pin[ 0 ] );
            }, 21
      );
      outTensorElem = 1;
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               dfloat norm = Norm( 3, pin );
               pout[ 0 ] = pin[ 0 ] == 0 ? 0 : std::atan( pin[ 1 ] / pin[ 0 ] );
               pout[ 1 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
            }, 26 + 21 + 21
      );
      outTensorElem = 2;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { outTensorElem }, *scanLineFilter );
}

void CartesianToPolar( Image const& in, Image& out ) {
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               pout[ 0 ] = Norm( 2, pin );
               pout[ 1 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
            }, 24 + 50
      );
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               dfloat norm = Norm( 3, pin );
               pout[ 0 ] = norm;
               pout[ 1 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
               pout[ 2 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
            }, 26 + 50 + 21
      );
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { n }, *scanLineFilter );
}

void PolarToCartesian( Image const& in, Image& out ) {
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               pout[ 0 ] = pin[ 0 ] * std::cos( pin[ 1 ] );
               pout[ 1 ] = pin[ 0 ] * std::sin( pin[ 1 ] );
            }, 42
      );
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) {
               dfloat sintheta = std::sin( pin[ 2 ] );
               pout[ 0 ] = pin[ 0 ] * std::cos( pin[ 1 ] ) * sintheta;
               pout[ 1 ] = pin[ 0 ] * std::sin( pin[ 1 ] ) * sintheta;
               pout[ 2 ] = pin[ 0 ] * std::cos( pin[ 2 ] );
            }, 65
      );
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { n }, *scanLineFilter );
}

void Determinant( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.Tensor().IsSquare(), "The determinant can only be computed from square matrices" );
   dip::uint n = in.TensorRows();
   if( n == 1 ) {
      out = in;
   } else {
      DataType outtype = DataType::SuggestFlex( in.DataType() );
      DataType buffertype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
         if( outtype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = DeterminantDiagonal( n, pin ); }, n
            );
            buffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = DeterminantDiagonal( n, pin ); }, n
            );
            buffertype = DT_DFLOAT;
         }
         Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter );
      } else {
         if( outtype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = Determinant( n, pin ); }, n
            );
            buffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                  [ n ]( auto const& pin, auto const& pout ) { *pout = Determinant( n, pin ); }, n
            );
            buffertype = DT_DFLOAT;
         }
         Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter, Framework::Scan_ExpandTensorInBuffer );
      }
   }
}

void Trace( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsSquare(), "Trace only defined for square matrix images" );
   if( in.TensorElements() == 1 ) {
      out = in;
   } else {
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      dip::uint n = in.TensorRows();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      switch( in.TensorShape() ) {
         case Tensor::Shape::DIAGONAL_MATRIX:
         case Tensor::Shape::SYMMETRIC_MATRIX:
         case Tensor::Shape::UPPTRIANG_MATRIX:
         case Tensor::Shape::LOWTRIANG_MATRIX: {
            DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
                  [ n ]( auto const& pin, auto const& pout ) { *pout = TraceDiagonal( n, pin ); }, n
            ), dtype );
            break;
         }
         default: {
            DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
                  [ n ]( auto const& pin, auto const& pout ) { *pout = Trace( n, pin ); }, n
            ), dtype );
            break;
         }
      }
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter );
   }

}

void Rank( Image const& in, Image& out ) {
   dip::uint m = in.TensorRows();
   dip::uint n = in.TensorColumns();
   DataType intype;
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( in.DataType().IsComplex() ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, uint8 >(
            [ m, n ]( auto const& pin, auto const& pout ) { *pout = clamp_cast< uint8 >( Rank( m, n, pin )); }, 200 * n
      );
      intype = DT_DCOMPLEX;
   } else {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, uint8 >(
            [ m, n ]( auto const& pin, auto const& pout ) { *pout = clamp_cast< uint8 >( Rank( m, n, pin )); }, 100 * n
      );
      intype = DT_DFLOAT;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { intype }, { DT_UINT8 }, { DT_UINT8 }, { 1 }, *scanLineFilter,
                    Framework::Scan_ExpandTensorInBuffer );
}

void Eigenvalues( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.Tensor().IsSquare(), "The eigenvalues can only be computed from square matrices" );
   dip::uint n = in.TensorRows();
   if( n == 1 ) {
      out = in;
   //} else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      // out = in.Diagonal();
      // TODO: This will only work if we can sort the values.
   } else {
      DataType intype = in.DataType();
      DataType inbuffertype;
      DataType outbuffertype;
      DataType outtype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if(( in.TensorShape() == Tensor::Shape::SYMMETRIC_MATRIX ) && ( !intype.IsComplex() )) {
         scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { SymmetricEigenDecomposition( n, pin, pout ); }, 400 * n // strange: it's much faster than EigenDecomposition, but parallelism is beneficial at same point.
         );
         inbuffertype = outbuffertype = DT_DFLOAT;
         outtype = intype;
      } else {
         if( intype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { EigenDecomposition( n, pin, pout ); }, 800 * n
            );
            inbuffertype = outbuffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { EigenDecomposition( n, pin, pout ); }, 400 * n
            );
            inbuffertype = DT_DFLOAT;
            outbuffertype = DT_DCOMPLEX;
         }
         outtype = DataType::SuggestComplex( intype );
      }
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { inbuffertype }, { outbuffertype }, { outtype }, { n }, *scanLineFilter,
                       Framework::Scan_ExpandTensorInBuffer );
   }
}

void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors ) {
   DIP_THROW_IF( !in.Tensor().IsSquare(), "The eigenvalues can only be computed from square matrices" );
   dip::uint n = in.TensorRows();
   if( n == 1 ) {
      out = in;
      eigenvectors.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      eigenvectors.Fill( 1.0 );
   //} else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      // out = in.Diagonal();
      // Identity( in, eigenvectors );
      // TODO: This will only work if we can sort the values.
   } else {
      DataType intype = in.DataType();
      DataType inbuffertype;
      DataType outbuffertype;
      DataType outtype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if(( in.TensorShape() == Tensor::Shape::SYMMETRIC_MATRIX ) && ( !intype.IsComplex() )) {
         scanLineFilter = NewTensorDyadicScanLineFilter< dfloat, dfloat >(
               [ n ]( auto const& pin, auto const& pout1, auto const& pout2 ) { SymmetricEigenDecomposition( n, pin, pout1, pout2 ); }, 600 * n // cost of decomposition???
         );
         inbuffertype = outbuffertype = DT_DFLOAT;
         outtype = intype;
      } else {
         if( intype.IsComplex() ) {
            scanLineFilter = NewTensorDyadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout1, auto const& pout2 ) { EigenDecomposition( n, pin, pout1, pout2 ); }, 1200 * n // cost of decomposition???
            );
            inbuffertype = outbuffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = NewTensorDyadicScanLineFilter< dfloat, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout1, auto const& pout2 ) { EigenDecomposition( n, pin, pout1, pout2 ); }, 600 * n // cost of decomposition ???
            );
            inbuffertype = DT_DFLOAT;
            outbuffertype = DT_DCOMPLEX;
         }
         outtype = DataType::SuggestComplex( intype );
      }
      ImageRefArray outar{ out, eigenvectors };
      Framework::Scan( { in }, outar, { inbuffertype }, { outbuffertype, outbuffertype }, { outtype, outtype },
                       { n, n * n }, *scanLineFilter, Framework::Scan_ExpandTensorInBuffer );
      eigenvectors.ReshapeTensor( n, n );
      out.ReshapeTensorAsDiagonal();
   }
}

void Inverse( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.Tensor().IsSquare(), "The regular inverse can only be computed from square matrices" );
   dip::uint n = in.TensorRows();
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   DataType buffertype;
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( outtype.IsComplex() ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
            [ n ]( auto const& pin, auto const& pout ) { Inverse( n, pin, pout ); }, 800 * n
      );
      buffertype = DT_DCOMPLEX;
   } else {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ n ]( auto const& pin, auto const& pout ) { Inverse( n, pin, pout ); }, 400 * n
      );
      buffertype = DT_DFLOAT;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { n * n }, *scanLineFilter,
                    Framework::Scan_ExpandTensorInBuffer );
   out.ReshapeTensor( n, n );
}

void PseudoInverse( Image const& in, Image& out, dfloat tolerance ) {
   dip::uint m = in.TensorRows();
   dip::uint n = in.TensorColumns();
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   DataType buffertype;
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( outtype.IsComplex() ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
            [ m, n ]( auto const& pin, auto const& pout ) { PseudoInverse( m, n, pin, pout ); }, 800 * n
      );
      buffertype = DT_DCOMPLEX;
   } else {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ m, n, tolerance ]( auto const& pin, auto const& pout ) { PseudoInverse( m, n, pin, pout, tolerance ); }, 400 * n
      );
      buffertype = DT_DFLOAT;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { n * m }, *scanLineFilter,
                    Framework::Scan_ExpandTensorInBuffer );
   out.ReshapeTensor( n, m );
}

void SingularValues( Image const& in, Image& out ) {
   dip::uint m = in.TensorRows();
   dip::uint n = in.TensorColumns();
   dip::uint p = std::min( m, n );
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   DataType buffertype;
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( outtype.IsComplex() ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
            [ m, n ]( auto const& pin, auto const& pout ) { SingularValueDecomposition( m, n, pin, pout ); }, 800 * n
      );
      buffertype = DT_DCOMPLEX;
   } else {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            [ m, n ]( auto const& pin, auto const& pout ) { SingularValueDecomposition( m, n, pin, pout ); }, 400 * n
      );
      buffertype = DT_DFLOAT;
   }
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { p }, *scanLineFilter,
                    Framework::Scan_ExpandTensorInBuffer );
}

void SingularValueDecomposition( Image const& in, Image& U, Image& out, Image& V ) {
   dip::uint m = in.TensorRows();
   dip::uint n = in.TensorColumns();
   dip::uint p = std::min( m, n );
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   DataType buffertype;
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( outtype.IsComplex() ) {
      scanLineFilter = NewTensorTriadicScanLineFilter< dcomplex, dcomplex >(
            [ m, n ]( auto const& pin, auto const& pout1, auto const& pout2, auto const& pout3 ) { SingularValueDecomposition( m, n, pin, pout1, pout2, pout3 ); }, 1000 * n // cost of decomposition???
      );
      buffertype = DT_DCOMPLEX;
   } else {
      scanLineFilter = NewTensorTriadicScanLineFilter< dfloat, dfloat >(
            [ m, n ]( auto const& pin, auto const& pout1, auto const& pout2, auto const& pout3 ) { SingularValueDecomposition( m, n, pin, pout1, pout2, pout3 ); }, 500 * n // cost of decomposition???
      );
      buffertype = DT_DFLOAT;
   }
   ImageRefArray outar{ out, U, V };
   Framework::Scan( { in }, outar, { buffertype }, { buffertype, buffertype, buffertype }, { outtype, outtype, outtype },
                    { p, m * p, n * p }, *scanLineFilter, Framework::Scan_ExpandTensorInBuffer );
   U.ReshapeTensor( m, p );
   V.ReshapeTensor( n, p );
   out.ReshapeTensorAsDiagonal();
}


} // namespace dip
