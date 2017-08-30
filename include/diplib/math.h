/*
 * DIPlib 3.0
 * This file contains declarations for image math functions.
 *
 * (c)2014-2017, Cris Luengo.
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


/// \defgroup math Image math and statistics
/// \brief The image math and statistics functions and operators.


//
// Arithmetic, trigonometric and similar monadic operators
//

#include "diplib/private/monadic_operators.h"

namespace dip {

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
   Supremum( { a, b }, out );
}
inline Image Supremum( Image const& a, Image const& b ) {
   Image out;
   Supremum( { a, b }, out );
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
   Infimum( { a, b }, out );
}
inline Image Infimum( Image const& a, Image const& b ) {
   Image out;
   Infimum( { a, b }, out );
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

// TODO: functions to port:
/*
   dip_AmplitudeModulation (dip_math.h)
   dip_CosinAmplitudeModulation (dip_math.h)
   dip_CosinAmplitudeDemodulation (dip_math.h)
*/

/// \}


//
// Tensor operators
//

/// \defgroup math_tensor Tensor operators
/// \ingroup math
/// \brief Operators specific to tensor images.
/// \{

/// \brief Transposes the tensor image, the data is not copied.
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
/// That is, in 2D it is an angle in the range (-pi/2, pi/2), and in 3D the *phi* component is mapped to
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
/// ```cpp
///     in[ 0 ] == out[ 0 ] * Cos( out[ 1 ] );
///     in[ 1 ] == out[ 0 ] * Sin( out[ 1 ] );
/// ```
/// and in 3D:
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
/// real-valued, then `out` is real-valued, and the eigenvalues are in descending
/// order. Otherwise, `out` is complex-valued, and not sorted in any specific way.
DIP_EXPORT void Eigenvalues( Image const& in, Image& out );
inline Image Eigenvalues( Image const& in ) {
   Image out;
   Eigenvalues( in, out );
   return out;
}

/// \brief Computes the eigenvalues and eigenvectors of the square matrix at each pixel in image `in`.
///
/// The decomposition is such that `in * eigenvectors == `eigenvectors * out`.
/// `eigenvectors` is almost always invertible, in which case one can write
/// `in == eigenvectors * out * Inverse( eigenvectors )`.
///
/// `out` is a diagonal matrix image containing the eigenvalues. If `in` is symmetric and
/// real-valued, then `out` is real-valued, and the eigenvalues are in descending
/// order. Otherwise, `out` is complex-valued, and not sorted in any specific way.
///
/// The eigenvectors are the columns `eigenvectors`. It has the same data type as `out`.
DIP_EXPORT void EigenDecomposition( Image const& in, Image& out, Image& eigenvectors );

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
/// `tolerance * max(rows,cols)` times the largest singular value will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse( Image const& in, Image& out, dfloat tolerance = 1e-7 );
inline Image PseudoInverse( Image const& in ) {
   Image out;
   PseudoInverse( in, out );
   return out;
}

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
/// ```cpp
///     in1 <selector> in2 ? in3 : in4
/// ```
///
/// The string `selector` can be one of: "==", "!=", ">", "<", ">=", "<="
///
/// An alternative (slower) implementation would be:
/// ```cpp
///     dip::Image mask = in1 <selector> in2;
///     out = in4;
///     out.CopyAt( in3.CopyAt( mask ), mask );
/// ```
///
/// Note that all input images are singleton-expanded to match in size, so the function can e.g. be used with scalar
/// values for `in3` and `in4`:
/// ```cpp
///     dip::Image result = dip::Select( in1, in2, dip::Image{ true }, dip::Image{ false }, "==" );
/// ```
/// The above is an (less efficient) implementation of
/// ```cpp
///     dip::Image result = in1 == in2;
/// ```
///
/// The output image has the same type as `in3` and `in4`. If these types are different, the output type is given by
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
/// ```cpp
///     mask ? in1 : in2
/// ```
///
/// An alternative (slower) implementation would be:
/// ```cpp
///     out = in2;
///     out.CopyAt( in1.CopyAt( mask ), mask );
/// ```
///
/// When `out` is the same image as `in1`, the operation becomes similar to:
/// ```cpp
///     in1.CopyAt( in2, !mask );
/// ```
/// Conversely, when `out` is the same image as `in2`, the operation becomes similar to:
/// ```cpp
///     in2.CopyAt( in1, mask );
/// ```
///
/// The output image has the same type as `in1` and `in2`. If these types are different, the output type is given by
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


