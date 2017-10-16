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

#ifdef _WIN32
   #define _USE_MATH_DEFINES // Needed to define M_PI in <cmath>
#endif

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
template <typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value, T>::type
div_ceil( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, signed, return ceil.
template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, T>::type
div_ceil( T lhs, T rhs ) {
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
template <typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value, T>::type
div_floor( T lhs, T rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return lhs / rhs;
}

/// \brief Integer division, signed, return floor.
template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, T>::type
div_floor( T lhs, T rhs ) {
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
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
div_round( T lhs, T rhs ) {
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

/// \brief Consistent rounding, where half-way cases are rounded in the same direction for positive and negative
/// values. `inverse` template parameter indicates the direction for these cases. By default, it matches
/// `std::round` for positive values.
template< typename T, bool inverse = false >
inline T consistent_round( T v ) {
   return inverse ? std::ceil( v - 0.5 ) : std::floor( v + 0.5 ); // conditional should be optimized out
};

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
template< typename T, typename std::enable_if< !std::is_integral< T >::value, int >::type = 0 >
AbsType< T > abs( T value ) {
   return static_cast< AbsType< T >>( std::abs( value ));
}
template< typename T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value, int >::type = 0 >
T abs( T value ) {
   return value;
}
template< typename T, typename std::enable_if< std::is_integral< T >::value && std::is_signed< T >::value, int >::type = 0 >
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

/// \brief Finds the eigenvalues and eigenvectors of a symmetric real matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order; only the lower triangle will be used.
///
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
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

/// \brief Finds the eigenvalues and eigenvectors of a symmetric real matrix, where only the unique values are given.
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

/// \brief Finds the eigenvalues and eigenvectors of a square real matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
///
/// `lambdas` is a pointer to space for `n` values, which don't have any specific ordering.
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

/// \brief Finds the eigenvalues and eigenvectors of a square complex matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
///
/// `lambdas` is a pointer to space for `n` values, which don't have any specific ordering.
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
   return std::accumulate( input, input + n, T( 0.0 ));
}

/// \brief Computes the sum of the square of the values of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline FloatType< T > SumAbsSquare( dip::uint n, ConstSampleIterator< T > input ) {
   return std::accumulate( input, input + n, FloatType< T >( 0.0 ), []( FloatType< T > a, T b ){ return a + std::abs( b ) * std::abs( b ); } );
}

/// \brief Computes the norm of a vector.
///
/// `input` is a pointer to `n` values.
template< typename T >
inline FloatType< T > Norm( dip::uint n, ConstSampleIterator< T > input ) {
   return std::sqrt( SumAbsSquare( n, input ));
}

/// \brief Computes the determinant of a square real matrix.
///
/// `input` is a pointer to `n*n` values, in column-major order.
DIP_EXPORT dfloat Determinant( dip::uint n, ConstSampleIterator< dfloat > input );

/// \brief Computes the determinant of a square complex matrix.
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

/// \brief Computes the "thin" singular value decomposition of a real matrix
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

/// \brief Computes the "thin" singular value decomposition of a complex matrix
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

/// \brief Computes the inverse of a square real matrix.
///
/// `input` and `output` are pointers to `n*n` values, in column-major order.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( dip::uint n, ConstSampleIterator< dfloat > input, SampleIterator< dfloat > output );

/// \brief Computes the inverse of a square complex matrix.
///
/// `input` and `output` are pointers to `n*n` values, in column-major order.
///
/// The result is undetermined if the matrix is not invertible.
DIP_EXPORT void Inverse( dip::uint n, ConstSampleIterator< dcomplex > input, SampleIterator< dcomplex > output );

/// \brief Computes the pseudo-inverse of a real matrix, using the Jacobi SVD decomposition.
///
/// `input` is a pointer to `m*n` values, in column-major order.
///
/// `output` is a pointer to `n*m` values, in column-major order.
///
/// `tolerance` is an appropriate tolerance. Singular values smaller than `tolerance * max(n,m)` times the largest
/// singular value will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dfloat > input,
      SampleIterator< dfloat > output,
      dfloat tolerance = 1e-7
);

/// \brief Computes the pseudo-inverse of a complex matrix, using the Jacobi SVD decomposition.
///
/// `input` and `output` are pointers to `m*n` values, in column-major order.
///
/// `output` is a pointer to `n*m` values, in column-major order.
///
/// `tolerance` is an appropriate tolerance. Singular values smaller than `tolerance * max(n,m)` times the largest
/// singular value will be set to zero in the inverse.
DIP_EXPORT void PseudoInverse(
      dip::uint m,
      dip::uint n,
      ConstSampleIterator< dcomplex > input,
      SampleIterator< dcomplex > output,
      dfloat tolerance = 1e-7
);

/// \brief Computes the rank of a real matrix.
///
/// `input` is a pointer to `m*n` values, in column-major order.
DIP_EXPORT dip::uint Rank( dip::uint m, dip::uint n, ConstSampleIterator< dfloat > input );

/// \brief Computes the rank of a complex matrix.
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


/// \brief `%StatisticsAccumulator` computes population statistics by accumulating the first four central moments.
///
/// Samples are added one by one, using the `Push` method. Other members are used to retrieve estimates of
/// the population statistics based on the samples seen up to that point. Formula used to compute population
/// statistics are corrected, though the standard deviation, skewness and excess kurtosis are not unbiased
/// estimators. The accumulator uses a stable algorithm to prevent catastrophic cancellation.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see VarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// **Literature**
/// - Code modified from <a href="http://www.johndcook.com/blog/skewness_kurtosis/">John D. Cook</a>
/// - <a href="https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance">Wikipedia</a> has the same code.
/// - <a href="http://people.xiph.org/~tterribe/notes/homs.html">T. B. Terriberry, "Computing higher-order moments online", 2008</a>.
/// - <a href="http://infoserve.sandia.gov/sand_doc/2008/086212.pdf">Philippe P. Pébay, "Formulas for Robust,
///   One-Pass Parallel Computation of Covariances and Arbitrary-Order Statistical Moments",
///   Technical Report SAND2008-6212, Sandia National Laboratories, September 2008</a>.
/// - <a href="https://en.wikipedia.org/wiki/Skewness#Sample_skewness">Wikipedia on Skewness</a>.
/// - <a href="https://en.wikipedia.org/wiki/Kurtosis#Estimators_of_population_kurtosis">Wikipedia on Kurtosis</a>.
class DIP_NO_EXPORT StatisticsAccumulator {
   public:
      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         n_ = 0;
         m1_ = 0.0;
         m2_ = 0.0;
         m3_ = 0.0;
         m4_ = 0.0;
      }

      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         ++n_;
         dfloat n = static_cast< dfloat >( n_ );
         dfloat delta = x - m1_;
         dfloat term1 = delta / n;
         dfloat term2 = term1 * term1;
         dfloat term3 = delta * term1 * ( n - 1 );
         m4_ += term3 * term2 * ( n * n - 3.0 * n + 3 ) + 6.0 * term2 * m2_ - 4.0 * term1 * m3_;
         m3_ += term3 * term1 * ( n - 2 ) - 3.0 * term1 * m2_; // old value used for m4_ calculation
         m2_ += term3; // old value used for m3_ and m4_ calculation.
         m1_ += term1;
      }

      /// Combine two accumulators
      StatisticsAccumulator& operator+=( StatisticsAccumulator const& b ) {
         dfloat an = static_cast< dfloat >( n_ );
         dfloat an2 = an * an;
         dfloat bn = static_cast< dfloat >( b.n_ );
         dfloat bn2 = bn * bn;
         dfloat xn2 = an * bn;
         n_ += b.n_;
         dfloat nn = static_cast< dfloat >( n_ );
         dfloat n2 = nn * nn;
         dfloat delta = b.m1_ - m1_;
         dfloat delta2 = delta * delta;
         m4_ += b.m4_ + delta2 * delta2 * xn2 * ( an2 - xn2 + bn2 ) / ( n2 * nn )
               + 6.0 * delta2 * ( an2 * b.m2_ + bn2 * m2_ ) / n2
               + 4.0 * delta * ( an * b.m3_ - bn * m3_ ) / nn;
         m3_ += b.m3_ + delta * delta2 * xn2 * ( an - bn ) / n2
                + 3.0 * delta * ( an * b.m2_ - bn * m2_ ) / nn;
         m2_ += b.m2_ + delta2 * xn2 / nn;
         m1_ += bn * delta / nn;
         return *this;
      }

      /// Number of samples
      dip::uint Number() const {
         return n_;
      }
      /// Unbiased estimator of population mean
      dfloat Mean() const {
         return m1_;
      }
      /// Unbiased estimator of population variance
      dfloat Variance() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? ( m2_ / ( n - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         return std::sqrt( Variance() );
      }
      /// \brief Estimator of population skewness. This estimator is unbiased only for symmetric distributions
      /// (it is not possible to derive an unbiased estimator).
      dfloat Skewness() const {
         if(( n_ > 2 ) && ( m2_ != 0 )) {
            dfloat n = static_cast< dfloat >( n_ );
            return (( n * n ) / (( n - 1 ) * ( n - 2 ))) * ( m3_ / ( n * std::pow( Variance(), 1.5 )));
         }
         return 0;
      }
      /// \brief Estimator of population excess kurtosis. This estimator is only unbiased for normally
      /// distributed data (it is not possible to derive an unbiased estimator).
      dfloat ExcessKurtosis() const {
         if( n_ > 3 && ( m2_ != 0 )) {
            dfloat n = static_cast< dfloat >( n_ );
            return ( n - 1 ) / (( n - 2 ) * ( n - 3 )) * (( n + 1 ) * n * m4_ / ( m2_ * m2_ ) - 3 * ( n - 1 ));
         }
         return 0;
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dfloat m1_ = 0.0;   // mean of values x
      dfloat m2_ = 0.0;   // sum of (x-mean(x))^2  --  `m2_ / n_` is second order central moment
      dfloat m3_ = 0.0;   // sum of (x-mean(x))^3  --  `m3_ / n_` is third order central moment
      dfloat m4_ = 0.0;   // sum of (x-mean(x))^4  --  `m4_ / n_` is fourth order central moment
};

/// \brief Combine two accumulators
inline StatisticsAccumulator operator+( StatisticsAccumulator lhs, StatisticsAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%VarianceAccumulator` computes mean and standard deviation by accumulating the first two
/// central moments.
///
/// Samples are added one by one, using the `Push` method. Other members are used to retrieve estimates of
/// the population statistics based on the samples seen up to that point. Formula used to compute population
/// statistics are corrected, though the standard deviation is not an unbiased estimator. The accumulator
/// uses a stable algorithm to prevent catastrophic cancellation.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// It is also possible to remove a sample from the accumulator, using the `Pop` method. It is assumed that the
/// particular value passed to this method had been added previously to the accumulator. If this is not the case,
/// resulting means and variances are no longer correct.
///
/// \see StatisticsAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// **Literature**
/// - Donald E. Knuth, "The Art of Computer Programming, Volume 2: Seminumerical Algorithms", 3rd Ed., 1998.
class DIP_NO_EXPORT VarianceAccumulator {
   public:
      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         n_ = 0;
         m1_ = 0.0;
         m2_ = 0.0;
      }

      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         ++n_;
         dfloat delta = x - m1_;
         m1_ += delta / static_cast< dfloat >( n_ );
         m2_ += delta * ( x - m1_ );
      }

      /// Remove a sample from the accumulator
      void Pop( dfloat x ) {
         if( n_ > 0 ) {
            dfloat delta = x - m1_;
            m1_ = ( m1_ * static_cast< dfloat >( n_ ) - x ) / static_cast< dfloat >( n_ - 1 );
            m2_ -= delta * ( x - m1_ );
            --n_;
         }
      }

      /// Combine two accumulators
      VarianceAccumulator& operator+=( VarianceAccumulator const& b ) {
         dfloat oldn = static_cast< dfloat >( n_ );
         n_ += b.n_;
         dfloat n = static_cast< dfloat >( n_ );
         dfloat bn = static_cast< dfloat >( b.n_ );
         dfloat delta = b.m1_ - m1_;
         m1_ += bn * delta / n;
         m2_ += b.m2_ + delta * delta * ( oldn * bn ) / n;
         return *this;
      }

      /// Number of samples
      dip::uint Number() const {
         return n_;
      }
      /// Unbiased estimator of population mean
      dfloat Mean() const {
         return m1_;
      }
      /// Unbiased estimator of population variance
      dfloat Variance() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? ( m2_ / ( n - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         return std::sqrt( Variance() );
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dfloat m1_ = 0.0;   // mean of values x
      dfloat m2_ = 0.0;   // sum of (x-mean(x))^2  --  `m2_ / n_` is second order central moment
};

/// \brief Combine two accumulators
inline VarianceAccumulator operator+( VarianceAccumulator lhs, VarianceAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%CovarianceAccumulator` computes covariance and correlation of pairs of samples by accumulating the
/// first two central moments and cross-moments.
///
/// Samples are added one pair at the time, using the `Push` method. Other members are used to retrieve the results.
///
/// The covariance matrix is formed by
/// ```
///    | cov.VarianceX()   cov.Covariance() |
///    | cov.Covariance()  cov.VarianceY()  |
/// ```
///
/// The `Regression` method returns the parameters to the least squares fit of the equation:
/// ```
///    `y = intercept + slope * x`
/// ```
/// where `x` is the first sample in each pair, and y is the second (this is linear regression). The `Slope` method
/// computes only the slope component.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see StatisticsAccumulator, VarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// **Literature**
/// - <a href="https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Covariance">Wikipedia</a>.
class DIP_NO_EXPORT CovarianceAccumulator {
      // TODO: rewrite this for arbitrary number of variables
   public:
      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         n_ = 0;
         meanx_ = 0;
         m2x_ = 0;
         meany_ = 0;
         m2y_ = 0;
         C_ = 0;
      }

      /// Add a pair of samples to the accumulator
      void Push( dfloat x, dfloat y ) {
         ++n_;
         dfloat dx = x - meanx_;
         meanx_ += dx / static_cast< dfloat >( n_ );
         m2x_ += dx * ( x - meanx_ );
         dfloat dy = y - meany_;
         meany_ += dy / static_cast< dfloat >( n_ );
         dfloat dy_new = y - meany_;
         m2y_ += dy * dy_new;
         C_ += dx * dy_new;
      }

      /// Combine two accumulators
      CovarianceAccumulator& operator+=( const CovarianceAccumulator& other ) {
         if( n_ == 0 ) {
            *this = other; // copy over the data
         } else if( other.n_ > 0 ) {
            size_t intN = n_ + other.n_;
            dfloat N = static_cast< dfloat >( intN );
            dfloat dx = other.meanx_ - meanx_;
            dfloat dy = other.meany_ - meany_;
            meanx_ = ( static_cast< dfloat >( n_ ) * meanx_ + static_cast< dfloat >( other.n_ ) * other.meanx_ ) / N;
            meany_ = ( static_cast< dfloat >( n_ ) * meany_ + static_cast< dfloat >( other.n_ ) * other.meany_ ) / N;
            dfloat fN = static_cast< dfloat >( n_ * other.n_ ) / N;
            m2x_ += other.m2x_ + dx * dx * fN;
            m2y_ += other.m2y_ + dy * dy * fN;
            C_ += other.C_ + dx * dy * fN;
            n_ = intN;
         }
         return *this;
      }

      /// Number of samples
      size_t Number() const {
         return n_;
      }
      /// Unbiased estimator of population mean for first variable
      dfloat MeanX() const {
         return meanx_;
      }
      /// Unbiased estimator of population mean for second variable
      dfloat MeanY() const {
         return meany_;
      }
      /// Unbiased estimator of population variance for first variable
      dfloat VarianceX() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? ( m2x_ / ( n - 1 )) : ( 0.0 );
      }
      /// Unbiased estimator of population variance for second variable
      dfloat VarianceY() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? ( m2y_ / ( n - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation for first variable (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviationX() const {
         return std::sqrt( VarianceX() );
      }
      /// Estimator of population standard deviation for second variable (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviationY() const {
         return std::sqrt( VarianceY() );
      }
      /// Unbiased estimator of population covariance
      dfloat Covariance() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? ( C_ / ( n - 1.0 )) : ( 0.0 );
      }
      /// Estimator of correlation between the two variables
      dfloat Correlation() const {
         dfloat S = std::sqrt( m2x_ * m2y_ );
         return (( n_ > 1 ) && ( S != 0.0 )) ? ( C_ / S ) : ( 0.0 );
      }
      /// Computes the slope of the regression line
      dfloat Slope() const {
         //dfloat stdX = StandardDeviationX();
         //return ( stdX != 0.0 ) ? ( Correlation() * StandardDeviationY() / stdX ) : ( 0.0 );
         return ( m2x_ != 0.0 ) ? ( C_ / m2x_ ) : ( 0.0 );
      }
      /// Contains the output of the `Regression` method.
      struct RegressionResult {
         dfloat intercept = 0.0;
         dfloat slope = 0.0;
      };
      /// Computes the slope and intercept of the regression line
      RegressionResult Regression() const {
         RegressionResult out;
         out.slope = Slope();
         out.intercept = meany_ - out.slope * meanx_;
         return out;
      };

   private:
      dip::uint n_ = 0;
      dfloat meanx_ = 0;
      dfloat m2x_ = 0;
      dfloat meany_ = 0;
      dfloat m2y_ = 0;
      dfloat C_ = 0;
};


/// \brief `%DirectionalStatisticsAccumulator` computes directional mean and standard deviation by accumulating
/// a unit vector with the input value as angle.
///
/// Samples are added one by one, using the `Push` method. Other members are used to retrieve estimates of
/// the sample statistics based on the samples seen up to that point.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see StatisticsAccumulator, VarianceAccumulator, CovarianceAccumulator, MinMaxAccumulator, MomentAccumulator
class DIP_NO_EXPORT DirectionalStatisticsAccumulator {
   public:
      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         n_ = 0;
         sum_ = { 0.0, 0.0 };
      }

      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         ++n_;
         sum_ += dcomplex{ std::cos( x ), std::sin( x ) };
      }

      /// Combine two accumulators
      DirectionalStatisticsAccumulator& operator+=( DirectionalStatisticsAccumulator const& b ) {
         n_ += b.n_;
         sum_ += b.sum_;
         return *this;
      }

      /// Number of samples
      dip::uint Number() const {
         return n_;
      }
      /// Unbiased estimator of population mean
      dfloat Mean() const {
         return std::arg( sum_ );
      }
      /// Unbiased estimator of population variance
      dfloat Variance() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 0 ) ? ( 1.0 - std::abs( sum_ ) / n ) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 0 ) ? ( std::sqrt( -2.0 * std::log( std::abs( sum_ ) / n ))) : ( 0.0 );
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dcomplex sum_ = { 0.0, 0.0 }; // sum of values exp(i x)
};

