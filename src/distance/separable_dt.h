/*
 * (c)2018-2019, Erik Wernersson and Cris Luengo.
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

#ifndef DIP_SEPARABLE_DT_H
#define DIP_SEPARABLE_DT_H

#include "diplib.h"

namespace dip {

// Implements `dip::EuclideanDistanceTransform(...,"separable")`
// There are no tests for inputs, since it's an internal function.
DIP_NO_EXPORT void SeparableDistanceTransform(
      Image const& in,              // Must be forged, scalar and binary
      Image& out,
      FloatArray const& spacing,    // Must be given, and have one value for each dimension in `in`
      bool border = false,          // Values outside the image are background by default
      bool squareDistance = false   // Set to true to return square distances -- should be slightly cheaper
);

} // namespace dip

#endif // DIP_SEPARABLE_DT_H
