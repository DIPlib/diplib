/*
* (c)2025, Cris Luengo.
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

#ifndef DIP_THRESHOLD_ALGORITHMS_H
#define DIP_THRESHOLD_ALGORITHMS_H

#include "diplib.h"

namespace dip {
namespace {

// *data is the histogram data, here must be nBins values. Can be any type, can be normalized.
// If the histogram can't be split, the return value is nBins (error condition, must check!).
// Otherwise returned value is always < nBins - 1, and is the last bin of the first half of the histogram.
template< typename TPI >
dip::uint OtsuThreshold( TPI const* data, dip::uint nBins ) {
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between ii and ii+1
   dfloat w1 = 0;
   dfloat w2 = 0;
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = 0;
   for( dip::uint ii = 0; ii < nBins; ++ii ) {
      w2 += static_cast< dfloat >( data[ ii ] );
      m2 += static_cast< dfloat >( data[ ii ] ) * static_cast< dfloat >( ii );
   }
   // Here we accumulate the max.
   dfloat ssMax = -1e6;
   dip::uint maxInd = 0;
   for( dip::uint ii = 0; ii < nBins - 1; ++ii, ++data ) {
      dfloat tmp = static_cast< dfloat >( *data );
      w1 += tmp;
      w2 -= tmp;
      tmp *= static_cast< dfloat >( ii );
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      dfloat c = c1 - c2;
      // ss(ii) is Otsu's measure for inter-class variance
      dfloat ss = w1 * w2 * c * c;
      if( ss > ssMax ) {
         ssMax = ss;
         maxInd = ii;
      }
   }
   return ( ssMax == -1e6 ) ? nBins : maxInd;
}

} // namespace
} // namespace dip

#endif //DIP_THRESHOLD_ALGORITHMS_H
