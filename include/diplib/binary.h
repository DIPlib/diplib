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
/// For dilations with arbitrary structuring elements, see `dip::Dilation`.
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
/// For erosions with arbitrary structuring elements, see `dip::Erosion`.
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
/// For closings with arbitrary structuring elements, see `dip::Closing`.
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
/// For openings with arbitrary structuring elements, see `dip::Opening`.
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
/// - B.J.H. Verwer, "Improved metrics in image processing applied to the Hilditch skeleton", 9<sup>th</sup> ICPR, 1988.
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


// The functionality below is inspired by MMorph (though no code was taken from MMorph).
// MMorph (SDC Morphology Toolbox) is a library (no longer supported) written by Roberto Lotufo
// and colleagues at University of Campinas (UNICAMP), Brazil.
//
// These operations are not necessarily efficient, in most cases more efficient algorithms can
// be devised. But they are general and allow for complicated binary filters to be built.

class DIP_NO_EXPORT Interval;
using IntervalArray = std::vector< Interval >;

/// \brief Represents the shape of an interval for inf-generating and sup-generating operators.
///
/// An interval is typically expressed as two structuring elements, one marking the foreground
/// pixels, one marking background pixels. Pixels not marked in either are the "don't care" pixels.
/// An interval must always have at least one foreground pixel. If there are no background pixels
/// in the interval, consider using a plain dilation or erosion instead.
///
/// There are two constructors: one accepting two binary structuring elements, and one accepting a single
/// kernel containing `1` for foreground, `0` for background, and any other value for "don't care".
/// The constructors only take their input in the form of images, not as a `dip::StructuringElement`
/// or `dip::Kernel`. The generic shapes that are easier to generate using those classes are not often
/// useful in intervals.
class DIP_NO_EXPORT Interval {
   public:

      /// \brief An interval can be constructed with a grey-value image, where `1` indicates foreground,
      /// `0` indicates background, and any other value indicates "don't care" pixels.
      ///
      /// The image must be odd in size, the origin is in the middle pixel.
      ///
      /// Such an image converts implicitly to an `%Interval`.
      DIP_EXPORT Interval( Image const& image );

      /// \brief An interval can be constructed with two binary images, one for the foreground mask
      /// and one for the background mask.
      ///
      /// The images must be odd in size, the origin is in the middle pixel.
      ///
      /// The two images must be disjoint, meaning that `dip::Any( dip::Infimum( hit, miss ))` must be false.
      /// An exception will be raised if this is not the case.
      DIP_EXPORT Interval( Image hit, Image miss );

      /// \brief Inverts the interval, swapping foreground and background pixels.
      void Invert() {
         DIP_THROW_IF( !miss_.IsForged(), "The inverted interval is not valid" );
         hit_.swap( miss_ );
      }

      /// \brief Returns the foreground mask image, a binary image.
      Image const& HitImage() const {
         return hit_;
      }

      /// \brief Returns the background mask image, a binary image.
      Image const& MissImage() const {
         return miss_;
      }

      /// \brief Returns true if the interval has at least one background pixel.
      bool HatMissSamples() const {
         return miss_.IsForged();
      }

      /// \brief Returns the sizes of the interval. The output array always has two elements.
      UnsignedArray const& Sizes() const {
         return hit_.Sizes();
      }

      /// \brief Returns rotated versions of the interval, applicable to 2D intervals only.
      ///
      /// `rotationAngle` can be 45, 90 or 180, the output vector will have 8, 4 or 2 intervals. Some
      /// of the interval images might point at the same data. Rotation angles of 45 degrees are only
      /// possible for square intervals. If the interval is not square, it will be made square by
      /// adding "don't care" pixels.
      ///
      /// `rotationDirection` affects the order of the intervals in the output vector:
      ///  - `"interleaved clockwise"` sorts the angles as follows: 0, 180, 45, 225, 90, 270, 135, 315.
      ///  - `"interleaved counter-clockwise"` is the same, but goes around the other way.
      ///  - `"clockwise"` sorts the angles as follows: 0, 45, 90, 135, 180, 225, 270, 315.
      ///  - `"counter-clockwise"` is the same, but goes around the other way.
      DIP_EXPORT IntervalArray GenerateRotatedVersions(
            dip::uint rotationAngle = 45,
            String rotationDirection = "interleaved clockwise"
      ) const;

