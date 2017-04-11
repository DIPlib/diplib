/*
 * DIPlib 3.0
 * This file contains the definition for the Label function.
 *
 * (c)2016, Cris Luengo.
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

#include <set>

#include "diplib.h"
#include "diplib/regions.h"

namespace dip {

dip::uint Label(
      Image const& /*binary*/,
      Image& /*out*/,
      dip::uint /*connectivity*/,
      String /*mode*/,
      dip::uint /*minSize*/,
      dip::uint /*maxSize*/,
      BoundaryConditionArray /*bc*/
) {
   // TODO
   return 0;
}

} // namespace dip
