/*
 * (c)2017-2022, Cris Luengo.
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

#include "diplib/math.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI, typename TPO, typename F >
class TensorMonadicScanLineFilter : public Framework::ScanLineFilter {
   public:
      TensorMonadicScanLineFilter( F const& func, dip::uint cost ) : func_( func ), cost_( cost ) {}
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return cost_; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return cost_; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return cost_; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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

template< class T >
struct GreaterMagnitude {
   bool operator()( T const& a, T const& b ) const {
      return std::abs( a ) > std::abs( b );
   }
};

} // namespace


void SortTensorElements( Image& out ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   if( !out.IsScalar() ) {
      DataType outtype = out.DataType();
      dip::uint n = out.TensorElements();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, NewTensorMonadicScanLineFilter, (
            // Note that pin and pout will point to the same data:
            // `Scan` is called with the same image as input and output, and the buffer and output types
            // are all the same as the image's type, so no intermediate buffers will be made.
            [ n ]( auto const& /*pin*/, auto const& pout ) { std::sort( pout, pout + n, std::greater<>() ); },
            static_cast< dip::uint >( 2 * static_cast< dfloat >( n ) * std::log2( n ))
      ), outtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { out }, outar, { outtype }, { outtype }, { outtype }, { n }, *scanLineFilter ));
   }
}

void SortTensorElementsByMagnitude( Image& out ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   if( !out.IsScalar() ) {
      DataType outtype = out.DataType();
      dip::uint n = out.TensorElements();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            // Note that pin and pout will point to the same data:
            // `Scan` is called with the same image as input and output, and the buffer and output types
            // are all the same as the image's type, so no intermediate buffers will be made.
            [ n ]( auto const& /*pin*/, auto const& pout ) { std::sort( pout, pout + n, GreaterMagnitude< decltype( *pout ) >() ); },
            static_cast< dip::uint >( 2 * static_cast< dfloat >( n ) * std::log2( n ))
      ), outtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { out }, outar, { outtype }, { outtype }, { outtype }, { n }, *scanLineFilter ));
   }
}


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
      DIP_STACK_TRACE_THIS( b = Conjugate( b )); // If we use `Conjugate( b, b )`, it would write into `rhs`, which we don't want to do.
   }
   DIP_STACK_TRACE_THIS( Multiply( a, b, out ));
}

namespace {
template< typename TPI >
class CrossProductLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint tensorElements ) override {
         return tensorElements == 2 ? 2 : 6;
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
               DIP_THROW_ASSERTION( E::NOT_REACHABLE );
         }
      }
};
} // namespace

void CrossProduct( Image const& lhs, Image const& rhs, Image& out ) {
   DIP_THROW_IF( !lhs.IsForged() || !rhs.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( lhs.TensorElements() != rhs.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( !lhs.IsVector() || !rhs.IsVector(), E::TENSOR_NOT_2_OR_3 );
   DataType dtype = DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() );
   dip::uint nElem{};
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
   DIP_STACK_TRACE_THIS( Framework::Scan( inar, outar, { dtype, dtype }, { dtype }, { dtype }, { nElem }, *scanLineFilter ));
}

void Norm( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      DIP_STACK_TRACE_THIS( Abs( in, out ));
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { intype }, { DT_DFLOAT }, { outtype }, { 1 }, *scanLineFilter ));
   }
}

void SquareNorm( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      DIP_STACK_TRACE_THIS( SquareModulus( in, out ));
   } else {
      DIP_THROW_IF( !in.IsVector(), E::IMAGE_NOT_VECTOR );
      dip::uint n = in.TensorElements();
      DataType outtype = DataType::SuggestFloat( in.DataType() );
      DataType intype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if( in.DataType().IsComplex() ) {
         scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = SquareNorm( n, pin ); }, 2 * n
         );
         intype = DT_DCOMPLEX;
      } else {
         scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
               [ n ]( auto const& pin, auto const& pout ) { *pout = SquareNorm( n, pin ); }, 2 * n
         );
         intype = DT_DFLOAT;
      }
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { intype }, { DT_DFLOAT }, { outtype }, { 1 }, *scanLineFilter ));
   }
}