   private:
      Image hit_;    // The interval must always have foreground pixels to be useful.
      Image miss_;   // If there are no background pixels in the interval, this one is raw.

      Interval() = default; // A default constructor that only class methods can use.
};

/// \brief Sup-generating operator, also known as hit-miss operator.
///
/// The sup-generating operator is a relaxed template matching, where `interval` is the template.
/// `interval` contains some pixels that must be foreground, and some that must be background, but
/// also allows "don't care" pixels, which will be ignored in the matching.
///
/// This operator is equal to the infimum of an erosion and an anti-erosion:
/// ```
///     out = dip::Infimum( dip::Erosion( in, hit ), dip::Erosion( ~in, miss ));
/// ```
/// where `hit` and `miss` are the two binary structuring elements in `interval`.
///
/// This function is specifically for binary images. Use `dip::HitAndMiss` for a more general operator.
DIP_EXPORT void SupGenerating(
      Image const& in,
      Image& out,
      Interval const& interval
);
inline Image SupGenerating(
      Image const& in,
      Interval const& interval
) {
   Image out;
   SupGenerating( in, out, interval );
   return out;
}

/// \brief Inf-generating operator, the dual of the Sup-generating operator.
///
/// This operator is equal to the supremum of a dilation and an anti-dilation:
/// ```
///     out = dip::Supremum( dip::Dilation( in, hit ), dip::Dilation( ~in, miss ));
/// ```
/// where `hit` and `miss` are the two binary structuring elements in `interval`.
///
/// This function is specifically for binary images.
DIP_EXPORT void InfGenerating(
      Image const& in,
      Image& out,
      Interval const& interval
);
inline Image InfGenerating(
      Image const& in,
      Interval const& interval
) {
   Image out;
   InfGenerating( in, out, interval );
   return out;
}

/// \brief Union of Sup-generating operators.
///
/// Applies the sup-generating operator with each of the intervals in `intervals`, and takes the union
/// of the results.
///
/// This function is specifically for binary images.
DIP_EXPORT void UnionSupGenerating(
      Image const& in,
      Image& out,
      IntervalArray const& intervals
);
inline Image UnionSupGenerating(
      Image const& in,
      IntervalArray const& intervals
) {
   Image out;
   UnionSupGenerating( in, out, intervals );
   return out;
}

/// \brief Union of Sup-generating operators.
///
/// Applies the sup-generating operator with all the rotated versions of `interval`, and takes the union
/// of the results. See `dip::Interval::GenerateRotatedVersions` for the definition of `rotationAngle`
/// and `rotationDirection`.
///
/// This function is specifically for 2D binary images.
inline void UnionSupGenerating2D(
      Image const& in,
      Image& out,
      Interval const& interval,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   DIP_START_STACK_TRACE
      auto intarray = interval.GenerateRotatedVersions( rotationAngle, rotationDirection );
      UnionSupGenerating( in, out, intarray );
   DIP_END_STACK_TRACE
}
inline Image UnionSupGenerating2D(
      Image const& in,
      Interval const& interval,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   Image out;
   UnionSupGenerating2D( in, out, interval, rotationAngle, rotationDirection );
   return out;
}

/// \brief Intersection of Inf-generating operators.
///
/// Applies the inf-generating operator with each of the intervals in `intervals`, and takes the intersection
/// of the results.
///
/// This function is specifically for binary images.
DIP_EXPORT void IntersectionInfGenerating(
      Image const& in,
      Image& out,
      IntervalArray const& intervals
);
inline Image IntersectionInfGenerating(
      Image const& in,
      IntervalArray const& intervals
) {
   Image out;
   IntersectionInfGenerating( in, out, intervals );
   return out;
}

/// \brief Intersection of Inf-generating operators.
///
/// Applies the inf-generating operator with all the rotated versions of `interval`, and takes the intersection
/// of the results. See `dip::Interval::GenerateRotatedVersions` for the definition of `rotationAngle`
/// and `rotationDirection`.
///
/// This function is specifically for 2D binary images.
inline void IntersectionInfGenerating2D(
      Image const& in,
      Image& out,
      Interval const& interval,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   DIP_START_STACK_TRACE
      auto intarray = interval.GenerateRotatedVersions( rotationAngle, rotationDirection );
      IntersectionInfGenerating( in, out, intarray );
   DIP_END_STACK_TRACE
}
inline Image IntersectionInfGenerating2D(
      Image const& in,
      Interval const& interval,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   Image out;
   IntersectionInfGenerating2D( in, out, interval, rotationAngle, rotationDirection );
   return out;
}

