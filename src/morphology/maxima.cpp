/*
 * DIPlib 3.0
 * This file contains the functions Maxima and Minima.
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
#include "diplib/morphology.h"

namespace dip {

namespace {

void Extrema(
      Image const& /*in*/,
      Image const& /*mask*/,
      Image& /*out*/,
      dip::uint /*connectivity*/,
      String const& /*output*/,
      bool /*maxima*/
) {
   // TODO
}

} // namespace

void Minima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, mask, out, connectivity, output, false );
}

void Maxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, mask, out, connectivity, output, true );
}

} // namespace dip