void Angle( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   if( n == 2 ) {
      DIP_STACK_TRACE_THIS( Atan2( in[ 1 ], in[ 0 ], out ));
      return;
   }
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
         []( auto const& pin, auto const& pout ) {
            dfloat norm = Norm( 3, pin );
            pout[ 0 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
            pout[ 1 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
         }, 26 + 50 + 21
   );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { 2 }, *scanLineFilter ));
}

void Orientation( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   dip::uint outTensorElem{};
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               *pout = pin[ 0 ] == 0 ? pi / 2.0 : std::atan( pin[ 1 ] / pin[ 0 ] );
            }, 21
      );
      outTensorElem = 1;
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               dfloat norm = Norm( 3, pin );
               pout[ 0 ] = pin[ 0 ] == 0 ? pi / 2.0 : std::atan( pin[ 1 ] / pin[ 0 ] );
               pout[ 1 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
            }, 26 + 21 + 21
      );
      outTensorElem = 2;
   }
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { outTensorElem }, *scanLineFilter ));
}

void CartesianToPolar( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               pout[ 0 ] = Norm( 2, pin );
               pout[ 1 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
            }, 24 + 50
      );
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               dfloat norm = Norm( 3, pin );
               pout[ 0 ] = norm;
               pout[ 1 ] = std::atan2( pin[ 1 ], pin[ 0 ] );
               pout[ 2 ] = norm == 0.0 ? pi / 2.0 : std::acos( pin[ 2 ] / norm );
            }, 26 + 50 + 21
      );
   }
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { n }, *scanLineFilter ));
}

void PolarToCartesian( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = in.TensorElements();
   DIP_THROW_IF( !in.IsVector() || ( n < 2 ) || ( n > 3 ), E::TENSOR_NOT_2_OR_3 );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType outtype = DataType::SuggestFloat( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   if( n == 2 ) {
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               pout[ 0 ] = pin[ 0 ] * std::cos( pin[ 1 ] );
               pout[ 1 ] = pin[ 0 ] * std::sin( pin[ 1 ] );
            }, 42
      );
   } else { // n == 3
      scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
            []( auto const& pin, auto const& pout ) {
               dfloat sintheta = std::sin( pin[ 2 ] );
               pout[ 0 ] = pin[ 0 ] * std::cos( pin[ 1 ] ) * sintheta;
               pout[ 1 ] = pin[ 0 ] * std::sin( pin[ 1 ] ) * sintheta;
               pout[ 2 ] = pin[ 0 ] * std::cos( pin[ 2 ] );
            }, 65
      );
   }
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { outtype }, { n }, *scanLineFilter ));
}

void Determinant( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.Tensor().IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX );
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
         DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter ));
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
         DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, buffertype, outtype, 1, *scanLineFilter,
                                                       Framework::ScanOption::ExpandTensorInBuffer ));
      }
   }
}

void Trace( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX );
   if( in.IsScalar() ) {
      out = in;
   } else {
      DIP_STACK_TRACE_THIS( SumTensorElements( in.Diagonal(), out ));
   }
}

void Rank( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
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
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { intype }, { DT_UINT8 }, { DT_UINT8 }, { 1 }, *scanLineFilter,
                                          Framework::ScanOption::ExpandTensorInBuffer ));
}

