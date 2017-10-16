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


/// \brief Binary morphological dilation operation.
///
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
///
/// The \p edgeCondition parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// 
/// \see { BinaryErosion, BinaryClosing, BinaryOpening, BinaryPropagation }
/// 
DIP_EXPORT void BinaryDilation(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
);

inline Image BinaryDilation(
   Image const& in,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   Image out;
   BinaryDilation( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological erosion operation.
///
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
///
/// The \p edgeCondition parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// 
/// \see { BinaryDilation, BinaryClosing, BinaryOpening, BinaryPropagation }
DIP_EXPORT void BinaryErosion(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
);

inline Image BinaryErosion(
   Image const& in,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   Image out;
   BinaryErosion( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological closing operation.
///
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
///
/// The \p edgeCondition parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// Additionally, you can set it to `"special"` for special handling:
/// `"background"` for the dilation, `"object"` for the erosion; this avoids the border
/// effect you can get in the corners of the image in some cases.
///
/// \see { BinaryDilation, BinaryErosion, BinaryOpening, BinaryPropagation }
DIP_EXPORT void BinaryClosing
(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
);

inline Image BinaryClosing(
   Image const& in,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   Image out;
   BinaryClosing( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological opening operation.
///
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
///
/// The \p edgeCondition parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// Additionally, you can set it to `"special"` for special handling:
/// `"object"` for the erosion, `"background"` for the dilation; this avoids the border
/// effect you can get in the corners of the image in some cases.
///
/// \see { BinaryDilation, BinaryErosion, BinaryClosing, BinaryPropagation }
DIP_EXPORT void BinaryOpening
(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
);

inline Image BinaryOpening(
   Image const& in,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   Image out;
   BinaryOpening( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Morphological propagation of binary objects.
///
/// \p inSeed contains the seeds to propagate. To use no seeds,
/// simply pass an 'empty' image with dimensionality zero, e.g., \p dip::Image().
/// \p inMask contains the mask in which propagation is allowed
/// 
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
///
/// The \p edgeCondition parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
///
/// The algorithm is repeated \p iterations times. Pass 0 to continue until propagation is completed.

/// \see { BinaryDilation, BinaryErosion, BinaryClosing, BinaryOpening }
DIP_EXPORT void BinaryPropagation(
   Image const& inSeed,
   Image const& inMask,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
);

inline Image BinaryPropagation(
   Image const& inSeed,
   Image const& inMask,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   Image out;
   BinaryPropagation( inSeed, inMask, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Remove binary edge objects.
///
/// Removes those binary objects from \p in
/// which are connected to the edges of the image. The connectivity of the
/// objects is determined by \p connectivity. This function is a front - end to
/// \p BinaryPropagation. It calls \p BinaryPropagation with no seed image and
/// the pixels past the border as `"object"`. The result of the propagation is
/// xor-ed with the input image.
///
/// The \p connectivity parameter defines the metric, that is, the shape of
/// the structuring element. 1 indicates city - block metric, or a diamond - shaped
/// structuring element. 2 indicates chessboard metric, or a square structuring
/// element. - 1 and -2 indicate alternating connectivity and produce an octagonal
/// structuring element. Alternating connectivity is only implemented for 2D and 3D images.
DIP_EXPORT void BinaryEdgeObjectsRemove(
   Image const& in,
   Image& out,
   dip::sint connectivity
);

inline Image BinaryEdgeObjectsRemove(
   Image const& in,
   dip::sint connectivity
) {
   Image out;
   BinaryEdgeObjectsRemove( in, out, connectivity );
   return out;
}


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
   dip_EdgeObjectsRemove (dip_binary.h)
*/

/// \}

} // namespace dip

#endif // DIP_BINARY_H
