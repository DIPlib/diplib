/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement image thresholding.
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
#include "diplib/segmentation.h"
#include "diplib/histogram.h"
#include "diplib/math.h"
#include "diplib/morphology.h"


namespace dip {


void KMeansClustering(
      Image const& in,
      Image& out,
      dip::uint nClusters
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}


FloatArray IsodataThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint nThresholds
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   // TODO: if the input image is not scalar, we can apply KMeansClustering to the histogram, then do an inverse mapping (not yet implemented).
   DIP_START_STACK_TRACE
      Histogram::Configuration conf( 0.0, 100.0, 200 );
      conf.lowerIsPercentile = true;
      conf.upperIsPercentile = true;
      Histogram hist( in, mask, conf );
      FloatArray thresholds = IsodataThreshold( hist, nThresholds );
      if( nThresholds == 1 ) {
         FixedThreshold( in, out, thresholds[ 0 ] );
      } else {
         MultipleThresholds( in, out, thresholds );
      }
      return thresholds;
   DIP_END_STACK_TRACE
}

dfloat OtsuThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_START_STACK_TRACE
      Histogram::Configuration conf( 0.0, 100.0, 200 );
      conf.lowerIsPercentile = true;
      conf.upperIsPercentile = true;
      Histogram hist( in, mask, conf );
      dfloat threshold = OtsuThreshold( hist );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat MinimumErrorThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_START_STACK_TRACE
      Histogram::Configuration conf( 0.0, 100.0, 200 );
      conf.lowerIsPercentile = true;
      conf.upperIsPercentile = true;
      Histogram hist( in, mask, conf );
      dfloat threshold = MinimumErrorThreshold( hist );
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

dfloat TriangleThreshold(
      Image const& in,
      Image const& mask,
      Image& out
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

dfloat BackgroundThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat distance
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

dfloat VolumeThreshold(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat volumeFraction
) {
   DIP_START_STACK_TRACE
      dfloat threshold = Percentile( in, mask, volumeFraction * 100 ).As< dfloat >();
      FixedThreshold( in, out, threshold );
      return threshold;
   DIP_END_STACK_TRACE
}

void FixedThreshold(
      Image const& in,
      Image& out,
      dfloat threshold,
      dfloat foreground,
      dfloat background,
      String const& output
) {
   DIP_START_STACK_TRACE
      if( output == "binary" ) {
         if( foreground == 0.0 ) {
            // out = in <= threshold
            NotGreater( in, Image{ threshold, in.DataType() }, out );
         } else {
            // out = in >= threshold
            NotLesser( in, Image{ threshold, in.DataType() }, out );
         }
      } else {
         // out = in >= threshold ? foreground : background
         Select( in, Image{ threshold, in.DataType() },
                 Image{ foreground, in.DataType() }, Image{ background, in.DataType() },
                 out, ">=" );
      }
   DIP_END_STACK_TRACE
}

void RangeThreshold(
      Image const& in,
      Image& out,
      dfloat lowerBound,
      dfloat upperBound,
      dfloat foreground,
      dfloat background,
      String const& output
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void HysteresisThreshold(
      Image const& in,
      Image& out,
      dfloat lowThreshold,
      dfloat highThreshold
) {
   DIP_START_STACK_TRACE
      Image low = in >= lowThreshold;
      Image high = in >= highThreshold;
      MorphologicalReconstruction( high, low, out );
   DIP_END_STACK_TRACE
}

void MultipleThresholds(
      Image const& in,
      Image& out,
      FloatArray const& thresholds
) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

} // namespace dip
