/*
 * DIPlib 3.0
 * This file declares functions to create and manipulate offset lists.
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

#ifndef DIP_OFFSETS_H
#define DIP_OFFSETS_H

#include "diplib.h"
#include "diplib/iterators.h"


namespace dip {


// Creates a list of offsets into an image with the given sizes and strides. Pixels at the image
// boundary are excluded.
DIP_NO_EXPORT std::vector< dip::sint > CreateOffsetsArray( UnsignedArray const& sizes, IntegerArray const& strides );

// Creates a list of offsets into an image with the size of `mask` and the given strides. Only those
// pixels set in `mask` are indexed. Pixels at the image boundary are excluded.
DIP_NO_EXPORT std::vector< dip::sint > CreateOffsetsArray( Image const& mask, IntegerArray const& strides );

// Sorts the list of offsets by the greyvalue they index.
DIP_NO_EXPORT void SortOffsets( Image const& img, std::vector< dip::sint >& offsets, bool lowFirst );

} // namespace dip

#endif // DIP_OFFSETS_H
