/*
 * DIPlib 3.0
 * This file contains various classes for on-line computation of data statistics.
 *
 * (c)2017, Cris Luengo.
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


#ifndef DIP_ACCUMULATORS_H
#define DIP_ACCUMULATORS_H

#include <cmath>
#include <numeric>
#include <limits>

#include "diplib/library/types.h"


/// \file
/// \brief Classes for on-line computation of data statistics.
/// \see numeric


namespace dip {


/// \addtogroup numeric
/// \{


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
/// \see VarianceAccumulator, FastVarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// \literature
/// <li>Code modified from [John D. Cook](http://www.johndcook.com/blog/skewness_kurtosis/)
///     ([Wikipedia](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance) has the same code).
/// <li>T. B. Terriberry, ["Computing higher-order moments online"](http://people.xiph.org/~tterribe/notes/homs.html), 2008.
/// <li>Philippe P. Pébay, "Formulas for Robust, One-Pass Parallel Computation of Covariances and Arbitrary-Order Statistical Moments",
///     Technical Report [SAND2008-6212](http://infoserve.sandia.gov/sand_doc/2008/086212.pdf), Sandia National Laboratories, September 2008.
/// <li>Wikipedia: ["Skewness", section "Sample skewness"](https://en.wikipedia.org/wiki/Skewness#Sample_skewness).
/// <li>Wikipedia: ["Kurtosis", section "Estimators of population kurtosis"](https://en.wikipedia.org/wiki/Kurtosis#Estimators_of_population_kurtosis).
/// \endliterature
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
/// \relates dip::StatisticsAccumulator
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
/// uses a stable algorithm to prevent catastrophic cancellation. If catastrophic cancellation is unlikely
/// or not important, use the faster `%dip::FastVarianceAccumulator`.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// It is also possible to remove a sample from the accumulator, using the `Pop` method. It is assumed that the
/// particular value passed to this method had been added previously to the accumulator. If this is not the case,
/// resulting means and variances are no longer correct.
///
/// \see StatisticsAccumulator, FastVarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// \literature
/// <li>Donald E. Knuth, "The Art of Computer Programming, Volume 2: Seminumerical Algorithms", 3<sup>rd</sup> Ed., 1998.
/// \endliterature
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
/// \relates dip::VarianceAccumulator
inline VarianceAccumulator operator+( VarianceAccumulator lhs, VarianceAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%FastVarianceAccumulator` computes mean and standard deviation by accumulating the sum of sample
/// values and the sum of the square of sample values.
///
/// Samples are added one by one, using the `Push` method. Other members are used to retrieve estimates of
/// the population statistics based on the samples seen up to that point. Formula used to compute population
/// statistics are corrected, though the standard deviation is not an unbiased estimator. The accumulator
/// uses a simple algorithm that could result in catastrophic cancellation if the variance is very small with
/// respect to the mean; use `%dip::VarianceAccumulator` to prevent it.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// It is also possible to remove a sample from the accumulator, using the `Pop` method. It is assumed that the
/// particular value passed to this method had been added previously to the accumulator. If this is not the case,
/// resulting means and variances are no longer correct.
///
/// \see StatisticsAccumulator, VarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
class DIP_NO_EXPORT FastVarianceAccumulator {
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
         m1_ += x;
         m2_ += x * x;
      }

      /// Remove a sample from the accumulator
      void Pop( dfloat x ) {
         if( n_ > 0 ) {
            --n_;
            m1_ -= x;
            m2_ -= x * x;
         }
      }

      /// Combine two accumulators
      FastVarianceAccumulator& operator+=( FastVarianceAccumulator const& b ) {
         n_ += b.n_;
         m1_ += b.m1_;
         m2_ += b.m2_;
         return *this;
      }

      /// Number of samples
      dip::uint Number() const {
         return n_;
      }
      /// Unbiased estimator of population mean
      dfloat Mean() const {
         return m1_ / static_cast< dfloat >( n_ );
      }
      /// Unbiased estimator of population variance
      dfloat Variance() const {
         dfloat n = static_cast< dfloat >( n_ );
         return ( n_ > 1 ) ? (( m2_ - ( m1_ * m1_ ) / n ) / ( n - 1 )) : ( 0.0 );
      }
      /// Estimator of population standard deviation (it is not possible to derive an unbiased estimator)
      dfloat StandardDeviation() const {
         return std::sqrt( Variance() );
      }

   private:
      dip::uint n_ = 0; // number of values x collected
      dfloat m1_ = 0.0; // sum of x
      dfloat m2_ = 0.0; // sum of x^2
};

/// \brief Combine two accumulators
/// \relates dip::FastVarianceAccumulator
inline FastVarianceAccumulator operator+( FastVarianceAccumulator lhs, FastVarianceAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \brief `%CovarianceAccumulator` computes covariance and correlation of pairs of samples by accumulating the
/// first two central moments and cross-moments.
///
/// Samples are added one pair at the time, using the `Push` method. Other members are used to retrieve the results.
/// The accumulator uses a stable algorithm to prevent catastrophic cancellation.
///
/// The covariance matrix is formed by
///
/// ```none
///    | cov.VarianceX()   cov.Covariance() |
///    | cov.Covariance()  cov.VarianceY()  |
/// ```
///
/// The `Regression` method returns the parameters to the least squares fit of the equation:
///
/// ```none
///    y = intercept + slope * x
/// ```
///
/// where `x` is the first sample in each pair, and `y` is the second (this is linear regression). The `Slope` method
/// computes only the slope component.
///
/// It is possible to accumulate samples in different objects (e.g. when processing with multiple threads),
/// and add the accumulators together using the `+` operator.
///
/// \see StatisticsAccumulator, VarianceAccumulator, FastVarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator, MomentAccumulator
///
/// \literature
/// <li>Wikipedia: ["Algorithms for calculating variance", section "Covariance"](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Covariance).
/// \endliterature
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
      /// Copying type for backward compatability.
      using RegressionResult = dip::RegressionParameters;
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
/// \see StatisticsAccumulator, VarianceAccumulator, FastVarianceAccumulator, CovarianceAccumulator, MinMaxAccumulator, MomentAccumulator
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
/// \relates dip::DirectionalStatisticsAccumulator
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
/// \see StatisticsAccumulator, VarianceAccumulator, FastVarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MomentAccumulator
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
/// \relates dip::MinMaxAccumulator
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
/// \see StatisticsAccumulator, VarianceAccumulator, FastVarianceAccumulator, CovarianceAccumulator, DirectionalStatisticsAccumulator, MinMaxAccumulator
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
      /// \f[ I = \Sigma_k m_k ((\vec{r_k} \cdot \vec{r_k}) E - \vec{r_k} \otimes \vec{r_k}) \f]
      ///
      /// where \f$E\f$ is the identity matrix ( \f$ E = \Sigma_i \vec{e_i} \otimes \vec{e_i} \f$ ), \f$m_k\f$
      /// is the weight of point \f$k\f$ , and \f$\vec{r_k}\f$ is its position. In 2D, this leads to:
      ///
      /// \f{eqnarray*}{
      ///       I_{xx} & = & \mathbin{\phantom{-}}\Sigma_k m_k y^2
      ///    \\ I_{yy} & = & \mathbin{\phantom{-}}\Sigma_k m_k x^2
      ///    \\ I_{xy} & = &                   -  \Sigma_k m_k x y
      /// \f}
      ///
      /// In 3D, it leads to:
      ///
      /// \f{eqnarray*}{
      ///       I_{xx} & = & \mathbin{\phantom{-}}\Sigma_k m_k y^2 + \Sigma_k m_k z^2
      ///    \\ I_{yy} & = & \mathbin{\phantom{-}}\Sigma_k m_k x^2 + \Sigma_k m_k z^2
      ///    \\ I_{zz} & = & \mathbin{\phantom{-}}\Sigma_k m_k x^2 + \Sigma_k m_k y^2
      ///    \\ I_{xy} & = &                   -  \Sigma_k m_k x y
      ///    \\ I_{xz} & = &                   -  \Sigma_k m_k x z
      ///    \\ I_{yz} & = &                   -  \Sigma_k m_k y z
      /// \f}
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
/// \relates dip::MomentAccumulator
inline MomentAccumulator operator+( MomentAccumulator lhs, MomentAccumulator const& rhs ) {
   lhs += rhs;
   return lhs;
}


/// \}

} // namespace dip

#endif // DIP_ACCUMULATORS_H