/// \brief Applies the thickening operator, optionally constrained by a mask, to an image.
///
/// Thickening is defined as `in + SupGenerating(in)`. The constrained operation is defined as
///  `in + (SupGenerating(in) & mask)`.
///
/// The operation is applied with each of the intervals in `intervals`, and repeated `iterations`
/// times. If `iterations` is 0, the operation is repeated until convergence.
///
/// A thickening with the right set of intervals leads to a background skeleton, also called skiz.
/// See `dip::HomotopicThickeningInterval2D`.
/// The intervals returned by `dip::HomotopicInverseEndPixelInterval2D` prune the skiz to single
/// points and circles.
///
/// This function is specifically for binary images.
DIP_EXPORT void Thickening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations = 0
);
inline Image Thickening(
      Image const& in,
      Image const& mask,
      IntervalArray const& intervals,
      dip::uint iterations = 0
) {
   Image out;
   Thickening( in, mask, out, intervals, iterations );
   return out;
}

/// \brief Applies the thickening operator, optionally constrained by a mask, to an image.
///
/// The operation is applied with with all the rotated versions of `interval`, and repeated `iterations`
/// times. See `dip::Thickening` for a description of the operation.
/// See `dip::Interval::GenerateRotatedVersions` for the definition of `rotationAngle` and `rotationDirection`.
///
/// This function is specifically for 2D binary images.
inline void Thickening2D(
      Image const& in,
      Image const& mask,
      Image& out,
      Interval const& interval,
      dip::uint iterations = 0,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   DIP_START_STACK_TRACE
      auto intarray = interval.GenerateRotatedVersions( rotationAngle, rotationDirection );
      Thickening( in, mask, out, intarray, iterations );
   DIP_END_STACK_TRACE
}
inline Image Thickening2D(
      Image const& in,
      Image const& mask,
      Interval const& interval,
      dip::uint iterations = 0,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   Image out;
   Thickening2D( in, mask, out, interval, iterations, rotationAngle, rotationDirection );
   return out;
}

/// \brief Applies the thinning operator, optionally constrained by a mask, to an image.
///
/// Thinning is defined as `in - SupGenerating(in)`. The constrained operation is defined as
///  `in - (SupGenerating(in) & mask)`.
///
/// The operation is applied with each of the intervals in `intervals`, and repeated `iterations`
/// times. If `iterations` is 0, the operation is repeated until convergence.
///
/// A thinning with the right set of intervals leads to a skeleton. See `dip::HomotopicThinningInterval2D`.
/// The intervals returned by `dip::HomotopicEndPixelInterval2D` prune the skeleton to single points and
/// circles.
///
/// This function is specifically for binary images.
DIP_EXPORT void Thinning(
      Image const& in,
      Image const& mask,
      Image& out,
      IntervalArray const& intervals,
      dip::uint iterations = 0
);
inline Image Thinning(
      Image const& in,
      Image const& mask,
      IntervalArray const& intervals,
      dip::uint iterations = 0
) {
   Image out;
   Thinning( in, mask, out, intervals, iterations );
   return out;
}

/// \brief Applies the thinning operator, optionally constrained by a mask, to an image.
///
/// The operation is applied with with all the rotated versions of `interval`, and repeated `iterations`
/// times. See `dip::Thinning` for a description of the operation.
/// See `dip::Interval::GenerateRotatedVersions` for the definition of `rotationAngle` and `rotationDirection`.
///
/// This function is specifically for 2D binary images.
inline void Thinning2D(
      Image const& in,
      Image const& mask,
      Image& out,
      Interval const& interval,
      dip::uint iterations = 0,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   DIP_START_STACK_TRACE
      auto intarray = interval.GenerateRotatedVersions( rotationAngle, rotationDirection );
      Thinning( in, mask, out, intarray, iterations );
   DIP_END_STACK_TRACE
}
inline Image Thinning2D(
      Image const& in,
      Image const& mask,
      Interval const& interval,
      dip::uint iterations = 0,
      dip::uint rotationAngle = 45,
      String const& rotationDirection = "interleaved clockwise"
) {
   Image out;
   Thinning2D( in, mask, out, interval, iterations, rotationAngle, rotationDirection );
   return out;
}

