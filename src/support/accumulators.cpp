/*
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

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

#include "diplib/accumulators.h"

#include <algorithm>
#include <cmath>

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/random.h"
#include "diplib/transform.h"

DOCTEST_TEST_CASE("[DIPlib] testing the statistical accumulators") {
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
      dip::FastVarianceAccumulator acc1;
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
      dip::FastVarianceAccumulator acc2;
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

DOCTEST_TEST_CASE("[DIPlib] testing the PRNG") {
   dip::Random rng( 0 );
   bool error = false;
#if defined(__SIZEOF_INT128__) || defined(DIP_CONFIG_ALWAYS_128_PRNG)
   // 128-bit PRNG has 64-bit output, we expect the following values:
   error |= rng() != 74029666500212977ULL;
   error |= rng() != 8088122161323000979ULL;
   error |= rng() != 16521829690994476282ULL;
   error |= rng() != 10814004662382438494ULL;
#else
   // 64-bit PRNG has 32-bit output, we expect the following values:
   error |= rng() != 3894649422UL;
   error |= rng() != 2055130073UL;
   error |= rng() != 2315086854UL;
   error |= rng() != 2925816488UL;
#endif
   DOCTEST_REQUIRE( !error );

   // Test Advance method
   dip::Random rng2 = rng;
   rng2.Advance( 10 );
   for( dip::uint ii = 0; ii < 10; ++ii ) { rng(); }
   DOCTEST_CHECK( rng() == rng2() );

   // Size of tests
   constexpr dip::uint N = 10000; // Ideally we'd use a larger set (previously this was an order of
   //                                magnitude larger), but it takes too much time.

   // Test uniform distribution
   dip::UniformRandomGenerator uniform( rng );
   dip::VarianceAccumulator acc_uniform;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc_uniform.Push( uniform( 2.0, 4.0 ));
   }
   DOCTEST_CHECK( std::abs( acc_uniform.Mean() - 3.0 ) < 0.01 );
   DOCTEST_CHECK( std::abs( acc_uniform.Variance() - 1.0 / 3.0 ) < 0.02 );

   // Test normal distribution
   dip::GaussianRandomGenerator normal( rng );
   dip::VarianceAccumulator acc_normal;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc_normal.Push( normal( 2.0, 4.0 ));
   }
   DOCTEST_CHECK( std::abs( acc_normal.Mean() - 2.0 ) < 0.04 );
   DOCTEST_CHECK( std::abs( acc_normal.StandardDeviation() - 4.0 ) < 0.05 );

   // Test poisson distribution
   dip::PoissonRandomGenerator poisson( rng );
   dip::VarianceAccumulator acc_poisson;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc_poisson.Push( static_cast< dip::dfloat >( poisson( 2.0 )));
   }
   DOCTEST_CHECK( std::abs( acc_poisson.Mean() - 2.0 ) < 0.04 );
   DOCTEST_CHECK( std::abs( acc_poisson.Variance() - 2.0 ) < 0.02 );
   dip::VarianceAccumulator acc2_poisson;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc2_poisson.Push( static_cast< dip::dfloat >( poisson( 2000.0 )));
   }
   DOCTEST_CHECK( std::abs( acc2_poisson.Mean() - 2000.0 ) < 1.0 );
   DOCTEST_CHECK( std::abs( acc2_poisson.Variance() - 2000.0 ) < 20.0 );

   // Test binary distribution
   dip::BinaryRandomGenerator binary( rng );
   dip::uint count = 0;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      if( binary( 0.5 )) {
         ++count;
      }
   }
   DOCTEST_CHECK( std::abs( static_cast< dip::sint >( count ) - static_cast< dip::sint >( N / 2 )) < 1000 );

   // Test Split method to produce uncorrelated sequence, and test autocorrelation function also
   rng2 = rng.Split();
   dip::GaussianRandomGenerator normal2( rng2 );
   dip::Image img1( { N }, 1, dip::DT_DFLOAT );
   dip::Image img2( { N }, 1, dip::DT_DFLOAT );
   dip::dfloat* data1 = static_cast< dip::dfloat* >( img1.Origin() );
   dip::dfloat* data2 = static_cast< dip::dfloat* >( img2.Origin() );
   for( dip::uint ii = 0; ii < N; ++ii ) {
      *( data1++ ) = normal( 0.0, 1.0 );
      *( data2++ ) = normal2( 0.0, 1.0 );
   }
   dip::Image FT1 = dip::FourierTransform( img1, { "corner" } );
   dip::Image FT2 = dip::FourierTransform( img2, { "corner" } );
   img1 = dip::FourierTransform( dip::SquareModulus( FT1 ), { "corner", "inverse", "real" } );
   img2 = dip::FourierTransform( dip::MultiplyConjugate( FT1, FT2 ), { "corner", "inverse", "real" } );
   DOCTEST_REQUIRE( img1.DataType() == dip::DT_DFLOAT );
   DOCTEST_REQUIRE( img2.DataType() == dip::DT_DFLOAT );
   data1 = static_cast< dip::dfloat* >( img1.Origin() );
   data2 = static_cast< dip::dfloat* >( img2.Origin() );
   dip::dfloat norm = std::abs( *( data1++ )); // value of auto-correlation for 0 shift
   dip::dfloat max = std::abs( *( data2++ ));
   for( dip::uint ii = 1; ii < N; ++ii ) {
      max = std::max( max, std::abs( *( data1++ ))); // max value of auto-correlation
      max = std::max( max, std::abs( *( data2++ ))); // max value of cross-correlation
   }
   DOCTEST_CHECK( max < norm / 20.0 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
