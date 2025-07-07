/*
 * (c)2014-2025, Cris Luengo.
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

#include <algorithm>

#include "diplib.h"


/// \file
/// \brief Declares image math functions, except basic arithmetic and comparison.
/// See \ref "diplib/library/operators.h", \ref math.


/// \group math Math and statistics
/// \brief The image math and statistics functions and operators.


namespace dip {


#define DIP_MONADIC_OPERATOR( functionName_ ) \
DIP_NODISCARD inline Image functionName_( Image const& in ) { Image out; functionName_( in, out ); return out; }

#define DIP_MONADIC_OPERATOR_WITH_PARAM( functionName_, paramType_ ) \
DIP_NODISCARD inline Image functionName_( Image const& in, paramType_ param ) { Image out; functionName_( in, out, param ); return out; }

#define DIP_MONADIC_OPERATOR_WITH_DEFAULTED_PARAM( functionName_, paramType_, defaultValue_ ) \
DIP_NODISCARD inline Image functionName_( Image const& in, paramType_ param = ( defaultValue_ )) { Image out; functionName_( in, out, param ); return out; }

#define DIP_DIADIC_OPERATOR( functionName_ ) \
DIP_NODISCARD inline Image functionName_( Image const& in1, Image const& in2 ) { Image out; functionName_( in1, in2, out ); return out; }


//
// Arithmetic, trigonometric and similar monadic operators
//

/// \addtogroup math_arithmetic

/// \brief Flushes [denormal](https://en.wikipedia.org/wiki/Denormal_number) sample values to zero.
/// Denormal floating-point values can slow down computation.
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void FlushToZero( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( FlushToZero )

/// \brief Computes the nearest integer to each sample (rounds).
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void Round( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Round )

/// \brief Computes the smallest integer larger or equal to each sample (rounds up).
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void Ceil( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Ceil )

/// \brief Computes the largest integer smaller or equal to each sample (rounds down).
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void Floor( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Floor )

/// \brief Computes the truncated value of each sample (rounds towards zero).
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void Truncate( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Truncate )

/// \brief Computes the fractional value of each sample (`out = in - dip::Truncate(in)`).
/// Only defined for floating-point types, the output is the same type.
DIP_EXPORT void Fraction( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Fraction )

/// \brief Computes the reciprocal of each sample: `out = in == 0 ? 0 : 1/in`.
DIP_EXPORT void Reciprocal( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Reciprocal )

/// \brief Computes the square of each sample.
DIP_EXPORT void Square( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Square )

/// \brief Computes the square root of each sample.
DIP_EXPORT void Sqrt( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Sqrt )

/// \brief Computes the base e exponent (natural exponential) of each sample.
DIP_EXPORT void Exp( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Exp )

/// \brief Computes the base 2 exponent of each sample.
DIP_EXPORT void Exp2( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Exp2 )

/// \brief Computes the base 10 exponent of each sample.
DIP_EXPORT void Exp10( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Exp10 )

/// \brief Computes the natural logarithm (base e logarithm) of each sample.
DIP_EXPORT void Ln( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Ln )

/// \brief Computes the base 2 logarithm of each sample.
DIP_EXPORT void Log2( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Log2 )

/// \brief Computes the base 10 logarithm of each sample.
DIP_EXPORT void Log10( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Log10 )

/// \endgroup

/// \addtogroup math_trigonometric

/// \brief Computes the sine of each sample.
DIP_EXPORT void Sin( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Sin )

/// \brief Computes the cosine of each sample.
DIP_EXPORT void Cos( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Cos )

/// \brief Computes the tangent of each sample.
DIP_EXPORT void Tan( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Tan )

/// \brief Computes the arc sine of each sample.
DIP_EXPORT void Asin( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Asin )

/// \brief Computes the arc cosine of each sample.
DIP_EXPORT void Acos( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Acos )

/// \brief Computes the arc tangent of each sample.
DIP_EXPORT void Atan( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Atan )

/// \brief Computes the hyperbolic sine of each sample.
DIP_EXPORT void Sinh( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Sinh )

/// \brief Computes the hyperbolic cosine of each sample.
DIP_EXPORT void Cosh( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Cosh )

/// \brief Computes the hyperbolic tangent of each sample.
DIP_EXPORT void Tanh( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Tanh )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 0. Precise up to about 7 digits.
DIP_EXPORT void BesselJ0( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( BesselJ0 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order alpha = 1. Precise up to about 7 digits.
DIP_EXPORT void BesselJ1( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( BesselJ1 )

/// \brief Computes the Bessel functions of the first kind of each sample, of order `alpha`. Precise up to about 7 digits.
DIP_EXPORT void BesselJN( Image const& in, Image& out, dip::uint alpha );
DIP_MONADIC_OPERATOR_WITH_PARAM( BesselJN, dip::uint )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 0. Precise up to about 7 digits.
DIP_EXPORT void BesselY0( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( BesselY0 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order alpha = 1. Precise up to about 7 digits.
DIP_EXPORT void BesselY1( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( BesselY1 )

/// \brief Computes the Bessel functions of the second kind of each sample, of order `alpha`. Precise up to about 7 digits.
DIP_EXPORT void BesselYN( Image const& in, Image& out, dip::uint alpha );
DIP_MONADIC_OPERATOR_WITH_PARAM( BesselYN, dip::uint )

/// \brief Computes the natural logarithm of the gamma function of each sample.
DIP_EXPORT void LnGamma( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( LnGamma )

/// \brief Computes the error function of each sample.
DIP_EXPORT void Erf( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Erf )

/// \brief Computes the complementary error function of each sample.
DIP_EXPORT void Erfc( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Erfc )

/// \brief Computes the sinc function of each sample. $\mathrm{sinc}(x) = \sin(x)/x$.
DIP_EXPORT void Sinc( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Sinc )

/// \endgroup

/// \addtogroup math_comparison

/// \brief True for each pixel that is NaN.
DIP_EXPORT void IsNotANumber( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( IsNotANumber )

/// \brief True for each pixel that is positive or negative infinity.
DIP_EXPORT void IsInfinite( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( IsInfinite )

/// \brief True for each pixel that is not NaN nor infinity.
DIP_EXPORT void IsFinite( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( IsFinite )

/// \endgroup


/// \group math_arithmetic Arithmetic operators
/// \ingroup math
/// \brief Monadic and dyadic image arithmetic operators.
/// \addtogroup

/// \brief Computes the absolute value of each sample.
DIP_EXPORT void Abs( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Abs )

/// \brief Computes the modulus (absolute value) of each sample. `dip::Modulus` is an alias for \ref dip::Abs.
inline void Modulus( Image const& in, Image& out ) { Abs( in, out ); }
DIP_MONADIC_OPERATOR( Modulus )

/// \brief Computes the square of the modulus of each sample.
DIP_EXPORT void SquareModulus( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( SquareModulus )

/// \brief Computes the phase (angle on complex plane, through `std::arg`) of each sample.
DIP_EXPORT void Phase( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Phase )

/// \brief Returns the real component of a complex image. Returns \ref dip::Image::Real if the input is complex.
DIP_NODISCARD inline Image Real( Image const& in ) { return in.DataType().IsComplex() ? Image( in.Real() ) : in; }
inline void Real( Image const& in, Image& out ) { out = Real( in ); }

/// \brief Returns the imaginary component of a complex image. Returns \ref dip::Image::Imaginary if the input is complex
DIP_NODISCARD inline Image Imaginary( Image const& in ) { return in.DataType().IsComplex() ? Image( in.Imaginary() ) : in; }
inline void Imaginary( Image const& in, Image& out ) { out = Imaginary( in ); }

/// \brief Computes the complex conjugate of each sample.
DIP_EXPORT void Conjugate( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Conjugate )

/// \brief Computes the sign of each sample. Only defined for signed real data types (signed integers
/// and floating-point types). Output is of type \ref dip::DT_SINT8, containing values -1, 0 and 1.
DIP_EXPORT void Sign( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Sign )

/// \brief Computes the integer closest to the value of each sample.
/// Only defined for floating-point types, the output is of type \ref dip::DT_SINT32.
DIP_EXPORT void NearestInt( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( NearestInt )

/// \brief Computes the sample-wise supremum (maximum) over all the input images. For binary images, this is the same
/// as the union.
DIP_EXPORT void Supremum( ImageConstRefArray const& in, Image& out );
DIP_NODISCARD inline Image Supremum( ImageConstRefArray const& in ) {
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
DIP_DIADIC_OPERATOR( Supremum )

/// \brief Computes the sample-wise infimum (minimum) over all the input images. For binary images, this is the same
/// as the intersection.
DIP_EXPORT void Infimum( ImageConstRefArray const& in, Image& out );
DIP_NODISCARD inline Image Infimum( ImageConstRefArray const& in ) {
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
DIP_DIADIC_OPERATOR( Infimum )

/// \brief Computes the sample-wise signed infimum (minimum) of the two input images: returns `-b` where `b < a`, a otherwise.
DIP_EXPORT void SignedInfimum( Image const& a, Image const& b, Image& out );
DIP_DIADIC_OPERATOR( SignedInfimum )

/// \brief Computes the linear combination of the two images, sample-wise.
///
/// The actual operation applied is:
/// ```cpp
/// out = a * aWeight + b * bWeight;
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
DIP_NODISCARD inline Image LinearCombination(
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
/// ```cpp
/// out = a * aWeight + b * bWeight;
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
DIP_NODISCARD inline Image LinearCombination(
      Image const& a,
      Image const& b,
      dcomplex aWeight,
      dcomplex bWeight
) {
   Image out;
   LinearCombination( a, b, out, aWeight, bWeight );
   return out;
}

/// \brief Overlays an image over another, using alpha blending.
///
/// `in` and `overlay` are scalar or tensor images of the same size (or can be singleton-expanded to a
/// matching size). `overlay` will be masked on top of `in`, using `alpha` as the alpha mask (also known
/// as matte). `alpha` is a scalar image of the same size as `in`.
///
/// If `in` is scalar and `overlay` is not, then `in` will be replicated across the output tensor elements.
/// If `overlay` is scalar and `in` is not, then `overlay` will be replicated across the tensor elements.
/// If both are tensor images, they must have the same number of tensor elements.
///
/// `out` will be of the same data type as `in`. `overlay` will be cast to that type, rounding and clamping
/// as usual.
///
/// `alpha` is expected to be in the range [0, 1]. Where `alpha` is 1, `overlay` will be opaque,
/// and `in` will not show at all. Where `alpha` is 0, `overlay` will be completely transparent, and `in`
/// will be fully seen. Values of `alpha` outside this range could lead to unexpected results.
/// Note that the alpha values are not supposed to be pre-multiplied.
DIP_EXPORT void AlphaBlend(
      Image const& in,
      Image const& overlay,
      Image const& alpha,
      Image& out
);
DIP_NODISCARD inline Image AlphaBlend(
      Image const& in,
      Image const& overlay,
      Image const& alpha
) {
   Image out;
   AlphaBlend( in, overlay, alpha, out );
   return out;
}

/// \brief Apply the alpha mask `alpha` to the image `in`, using the background color `background`.
///
/// `alpha` is a scalar image of the same size as `in` (or can be singleton-expanded to a
/// matching size). `alpha / scaling` must be in the range [0, 1], if there are values outside of
/// that range, expect strange results.
/// Note that the alpha values are not supposed to be pre-multiplied.
///
/// `out` will be of the same data type as `in`.
inline void AlphaMask(
      Image const& in,
      Image const& alpha,
      Image& out,
      Image::Pixel const& background = { 0 },
      dfloat scaling = 255
) {
   Image base{ background };
   base.Convert( in.DataType() ); // Ensure we preserve the data type of `in` in the output.
   DIP_STACK_TRACE_THIS( AlphaBlend( base, in, alpha / scaling, out ));
}
DIP_NODISCARD inline Image AlphaMask(
      Image const& in,
      Image const& alpha,
      Image::Pixel const& background = { 0 },
      dfloat scaling = 255
) {
   Image out;
   AlphaMask( in, alpha, out, background, scaling );
   return out;
}

/// \endgroup


//
// Arithmetic dyadic operators that are not already declared in diplib/library/operators.h
//

/// \group math_trigonometric Trigonometric operators
/// \ingroup math
/// \brief Monadic and dyadic image trigonometric operators and other complex functions.
/// \addtogroup

/// \brief Computes the four-quadrant arc tangent of `y/x`.
///
/// The operation can be understood as the angle of the vector formed by the two input images.
/// The result is always in the range $[-\pi,\pi]$. The inputs must be a real type.
DIP_EXPORT void Atan2( Image const& y, Image const& x, Image& out );
DIP_DIADIC_OPERATOR( Atan2 )

/// \brief Computes the square root of the sum of the squares of corresponding samples in `a` and `b`.
///
/// The computation is performed carefully, so there is no undue overflow or underflow at intermediate
/// stages of the computation. The inputs must be a real type.
DIP_EXPORT void Hypot( Image const& a, Image const& b, Image& out );
DIP_DIADIC_OPERATOR( Hypot )

/// \endgroup


//
// Tensor operators
//

/// \group math_tensor Tensor operators
/// \ingroup math
/// \brief Operators specific to tensor images.
/// \addtogroup

/// \brief Transposes the tensor image, the data are not copied.
DIP_NODISCARD inline Image Transpose( Image const& in ) {
   Image out = in;
   out.Transpose();
   return out;
}
inline void Transpose( Image const& in, Image& out ) { out = Transpose( in ); }

/// \brief Computes the conjugate transpose of the tensor image `in`.
inline void ConjugateTranspose( Image const& in, Image& out ) {
   Conjugate( in, out );
   out.Transpose();
}
DIP_NODISCARD inline Image ConjugateTranspose( Image const& in ) {
   return Conjugate( in ).Transpose();
}

/// \brief Computes the dot product (inner product) of two vector images.
DIP_EXPORT void DotProduct( Image const& lhs, Image const& rhs, Image& out );
DIP_DIADIC_OPERATOR( DotProduct )

/// \brief Computes the cross product (inner product) of two vector images.
///
/// Input image tensors must be 2-vectors or 3-vectors. For 3-vectors, the cross product is as
/// commonly defined in 3D. For 2-vectors, we define the cross product as the z-component
/// of the cross product of the 3D vectors obtained by adding a 0 z-component to the inputs.
/// That is, it is the area of the parallelogram formed by the two 2D vectors.
DIP_EXPORT void CrossProduct( Image const& lhs, Image const& rhs, Image& out );
DIP_DIADIC_OPERATOR( CrossProduct )

/// \brief Computes the norm of the vector at each pixel in image `in`.
DIP_EXPORT void Norm( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Norm )

/// \brief Computes the square of the norm of the vector at each pixel in image `in`.
DIP_EXPORT void SquareNorm( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( SquareNorm )

/// \brief Computes the angle of the vector at each pixel in image `in`.
///
/// `in` must be a 2-vector or a 3-vector. For a 2-vector, `out` is a scalar image representing
/// *phi*, the angle from the x-axis. For a 3-vector, `out` has 2 tensor components, corresponding
/// to *phi* and *theta*. *phi*, as in the 2D case, is the angle from the x-axis within the x-y plane
/// (azimuth). *theta* is the angle from the z-axis (inclination). See \ref dip::CartesianToPolar for
/// more details. This function yields the same output as \ref dip::CartesianToPolar, but without
/// the first tensor component.
///
/// \see dip::Norm, dip::Orientation, dip::PolarToCartesian, dip::CartesianToPolar
DIP_EXPORT void Angle( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Angle )

/// \brief Computes the orientation of the vector at each pixel in image `in`.
///
/// Orientation is defined as the angle mapped to the half-circle or half-sphere with positive x-coordinate.
/// That is, in 2D it is an angle in the range (-&pi;/2, &pi;/2), and in 3D the *phi* component is mapped to
/// that same range. See \ref dip::Angle for more information.
///
/// \see dip::Norm, dip::Angle
DIP_EXPORT void Orientation( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Orientation )

/// \brief Converts the vector at each pixel in image `in` from Cartesian coordinates to polar
/// (or spherical) coordinates.
///
/// `in` must be a 2-vector or a 3-vector. `out` is a same-size vector containing *r* and *phi*
/// in the 2D case, and *r*, *phi* and *theta* in the 3D case. *phi* is the angle to the x-axis
/// within the x-y plane (azimuth). *theta* is the angle from the z-axis (inclination).
///
/// That is, in 2D:
/// ```cpp
/// in[ 0 ] == out[ 0 ] * Cos( out[ 1 ] );
/// in[ 1 ] == out[ 0 ] * Sin( out[ 1 ] );
/// ```
/// and in 3D:
/// ```cpp
/// in[ 0 ] == out[ 0 ] * Cos( out[ 1 ] ) * Sin( out[ 2 ] );
/// in[ 1 ] == out[ 0 ] * Sin( out[ 1 ] ) * Sin( out[ 2 ] );
/// in[ 2 ] == out[ 0 ] * Cos( out[ 2 ] );
/// ```
///
/// \see dip::PolarToCartesian, dip::Norm, dip::Angle
DIP_EXPORT void CartesianToPolar( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( CartesianToPolar )

/// \brief Converts the vector at each pixel in image `in` from polar (or spherical) coordinates to
/// Cartesian coordinates.
///
/// `in` must be a 2-vector or a 3-vector. See \ref dip::CartesianToPolar for a description of the polar
/// coordinates used.
///
/// \see dip::CartesianToPolar, dip::Norm, dip::Angle
DIP_EXPORT void PolarToCartesian( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( PolarToCartesian )

/// \brief Computes the determinant of the square matrix at each pixel in image `in`.
DIP_EXPORT void Determinant( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Determinant )

/// \brief Computes the trace of the square matrix at each pixel in image `in`.
DIP_EXPORT void Trace( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Trace )

/// \brief Computes the rank of the square matrix at each pixel in image `in`.
/// The output is DT_UINT8, under the assumption that we won't have tensor images with a rank higher than 255.
DIP_EXPORT void Rank( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Rank )

/// \brief Computes the eigenvalues of the square matrix at each pixel in image `in`.
///
/// `out` is a vector image containing the eigenvalues. If `in` is symmetric and
/// real-valued, then `out` is real-valued, otherwise, `out` is complex-valued.
/// The eigenvalues are sorted by magnitude, in descending order.
///
/// `method` is either `"precise"` or `"fast"`. The precise method uses an iterative QR decomposition.
/// The fast method uses a closed-form algorithm that is much faster, but potentially less accurate.
/// This faster algorithm is only used for real-valued, 2x2 or 3x3 symmetric tensor images.
DIP_EXPORT void Eigenvalues( Image const& in, Image& out, String const& method = S::PRECISE );
DIP_MONADIC_OPERATOR_WITH_DEFAULTED_PARAM( Eigenvalues, String const&, S::PRECISE )

/// \brief Finds the largest eigenvalue of the square matrix at each pixel in image `in`.
///
/// Computes the eigenvalues in the same way as \ref dip::Eigenvalues, but
/// outputs only the eigenvalue with the largest magnitude.
/// See the linked function's documentation for a description of the `method` parameter.
DIP_EXPORT void LargestEigenvalue( Image const& in, Image& out, String const& method = S::PRECISE );
DIP_MONADIC_OPERATOR_WITH_DEFAULTED_PARAM( LargestEigenvalue, String const&, S::PRECISE )

/// \brief Finds the smallest eigenvalue of the square matrix at each pixel in image `in`.
///
/// Computes the eigenvalues in the same way as \ref dip::Eigenvalues, but
/// outputs only the eigenvalue with the smallest magnitude.
/// See the linked function's documentation for a description of the `method` parameter.
DIP_EXPORT void SmallestEigenvalue( Image const& in, Image& out, String const& method = S::PRECISE );
DIP_MONADIC_OPERATOR_WITH_DEFAULTED_PARAM( SmallestEigenvalue, String const&, S::PRECISE )

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
/// The eigenvectors are the columns of `eigenvectors`. It has the same data type as `out`.
///
/// `method` is either `"precise"` or `"fast"`. The precise method uses an iterative QR decomposition.
/// The fast method uses a closed-form algorithm that is much faster, but potentially less accurate.
/// This faster algorithm is only used for real-valued, 2x2 or 3x3 symmetric tensor images.
DIP_EXPORT void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors, String const& method = S::PRECISE );

/// \brief Finds the largest eigenvector of the symmetric matrix at each pixel in image `in`.
///
/// Computes the eigen decomposition in the same way as \ref dip::EigenDecomposition, but
/// outputs only the eigenvector that corresponds to the eigenvalue with largest magnitude.
/// Always uses the precise algorithm.
///
/// `in` must be symmetric and real-valued.
// TODO: Rewrite to allow the fast algorithm here as well.
DIP_EXPORT void LargestEigenvector( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( LargestEigenvector )

/// \brief Finds the smallest eigenvector of the symmetric matrix at each pixel in image `in`.
///
/// Computes the eigen decomposition in the same way as \ref dip::EigenDecomposition, but
/// outputs only the eigenvector that corresponds to the eigenvalue with smallest magnitude.
///
/// `in` must be symmetric and real-valued.
// TODO: Rewrite to allow the fast algorithm here as well.
DIP_EXPORT void SmallestEigenvector( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( SmallestEigenvector )

/// \brief Computes the inverse of the square matrix at each pixel in image `in`.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( Inverse )

/// \brief Computes the pseudo-inverse of the matrix at each pixel in image `in`.
///
/// Computes the Moore-Penrose pseudo-inverse using `tolerance`. Singular values smaller than
/// `tolerance * max(rows,cols) * p`, with `p` the largest singular value, will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse( Image const& in, Image& out, dfloat tolerance = 1e-7 );
DIP_NODISCARD inline Image PseudoInverse( Image const& in, dfloat tolerance = 1e-7 ) {
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
/// Use \ref dip::SingularValueDecomposition if you need the full decomposition.
///
/// This function uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValues( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( SingularValues )

/// \brief Computes the "thin" singular value decomposition of the matrix at each pixel in image `in`.
///
/// For an input image `A` with a tensor size of NxP, and with M the smaller of N and P, `S` is a
/// square diagonal MxM matrix, `U` is a NxM matrix, and V is a PxM matrix. These matrices satisfy
/// the relation $A = USV^*$.
///
/// The (diagonal) elements of `S` are the singular values, sorted in decreasing order.
/// You can use \ref dip::SingularValues if you are not interested in computing `U` and `V`.
///
/// This function uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValueDecomposition( Image const& A, Image& U, Image& S, Image& V );

/// \brief Creates an image whose pixels are identity matrices.
///
/// `out` will have the same sizes as `in`, and with a tensor representation of a diagonal matrix
/// with a size concordant to that of the tensor representation of `in`. For example, for an N-vector
/// image, the resulting output matrix image will be NxN. `out` will be of type \ref dip::DT_SFLOAT.
inline void Identity( Image const& in, Image& out ) {
   dip::uint telems = std::max( in.TensorColumns(), in.TensorRows() );
   out.ReForge( in.Sizes(), telems, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.Fill( 1.0 );
   out.ReshapeTensorAsDiagonal();
}
DIP_MONADIC_OPERATOR( Identity )

/// \brief Adds all tensor elements, producing a scalar image.
DIP_EXPORT void SumTensorElements( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( SumTensorElements )

/// \brief Multiplies all tensor elements, producing a scalar image.
DIP_EXPORT void ProductTensorElements( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( ProductTensorElements )

/// \brief Determines if all tensor elements are non-zero, producing a binary scalar image.
DIP_EXPORT void AllTensorElements( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( AllTensorElements )

/// \brief Determines if any tensor element is non-zero, producing a binary scalar image.
DIP_EXPORT void AnyTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( AnyTensorElement )

/// \brief Takes the maximum tensor element at each pixel, producing a scalar image.
DIP_EXPORT void MaximumTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( MaximumTensorElement )

/// \brief Takes the maximum absolute tensor element at each pixel, producing a scalar image. For float and complex images only.
DIP_EXPORT void MaximumAbsTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( MaximumAbsTensorElement )

/// \brief Takes the minimum tensor element at each pixel, producing a scalar image.
DIP_EXPORT void MinimumTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( MinimumTensorElement )

/// \brief Takes the minimum absolute tensor element at each pixel, producing a scalar image. For float and complex images only.
DIP_EXPORT void MinimumAbsTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( MinimumAbsTensorElement )

/// \brief Computes the mean tensor element value at each pixel, producing a scalar image.
DIP_EXPORT void MeanTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( MeanTensorElement )

/// \brief Computes the geometric mean tensor element value at each pixel, producing a scalar image.
DIP_EXPORT void GeometricMeanTensorElement( Image const& in, Image& out );
DIP_MONADIC_OPERATOR( GeometricMeanTensorElement )

/// \brief Sorts the tensor elements within each pixel from largest to smallest. Works in-place. `out` must be
/// real-valued.
DIP_EXPORT void SortTensorElements( Image& out );

/// \brief Sorts the tensor elements within each pixel by magnitude from largest to smallest. Works in-place.
/// `out` must be of a floating point or complex type.
DIP_EXPORT void SortTensorElementsByMagnitude( Image& out );


/// \endgroup


//
// Functions that combine two source images
//

/// \group math_comparison Comparison operators
/// \ingroup math
/// \brief Monadic and dyadic image comparison operators.
/// \addtogroup

/// \brief Compares `in1` to `in2` according to `selector`, and writes `in3` or `in4` to `out` depending on the result.
///
/// In short, this is the operation that is applied sample by sample:
/// ```cpp
/// in1 <selector> in2 ? in3 : in4
/// ```
///
/// The string `selector` can be one of: "==", "!=", ">", "<", ">=", "<="
///
/// An alternative (slower) implementation would be:
/// ```cpp
/// dip::Image mask = in1 <selector> in2;
/// out = in4.Copy();
/// out.At( mask ) = in3.At( mask );
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
/// ```cpp
/// dip::Image result = in1 == in2;
/// ```
///
/// The output image has the same type as `in3` and `in4`. If these types are different, the output type is given by
/// ```cpp
/// dip::DataType::SuggestDyadicOperation( in3.DataType(), in4.DataType() );
/// ```
DIP_EXPORT void Select( Image const& in1, Image const& in2, Image const& in3, Image const& in4, Image& out, String const& selector );
DIP_NODISCARD inline Image Select( Image const& in1, Image const& in2, Image const& in3, Image const& in4, String const& selector ) {
   Image out;
   Select( in1, in2, in3, in4, out, selector );
   return out;
}

/// \brief Writes either `in1` or `in2` to `out` depending on the value of `mask`.
///
/// In short, this is the operation that is applied sample by sample:
/// ```cpp
/// mask ? in1 : in2
/// ```
///
/// An alternative (slower) implementation would be:
/// ```cpp
/// out = in2.Copy();
/// out.At( mask ) = in1.At( mask );
/// ```
///
/// When `out` is the same image as `in1`, the operation becomes similar to (but faster than):
/// ```cpp
/// in1.At( !mask ) = in2.At( !mask );
/// ```
///
/// Conversely, when `out` is the same image as `in2`, the operation becomes similar to (but faster than):
/// ```cpp
/// in2.At( mask ) = in1.At( mask );
/// ```
///
/// The output image has the same type as `in1` and `in2`. If these types are different, the output type is given by
/// ```cpp
/// dip::DataType::SuggestDyadicOperation( in1.DataType(), in2.DataType() );
/// ```
DIP_EXPORT void Select( Image const& in1, Image const& in2, Image const& mask, Image& out );
DIP_NODISCARD inline Image Select( Image const& in1, Image const& in2, Image const& mask ) {
   Image out;
   Select( in1, in2, mask, out );
   return out;
}

/// \brief Writes to `out` whichever of `in1` or `in2` is closest to `in`.
///
/// Each pixel in `out` will contain the corresponding value in `in1` or `in2`, whichever is closer to the value of `in`.
///
/// An alternative, slower implementation would be:
/// ```cpp
/// mask = dip::Abs( in - in1 ) < dip::Abs( in - in2 );
/// dip::Select( in1, in2, mask, out );
/// ```
///
/// The output image has the same type as `in1` and `in2`. If these types are different, the output type is given by
/// ```cpp
/// dataType = dip::DataType::SuggestDyadicOperation( in1.DataType(), in2.DataType() );
/// ```
DIP_EXPORT void Toggle( Image const& in, Image const& in1, Image const& in2, Image& out );
DIP_NODISCARD inline Image Toggle( Image const& in, Image const& in1, Image const& in2 ) {
   Image out;
   Toggle( in, in1, in2, out );
   return out;
}

/// \endgroup

#undef DIP_MONADIC_OPERATOR
#undef DIP_MONADIC_OPERATOR_WITH_PARAM
#undef DIP_DIADIC_OPERATOR

} // namespace dip

#endif // DIP_MATH_H