//
// Functions for grey-value mapping
//

/// \defgroup math_contrast Grey-value mapping operators
/// \ingroup math
/// \brief Operators that map input grey values to a new range.
/// \{

/// \brief Clips the sample values in `in` to a specified range.
///
/// The input values are written unmodified to `out` if they are within the given range, otherwise the closest
/// value within the range is used. Clipping is also known as clamping or thresholding, though in *DIPlib* we
/// use "thresholding" for the process that yields a binary image.
///
/// The output range is given by `low` and `high`. `mode` can be one of the following strings:
///  - `"both"`: any value lower than `low` is set to `low`, and any value higher than `high` is set to `high`.
///  - `"low"`: only the lower bound is enforced, yields same result as setting `high` to infinity.
///  - `"high"`: only the upper bound is enforced, yields same result as setting `low` to infinity.
///  - `"range"`: `low` is interpreted as the middle of the range, and `high` as the length of the range. The
///    output range is given by [`low-high/2`,`low+high/2`].
///
/// `in` must be real-valued.
DIP_EXPORT void Clip(
      Image const& in,
      Image& out,
      dfloat low = 0.0,
      dfloat high = 255.0,
      String const& mode = "both"
);
inline Image Clip(
      Image const& in,
      dfloat low = 0.0,
      dfloat high = 255.0,
      String const& mode = "both"
) {
   Image out;
   Clip( in, out, low, high, mode );
   return out;
}

/// \brief Clips the sample values in `in` to a specified range, using the error function.
///
/// The input values are mapped through the error function. This leads to values in the middle of the range
/// being unaffected, and values larger than `high` asymptotically reaching 1, and values lower than `low`
/// asymptotically reaching 0. This process is also known as soft thresholding, and leads to a quasi-binary
/// image where the slow transition between foreground and background is preserved, thereby avoiding a most
/// of the aliasing that is introduced by binarization (van Vliet, 1993).
///
/// The range to map is given by `low` and `high`. `mode` can be one of the following strings:
///  - `"both"`: any value lower than `low` is set to `low`, and any value higher than `high` is set to `high`.
///  - `"low"`: only the lower bound is enforced, but the value of `high` still affects the mapping.
///  - `"high"`: only the upper bound is enforced, but the value of `low` still affects the mapping.
///  - `"range"`: `low` is interpreted as the middle of the range, and `high` as the length of the range. The
///    input range is given by [`low-high/2`, `low+high/2`]. Note that this is the default mode.
///
/// `in` must be real-valued.
///
/// **Literature**
/// - L.J. van Vliet, "Grey-Scale Measurements in Multi-Dimensional Digitized Images", Ph.D. thesis, Delft University
///   of Technology, The Netherlands, 1993
DIP_EXPORT void ErfClip(
      Image const& in,
      Image& out,
      dfloat low = 128.0,
      dfloat high = 64.0,
      String const& mode = "range"
);
inline Image ErfClip(
      Image const& in,
      dfloat low = 128.0,
      dfloat high = 64.0,
      String const& mode = "range"
) {
   Image out;
   ErfClip( in, out, low, high, mode );
   return out;
}

