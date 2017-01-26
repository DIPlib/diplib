/*
 * DIPlib 3.0
 * This file contains the declaration of the image arithmetic and logical operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_OPERATORS_H
#define DIP_OPERATORS_H

#include "diplib/library/image.h"


/// \file
/// \brief Declares the overloaded arithmetic, logical and comparison operators for `dip::Image`,
/// as well as functions implementing their functionality.
/// This file is always included through `diplib.h`.


namespace dip {


/// \defgroup operators Arithmetic, logical and comparison operators
/// \brief Operators that work on a `dip::Image`, and the functions that implement their functionality.
/// \{


//
// Functions for arithmetic operations
//


/// \brief Adds two images, sample-wise, with singleton expansion.
///
/// Out will have the type `dt`.
///
/// \see Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator+
void Add(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// \brief Adds a constant to each sample in an image.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator+
template< typename T >
inline void Add(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Add( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Add(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Add( lhs, rhs, out, dt );
   return out;
}


/// \brief Subtracts two images, sample-wise, with singleton expansion.
///
/// Out will have the type `dt`.
///
/// \see Add, Multiply, MultiplySampleWise, Divide, Modulo, operator-
void Subtract(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// \brief Subtracts a constant from each sample in an image.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator-
template< typename T >
inline void Subtract(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Subtract( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Subtract(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Subtract( lhs, rhs, out, dt );
   return out;
}


/// \brief Multiplies two images, pixel-wise, with singleton expansion.
///
/// Tensor dimensions
/// of the two images must have identical inner dimensions, and the output at
/// each pixel will be the matrix multiplication of the two input pixels.
/// Out will have the type `dt`.
///
/// To obtain a sample-wise multiplication, convert the tensor dimension into
/// a spatial dimension using dip::Image::TensorToSpatial, and reshape the
/// output with dip::Image::SpatialToTensor. The helper function MulSamaples
/// does this.
///
/// \see Add, Subtract, MultiplySampleWise, Divide, Modulo, operator*
void Multiply(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// \brief Multiplies each sample in an image by a constant.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator*
template< typename T >
inline void Multiply(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Multiply( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Multiply(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Multiply( lhs, rhs, out, dt );
   return out;
}

/// \brief Adds two images, sample-wise, with singleton expansion.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, Divide, Modulo
void MultiplySampleWise(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

inline Image MultiplySampleWise(
      Image const& lhs,
      Image const& rhs,
      DataType dt
) {
   Image out;
   MultiplySampleWise( lhs, rhs, out, dt );
   return out;
}


/// \brief Divides two images, sample-wise, with singleton expansion.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Modulo, operator/
void Divide(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// \brief Divides each sample in an image by a constant.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator/
template< typename T >
inline void Divide(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Divide( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Divide(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Divide( lhs, rhs, out, dt );
   return out;
}


/// \brief Computes the modulo of two images, sample-wise, with singleton expansion.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, operator%
void Modulo(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// \brief Computes the modulo of each sample in an image with a constant.
///
/// Out will have the type `dt`.
///
/// \see Add, Subtract, Multiply, MultiplySampleWise, Divide, Modulo, operator%
template< typename T >
inline void Modulo(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Modulo( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Modulo(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Modulo( lhs, rhs, out, dt );
   return out;
}


/// \brief Inverts each sample of the input image, yielding an image of the same type.
///
/// For unsigned images, the output is `std::numeric_limits::max() - in`. For
/// signed and complex types, it is `0 - in`. For binary types it is the same as `dip::Not`.
///
/// \see operator-, Not
void Invert(
      Image const& in,
      Image& out
);

inline Image Invert(
      Image const& in
) {
   Image out;
   Invert( in, out );
   return out;
}


//
// Functions for bit-wise operations
//


/// \brief Bit-wise and of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see Or, Xor, operator&
void And(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

inline Image And(
      Image const& lhs,
      Image const& rhs
) {
   Image out;
   And( lhs, rhs, out );
   return out;
}


/// \brief Bit-wise or of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see And, Xor, operator|
void Or(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

inline Image Or(
      Image const& lhs,
      Image const& rhs
) {
   Image out;
   Or( lhs, rhs, out );
   return out;
}


/// \brief Bit-wise exclusive-or of two binary or integer images, sample-wise, with singleton expansion.
///
/// Out will have the type of `lhs`, and `rhs` will be converted to that type
/// before applying the operation.
///
/// \see And, Or, operator^
void Xor(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

inline Image Xor(
      Image const& lhs,
      Image const& rhs
) {
   Image out;
   Xor( lhs, rhs, out );
   return out;
}


/// \brief Applies bit-wise negation to each sample of the input image, yielding an
/// image of the same type.
///
/// \see operator!, operator~, Invert
void Not(
      Image const& in,
      Image& out
);

inline Image Not(
      Image const& in
) {
   Image out;
   Not( in, out );
   return out;
}


//
// Functions for comparison
//

/// \brief Equality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see NotEqual, Lesser, Greater, NotGreater, NotLesser, operator==
void Equal(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Equality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see NotEqual, Lesser, Greater, NotGreater, NotLesser, operator==
template< typename T >
inline void Equal(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   Equal( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image Equal(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   Equal( lhs, rhs, out );
   return out;
}


/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, Lesser, Greater, NotGreater, NotLesser, operator!=
void NotEqual(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, Lesser, Greater, NotGreater, NotLesser, operator!=
template< typename T >
inline void NotEqual(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   NotEqual( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image NotEqual(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   NotEqual( lhs, rhs, out );
   return out;
}


/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Greater, NotGreater, NotLesser, operator<
void Lesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Greater, NotGreater, NotLesser, operator<
template< typename T >
inline void Lesser(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   Lesser( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image Lesser(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   Lesser( lhs, rhs, out );
   return out;
}


/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, NotGreater, NotLesser, operator>
void Greater(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, NotGreater, NotLesser, operator>
template< typename T >
inline void Greater(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   Greater( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image Greater(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   Greater( lhs, rhs, out );
   return out;
}


/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotLesser, operator<=
void NotGreater(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotLesser, operator<=
template< typename T >
inline void NotGreater(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   NotGreater( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image NotGreater(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   NotGreater( lhs, rhs, out );
   return out;
}

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotGreater, operator>=
void NotLesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// \brief Inequality comparison, sample-wise, with singleton expansion.
///
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotGreater, operator>=
template< typename T >
inline void NotLesser(
      Image const& lhs,
      T const& rhs,
      Image& out
) {
   NotLesser( lhs, Image{ rhs }, out );
}

template< typename T >
inline Image NotLesser(
      Image const& lhs,
      T const& rhs
) {
   Image out;
   NotLesser( lhs, rhs, out );
   return out;
}


//
// Arithmetic operators
//

/// \brief Arithmetic operator, calls `dip::Add`.
inline Image operator+( Image const& lhs, Image const& rhs ) {
   return Add( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// \brief Arithmetic operator, calls `dip::Add`.
template< typename T >
inline Image operator+( Image const& lhs, T const& rhs ) {
   return Add( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// \brief Arithmetic operator, calls `dip::Subtract`.
inline Image operator-( Image const& lhs, Image const& rhs ) {
   return Subtract( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// \brief Arithmetic operator, calls `dip::Subtract`.
template< typename T >
inline Image operator-( Image const& lhs, T const& rhs ) {
   return Subtract( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// \brief Arithmetic operator, calls `dip::Multiply`.
inline Image operator*( Image const& lhs, Image const& rhs ) {
   return Multiply( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// \brief Arithmetic operator, calls `dip::Multiply`.
template< typename T >
inline Image operator*( Image const& lhs, T const& rhs ) {
   return Multiply( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// \brief Arithmetic operator, calls `dip::Divide`.
inline Image operator/( Image const& lhs, Image const& rhs ) {
   return Divide( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// \brief Arithmetic operator, calls `dip::Divide`.
template< typename T >
inline Image operator/( Image const& lhs, T const& rhs ) {
   return Divide( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// \brief Arithmetic operator, calls `dip::Modulo`.
inline Image operator%( Image const& lhs, Image const& rhs ) {
   return Modulo( lhs, rhs, lhs.DataType() );
}

/// \brief Arithmetic operator, calls `dip::Modulo`.
template< typename T >
inline Image operator%( Image const& lhs, T const& rhs ) {
   return Modulo( lhs, Image{ rhs }, lhs.DataType() );
}


//
// Bit-wise operators
//

/// \brief Bit-wise operator, calls `dip::And`.
inline Image operator&( Image const& lhs, Image const& rhs ) {
   return And( lhs, rhs );
}

/// \brief Bit-wise operator, calls `dip::Or`.
inline Image operator|( Image const& lhs, Image const& rhs ) {
   return Or( lhs, rhs );
}

/// \brief Bit-wise operator, calls `dip::Xor`.
inline Image operator^( Image const& lhs, Image const& rhs ) {
   return Xor( lhs, rhs );
}


//
// Unary operators
//

/// \brief Unary operator, calls `dip::Invert`.
inline Image operator-( Image const& in ) {
   return Invert( in );
}

/// \brief Bit-wise unary operator, calls `dip::Not`.
inline Image operator~( Image const& in ) {
   DIP_THROW_IF( !in.DataType().IsInteger(), "Bit-wise unary not operator only applicable to integer images" );
   return Not( in );
}

/// \brief Boolean unary operator, calls `dip::Not`.
inline Image operator!( Image const& in ) {
   DIP_THROW_IF( !in.DataType().IsBinary(), "Boolean unary not operator only applicable to binary images" );
   return Not( in );
}

//
// Comparison operators
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

//
// Compound assignment operators
//

/// \brief Compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator+=( Image& lhs, T const& rhs ) {
   Add( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator-=( Image& lhs, T const& rhs ) {
   Subtract( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator*=( Image& lhs, T const& rhs ) {
   Multiply( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator/=( Image& lhs, T const& rhs ) {
   Divide( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator%=( Image& lhs, T const& rhs ) {
   Modulo( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// \brief Bit-wise compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator&=( Image& lhs, Image const& rhs ) {
   And( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator|=( Image& lhs, Image const& rhs ) {
   Or( lhs, rhs, lhs );
   return lhs;
}

/// \brief Bit-wise compount assignment operator.
///
/// The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator^=( Image& lhs, Image const& rhs ) {
   Xor( lhs, rhs, lhs );
   return lhs;
}

/// \}

} // namespace dip

#endif // DIP_OPERATORS_H
