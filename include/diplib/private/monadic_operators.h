/*
 * DIPlib 3.0
 * This file contains the declaration and definition of arithmetic, trigonometric and similar monadic operators.
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


// This file is meant to be included only by `include/diplib/math.h` and `src/math/monadic_operators.cpp`
// Do not include this file elsewhere! Things will break!

// Note that this file generates function declarations when included by `include/diplib/math.h`,
// and function definitions when included by `src/math/monadic_operators.cpp`.

#ifndef DIP__MONADIC_OPERATORS_PRIVATE
#define DIP__MONADIC_OPERATORS_PRIVATE

#if !defined(DIP_MATH_H)
#error Do not include the 'diplib/monadic_operators.private' file directly!
#endif


#ifndef DIP__MONADIC_OPERATOR_FLEX

#define DIP__MONADIC_OPERATOR_FLEX( functionName_, functionLambda_, inputDomain_, cost_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#define DIP__MONADIC_OPERATOR_FLOAT( functionName_, functionLambda_, inputDomain_, cost_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#define DIP__MONADIC_OPERATOR_FLOAT_WITH_PARAM( functionName_, paramType_, paramName_, functionLambda_, inputDomain_, cost_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out, paramType_ paramName_ ); \
   inline Image functionName_( Image const& in, paramType_ paramName_ ) { Image out; functionName_( in, out, paramName_ ); return out; }

#define DIP__MONADIC_OPERATOR_BIN( functionName_, functionLambda_, inputDomain_, defaultValue_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#endif

namespace dip {

/// \addtogroup math_arithmetic
/// \{

/// \brief Computes the nearest integer to each sample (rounds).
/// Only defined for floating-point types, the output is the same type.
DIP__MONADIC_OPERATOR_FLOAT( Round, []( auto its ) { return std::round( *its[ 0 ] ); }, DataType::Class_Float, 1 )

/// \brief Computes the smallest integer larger or equal to each sample (rounds up).
/// Only defined for floating-point types, the output is the same type.
DIP__MONADIC_OPERATOR_FLOAT( Ceil, []( auto its ) { return std::ceil( *its[ 0 ] ); }, DataType::Class_Float, 1 )

/// \brief Computes the largest integer smaller or equal to each sample (rounds down).
/// Only defined for floating-point types, the output is the same type.
DIP__MONADIC_OPERATOR_FLOAT( Floor, []( auto its ) { return std::floor( *its[ 0 ] ); }, DataType::Class_Float, 1 )

/// \brief Computes the truncated value of each sample (rounds towards zero).
/// Only defined for floating-point types, the output is the same type.
DIP__MONADIC_OPERATOR_FLOAT( Truncate, []( auto its ) { return std::trunc( *its[ 0 ] ); }, DataType::Class_Float, 1 )

/// \brief Computes the fractional value of each sample (`out = in - dip::Truncate(in)`).
/// Only defined for floating-point types, the output is the same type.
DIP__MONADIC_OPERATOR_FLOAT( Fraction, []( auto its ) { return dipm__Fraction( *its[ 0 ] ); }, DataType::Class_Float, 1 )

/// \brief Computes the reciprocal of each sample: out = in == 0 ? 0 : 1/in.
DIP__MONADIC_OPERATOR_FLEX( Reciprocal, []( auto its ) { return dipm__Reciprocal( *its[ 0 ] ); }, DataType::Class_NonBinary, 1 )

/// \brief Computes the square of each sample.
DIP__MONADIC_OPERATOR_FLEX( Square, []( auto its ) { return *its[ 0 ] * *its[ 0 ]; }, DataType::Class_NonBinary, 1 )

/// \brief Computes the square root of each sample.
DIP__MONADIC_OPERATOR_FLEX( Sqrt, []( auto its ) { return std::sqrt( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the base e exponent (natural exponential) of each sample.
DIP__MONADIC_OPERATOR_FLEX( Exp, []( auto its ) { return std::exp( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the base 2 exponent of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Exp2, []( auto its ) { return std::exp2( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the base 10 exponent of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Exp10, []( auto its ) { return std::pow( decltype( *its[ 0 ] )( 10 ), *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the natural logarithm (base e logarithm) of each sample.
DIP__MONADIC_OPERATOR_FLEX( Ln, []( auto its ) { return std::log( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the base 2 logarithm of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Log2, []( auto its ) { return std::log( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the base 10 logarithm of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Log10, []( auto its ) { return std::log10( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \}

/// \addtogroup math_trigonometric
/// \{

/// \brief Computes the sine of each sample.
DIP__MONADIC_OPERATOR_FLEX( Sin, []( auto its ) { return std::sin( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the cosine of each sample.
DIP__MONADIC_OPERATOR_FLEX( Cos, []( auto its ) { return std::cos( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the tangent of each sample.
DIP__MONADIC_OPERATOR_FLEX( Tan, []( auto its ) { return std::tan( *its[ 0 ] ); }, DataType::Class_NonBinary, 20 )

/// \brief Computes the arc sine of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Asin, []( auto its ) { return std::asin( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the arc cosine of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Acos, []( auto its ) { return std::acos( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the arc tangent of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Atan, []( auto its ) { return std::atan( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the hyperbolic sine of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Sinh, []( auto its ) { return std::sinh( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the hyperbolic cosine of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Cosh, []( auto its ) { return std::cosh( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the hyperbolic tangent of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Tanh, []( auto its ) { return std::tanh( *its[ 0 ] ); }, DataType::Class_Real, 20 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 0.
DIP__MONADIC_OPERATOR_FLOAT( BesselJ0, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJ0( *its[ 0 ] )); }, DataType::Class_Real, 100 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 1.
DIP__MONADIC_OPERATOR_FLOAT( BesselJ1, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJ1( *its[ 0 ] )); }, DataType::Class_Real, 100 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order `alpha`.
DIP__MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselJN, dip::uint, alpha, [ alpha ]( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselJN( *its[ 0 ], alpha )); }, DataType::Class_Real, 200 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 0.
DIP__MONADIC_OPERATOR_FLOAT( BesselY0, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselY0( *its[ 0 ] )); }, DataType::Class_Real, 100 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 1.
DIP__MONADIC_OPERATOR_FLOAT( BesselY1, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselY1( *its[ 0 ] )); }, DataType::Class_Real, 100 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order `alpha`.
DIP__MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselYN, dip::uint, alpha, [ alpha ]( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( BesselYN( *its[ 0 ], alpha )); }, DataType::Class_Real, 200 )

/// \brief Computes the natural logarithm of the gamma function of each sample.
DIP__MONADIC_OPERATOR_FLOAT( LnGamma, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::lgamma( *its[ 0 ] )); }, DataType::Class_Real, 100 )

/// \brief Computes the error function of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Erf, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::erf( *its[ 0 ] )); }, DataType::Class_Real, 50 )

/// \brief Computes the complementary error function of each sample.
DIP__MONADIC_OPERATOR_FLOAT( Erfc, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( std::erfc( *its[ 0 ] )); }, DataType::Class_Real, 50 )

/// \brief Computes the sinc function of each sample. \f$\mathrm{sinc}(x) = \sin(x)/x\f$.
DIP__MONADIC_OPERATOR_FLOAT( Sinc, []( auto its ) { return static_cast< decltype( *its[ 0 ] ) >( Sinc( *its[ 0 ] )); }, DataType::Class_Real, 22 )

/// \}

/// \addtogroup math_comparison
/// \{

/// \brief True for each pixel that is NaN.
DIP__MONADIC_OPERATOR_BIN( IsNotANumber, []( auto in ) { return dipm__IsNaN( in ); }, DataType::Class_Flex, false )

/// \brief True for each pixel that is positive or negative infinity.
DIP__MONADIC_OPERATOR_BIN( IsInfinite, []( auto in ) { return dipm__IsInf( in ); }, DataType::Class_Flex, false )

/// \brief True for each pixel that is not NaN nor infinity.
DIP__MONADIC_OPERATOR_BIN( IsFinite, []( auto in ) { return dipm__IsFinite( in ); }, DataType::Class_Flex, true )

/// \}

} // namespace dip

#undef DIP__MONADIC_OPERATOR_FLEX
#undef DIP__MONADIC_OPERATOR_FLOAT
#undef DIP__MONADIC_OPERATOR_FLOAT_WITH_PARAM
#undef DIP__MONADIC_OPERATOR_BIN

#endif // DIP__MONADIC_OPERATORS_PRIVATE
