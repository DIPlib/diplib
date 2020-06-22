/*
 * DIPlib 3.0
 * This file contains implements arithmetic, trigonometric and similar monadic operators.
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

#include <array>

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

namespace dipm {

template< typename T >
T Fraction( T v ) { return v - std::trunc( v ); }

template< typename T >
T Reciprocal( T v ) { return v == T( 0 ) ? T( 0 ) : T( 1 ) / v; }

template< typename T >
bool IsNaN( T v ) { return std::isnan( v ); }
template< typename T >
bool IsNaN( std::complex< T > v ) { return std::isnan( v.real()) || std::isnan( v.imag()); }

template< typename T >
bool IsInf( T v ) { return std::isinf( v ); }
template< typename T >
bool IsInf( std::complex< T > v ) { return std::isinf( v.real()) || std::isinf( v.imag()); }

template< typename T >
bool IsFinite( T v ) { return !dipm::IsNaN( v ) && !dipm::IsInf( v ); }

} // namespace dipm

template< typename TPI, typename F >
class BinScanLineFilter : public Framework::ScanLineFilter {
   public:
      BinScanLineFilter( F const& func ) : func_( func ) {}
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::uint const bufferLength = params.bufferLength;
         bin* out = static_cast< bin* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = func_( *in );
            in += inStride;
            out += outStride;
         }
      }
   private:
      F func_;
};

template< typename TPI, typename F >
inline std::unique_ptr< Framework::ScanLineFilter > NewBinScanLineFilter( F const& func ) {
   return static_cast< std::unique_ptr< Framework::ScanLineFilter >>( new BinScanLineFilter< TPI, F >( func ));
}


} // namespace

} // namespace dip

#define DIP_MONADIC_OPERATOR_FLEX( functionName_, functionLambda_, inputDomain_, cost_ ) \
   void functionName_( Image const& in, Image& out ) { \
      DIP_THROW_IF( !in.DataType().IsA( inputDomain_ ), E::DATA_TYPE_NOT_SUPPORTED ); \
      DataType dtype = DataType::SuggestFlex( in.DataType() ); \
      std::unique_ptr <Framework::ScanLineFilter> scanLineFilter; \
      DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, Framework::NewMonadicScanLineFilter, ( functionLambda_, cost_ ), dtype ); \
      DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter, \
            Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim )); \
   }

#define DIP_MONADIC_OPERATOR_FLOAT( functionName_, functionLambda_, inputDomain_, cost_ ) \
   void functionName_( Image const& in, Image& out ) { \
      DIP_THROW_IF( !in.DataType().IsA( inputDomain_ ), E::DATA_TYPE_NOT_SUPPORTED ); \
      DataType dtype = DataType::SuggestFloat( in.DataType() ); \
      std::unique_ptr <Framework::ScanLineFilter> scanLineFilter; \
      DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewMonadicScanLineFilter, ( functionLambda_, cost_ ), dtype ); \
      DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter, \
            Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim )); \
   }

#define DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( functionName_, paramType_, paramName_, functionLambda_, inputDomain_, cost_ ) \
   void functionName_( Image const& in, Image& out, paramType_ paramName_ ) { \
      DIP_THROW_IF( !in.DataType().IsA( inputDomain_ ), E::DATA_TYPE_NOT_SUPPORTED ); \
      DataType dtype = DataType::SuggestFloat( in.DataType() ); \
      std::unique_ptr <Framework::ScanLineFilter> scanLineFilter; \
      DIP_OVL_CALL_ASSIGN_FLOAT( scanLineFilter, Framework::NewMonadicScanLineFilter, ( functionLambda_, cost_ ), dtype ); \
      DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter, \
            Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim )); \
   }

#define DIP_MONADIC_OPERATOR_BIN( functionName_, functionLambda_, inputDomain_, defaultValue_ ) \
   void functionName_( Image const& in, Image& out ) { \
      DataType dtype = in.DataType(); \
      if( dtype.IsA( inputDomain_ )) { \
         std::unique_ptr <Framework::ScanLineFilter> scanLineFilter; \
         DIP_OVL_CALL_ASSIGN_FLEX( scanLineFilter, NewBinScanLineFilter, ( functionLambda_ ), dtype ); \
         ImageRefArray outar{ out }; \
         DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { DT_BIN }, { DT_BIN }, { 1 }, *scanLineFilter, \
               Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim )); \
      } else { \
         out.ReForge( in, DT_BIN ); \
         out.SetPixelSize( in.PixelSize() ); \
         out.SetColorSpace( in.ColorSpace() ); \
         out.Fill( defaultValue_ ); \
      } \
   }

#undef DIP_MONADIC_OPERATORS_PRIVATE
#include "diplib/private/monadic_operators.h"


namespace dip {


namespace {

template< typename T >
dip::uint AbsCost() { return 1; }
template<>
dip::uint AbsCost< scomplex >() { return 20; }
template<>
dip::uint AbsCost< dcomplex >() { return 20; }

template< typename TPI >
class AbsLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return AbsCost< TPI >(); }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;;
         dip::uint const bufferLength = params.bufferLength;
         AbsType< TPI >* out = static_cast< AbsType< TPI >* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = static_cast< AbsType< TPI >>( std::abs( *in ));
            in += inStride;
            out += outStride;
         }
      }
};

template< typename TPI >
class SquareModulusLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;;
         dip::uint const bufferLength = params.bufferLength;
         FloatType< TPI >* out = static_cast< FloatType< TPI >* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = in->real() * in->real() + in->imag() * in->imag();
            in += inStride;
            out += outStride;
         }
      }
};

template< typename TPI >
class PhaseLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 20; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;;
         dip::uint const bufferLength = params.bufferLength;
         FloatType< TPI >* out = static_cast< FloatType< TPI >* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = std::arg( *in );
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void Abs( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType dtype = in.DataType();
   if( dtype.IsSigned() ) {
      DataType otype = DataType::SuggestAbs( dtype );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_SIGNED( scanLineFilter, AbsLineFilter, (), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { otype }, { otype }, { 1 }, *scanLineFilter,
             Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
   } else {
      out = in;
   }
}

void SquareModulus( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType dtype = in.DataType();
   if( dtype.IsComplex() ) {
      DataType otype = DataType::SuggestFloat( dtype );
      std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
      DIP_OVL_NEW_COMPLEX( scanLineFilter, SquareModulusLineFilter, ( ), dtype );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { otype }, { otype }, { 1 }, *scanLineFilter,
            Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
   } else {
      MultiplySampleWise( in, in, out );
   }
}

void Phase( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType dtype = in.DataType();
   DataType otype = DataType::SuggestFloat( dtype );
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_COMPLEX( scanLineFilter, PhaseLineFilter, (), dtype );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { otype }, { otype }, { 1 }, *scanLineFilter,
          Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
}


void Conjugate( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DataType dtype = in.DataType();
   if( dtype.IsComplex() ) {
      std::unique_ptr <Framework::ScanLineFilter> scanLineFilter;
      DIP_OVL_CALL_ASSIGN_COMPLEX( scanLineFilter, Framework::NewMonadicScanLineFilter, (
            []( auto its ) { return std::conj( *its[ 0 ] ); }
      ), dtype );
      DIP_STACK_TRACE_THIS( Framework::ScanMonadic( in, out, dtype, dtype, in.TensorElements(), *scanLineFilter,
             Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
   } else {
      out = in;
   }
}


namespace {

template< typename TPI >
class SignLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 2; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;;
         dip::uint const bufferLength = params.bufferLength;
         sint8* out = static_cast< sint8* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = *in < 0 ? sint8( -1 ) : *in > 0 ? sint8( 1 ) : sint8( 0 );
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void Sign( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.DataType().IsA( DataType::Class_SInt + DataType::Class_Float ), E::DATA_TYPE_NOT_SUPPORTED );
   DataType dtype = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, SignLineFilter, (), dtype );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { DT_SINT8 }, { DT_SINT8 }, { 1 }, *scanLineFilter,
          Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
}


namespace {

template< typename TPI >
class NearestIntLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override { return 3; }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;;
         dip::uint const bufferLength = params.bufferLength;
         sint32* out = static_cast< sint32* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
            *out = clamp_cast< sint32 >( round_cast( *in ));
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void NearestInt( Image const& in, Image& out ) {
   DIP_THROW_IF( !in.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DataType dtype = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   DIP_OVL_NEW_FLOAT( scanLineFilter, NearestIntLineFilter, (), dtype );
   ImageRefArray outar{ out };
   DIP_STACK_TRACE_THIS( Framework::Scan( { in }, outar, { dtype }, { DT_SINT32 }, { DT_SINT32 }, { 1 }, *scanLineFilter,
          Framework::ScanOption::NoSingletonExpansion + Framework::ScanOption::TensorAsSpatialDim ));
}


} // namespace dip
