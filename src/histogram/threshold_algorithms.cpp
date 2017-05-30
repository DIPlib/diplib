/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement threshold estimation algorithms.
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

#include "diplib.h"
#include "diplib/histogram.h"


namespace dip {

namespace {

// Translate thresholds from bin indices to intensities (using linear interpolation)
dfloat FindBin( FloatArray bins, dfloat threshold ) {
   if( threshold <= 0 ) {
      return bins.front();
   } else if( threshold >= static_cast< dfloat >( bins.size() - 1 )) {
      return bins.back();
   } else {
      dfloat frac = threshold - std::floor( threshold );
      return bins[ static_cast< dip::uint >( threshold ) ] * ( 1.0 - frac ) +
             bins[ static_cast< dip::uint >( threshold ) + 1 ] * frac;
   }
}

} // namespace

FloatArray IsodataThreshold(
      Histogram const& in,
      dip::uint nThresholds
) {
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_UINT32 );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   DIP_THROW_IF( hist.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint nBins = hist.Size( 0 );
   FloatArray thresholds( nThresholds );
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      thresholds[ ii ] = static_cast< dfloat >(( ii + 1 ) * nBins ) / static_cast< dfloat >( nThresholds + 1 );
   }
   FloatArray old;
   uint32 const* data = static_cast< uint32 const* >( hist.Origin() );
   do {
      old = thresholds;
      dip::uint origin1 = 0;
      dip::uint origin2;
      FloatArray centers( nThresholds + 1 );
      for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
         origin2 = static_cast< dip::uint >( std::ceil( thresholds[ ii ] ));
         dfloat moment = 0;
         dfloat sum = 0;
         for( dip::uint jj = origin1; jj < origin2; ++jj ) {
            moment += static_cast< dfloat >( jj ) * data[ jj ];
            sum += data[ jj ];
         }
         centers[ ii ] = moment / sum;
         origin1 = origin2;
      }
      dfloat moment = 0;
      dfloat sum = 0;
      for( dip::uint jj = origin1; jj < nBins; ++jj ) {
         moment += static_cast< dfloat >( jj ) * data[ jj ];
         sum += data[ jj ];
      }
      centers.back() = moment / sum;
      for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
         thresholds[ ii ] = ( centers[ ii + 1 ] + centers[ ii ] ) / 2.0;
      }
   } while( thresholds != old );
   // Translate thresholds from bin indices to intensities (using linear interpolation)
   FloatArray bins = in.BinCenters();
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      thresholds[ ii ] = FindBin( bins, thresholds[ ii ] );
   }
   return thresholds;
}

dfloat OtsuThreshold(
      Histogram const& in
) {
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged());
   DIP_ASSERT( hist.DataType() == DT_UINT32 );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   DIP_THROW_IF( hist.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint nBins = hist.Size( 0 );
   FloatArray bins = in.BinCenters();
   auto histIt = ImageIterator< uint32 >( hist );
   auto histEnd = ImageIterator< uint32 >();
   auto binIt = bins.begin();
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bins(ii) and bins(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( histIt, histEnd, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = std::inner_product( histIt, histEnd, binIt, 0.0 );
   // Here we accumulate the max.
   dfloat ssMax = -1e6;
   dip::uint maxInd = 0;
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      w1 += *histIt;
      w2 -= *histIt;
      dfloat tmp = *histIt * *binIt;
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
      ++histIt;
      ++binIt;
   }
   DIP_THROW_IF( ssMax == -1e6, "Could not find a maximum in Otsu's measure for inter-class variance" );
   return ( bins[ maxInd ] + bins[ maxInd + 1 ] ) / 2.0;
}

dfloat MinimumErrorThreshold(
      Histogram const& in
) {
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged());
   DIP_ASSERT( hist.DataType() == DT_UINT32 );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   DIP_THROW_IF( hist.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint nBins = hist.Size( 0 );
   FloatArray bins = in.BinCenters();
   auto histIt = ImageIterator< uint32 >( hist );
   auto histEnd = ImageIterator< uint32 >();
   auto binIt = bins.begin();
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bins(ii) and bins(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( histIt, histEnd, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = std::inner_product( histIt, histEnd, binIt, 0.0 );
   // Here we accumulate the error measure.
   FloatArray J( nBins - 1 );
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      w1 += *histIt;
      w2 -= *histIt;
      dfloat tmp = *histIt * *binIt;
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      // v1(ii), v2(ii) are the corresponding second order central moments
      dfloat v1 = 0;
      auto it = ImageIterator< uint32 >( hist );
      for( dip::uint jj = 0; jj <= ii; ++jj ) {
         dfloat d = bins[ jj ] - c1;
         v1 += *it * d * d;
         ++it;
      }
      v1 /= w2;
      dfloat v2 = 0;
      for( dip::uint jj = ii + 1; jj < nBins; ++jj ) {
         dfloat d = bins[ jj ] - c2;
         v2 += *it * d * d;
         ++it;
      }
      v2 /= w2;
      // J(ii) is the measure for error
      J[ ii ] = 1.0 + w1 * std::log( v1 ) + w2 * std::log( v2 ) - 2.0 * ( w1 * std::log( w1 ) + w2 * std::log( w2 ));
      ++histIt;
      ++binIt;
   }
   // Now we need to find the minimum in J, but ignore the values at the edges, if they are lowest.
   dip::uint begin = 0;
   dip::uint end = nBins;
   for( ; begin < end - 1; ++begin ) {
      if( J[ begin ] > J[ begin + 1 ] ) {
         break;
      }
   }
   for( ; begin < end - 1; --end ) {
      if( J[ end ] > J[ end - 1 ] ) {
         break;
      }
   }
   dfloat minJ = J[ begin ];
   dip::uint minInd = begin;
   for( dip::uint ii = begin + 1; ii < end; ++ii ) {
      if( J[ ii ] < minJ ) {
         minJ = J[ ii ];
         minInd = ii;
      }
   }
   return ( bins[ minInd ] + bins[ minInd + 1 ] ) / 2.0;
}

dfloat TriangleThreshold(
      Histogram const& in
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

} // namespace dip

