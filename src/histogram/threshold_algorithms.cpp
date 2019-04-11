/*
 * DIPlib 3.0
 * This file contains definitions of threshold estimation algorithms.
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
#include "diplib/segmentation.h"
#include "diplib/statistics.h"
#include "diplib/chain_code.h" // for VertexFloat, TriangleHeight, etc.


namespace dip {

using CountType = Histogram::CountType;

Histogram KMeansClustering(
      Histogram const& in,
      dip::uint nClusters
) {
   Image labs;
   KMeansClustering( in.GetImage(), labs, nClusters );
   Histogram out = in.Copy();
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return out;
}

Histogram MinimumVariancePartitioning(
      Histogram const& in,
      dip::uint nClusters
) {
   Image labs;
   MinimumVariancePartitioning( in.GetImage(), labs, nClusters );
   Histogram out = in.Copy();
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return out;
}

namespace {

// Translate thresholds from bin indices to intensities (using linear interpolation)
dfloat FindBin( FloatArray bins, dfloat threshold ) {
   if( threshold <= 0 ) {
      return bins.front();
   }
   if( threshold >= static_cast< dfloat >( bins.size() - 1 )) {
      return bins.back();
   }
   dfloat frac = threshold - static_cast< dfloat >( static_cast< dip::sint >( threshold )); // casting to round down
   return bins[ static_cast< dip::uint >( threshold ) ] * ( 1.0 - frac ) +
          bins[ static_cast< dip::uint >( threshold ) + 1 ] * frac;
}

} // namespace

FloatArray IsodataThreshold(
      Histogram const& in,
      dip::uint nThresholds
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   FloatArray thresholds( nThresholds );
   // Initialize thresholds such that each interval has approximately same number of pixels
   Histogram cumh = CumulativeHistogram( in );
   Image const& cum = cumh.GetImage();
   DIP_ASSERT( cum.IsForged() );
   DIP_ASSERT( cum.DataType() == DT_COUNT );
   DIP_ASSERT( cum.Stride( 0 ) == 1 );
   DIP_ASSERT( cum.Size( 0 ) == nBins );
   Histogram::CountType* ptr = static_cast< Histogram::CountType* >( cum.Origin() );
   dip::uint N = ptr[ nBins - 1 ] / ( nThresholds + 1 );
   dip::uint index = 1;
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      while( ptr[ index ] < N * ( ii + 1 )) {
         ++index;
      }
      thresholds[ ii ] = static_cast< dfloat >( index );
   }
   // Apply the iterative process
   FloatArray old;
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
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
            moment += static_cast< dfloat >( jj ) * static_cast< dfloat >( data[ jj ] );
            sum += static_cast< dfloat >( data[ jj ] );
         }
         if( sum > 0 ) {
            centers[ ii ] = moment / sum;
         } else {
            centers[ ii ] = static_cast< dfloat >( origin1 + origin2 ) / 2.0;
         }
         origin1 = origin2;
      }
      dfloat moment = 0;
      dfloat sum = 0;
      for( dip::uint jj = origin1; jj < nBins; ++jj ) {
         moment += static_cast< dfloat >( jj ) * static_cast< dfloat >( data[ jj ] );
         sum += static_cast< dfloat >( data[ jj ] );
      }
      if( sum > 0 ) {
         centers.back() = moment / sum;
      } else {
         centers.back() = static_cast< dfloat >( origin1 + nBins ) / 2.0;
      }
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
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   FloatArray bins = in.BinCenters();
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   dfloat const* binPtr = bins.data();
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bins(ii) and bins(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( data, data + nBins, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = std::inner_product( data, data + nBins, binPtr, 0.0 );
   // Here we accumulate the max.
   dfloat ssMax = -1e6;
   dip::uint maxInd = 0;
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      w1 += static_cast< dfloat >( *data );
      w2 -= static_cast< dfloat >( *data );
      dfloat tmp = static_cast< dfloat >( *data ) * *binPtr;
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
      ++data;
      ++binPtr;
   }
   DIP_THROW_IF( ssMax == -1e6, "Could not find a maximum in Otsu's measure for inter-class variance" );
   return ( bins[ maxInd ] + bins[ maxInd + 1 ] ) / 2.0;
}

dfloat MinimumErrorThreshold(
      Histogram const& in
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   FloatArray bins = in.BinCenters();
   Histogram::CountType const* data = static_cast< Histogram::CountType* >( hist.Origin() );
   dfloat const* binPtr = bins.data();
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bins(ii) and bins(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( data, data + nBins, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = std::inner_product( data, data + nBins, binPtr, 0.0 );
   // Here we accumulate the error measure.
   FloatArray J( nBins - 1 );
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      w1 += static_cast< dfloat >( *data );
      w2 -= static_cast< dfloat >( *data );
      dfloat tmp = static_cast< dfloat >( *data ) * *binPtr;
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      // v1(ii), v2(ii) are the corresponding second order central moments
      dfloat v1 = 0;
      Histogram::CountType const* it = static_cast< Histogram::CountType* >( hist.Origin() );
      for( dip::uint jj = 0; jj <= ii; ++jj ) {
         dfloat d = bins[ jj ] - c1;
         v1 += static_cast< dfloat >( *it ) * d * d;
         ++it;
      }
      v1 /= w2;
      dfloat v2 = 0;
      for( dip::uint jj = ii + 1; jj < nBins; ++jj ) {
         dfloat d = bins[ jj ] - c2;
         v2 += static_cast< dfloat >( *it ) * d * d;
         ++it;
      }
      v2 /= w2;
      // J(ii) is the measure for error
      J[ ii ] = 1.0 + w1 * std::log( v1 ) + w2 * std::log( v2 ) - 2.0 * ( w1 * std::log( w1 ) + w2 * std::log( w2 ));
      ++data;
      ++binPtr;
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
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, 4 );
   Image const& hist = smoothIn.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Find the peak
   UnsignedArray maxCoords = MaximumPixel( hist );
   dip::uint maxElement = maxCoords[ 0 ];
   Histogram::CountType maxValue = data[ maxElement ];
   // Define: start, peak, stop positions in histogram
   VertexFloat left_bin{ 0.0, static_cast< dfloat >( data[ 0 ] ) };
   VertexFloat right_bin{ static_cast< dfloat >( nBins - 1 ), static_cast< dfloat >( data[ nBins - 1 ] ) };
   VertexFloat top_bin{ static_cast< dfloat >( maxElement ), static_cast< dfloat >( maxValue ) };
   // Find the location of the maximum distance to the triangle
   dip::uint bin = 0;
   dfloat maxDistance = 0;
   for( dip::uint ii = 1; ii < maxElement; ++ii ) {
      VertexFloat pos{ static_cast< dfloat >( ii ), static_cast< dfloat >( data[ ii ] ) };
      dfloat distance = TriangleHeight( left_bin, top_bin, pos );
      if( distance > maxDistance ) {
         maxDistance = distance;
         bin = ii;
      }
   }
   for( dip::uint ii = maxElement + 1; ii < nBins - 1; ++ii ) {
      VertexFloat pos{ static_cast< dfloat >( ii ), static_cast< dfloat >( data[ ii ] ) };
      dfloat distance = TriangleHeight( right_bin, top_bin, pos );
      if( distance > maxDistance ) {
         maxDistance = distance;
         bin = ii;
      }
   }
   return smoothIn.BinCenters()[ bin ];
}

dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance
) {
   DIP_THROW_IF( distance <= 0, E::INVALID_PARAMETER );
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, 4 );
   Image const& hist = smoothIn.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Find the peak
   UnsignedArray maxCoords = MaximumPixel( hist );
   dip::uint maxElement = maxCoords[ 0 ];
   Histogram::CountType maxValue = data[ maxElement ];
   dfloat threshold = smoothIn.BinCenters()[ maxElement ];
   // Is the peak on the left or right side of the histogram?
   bool rightPeak = maxElement > ( nBins / 2 );
   // Find the 50% height & the threshold
   if( rightPeak ) {
      dip::uint sigma = nBins - 1;
      for( ; sigma >= maxElement; --sigma ) {
         if( data[ sigma ] > maxValue / 2 ) {
            break;
         }
      }
      sigma -= maxElement;
      if( sigma == 0 ) {
         sigma = 1;
      }
      threshold -= static_cast< dfloat >( sigma ) * distance;
   } else {
      dip::uint sigma = 0;
      for( ; sigma <= maxElement; ++sigma ) {
         if( data[ sigma ] > maxValue / 2 ) {
            break;
         }
      }
      sigma = maxElement - sigma;
      if( sigma == 0 ) {
         sigma = 1;
      }
      threshold += static_cast< dfloat >( sigma ) * distance;
   }
   return threshold;
}

} // namespace dip
