/*
 * (c)2024, Cris Luengo.
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

#ifndef FRAMEWORK_SUPPORT_H
#define FRAMEWORK_SUPPORT_H

#include "diplib/framework.h"

#include <vector>

#include "diplib.h"

namespace dip {
namespace Framework {

std::vector< UnsignedArray >  SplitImageEvenlyForProcessing(
   UnsignedArray const& sizes,
   dip::uint nBlocks,
   dip::uint nPixelsPerBlock,
   dip::uint processingDim // set to sizes.size() or larger if there's none
);

} // namespace Framework
} // namespace dip

#endif //FRAMEWORK_SUPPORT_H
