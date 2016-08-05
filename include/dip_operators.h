/*
 * DIPlib 3.0
 * This file contains the definition of the image arithmetic and logical operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

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
/// \see Sub, Mul, Div, Mod, operator+
void Add(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
);

/// Adds a constant to each sample in an image. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod, operator+
template< typename T >
inline void Add(
      const Image& lhs,
      T rhs,
      Image& out,
      DataType dt
) {
   Add( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Add(
      const Image& lhs,
      T rhs,
      DataType dt
) {
   Image out;
   Add( lhs, rhs, out, dt );
   return out;
}


/// Subtracts two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Add, Mul, Div, Mod, operator-
void Sub(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
);

/// Subtracts a constant from each sample in an image. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod, operator-
template< typename T >
inline void Sub(
      const Image& lhs,
      T rhs,
      Image& out,
      DataType dt
) {
   Sub( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Sub(
      const Image& lhs,
      T rhs,
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
/// \see Add, Sub, Div, Mod, operator*, MulSamples
void Mul(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
);

/// Multiplies each sample in an image by a constant. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod, operator*
template< typename T >
inline void Mul(
      const Image& lhs,
      T rhs,
      Image& out,
      DataType dt
) {
   Mul( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Mul(
      const Image& lhs,
      T rhs,
      DataType dt
) {
   Image out;
   Mul( lhs, rhs, out, dt );
   return out;
}

/// Multiplies two images, sample-wise, with singleton expansion. If the tensor
/// dimension is singleton-expanded, it might be reshaped to a vector.
/// Out will have the type `dt`.
///
/// \see Mul
inline void MulSamples(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
) {
   dip_ThrowIf( lhs.Dimensionality() != rhs.Dimensionality(), E::DIMENSIONS_DONT_MATCH );
   Image newlhs = lhs; newlhs.TensorToSpatial(-1);
   Image newrhs = rhs; newrhs.TensorToSpatial(-1);
   Mul( newlhs, newrhs, out, dt );
   out.SpatialToTensor(-1);
   // If the number of tensor elements matches either LHS or RHS operand,
   // then we copy over that tensor shape. Otherwise we leave it as a vector.
   if( out.TensorElements() == lhs.TensorElements() ) {
      out.ReshapeTensor( lhs.Tensor() );
   } else if( out.TensorElements() == rhs.TensorElements() ) {
      out.ReshapeTensor( rhs.Tensor() );
   }
}

inline Image MulSamples(
      const Image& lhs,
      const Image& rhs,
      DataType dt
) {
   Image out;
   MulSamples( lhs, rhs, out, dt );
   return out;
}


/// Divides two images, sample-wise, with singleton expansion. Out will have the
/// type `dt`.
///
/// \see Add, Sub, Mul, Mod, operator/
void Div(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
);

/// Divides each sample in an image by a constant. Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod, operator/
template< typename T >
inline void Div(
      const Image& lhs,
      T rhs,
      Image& out,
      DataType dt
) {
   Div( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Div(
      const Image& lhs,
      T rhs,
      DataType dt
) {
   Image out;
   Div( lhs, rhs, out, dt );
   return out;
}


/// Computes the modulo of two images, sample-wise, with singleton expansion.
/// Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, operator%
void Mod(
      const Image& lhs,
      const Image& rhs,
      Image& out,
      DataType dt
);

/// Computes the modulo of each sample in an image with a constant.
/// Out will have the type `dt`.
///
/// \see Add, Sub, Mul, Div, Mod, operator%
template< typename T >
inline void Mod(
      const Image& lhs,
      T rhs,
      Image& out,
      DataType dt
) {
   Mod( lhs, Image{ rhs }, out, dt );
}

template< typename T >
inline Image Mod(
      const Image& lhs,
      T rhs,
      DataType dt
) {
   Image out;
   Mod( lhs, rhs, out, dt );
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
      const Image& lhs,
      const Image& rhs,
      Image& out
);

inline Image And(
      const Image& lhs,
      const Image& rhs
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
      const Image& lhs,
      const Image& rhs,
      Image& out
);

inline Image Or(
      const Image& lhs,
      const Image& rhs
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
      const Image& lhs,
      const Image& rhs,
      Image& out
);

inline Image Xor(
      const Image& lhs,
      const Image& rhs
) {
   Image out;
   Xor( lhs, rhs, out );
   return out;
}


//
// Functions for unary negation operations
//

/// Inverts each sample of the input image, yielding an image of the same type.
/// For unsigned images, the output is `std::numeric_limits::max() - in`, for
/// signed and complex types, it is `0 - in`.
///
/// \see operator-, Not
void Invert(
      const Image& in,
      Image& out
);
// out = Invert( in );

/// Applies bit-wise negation to each sample of the input image, yielding an
/// image of the same type.
///
/// \see operator!, operator~, Invert
void Not(
      const Image& in,
      Image& out
);
// out = Not( in );


//
// Functions for comparison
//

   // Equal( lhs, rhs, out );
   // NotEqual( lhs, rhs, out );
   // Lesser( lhs, rhs, out );
   // Greater( lhs, rhs, out );
   // NotGreater( lhs, rhs, out );
   // NotLesser( lhs, rhs, out );


//
// Arithmetic operators
//

/// Arithmetic operator, calls Add.
inline Image operator+( const Image& lhs, const Image& rhs ) {
   return Add( lhs, rhs, DataType::SuggestArithmetic( lhs, rhs ) );
}

/// Arithmetic operator, calls Add.
template< typename T >
inline Image operator+( const Image& lhs, T rhs ) {
   return Add( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Sub.
inline Image operator-( const Image& lhs, const Image& rhs ) {
   return Sub( lhs, rhs, DataType::SuggestArithmetic( lhs, rhs ) );
}

/// Arithmetic operator, calls Sub.
template< typename T >
inline Image operator-( const Image& lhs, T rhs ) {
   return Sub( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Mul.
inline Image operator*( const Image& lhs, const Image& rhs ) {
   return Mul( lhs, rhs, DataType::SuggestArithmetic( lhs, rhs ) );
}

/// Arithmetic operator, calls Mul.
template< typename T >
inline Image operator*( const Image& lhs, T rhs ) {
   return Mul( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Div.
inline Image operator/( const Image& lhs, const Image& rhs ) {
   return Div( lhs, rhs, DataType::SuggestArithmetic( lhs, rhs ) );
}

/// Arithmetic operator, calls Div.
template< typename T >
inline Image operator/( const Image& lhs, T rhs ) {
   return Div( lhs, Image{ rhs }, DataType::SuggestArithmetic( lhs.DataType(), DataType( rhs ) ) );
}

/// Arithmetic operator, calls Mod.
inline Image operator%( const Image& lhs, const Image& rhs ) {
   return Mod( lhs, rhs, lhs.DataType() );
}

/// Arithmetic operator, calls Mod.
template< typename T >
inline Image operator%( const Image& lhs, T rhs ) {
   return Mod( lhs, Image{ rhs }, lhs.DataType() );
}


//
// Bit-wise operators
//

/// Boolean operator, calls And.
inline Image operator&( const Image& lhs, const Image& rhs ) {
   return And( lhs, rhs );
}

/// Boolean operator, calls Or.
inline Image operator|( const Image& lhs, const Image& rhs ) {
   return Or( lhs, rhs );
}

/// Boolean operator, calls Xor.
inline Image operator^( const Image& lhs, const Image& rhs ) {
   return Xor( lhs, rhs );
}


//
// Unary operators
//

/// Unary operator, calls Invert.
inline Image operator-( const Image& in ) {
   //Invert( in, out );
   return {};
}

/// Bit-wise unary operator, calls Not.
inline Image operator~( const Image& in ) {
   dip_ThrowIf( !in.DataType().IsInteger(), "Bit-wise unary not operator only applicable to integer images" );
   //Not( in, out );
   return {};
}

/// Boolean unary operator, calls Not.
inline Image operator!( const Image& in ) {
   dip_ThrowIf( !in.DataType().IsBinary(), "Boolean unary not operator only applicable to binary images" );
   //Not( in, out );
   return {};
}

//
// Comparison operators
//

/// Comparison operator, calls Equal.
inline Image operator==( const Image& lhs, const Image& rhs ) {
   // return Equal( lhs, rhs );
   return {};
}

/// Comparison operator, calls Equal.
inline Image operator==( const Image& lhs, dip::sint rhs ) {
   // return Equal( lhs, rhs );
   return {};
}

/// Comparison operator, calls Equal.
inline Image operator==( const Image& lhs, dfloat rhs ) {
   // return Equal( lhs, rhs );
   return {};
}

/// Comparison operator, calls Equal.
inline Image operator==( const Image& lhs, dcomplex rhs ) {
   // return Equal( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotEqual.
inline Image operator!=( const Image& lhs, const Image& rhs ) {
   // return NotEqual( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotEqual.
inline Image operator!=( const Image& lhs, dip::sint rhs ) {
   // return NotEqual( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotEqual.
inline Image operator!=( const Image& lhs, dfloat rhs ) {
   // return NotEqual( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotEqual.
inline Image operator!=( const Image& lhs, dcomplex rhs ) {
   // return NotEqual( lhs, rhs );
   return {};
}

/// Comparison operator, calls Lesser.
inline Image operator< ( const Image& lhs, const Image& rhs ) {
   // return Lesser( lhs, rhs );
   return {};
}

/// Comparison operator, calls Lesser.
inline Image operator< ( const Image& lhs, dip::sint rhs ) {
   // return Lesser( lhs, rhs );
   return {};
}

/// Comparison operator, calls Lesser.
inline Image operator< ( const Image& lhs, dfloat rhs ) {
   // return Lesser( lhs, rhs );
   return {};
}

/// Comparison operator, calls Greater.
inline Image operator> ( const Image& lhs, const Image& rhs ) {
   // return Greater( lhs, rhs );
   return {};
}

/// Comparison operator, calls Greater.
inline Image operator> ( const Image& lhs, dip::sint rhs ) {
   // return Greater( lhs, rhs );
   return {};
}

/// Comparison operator, calls Greater.
inline Image operator> ( const Image& lhs, dfloat rhs ) {
   // return Greater( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotGreater.
inline Image operator<=( const Image& lhs, const Image& rhs ) {
   // return NotGreater( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotGreater.
inline Image operator<=( const Image& lhs, dip::sint rhs ) {
   // return NotGreater( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotGreater.
inline Image operator<=( const Image& lhs, dfloat rhs ) {
   // return NotGreater( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotLesser.
inline Image operator>=( const Image& lhs, const Image& rhs ) {
   // return NotLesser( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotLesser.
inline Image operator>=( const Image& lhs, dip::sint rhs ) {
   // return NotLesser( lhs, rhs );
   return {};
}

/// Comparison operator, calls NotLesser.
inline Image operator>=( const Image& lhs, dfloat rhs ) {
   // return NotLesser( lhs, rhs );
   return {};
}

//
// Compound assignment operators
//

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator+=( Image& lhs, const Image& rhs ) {
   Add( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator-=( Image& lhs, const Image& rhs ) {
   Sub( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator*=( Image& lhs, const Image& rhs ) {
   Mul( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator/=( Image& lhs, const Image& rhs ) {
   Div( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator%=( Image& lhs, const Image& rhs ) {
   Mod( lhs, rhs, lhs, lhs.DataType() );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator&=( Image& lhs, const Image& rhs ) {
   And( lhs, rhs, lhs );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator|=( Image& lhs, const Image& rhs ) {
   Or( lhs, rhs, lhs );
   return lhs;
}

/// Bit-wise compount assignment operator. The operation is performed in-place only
/// if size is not changed by the operation. Singleton expansion
/// could change the size of `lhs`.
inline Image& operator^=( Image& lhs, const Image& rhs ) {
   Xor( lhs, rhs, lhs );
   return lhs;
}


} // namespace dip

#endif // DIP_OPERATORS_H