void Eigenvalues( Image const& in, Image& out, String const& method ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.Tensor().IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX );
   bool precise{};
   DIP_STACK_TRACE_THIS( precise = BooleanFromString( method, S::PRECISE, S::FAST ));
   Option::DecompositionMethod m = precise ? Option::DecompositionMethod::PRECISE : Option::DecompositionMethod::FAST;
   if( in.IsScalar() ) {
      out = in;
   } else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      DIP_STACK_TRACE_THIS( out.Copy( in.Diagonal() ));
      DIP_STACK_TRACE_THIS( SortTensorElementsByMagnitude( out ));
   } else {
      dip::uint n = in.TensorRows();
      DataType intype = in.DataType();
      DataType inbuffertype;
      DataType outbuffertype;
      DataType outtype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if(( in.TensorShape() == Tensor::Shape::SYMMETRIC_MATRIX ) && ( !intype.IsComplex() )) {
         dip::uint cost = precise ? 400 * n : 60 * n; // strange: it's much faster than EigenDecomposition, but parallelism is beneficial at same point.
         switch( n ) {
            case 2:
               scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                     [ m ]( auto const& pin, auto const& pout ) { SymmetricEigenDecomposition2( pin, pout, nullptr, m ); }, cost
               );
               break;
            case 3:
               scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                     [ m ]( auto const& pin, auto const& pout ) { SymmetricEigenDecomposition3( pin, pout, nullptr, m ); }, cost
               );
               break;
            default:
               scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
                     [ n ]( auto const& pin, auto const& pout ) { SymmetricEigenDecomposition( n, pin, pout ); }, 400 * n
               );
               break;
         }
         inbuffertype = outbuffertype = DT_DFLOAT;
         outtype = DataType::SuggestFlex( intype );
      } else {
         if( intype.IsComplex() ) {
            scanLineFilter = NewTensorMonadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout ) { EigenDecomposition( n, pin, pout ); }, 2 * 400 * n
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { inbuffertype }, { outbuffertype }, { outtype }, { n }, *scanLineFilter,
                                             Framework::ScanOption::ExpandTensorInBuffer ));
   }
}

namespace {

template< typename TPI, typename TPO >
class SelectEigenvalueLineFilter : public Framework::ScanLineFilter {
   using funcType = void ( * )( dip::uint, ConstSampleIterator< TPI >, SampleIterator< TPO >, SampleIterator< TPO > );
   public:
      SelectEigenvalueLineFilter( funcType function, dip::uint n, bool first ) : function_( function ), n_( n ), first_( first ) {}
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint tensorElements ) override {
         return ( typeid( TPI ) == typeid( dfloat ) ? 400 : 800 ) * tensorElements;
      }
      void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
               params.outBuffer[ 0 ].tensorLength, // == 1
               params.outBuffer[ 0 ].tensorStride
         );
         // Allocate buffer if it's not yet there.
         std::vector< TPO >& buf = buffers_[ params.thread ];
         if( buf.size() != n_ ) {
            buf.resize( n_ );
         }
         TPO* bufPtr = buf.data();
         if( !first_ ) {
            bufPtr += n_ - 1;
         }
         // Compute
         do {
            function_( n_, in.begin(), buf.data(), nullptr );
            *out = *bufPtr;
         } while( ++in, ++out );
      }
   private:
      funcType function_;
      dip::uint n_;
      bool first_;
      std::vector< std::vector< TPO >> buffers_; // one for each thread
};

template< typename TPI, typename TPO >
class SelectEigenvalueLineFilterN : public Framework::ScanLineFilter {
   using funcType = void ( * )( ConstSampleIterator< TPI >, SampleIterator< TPO >, SampleIterator< TPO >, Option::DecompositionMethod );
   public:
      SelectEigenvalueLineFilterN( funcType function, dip::uint n, Option::DecompositionMethod method, bool first ) :
         function_( function ), n_( n ), method_( method ), first_( first ) {}
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint tensorElements ) override {
         return (( method_ == Option::DecompositionMethod::PRECISE ) ? 400 : 60 ) * tensorElements * (( typeid( TPI ) == typeid( dfloat )) ? 1 : 2 );
         // NOTE: currently TPI is always dfloat here, but it could be dcomplex in the future (where the cost doubles).
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
               params.outBuffer[ 0 ].tensorLength, // == 1
               params.outBuffer[ 0 ].tensorStride
         );
         std::array< TPO, 3 > buf{}; // NOTE!!! `function_` is either `SymmetricEigenDecomposition2` or `SymmetricEigenDecomposition3`
         TPO* bufPtr = buf.data();
         if( !first_ ) {
            bufPtr += n_ - 1;
         }
         // Compute
         do {
            function_( in.begin(), buf.data(), nullptr, method_ );
            *out = *bufPtr;
         } while( ++in, ++out );
      }
   private:
      funcType function_;
      dip::uint n_;
      Option::DecompositionMethod method_;
      bool first_;
};

