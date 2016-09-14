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

#include "dip_image.h"

/// \file
/// Declares the overloaded arithmetic and logical operators for dip::Image,
/// as well as functions implementing their functionality.
/// This file is always included through diplib.h.


namespace dip {


//
// Functions for arithmetic operations
//


/// Adds two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Sub, Mul, MulSamples, Div, Mod, operator+
void Add(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// Adds a constant to each sample in an image. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, Mod, operator+
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


/// Subtracts two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Add, Mul, MulSamples, Div, Mod, operator-
void Sub(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// Subtracts a constant from each sample in an image. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, Mod, operator-
template< typename T >
inline void Sub(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Sub( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Sub(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Sub( lhs, rhs, out, dt );
   return out;
}


/// Multiplies two images, pixel-wise, with singleton expansion. Tensor dimensions
/// of the two images must have identical inner dimensions, and the output at
/// each pixel will be the matrix multiplication of the two input pixels.
/// Out will have the type `dt`.
///
/// To obtain a sample-wise multiplication, convert the tensor dimension into
/// a spatial dimension using dip::Image::TensorToSpatial, and reshape the
/// output with dip::Image::SpatialToTensor. The helper function MulSamaples
/// does this.
///
/// \see Add, Sub, MulSamples, Div, Mod, operator*
void Mul(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// Multiplies each sample in an image by a constant. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, Mod, operator*
template< typename T >
inline void Mul(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Mul( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Mul(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Mul( lhs, rhs, out, dt );
   return out;
}

/// Adds two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod
void MulSamples(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

inline Image MulSamples(
      Image const& lhs,
      Image const& rhs,
      DataType dt
) {
   Image out;
   MulSamples( lhs, rhs, out, dt );
   return out;
}


/// Divides two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Mod, operator/
void Div(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// Divides each sample in an image by a constant. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, Mod, operator/
template< typename T >
inline void Div(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Div( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Div(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Div( lhs, rhs, out, dt );
   return out;
}


/// Computes the modulo of two images, sample-wise, with singleton expansion.
/// Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, operator%
void Mod(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
);

/// Computes the modulo of each sample in an image with a constant.
/// Out will have the type `dt`.
///
/// \see Add, Sub, Mul, MulSamples, Div, Mod, operator%
template< typename T >
inline void Mod(
      Image const& lhs,
      T const& rhs,
      Image& out,
      DataType dt
) {
   Mod( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Mod(
      Image const& lhs,
      T const& rhs,
      DataType dt
) {
   Image out;
   Mod( lhs, rhs, out, dt );
   return out;
}


/// Inverts each sample of the input image, yielding an image of the same type.
/// For unsigned images, the output is `std::numeric_limits::max() - in`, for
/// signed and complex types, it is `0 - in`.
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


/// Bit-wise and of two binary or integer images, sample-wise, with singleton expansion.
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


/// Bit-wise or of two binary or integer images, sample-wise, with singleton expansion.
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


/// Bit-wise exclusive-or of two binary or integer images, sample-wise, with singleton expansion.
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


/// Applies bit-wise negation to each sample of the input image, yielding an
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

/// Equality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see NotEqual, Lesser, Greater, NotGreater, NotLesser, operator==
void Equal(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Equality comparison, sample-wise, with singleton expansion.
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


/// Inequality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see Equal, Lesser, Greater, NotGreater, NotLesser, operator!=
void NotEqual(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Inequality comparison, sample-wise, with singleton expansion.
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


/// Inequality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see Equal, NotEqual, Greater, NotGreater, NotLesser, operator<
void Lesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Inequality comparison, sample-wise, with singleton expansion.
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


/// Inequality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, NotGreater, NotLesser, operator>
void Greater(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Inequality comparison, sample-wise, with singleton expansion.
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


/// Inequality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotLesser, operator<=
void NotGreater(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Inequality comparison, sample-wise, with singleton expansion.
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

/// Inequality comparison, sample-wise, with singleton expansion.
/// Out will be binary.
///
/// \see Equal, NotEqual, Lesser, Greater, NotGreater, operator>=
void NotLesser(
      Image const& lhs,
      Image const& rhs,
      Image& out
);

/// Inequality comparison, sample-wise, with singleton expansion.
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

/// Arithmetic operator, calls Add.
inline Image operator+( Image const& lhs, Image const& rhs ) {
   return Add( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// Arithmetic operator, calls Add.
template< typename T >
inline Image operator+( Image const& lhs, T const& rhs ) {
   return Add( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Sub.
inline Image operator-( Image const& lhs, Image const& rhs ) {
   return Sub( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// Arithmetic operator, calls Sub.
template< typename T >
inline Image operator-( Image const& lhs, T const& rhs ) {
   return Sub( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Mul.
inline Image operator*( Image const& lhs, Image const& rhs ) {
   return Mul( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// Arithmetic operator, calls Mul.
template< typename T >
inline Image operator*( Image const& lhs, T const& rhs ) {
   return Mul( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Div.
inline Image operator/( Image const& lhs, Image const& rhs ) {
   return Div( lhs, rhs, DataType::SuggestArithmetic( lhs.DataType(), rhs.DataType() ) );
}

/// Arithmetic operator, calls Div.
template< typename T >
inline Image operator/( Image const& lhs, T const& rhs ) {
   return Div( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Mod.
inline Image operator%( Image const& lhs, Image const& rhs ) {
   return Mod( lhs, rhs, lhs.DataType() );
}

/// Arithmetic operator, calls Mod.
template< typename T >
inline Image operator%( Image const& lhs, T const& rhs ) {
   return Mod( lhs, Image{ rhs }, lhs.DataType() );
}


//
// Bit-wise operators
//

/// Boolean operator, calls And.
inline Image operator&( Image const& lhs, Image const& rhs ) {
   return And( lhs, rhs );
}

/// Boolean operator, calls Or.
inline Image operator|( Image const& lhs, Image const& rhs ) {
   return Or( lhs, rhs );
}

/// Boolean operator, calls Xor.
inline Image operator^( Image const& lhs, Image const& rhs ) {
   return Xor( lhs, rhs );
}


//
// Unary operators
//

/// Unary operator, calls Invert.
inline Image operator-( Image const& in ) {
   return Invert( in );
}

/// Bit-wise unary operator, calls Not.
inline Image operator~( Image const& in ) {
   dip_ThrowIf( !in.DataType().IsInteger(), "Bit-wise unary not operator only applicable to integer images" );
   return Not( in );
}

/// Boolean unary operator, calls Not.
inline Image operator!( Image const& in ) {
   dip_ThrowIf( !in.DataType().IsBinary(), "Boolean unary not operator only applicable to binary images" );
   return Not( in );
}

//
// Comparison operators
//

/// Comparison operator, calls Equal.
template< typename T >
inline Image operator==( Image const& lhs, T const& rhs ) {
   return Equal( lhs, rhs );
}

/// Comparison operator, calls NotEqual.
template< typename T >
inline Image operator!=( Image const& lhs, T const& rhs ) {
   return NotEqual( lhs, rhs );
}

/// Comparison operator, calls Lesser.
template< typename T >
inline Image operator<( Image const& lhs, T const& rhs ) {
   return Lesser( lhs, rhs );
}

/// Comparison operator, calls Greater.
template< typename T >
inline Image operator>( Image const& lhs, T const& rhs ) {
   return Greater( lhs, rhs );
}

/// Comparison operator, calls NotGreater.
template< typename T >
inline Image operator<=( Image const& lhs, T const& rhs ) {
   return NotGreater( lhs, rhs );
}

/// Comparison operator, calls NotLesser.
template< typename T >
inline Image operator>=( Image const& lhs, T const& rhs ) {
   return NotLesser( lhs, rhs );
}

//
// Compound assignment operators
//

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator+=( Image& lhs, T const& rhs ) {
   Add( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator-=( Image& lhs, T const& rhs ) {
   Sub( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator*=( Image& lhs, T const& rhs ) {
   Mul( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator/=( Image& lhs, T const& rhs ) {
   Div( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
template< typename T >
inline Image& operator%=( Image& lhs, T const& rhs ) {
   Mod( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator&=( Image& lhs, Image const& rhs ) {
   And( lhs, rhs, lhs );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator|=( Image& lhs, Image const& rhs ) {
   Or( lhs, rhs, lhs );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator^=( Image& lhs, Image const& rhs ) {
   Xor( lhs, rhs, lhs );
   return lhs;
}


} // namespace dip

#endif // DIP_OPERATORS_H
