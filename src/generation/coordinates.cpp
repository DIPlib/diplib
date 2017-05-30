/*
 * DIPlib 3.0
 * This file contains definitions for coordinate image generation
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
#include "diplib/generation.h"

namespace dip {

void FillRamp( Image& /*out*/, dip::uint /*dimension*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void FillPhiCoordinate( Image& /*out*/, dip::uint /*axis*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void FillRCoordinate( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

void FillCoordinates( Image& /*out*/, StringSet const& /*mode*/ ) {
   DIP_THROW( E::NOT_IMPLEMENTED );
   // TODO
}

} // namespace dip
