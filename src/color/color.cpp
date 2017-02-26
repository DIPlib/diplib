/*
 * DIPlib 3.0
 * This file contains main functionality for color image support.
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

#include "diplib/color.h"

namespace dip {

std::array< double, 9 > WhitePoint::InverseMatrix() {
   // TODO
   return {};
}

ColorSpaceManager::ColorSpaceManager() {
   // TODO
}

void ColorSpaceManager::Set( Image& in, String const& name ) const {
   // TODO
}

void ColorSpaceManager::Convert(
      Image const& in,
      Image const& out,
      String const& name,
      WhitePoint const& whitepoint
) const {
   // TODO
}

std::vector< dip::uint > ColorSpaceManager::FindPath( dip::uint start, dip::uint stop ) const {
   // TODO
   return {};
}

} // namespace dip