void SelectEigenvalue( Image const& in, Image& out, String const& method, bool first ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.Tensor().IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX );
   bool precise{};
   DIP_STACK_TRACE_THIS( precise = BooleanFromString( method, S::PRECISE, S::FAST ));
   Option::DecompositionMethod m = precise ? Option::DecompositionMethod::PRECISE : Option::DecompositionMethod::FAST;
   if( in.IsScalar() ) {
      out = in;
   } else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      DIP_STACK_TRACE_THIS( MaximumAbsTensorElement( in, out ));
   } else {
      dip::uint n = in.TensorRows();
      DataType intype = in.DataType();
      DataType inbuffertype;
      DataType outbuffertype;
      DataType outtype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if(( in.TensorShape() == Tensor::Shape::SYMMETRIC_MATRIX ) && ( !intype.IsComplex() )) {
         switch( n ) {
            case 2:
               scanLineFilter = static_cast< std::unique_ptr< Framework::ScanLineFilter >>(
                     new SelectEigenvalueLineFilterN< dfloat, dfloat >( &SymmetricEigenDecomposition2, 2, m, first ));
               break;
            case 3:
               scanLineFilter = static_cast< std::unique_ptr< Framework::ScanLineFilter >>(
                     new SelectEigenvalueLineFilterN< dfloat, dfloat >( &SymmetricEigenDecomposition3, 3, m, first ));
               break;
            default:
               scanLineFilter = static_cast< std::unique_ptr< Framework::ScanLineFilter >>(
                     new SelectEigenvalueLineFilter< dfloat, dfloat >( &SymmetricEigenDecomposition, n, first ));
               break;
         }
         inbuffertype = DT_DFLOAT;
         outbuffertype = DT_DFLOAT;
         outtype = DataType::SuggestFlex( intype );
      } else {
         if( intype.IsComplex() ) {
            scanLineFilter = static_cast< std::unique_ptr< Framework::ScanLineFilter >>(
                  new SelectEigenvalueLineFilter< dcomplex, dcomplex >( &EigenDecomposition, n, first ));
            inbuffertype = DT_DCOMPLEX;
         } else {
            scanLineFilter = static_cast< std::unique_ptr< Framework::ScanLineFilter >>(
                  new SelectEigenvalueLineFilter< dfloat, dcomplex >( &EigenDecomposition, n, first ));
            inbuffertype = DT_DFLOAT;
         }
         outbuffertype = DT_DCOMPLEX;
         outtype = DataType::SuggestComplex( intype );
      }
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { inbuffertype }, { outbuffertype }, { outtype }, { 1 }, *scanLineFilter,
                                             Framework::ScanOption::ExpandTensorInBuffer ));
   }
}

} // namespace

void LargestEigenvalue( Image const& in, Image& out, String const& method ) {
   SelectEigenvalue( in, out, method, true );
}

void SmallestEigenvalue( Image const& in, Image& out, String const& method ) {
   SelectEigenvalue( in, out, method, false );
}

