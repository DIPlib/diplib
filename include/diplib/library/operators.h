/*
 * (c)2016-2021, Cris Luengo.
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
// IWYU pragma: private, include "diplib.h"


#ifndef DIP_OPERATORS_H
#define DIP_OPERATORS_H

#include <type_traits>

#include "diplib/library/export.h"
#include "diplib/library/datatype.h"
#include "diplib/library/image.h"


/// \file
/// \brief Declares the overloaded arithmetic, logical and comparison operators for \ref dip::Image.
/// This file is always included through \ref "diplib.h".
/// See \ref math_arithmetic, \ref math_comparison.

namespace dip {

namespace detail {

template< typename T, typename U >
using isa = std::is_same< std::remove_cv_t< std::remove_reference_t< T >>, U >;

template< typename T >
using isImage = isa< T, Image >;

template< typename T >
using isView = isa< T, Image::View >;

}

template< typename T >
using EnableIfNotImageOrView = std::enable_if_t< !detail::isImage< T >::value && !detail::isView< T >::value >;

template< typename T1, typename T2 >
using EnableIfOneIsImageOrView = std::enable_if_t< ( detail::isImage< T1 >::value || detail::isView< T1 >::value ) ||
                                                   ( detail::isImage< T2 >::value || detail::isView< T2 >::value ) >;

#define DIP_DEFINE_ARITHMETIC_OVERLOADS( name ) \
inline     void name( Image const& lhs, Image const& rhs, Image& out ) { name( lhs, rhs, out, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )); } \
DIP_NODISCARD inline Image name( Image const& lhs, Image const& rhs, DataType dt ) { Image out; name( lhs, rhs, out, dt ); return out; } \
DIP_NODISCARD inline Image name( Image const& lhs, Image const& rhs )              { Image out; name( lhs, rhs, out ); return out; } \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( Image const& lhs, T const& rhs, Image& out, DataType dt ) { name( lhs, Image{ rhs }, out, dt ); } \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( Image const& lhs, T const& rhs, Image& out )              { name( lhs, Image{ rhs }, out ); } \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( T const& lhs, Image const& rhs, Image& out, DataType dt ) { name( Image{ lhs }, rhs, out, dt ); } \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( T const& lhs, Image const& rhs, Image& out )              { name( Image{ lhs }, rhs, out ); } \
template< typename T1, typename T2 > DIP_NODISCARD inline Image name( T1&& lhs, T2&& rhs, DataType dt ) { Image out; name( std::forward< T1 >( lhs ), std::forward< T2 >( rhs ), out, dt ); return out; } \
template< typename T1, typename T2 > DIP_NODISCARD inline Image name( T1&& lhs, T2&& rhs )              { Image out; name( std::forward< T1 >( lhs ), std::forward< T2 >( rhs ), out ); return out; }

#define DIP_DEFINE_DYADIC_OVERLOADS( name ) \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( Image const& lhs, T const& rhs, Image& out ) { name( lhs, Image{ rhs }, out ); } \
template< typename T1, typename T2 > DIP_NODISCARD inline Image name( T1&& lhs, T2&& rhs ) { Image out; name( std::forward< T1 >( lhs ), std::forward< T2 >( rhs ), out ); return out; }

#define DIP_DEFINE_TRIADIC_OVERLOADS( name ) \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( Image const& in, Image const& lhs, T const& rhs, Image& out ) { name( in, lhs, Image{ rhs }, out ); } \
template< typename T, typename = EnableIfNotImageOrView< T >> inline void  name( Image const& in, T const& lhs, Image const& rhs, Image& out ) { name( in, Image{ lhs }, rhs, out ); } \
template< typename T1, typename T2, typename = EnableIfNotImageOrView< T1 >, typename = EnableIfNotImageOrView< T2 >> inline void name( Image const& in, T1 const& lhs, T2 const& rhs, Image& out ) { name( in, Image{ lhs }, Image{ rhs }, out ); } \
template< typename T1, typename T2 > DIP_NODISCARD inline Image name( Image const& in, T1&& lhs, T2&& rhs ) { Image out; name( in, std::forward< T1 >( lhs ), std::forward< T2 >( rhs ), out ); return out; }


/// \addtogroup math_arithmetic


//
// Functions for arithmetic operations
//

/// \brief Adds two images, sample-wise, with singleton expansion, and using saturated arithmetic.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
/// Pixel values from both images will be cast to type `dt` before applying the operation.
///
/// For binary types, saturated addition is equivalent to the Boolean OR operation.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Subtract, Multiply, MultiplySampleWise, Divide, Modulo, Power, operator+(T1 const&, T2 const&), operator+=(Image&, T const&)
DIP_EXPORT void Add( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( Add )

/// \brief Subtracts two images, sample-wise, with singleton expansion, and using saturated arithmetic.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
/// Pixel values from both images will be cast to type `dt` before applying the operation.
///
/// For binary types, saturated subtraction is equivalent to the Boolean AND NOT operation.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Multiply, MultiplySampleWise, Divide, Modulo, Power, operator-(T1 const&, T2 const&), operator-=(Image&, T const&)
DIP_EXPORT void Subtract( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( Subtract )

/// \brief Multiplies two images, pixel-wise, with singleton expansion, and using saturated arithmetic.
///
/// Tensor dimensions
/// of the two images must have identical inner dimensions, and the output at
/// each pixel will be the matrix multiplication of the two input pixels.
///
/// To obtain a sample-wise multiplication, Use \ref dip::MultiplySampleWise instead.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// For binary types, saturated multiplication is equivalent to the Boolean AND operation.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Subtract, MultiplySampleWise, MultiplyConjugate, Divide, Modulo, Power, operator*(T1 const&, T2 const&), operator*=(Image&, T const&)
DIP_EXPORT void Multiply( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( Multiply )

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
DIP_EXPORT void MultiplySampleWise( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( MultiplySampleWise )

/// \brief Multiplies two images with complex conjugation, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// 'rhs' will be complex-conjugated before the multiplication. This requires that it is complex
/// and that `dt` is a complex type. Otherwise, \ref dip::MultiplySampleWise will be called instead.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Subtract, Multiply, Divide, Modulo, Power
DIP_EXPORT void MultiplyConjugate( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( MultiplyConjugate )

/// \brief Divides two images, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// For binary types, saturated division is equivalent to the Boolean OR NOT operation.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, SafeDivide, Modulo, Power, operator/(T1 const&, T2 const&), operator/=(Image&, T const&)
DIP_EXPORT void Divide( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( Divide )

/// \brief Divides two images, sample-wise, with singleton expansion. Tests for division by zero, filling in 0 instead.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// For binary images, this function calls \ref dip::Divide.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, Power, operator/(T1 const&, T2 const&), operator/=(Image&, T const&)
DIP_EXPORT void SafeDivide( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( SafeDivide )

/// \brief Computes the modulo of two images, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to `lhs.DataType()` if left out.
/// Works for all real types (i.e. not complex). For floating-point types, uses `std::fmod`.
/// Pixel values from both images will be cast to type `dt` (with saturation as usual) before applying the modulo
/// operation, which might yield surprising results if `lhs` is an integer type and `rhs` has a fractional component,
/// or if `rhs` saturates in the cast.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, SafeDivide, Power, operator%(T1 const&, T2 const&), operator%=(Image&, T const&)
DIP_EXPORT void Modulo( Image const& lhs, Image const& rhs, Image& out, DataType dt );
// We cannot use DIP_DEFINE_ARITHMETIC_OVERLOADS here because the default data type is computed differently:
inline     void Modulo( Image const& lhs, Image const& rhs, Image& out ) { Modulo( lhs, rhs, out, lhs.DataType() ); }
DIP_NODISCARD inline Image Modulo( Image const& lhs, Image const& rhs, DataType dt ) { Image out; Modulo( lhs, rhs, out, dt ); return out; }
DIP_NODISCARD inline Image Modulo( Image const& lhs, Image const& rhs )              { Image out; Modulo( lhs, rhs, out ); return out; }
template< typename T, typename = EnableIfNotImageOrView< T >> inline void Modulo( Image const& lhs, T const& rhs, Image& out, DataType dt ) { Modulo( lhs, Image{ rhs }, out, dt ); }
template< typename T, typename = EnableIfNotImageOrView< T >> inline void Modulo( Image const& lhs, T const& rhs, Image& out )              { Modulo( lhs, Image{ rhs }, out ); }
template< typename T, typename = EnableIfNotImageOrView< T >> inline void Modulo( T const& lhs, Image const& rhs, Image& out, DataType dt ) { Modulo( Image{ lhs }, rhs, out, dt ); }
template< typename T, typename = EnableIfNotImageOrView< T >> inline void Modulo( T const& lhs, Image const& rhs, Image& out )              { Modulo( Image{ lhs }, rhs, out ); }
template< typename T, typename = EnableIfNotImageOrView< T >> DIP_NODISCARD inline Image Modulo( Image const& lhs, T const& rhs, DataType dt ) { return Modulo( lhs, Image{ rhs }, dt ); }
template< typename T, typename = EnableIfNotImageOrView< T >> DIP_NODISCARD inline Image Modulo( Image const& lhs, T const& rhs )              { return Modulo( lhs, Image{ rhs } ); }
template< typename T, typename = EnableIfNotImageOrView< T >> DIP_NODISCARD inline Image Modulo( T const& lhs, Image const& rhs, DataType dt ) { return Modulo( Image{ lhs }, rhs, dt ); }
template< typename T, typename = EnableIfNotImageOrView< T >> DIP_NODISCARD inline Image Modulo( T const& lhs, Image const& rhs )              { return Modulo( Image{ lhs }, rhs ); }

/// \brief Elevates `lhs` to the power of `rhs`, sample-wise, with singleton expansion.
///
/// The image `out` will have the type `dt`, which defaults to
/// `dip::DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() )` if left out.
///
/// Either `lhs` or `rhs` can be a scalar value of any of the supported pixel types, as long as at
/// least one input is an image.
///
/// \see Subtract, Multiply, MultiplySampleWise, Divide, Modulo
DIP_EXPORT void Power( Image const& lhs, Image const& rhs, Image& out, DataType dt );
DIP_DEFINE_ARITHMETIC_OVERLOADS( Power )

/// \brief Inverts each sample of the input image, yielding an image of the same type.
///
/// For unsigned images, the output is `std::numeric_limits::max() - in`. For
/// signed and complex types, it is `0 - in`. For binary images it is the logical NOT.
///
/// \see operator-(Image const&), operator!(Image const&), Not
DIP_EXPORT void Invert( Image const& in, Image& out );
DIP_NODISCARD inline Image Invert( Image const& in ) { Image out; Invert( in, out ); return out; }


//
// Functions for bit-wise operations
//

/// \brief Bit-wise AND of two integer images, or logical AND of two binary images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Or, Xor, operator&(Image const&, T const&)
DIP_EXPORT void And( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( And )

/// \brief Bit-wise OR of two integer images, or logical OR of two binary images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see And, Xor, operator|(Image const&, T const&)
DIP_EXPORT void Or( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( Or )

/// \brief Bit-wise XOR of two integer images, or logical XOR of two binary images, sample-wise, with singleton expansion.
///
/// XOR is "exclusive or".
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see And, Or, operator^(Image const&, T const&)
DIP_EXPORT void Xor( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( Xor )

/// \brief Bit-wise NOT of an integer image, or logical NOT of a binary image, sample-wise.
///
/// Out will have the type of `in`.
///
/// For binary images, this function calls \ref dip::Invert.
///
/// \see operator~(Image const&), Invert
DIP_EXPORT void Not( Image const& in, Image& out );
DIP_NODISCARD inline Image Not( Image const& in ) { Image out; Not( in, out ); return out; }


//
// Arithmetic operator overloads
//

/// \brief Arithmetic operator, calls \ref dip::Add.
template< typename T1, typename T2, typename = EnableIfOneIsImageOrView< T1, T2 >>
DIP_NODISCARD inline Image operator+( T1 const& lhs, T2 const& rhs ) {
   return Add( lhs, rhs );
}

/// \brief Arithmetic operator, calls \ref dip::Subtract.
template< typename T1, typename T2, typename = EnableIfOneIsImageOrView< T1, T2 >>
DIP_NODISCARD inline Image operator-( T1 const& lhs, T2 const& rhs ) {
   return Subtract( lhs, rhs );
}

/// \brief Arithmetic operator, calls \ref dip::Multiply.
template< typename T1, typename T2, typename = EnableIfOneIsImageOrView< T1, T2 >>
DIP_NODISCARD inline Image operator*( T1 const& lhs, T2 const& rhs ) {
   return Multiply( lhs, rhs );
}

/// \brief Arithmetic operator, calls \ref dip::Divide.
template< typename T1, typename T2, typename = EnableIfOneIsImageOrView< T1, T2 >>
DIP_NODISCARD inline Image operator/( T1 const& lhs, T2 const& rhs ) {
   return Divide( lhs, rhs );
}

/// \brief Arithmetic operator, calls \ref dip::Modulo.
template< typename T1, typename T2, typename = EnableIfOneIsImageOrView< T1, T2 >>
DIP_NODISCARD inline Image operator%( T1 const& lhs, T2 const& rhs ) {
   return Modulo( lhs, rhs );
}

/// \brief Bit-wise and logical operator, calls \ref dip::And.
template< typename T >
DIP_NODISCARD inline Image operator&( Image const& lhs, T const& rhs ) {
   return And( lhs, rhs );
}

/// \brief Bit-wise and logical operator, calls \ref dip::Or.
template< typename T >
DIP_NODISCARD inline Image operator|( Image const& lhs, T const& rhs ) {
   return Or( lhs, rhs );
}

/// \brief Bit-wise and logical operator, calls \ref dip::Xor.
template< typename T >
DIP_NODISCARD inline Image operator^( Image const& lhs, T const& rhs ) {
   return Xor( lhs, rhs );
}

/// \brief Unary operator, converts binary image to `dip::DT_UINT` and leaves other images unchanged, does not copy data.
DIP_NODISCARD inline Image operator+( Image const& in ) {
   if (in.DataType().IsBinary()) {
      Image out = in;
      out.ReinterpretCastBinToUint8();
      return out;
   }
   return in;
}

/// \brief Unary operator, calls \ref dip::Invert.
DIP_NODISCARD inline Image operator-( Image const& in ) {
   return Invert( in );
}

/// \brief Bit-wise and logical unary operator, calls \ref dip::Not.
DIP_NODISCARD inline Image operator~( Image const& in ) {
   return Not( in );
}

/// \brief Logical unary operator. The input is converted to a binary image, then calls \ref dip::Invert.
DIP_NODISCARD inline Image operator!( Image const& in ) {
   if( in.DataType().IsBinary() ) {
      return Invert( in );
   }
   Image out = Convert( in, DT_BIN );
   Invert( out, out );
   return out;
}


//
// Compound assignment operator overload
//

/// \brief Compound assignment operator, calls \ref dip::Add.
///
/// Equivalent, but usually faster, than `lhs = lhs + rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator+=( Image& lhs, T const& rhs ) {
   Add( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator, calls \ref dip::Subtract.
///
/// Equivalent, but usually faster, than `lhs = lhs - rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator-=( Image& lhs, T const& rhs ) {
   Subtract( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator, calls \ref dip::Multiply.
///
/// Equivalent, but usually faster, than `lhs = lhs * rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator*=( Image& lhs, T const& rhs ) {
   Multiply( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator, calls \ref dip::Divide.
///
/// Equivalent, but usually faster, than `lhs = lhs / rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator/=( Image& lhs, T const& rhs ) {
   Divide( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compound assignment operator, calls \ref dip::Modulo.
///
/// Equivalent, but usually faster, than `lhs = lhs % rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator%=( Image& lhs, T const& rhs ) {
   Modulo( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Bit-wise compound assignment operator, calls \ref dip::And.
///
/// Equivalent, but usually faster, than `lhs = lhs & rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator&=( Image& lhs, T const& rhs ) {
   And( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compound assignment operator, calls \ref dip::Or.
///
/// Equivalent, but usually faster, than `lhs = lhs | rhs`.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator|=( Image& lhs, T const& rhs ) {
   Or( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compound assignment operator, calls \ref dip::Xor.
///
/// Equivalent, but usually faster, than `lhs = lhs ^ rhs`.
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
//
// NOTE that we take the view as a const reference, such that r-values can be used as input.
//    We then cast the constness away. If we take a non-const reference, we can only use
//    l-values as input. For example,
//        Image::View tmp = image[0];
//        tmp += tmp;                // OK, we can take an l-value as non-const reference
//        image[0] += tmp;           // error, we cannot take an r-value as non-const reference
//    The second operation would work only with a const reference.
//    The operator still does the right thing, because we're referencing data in an image,
//    we don't care about the view. This is really a trick to get around C++ cleverness.
#define DIP_DEFINE_INPLACE_VIEW_OPERATOR( op ) \
template< typename T > \
inline Image::View& operator op( Image::View const& lhs, T const& rhs ) { \
   Image tmp = lhs; \
   tmp op rhs; \
   if( !tmp.IsShared() ) { const_cast< Image::View& >( lhs ).Copy( tmp ); } \
   return const_cast< Image::View& >( lhs ); }

DIP_DEFINE_INPLACE_VIEW_OPERATOR( += )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( -= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( *= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( /= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( %= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( &= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( |= )

DIP_DEFINE_INPLACE_VIEW_OPERATOR( ^= )

#undef DIP_DEFINE_INPLACE_VIEW_OPERATOR


/// \endgroup


/// \addtogroup math_comparison


//
// Functions for comparison
//

/// \brief Equality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see NotEqual, Lesser, Greater, NotGreater, NotLesser, operator==(Image const&, T const&)
DIP_EXPORT void Equal( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( Equal )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Equal, Lesser, Greater, NotGreater, NotLesser, operator!=(Image const&, T const&)
DIP_EXPORT void NotEqual( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( NotEqual )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Equal, NotEqual, Greater, NotGreater, NotLesser, operator<(Image const&, T const&)
DIP_EXPORT void Lesser( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( Lesser )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Equal, NotEqual, Lesser, NotGreater, NotLesser, operator>(Image const&, T const&)
DIP_EXPORT void Greater( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( Greater )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Equal, NotEqual, Lesser, Greater, NotLesser, operator<=(Image const&, T const&)
DIP_EXPORT void NotGreater( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( NotGreater )

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary. `lhs` must be an image, but `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
///
/// \see Equal, NotEqual, Lesser, Greater, NotGreater, operator>=(Image const&, T const&)
DIP_EXPORT void NotLesser( Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_DYADIC_OVERLOADS( NotLesser )

/// \brief In-range ternary comparison, sample-wise, with singleton expansion.
///
/// Computes
///
/// ```cpp
/// out = ( in >= lhs ) && ( in <= rhs );
/// ```
///
/// Out will be binary. `in` must be an image, but `lhs` and `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
DIP_EXPORT void InRange( Image const& in, Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_TRIADIC_OVERLOADS( InRange )

/// \brief Out-of-range ternary comparison, sample-wise, with singleton expansion.
///
/// Computes
///
/// ```cpp
/// out = ( in < lhs ) || ( in > rhs );
/// ```
///
/// Out will be binary. `in` must be an image, but `lhs` and `rhs` can also be a pixel or a sample
/// (or a scalar value that implicitly converts to one).
DIP_EXPORT void OutOfRange( Image const& in, Image const& lhs, Image const& rhs, Image& out );
DIP_DEFINE_TRIADIC_OVERLOADS( OutOfRange )


//
// Comparison operator overloads
//

/// \brief Comparison operator, calls \ref dip::Equal.
template< typename T >
DIP_NODISCARD inline Image operator==( Image const& lhs, T const& rhs ) {
   return Equal( lhs, rhs );
}

/// \brief Comparison operator, calls \ref dip::NotEqual.
template< typename T >
DIP_NODISCARD inline Image operator!=( Image const& lhs, T const& rhs ) {
   return NotEqual( lhs, rhs );
}

/// \brief Comparison operator, calls \ref dip::Lesser.
template< typename T >
DIP_NODISCARD inline Image operator<( Image const& lhs, T const& rhs ) {
   return Lesser( lhs, rhs );
}

/// \brief Comparison operator, calls \ref dip::Greater.
template< typename T >
DIP_NODISCARD inline Image operator>( Image const& lhs, T const& rhs ) {
   return Greater( lhs, rhs );
}

/// \brief Comparison operator, calls \ref dip::NotGreater.
template< typename T >
DIP_NODISCARD inline Image operator<=( Image const& lhs, T const& rhs ) {
   return NotGreater( lhs, rhs );
}

/// \brief Comparison operator, calls \ref dip::NotLesser.
template< typename T >
DIP_NODISCARD inline Image operator>=( Image const& lhs, T const& rhs ) {
   return NotLesser( lhs, rhs );
}


/// \endgroup


#undef DIP_DEFINE_ARITHMETIC_OVERLOADS
#undef DIP_DEFINE_DYADIC_OVERLOADS
#undef DIP_DEFINE_TRIADIC_OVERLOADS


} // namespace dip

#endif // DIP_OPERATORS_H
