/*
 * DIPlib 3.0
 * This file contains declarations for binary image processing
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

#ifndef DIP_BINARY_H
#define DIP_BINARY_H

#include "diplib.h"


/// \file
/// \brief Functions for binary image processing.
/// \see binary


namespace dip {


/// \defgroup binary Binary image filters
/// \ingroup filtering
/// \brief Processing binary images, including binary mathematical morphology.
/// \{

/// \brief Accurate binary skeleton (2D and 3D only).
///
/// This algorithm computes quasi-Euclidean distances and tests Hilditch conditions to preserve topology. In 2D,
/// integer distances to neighbors are as follows:
///
/// neighbors     | distance
/// --------------|----------
/// 4-connected   | 5
/// 8-connected   | 7
/// knight's move | 11
///
/// and in 3D as follows:
///
/// neighbors              | distance
/// -----------------------|----------
/// 6-connected neighbors  | 4
/// 18-connected neighbors | 6
/// 26-connected neighbors | 7
/// knight's move          | 9
/// (2,1,1) neighbors      | 10
/// (2,2,1) neighbors      | 12
///
/// The `endPixelCondition` parameter determines what is considered an "end pixel" in the skeleton, and thus affects
/// how many branches are generated. It is one of the following strings:
///  - `"loose ends away"`: Loose ends are eaten away (nothing is considered an end point).
///  - `"natural"`: "natural" end pixel condition of this algorithm.
///  - `"one neighbor"`: Keep endpoint if it has one neighbor.
///  - `"two neighbors"`: Keep endpoint if it has two neighbors.
///  - `"three neighbors"`: Keep endpoint if it has three neighbors.
///
/// The `edgeCondition` parameter specifies whether the border of the image should be treated as object (`"object"`)
/// or as background (`"background"`).
///
/// **Limitations**
///  - This function is only implemented for 2D and 3D images.
///  - Pixels in a 2-pixel border around the edge are not processed. If this is an issue, consider adding 2 pixels
///    on each side of your image.
///  - Results in 3D are not optimal: `"loose ends away"`, `"one neighbor"` and `"three neighbors"` produce the
///    same results, and sometimes planes in the skeleton are not thinned to a single pixel thickness.
///
/// **Literature**
/// - B.J.H. Verwer, "Improved metrics in image processing applied to the Hilditch skeleton", 9th ICPR, 1988.
DIP_EXPORT void EuclideanSkeleton(
      Image const& in,
      Image& out,
      String const& endPixelCondition = "natural",
      String const& edgeCondition = "background"
);
inline Image EuclideanSkeleton(
      Image const& in,
      String const& endPixelCondition = "natural",
      String const& edgeCondition = "background"
) {
   Image out;
   EuclideanSkeleton( in, out, endPixelCondition, edgeCondition );
   return out;
}

/// \brief Counts the number of set neighbors for each pixel in the binary image `in`.
///
/// Out will contain, for each set pixel, 1 + the number of neighbors that are also set. The neighborhood is
/// given by `connectivity`, see \ref connectivity for more information. If `mode` is set to `"all"`, the
/// count is computed for all pixels, not only the foreground ones. In this case, for the non-set pixels the
/// count is not increased by 1, and therefore yields simply the count of set pixels in the full neighborhood.
///
/// `edgeCondition` determines the value of pixels outside the image domain, and can be `"object"` or `"background"`.
///
/// This function is typically used on the output of `dip::EuclideanSkeleton` to distinguish different types
/// of pixels. See also `dip::GetSinglePixels`, `dip::GetEndPixels`, `dip::GetLinkPixels`, and `dip::GetBranchPixels`.
DIP_EXPORT void CountNeighbors(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& mode = "object",
      dip::String const& edgeCondition = "background"
);
inline Image CountNeighbors(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& mode = "object",
      dip::String const& edgeCondition = "background"
) {
   Image out;
   CountNeighbors( in, out, connectivity, mode, edgeCondition );
   return out;
}

/// \brief Returns the isolated pixels in the binary image `in`. That is, the set pixels with zero neighbors. See `dip::CountNeighbors`.
inline void GetSinglePixels(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image nn = CountNeighbors( in, connectivity, "foreground", edgeCondition );
   Equal( nn, 1, out );
}
inline Image GetSinglePixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image out;
   GetSinglePixels( in, out, connectivity, edgeCondition );
   return out;
}

/// \brief Returns the end pixels in the skeleton image `in`. That is, the set pixels with one neighbors. See `dip::CountNeighbors`.
inline void GetEndPixels(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image nn = CountNeighbors( in, connectivity, "foreground", edgeCondition );
   Equal( nn, 2, out );
}
inline Image GetEndPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image out;
   GetEndPixels( in, out, connectivity, edgeCondition );
   return out;
}

/// \brief Returns the link pixels in the skeleton image `in`. That is, the set pixels with two neighbors. See `dip::CountNeighbors`.
inline void GetLinkPixels(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image nn = CountNeighbors( in, connectivity, "foreground", edgeCondition );
   Equal( nn, 3, out );
}
inline Image GetLinkPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image out;
   GetLinkPixels( in, out, connectivity, edgeCondition );
   return out;
}

/// \brief Returns the branch pixels in the skeleton image `in`. That is, the set pixels with more than two neighbors. See `dip::CountNeighbors`.
inline void GetBranchPixels(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image nn = CountNeighbors( in, connectivity, "foreground", edgeCondition );
   Greater( nn, 3, out );
}
inline Image GetBranchPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = "background"
) {
   Image out;
   GetBranchPixels( in, out, connectivity, edgeCondition );
   return out;
}


// TODO: functions to port:
/*
   dip_BinaryDilation (dip_binary.h)
   dip_BinaryErosion (dip_binary.h)
   dip_BinaryClosing (dip_binary.h)
   dip_BinaryOpening (dip_binary.h)
   dip_BinaryPropagation (dip_binary.h)
   dip_EdgeObjectsRemove (dip_binary.h)
*/

/// \}

} // namespace dip

#endif // DIP_BINARY_H