/// \brief Applies a mapping function according to the input image's range and the given output range.
///
/// The mapping function is defined as follows: sample values higher or equal to `upperBound` are mapped
/// to the `outMax` value, and sample values lower or equal to `lowerBound` are mapped to the `outMin` value.
/// `method` determines how pixel values are mapped in between these limits. Valid strings for `mode` are:
///  - `"linear"`: linear mapping.
///  - `"signed linear"`: linear mapping with zero at fixed value in the middle of the output range.
///  - `"logarithmic"`: logarithmic mapping.
///  - `"signed logarithmic"`: logarithmic mapping with zero at fixed location in the output range.
///  - `"erf"`: error function mapping.
///  - `"decade"`: decade contrast stretch (uses `parameter1`).
///  - `"sigmoid"`: sigmoid function contrast stretch (uses `parameter1` and `parameter2`).
///
/// `in` must be real-valued. `out` will be of an arithmetic type (single or double float), unless it is protected,
/// in which case its data type is preserved (see \ref protect).
///
/// Below follow the equations used for each of the mappings. They all use (with the percentile computed across all
/// samples, not independently for each channel):
/// ```cpp
///     dfloat inMin = dip::Percentile( in, {}, lowerBound ).As< dfloat >();
///     dfloat inMax = dip::Percentile( in, {}, upperBound ).As< dfloat >();
///     in = dip::Clip( in, inMin, inMax );
/// ```
///
/// Next, `"linear"` computes `(( outMax - outMin ) / ( inMax - inMin )) * ( in - inMin ) + outMin`.
///
/// `"signed linear"` computes the same thing, but first sets `inMax = std::max( std::abs(inMax), std::abs(inMin) )`,
/// and `inMin = -inMax`.
///
/// `"logarithmic"` computes:
/// ```cpp
///     dfloat offset = inMin - 1;
///     out = ( outMax - outMin ) * Log( in - offset ) / std::log( inMax - offset ) + outMin;
/// ```
/// whereas `"signed logarithmic"` computes a similar mapping, but first sets
/// `inMax = std::max( std::abs(inMax), std::abs(inMin) )`, and `inMin = -inMax`, and then takes the logarithm
/// of `in+1` for positive `in` or `inMax+in+1` for negative `in`.
///
/// `"erf"` applies a mapping identical to that of the `dip::ErfClip` function with the lower range bound set to
/// `inMin` and the upper one set to `inMax`, and then scales the output to the requested range. Note that in this
/// case, the input is not hard-clipped to the range, but soft-clipped through the error function.
///
/// `"decade"` applies the following mapping to each sample:
/// ```cpp
///     dfloat decade = std::log10(( inMax - inMin ) / ( in - inMin + EPSILON ));
///     if( decade < parameter1 )
///        out = ( outMax - outMin ) * ( 1 + std::floor(decade) - decade ) + outMin;
///     else
///        out = 0;
/// ```
///
/// `"sigmoid"` applies the following mapping to each sample:
/// ```cpp
///     dfloat min = sigmoid( parameter1 * inMin + parameter2 );
///     dfloat max = sigmoid( parameter1 * inMax + parameter2 );
///     out = ( outMax - outMin ) / ( max - min ) * ( sigmoid( parameter1 * in + parameter2 ) - min ) + outMin;
/// ```
/// Here, the `sigmoid` function is `sigmoid(x) = x / ( 1 + std::abs(x) )`. `parameter1` represents the slope and
/// `parameter2` the point around which the sigmoid is centered.
DIP_EXPORT void ContrastStretch(
      Image const& in,
      Image& out,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 100.0,
      dfloat outMin = 0.0,
      dfloat outMax = 255.0,
      String const& method = "linear",
      dfloat parameter1 = 1.0,
      dfloat parameter2 = 0.0
);
inline Image ContrastStretch(
      Image const& in,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 100.0,
      dfloat outMin = 0.0,
      dfloat outMax = 255.0,
      String const& method = "linear",
      dfloat parameter1 = 1.0,
      dfloat parameter2 = 0.0
) {
   Image out;
   ContrastStretch( in, out, lowerBound, upperBound, outMin, outMax, method, parameter1, parameter2 );
   return out;
}


/// \}

// TODO: functions to port:
/*
   dip_RemapOrientation (dip_point.h)
   dip_ChangeByteOrder (dip_manipulation.h)
   dip_SimpleGaussFitImage (dip_numerical.h)
   dip_EmFitGaussians (dip_numerical.h)
   dip_EmGaussTest (dip_numerical.h)
*/

} // namespace dip

#endif // DIP_MATH_H
