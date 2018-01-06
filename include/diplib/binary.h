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
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity). Alternating connectivity
/// is only implemented for 2D and 3D images.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// 
/// \see { BinaryErosion, BinaryClosing, BinaryOpening, BinaryPropagation }
DIP_EXPORT void BinaryDilation(
      Image const& in,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::BACKGROUND
);

inline Image BinaryDilation(
      Image const& in,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   BinaryDilation( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological erosion operation.
///
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity). Alternating connectivity
/// is only implemented for 2D and 3D images.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// 
/// \see { BinaryDilation, BinaryClosing, BinaryOpening }
DIP_EXPORT void BinaryErosion(
      Image const& in,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::OBJECT
);

inline Image BinaryErosion(
      Image const& in,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::OBJECT
) {
   Image out;
   BinaryErosion( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological closing operation.
///
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity). Alternating connectivity
/// is only implemented for 2D and 3D images.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// Additionally, you can set it to `"special"` for special handling:
/// `"background"` for the dilation, `"object"` for the erosion; this avoids the border
/// effect you can get in the corners of the image in some cases.
///
/// \see { BinaryDilation, BinaryErosion, BinaryOpening }
DIP_EXPORT void BinaryClosing(
      Image const& in,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::SPECIAL
);

inline Image BinaryClosing(
      Image const& in,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::SPECIAL
) {
   Image out;
   BinaryClosing( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Binary morphological opening operation.
///
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity). Alternating connectivity
/// is only implemented for 2D and 3D images.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
/// Additionally, you can set it to `"special"` for special handling:
/// `"object"` for the erosion, `"background"` for the dilation; this avoids the border
/// effect you can get in the corners of the image in some cases.
///
/// \see { BinaryDilation, BinaryErosion, BinaryClosing }
DIP_EXPORT void BinaryOpening(
      Image const& in,
      Image& out,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::SPECIAL
);

inline Image BinaryOpening(
      Image const& in,
      dip::sint connectivity = -1,
      dip::uint iterations = 3,
      String const& edgeCondition = S::SPECIAL
) {
   Image out;
   BinaryOpening( in, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Morphological propagation of binary objects.
///
/// `inSeed` contains the seeds to propagate. To use no seeds, simply pass a raw image, i.e. `dip::Image()`.
/// `inMask` contains the mask in which propagation is allowed.
/// 
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity). Alternating connectivity
/// is only implemented for 2D and 3D images.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
///
/// The algorithm is repeated `iterations` times. Pass 0 to continue until propagation is completed.
///
/// The function `dip::MorphologicalReconstruction` provides similar functionality also for other data types.
///
/// \see { dip::MorphologicalReconstruction, dip::BinaryDilation, dip::BinaryErosion }
DIP_EXPORT void BinaryPropagation(
      Image const& inSeed,
      Image const& inMask,
      Image& out,
      dip::sint connectivity = 1,
      dip::uint iterations = 0,
      String const& edgeCondition = S::BACKGROUND
);

inline Image BinaryPropagation(
      Image const& inSeed,
      Image const& inMask,
      dip::sint connectivity = 1,
      dip::uint iterations = 0,
      String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   BinaryPropagation( inSeed, inMask, out, connectivity, iterations, edgeCondition );
   return out;
}

/// \brief Remove binary edge objects.
///
/// Removes those binary objects from `in` that are connected to the edges of the image.
/// The connectivity of the objects is determined by `connectivity`. This function
/// calls `dip::BinaryPropagation` with no seed image and `edgeCondition` set to `"object"`.
/// The result of the propagation is xor-ed with the input image.
///
/// The `connectivity` parameter defines the metric, that is, the shape of
/// the structuring element (see \ref connectivity).
inline void EdgeObjectsRemove(
      Image const& in,
      Image& out,
      dip::uint connectivity = 1
){
   DIP_START_STACK_TRACE
      // Propagate with empty seed mask, iteration until done and treating outside the image as object
      BinaryPropagation( Image(), in, out, static_cast< dip::sint >( connectivity ), 0, S::OBJECT );
      // The out-image now contains the edge objects
      // Remove them by toggling these bits in the in-image and writing the result in out
      out ^= in;
   DIP_END_STACK_TRACE
}

inline Image EdgeObjectsRemove(
      Image const& in,
      dip::uint connectivity = 1
) {
   Image out;
   EdgeObjectsRemove( in, out, connectivity );
   return out;
}

/// \brief Thickens the image `in` conditioned on the mask (2D only).
///
/// A thickening is a dilation that preserves topology. If no `mask` is given (i.e. the image is raw),
/// it will produce an inverse skeleton of the background (given sufficient iterations).
/// If a `mask` is given, the dilation will not propagate outside of the set pixels in `mask`.
///
/// The dilation always uses the unit 4-connected neighborhood. That is, it iteratively propagates using
/// a connectivity of 1. The topology preserved is that of the 8-connected background. `iterations` iterations
/// are applied. If `iterations` is 0, the algorithm iterates until idempotency.
///
/// `endPixelCondition` determines if background branches are kept. The string `"keep"` is equivalent to the
/// `"natural"` end pixel condition in `dip::EuclideanSkeleton`, and `"lose"` causes branches to not be kept
/// (meaning that only single background pixels and loops in background are kept).
///
/// The `edgeCondition` parameter specifies whether the border of the image should be treated as object (`"object"`)
/// or as background (`"background"`). Note that the algorithm doesn't propagate into the pixels around the edge
/// of the image. The `edgeCondition` is used to modify the input image before the iterative process starts.
DIP_EXPORT void ConditionalThickening2D(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint iterations = 0,
      String const& endPixelCondition = S::LOSE,
      String const& edgeCondition = S::BACKGROUND
);
inline Image ConditionalThickening2D(
      Image const& in,
      Image const& mask,
      dip::uint iterations = 0,
      String const& endPixelCondition = S::LOSE,
      String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   ConditionalThickening2D( in, mask, out, iterations, endPixelCondition, edgeCondition );
   return out;
}

/// \brief Thins the image `in` conditioned on the mask (2D only).
///
/// A thinning is an erosion that preserves topology. If no `mask` is given (i.e. the image is raw),
/// it will produce a skeleton of the object (given sufficient iterations).
/// If a `mask` is given, the erosion will not propagate outside of the set pixels in `mask`.
/// Note that `dip::EuclideanSkeleton` produces a better skeleton.
///
/// The erosion always uses the unit 4-connected neighborhood. That is, it iteratively propagates using
/// a connectivity of 1. The topology preserved is that of the 8-connected foreground. `iterations` iterations
/// are applied. If `iterations` is 0, the algorithm iterates until idempotency.
///
/// `endPixelCondition` determines if branches are kept, and how many are generated. See `dip::EuclideanSkeleton`
/// for a list of possible values.
///
/// The `edgeCondition` parameter specifies whether the border of the image should be treated as object (`"object"`)
/// or as background (`"background"`). Note that the algorithm doesn't propagate into the pixels around the edge
/// of the image. The `edgeCondition` is used to modify the input image before the iterative process starts.
DIP_EXPORT void ConditionalThinning2D(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint iterations = 0,
      String const& endPixelCondition = S::LOSE,
      String const& edgeCondition = S::BACKGROUND
);
inline Image ConditionalThinning2D(
      Image const& in,
      Image const& mask,
      dip::uint iterations = 0,
      String const& endPixelCondition = S::LOSE,
      String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   ConditionalThinning2D( in, mask, out, iterations, endPixelCondition, edgeCondition );
   return out;
}

/// \brief Computes the area opening of a binary image
///
/// The area opening removes all connected components that have an area smaller than the given parameter `filterSize`,
/// and is equivalent to the supremum of openings with all possible connected structuring elements of that area.
///
/// `connectivity` determines what a connected component is. See \ref connectivity for information on the
/// connectivity parameter.
///
/// The `edgeCondition` parameter specifies whether pixels past the border of the image should be
/// treated as object (by passing `"object"`) or as background (by passing `"background"`).
///
/// The operation is implemented through `dip::Label`.
///
/// \see dip::AreaOpening, dip::AreaClosing
DIP_EXPORT void BinaryAreaOpening(
      Image const& in,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      String const& edgeCondition = S::BACKGROUND
);
inline Image BinaryAreaOpening(
      Image const& in,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   BinaryAreaOpening( in, out, filterSize, connectivity, edgeCondition );
   return out;
}

/// \brief Computes the area closing of a binary image, by calling `dip::BinaryAreaOpening` on the inverse
/// of the input image.
inline void BinaryAreaClosing(
      Image const& in,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      String const& s_edgeCondition = S::BACKGROUND
) {
   DIP_START_STACK_TRACE
      bool edgeCondition = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND );
      Not( in, out );
      BinaryAreaOpening( out, out, filterSize, connectivity, edgeCondition ? S::BACKGROUND : S::OBJECT ); // invert edge condition also
      Not( out, out );
   DIP_END_STACK_TRACE
}
inline Image BinaryAreaClosing(
        Image const& in,
        dip::uint filterSize,
        dip::uint connectivity = 0,
        String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   BinaryAreaClosing( in, out, filterSize, connectivity, edgeCondition );
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
      String const& endPixelCondition = S::NATURAL,
      String const& edgeCondition = S::BACKGROUND
);

inline Image EuclideanSkeleton(
      Image const& in,
      String const& endPixelCondition = S::NATURAL,
      String const& edgeCondition = S::BACKGROUND
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
      dip::String const& mode = S::FOREGROUND,
      dip::String const& edgeCondition = S::BACKGROUND
);
inline Image CountNeighbors(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& mode = S::FOREGROUND,
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   CountNeighbors( in, out, connectivity, mode, edgeCondition );
   return out;
}

/// \brief Filters the binary image by setting each pixel to the phase with more pixels in the neighborhood.
///
/// The majority vote filter is the binary equivalent to the median filter. If in the neighborhood of a pixel there
/// are more foreground than background pixels, the pixel will be set to foreground. Otherwise it will be set to
/// background. The pixel itself is part of the neighborhood, and therefore the neighborhood always has an odd number
/// of pixels.
///
/// Note that this is equivalent to (but more efficient than):
/// ```
///     dip::uint neighborhoodSize = dip::NeighborList( { "connected", connectivity }, in.Dimensionality() ).Size();
///     out = dip::CountNeighbors( in, connectivity, "all", edgeCondition ) > neighborhoodSize / 2;
/// ```
/// with `neighborhoodSize` the number of pixels in the neighborhood given by `connectivity`.
///
/// `edgeCondition` determines the value of pixels outside the image domain, and can be `"object"` or `"background"`.
///
/// \see dip::MedianFilter, dip::CountNeighbors
DIP_EXPORT void MajorityVote(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
);
inline Image MajorityVote(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   MajorityVote( in, out, connectivity, edgeCondition );
   return out;
}

/// \brief Returns the isolated pixels in the binary image `in`. That is, the set pixels with zero neighbors. See `dip::CountNeighbors`.
inline void GetSinglePixels(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image nn = CountNeighbors( in, connectivity, S::FOREGROUND, edgeCondition );
   Equal( nn, 1, out );
}
inline Image GetSinglePixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
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
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image nn = CountNeighbors( in, connectivity, S::FOREGROUND, edgeCondition );
   Equal( nn, 2, out );
}
inline Image GetEndPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
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
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image nn = CountNeighbors( in, connectivity, S::FOREGROUND, edgeCondition );
   Equal( nn, 3, out );
}
inline Image GetLinkPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
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
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image nn = CountNeighbors( in, connectivity, S::FOREGROUND, edgeCondition );
   Greater( nn, 3, out );
}
inline Image GetBranchPixels(
      Image const& in,
      dip::uint connectivity = 0,
      dip::String const& edgeCondition = S::BACKGROUND
) {
   Image out;
   GetBranchPixels( in, out, connectivity, edgeCondition );
   return out;
}


/// \}

} // namespace dip

#endif // DIP_BINARY_H
