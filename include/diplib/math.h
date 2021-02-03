/*
 * DIPlib 3.0
 * This file contains declarations for image math functions.
 *
 * (c)2014-2020, Cris Luengo.
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

#ifndef DIP_MATH_H
#define DIP_MATH_H

#include "diplib.h"


/// \file
/// \brief Declares image math functions, except basic arithmetic and comparison.
/// \see diplib/library/operators.h, math


/// \defgroup math Math and statistics
/// \brief The image math and statistics functions and operators.


namespace dip {


//
// Arithmetic, trigonometric and similar monadic operators
//

#define DIP_MONADIC_OPERATOR_FLEX( functionName_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#define DIP_MONADIC_OPERATOR_FLOAT( functionName_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#define DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( functionName_, paramType_, paramName_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out, paramType_ paramName_ ); \
   inline Image functionName_( Image const& in, paramType_ paramName_ ) { Image out; functionName_( in, out, paramName_ ); return out; }

#define DIP_MONADIC_OPERATOR_BIN( functionName_ ) \
   DIP_EXPORT void functionName_( Image const& in, Image& out ); \
   inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

/// \addtogroup math_arithmetic
/// \{

/// \brief Computes the nearest integer to each sample (rounds).
/// Only defined for floating-point types, the output is the same type.
DIP_MONADIC_OPERATOR_FLOAT( Round )

/// \brief Computes the smallest integer larger or equal to each sample (rounds up).
/// Only defined for floating-point types, the output is the same type.
DIP_MONADIC_OPERATOR_FLOAT( Ceil )

/// \brief Computes the largest integer smaller or equal to each sample (rounds down).
/// Only defined for floating-point types, the output is the same type.
DIP_MONADIC_OPERATOR_FLOAT( Floor )

/// \brief Computes the truncated value of each sample (rounds towards zero).
/// Only defined for floating-point types, the output is the same type.
DIP_MONADIC_OPERATOR_FLOAT( Truncate )

/// \brief Computes the fractional value of each sample (`out = in - dip::Truncate(in)`).
/// Only defined for floating-point types, the output is the same type.
DIP_MONADIC_OPERATOR_FLOAT( Fraction )

/// \brief Computes the reciprocal of each sample: out = in == 0 ? 0 : 1/in.
DIP_MONADIC_OPERATOR_FLEX( Reciprocal )

/// \brief Computes the square of each sample.
DIP_MONADIC_OPERATOR_FLEX( Square )

/// \brief Computes the square root of each sample.
DIP_MONADIC_OPERATOR_FLEX( Sqrt )

/// \brief Computes the base e exponent (natural exponential) of each sample.
DIP_MONADIC_OPERATOR_FLEX( Exp )

/// \brief Computes the base 2 exponent of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Exp2 )

/// \brief Computes the base 10 exponent of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Exp10 )

/// \brief Computes the natural logarithm (base e logarithm) of each sample.
DIP_MONADIC_OPERATOR_FLEX( Ln )

/// \brief Computes the base 2 logarithm of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Log2 )

/// \brief Computes the base 10 logarithm of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Log10 )

/// \}

/// \addtogroup math_trigonometric
/// \{

/// \brief Computes the sine of each sample.
DIP_MONADIC_OPERATOR_FLEX( Sin )

/// \brief Computes the cosine of each sample.
DIP_MONADIC_OPERATOR_FLEX( Cos )

/// \brief Computes the tangent of each sample.
DIP_MONADIC_OPERATOR_FLEX( Tan )

/// \brief Computes the arc sine of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Asin )

/// \brief Computes the arc cosine of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Acos )

/// \brief Computes the arc tangent of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Atan )

/// \brief Computes the hyperbolic sine of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Sinh )

/// \brief Computes the hyperbolic cosine of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Cosh )

/// \brief Computes the hyperbolic tangent of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Tanh )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 0.
DIP_MONADIC_OPERATOR_FLOAT( BesselJ0 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 1.
DIP_MONADIC_OPERATOR_FLOAT( BesselJ1 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order `alpha`.
DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselJN, dip::uint, alpha )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 0.
DIP_MONADIC_OPERATOR_FLOAT( BesselY0 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 1.
DIP_MONADIC_OPERATOR_FLOAT( BesselY1 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order `alpha`.
DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM( BesselYN, dip::uint, alpha )

/// \brief Computes the natural logarithm of the gamma function of each sample.
DIP_MONADIC_OPERATOR_FLOAT( LnGamma )

/// \brief Computes the error function of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Erf )

/// \brief Computes the complementary error function of each sample.
DIP_MONADIC_OPERATOR_FLOAT( Erfc )

/// \brief Computes the sinc function of each sample. $\mathrm{sinc}(x) = \sin(x)/x$.
DIP_MONADIC_OPERATOR_FLOAT( Sinc )

/// \}

/// \addtogroup math_comparison
/// \{

/// \brief True for each pixel that is NaN.
DIP_MONADIC_OPERATOR_BIN( IsNotANumber )

/// \brief True for each pixel that is positive or negative infinity.
DIP_MONADIC_OPERATOR_BIN( IsInfinite )

/// \brief True for each pixel that is not NaN nor infinity.
DIP_MONADIC_OPERATOR_BIN( IsFinite )

/// \}

#undef DIP_MONADIC_OPERATOR_FLEX
#undef DIP_MONADIC_OPERATOR_FLOAT
#undef DIP_MONADIC_OPERATOR_FLOAT_WITH_PARAM
#undef DIP_MONADIC_OPERATOR_BIN



/// \defgroup math_arithmetic Arithmetic operators
/// \ingroup math
/// \brief Monadic and dyadic image arithmetic operators.
/// \{

/// \brief Computes the absolute value of each sample.
DIP_EXPORT void Abs( Image const& in, Image& out );
inline Image Abs( Image const& in ) {
   Image out;
   Abs( in, out );
   return out;
}

/// \brief Computes the modulus (absolute value) of each sample. `%dip::Modulus` is an alias for `dip::Abs`.
inline void Modulus( Image const& in, Image& out ) { Abs( in, out ); }
inline Image Modulus( Image const& in ) { return Abs( in ); }

/// \brief Computes the square of the modulus of each sample.
DIP_EXPORT void SquareModulus( Image const& in, Image& out );
inline Image SquareModulus( Image const& in ) {
   Image out;
   SquareModulus( in, out );
   return out;
}

/// \brief Computes the phase (angle on complex plane, through `std::arg`) of each sample.
DIP_EXPORT void Phase( Image const& in, Image& out );
inline Image Phase( Image const& in ) {
   Image out;
   Phase( in, out );
   return out;
}

inline Image Real( Image const& in ) { return in.DataType().IsComplex() ? Image( in.Real() ) : in; }
/// \brief Returns the real component of a complex image. Returns `dip::Image::Real` if the input is complex.
inline void Real( Image const& in, Image& out ) { out = Real( in ); }

inline Image Imaginary( Image const& in ) { return in.DataType().IsComplex() ? Image( in.Imaginary() ) : in; }
/// \brief Returns the imaginary component of a complex image. Returns `dip::Image::Imaginary` if the input is complex
inline void Imaginary( Image const& in, Image& out ) { out = Imaginary( in ); }

/// \brief Computes the complex conjugate of each sample.
DIP_EXPORT void Conjugate( Image const& in, Image& out );
inline Image Conjugate( Image const& in ) {
   Image out;
   Conjugate( in, out );
   return out;
}

/// \brief Computes the sign of each sample. Only defined for signed real data types (signed integers
/// and floating-point types). Output is of type `dip::DT_SINT8`, containing values -1, 0 and 1.
DIP_EXPORT void Sign( Image const& in, Image& out );
inline Image Sign( Image const& in ) {
   Image out;
   Sign( in, out );
   return out;
}

/// \brief Computes the integer closest to the value of each sample.
/// Only defined for floating-point types, the output is of type `dip::DT_SINT32`.
DIP_EXPORT void NearestInt( Image const& in, Image& out );
inline Image NearestInt( Image const& in ) {
   Image out;
   NearestInt( in, out );
   return out;
}

/// \brief Computes the sample-wise supremum (maximum) over all the input images. For binary images, this is the same
/// as the union.
DIP_EXPORT void Supremum( ImageConstRefArray const& in, Image& out );
inline Image Supremum( ImageConstRefArray const& in ) {
   Image out;
   Supremum( in, out );
   return out;
}

/// \brief Computes the sample-wise supremum (maximum) of the two input images. For binary images, this is the same
/// as the union.
inline void Supremum( Image const& a, Image const& b, Image& out ) {
   if( a.DataType().IsBinary() && b.DataType().IsBinary() ) {
      Or( a, b, out );
   } else {
      Supremum( { a, b }, out );
   }
}
inline Image Supremum( Image const& a, Image const& b ) {
   Image out;
   Supremum( a, b, out );
   return out;
}

/// \brief Computes the sample-wise infimum (minimum) over all the input images. For binary images, this is the same
/// as the intersection.
DIP_EXPORT void Infimum( ImageConstRefArray const& in, Image& out );
inline Image Infimum( ImageConstRefArray const& in ) {
   Image out;
   Infimum( in, out );
   return out;
}

/// \brief Computes the sample-wise infimum (minimum) of the two input images. For binary images, this is the same
/// as the intersection.
inline void Infimum( Image const& a, Image const& b, Image& out ) {
   if( a.DataType().IsBinary() && b.DataType().IsBinary() ) {
      And( a, b, out );
   } else {
      Infimum( { a, b }, out );
   }
}
inline Image Infimum( Image const& a, Image const& b ) {
   Image out;
   Infimum( a, b, out );
   return out;
}

/// \brief Computes the sample-wise signed infimum (minimum) of the two input images: returns `-b` where `b < a`, a otherwise.
DIP_EXPORT void SignedInfimum( Image const& a, Image const& b, Image& out );
inline Image SignedInfimum( Image const& a, Image const& b ) {
   Image out;
   SignedInfimum( a, b, out );
   return out;
}

/// \brief Computes the linear combination of the two images, sample-wise.
///
/// The actual operation applied is:
///
/// ```cpp
///     out = a * aWeight + b * bWeight;
/// ```
///
/// With defaults weights of 0.5, the function computes the average of two images.
DIP_EXPORT void LinearCombination(
      Image const& a,
      Image const& b,
      Image& out,
      dfloat aWeight = 0.5,
      dfloat bWeight = 0.5
);
inline Image LinearCombination(
      Image const& a,
      Image const& b,
      dfloat aWeight = 0.5,
      dfloat bWeight = 0.5
) {
   Image out;
   LinearCombination( a, b, out, aWeight, bWeight );
   return out;
}

/// \brief Computes the linear combination of the two complex images, sample-wise, yielding a complex output,
///
/// The actual operation applied is:
///
/// ```cpp
///     out = a * aWeight + b * bWeight;
/// ```
///
/// The images `a` and `b` do not necessarily need to be complex, but the computation will be performed with
/// complex arithmetic.
DIP_EXPORT void LinearCombination(
      Image const& a,
      Image const& b,
      Image& out,
      dcomplex aWeight,
      dcomplex bWeight
);
inline Image LinearCombination(
      Image const& a,
      Image const& b,
      dcomplex aWeight,
      dcomplex bWeight
) {
   Image out;
   LinearCombination( a, b, out, aWeight, bWeight );
   return out;
}


/// \}


//
// Arithmetic dyadic operators that are not already declared in diplib/library/operators.h
//

/// \defgroup math_trigonometric Trigonometric operators
/// \ingroup math
/// \brief Monadic and dyadic image trigonometric operators and other complex functions.
/// \{

/// \brief Computes the four-quadrant arc tangent of `y/x`.
///
/// The operation can be understood as the angle of the vector formed by the two input images.
/// The result is always in the range \f$[-\pi,\pi]\f$. The inputs must be a real type.
DIP_EXPORT void Atan2( Image const& y, Image const& x, Image& out );
inline Image Atan2( Image const& y, Image const& x ) {
   Image out;
   Atan2( y, x, out );
   return out;
}

/// \brief Computes the square root of the sum of the squares of corresponding samples in `a` and `b`.
///
/// The computation is performed carefully, so there is no undue overflow or underflow at intermediate
/// stages of the computation. The inputs must be a real type.
DIP_EXPORT void Hypot( Image const& a, Image const& b, Image& out );
inline Image Hypot( Image const& a, Image const& b ) {
   Image out;
   Hypot( a, b, out );
   return out;
}

/// \}


//
// Tensor operators
//

/// \defgroup math_tensor Tensor operators
/// \ingroup math
/// \brief Operators specific to tensor images.
/// \{

/// \brief Transposes the tensor image, the data are not copied.
inline Image Transpose( Image const& in ) {
   Image out = in;
   out.Transpose();
   return out;
}

/// \brief Computes the conjugate transpose of the tensor image `in`.
inline void ConjugateTranspose( Image const& in, Image& out ) {
   Conjugate( in, out );
   out.Transpose();
}
inline Image ConjugateTranspose( Image const& in ) {
   return Conjugate( in ).Transpose();
}

/// \brief Computes the dot product (inner product) of two vector images.
DIP_EXPORT void DotProduct( Image const& lhs, Image const& rhs, Image& out );
inline Image DotProduct( Image const& lhs, Image const& rhs ) {
   Image out;
   DotProduct( lhs, rhs, out );
   return out;
}

/// \brief Computes the cross product (inner product) of two vector images.
///
/// Input image tensors must be 2-vectors or 3-vectors. For 3-vectors, the cross product is as
/// commonly defined in 3D. For 2-vectors, we define the cross product as the z-component
/// of the cross product of the 3D vectors obtained by adding a 0 z-component to the inputs.
/// That is, it is the area of the parallelogram formed by the two 2D vectors.
DIP_EXPORT void CrossProduct( Image const& lhs, Image const& rhs, Image& out );
inline Image CrossProduct( Image const& lhs, Image const& rhs ) {
   Image out;
   CrossProduct( lhs, rhs, out );
   return out;
}

/// \brief Computes the norm of the vector at each pixel in image `in`.
DIP_EXPORT void Norm( Image const& in, Image& out );
inline Image Norm( Image const& in ) {
   Image out;
   Norm( in, out );
   return out;
}

/// \brief Computes the square of the norm of the vector at each pixel in image `in`.
DIP_EXPORT void SquareNorm( Image const& in, Image& out );
inline Image SquareNorm( Image const& in ) {
   Image out;
   SquareNorm( in, out );
   return out;
}

/// \brief Computes the angle of the vector at each pixel in image `in`.
///
/// `in` must be a 2-vector or a 3-vector. For a 2-vector, `out` is a scalar image representing
/// *phi*, the angle from the x-axis. For a 3-vector, `out` has 2 tensor components, corresponding
/// to *phi* and *theta*. *phi*, as in the 2D case, is the angle from the x-axis within the x-y plane
/// (azimuth). *theta* is the angle from the z-axis (inclination). See `dip::CartesianToPolar` for
/// more details. This function yields the same output as `dip::CartesianToPolar`, but without
/// the first tensor component.
///
/// \see dip::Norm, dip::Orientation, dip::PolarToCartesian, dip::CartesianToPolar
DIP_EXPORT void Angle( Image const& in, Image& out );
inline Image Angle( Image const& in ) {
   Image out;
   Angle( in, out );
   return out;
}

/// \brief Computes the orientation of the vector at each pixel in image `in`.
///
/// Orientation is defined as the angle mapped to the half-circle or half-sphere with positive x-coordinate.
/// That is, in 2D it is an angle in the range (-&pi;/2, &pi;/2), and in 3D the *phi* component is mapped to
/// that same range. See `dip::Angle` for more information.
///
/// \see dip::Norm, dip::Angle
DIP_EXPORT void Orientation( Image const& in, Image& out );
inline Image Orientation( Image const& in ) {
   Image out;
   Orientation( in, out );
   return out;
}

/// \brief Converts the vector at each pixel in image `in` from Cartesian coordinates to polar
/// (or spherical) coordinates.
///
/// `in` must be a 2-vector or a 3-vector. `out` is a same-size vector containing *r* and *phi*
/// in the 2D case, and *r*, *phi* and *theta* in the 3D case. *phi* is the angle to the x-axis
/// within the x-y plane (azimuth). *theta* is the angle from the z-axis (inclination).
///
/// That is, in 2D:
///
/// ```cpp
///     in[ 0 ] == out[ 0 ] * Cos( out[ 1 ] );
///     in[ 1 ] == out[ 0 ] * Sin( out[ 1 ] );
/// ```
///
/// and in 3D:
///
/// ```cpp
///     in[ 0 ] == out[ 0 ] * Cos( out[ 1 ] ) * Sin( out[ 2 ] );
///     in[ 1 ] == out[ 0 ] * Sin( out[ 1 ] ) * Sin( out[ 2 ] );
///     in[ 2 ] == out[ 0 ] * Cos( out[ 2 ] );
/// ```
///
/// \see dip::PolarToCartesian, dip::Norm, dip::Angle
DIP_EXPORT void CartesianToPolar( Image const& in, Image& out );
inline Image CartesianToPolar( Image const& in ) {
   Image out;
   CartesianToPolar( in, out );
   return out;
}

/// \brief Converts the vector at each pixel in image `in` from polar (or spherical) coordinates to
/// Cartesian coordinates.
///
/// `in` must be a 2-vector or a 3-vector. See `dip::CartesianToPolar` for a description of the polar
/// coordinates used.
///
/// \see dip::CartesianToPolar, dip::Norm, dip::Angle
DIP_EXPORT void PolarToCartesian( Image const& in, Image& out );
inline Image PolarToCartesian( Image const& in ) {
   Image out;
   PolarToCartesian( in, out );
   return out;
}

/// \brief Computes the determinant of the square matrix at each pixel in image `in`.
DIP_EXPORT void Determinant( Image const& in, Image& out );
inline Image Determinant( Image const& in ) {
   Image out;
   Determinant( in, out );
   return out;
}

/// \brief Computes the trace of the square matrix at each pixel in image `in`.
DIP_EXPORT void Trace( Image const& in, Image& out );
inline Image Trace( Image const& in ) {
   Image out;
   Trace( in, out );
   return out;
}

/// \brief Computes the rank of the square matrix at each pixel in image `in`.
/// The output is DT_UINT8, under the assumption that we won't have tensor images with a rank higher than 255.
DIP_EXPORT void Rank( Image const& in, Image& out );
inline Image Rank( Image const& in ) {
   Image out;
   Rank( in, out );
   return out;
}

/// \brief Computes the eigenvalues of the square matrix at each pixel in image `in`.
///
/// `out` is a vector image containing the eigenvalues. If `in` is symmetric and
/// real-valued, then `out` is real-valued, otherwise, `out` is complex-valued.
/// The eigenvalues are sorted by magnitude, in descending order.
DIP_EXPORT void Eigenvalues( Image const& in, Image& out );
inline Image Eigenvalues( Image const& in ) {
   Image out;
   Eigenvalues( in, out );
   return out;
}

/// \brief Finds the largest eigenvalue of the square matrix at each pixel in image `in`.
///
/// Computes the eigenvalues in the same way as `dip::Eigenvalues`, but
/// outputs only the eigenvector with the largest magnitude.
DIP_EXPORT void LargestEigenvalue( Image const& in, Image& out );
inline Image LargestEigenvalue( Image const& in ) {
   Image out;
   LargestEigenvalue( in, out );
   return out;
}

/// \brief Finds the smallest eigenvalue of the square matrix at each pixel in image `in`.
///
/// Computes the eigenvalues in the same way as `dip::Eigenvalues`, but
/// outputs only the eigenvector with the smallest magnitude.
DIP_EXPORT void SmallestEigenvalue( Image const& in, Image& out );
inline Image SmallestEigenvalue( Image const& in ) {
   Image out;
   SmallestEigenvalue( in, out );
   return out;
}

/// \brief Computes the eigenvalues and eigenvectors of the square matrix at each pixel in image `in`.
///
/// The decomposition is such that `in * eigenvectors == eigenvectors * out`.
/// `eigenvectors` is almost always invertible, in which case one can write
/// `in == eigenvectors * out * Inverse( eigenvectors )`.
///
/// `out` is a diagonal matrix image containing the eigenvalues. If `in` is symmetric and
/// real-valued, then `out` is real-valued, otherwise, `out` is complex-valued.
/// The eigenvalues are sorted by magnitude, in descending order.
///
/// The eigenvectors are the columns `eigenvectors`. It has the same data type as `out`.
DIP_EXPORT void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors );

/// \brief Finds the largest eigenvector of the symmetric matrix at each pixel in image `in`.
///
/// Computes the eigen decomposition in the same way as `dip::EigenDecomposition`, but
/// outputs only the eigenvector that corresponds to the eigenvalue with largest magnitude.
///
/// `in` must be symmetric and real-valued.
DIP_EXPORT void LargestEigenvector( Image const& in, Image& out );
inline Image LargestEigenvector( Image const& in ) {
   Image out;
   LargestEigenvector( in, out );
   return out;
}

/// \brief Finds the smallest eigenvector of the symmetric matrix at each pixel in image `in`.
///
/// Computes the eigen decomposition in the same way as `dip::EigenDecomposition`, but
/// outputs only the eigenvector that corresponds to the eigenvalue with smallest magnitude.
///
/// `in` must be symmetric and real-valued.
DIP_EXPORT void SmallestEigenvector( Image const& in, Image& out );
inline Image SmallestEigenvector( Image const& in ) {
   Image out;
   SmallestEigenvector( in, out );
   return out;
}

/// \brief Computes the inverse of the square matrix at each pixel in image `in`.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( Image const& in, Image& out );
inline Image Inverse( Image const& in ) {
   Image out;
   Inverse( in, out );
   return out;
}

/// \brief Computes the pseudo-inverse of the matrix at each pixel in image `in`.
///
/// Computes the Moore-Penrose pseudo-inverse using `tolerance`. Singular values smaller than
/// `tolerance * max(rows,cols) * p`, with `p` the largest singular value, will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse( Image const& in, Image& out, dfloat tolerance = 1e-7 );
inline Image PseudoInverse( Image const& in, dfloat tolerance = 1e-7 ) {
   Image out;
   PseudoInverse( in, out, tolerance );
   return out;
}

// \brief Solves the matrix equation `A x = b` for real-valued `A` and `b`.
//DIP_EXPORT void Solve( Image const& A, Image const& b, Image& x )
//       If `A` is a 0D image, compute SVD, and do `SVD.solve(b)`. Otherwise compute SVD for each pixel and do `SVD.solve(b)`.
//       In reality, SVD is only a good choice if `A` is not square. Should we implement other possible options? For
//       example specific cases for `A` is symmetric, diagonal, etc.

/// \brief Computes the "thin" singular value decomposition of the matrix at each pixel in image `in`.
///
/// For an input image `in` with a tensor size of NxP, and with M the smaller of N and P, `out` is a
/// vector image with M elements, corresponding to the singular values, sorted in decreasing order.
///
/// Use `dip::SingularValueDecomposition` if you need the full decomposition.
///
/// This function uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValues( Image const& in, Image& out );
inline Image SingularValues( Image const& in ) {
   Image out;
   SingularValues( in, out );
   return out;
}

/// \brief Computes the "thin" singular value decomposition of the matrix at each pixel in image `in`.
///
/// For an input image `A` with a tensor size of NxP, and with M the smaller of N and P, `S` is a
/// square diagonal MxM matrix, `U` is a NxM matrix, and V is a PxM matrix. These matrices satisfy
/// the relation \f$A = USV^*\f$.
///
/// The (diagonal) elements of `S` are the singular values, sorted in decreasing order.
/// You can use `dip::SingularValues` if you are not interested in computing `U` and `V`.
///
/// This function uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValueDecomposition( Image const& A, Image& U, Image& S, Image& V );

/// \brief Creates an image whose pixels are identity matrices.
///
/// `out` will have the same sizes as `in`, and with a tensor representation of a diagonal matrix
/// with a size concordant to that of the tensor representation of `in`. For example, for an N-vector
/// image, the resulting output matrix image will be NxN. `out` will be of type `dip::DT_SFLOAT`.
inline void Identity( Image const& in, Image& out ) {
   dip::uint telems = std::max( in.TensorColumns(), in.TensorRows() );
   out.ReForge( in.Sizes(), telems, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.Fill( 1.0 );
   out.ReshapeTensorAsDiagonal();
}
inline Image Identity( Image const& in ) {
   Image out;
   Identity( in, out );
   return out;
}


/// \brief Adds all tensor elements, producing a scalar image.
DIP_EXPORT void SumTensorElements( Image const& in, Image& out );
inline Image SumTensorElements( Image const& in ) {
   Image out;
   SumTensorElements( in, out );
   return out;
}

/// \brief Multiplies all tensor elements, producing a scalar image.
DIP_EXPORT void ProductTensorElements( Image const& in, Image& out );
inline Image ProductTensorElements( Image const& in ) {
   Image out;
   ProductTensorElements( in, out );
   return out;
}

/// \brief Determines if all tensor elements are non-zero, producing a binary scalar image.
DIP_EXPORT void AllTensorElements( Image const& in, Image& out );
inline Image AllTensorElements( Image const& in ) {
   Image out;
   AllTensorElements( in, out );
   return out;
}

/// \brief Determines if any tensor element is non-zero, producing a binary scalar image.
DIP_EXPORT void AnyTensorElement( Image const& in, Image& out );
inline Image AnyTensorElement( Image const& in ) {
   Image out;
   AnyTensorElement( in, out );
   return out;
}

/// \brief Takes the maximum tensor element at each pixel, producing a scalar image.
DIP_EXPORT void MaximumTensorElement( Image const& in, Image& out );
inline Image MaximumTensorElement( Image const& in ) {
   Image out;
   MaximumTensorElement( in, out );
   return out;
}

/// \brief Takes the maximum absolute tensor element at each pixel, producing a scalar image. For float and complex images only.
DIP_EXPORT void MaximumAbsTensorElement( Image const& in, Image& out );
inline Image MaximumAbsTensorElement( Image const& in ) {
   Image out;
   MaximumAbsTensorElement( in, out );
   return out;
}

/// \brief Takes the minimum tensor element at each pixel, producing a scalar image.
DIP_EXPORT void MinimumTensorElement( Image const& in, Image& out );
inline Image MinimumTensorElement( Image const& in ) {
   Image out;
   MinimumTensorElement( in, out );
   return out;
}

/// \brief Takes the minimum absolute tensor element at each pixel, producing a scalar image. For float and complex images only.
DIP_EXPORT void MinimumAbsTensorElement( Image const& in, Image& out );
inline Image MinimumAbsTensorElement( Image const& in ) {
   Image out;
   MinimumAbsTensorElement( in, out );
   return out;
}

/// \brief Computes the mean tensor element value at each pixel, producing a scalar image.
DIP_EXPORT void MeanTensorElement( Image const& in, Image& out );
inline Image MeanTensorElement( Image const& in ) {
   Image out;
   MeanTensorElement( in, out );
   return out;
}

/// \brief Computes the geometric mean tensor element value at each pixel, producing a scalar image.
DIP_EXPORT void GeometricMeanTensorElement( Image const& in, Image& out );
inline Image GeometricMeanTensorElement( Image const& in ) {
   Image out;
   GeometricMeanTensorElement( in, out );
   return out;
}

/// \brief Sorts the tensor elements within each pixel from largest to smallest. Works in-place. `out` must be
/// real-valued.
DIP_EXPORT void SortTensorElements( Image& out );

/// \brief Sorts the tensor elements within each pixel by magnitude from largest to smallest. Works in-place.
/// `out` must be of a floating point or complex type.
DIP_EXPORT void SortTensorElementsByMagnitude( Image& out );


/// \}


//
// Functions that combine two source images
//

/// \defgroup math_comparison Comparison operators
/// \ingroup math
/// \brief Monadic and dyadic image comparison operators.
/// \{

/// \brief Compares `in1` to `in2` according to `selector`, and writes `in3` or `in4` to `out` depending on the result.
///
/// In short, this is the operation that is applied sample by sample:
///
/// ```cpp
///     in1 <selector> in2 ? in3 : in4
/// ```
///
/// The string `selector` can be one of: "==", "!=", ">", "<", ">=", "<="
///
/// An alternative (slower) implementation would be:
///
/// ```cpp
///     dip::Image mask = in1 <selector> in2;
///     out = in4.Copy();
///     out.At( mask ) = in3.At( mask );
/// ```
///
/// Note that all input images are singleton-expanded to match in size, so the function can e.g. be used with scalar
/// values for `in3` and `in4`:
///
/// ```cpp
///     dip::Image result = dip::Select( in1, in2, dip::Image{ true }, dip::Image{ false }, "==" );
/// ```
///
/// The above is an (less efficient) implementation of
///
/// ```cpp
///     dip::Image result = in1 == in2;
/// ```
///
/// The output image has the same type as `in3` and `in4`. If these types are different, the output type is given by
///
/// ```cpp
///     dip::DataType::SuggestDyadicOperation( in3.DataType(), in4.DataType() );
/// ```
DIP_EXPORT void Select( Image const& in1, Image const& in2, Image const& in3, Image const& in4, Image& out, String const& selector );
inline Image Select( Image const& in1, Image const& in2, Image const& in3, Image const& in4, String const& selector ) {
   Image out;
   Select( in1, in2, in3, in4, out, selector );
   return out;
}

/// \brief Writes either `in1` or `in2` to `out` depending on the value of `mask`.
///
/// In short, this is the operation that is applied sample by sample:
///
/// ```cpp
///     mask ? in1 : in2
/// ```
///
/// An alternative (slower) implementation would be:
///
/// ```cpp
///     out = in2.Copy();
///     out.At( mask ) = in1.At( mask );
/// ```
///
/// When `out` is the same image as `in1`, the operation becomes similar to (but faster than):
///
/// ```cpp
///     in1.At( !mask ) = in2.At( !mask );
/// ```
///
/// Conversely, when `out` is the same image as `in2`, the operation becomes similar to (but faster than):
///
/// ```cpp
///     in2.At( mask) = in1.At( mask );
/// ```
///
/// The output image has the same type as `in1` and `in2`. If these types are different, the output type is given by
///
/// ```cpp
///     dip::DataType::SuggestDyadicOperation( in1.DataType(), in2.DataType() );
/// ```
DIP_EXPORT void Select( Image const& in1, Image const& in2, Image const& mask, Image& out );
inline Image Select( Image const& in1, Image const& in2, Image const& mask ) {
   Image out;
   Select( in1, in2, mask, out );
   return out;
}

/// \}


} // namespace dip

#endif // DIP_MATH_H