/// \brief Returns a 2D interval array for homotopic thinning.
///
/// Use with `dip::Thinning` to shrink objects without changing the Euler number.
/// Note that `dip::ConditionalThinning2D` is more efficient, though the two functions do not produce exactly
/// the same output. To create a skeleton, use `dip::EuclideanSkeleton`.
///
/// `connectivity` can be 1 to produce 4-connected skeletons, or 2 for 8-connected skeletons.
DIP_EXPORT IntervalArray HomotopicThinningInterval2D( dip::uint connectivity = 2 );

/// \brief Returns a 2D interval array for homotopic thickening. Use with `dip::Thickening` to grow objects
/// without merging them. This produces a background skeleton (also known as skiz).
/// Note that `dip::ConditionalThickening2D` is more efficient, though the two options do not produce exactly
/// the same output. To create a background skeleton, use `dip::EuclideanSkeleton` on the inverted image.
///
/// `connectivity` can be 1 to produce 4-connected skeletons, or 2 for 8-connected skeletons.
inline IntervalArray HomotopicThickeningInterval2D( dip::uint connectivity = 2 ) {
   DIP_START_STACK_TRACE
      IntervalArray out = HomotopicThinningInterval2D( connectivity );
      for( auto& intv : out ) {
         intv.Invert();
      }
      return out;
   DIP_END_STACK_TRACE
}

/// \brief Returns an interval array for detecting end pixels. Includes isolated pixels.
///
/// Use with `dip::UnionSupGenerating` to detect skeleton end pixels. Note that `dip::GetEndPixels`
/// is more efficient.
///
/// `connectivity` can be 1 to work with 4-connected skeletons, or 2 for 8-connected skeletons.
DIP_EXPORT IntervalArray EndPixelInterval2D( dip::uint connectivity = 2 );

/// \brief Returns an interval array for detecting end pixels. Excludes isolated pixels
///
/// Use with `dip::Thinning` to prune end points from the skeleton.
///
/// `connectivity` can be 1 to prune 4-connected skeletons, or 2 for 8-connected skeletons.
DIP_EXPORT IntervalArray HomotopicEndPixelInterval2D( dip::uint connectivity = 2 );

/// \brief Returns an interval array for detecting end background pixels. Excludes isolated pixels.
///
/// Use with `dip::Thickening` to prune end points from the background skeleton.
///
/// `connectivity` can be 1 to prune 4-connected skeletons, or 2 for 8-connected skeletons.
inline IntervalArray HomotopicInverseEndPixelInterval2D( dip::uint connectivity = 2 ) {
   DIP_START_STACK_TRACE
      IntervalArray out = HomotopicEndPixelInterval2D( connectivity );
      for( auto& intv : out ) {
         intv.Invert();
      }
      return out;
   DIP_END_STACK_TRACE
}

/// \brief Returns an interval for detecting single pixels. Use with `dip::SupGenerating` to detect isolated
/// pixels. Note that `dip::GetSinglePixels` is more efficient.
DIP_EXPORT Interval SinglePixelInterval( dip::uint nDims = 2 );

/// \brief Returns a 2D interval array for detecting branch pixels.
///
/// Use with `dip::UnionSupGenerating` to detect skeleton branch pixels. Note that `dip::GetBranchPixels`
/// is more efficient.
DIP_EXPORT IntervalArray BranchPixelInterval2D();

/// \brief Returns a 2D interval for detecting boundary pixels.
///
/// Use with `dip::UnionSupGenerating2D` to detect object boundary pixels. Set `rotationAngle` to 45 to detect pixels
/// 8-connected to the background, and to 90 to detect pixels 4-connected to the background. Note that the
/// difference between the input image and the eroded image accomplishes the same thing.
DIP_EXPORT Interval BoundaryPixelInterval2D();

/// \brief Returns a 2D interval array to thicken to a convex hull.
///
/// Use with `dip::Thickening` to grow concave sections of objects until the objects are all convex.
/// This leads to hexagonal shapes.
DIP_EXPORT IntervalArray ConvexHullInterval2D();

/// \}

} // namespace dip

#endif // DIP_BINARY_H
