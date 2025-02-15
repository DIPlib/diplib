/*
 * (c)2017-2025, Cris Luengo.
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

#include "diplib/histogram.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/polygon.h"
#include "diplib/random.h"
#include "diplib/segmentation.h"
#include "diplib/statistics.h"

#include "threshold_algorithms.h"


namespace dip {

using CountType = Histogram::CountType;

namespace {

// Compute nD bin centers from bin indices
FloatCoordinateArray ComputeCoordinates( Histogram const& hist, CoordinateArray const& bins ) {
   dip::uint nDims = hist.Dimensionality();
   FloatCoordinateArray out( bins.size(), FloatArray( nDims, 0 ) );
   for( dip::uint ii = 0; ii < bins.size(); ++ii ) {
      DIP_ASSERT( bins[ ii ].size() == nDims );
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         out[ ii ][ jj ] = hist.BinCenter( bins[ ii ][ jj ], jj );
      }
   }
   return out;
}

} // namespace


FloatCoordinateArray KMeansClustering(
      Histogram const& in,
      Histogram& out,
      Random& random,
      dip::uint nClusters
) {
   Image labs;
   auto centers = KMeansClustering( in.GetImage(), labs, random, nClusters );
   if( &out != &in ) {
      out = in.Copy();
   }
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return ComputeCoordinates( out, centers );
}

FloatCoordinateArray MinimumVariancePartitioning(
      Histogram const& in,
      Histogram& out,
      dip::uint nClusters
) {
   Image labs;
   auto centers = MinimumVariancePartitioning( in.GetImage(), labs, nClusters );
   if( &out != &in ) {
      out = in.Copy();
   }
   Image tmp = out.GetImage(); // Copy with shared data
   tmp.Copy( labs );
   return ComputeCoordinates( out, centers );
}

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
   dip::uint N = static_cast< dip::uint >( ptr[ nBins - 1 ] ) / ( nThresholds + 1 );
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
      FloatArray centers( nThresholds + 1 );
      for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
         dip::uint origin2 = static_cast< dip::uint >( std::ceil( thresholds[ ii ] ));
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
   dfloat scale = in.BinSize();
   dfloat offset = in.LowerBound() + scale / 2; // bin[ii] = offset + ii * scale;
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      thresholds[ ii ] = offset + thresholds[ ii ] * scale;
   }
   return thresholds;
}

dfloat OtsuThreshold( Histogram const& in ) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   dip::uint nBins = hist.Size( 0 );
   dip::uint maxInd = OtsuThreshold( data, nBins );
   DIP_THROW_IF( maxInd == nBins, "Could not find a maximum in Otsu's measure for inter-class variance" );
   return ( in.BinCenter( maxInd ) + in.BinCenter( maxInd + 1 )) / 2.0;
}

dfloat MinimumErrorThreshold( Histogram const& in ) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image const& hist = in.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   dfloat scale = in.BinSize();
   dfloat offset = in.LowerBound() + scale / 2; // bin[ii] = offset + ii * scale;
   Histogram::CountType const* data = static_cast< Histogram::CountType* >( hist.Origin() );
   // w1(ii), w2(ii) are the probabilities of each of the halves of the histogram thresholded between bin(ii) and bin(ii+1)
   dfloat w1 = 0;
   dfloat w2 = std::accumulate( data, data + nBins, 0.0 );
   // m1(ii), m2(ii) are the corresponding first order moments
   dfloat m1 = 0;
   dfloat m2 = 0;
   for( dip::uint ii = 0; ii < nBins; ++ii ) {
      m2 += static_cast< dfloat >( data[ ii ] ) * ( offset + static_cast< dfloat >( ii ) * scale );
   }
   // Here we accumulate the error measure.
   std::vector< dfloat > J( nBins - 1 );
   for( dip::uint ii = 0; ii < nBins - 1; ++ii ) {
      dfloat value = static_cast< dfloat >( data[ ii ] );
      w1 += value;
      w2 -= value;
      dfloat tmp = value * ( offset + static_cast< dfloat >( ii ) * scale );
      m1 += tmp;
      m2 -= tmp;
      // c1(ii), c2(ii) are the centers of gravity
      dfloat c1 = m1 / w1;
      dfloat c2 = m2 / w2;
      // v1(ii), v2(ii) are the corresponding second order central moments
      dfloat v1 = 0;
      for( dip::uint jj = 0; jj <= ii; ++jj ) {
         dfloat d = ( offset + static_cast< dfloat >( jj ) * scale ) - c1;
         v1 += static_cast< dfloat >( data[ jj ] ) * d * d;
      }
      v1 /= w1;
      dfloat v2 = 0;
      for( dip::uint jj = ii + 1; jj < nBins; ++jj ) {
         dfloat d = ( offset + static_cast< dfloat >( jj ) * scale ) - c2;
         v2 += static_cast< dfloat >( data[ jj ] ) * d * d;
      }
      v2 /= w2;
      // J(ii) is the measure for error
      J[ ii ] = 1.0 + w1 * std::log( v1 ) + w2 * std::log( v2 ) - 2.0 * ( w1 * std::log( w1 ) + w2 * std::log( w2 ));
      //std::cout << J[ii] << ',';
   }
   //std::cout << '\n';
   // Now we need to find the minimum in J, but ignore the values at the edges, if they are lowest.
   dip::uint begin = 0;
   dip::uint end = nBins - 2;
   for( ; ( begin < end - 1 ) && ( J[ begin ] <= J[ begin + 1 ] ); ++begin ) {}
   for( ; ( begin < end - 1 ) && ( J[ end ] <= J[ end - 1 ] ); --end ) {}
   dfloat minJ = J[ begin ];
   dip::uint minInd = begin;
   for( dip::uint ii = begin + 1; ii < end; ++ii ) {
      if( J[ ii ] < minJ ) {
         minJ = J[ ii ];
         minInd = ii;
      }
   }
   dip::uint maxInd = minInd + 1;
   for( ; ( maxInd < end ) && ( J[ maxInd ] == minJ ); ++maxInd ) {}
   return offset + ( static_cast< dfloat >( minInd ) + static_cast< dfloat >( maxInd )) / 2.0 * scale;
}

FloatArray GaussianMixtureModelThreshold(
      Histogram const& in,
      dip::uint nThresholds
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   // Apply model to histogram
   std::vector< GaussianParameters > params = GaussianMixtureModel( in, nThresholds + 1 );
   // Sort Gaussians by position
   std::sort( params.begin(), params.end(), []( GaussianParameters const& a, GaussianParameters const& b ) { return a.position < b.position; } );
   // Find thresholds
   dfloat tolerance = 1e6 * in.BinSize();
   FloatArray thresholds( nThresholds );
   for( dip::uint ii = 0; ii < nThresholds; ++ii ) {
      dfloat x1 = params[ ii     ].position;
      dfloat x2 = params[ ii + 1 ].position;
      dfloat d = x2 - x1;
      DIP_ASSERT( d >= 0.0 ); // ensures they're sorted
      if( d < tolerance ) { // too small to bother
         thresholds[ ii ] = x1;
         continue;
      }
      dfloat a1 = params[ ii     ].amplitude;
      dfloat a2 = params[ ii + 1 ].amplitude;
      dfloat s1 = params[ ii     ].sigma;
      dfloat s2 = params[ ii + 1 ].sigma;
      if( std::abs( s1 - s2 ) < tolerance ) { // too small to bother
         // assume s1 == s2
         thresholds[ ii ] = 0.5 * d + s1 * s1 / d * std::log( a1 / a2 );
         continue;
      }
      dfloat ds2 = s2 * s2 - s1 * s1;
      dfloat t1 = -s1 * s1 * d;
      dfloat t2 = s1 * s2 * std::sqrt( d * d + 2 * ds2 * std::log( a1 / a2 ));
      thresholds[ ii ] = ( t1 + t2 ) / ds2;
      if(( thresholds[ ii ] < x1 ) || ( thresholds[ ii ] > x2 )) {
         thresholds[ ii ] = ( t1 - t2 ) / ds2; // the first root is outside of the domain, try the second root
         if(( thresholds[ ii ] < x1 ) || ( thresholds[ ii ] > x2 )) {
            thresholds[ ii ] = ( x1 + x2 ) / 2; // Both thresholds are outside of the domain, let's pick a number half-way
         }
      }
   }
   return thresholds;
}

dfloat TriangleThreshold(
      Histogram const& in,
      dfloat sigma
) {
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, sigma );
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
   return smoothIn.BinCenter( bin );
}

dfloat BackgroundThreshold(
      Histogram const& in,
      dfloat distance,
      dfloat sigma
) {
   DIP_THROW_IF( distance <= 0, E::INVALID_PARAMETER );
   DIP_THROW_IF( in.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Histogram smoothIn = Smooth( in, sigma );
   Image const& hist = smoothIn.GetImage();
   DIP_ASSERT( hist.IsForged() );
   DIP_ASSERT( hist.DataType() == DT_COUNT );
   DIP_ASSERT( hist.Stride( 0 ) == 1 );
   dip::uint nBins = hist.Size( 0 );
   Histogram::CountType const* data = static_cast< Histogram::CountType const* >( hist.Origin() );
   // Find the peak with sub-sample precision
   dip::uint maxElement = MaximumPixel( hist )[ 0 ];
   SubpixelLocationResult maxLoc = SubpixelLocation( hist, { maxElement }, S::MAXIMUM, S::GAUSSIAN_SEPARABLE );
   dfloat halfMaxValue = maxLoc.value / 2;
   dfloat binSize = smoothIn.BinSize();
   // Is the peak on the left or right side of the histogram?
   bool rightPeak = maxElement > ( nBins / 2 );
   // Find the 50% height
   dip::uint bin = 0;
   if( rightPeak ) {
      for( bin = nBins - 1 ; bin >= maxElement; --bin ) {
         if( static_cast< dfloat >( data[ bin ] ) > halfMaxValue ) {
            break;
         }
      }
   } else {
      for( ; bin <= maxElement; ++bin ) {
         if( static_cast< dfloat >( data[ bin ] ) > halfMaxValue ) {
            break;
         }
      }
   }
   // Linear interpolation to refine that
   dfloat subsampleX = 0;
   if(( bin >= 1 ) && ( bin < nBins - 1 )) {
      dip::uint lowerBin = rightPeak ? bin + 1 : bin - 1;
      dfloat y0 = static_cast< dfloat >( data[ bin ] );
      dfloat y1 = static_cast< dfloat >( data[ lowerBin ] );
      subsampleX = ( y0 - halfMaxValue ) / ( y0 - y1 );
   }
   dfloat observed_HWHM = rightPeak ? ( static_cast< dfloat >( bin ) + subsampleX ) - maxLoc.coordinates[ 0 ]
                                    : maxLoc.coordinates[ 0 ] - ( static_cast< dfloat >( bin ) - subsampleX );
   // Correct for smoothing applied to histogram
   constexpr dfloat factor = 2.355 / 2.0; // Assuming a Gaussian background peak, then sigma * factor = hwhm
   dfloat smoothing_HWHM = sigma * factor;
   // Sigmas are added in a square sense (true_sigma^2 + smoothing_sigma^2 = observed_sigma^2)
   // Thus the same is true for HWHM values (true_HWHM^2 + smoothing_HWHM^2 = observed_HWHM^2)
   // Find true_HWHM given observed_HWHM and smoothing_HWHM
   dfloat true_HWHM = ( observed_HWHM > smoothing_HWHM )
                      ? std::sqrt( observed_HWHM * observed_HWHM - smoothing_HWHM * smoothing_HWHM )
                      : 0.0;
   true_HWHM = std::max( true_HWHM, 1.0 );
   // Find the threshold
   dfloat peak_location = smoothIn.LowerBound( 0 ) + ( maxLoc.coordinates[ 0 ] + 0.5 ) * binSize;
   dfloat sign = rightPeak ? -1.0 : 1.0;
   return peak_location + sign * true_HWHM * distance * binSize;
}

} // namespace dip