void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors, String const& method ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.Tensor().IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX );
   bool precise{};
   DIP_STACK_TRACE_THIS( precise = BooleanFromString( method, S::PRECISE, S::FAST ));
   Option::DecompositionMethod m = precise ? Option::DecompositionMethod::PRECISE : Option::DecompositionMethod::FAST;
   if( in.IsScalar() ) {
      out = in;
      eigenvectors.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      eigenvectors.Fill( 1.0 );
   //} else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      //SortTensorElementsByMagnitude( in, out );
      //Identity( in, eigenvectors );
      // TODO: the `eigenvectors` have to be sorted in the same way as `out`.
   } else {
      dip::uint n = in.TensorRows();
      DataType intype = in.DataType();
      DataType inbuffertype;
      DataType outbuffertype;
      DataType outtype;
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      if(( in.TensorShape() == Tensor::Shape::SYMMETRIC_MATRIX ) && ( !intype.IsComplex() )) {
         dip::uint cost = precise ? 600 * n : 90 * n; // cost of decomposition???
         switch( n ) {
            case 2:
               scanLineFilter = NewTensorDyadicScanLineFilter< dfloat, dfloat >(
                     [ m ]( auto const& pin, auto const& pout1, auto const& pout2 ) { SymmetricEigenDecomposition2( pin, pout1, pout2, m ); }, cost
               );
               break;
            case 3:
               scanLineFilter = NewTensorDyadicScanLineFilter< dfloat, dfloat >(
                     [ m ]( auto const& pin, auto const& pout1, auto const& pout2 ) { SymmetricEigenDecomposition3( pin, pout1, pout2, m ); }, cost
               );
               break;
            default:
               scanLineFilter = NewTensorDyadicScanLineFilter< dfloat, dfloat >(
                     [ n ]( auto const& pin, auto const& pout1, auto const& pout2 ) { SymmetricEigenDecomposition( n, pin, pout1, pout2 ); }, 600 * n // cost of decomposition???
               );
               break;
         }
         inbuffertype = outbuffertype = DT_DFLOAT;
         outtype = DataType::SuggestFlex( intype );
      } else {
         if( intype.IsComplex() ) {
            scanLineFilter = NewTensorDyadicScanLineFilter< dcomplex, dcomplex >(
                  [ n ]( auto const& pin, auto const& pout1, auto const& pout2 ) { EigenDecomposition( n, pin, pout1, pout2 ); }, 2 * 600 * n // cost of decomposition???
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { inbuffertype }, { outbuffertype, outbuffertype }, { outtype, outtype },
                                             { n, n * n }, *scanLineFilter, Framework::ScanOption::ExpandTensorInBuffer ));
      eigenvectors.ReshapeTensor( n, n );
      out.ReshapeTensorAsDiagonal();
   }
}

void LargestEigenvector( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.TensorShape() != Tensor::Shape::SYMMETRIC_MATRIX, E::IMAGE_NOT_SYMMETRIC_MATRIX );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint n = in.TensorRows();
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
         [ n ]( auto const& pin, auto const& pout ) { LargestEigenvector( n, pin, pout ); }, 600 * n // cost of decomposition???
   );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { dataType },
                                          { n }, *scanLineFilter, Framework::ScanOption::ExpandTensorInBuffer ));
}

void SmallestEigenvector( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( in.TensorShape() != Tensor::Shape::SYMMETRIC_MATRIX, E::IMAGE_NOT_SYMMETRIC_MATRIX );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint n = in.TensorRows();
   DataType dataType = DataType::SuggestFlex( in.DataType() );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   scanLineFilter = NewTensorMonadicScanLineFilter< dfloat, dfloat >(
         [ n ]( auto const& pin, auto const& pout ) { SmallestEigenvector( n, pin, pout ); }, 600 * n // cost of decomposition???
   );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_DFLOAT }, { DT_DFLOAT }, { dataType },
                                          { n }, *scanLineFilter, Framework::ScanOption::ExpandTensorInBuffer ));
}

void Inverse( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.Tensor().IsSquare(), E::IMAGE_NOT_SQUARE_MATRIX);
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   if(( in.IsScalar() ) || ( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX )) {
      DIP_STACK_TRACE_THIS( Divide( Image( 1, outtype ), in, out, outtype )); // computes 1/in for each of the diagonal elements
   } else {
      // TODO: Create a specialization for symmetric real-valued matrices, so that the output can be symmetric as well.
      dip::uint n = in.TensorRows();
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { n * n }, *scanLineFilter,
                                             Framework::ScanOption::ExpandTensorInBuffer ));
      out.ReshapeTensor( n, n );
   }
}

void PseudoInverse( Image const& in, Image& out, dfloat tolerance ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   if(( in.IsScalar() ) || ( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX )) {
      DIP_STACK_TRACE_THIS( Divide( Image( 1, outtype ), in, out, outtype )); // computes 1/in for each of the diagonal elements
   } else {
      dip::uint m = in.TensorRows();
      dip::uint n = in.TensorColumns();
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { n * m }, *scanLineFilter,
                                             Framework::ScanOption::ExpandTensorInBuffer ));
      out.ReshapeTensor( n, m );
   }
}

void SingularValues( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
   } else if( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) {
      if( in.DataType().IsComplex() ) {
         out = in.Diagonal();
      } else {
         if( &in == &out ) {
            out = in.Diagonal();
         } else {
            out.Copy( in.Diagonal() );
         }
         DIP_STACK_TRACE_THIS( SortTensorElements( out ));
      }
   } else {
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { buffertype }, { buffertype }, { outtype }, { p }, *scanLineFilter,
                                             Framework::ScanOption::ExpandTensorInBuffer ));
   }
}

