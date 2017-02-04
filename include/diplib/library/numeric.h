/*
 * DIPlib 3.0
 * This file contains numeric algorithms unrelated to images.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include <cmath>
#include <algorithm>

#include "diplib/library/types.h"

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#endif


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
constexpr double pi = 3.14159265358979323846264338327950288;
// std::acos( -1 ) is the good way of definig pi, but it's not constexpr.

/// \brief Compute the greatest common denominator of two positive integers.
// `std::gcd` will be available in C++17.
inline dip::uint gcd( dip::uint a, dip::uint b ) {
   return b == 0 ? a : gcd( b, a % b );
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::gcd function") {
   DOCTEST_CHECK( gcd( 10, 10 ) == 10 );
   DOCTEST_CHECK( gcd( 10, 5 ) == 5 );
   DOCTEST_CHECK( gcd( 10, 1 ) == 1 );
   DOCTEST_CHECK( gcd( 10, 12 ) == 2 );
   DOCTEST_CHECK( gcd( 10, 15 ) == 5 );
   DOCTEST_CHECK( gcd( 15, 10 ) == 5 );
}

#endif

/// \brief Integer division, return ceil.
inline dip::uint div_ceil( dip::uint lhs, dip::uint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, return ceil.
inline dip::sint div_ceil( dip::sint lhs, dip::sint rhs ) {
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

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_ceil function") {
   DOCTEST_CHECK( div_ceil( 11l, 11l ) == 1 );
   DOCTEST_CHECK( div_ceil( 11l, 6l ) == 2 );
   DOCTEST_CHECK( div_ceil( 11l, 5l ) == 3 );
   DOCTEST_CHECK( div_ceil( 11l, 4l ) == 3 );
   DOCTEST_CHECK( div_ceil( 11l, 3l ) == 4 );
   DOCTEST_CHECK( div_ceil( -11l, 3l ) == -3 );
   DOCTEST_CHECK( div_ceil( -11l, 4l ) == -2 );
   DOCTEST_CHECK( div_ceil( -11l, 5l ) == -2 );
   DOCTEST_CHECK( div_ceil( -11l, 6l ) == -1 );
   DOCTEST_CHECK( div_ceil( 11l, -3l ) == -3 );
   DOCTEST_CHECK( div_ceil( 11l, -4l ) == -2 );
   DOCTEST_CHECK( div_ceil( 11l, -5l ) == -2 );
   DOCTEST_CHECK( div_ceil( 11l, -6l ) == -1 );
   DOCTEST_CHECK( div_ceil( -11l, -6l ) == 2 );
   DOCTEST_CHECK( div_ceil( -11l, -5l ) == 3 );
   DOCTEST_CHECK( div_ceil( -11l, -4l ) == 3 );
   DOCTEST_CHECK( div_ceil( -11l, -3l ) == 4 );
}

#endif

/// \brief Integer division, return floor.
inline dip::uint div_floor( dip::uint lhs, dip::uint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return lhs / rhs;
}

/// \brief Integer division, return floor.
inline dip::sint div_floor( dip::sint lhs, dip::sint rhs ) {
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

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_floor function") {
   DOCTEST_CHECK( div_floor( 10l, 10l ) == 1 );
   DOCTEST_CHECK( div_floor( 11l, 6l ) == 1 );
   DOCTEST_CHECK( div_floor( 11l, 5l ) == 2 );
   DOCTEST_CHECK( div_floor( 11l, 4l ) == 2 );
   DOCTEST_CHECK( div_floor( 11l, 3l ) == 3 );
   DOCTEST_CHECK( div_floor( -11l, 3l ) == -4 );
   DOCTEST_CHECK( div_floor( -11l, 4l ) == -3 );
   DOCTEST_CHECK( div_floor( -11l, 5l ) == -3 );
   DOCTEST_CHECK( div_floor( -11l, 6l ) == -2 );
   DOCTEST_CHECK( div_floor( 11l, -3l ) == -4 );
   DOCTEST_CHECK( div_floor( 11l, -4l ) == -3 );
   DOCTEST_CHECK( div_floor( 11l, -5l ) == -3 );
   DOCTEST_CHECK( div_floor( 11l, -6l ) == -2 );
   DOCTEST_CHECK( div_floor( -11l, -6l ) == 1 );
   DOCTEST_CHECK( div_floor( -11l, -5l ) == 2 );
   DOCTEST_CHECK( div_floor( -11l, -4l ) == 2 );
   DOCTEST_CHECK( div_floor( -11l, -3l ) == 3 );
}

#endif

/// \brief Integer division, return rounded.
inline dip::uint div_round( dip::uint lhs, dip::uint rhs ) {
   return div_floor( lhs + rhs / 2, rhs );
}

/// \brief Integer division, return rounded.
inline dip::sint div_round( dip::sint lhs, dip::sint rhs ) {
   return div_floor( lhs + rhs / 2, rhs );
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_round function") {
   DOCTEST_CHECK( div_round( 10l, 10l ) == 1 );
   DOCTEST_CHECK( div_round( 11l, 6l ) == 2 );
   DOCTEST_CHECK( div_round( 11l, 5l ) == 2 );
   DOCTEST_CHECK( div_round( 11l, 4l ) == 3 );
   DOCTEST_CHECK( div_round( 11l, 3l ) == 4 );
   DOCTEST_CHECK( div_round( -11l, 3l ) == -4 );
   DOCTEST_CHECK( div_round( -11l, 4l ) == -3 );
   DOCTEST_CHECK( div_round( -11l, 5l ) == -2 );
   DOCTEST_CHECK( div_round( -11l, 6l ) == -2 );
   DOCTEST_CHECK( div_round( 11l, -3l ) == -4 );
   DOCTEST_CHECK( div_round( 11l, -4l ) == -3 );
   DOCTEST_CHECK( div_round( 11l, -5l ) == -2 );
   DOCTEST_CHECK( div_round( 11l, -6l ) == -2 );
   DOCTEST_CHECK( div_round( -11l, -6l ) == 2 );
   DOCTEST_CHECK( div_round( -11l, -5l ) == 2 );
   DOCTEST_CHECK( div_round( -11l, -4l ) == 3 );
   DOCTEST_CHECK( div_round( -11l, -3l ) == 4 );
}

#endif

/// \brief Clamps a value between a min and max value (a.k.a. clip, saturate, etc.).
// `std::clamp` will be available in C++17.
template< typename T >
constexpr inline const T& clamp( const T& v, const T& lo, const T& hi ) {
   return std::min( std::max( v, lo ), hi );
}

/// \brief Computes integer powers of 10, assuming the power is relatively small.
inline double pow10( dip::sint power ) {
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

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::pow10 function") {
   DOCTEST_CHECK( pow10( 25 ) == doctest::Approx( std::pow( 10, 25 )) );
   DOCTEST_CHECK( pow10( 10 ) == std::pow( 10, 10 ) );
   DOCTEST_CHECK( pow10( 1 ) == std::pow( 10, 1 ) );
   DOCTEST_CHECK( pow10( 0 ) == std::pow( 10, 0 ) );
   DOCTEST_CHECK( pow10( -5 ) == std::pow( 10, -5 ) );
   DOCTEST_CHECK( pow10( -21 ) == doctest::Approx( std::pow( 10, -21 )) );
}

#endif


/// \brief Finds the eigenvalues of a 2D symmetric matrix.
///
/// `input` is a pointer to 4 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
void SymmetricEigenValues2D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues of a 2D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 3 values: { xx, xy, yy }.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
inline void SymmetricEigenValues2DPacked( double const* input, double* lambdas ) {
   double matrix[ 4 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   // matrix[ 2 ] is never used
   matrix[ 3 ] = input[ 2 ];
   SymmetricEigenValues2D( matrix, lambdas );
}

/// \brief Finds the eigenvalues and eigenvectors of a 2D symmetric matrix.
///
/// `input` is a pointer to 4 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for 4 values and will receive the 2 eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]` and `&vectors[ 2 ]`.
void SymmetricEigenSystem2D( double const* input, double* lambdas, double* vectors );

/// \brief Finds the eigenvalues and eigenvectors of a 2D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 3 values: { xx, xy, yy }.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for 4 values and will receive the 2 eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]` and `&vectors[ 2 ]`.
inline void SymmetricEigenSystem2DPacked( double const* input, double* lambdas, double* vectors ) {
   double matrix[ 4 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   // matrix[ 2 ] is never used
   matrix[ 3 ] = input[ 2 ];
   SymmetricEigenSystem2D( matrix, lambdas, vectors );
}


/// \brief Finds the eigenvalues of a 3D symmetric matrix.
///
/// `input` is a pointer to 9 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
void SymmetricEigenValues3D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues of a 3D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 6 values: { xx, xy, xz, yy, yz, zz }.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
inline void SymmetricEigenValues3DPacked( double const* input, double* lambdas ) {
   double matrix[ 9 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   matrix[ 2 ] = input[ 2 ];
   // matrix[ 3 ] is never used
   matrix[ 4 ] = input[ 3 ];
   matrix[ 5 ] = input[ 4 ];
   // matrix[ 6 ] is never used
   // matrix[ 7 ] is never used
   matrix[ 8 ] = input[ 5 ];
   SymmetricEigenValues3D( matrix, lambdas );
}

/// \brief Finds the eigenvalues and eigenvectors of a 3D symmetric matrix.
///
/// `input` is a pointer to 9 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for 9 values and will receive the 3 eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ 3 ]` and `&vectors[ 6 ]`.
void SymmetricEigenSystem3D( double const* input, double* lambdas, double* vectors );

/// \brief Finds the eigenvalues and eigenvectors of a 3D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 6 values: { xx, xy, xz, yy, yz, zz }.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for 9 values and will receive the 3 eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ 3 ]` and `&vectors[ 6 ]`.
inline void SymmetricEigenSystem3DPacked( double const* input, double* lambdas, double* vectors ) {
   double matrix[ 9 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   matrix[ 2 ] = input[ 2 ];
   // matrix[ 3 ] is never used
   matrix[ 4 ] = input[ 3 ];
   matrix[ 5 ] = input[ 4 ];
   // matrix[ 6 ] is never used
   // matrix[ 7 ] is never used
   matrix[ 8 ] = input[ 5 ];
   SymmetricEigenSystem3D( matrix, lambdas, vectors );
}


/// \brief Finds the eigenvalues of a square matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
void EigenValues( dip::uint n, double const* input, dcomplex* lambdas );

/// \brief Finds the eigenvalues of a square complex matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
void EigenValues( dip::uint n, dcomplex const* input, dcomplex* lambdas );

/// \brief Finds the eigenvalues and eigenvectors of a square matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for `n`*`n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
void EigenSystem( dip::uint n, double const* input, dcomplex* lambdas, dcomplex* vectors );

/// \brief Finds the eigenvalues and eigenvectors of a square complex matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
/// `vectors` is a pointer to space for `n`*`n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
void EigenSystem( dip::uint n, dcomplex const* input, dcomplex* lambdas, dcomplex* vectors );


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
/// \see VarianceAccumulator
///
/// ###Source
///
/// Code modified from <a href="http://www.johndcook.com/blog/skewness_kurtosis/">John D. Cook</a>,
/// but the same code appears on <a href="https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance">Wikipedia</a>.
/// Method for 3rd and 4th order moments was first published by:
///    <a href="http://people.xiph.org/~tterribe/notes/homs.html">T. B. Terriberry,
///    "Computing higher-order moments online", 2008</a>.
/// For more information:
///    <a href="http://infoserve.sandia.gov/sand_doc/2008/086212.pdf">Philippe P. Pébay, "Formulas for Robust,
///    One-Pass Parallel Computation of Covariances and Arbitrary-Order Statistical Moments",
///    Technical Report SAND2008-6212, Sandia National Laboratories, September 2008</a>.
///
/// Computation of statistics from moments according to Wikipedia:
///    <a href="https://en.wikipedia.org/wiki/Skewness#Sample_skewness">Skewness</a> and
///    <a href="https://en.wikipedia.org/wiki/Kurtosis#Estimators_of_population_kurtosis">Kurtosis</a>.
class StatisticsAccumulator {
   public:
      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         ++n_;
         dfloat delta = x - m1_;
         dfloat term1 = delta / n_;
         dfloat term2 = term1 * term1;
         dfloat term3 = delta * term1 * ( n_ - 1 );
         m4_ += term3 * term2 * ( n_ * n_ - 3.0 * n_ + 3 ) + 6.0 * term2 * m2_ - 4.0 * term1 * m3_;
         m3_ += term3 * term1 * ( n_ - 2 ) - 3.0 * term1 * m2_; // old value used for m4_ calculation
         m2_ += term3; // old value used for m3_ and m4_ calculation.
         m1_ += term1;
      }

      /// Combine two accumulators
      StatisticsAccumulator& operator+=( StatisticsAccumulator const& b ) {
         dip::uint an = n_;
         dip::uint an2 = an * an;
         dip::uint bn2 = b.n_ * b.n_;
         dip::uint xn2 = an * b.n_;
         n_ += b.n_;
         dip::uint n2 = n_ * n_;
         dfloat delta = b.m1_ - m1_;
         dfloat delta2 = delta * delta;
         m4_ += b.m4_ + delta2 * delta2 * xn2 * ( an2 - xn2 + bn2 ) / ( n2 * n_ )
               + 6.0 * delta2 * ( an2 * b.m2_ + bn2 * m2_ ) / n2
               + 4.0 * delta * ( an * b.m3_ - b.n_ * m3_ ) / n_;
         m3_ += b.m3_ + delta * delta2 * xn2 * ( an - b.n_ ) / n2
                + 3.0 * delta * ( an * b.m2_ - b.n_ * m2_ ) / n_;
         m2_ += b.m2_ + delta2 * xn2 / n_;
         m1_ = ( an * m1_ + b.n_ * b.m1_ ) / n_;
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
         return ( n_ > 1 ) ? ( m2_ / ( n_ - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         return std::sqrt( Variance() );
      }
      /// \brief Estimator of population skewness. This estimator is unbiased only for symetric distributions
      /// (it is not possible to derive an unbiased estimator).
      dfloat Skewness() const {
         if(( n_ > 2 ) && ( m2_ != 0 )) {
            dfloat n = n_;
            return (( n * n ) / (( n - 1 ) * ( n - 2 ))) * ( m3_ / ( n * std::pow( Variance(), 1.5 )));
         }
         return 0;
      }
      /// \brief Estimator of population excess kurtosis. This estimator is only unbiased for normally
      /// distributed data (it is not possible to derive an unbiased estimator).
      dfloat ExcessKurtosis() const {
         if( n_ > 3 && ( m2_ != 0 )) {
            dfloat n = n_;
            return ( n - 1 ) / (( n - 2 ) * ( n - 3 )) * (( n + 1 ) * n * m4_ / ( m2_ * m2_ ) - 3 * ( n - 1 ));
         }
         return 0;
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dfloat m1_ = 0;   // mean of values x
      dfloat m2_ = 0;   // sum of (x-mean(x))^2  --  `m2_ / n_` is second order central moment
      dfloat m3_ = 0;   // sum of (x-mean(x))^3  --  `m3_ / n_` is third order central moment
      dfloat m4_ = 0;   // sum of (x-mean(x))^4  --  `m4_ / n_` is fourth order central moment
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
/// \see StatisticsAccumulator
///
/// ### Source
///
/// Donald E. Knuth, "The Art of Computer Programming, Volume 2: Seminumerical Algorithms", 3rd Ed., 1998.
class VarianceAccumulator {
   public:
      /// Add a sample to the accumulator
      void Push( dfloat x ) {
         ++n_;
         dfloat delta = x - m1_;
         m1_ += delta / n_;
         m2_ += delta * ( x - m1_ );
      }

      /// Combine two accumulators
      VarianceAccumulator& operator+=( VarianceAccumulator const& b ) {
         dip::uint oldn = n_;
         n_ += b.n_;
         dfloat delta = b.m1_ - m1_;
         m1_ = ( oldn * m1_ + b.n_ * b.m1_ ) / n_;
         m2_ += b.m2_ + delta * delta * ( oldn * b.n_ ) / n_;
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
         return ( n_ > 1 ) ? ( m2_ / ( n_ - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         return std::sqrt( Variance() );
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dfloat m1_ = 0;   // mean of values x
      dfloat m2_ = 0;   // sum of (x-mean(x))^2  --  `m2_ / n_` is second order central moment
};

/// \brief Combine two accumulators
inline VarianceAccumulator operator+( VarianceAccumulator lhs, VarianceAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \}

} // namespace dip

#endif // DIP_NUMERIC_H
