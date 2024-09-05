/*
 * (c)2017-2021, Cris Luengo.
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

#include "diplib/math.h"

#include <complex>
#include <memory>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

namespace dipm {

template< typename T >
T FlushToZero( T v ) { return !std::isnormal( v ) && std::isfinite( v ) ? T( 0 ) : v; }

template< typename T >
std::complex< T > FlushToZero( std::complex< T > v ) { return { FlushToZero( v.real()), FlushToZero( v.imag()) }; }

template< typename T >
T Fraction( T v ) { return v - std::trunc( v ); }

template< typename T >
T Reciprocal( T v ) { return v == T( 0 ) ? T( 0 ) : T( 1 ) / v; }

template< typename T >
bool IsNaN( T v ) { return std::isnan( v ); }
template< typename T >
bool IsNaN( std::complex< T > v ) { return std::isnan( v.real() ) || std::isnan( v.imag() ); }

template< typename T >
bool IsInf( T v ) { return std::isinf( v ); }
template< typename T >
bool IsInf( std::complex< T > v ) { return std::isinf( v.real() ) || std::isinf( v.imag() ); }

template< typename T >
bool IsFinite( T v ) { return std::isfinite( v ); }

template< typename T >
bool IsFinite( std::complex< T > v ) { return std::isfinite( v.real() ) || std::isfinite( v.imag() ); }

} // namespace dipm

template< typename TPI, typename F >
class BinScanLineFilter : public Framework::ScanLineFilter {
   public:
      BinScanLineFilter( F const& func ) : func_( func ) {}
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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

DIP_MONADIC_OPERATOR_FLEX( FlushToZero, []( auto its ) { return dipm::FlushToZero( *its[ 0 ] ); }, DataType::Class_Flex, 1 )
DIP_MONADIC_OPERATOR_FLOAT( Round, []( auto its ) { return std::round( *its[ 0 ] ); }, DataType::Class_Float, 1 )
DIP_MONADIC_OPERATOR_FLOAT( Ceil, []( auto its ) { return std::ceil( *its[ 0 ] ); }, DataType::Class_Float, 1 )
DIP_MONADIC_OPERATOR_FLOAT( Floor, []( auto its ) { return std::floor( *its[ 0 ] ); }, DataType::Class_Float, 1 )
DIP_MONADIC_OPERATOR_FLOAT( Truncate, []( auto its ) { return std::trunc( *its[ 0 ] ); }, DataType::Class_Float, 1 )
DIP_MONADIC_OPERATOR_FLOAT( Fraction, []( auto its ) { return dipm::Fraction( *its[ 0 ] ); }, DataType::Class_Float, 1 )
DIP_MONADIC_OPERATOR_FLEX( Reciprocal, []( auto its ) { return dipm::Reciprocal( *its[ 0 ] ); }, DataType::Class_NonBinary, 1 )
DIP_MONADIC_OPERATOR_FLEX( Square, []( auto its ) { return *its[ 0 ] * *its[ 0 ]; }, DataType::Class_NonBinary, 1 )
DIP_MONADIC_OPERATOR_FLEX( Sqrt, []( auto its ) { return std::sqrt( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLEX( Exp, []( auto its ) { return std::exp( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Exp2, []( auto its ) { return std::exp2( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Exp10, []( auto its ) { return std::pow( decltype( *its[ 0 ] )( 10 ), *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLEX( Ln, []( auto its ) { return std::log( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Log2, []( auto its ) { return std::log2( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Log10, []( auto its ) { return std::log10( *its[ 0 ] ); }, DataType::Class_Real, 20 )

DIP_MONADIC_OPERATOR_FLEX( Sin, []( auto its ) { return std::sin( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLEX( Cos, []( auto its ) { return std::cos( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLEX( Tan, []( auto its ) { return std::tan( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Asin, []( auto its ) { return std::asin( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Acos, []( auto its ) { return std::acos( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Atan, []( auto its ) { return std::atan( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Sinh, []( auto its ) { return std::sinh( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Cosh, []( auto its ) { return std::cosh( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( Tanh, []( auto its ) { return std::tanh( *its[ 0 ] ); }, DataType::Class_Real, 20 )
DIP_MONADIC_OPERATOR_FLOAT( BesselJ0, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJ0( *its[ 0 ] )); }, DataType::Class_Real, 100 )
DIP_MONADIC_OPERATOR_FLOAT( BesselJ1, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJ1( *its[ 0 ] )); }, DataType::Class_Real, 100 )
DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselJN, dip::uint, alpha, [ alpha ]( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJN( *its[ 0 ], alpha )); }, DataType::Class_Real, 200 )
DIP_MONADIC_OPERATOR_FLOAT( BesselY0, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselY0( *its[ 0 ] )); }, DataType::Class_Real, 100 )
DIP_MONADIC_OPERATOR_FLOAT( BesselY1, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselY1( *its[ 0 ] )); }, DataType::Class_Real, 100 )
DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselYN, dip::uint, alpha, [ alpha ]( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselYN( *its[ 0 ], alpha )); }, DataType::Class_Real, 200 )
DIP_MONADIC_OPERATOR_FLOAT( LnGamma, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::lgamma( *its[ 0 ] )); }, DataType::Class_Real, 100 )
DIP_MONADIC_OPERATOR_FLOAT( Erf, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::erf( *its[ 0 ] )); }, DataType::Class_Real, 50 )
DIP_MONADIC_OPERATOR_FLOAT( Erfc, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::erfc( *its[ 0 ] )); }, DataType::Class_Real, 50 )
DIP_MONADIC_OPERATOR_FLOAT( Sinc, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( Sinc( *its[ 0 ] )); }, DataType::Class_Real, 22 )

DIP_MONADIC_OPERATOR_BIN( IsNotANumber, []( auto in ) { return dipm::IsNaN( in ); }, DataType::Class_Flex, false )
DIP_MONADIC_OPERATOR_BIN( IsInfinite, []( auto in ) { return dipm::IsInf( in ); }, DataType::Class_Flex, false )
DIP_MONADIC_OPERATOR_BIN( IsFinite, []( auto in ) { return dipm::IsFinite( in ); }, DataType::Class_Flex, true )


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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return AbsCost< TPI >(); }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 20; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      Square( in, out );
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 2; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 3; }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
