/*
 * DIPlib 3.0
 * This file contains the declaration of the image arithmetic and logical operators.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_OPERATORS_H
#define DIP_OPERATORS_H

#include "diplib/library/image.h"


/// \file
/// \brief Declares the overloaded arithmetic, logical and comparison operators for `dip::Image`.
/// This file is always included through `diplib.h`.
/// \see math_arithmetic, math_comparison

namespace dip {


#define DIP__DEFINE_ARITHMETIC_OVERLOADS( name ) \
DIP_EXPORT void name( Image const& lhs, Image const& rhs, Image& out, DataType dt ); \
inline void name( Image const& lhs, Image const& rhs, Image& out ) { name( lhs, rhs, out, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )); } \
template< typename T > inline void name( Image const& lhs, T const& rhs, Image& out, DataType dt ) { name( lhs, Image{ rhs }, out, dt ); } \
template< typename T > inline void name( Image const& lhs, T const& rhs, Image& out ) { name( lhs, Image{ rhs }, out ); } \
template< typename T > inline Image name( Image const& lhs, T const& rhs, DataType dt ) { Image out; name( lhs, rhs, out, dt ); return out; } \
template< typename T > inline Image name( Image const& lhs, T const& rhs ) { Image out; name( lhs, rhs, out ); return out; }

#define DIP__DEFINE_DYADIC_OVERLOADS( name ) \
DIP_EXPORT void name( Image const& lhs, Image const& rhs, Image& out ); \
template< typename T > inline void name( Image const& lhs, T const& rhs, Image& out ) { name( lhs, Image{ rhs }, out ); } \
template< typename T > inline Image name( Image const& lhs, T const& rhs ) { Image out; name( lhs, rhs, out ); return out; }

#define DIP__DEFINE_TRIADIC_OVERLOADS( name ) \
DIP_EXPORT void name( Image const& in, Image const& lhs, Image const& rhs, Image& out ); \
template< typename T > inline void name( Image const& in, T const& lhs, T const& rhs, Image& out ) { name( in, Image{ lhs }, Image{ rhs }, out ); } \
template< typename T > inline Image name( Image const& in, T const& lhs, T const& rhs ) { Image out; name( in, lhs, rhs, out ); return out; }


/// \ingroup math_arithmetic
/// \{


//
// Functions for arithmetic operations
//

/// \brief Adds two images, sample-wise, with singleton expansion, and using saturated arithmetic.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// For binary types, saturated addition is equivalent to the Boolean OR operation.
///
/// \see Subtract, Multiply, MultiplySampleWise, Divide, Modulo, Power, operator+(Image const&, T const&)
DIP__DEFINE_ARITHMETIC_OVERLOADS( Add )

/// \brief Subtracts two images, sample-wise, with singleton expansion, and using saturated arithmetic.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// For binary types, saturated subtraction is equivalent to the Boolean AND NOT operation.
///
/// \see Add, Multiply, MultiplySampleWise, Divide, Modulo, Power, operator-(Image const&, T const&)
DIP__DEFINE_ARITHMETIC_OVERLOADS( Subtract )

/// \brief Multiplies two images, pixel-wise, with singleton expansion, and using saturated arithmetic.
///
/// %Tensor dimensions
/// of the two images must have identical inner dimensions, and the output at
/// each pixel will be the matrix multiplication of the two input pixels.
///
/// To obtain a sample-wise multiplication, Use `dip::MultiplySampleWise` instead.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// For binary types, saturated multiplication is equivalent to the Boolean AND operation.
///
/// \see Add, Subtract, MultiplySampleWise, MultiplyConjugate, Divide, Modulo, Power, operator*(Image const&, T const&)
DIP__DEFINE_ARITHMETIC_OVERLOADS( Multiply )

/// \brief Multiplies two images, sample-wise, with singleton expansion, and using saturated arithmetic.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// For binary types, saturated multiplication is equivalent to the Boolean AND operation.
///
/// \see Add, Subtract, Multiply, Divide, Modulo, Power
DIP__DEFINE_ARITHMETIC_OVERLOADS( MultiplySampleWise )

/// \brief Multiplies two images with complex conjugation, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// 'rhs' will be complex-conjugated before the multiplication. This requires that it is complex
/// and that `dt` is a complex type. Otherwise, `dip::MultiplySampleWise` will be called instead.
///
/// \see Add, Subtract, Multiply, Divide, Modulo, Power
DIP__DEFINE_ARITHMETIC_OVERLOADS( MultiplyConjugate )

/// \brief Divides two images, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// For binary types, saturated division is equivalent to the Boolean OR NOT operation.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Modulo, Power, operator/(Image const&, T const&)
DIP__DEFINE_ARITHMETIC_OVERLOADS( Divide )

/// \brief Computes the modulo of two images, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to `lhs.DataType()` if left out.
/// Works for all real types (i.e. not complex). For floating-point types, uses `std::fmod`.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Power, operator%(Image const&, T const&)
DIP_EXPORT void Modulo( Image const& lhs, Image const& rhs, Image& out, DataType dt ); \
inline void Modulo( Image const& lhs, Image const& rhs, Image& out ) { Modulo( lhs, rhs, out, lhs.DataType() ); } \
template< typename T > inline void Modulo( Image const& lhs, T const& rhs, Image& out, DataType dt ) { Modulo( lhs, Image{ rhs }, out, dt ); } \
template< typename T > inline void Modulo( Image const& lhs, T const& rhs, Image& out ) { Modulo( lhs, Image{ rhs }, out ); } \
template< typename T > inline Image Modulo( Image const& lhs, T const& rhs, DataType dt ) { Image out; Modulo( lhs, rhs, out, dt ); return out; } \
template< typename T > inline Image Modulo( Image const& lhs, T const& rhs ) { Image out; Modulo( lhs, rhs, out ); return out; }

/// \brief Elevates `lhs` to the power of `rhs`, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, as long as `dt` is a floating-point or complex type.
/// It defaults to `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// `rhs` can be a scalar value of any of the supported pixel types.
///
/// \see Subtract, Multiply, MultiplySampleWise, Divide, Modulo
DIP__DEFINE_ARITHMETIC_OVERLOADS( Power )

/// \brief Inverts each sample of the input image, yielding an image of the same type.
///
/// For unsigned images, the output is `std::numeric_limits::max() - in`. For
/// signed and complex types, it is `0 - in`. For binary types it is the same as `dip::Not`.
///
/// \see operator-(Image const&), Not
DIP_EXPORT void Invert( Image const& in, Image& out );
inline Image Invert( Image const& in ) { Image out; Invert( in, out ); return out; }


//
// Functions for bit-wise operations
//

/// \brief Bit-wise and of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see Or, Xor, operator&(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( And )

/// \brief Bit-wise or of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see And, Xor, operator|(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( Or )

/// \brief Bit-wise exclusive-or of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see And, Or, operator^(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( Xor )

/// \brief Applies bit-wise negation to each sample of the input image, yielding an
/// image of the same type.
///
/// \see operator!(Image const&), operator~(Image const&), Invert
DIP_EXPORT void Not( Image const& in, Image& out );
inline Image Not( Image const& in ) { Image out; Not( in, out ); return out; }


//
// Arithmetic operator overloads
//

/// \brief Arithmetic operator, calls `dip::Add`.
template< typename T >
inline Image operator+( Image const& lhs, T const& rhs ) {
   return Add( lhs, rhs );
}

/// \brief Arithmetic operator, calls `dip::Subtract`.
template< typename T >
inline Image operator-( Image const& lhs, T const& rhs ) {
   return Subtract( lhs, rhs );
}

/// \brief Arithmetic operator, calls `dip::Multiply`.
template< typename T >
inline Image operator*( Image const& lhs, T const& rhs ) {
   return Multiply( lhs, rhs );
}

/// \brief Arithmetic operator, calls `dip::Divide`.
template< typename T >
inline Image operator/( Image const& lhs, T const& rhs ) {
   return Divide( lhs, rhs );
}

/// \brief Arithmetic operator, calls `dip::Modulo`.
template< typename T >
inline Image operator%( Image const& lhs, T const& rhs ) {
   return Modulo( lhs, rhs );
}

/// \brief Bit-wise operator, calls `dip::And`.
template< typename T >
inline Image operator&( Image const& lhs, T const& rhs ) {
   return And( lhs, rhs );
}

/// \brief Bit-wise operator, calls `dip::Or`.
template< typename T >
inline Image operator|( Image const& lhs, T const& rhs ) {
   return Or( lhs, rhs );
}

/// \brief Bit-wise operator, calls `dip::Xor`.
template< typename T >
inline Image operator^( Image const& lhs, T const& rhs ) {
   return Xor( lhs, rhs );
}

/// \brief Unary operator, calls `dip::Invert`.
inline Image operator-( Image const& in ) {
   return Invert( in );
}

/// \brief Bit-wise unary operator, calls `dip::Not` for integer images.
inline Image operator~( Image const& in ) {
   DIP_THROW_IF( !in.DataType().IsInteger(), "Bit-wise unary not operator only applicable to integer images" );
   return Not( in );
}

/// \brief Boolean unary operator, calls `dip::Not` for binary images.
inline Image operator!( Image const& in ) {
   DIP_THROW_IF( !in.DataType().IsBinary(), "Boolean unary not operator only applicable to binary images" );
   return Not( in );
}


//
// Compound assignment operator overload
//

/// \brief Compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs + rhs`. See `dip::Add`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator+=( Image& lhs, T const& rhs ) {
   Add( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs - rhs`. See `dip::Subtract`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator-=( Image& lhs, T const& rhs ) {
   Subtract( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs * rhs`. See `dip::Multiply`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator*=( Image& lhs, T const& rhs ) {
   Multiply( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs / rhs`. See `dip::Divide`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator/=( Image& lhs, T const& rhs ) {
   Divide( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs % rhs`. See `dip::Modulo`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator%=( Image& lhs, T const& rhs ) {
   Modulo( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs & rhs`. See `dip::And`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator&=( Image& lhs, T const& rhs ) {
   And( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs | rhs`. See `dip::Or`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator|=( Image& lhs, T const& rhs ) {
   Or( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
///
/// Equivalent, but usually faster, than `lhs = lhs ^ rhs`. See `dip::Xor`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator^=( Image& lhs, T const& rhs ) {
   Xor( lhs, rhs, lhs );
   return lhs;
}

// Idem for views

/// \brief Compound assignment operator.
template< typename T >
inline Image::View& operator+=( Image::View& lhs, T const& rhs ) {
   Image tmp = lhs;
   lhs.Copy( Add( tmp, rhs, tmp.DataType() ));
   return lhs;
}

/// \brief Compound assignment operator.
template< typename T >
inline Image::View& operator-=( Image::View& lhs, T const& rhs ) {
   Image tmp = lhs;
   lhs.Copy( Subtract( tmp, rhs, tmp.DataType() ));
   return lhs;
}

/// \brief Compound assignment operator.
template< typename T >
inline Image::View& operator*=( Image::View& lhs, T const& rhs ) {
   Image tmp = lhs;
   lhs.Copy( Multiply( tmp, rhs, tmp.DataType() ));
   return lhs;
}

/// \brief Compound assignment operator.
template< typename T >
inline Image::View& operator/=( Image::View& lhs, T const& rhs ) {
   Image tmp = lhs;
   lhs.Copy( Divide( tmp, rhs, tmp.DataType() ));
   return lhs;
}

/// \brief Compound assignment operator.
template< typename T >
inline Image::View& operator%=( Image::View& lhs, T const& rhs ) {
   Image tmp = lhs;
   lhs.Copy( Modulo( tmp, rhs, tmp.DataType() ));
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
template< typename T >
inline Image::View& operator&=( Image::View& lhs, T const& rhs ) {
   lhs.Copy( And( lhs, rhs ));
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
template< typename T >
inline Image::View& operator|=( Image::View& lhs, T const& rhs ) {
   lhs.Copy( Or( lhs, rhs ));
   return lhs;
}

/// \brief Bit-wise compound assignment operator.
template< typename T >
inline Image::View& operator^=( Image::View& lhs, T const& rhs ) {
   lhs.Copy( Xor( lhs, rhs ));
   return lhs;
}


/// \}


/// \ingroup math_comparison
/// \{


//
// Functions for comparison
//

/// \brief Equality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see NotEqual, Lesser, Greater, NotGreater, NotLesser, operator==(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( Equal )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, Lesser, Greater, NotGreater, NotLesser, operator!=(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( NotEqual )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Greater, NotGreater, NotLesser, operator<(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( Lesser )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, NotGreater, NotLesser, operator>(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( Greater )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotLesser, operator<=(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( NotGreater )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotGreater, operator>=(Image const&, T const&)
DIP__DEFINE_DYADIC_OVERLOADS( NotLesser )

/// \brief In-range ternary comparison, sample-wise, with singleton expansion.
///
/// Computes
///
/// ```cpp
///     out = ( in >= lhs ) && ( in <= rhs );
/// ```
///
/// Out will be binary.
DIP__DEFINE_TRIADIC_OVERLOADS( InRange )

/// \brief Out-of-range ternary comparison, sample-wise, with singleton expansion.
///
/// Computes
///
/// ```cpp
///     out = ( in < lhs ) || ( in > rhs );
/// ```
///
/// Out will be binary.
DIP__DEFINE_TRIADIC_OVERLOADS( OutOfRange )


//
// Comparison operator overloads
//

/// \brief Comparison operator, calls `dip::Equal`.
template< typename T >
inline Image operator==( Image const& lhs, T const& rhs ) {
   return Equal( lhs, rhs );
}

/// \brief Comparison operator, calls `dip::NotEqual`.
template< typename T >
inline Image operator!=( Image const& lhs, T const& rhs ) {
   return NotEqual( lhs, rhs );
}

/// \brief Comparison operator, calls `dip::Lesser`.
template< typename T >
inline Image operator<( Image const& lhs, T const& rhs ) {
   return Lesser( lhs, rhs );
}

/// \brief Comparison operator, calls `dip::Greater`.
template< typename T >
inline Image operator>( Image const& lhs, T const& rhs ) {
   return Greater( lhs, rhs );
}

/// \brief Comparison operator, calls `dip::NotGreater`.
template< typename T >
inline Image operator<=( Image const& lhs, T const& rhs ) {
   return NotGreater( lhs, rhs );
}

/// \brief Comparison operator, calls `dip::NotLesser`.
template< typename T >
inline Image operator>=( Image const& lhs, T const& rhs ) {
   return NotLesser( lhs, rhs );
}


/// \}


#undef DIP__DEFINE_ARITHMETIC_OVERLOADS
#undef DIP__DEFINE_DYADIC_OVERLOADS
#undef DIP__DEFINE_TRIADIC_OVERLOADS


} // namespace dip

#endif // DIP_OPERATORS_H
