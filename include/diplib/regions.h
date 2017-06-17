/*
 * DIPlib 3.0
 * This file contains declarations for functions that work with labeled images.
 *
 * (c)2016-2017, Cris Luengo.
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

#ifndef DIP_REGIONS_H
#define DIP_REGIONS_H

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Declares image processing functions that work with labeled images.


namespace dip {


/// \defgroup regions Connected component analysis
/// \brief Label connected component and process labeled images.
/// \{

// TODO: functions to port:
/*
   dip_Label (dip_regions.h)
   dip_RegionConnectivity (dip_regions.h)
   dip_GrowRegions (dip_regions.h)
   dip_GrowRegionsWeighted (dip_regions.h)
   dip_SmallObjectsRemove (dip_measurement.h)
*/

/// \brief Labels the connected components in a binary image
///
/// The output is an unsigned integer image. Each object (respecting the connectivity,
/// see \ref connectivity) in the input image receives a unique number. This number ranges
/// from 1 to the number of objects in the image. The pixels in the output image corresponding
/// to a given object are set to this number (label). The remaining pixels in the output image
/// are set to 0.
///
/// The `minSize` and `maxSize` set limits on the size of the objects: Objects smaller than `minSize`
/// or larger than `maxSize` do not receive a label and the corresponding pixels in the output
/// image are set to zero. Setting either to zero disables the corresponding check. Setting both
/// to zero causes all objects to be labeled, irrespective of size.
///
/// The boundary conditions are generally ignored (labeling stops at the boundary). The exception
/// is `"periodic"`, which is the only one that makes sense for this algorithm.
DIP_EXPORT dip::uint Label(
      Image const& binary,
      Image& out,
      dip::uint connectivity,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray const& boundaryCondition = {}
);

inline Image Label(
      Image const& binary,
      dip::uint connectivity,
      dip::uint minSize = 0,
      dip::uint maxSize = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Label( binary, out, connectivity, minSize, maxSize, boundaryCondition );
   return out;
}

/// \brief Gets a list of object labels in the labeled image. A labeled image must be of an unsigned type.
DIP_EXPORT UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      bool nullIsObject
);

/// \}

} // namespace dip

#endif // DIP_REGIONS_H
