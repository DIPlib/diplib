/*
 * (c)2019-2022, Cris Luengo.
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

#include "diplib/library/numeric.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <vector>

#include "diplib.h"
#include "diplib/random.h"

namespace dip {

std::vector< GaussianParameters > GaussianMixtureModel(
      ConstSampleIterator< dfloat > data,
      SampleIterator< dfloat > responsibilities,
      dip::uint size,
      dip::uint numberOfGaussians,
      dip::uint maxIter,
      Option::Periodicity periodicity
) {
   DIP_THROW_IF( numberOfGaussians < 1, E::INVALID_PARAMETER );

   // Initialize parameters
   std::vector< GaussianParameters > params( numberOfGaussians );
   dfloat sz = static_cast< dfloat >( size );
   dfloat hsz = 0.5 * sz;
   dfloat ssz = sz / static_cast< dfloat >( numberOfGaussians + 1 );
   for( dip::uint ii = 0; ii < numberOfGaussians; ++ii ) {
      params[ ii ].position = static_cast< dfloat >( ii + 1 ) * ssz;
      params[ ii ].amplitude = 1;
      params[ ii ].sigma = ssz;
   }
   bool isPeriodic = periodicity == Option::Periodicity::PERIODIC;

   // Indicators
   SampleIterator< dfloat > indicators = responsibilities;
   std::vector< dfloat > localResponsibilities;
   if( !indicators ) { // use a local buffer for this
      localResponsibilities.resize( size * numberOfGaussians );
      indicators = localResponsibilities.data();
   }

   // Iterations
   for( dip::uint ii = 0; ii < maxIter; ++ii ) {
      // The E (expectation) step (computing indicators == responsibilities)
      for( dip::uint jj = 0; jj < size; ++jj ) {
         dfloat sum = 0.0;
         SampleIterator< dfloat > tp = indicators + jj;
         for( dip::uint kk = 0; kk < numberOfGaussians; ++kk ) {
            dfloat dif = static_cast< dfloat >( jj ) - params[ kk ].position;
            dif = isPeriodic ? std::min( std::min( std::abs( dif ), std::abs( dif - sz )), std::abs( dif + sz ))
                             : std::abs( dif );
            if( params[ kk ].sigma > 0 ) {
               *tp = params[ kk ].amplitude * std::exp(( -0.5 * dif * dif ) / ( params[ kk ].sigma * params[ kk ].sigma ));
            } else {
               *tp = 0;
            }
            sum += *tp;
            tp += size;
         }
         tp = indicators + jj;
         if( sum != 0 ) {
            for( dip::uint kk = 0; kk < numberOfGaussians; ++kk ) {
               *tp /= sum;
               tp += size;
            }
         } else {
            for( dip::uint kk = 0; kk < numberOfGaussians; ++kk ) {
               *tp = 1.0 / static_cast< dfloat >( numberOfGaussians );
               tp += size;
            }
         }
      }

      // The M (maximization) step (computing parameters)
      {
         SampleIterator< dfloat > tp = indicators;
         for( dip::uint kk = 0; kk < numberOfGaussians; ++kk ) {
            dfloat oldPos = params[ kk ].position;
            params[ kk ].position = 0.0;
            params[ kk ].amplitude = 0.0;
            dfloat norm = 0.0;
            for( dip::uint jj = 0; jj < size; ++jj ) {
               dfloat xf = static_cast< dfloat >( jj );
               if( isPeriodic ) {
                  if( xf - oldPos > hsz ) {
                     xf -= sz;
                  } else if( xf - oldPos < -hsz ) {
                     xf += sz;
                  }
               }
               dfloat w = data[ jj ] * tp[ jj ];
               params[ kk ].amplitude += w;
               params[ kk ].position += w * xf;
               norm += w;
            }
            if( norm != 0 ) {
               params[ kk ].position /= norm;
            }
            if( isPeriodic ) {
               if( params[ kk ].position < 0 ) {
                  params[ kk ].position += sz;
               }
               if( params[ kk ].position >= sz ) {
                  params[ kk ].position -= sz;
               }
            }
            params[ kk ].sigma = 0.0;
            norm = 0.0;
            for( dip::uint jj = 0; jj < size; ++jj ) {
               dfloat dif = static_cast< dfloat >( jj ) - params[ kk ].position;
               dif = isPeriodic ? std::min( std::min( std::abs( dif ), std::abs( dif - sz )), std::abs( dif + sz ))
                                : std::abs( dif );
               dfloat w = data[ jj ] * tp[ jj ];
               params[ kk ].sigma += w * dif * dif;
               norm += w;
            }
            if( norm != 0 ) {
               params[ kk ].sigma /= norm;
            }
            params[ kk ].sigma = std::sqrt( std::abs( params[ kk ].sigma ));
            if( params[ kk ].sigma != 0 ) {
               params[ kk ].amplitude /= params[ kk ].sigma * std::sqrt( 2.0 * dip::pi );
            }
            tp += size;
         }
      }
      // TODO: compute log likelihood and break if it doesn't change significantly
   }

   // Sort Gaussians by amplitude
   std::sort( params.begin(), params.end(), []( GaussianParameters const& a, GaussianParameters const& b ) { return a.amplitude > b.amplitude; } );

   return params;
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"

DOCTEST_TEST_CASE("[DIPlib] testing GaussianMixtureModel") {
   dip::Random random( 0 );

   dip::uint N = 300;   // image size
   dip::Image input( { N }, 1, dip::DT_DFLOAT );
   input = 0;
   dip::DrawBandlimitedPoint( input, {  50.0 }, { 150.0 }, { 15.0 }, 5.0 );
   dip::DrawBandlimitedPoint( input, { 250.0 }, { 100.0 }, { 15.0 }, 5.0 );
   dip::DrawBandlimitedPoint( input, { 100.0 }, {  30.0 }, { 20.0 }, 5.0 );
   dip::GaussianNoise( input, input, random, 0.0002 );
   dip::uint M = 3;     // number of Gaussians
   auto params = dip::GaussianMixtureModel( static_cast< dip::dfloat* >( input.Origin() ), {}, N, M, 20, dip::Option::Periodicity::NOT_PERIODIC );
   //std::cout << "[gaussian #1] position = " << params[0].position << ", amplitude = " << params[0].amplitude << ", sigma = " << params[0].sigma << '\n';
   //std::cout << "[gaussian #2] position = " << params[1].position << ", amplitude = " << params[1].amplitude << ", sigma = " << params[1].sigma << '\n';
   //std::cout << "[gaussian #3] position = " << params[2].position << ", amplitude = " << params[2].amplitude << ", sigma = " << params[2].sigma << '\n';

   DOCTEST_REQUIRE( params.size() == 3 );
   DOCTEST_CHECK( std::abs( params[ 0 ].position -  50.0 ) < 0.6 );
   DOCTEST_CHECK( std::abs( params[ 1 ].position - 250.0 ) < 0.5 );
   DOCTEST_CHECK( std::abs( params[ 2 ].position - 100.0 ) < 3.1 ); // 3rd peak is weak and less precise
   DOCTEST_CHECK( std::abs( params[ 0 ].sigma - 15.0 ) < 0.5 );
   DOCTEST_CHECK( std::abs( params[ 1 ].sigma - 15.0 ) < 0.5 );
   DOCTEST_CHECK( std::abs( params[ 2 ].sigma - 20.0 ) < 2.1 );
   DOCTEST_CHECK( std::abs( params[ 0 ].amplitude - 150.0 / ( 15.0 * std::sqrt( 2.0 * dip::pi ))) < 0.1 );
   DOCTEST_CHECK( std::abs( params[ 1 ].amplitude - 100.0 / ( 15.0 * std::sqrt( 2.0 * dip::pi ))) < 0.1 );
   DOCTEST_CHECK( std::abs( params[ 2 ].amplitude -  30.0 / ( 20.0 * std::sqrt( 2.0 * dip::pi ))) < 0.1 );

   input = 0;
   dip::DrawBandlimitedPoint( input, {   0.0 }, { 100.0 }, { 15.0 }, 5.0 );
   dip::DrawBandlimitedPoint( input, { 300.0 }, { 100.0 }, { 15.0 }, 5.0 ); // 300.0 == N!
   dip::DrawBandlimitedPoint( input, { 100.0 }, {  30.0 }, { 20.0 }, 5.0 );
   dip::GaussianNoise( input, input, random, 0.0002 );
   M = 2;     // number of Gaussians
   params = dip::GaussianMixtureModel( static_cast< dip::dfloat* >( input.Origin() ), {}, N, M, 20, dip::Option::Periodicity::PERIODIC );
   //std::cout << "[gaussian #1] position = " << params[0].position << ", amplitude = " << params[0].amplitude << ", sigma = " << params[0].sigma << '\n';
   //std::cout << "[gaussian #2] position = " << params[1].position << ", amplitude = " << params[1].amplitude << ", sigma = " << params[1].sigma << '\n';

   DOCTEST_REQUIRE( params.size() == 2 );
   if( params[ 0 ].position < 150.0 ) {
      DOCTEST_CHECK( std::abs( params[ 0 ].position - 0.0 ) < 0.5 );
   } else {
      DOCTEST_CHECK( std::abs( params[ 0 ].position - 300.0 ) < 0.5 );
   }
   DOCTEST_CHECK( std::abs( params[ 1 ].position - 100.0 ) < 0.75 );
   DOCTEST_CHECK( std::abs( params[ 0 ].sigma - 15.0 ) < 0.5 );
   DOCTEST_CHECK( std::abs( params[ 1 ].sigma - 20.0 ) < 1.5 );
   DOCTEST_CHECK( std::abs( params[ 0 ].amplitude - 100.0 / ( 15.0 * std::sqrt( 2.0 * dip::pi ))) < 0.1 );
   DOCTEST_CHECK( std::abs( params[ 1 ].amplitude -  30.0 / ( 20.0 * std::sqrt( 2.0 * dip::pi ))) < 0.1 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
