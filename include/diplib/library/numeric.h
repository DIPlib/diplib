/*
 * DIPlib 3.0
 * This file contains numeric algorithms unrelated to images.
 *
 * (c)2015-2017, Cris Luengo.
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


#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include <cmath>
#include <numeric>
#include <limits>

#include "diplib/library/sample_iterator.h"


/// \file
/// \brief Numeric algorithms and constants unrelated to images. This file is always included through `diplib.h`.
/// \see numeric


namespace dip {


/// \defgroup numeric Numeric algorithms and constants
/// \ingroup infrastructure
/// \brief Functions and constants to be used in numeric computation, unrelated to images.
///
/// These functions and constants are made available when including `diplib.h`.
/// \{


/// \brief The constant pi.
constexpr dfloat pi = 3.14159265358979323846264338327950288;
// std::acos( -1 ) is the good way of defining pi, but it's not constexpr.

/// \brief A NaN value.
constexpr dfloat nan = std::numeric_limits< dfloat >::quiet_NaN();

/// \brief Infinity.
constexpr dfloat infinity = std::numeric_limits< dfloat >::infinity();


/// \brief Compute the greatest common denominator of two positive integers.
// `std::gcd` will be available in C++17.
inline dip::uint gcd( dip::uint a, dip::uint b ) {
   return b == 0 ? a : gcd( b, a % b );
}

/// \brief Integer division, unsigned, return ceil.
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && !std::is_signed< T >::value, int > = 0 >
T div_ceil( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, signed, return ceil.
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && std::is_signed< T >::value, int > = 0 >
T div_ceil( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   if( lhs * rhs < 0 ) {
      return lhs / rhs;
   } else {
      if( lhs < 0 ) {
         return ( lhs + 1 ) / rhs + 1;
      } else {
         return ( lhs - 1 ) / rhs + 1;
      }
   }
}

/// \brief Integer division, unsigned, return floor.
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && !std::is_signed< T >::value, int > = 0 >
T div_floor( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return lhs / rhs;
}

/// \brief Integer division, signed, return floor.
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && std::is_signed< T >::value, int > = 0 >
T div_floor( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   if( lhs * rhs < 0 ) {
      if( lhs < 0 ) {
         return ( lhs + 1 ) / rhs - 1;
      } else {
         return ( lhs - 1 ) / rhs - 1;
      }
   } else {
      return lhs / rhs;
   }
}

/// \brief Integer division, return rounded.
template< typename T, typename = std::enable_if_t< std::is_integral< T >::value, T >>
T div_round( T lhs, T rhs ) {
   return div_floor( lhs + rhs / 2, rhs );
}

/// \brief Integer modulo, result is always positive, as opposed to % operator.
inline dip::uint modulo( dip::uint value, dip::uint period ) {
   return value % period;
}

/// \brief Integer modulo, result is always positive, as opposed to % operator.
inline dip::sint modulo( dip::sint value, dip::sint period ) {
   return ( value < 0 ) ? ( period - ( -value % period )) : ( value % period );
}

/// \brief Fast floor operation, without checks, returning a `dip::sint`.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
dip::sint floor_cast( T v ) {
   auto w = static_cast< dip::sint >( v );
   return w - ( v < static_cast< T >( w ));
}

/// \brief Fast ceil operation, without checks, returning a `dip::sint`.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
dip::sint ceil_cast( T v ) {
   auto w = static_cast< dip::sint >( v );
   return w + ( v > static_cast< T >( w ));
}

/// \brief Fast round operation, without checks, returning a `dip::sint`.
// Adapted from: https://stackoverflow.com/a/30308919/7328782
template< typename T, typename = std::enable_if_t< std::is_floating_point< T >::value >>
dip::sint round_cast( T v ) {
   return floor_cast( v + 0.5 );
}

/// \brief Consistent rounding, without checks, returning a `dip::sint`.
///
/// This rounding is consistent in that half-way cases are rounded in the same direction for positive and negative
/// values. The `inverse` template parameter indicates the direction for these cases. By default, it matches
/// `std::round` for positive values.
template< typename T, bool inverse = false, typename = std::enable_if_t< std::is_floating_point< T >::value >>
dip::sint consistent_round( T v ) {
   return inverse ? ceil_cast( v - 0.5 ) : floor_cast( v + 0.5 ); // conditional should be optimized out
}

/// \brief Computes the absolute value in such a way that the result is always correct for pixel types.
/// For `dip::sint` use `std::abs` instead.
///
/// Note that, for signed integer types, `std::abs` returns an implementation-defined value when
/// the input is equal to the smallest possible value, since its positive counterpart cannot be
/// represented. For example, the absolute value of `dip::sint8(-128)` cannot be represented
/// as an 8-bit signed value. `dip::abs` would return an 8-bit unsigned value of 128.
///
/// The return type is the same as the input type, except if the input is a signed integer,
/// in which case the return type is the unsigned counterpart.
template< typename T, typename std::enable_if_t< !std::is_integral< T >::value, int > = 0 >
AbsType< T > abs( T value ) {
   return static_cast< AbsType< T >>( std::abs( value ));
}
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && std::is_unsigned< T >::value, int > = 0 >
T abs( T value ) {
   return value;
}
template< typename T, typename std::enable_if_t< std::is_integral< T >::value && std::is_signed< T >::value, int > = 0 >
AbsType< T > abs( T value ) {
   return static_cast< AbsType< T >>( std::abs( static_cast< dfloat >( value )));
}
template<> inline bin abs( bin value ) { return value; }

/// \brief Clamps a value between a min and max value (a.k.a. clip, saturate, etc.).
// `std::clamp` will be available in C++17.
template< typename T >
constexpr inline const T& clamp( const T& v, const T& lo, const T& hi ) {
   return std::min( std::max( v, lo ), hi );
}

/// \brief Computes integer powers of 10, assuming the power is relatively small.
inline dfloat pow10( dip::sint power ) {
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

/// \brief Counts the length of a (UTF-8 encoded) Unicode string.
inline dip::uint LengthUnicode( String const& string ) {
#ifdef DIP__ENABLE_UNICODE
   dip::uint len = 0;
   for( auto& s : string ) {
      len += static_cast< dip::uint >(( s & 0xc0 ) != 0x80 );
   }
   return len;
#else
   return string.length();
#endif
}

/// \brief Computes the Bessel function J of the order 0.
DIP_EXPORT dfloat BesselJ0( dfloat x );

/// \brief Computes the Bessel function J of the order 1.
DIP_EXPORT dfloat BesselJ1( dfloat x );

/// \brief Computes the Bessel function J of the order `n`.
DIP_EXPORT dfloat BesselJN( dfloat x, dip::uint n );

/// \brief Computes the Bessel function Y of the order 0.
DIP_EXPORT dfloat BesselY0( dfloat x );

/// \brief Computes the Bessel function Y of the order 1.
DIP_EXPORT dfloat BesselY1( dfloat x );

/// \brief Computes the Bessel function Y of the order `n`.
DIP_EXPORT dfloat BesselYN( dfloat x, dip::uint n );

/// \brief Computes the sinc function.
inline dfloat Sinc( dfloat x ) {
   return x == 0.0 ? 1.0 : std::sin( x ) / x;
}

/// \brief Computes the surface area of an `n`-dimensional hypershpere with radius `r`.
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

/// \brief Computes the volume of an `n`-dimensional hypershpere with radius `r`.
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
/// largest eigenvalue by magnitude. The full decomposition as in `dip::SymmetricEigenDecomposition` is computed,
/// but only one eigenvector is written to the output.
void LargestEigenVector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
);

/// \brief Finds the smallest eigenvector of a symmetric, real-valued matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order; only the lower triangle will be used.
///
/// `vector` is a pointer to space for `n` values, and will receive the eigenvector corresponding to the
/// smallest eigenvalue by magnitude. The full decomposition as in `dip::SymmetricEigenDecomposition` is computed,
/// but only one eigenvector is written to the output.
void SmallestEigenVector(
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > vector
);

/// \brief Finds the eigenvalues and eigenvectors of a symmetric, real-valued matrix, where only the unique values are given.
///
/// Calls `dip::SymmetricEigenDecomposition` after copying over the input values to a temporary buffer.
///
/// `input` is a pointer to `n*(n+1)/2` values, stored in the same order as symmetric tensors are stored in an image
/// (see dip::Tensor::Shape). That is, fist are the main diagonal elements, then the elements above the diagonal,
/// column-wise. This translates to:
///  - 2D: xx, yy, xy
///  - 3D: xx, yy, zz, xy, xz, yz
///  - 4D: xx, yy, zz, tt, xy, xz, yz, xt, yt, zt
///  - etc.
///
/// See `dip::SymmetricEigenDecomposition` for information on `lambdas` and `vectors`.
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

/// \brief Computes the sum of the values of a vector.
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
/// `%SingularValueDecomposition` uses the two-sided Jacobi SVD decomposition algorithm.
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
/// `%SingularValueDecomposition` uses the two-sided Jacobi SVD decomposition algorithm.
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
/// Solves \f$A x = b\f$, where `A` is an `m`x`n` matrix (stored in column-major order),
/// and `b` is a vector with `m` values.
/// The unknown `x` will have `n` values, and will be written to `output`.
DIP_EXPORT void Solve(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > A,
      ConstSampleIterator< dfloat > b,
      SampleIterator< dfloat > output
);

/// \}

} // namespace dip


#endif // DIP_NUMERIC_H