void SingularValueDecomposition( Image const& in, Image& U, Image& out, Image& V ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType outtype = DataType::SuggestFlex( in.DataType() );
   if( in.IsScalar() && !outtype.IsComplex() ) {
      out = in;
      U.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      V.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      U.Fill( 1.0 );
      V.Fill( 1.0 );
   //} else if(( in.TensorShape() == Tensor::Shape::DIAGONAL_MATRIX ) && !outtype.IsComplex() ) {
      //if( &in == &out ) {
      //   out = in.Diagonal();
      //   U.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      //   V.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      //} else {
      //   U.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      //   V.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      //   out.Copy( in.Diagonal() );
      //}
      //SortTensorElements( out ); // U and V do not need any sorting.
      //U.Fill( 1.0 );
      //V.Fill( 1.0 );
      // TODO: the `eigenvectors` have to be sorted in the same way as `out`.
   } else {
      dip::uint m = in.TensorRows();
      dip::uint n = in.TensorColumns();
      dip::uint p = std::min( m, n );
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
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { buffertype }, { buffertype, buffertype, buffertype }, { outtype, outtype, outtype },
                                             { p, m * p, n * p }, *scanLineFilter, Framework::ScanOption::ExpandTensorInBuffer ));
      U.ReshapeTensor( m, p );
      V.ReshapeTensor( n, p );
      out.ReshapeTensorAsDiagonal();
   }
}

void SumTensorElements( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) { *pout = Sum( n, pin ); }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void ProductTensorElements( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) { *pout = Product( n, pin ); }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void AllTensorElements( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      DIP_STACK_TRACE_THIS( NotEqual( in, 0, out ));
   } else {
      dip::uint n = in.TensorElements();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter = NewTensorMonadicScanLineFilter< bin >(
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout &= pin[ ii ];
               }
            }, n
      );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_BIN }, { DT_BIN }, { DT_BIN }, { 1 }, *scanLineFilter ));
   }
}

void AnyTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      DIP_STACK_TRACE_THIS( NotEqual( in, 0, out ));
   } else {
      dip::uint n = in.TensorElements();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter = NewTensorMonadicScanLineFilter< bin >(
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout |= pin[ ii ];
               }
            }, n
      );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { DT_BIN }, { DT_BIN }, { DT_BIN }, { 1 }, *scanLineFilter ));
   }
}

void MaximumTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.DataType().IsBinary() ) {
      DIP_STACK_TRACE_THIS( AnyTensorElement( in, out ));
   } else if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout = std::max( *pout, pin[ ii ] );
               }
            }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void MaximumAbsTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.DataType().IsBinary() ) {
      DIP_STACK_TRACE_THIS( AnyTensorElement( in, out ));
   } else if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout = std::max( std::abs( *pout ), std::abs( pin[ ii ] ));
               }
            }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void MinimumTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.DataType().IsBinary() ) {
      DIP_STACK_TRACE_THIS( AllTensorElements( in, out ));
   } else if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout = std::min( *pout, pin[ ii ] );
               }
            }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void MinimumAbsTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.DataType().IsBinary() ) {
      DIP_STACK_TRACE_THIS( AllTensorElements( in, out ));
   } else if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) {
               *pout = *pin;
               for( dip::uint ii = 1; ii < n; ++ii ) {
                  *pout = std::min( std::abs( *pout ), std::abs( pin[ ii ] ));
               }
            }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void MeanTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) { *pout = Sum( n, pin ) / static_cast< FloatType< decltype( *pout ) >>( n ); }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

void GeometricMeanTensorElement( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( in.IsScalar() ) {
      out = in;
   } else {
      dip::uint n = in.TensorElements();
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewTensorMonadicScanLineFilter, (
            [ n ]( auto const& pin, auto const& pout ) { *pout = std::pow( Product( n, pin ), 1 / static_cast< FloatType< decltype( *pout ) >>( n )); }, n
      ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { dtype }, { dtype }, { 1 }, *scanLineFilter ));
   }
}

} // namespace dip