/// \brief Combine two accumulators
inline DirectionalStatisticsAccumulator operator+( DirectionalStatisticsAccumulator lhs, DirectionalStatisticsAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%MinMaxAccumulator` computes minimum and maximum values of a sequence of values.
///
/// Samples are added one by one or two by two, using the `Push` method. Other members are used to retrieve the results.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see StatisticsAccumulator, VarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MomentAccumulator
class DIP_NO_EXPORT MinMaxAccumulator {
   public:
      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         min_ = std::numeric_limits< dfloat >::max();
         max_ = std::numeric_limits< dfloat >::lowest();
      }

      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         max_ = std::max( max_, x );
         min_ = std::min( min_, x );
      }

      /// \brief Add two samples to the accumulator. Prefer this over adding one value at the time.
      void Push( dfloat x, dfloat y ) {
         if( x > y ) {
            max_ = std::max( max_, x );
            min_ = std::min( min_, y );
         } else { // y >= x
            max_ = std::max( max_, y );
            min_ = std::min( min_, x );
         }
      }

      /// Combine two accumulators
      MinMaxAccumulator& operator+=( MinMaxAccumulator const& other ) {
         min_ = std::min( min_, other.min_ );
         max_ = std::max( max_, other.max_ );
         return *this;
      }

      /// Minimum value seen so far
      dfloat Minimum() const {
         return min_;
      }

      /// Maximum value seen so far
      dfloat Maximum() const {
         return max_;
      }

   private:
      dfloat min_ = std::numeric_limits< dfloat >::max();
      dfloat max_ = std::numeric_limits< dfloat >::lowest();
};

/// \brief Combine two accumulators
inline MinMaxAccumulator operator+( MinMaxAccumulator lhs, MinMaxAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%MomentAccumulator` accumulates the zeroth order moment, the first order normalized moments, and the
/// second order central normalized moments, in `N` dimensions.
///
/// Samples are added one by one, using the `Push` method. Other members are used to retrieve the moments.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see StatisticsAccumulator, VarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator
class DIP_NO_EXPORT MomentAccumulator {
   public:
      /// The constructor determines the dimensionality for the object.
      MomentAccumulator( dip::uint N ) {
         DIP_THROW_IF( N < 1, E::PARAMETER_OUT_OF_RANGE );
         m0_ = 0.0;
         m1_.resize( N, 0.0 );
         m2_.resize( N * ( N + 1 ) / 2, 0.0 );
      }

      /// Reset the accumulator, leaving it as if newly allocated.
      void Reset() {
         m0_ = 0.0;
         m1_.fill( 0.0 );
         m2_.fill( 0.0 );
      }

      /// \brief Add a sample to the accumulator. `pos` must have `N` dimensions.
      void Push( FloatArray pos, dfloat weight ) {
         dip::uint N = m1_.size();
         DIP_ASSERT( pos.size() == N );
         m0_ += weight;
         dip::uint kk = 0;
         for( dip::uint ii = 0; ii < N; ++ii ) {
            m1_[ ii ] += pos[ ii ] * weight;
            for( dip::uint jj = 0; jj <= ii; ++jj ) {
               m2_[ kk ] += pos[ ii ] * pos[ jj ] * weight;
               ++kk;
            }
         }
      }

      /// Combine two accumulators
      MomentAccumulator& operator+=( MomentAccumulator const& b ) {
         m0_ += b.m0_;
         m1_ += b.m1_;
         m2_ += b.m2_;
         return *this;
      }

      /// Sum of weights (zeroth order moment)
      dfloat Sum() const {
         return m0_;
      }

      /// First order moments, normalized
      FloatArray FirstOrder() const {
         if( m0_ == 0 ) {
            return FloatArray( m1_.size(), 0.0 );
         } else {
            FloatArray out = m1_;
            for( dfloat& v : out ) {
               v /= m0_;
            }
            return out;
         }
      }

      /// \brief Second order central moment tensor, normalized
      ///
      /// The moments are stored in the same order as symmetric tensors are stored in an image
      /// (see dip::Tensor::Shape). That is, fist are the main diagonal elements, then the elements
      /// above the diagonal, column-wise. This translates to:
      ///  - 2D: xx, yy, xy
      ///  - 3D: xx, yy, zz, xy, xz, yz
      ///  - 4D: xx, yy, zz, tt, xy, xz, yz, xt, yt, zt
      ///  - etc.
      ///
      /// The second order moment tensor is defined as:
      ///
      ///    \f$ I = \Sigma_k m_k ((\vec{r_k} \cdot \vec{r_k}) E - \vec{r_k} \otimes \vec{r_k}) \f$
      ///
      /// where \f$ E \f$ is the identity matrix ( \f$ E = \Sigma_i \vec{e_i} \otimes \vec{e_i} \f$ ), \f$ m_k \f$
      /// is the weight of point \f$ k \f$ , and \f$ \vec{r_k} \f$ is its position. In 2D, this leads to:
      ///
      ///     \f$ I_{xx} = \Sigma_k m_k y^2 \f$<br>
      ///     \f$ I_{yy} = \Sigma_k m_k x^2 \f$<br>
      ///     \f$ I_{xy} = -\Sigma_k m_k x y \f$
      ///
      /// In 3D, it leads to:
      ///
      ///     \f$ I_{xx} = \Sigma_k m_k y^2 + \Sigma_k m_k z^2 \f$<br>
      ///     \f$ I_{yy} = \Sigma_k m_k x^2 + \Sigma_k m_k z^2 \f$<br>
      ///     \f$ I_{zz} = \Sigma_k m_k x^2 + \Sigma_k m_k y^2 \f$<br>
      ///     \f$ I_{xy} = -\Sigma_k m_k x y \f$<br>
      ///     \f$ I_{xz} = -\Sigma_k m_k x z \f$<br>
      ///     \f$ I_{yz} = -\Sigma_k m_k y z \f$
      ///
      /// The equations above represent the second order moments, we compute instead the central moments, and
      /// normalize them by the sum of weights.
      FloatArray SecondOrder() const {
         FloatArray out( m2_.size(), 0.0 ); // output tensor
         if( m0_ != 0 ) {
            dip::uint N = m1_.size();
            FloatArray m1 = FirstOrder(); // normalized first order moments
            FloatArray m2( N, 0.0 );      // normalized second order central moments, diagonal elements
            for( dip::uint ii = 0, kk = 0; ii < N; ++ii, kk += 1 + ii ) {
               m2[ ii ] = m2_[ kk ] / m0_ - m1[ ii ] * m1[ ii ];
            }
            for( dip::uint ii = 0; ii < N; ++ii ) {
               dfloat acc = 0.0;
               for( dip::uint jj = 0; jj < N; ++jj ) {
                  if( jj != ii ) {
                     acc += m2[ jj ];
                  }
               }
               out[ ii ] = acc;
            }
            for( dip::uint ii = 1, kk = N, ll = 1; ii < N; ++ii, ++ll ) {
               for( dip::uint jj = 0; jj < ii; ++jj, ++kk, ++ll ) {
                  out[ kk ] = m1[ ii ] * m1[ jj ] - m2_[ ll ] / m0_;
               }
            }
         }
         return out;
      }

   private:
      dfloat m0_;       // zeroth order moments accumulated here (sum of weights)
      FloatArray m1_;   // first order moments accumulated here (N values)
      FloatArray m2_;   // second order moments accumulated here (N*(N+1)/2 values)
      // Second order moments are stored column-wise, after removing the symmetric elements below the main diagonal:
      //  - 2D: xx, xy, yy
      //  - 3D: xx, xy, yy, xz, yz, zz
      //  - 4D: xx, xy, yy, xz, yz, zz, xt, yt, zt, tt
      //  - etc.
      // Note that this order is different from the one we use for the output. This is more convenient in computation,
      // the output order matches that of pixel storage.
};

/// \brief Combine two accumulators
inline MomentAccumulator operator+( MomentAccumulator lhs, MomentAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::gcd function") {
   DOCTEST_CHECK( dip::gcd( 10, 10 ) == 10 );
   DOCTEST_CHECK( dip::gcd( 10, 5 ) == 5 );
   DOCTEST_CHECK( dip::gcd( 10, 1 ) == 1 );
   DOCTEST_CHECK( dip::gcd( 10, 12 ) == 2 );
   DOCTEST_CHECK( dip::gcd( 10, 15 ) == 5 );
   DOCTEST_CHECK( dip::gcd( 15, 10 ) == 5 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_XXX functions") {
   DOCTEST_CHECK( dip::div_ceil( 11l, 11l ) == 1 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 6l ) == 2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 5l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 4l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 3l ) == 4 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 3l ) == -3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 4l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 5l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 6l ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -3l ) == -3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -4l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -5l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -6l ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -6l ) == 2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -5l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -4l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -3l ) == 4 );

   DOCTEST_CHECK( dip::div_floor( 10l, 10l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( 11l, 6l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( 11l, 5l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( 11l, 4l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( 11l, 3l ) == 3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 3l ) == -4 );
   DOCTEST_CHECK( dip::div_floor( -11l, 4l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 5l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 6l ) == -2 );
   DOCTEST_CHECK( dip::div_floor( 11l, -3l ) == -4 );
   DOCTEST_CHECK( dip::div_floor( 11l, -4l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( 11l, -5l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( 11l, -6l ) == -2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -6l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( -11l, -5l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -4l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -3l ) == 3 );

   DOCTEST_CHECK( dip::div_round( 10l, 10l ) == 1 );
   DOCTEST_CHECK( dip::div_round( 11l, 6l ) == 2 );
   DOCTEST_CHECK( dip::div_round( 11l, 5l ) == 2 );
   DOCTEST_CHECK( dip::div_round( 11l, 4l ) == 3 );
   DOCTEST_CHECK( dip::div_round( 11l, 3l ) == 4 );
   DOCTEST_CHECK( dip::div_round( -11l, 3l ) == -4 );
   DOCTEST_CHECK( dip::div_round( -11l, 4l ) == -3 );
   DOCTEST_CHECK( dip::div_round( -11l, 5l ) == -2 );
   DOCTEST_CHECK( dip::div_round( -11l, 6l ) == -2 );
   DOCTEST_CHECK( dip::div_round( 11l, -3l ) == -4 );
   DOCTEST_CHECK( dip::div_round( 11l, -4l ) == -3 );
   DOCTEST_CHECK( dip::div_round( 11l, -5l ) == -2 );
   DOCTEST_CHECK( dip::div_round( 11l, -6l ) == -2 );
   DOCTEST_CHECK( dip::div_round( -11l, -6l ) == 2 );
   DOCTEST_CHECK( dip::div_round( -11l, -5l ) == 2 );
   DOCTEST_CHECK( dip::div_round( -11l, -4l ) == 3 );
   DOCTEST_CHECK( dip::div_round( -11l, -3l ) == 4 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::pow10 function") {
   DOCTEST_CHECK( dip::pow10( 25 ) == doctest::Approx( std::pow( 10, 25 )) );
   DOCTEST_CHECK( dip::pow10( 10 ) == std::pow( 10, 10 ) );
   DOCTEST_CHECK( dip::pow10( 1 ) == std::pow( 10, 1 ) );
   DOCTEST_CHECK( dip::pow10( 0 ) == std::pow( 10, 0 ) );
   DOCTEST_CHECK( dip::pow10( -5 ) == std::pow( 10, -5 ) );
   DOCTEST_CHECK( dip::pow10( -21 ) == doctest::Approx( std::pow( 10, -21 )) );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::abs function") {
   DOCTEST_CHECK( dip::abs( dip::uint8( 25 )) == dip::uint8( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint8( 25 )) == dip::uint8( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint8( -25 )) == dip::uint8( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint8( -128 )) == dip::uint8( 128 ) );
   DOCTEST_CHECK( dip::abs( dip::uint32( 25 )) == dip::uint32( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint32( 25 )) == dip::uint32( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint32( -25 )) == dip::uint32( 25 ) );
   DOCTEST_CHECK( dip::abs( dip::sint32( -2147483648 )) == dip::uint32( 2147483648u ) );
   DOCTEST_CHECK( dip::abs( dip::sfloat( 25.6 )) == dip::sfloat( 25.6 ) );
   DOCTEST_CHECK( dip::abs( dip::sfloat( -25.6 )) == dip::sfloat( 25.6 ) );
   DOCTEST_CHECK( dip::abs( dip::scomplex{ 1.2f, 5.3f } ) == std::hypot( 1.2f, 5.3f ) );
}

DOCTEST_TEST_CASE("[DIPlib] testing the statictical accumulators") {
   {
      dip::StatisticsAccumulator acc1;
      acc1.Push( 1.0 );
      acc1.Push( 1.0 );
      acc1.Push( 1.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 1.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.0 ));
      acc1.Push( 2.0 );
      acc1.Push( 2.0 );
      acc1.Push( 2.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 1.5 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.5 * 0.5 * 6.0 / 5.0 ));
      dip::StatisticsAccumulator acc2;
      acc2.Push( 3.0 );
      acc2.Push( 3.0 );
      acc2.Push( 3.0 );
      acc1 += acc2;
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 2.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 6.0 / 8.0 ));
   }
   {
      dip::VarianceAccumulator acc1;
      acc1.Push( 1.0 );
      acc1.Push( 1.0 );
      acc1.Push( 1.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 1.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.0 ));
      acc1.Push( 2.0 );
      acc1.Push( 2.0 );
      acc1.Push( 2.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 1.5 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.5 * 0.5 * 6.0 / 5.0 ));
      dip::VarianceAccumulator acc2;
      acc2.Push( 3.0 );
      acc2.Push( 3.0 );
      acc2.Push( 3.0 );
      acc1 += acc2;
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 2.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 6.0 / 8.0 ));
      acc1.Pop( 3.0 );
      acc1.Pop( 3.0 );
      acc1.Pop( 3.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 1.5 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.5 * 0.5 * 6.0 / 5.0 ));
   }
   {
      dip::CovarianceAccumulator acc1;
      acc1.Push( 1.0, 1.0 );
      acc1.Push( 1.0, 1.0 );
      acc1.Push( 1.0, 1.0 );
      DOCTEST_CHECK( acc1.MeanX() == doctest::Approx( 1.0 ));
      DOCTEST_CHECK( acc1.VarianceX() == doctest::Approx( 0.0 ));
      DOCTEST_CHECK( acc1.MeanY() == doctest::Approx( 1.0 ));
      DOCTEST_CHECK( acc1.VarianceY() == doctest::Approx( 0.0 ));
      DOCTEST_CHECK( acc1.Covariance() == doctest::Approx( 0.0 ));
      acc1.Push( 2.0, 1.0 );
      acc1.Push( 2.0, 1.0 );
      acc1.Push( 2.0, 1.0 );
      DOCTEST_CHECK( acc1.MeanX() == doctest::Approx( 1.5 ));
      DOCTEST_CHECK( acc1.VarianceX() == doctest::Approx( 0.5 * 0.5 * 6.0 / 5.0 ));
      DOCTEST_CHECK( acc1.MeanY() == doctest::Approx( 1.0 ));
      DOCTEST_CHECK( acc1.VarianceY() == doctest::Approx( 0.0 ));
      DOCTEST_CHECK( acc1.Covariance() == doctest::Approx( 0.0 ));
      dip::CovarianceAccumulator acc2;
      acc2.Push( 3.0, 2.0 );
      acc2.Push( 3.0, 2.0 );
      acc2.Push( 3.0, 2.0 );
      acc1 += acc2;
      DOCTEST_CHECK( acc1.MeanX() == doctest::Approx( 2.0 ));
      DOCTEST_CHECK( acc1.VarianceX() == doctest::Approx( 1.0 * 6.0 / 8.0 ));
      DOCTEST_CHECK( acc1.MeanY() == doctest::Approx( 12.0 / 9.0 ));
      DOCTEST_CHECK( acc1.VarianceY() == doctest::Approx(( 6.0 / 9.0 + 4.0 / 3.0 ) / 8.0 ));
      DOCTEST_CHECK( acc1.Covariance() == doctest::Approx( 3.0 / 8.0 ));
   }
   {
      dip::CovarianceAccumulator acc;
      acc.Push( 1.0, 3.2 * 1.0 + 5.5 );
      acc.Push( 2.0, 3.2 * 2.0 + 5.5 );
      acc.Push( 3.0, 3.2 * 3.0 + 5.5 );
      acc.Push( 4.0, 3.2 * 4.0 + 5.5 );
      acc.Push( 5.0, 3.2 * 5.0 + 5.5 );
      acc.Push( 6.0, 3.2 * 6.0 + 5.5 );
      DOCTEST_CHECK( acc.MeanX() == doctest::Approx( 3.5 ));
      DOCTEST_CHECK( acc.VarianceX() == doctest::Approx( 3.5 ));
      DOCTEST_CHECK( acc.MeanY() == doctest::Approx( 3.5 * 3.2 + 5.5 ));
      DOCTEST_CHECK( acc.VarianceY() == doctest::Approx( 3.5 * 3.2 * 3.2 ));
      DOCTEST_CHECK( acc.Covariance() == doctest::Approx( 3.5 * 3.2 ));
      DOCTEST_CHECK( acc.Slope() == doctest::Approx( 3.2 ));
      auto res = acc.Regression();
      DOCTEST_CHECK( res.slope == doctest::Approx( 3.2 ));
      DOCTEST_CHECK( res.intercept == doctest::Approx( 5.5 ));
   }
   {
      dip::DirectionalStatisticsAccumulator acc1;
      acc1.Push( 0.0 );
      acc1.Push( 0.0 );
      acc1.Push( 0.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 0.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 0.0 ));
      acc1.Push( dip::pi / 2.0 );
      acc1.Push( dip::pi / 2.0 );
      acc1.Push( dip::pi / 2.0 );
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( dip::pi / 4.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 1.0 - std::sqrt( 2.0 ) / 2.0 ));
      dip::DirectionalStatisticsAccumulator acc2;
      acc2.Push( -dip::pi / 2.0 );
      acc2.Push( -dip::pi / 2.0 );
      acc2.Push( -dip::pi / 2.0 );
      acc1 += acc2;
      DOCTEST_CHECK( acc1.Mean() == doctest::Approx( 0.0 ));
      DOCTEST_CHECK( acc1.Variance() == doctest::Approx( 1.0 - 3.0 / 9.0 ));
   }
   {
      dip::MinMaxAccumulator acc1;
      acc1.Push( 0.0 );
      acc1.Push( 1.0 );
      acc1.Push( 2.0 );
      DOCTEST_CHECK( acc1.Maximum() == 2.0 );
      DOCTEST_CHECK( acc1.Minimum() == 0.0 );
      acc1.Push( 1.2, 1.4 );
      acc1.Push( -1.0, 5.0 );
      DOCTEST_CHECK( acc1.Maximum() == 5.0 );
      DOCTEST_CHECK( acc1.Minimum() == -1.0 );
      dip::MinMaxAccumulator acc2;
      acc2.Push( 6.0 );
      acc2.Push( 4.0 );
      acc2.Push( 1.0 );
      acc1 += acc2;
      DOCTEST_CHECK( acc1.Maximum() == 6.0 );
      DOCTEST_CHECK( acc1.Minimum() == -1.0 );
   }
}

#endif // DIP__ENABLE_DOCTEST

#endif // DIP_NUMERIC_H
