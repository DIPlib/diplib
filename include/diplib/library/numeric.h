/*
 * (c)2015-2022, Cris Luengo.
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


#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include <cmath>
#include <complex>
#include <functional>
#include <limits>
#include <numeric>
#include <type_traits>
#include <vector>

#include "diplib/library/export.h"
#include "diplib/library/types.h"
#include "diplib/library/sample_iterator.h"


/// \file
/// \brief Numeric algorithms and constants unrelated to images. This file is always included through \ref "diplib.h".
/// See \ref numeric.


namespace dip {


/// \group numeric Numeric algorithms and constants
/// \ingroup infrastructure
/// \brief Functions and constants to be used in numeric computation, unrelated to images.
/// \addtogroup

/// \brief The constant &pi;.
constexpr dfloat pi = 3.14159265358979323846264338327950288;
// std::acos( -1 ) is the good way of defining pi, but it's not constexpr.

/// \brief A NaN value.
constexpr dfloat nan = std::numeric_limits< dfloat >::quiet_NaN();

/// \brief Infinity.
constexpr dfloat infinity = std::numeric_limits< dfloat >::infinity();

/// \brief Maximum meaningful truncation value for a Gaussian. Larger truncation values will lead to differences
/// of more than one machine epsilon between the middle and the ends of the Gaussian. `T` must be a floating-point type.
template< typename T >
constexpr dfloat maximum_gauss_truncation() {
   // Largest x/s for which `exp(-0.5 x^2 / s^2) >= eps`:
   //                            -0.5 x^2 / s^2  >=    ln(eps)
   //                                  -(x/s)^2  >=  2 ln(eps)
   //                                   (x/s)^2  <= -2 ln(eps)
   //                                    x/s     <= sqrt(-2 ln(eps))
   return std::sqrt( -2.0 * std::log( static_cast< dfloat >( std::numeric_limits< T >::epsilon() )));
}

/// \brief Compute the greatest common denominator of two positive integers.
// `std::gcd` will be available in C++17.
constexpr inline dip::uint gcd( dip::uint a, dip::uint b ) {
   return b == 0 ? a : gcd( b, a % b );
}

/// \brief Integer division, unsigned, return ceil.
template< typename T, std::enable_if_t< std::is_integral< T >::value && !std::is_signed< T >::value, int > = 0 >
constexpr T div_ceil( T lhs, T rhs ) {
   if(( lhs == 0 ) || ( rhs == 0 )) {
      return 0;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, signed, return ceil. If signs differ, adding or subtracting one from `lhs` should not overflow.
template< typename T, std::enable_if_t< std::is_integral< T >::value && std::is_signed< T >::value, int > = 0 >
constexpr T div_ceil( T lhs, T rhs ) {
   if(( lhs == 0 ) || ( rhs == 0 )) {
      return 0;
   }
   if(( lhs ^ rhs ) < 0 ) {
      return lhs / rhs;
   }
   if( lhs < 0 ) {
      return ( lhs + 1 ) / rhs + 1;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, unsigned, return floor.
template< typename T, std::enable_if_t< std::is_integral< T >::value && !std::is_signed< T >::value, int > = 0 >
constexpr T div_floor( T lhs, T rhs ) {
   if(( lhs == 0 ) || ( rhs == 0 )) {
      return 0;
   }
   return lhs / rhs;
}

/// \brief Integer division, signed, return floor. If signs differ, adding or subtracting one from `lhs` should not overflow.
template< typename T, std::enable_if_t< std::is_integral< T >::value && std::is_signed< T >::value, int > = 0 >
constexpr T div_floor( T lhs, T rhs ) {
   if(( lhs == 0 ) || ( rhs == 0 )) {
      return 0;
   }
   if(( lhs ^ rhs ) < 0 ) {
      if( lhs < 0 ) {
         return ( lhs + 1 ) / rhs - 1;
      }
      return ( lhs - 1 ) / rhs - 1;
   }
   return lhs / rhs;
}

/// \brief Integer division, return rounded.
template< typename T, typename = std::enable_if_t< std::is_integral< T >::value, T >>
constexpr T div_round( T lhs, T rhs ) {
   // Adapted from: https://stackoverflow.com/a/60009773/7328782
   return ( lhs < 0 ) != ( rhs < 0 ) ? (( lhs - rhs / 2 ) / rhs )
                                     : (( lhs + rhs / 2 ) / rhs );
}

/// \brief Integer modulo, result is always positive, as opposed to % operator.
constexpr inline dip::uint modulo( dip::uint value, dip::uint period ) {
   return value % period;
}

/// \brief Integer modulo, result is always positive, as opposed to % operator.
constexpr inline dip::sint modulo( dip::sint value, dip::sint period ) {
   return ( value < 0 ) ? ( period - ( -value % period )) : ( value % period );
}

/// \brief Fast floor operation, without checks, returning a \ref dip::sint.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
constexpr dip::sint floor_cast( T v ) {
   auto w = static_cast< dip::sint >( v );
   return w - ( v < static_cast< T >( w ));
}

/// \brief Fast ceil operation, without checks, returning a \ref dip::sint.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
constexpr dip::sint ceil_cast( T v ) {
   auto w = static_cast< dip::sint >( v );
   return w + ( v > static_cast< T >( w ));
}

/// \brief Fast round operation, without checks, returning a \ref dip::sint.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
dip::sint round_cast( T v ) {
   return floor_cast( v + 0.5 );
}

/// \brief Consistent rounding, without checks, returning a \ref dip::sint.
///
/// This rounding is consistent in that half-way cases are rounded in the same direction for positive and negative
/// values. The `inverse` template parameter indicates the direction for these cases. By default, it matches
/// `std::round` for positive values.
template< typename T, bool inverse = false, typename = std::enable_if_t< std::is_floating_point< T >::value >>
constexpr dip::sint consistent_round( T v ) {
   return inverse ? ceil_cast( v - 0.5 ) : floor_cast( v + 0.5 ); // conditional should be optimized out
}

/// \brief `constexpr` version of `std::abs`. Prefer `std::abs` outside of `constexpr` functions.
template< typename T >
constexpr T abs( T value ) {
   return value >= 0 ? value : -value;
}

/// \brief Clamps a value between a min and max value (a.k.a. clip, saturate, etc.).
// `std::clamp` will be available in C++17.
template< typename T >
constexpr inline const T& clamp( const T& v, const T& lo, const T& hi ) {
   return std::min( std::max( v, lo ), hi );
}

/// \brief Computes integer powers of 10, assuming the power is relatively small.
constexpr inline dfloat pow10( dip::sint power ) {
   switch( power ) {
      case -6: return 1e-6;
      case -5: return 1e-5;
      case -4: return 1e-4;
      case -3: return 1e-3;
      case -2: return 1e-2;
      case -1: return 1e-1;
      case  0: return 1;
      case  1: return 1e1;
      case  2: return 1e2;
      case  3: return 1e3;
      case  4: return 1e4;
      case  5: return 1e5;
      case  6: return 1e6;
      default:
         if( power > 0 ) {
            return 1e6 * pow10( power - 6 );
         } else {
            return 1e-6 * pow10( power + 6 );
         }
   }
}

/// \brief Approximate floating-point equality: `abs(lhs-rhs)/lhs <= tolerance`.
constexpr inline bool ApproximatelyEquals( dfloat lhs, dfloat rhs, dfloat tolerance = 1e-6 ) {
   return tolerance == 0.0 ? lhs == rhs : ( lhs == 0.0
                                            ? dip::abs( rhs ) <= tolerance
                                            : dip::abs( lhs - rhs ) / lhs <= tolerance );
}

/// \brief Counts the length of a (UTF-8 encoded) Unicode string.
inline dip::uint LengthUnicode( String const& string ) {
#ifdef DIP_CONFIG_ENABLE_UNICODE
   dip::uint len = 0;
   for( auto& s : string ) {
      len += static_cast< dip::uint >(( s & 0xc0 ) != 0x80 );
   }
   return len;
#else
   return string.length();
#endif
}

/// \brief Computes the Bessel function J of the order 0 (with around 7 digits of precision).
DIP_EXPORT dfloat BesselJ0( dfloat x );

/// \brief Computes the Bessel function J of the order 1 (with around 7 digits of precision).
DIP_EXPORT dfloat BesselJ1( dfloat x );

/// \brief Computes the Bessel function J of the order `n` (with around 7 digits of precision).
DIP_EXPORT dfloat BesselJN( dfloat x, dip::uint n );

/// \brief Computes the Bessel function Y of the order 0 (with around 7 digits of precision).
DIP_EXPORT dfloat BesselY0( dfloat x );

/// \brief Computes the Bessel function Y of the order 1 (with around 7 digits of precision).
DIP_EXPORT dfloat BesselY1( dfloat x );

/// \brief Computes the Bessel function Y of the order `n` (with around 7 digits of precision).
DIP_EXPORT dfloat BesselYN( dfloat x, dip::uint n );

/// \brief Computes the sinc function.
inline dfloat Sinc( dfloat x ) {
   return x == 0.0 ? 1.0 : std::sin( x ) / x;
}

/// \brief Computes phi, the integral of the PDF of a Normal distribution with
/// unit variance and zero mean from minus infinity to `x`.
inline dfloat Phi( dfloat x ) {
   return 0.5 * ( 1.0 + std::erf( x * std::sqrt( 2.0 )));
}

/// \brief Computes phi, the integral of the PDF of a Normal distribution with
/// standard deviation `s` and mean `m` from minus infinity to `x`.
inline dfloat Phi( dfloat x, dfloat m, dfloat s ) {
   return Phi(( x - m ) / s );
}

/// \brief Computes the surface area of an `n`-dimensional hypersphere with radius `r`.
constexpr inline dfloat HypersphereSurface( dip::uint n, dfloat r ) {
   // See https://en.wikipedia.org/wiki/N-sphere#Recurrences
   // (but note that we're calculating S_{n-1}, not S_n)
   // We're using the recursive definition because `n` is always small, and Gamma functions would be way more expensive to compute.
   switch( n ) {
      case 0:
         return 0;
      case 1:
         return 2;
      case 2:
         return 2 * pi * r;
      case 3:
         return pi * r * r; // This case is not necessary, but it saves a function call.
      default:
         n -= 2;
         return 2 * pi * r * r / static_cast< dfloat >( n ) * HypersphereSurface( n, r );
   }
}

/// \brief Computes the volume of an `n`-dimensional hypersphere with radius `r`.
constexpr inline dfloat HypersphereVolume( dip::uint n, dfloat r ) {
   // For simplicity, we base this one on the surface area.
   return HypersphereSurface( n, r ) * r / static_cast< dfloat >( n );
}

/// \brief Finds the eigenvalues and eigenvectors of a symmetric, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order; only the lower triangle will be used.
///
/// `lambdas` is a pointer to space for `n` values, which will be written sorted by magnitude, largest to smallest.
///
/// `vectors` is a pointer to space for `n*n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
/// If `vectors` is `nullptr`, no eigenvectors are computed.
DIP_EXPORT void SymmetricEigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors = nullptr
);

/// \brief Finds the eigenvalues and eigenvectors of a 2x2 symmetric, real-valued matrix.
DIP_EXPORT void SymmetricEigenDecomposition2(
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors = nullptr
);

/// \brief Finds the eigenvalues and eigenvectors of a 3x3 symmetric, real-valued matrix.
DIP_EXPORT void SymmetricEigenDecomposition3(
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors = nullptr
);

/// \brief Finds the largest eigenvector of a symmetric, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order; only the lower triangle will be used.
///
/// `vector` is a pointer to space for `n` values, and will receive the eigenvector corresponding to the
/// largest eigenvalue by magnitude. The full decomposition as in \ref dip::SymmetricEigenDecomposition is computed,
/// but only one eigenvector is written to the output.
void LargestEigenvector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
);

/// \brief Finds the smallest eigenvector of a symmetric, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order; only the lower triangle will be used.
///
/// `vector` is a pointer to space for `n` values, and will receive the eigenvector corresponding to the
/// smallest eigenvalue by magnitude. The full decomposition as in \ref dip::SymmetricEigenDecomposition is computed,
/// but only one eigenvector is written to the output.
void SmallestEigenvector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
);

/// \brief Finds the eigenvalues and eigenvectors of a symmetric, real-valued matrix, where only the unique values are given.
///
/// Calls \ref dip::SymmetricEigenDecomposition after copying over the input values to a temporary buffer.
///
/// `input` is a pointer to `n*(n+1)/2` values, stored in the same order as symmetric tensors are stored in an image
/// (see dip::Tensor::Shape). That is, fist are the main diagonal elements, then the elements above the diagonal,
/// column-wise. This translates to:
///
/// - 2D: xx, yy, xy
/// - 3D: xx, yy, zz, xy, xz, yz
/// - 4D: xx, yy, zz, tt, xy, xz, yz, xt, yt, zt
/// - etc.
///
/// See \ref dip::SymmetricEigenDecomposition for information on `lambdas` and `vectors`.
inline void SymmetricEigenDecompositionPacked(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > lambdas,
      SampleIterator< dfloat > vectors = nullptr
) {
   FloatArray matrix( n * n );
   // Copy over diagonal elements, which are stored sequentially at the beginning of `input`.
   for( dip::uint ii = 0, kk = 0; ii < n; ++ii, kk += 1 + n ) {
      matrix[ kk ] = *input;
      ++input;
   }
   // Copy over remaining elements, but copy to the lower triangular elements of `matrix`.
   // The upper triangular elements won't be used.
   for( dip::uint ii = 1; ii < n; ++ii ) {
      for( dip::uint jj = 0; jj < ii; ++jj ) {
         matrix[ ii + jj * n ] = *input;
         ++input;
      }
   }
   SymmetricEigenDecomposition( n, matrix.data(), lambdas, vectors );
}

/// \brief Finds the eigenvalues and eigenvectors of a square, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
///
/// `lambdas` is a pointer to space for `n` values, sorted by magnitude, largest to smallest
///
/// `vectors` is a pointer to space for `n*n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
/// If `vectors` is `nullptr`, no eigenvectors are computed.
DIP_EXPORT void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors = nullptr
);

/// \brief Finds the eigenvalues and eigenvectors of a square, complex-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
///
/// `lambdas` is a pointer to space for `n` values, sorted by magnitude, largest to smallest
///
/// `vectors` is a pointer to space for `n*n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
/// If `vectors` is `nullptr`, no eigenvectors are computed.
DIP_EXPORT void EigenDecomposition(
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > lambdas,
      SampleIterator< dcomplex > vectors = nullptr
);

/// \brief Computes the sum of the values of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline T Sum( dip::uint n, ConstSampleIterator< T > input ) {
   return std::accumulate( input, input + n, T( 0 ));
}

/// \brief Computes the sum of the square of the values of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline FloatType< T > SumAbsSquare( dip::uint n, ConstSampleIterator< T > input ) {
   return std::accumulate( input, input + n, FloatType< T >( 0 ), []( FloatType< T > a, T b ){ return a + b * b; } );
}
template< typename T >
inline T SumAbsSquare( dip::uint n, ConstSampleIterator< std::complex< T >> input ) {
   return std::accumulate( input, input + n, T( 0 ), []( T a, std::complex< T > b ){ return a + ( b * std::conj( b )).real(); } );
}

/// \brief Computes the product of the values of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline T Product( dip::uint n, ConstSampleIterator< T > input ) {
   return std::accumulate( input, input + n, T( 1 ), std::multiplies< T >() );
}

/// \brief Computes the norm of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline FloatType< T > Norm( dip::uint n, ConstSampleIterator< T > input ) {
   return std::sqrt( SumAbsSquare( n, input ));
}

/// \brief Computes the square norm of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline FloatType< T > SquareNorm( dip::uint n, ConstSampleIterator< T > input ) {
   return SumAbsSquare( n, input );
}

/// \brief Computes the determinant of a square, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
DIP_EXPORT dfloat Determinant( dip::uint n, ConstSampleIterator< dfloat > input );

/// \brief Computes the determinant of a square, complex-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
DIP_EXPORT dcomplex Determinant( dip::uint n, ConstSampleIterator< dcomplex > input );

/// \brief Computes the determinant of a diagonal matrix.
///
/// `input` is a pointer to `n` values, representing the matrix's main diagonal.
template< typename T >
inline T DeterminantDiagonal( dip::uint n, ConstSampleIterator< T > input ) {
   return std::accumulate( input, input + n, T( 1.0 ), std::multiplies< T >() );
}

/// \brief Computes the trace of a square matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
template< typename T >
inline T Trace( dip::uint n, ConstSampleIterator< T > input ) {
   return Sum( n, ConstSampleIterator< T >( input.Pointer(), input.Stride() * static_cast< dip::sint >( n + 1 )));
}

/// \brief Computes the trace of a diagonal matrix.
///
/// `input` is a pointer to `n` values, representing the matrix's main diagonal.
template< typename T >
inline T TraceDiagonal( dip::uint n, ConstSampleIterator< T > input ) {
   return Sum( n, input );
}

/// \brief Computes the "thin" singular value decomposition of a real-valued matrix
///
/// `input` is a pointer to `m*n` values, in column-major order.
///
/// `output` is a pointer to `p` values, where `p = std::min( m, n )`. It contains the
/// singular values of `input`, sorted in decreasing order.
///
/// `U` and `V` are pointers to `m*p` and `n*p` values, respectively. The left and right
/// singular vectors will be written to then.
/// If either of them is `nullptr`, neither is computed, and only `output` is filled.
///
/// `SingularValueDecomposition` uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > output,
      SampleIterator< dfloat > U = nullptr,
      SampleIterator< dfloat > V = nullptr
);

/// \brief Computes the "thin" singular value decomposition of a complex-valued matrix
///
/// `input` is a pointer to `m*n` values, in column-major order.
///
/// `output` is a pointer to `p` values, where `p = std::min( m, n )`. It contains the
/// singular values of `input`, sorted in decreasing order.
///
/// `U` and `V` are pointers to `m*p` and `n*p` values, respectively. The left and right
/// singular vectors will be written to then.
/// If either of them is `nullptr`, neither is computed, and only `output` is filled.
///
/// `SingularValueDecomposition` uses the two-sided Jacobi SVD decomposition algorithm.
/// This is efficient for small matrices only.
DIP_EXPORT void SingularValueDecomposition(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > output,
      SampleIterator< dcomplex > U = nullptr,
      SampleIterator< dcomplex > V = nullptr
);

/// \brief Computes the inverse of a square, real-valued matrix.
///
/// `input` and `output` are pointers to `n*n` values, in column-major order.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( dip::uint n, ConstSampleIterator< dfloat > input, SampleIterator< dfloat > output );

/// \brief Computes the inverse of a square, complex-valued matrix.
///
/// `input` and `output` are pointers to `n*n` values, in column-major order.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( dip::uint n, ConstSampleIterator< dcomplex > input, SampleIterator< dcomplex > output );

/// \brief Computes the Moore-Penrose pseudo-inverse of a real-valued matrix, using the Jacobi SVD decomposition.
///
/// `input` is a pointer to `m*n` values, in column-major order.
///
/// `output` is a pointer to `n*m` values, in column-major order.
///
/// `tolerance` is an appropriate tolerance. Singular values smaller than `tolerance * max(n,m) * p`, with `p`
/// the largest singular value, will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > output,
      dfloat tolerance = 1e-7
);

/// \brief Computes the Moore-Penrose pseudo-inverse of a complex-valued matrix, using the Jacobi SVD decomposition.
///
/// `input` and `output` are pointers to `m*n` values, in column-major order.
///
/// `output` is a pointer to `n*m` values, in column-major order.
///
/// `tolerance` is an appropriate tolerance. Singular values smaller than `tolerance * max(n,m) * p`, with `p`
/// the largest singular value, will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > output,
      dfloat tolerance = 1e-7
);

/// \brief Computes the rank of a real-valued matrix.
///
/// `input` is a pointer to `m*n` values, in column-major order.
DIP_EXPORT dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dfloat > input );

/// \brief Computes the rank of a complex-valued matrix.
///
/// `input` is a pointer to `m*n` values, in column-major order.
DIP_EXPORT dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dcomplex > input );

/// \brief Solves a system of real-valued equations, using the Jacobi SVD decomposition.
///
/// Solves $A x = b$, where `A` is an `m`x`n` matrix (stored in column-major order),
/// and `b` is a vector with `m` values.
/// The unknown `x` will have `n` values, and will be written to `output`.
DIP_EXPORT void Solve(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > A,
      ConstSampleIterator< dfloat > b,
      SampleIterator< dfloat > output
);

/// \brief Fits a thin plate spline function to a set of points. Useful for interpolation of scattered points.
///
/// This class implements a generalization of the 2D thin plate spline to arbitrary dimensionality. The
/// constructor builds the thin plate spline function, and the `Evaluate` method evaluates this function
/// at a given point.
///
/// !!! literature
///     - J. Elonen, "Thin Plate Spline editor - an example program in C++", <https://elonen.iki.fi/code/tpsdemo/index.html>
///       (last retrieved June 25, 2019). The page explains the math behind 2D thin plate splines, we did not use
///       the source code linked from that page.
class DIP_NO_EXPORT ThinPlateSpline {
   public:
      /// \brief Creates a function that maps `coordinate` to `value`.
      ///
      /// `coordinate` gives the spatial location of samples, and `value` their values. The thin
      /// plate spline function therefore maps `coordinate` to `value`. `value[ ii ]` corresponds to
      /// `coordinate[ ii ]`, both arrays must have the same number of elements.
      ///
      /// This function also expects `coordinate` and `value` to have the same dimensionality. That is,
      /// they both represent spatial coordinates in the same image. Internally, the function maps
      /// `coordinate` to `value - coordinate`, `Evaluate( pt )` adds `pt` to the interpolated
      /// value at `pt`, and thus it looks externally as if the function maps to `value` rather than
      /// `value-coordinate`. This improves extrapolation.
      ///
      /// To use this class for warping, set `coordinate` to control points in the fixed image, and `value`
      /// to corresponding points in the floating image. Then, for each point in the fixed image,
      /// `Evaluate` will return the corresponding coordinates in the floating image, which can be
      /// used to sample it.
      ///
      /// If `lambda` is larger than 0, the mapping is not exact, leading to a smoother function.
      ///
      /// When applied with a large coordinates array, this constructor can take quite a long time. It is
      /// worth while to split up such a problem into blocks.
      ///
      /// The first input argument, `coordinate`, is taken by value and stored inside the object. Use
      /// `std::move` to pass this argument if you no longer use it later.
      DIP_EXPORT ThinPlateSpline(
            FloatCoordinateArray coordinate,    // use std::move if you can!
            FloatCoordinateArray const& value,  // correspondence points
            dfloat lambda = 0
      );

      /// \brief Evaluates the thin plate spline function at point `pt`.
      DIP_EXPORT FloatArray Evaluate( FloatArray const& pt );

   private:
      std::vector< dfloat > x_;
      FloatCoordinateArray c_;
};

namespace Option {

/// \brief Select if the operation is periodic or not. Used in \ref dip::GaussianMixtureModel.
enum class DIP_NO_EXPORT Periodicity : uint8 {
      NOT_PERIODIC,  ///< The operation is not periodic
      PERIODIC       ///< The operation is periodic, left and right ends of the data are contiguous
};

} // namespace Option

/// \brief Parameters defining a 1D Gaussian. Returned by \ref dip::GaussianMixtureModel.
struct GaussianParameters {
   dfloat position;  ///< The location of the origin, in pixels
   dfloat amplitude; ///< The amplitude (value at the origin)
   dfloat sigma;     ///< The sigma (width)
};

/// \brief Determines the parameters for a Gaussian Mixture Model.
///
/// `data` is an iterator (or pointer) to the first of `size` samples of a GMM (not random samples drawn
/// from such a distribution, but rather samples of a function representing the distribution).
/// `numberOfGaussians` Gaussians will be fitted to it using the Expectation Maximization (EM) procedure.
///
/// The parameters are initialized deterministically, the means are distributed equally over the domain,
/// the sigma are all set to the distance between means, and the amplitude are set to 1.
///
/// `responsibilities` optionally points to a buffer of size `size * numberOfGaussians` that will be used
/// internally. If set to `nullptr` or a default-initialized iterator, a buffer will be allocated internally.
/// Use this parameter when repeatedly calling this function to avoid memory allocations.
///
/// `maxIter` sets how many iterations are run. There is currently no other stopping criterion.
/// `periodicity` determines if the data is considered periodic or not.
///
/// The output is sorted by amplitude, most important component first.
DIP_EXPORT std::vector< GaussianParameters > GaussianMixtureModel(
      ConstSampleIterator< dfloat > data,
      SampleIterator< dfloat > responsibilities,
      dip::uint size,
      dip::uint numberOfGaussians,
      dip::uint maxIter = 20,
      Option::Periodicity periodicity = Option::Periodicity::NOT_PERIODIC
);

/// \brief Computes the rank (index into array) for a given percentile and an array of length `n`.
///
/// The rank is symmetric (i.e. if the 5^th^ percentile translates to rank 14, then the 95^th^ percentile
/// translates to rank n-1-14).
///
/// `percentile` is clamped to the range [0, 100], no error is produced for a percentile outside the valid range.
constexpr inline dip::uint RankFromPercentile( dfloat percentile, dip::uint n ) {
   DIP_THROW_IF( n < 1, E::PARAMETER_OUT_OF_RANGE );
   if ( percentile > 50.0 ) {
      return n - 1 - RankFromPercentile( 100.0 - percentile, n );
   }
   dfloat fraction = std::max( percentile, 0.0 ) / 100.0; // Only need to clamp on bottom, value is never larger than 50.
   return static_cast< dip::uint >( std::floor( fraction * static_cast< dfloat >( n - 1 ) + 0.5 )); // Using consistent rounding.
}

/// \endgroup

} // namespace dip


#endif // DIP_NUMERIC_H
