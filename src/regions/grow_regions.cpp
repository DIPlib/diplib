/*
 * DIPlib 3.0
 * This file contains the definition for the Label function.
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/regions.h"
#include "diplib/distance.h"
#include "diplib/morphology.h"

namespace dip {

void GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Image& out,
      Metric const& metric
) {
   // Compute grey-weighted distance transform
   Image binary = label == 0;
   Image distance;
   DIP_STACK_TRACE_THIS( GreyWeightedDistanceTransform( grey, binary, mask, distance, metric ));
   binary.Strip();

   // Grow regions
   DIP_STACK_TRACE_THIS( SeededWatershed( distance, label, mask, out, 1, -1, 0, { S::NOGAPS } )); // maxDepth = -1: disables region merging
}

} // namespace dip
