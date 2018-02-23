/*
 * DIPlib 3.0
 * This file contains declarations for file I/O support functions.
 *
 * (c)2017-2018, Cris Luengo.
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

#ifndef DIP_FILE_IO_SUPPORT_H
#define DIP_FILE_IO_SUPPORT_H

#include "diplib.h"
#include "diplib/file_io.h"

namespace dip {

RangeArray ConvertRoiSpec(
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing
);

struct RoiSpec{
   RangeArray roi;
   Range channels;
   UnsignedArray sizes;
   dip::uint tensorElements;
   BooleanArray mirror;
   bool isFullImage = true;
   bool isAllChannels = true;
};
RoiSpec CheckAndConvertRoi(
      RangeArray const& roi,
      Range const& channels,
      FileInformation const& fileInformation,
      dip::uint nDims
);

} // namespace dip

#endif //DIP_FILE_IO_SUPPORT_H
