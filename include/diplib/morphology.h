/*
 * DIPlib 3.0
 * This file contains declarations for mathematical morphology functions
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

#ifndef DIP_MORPHOLOGY_H
#define DIP_MORPHOLOGY_H

#include "diplib.h"
#include "diplib/neighborhood.h"


/// \file
/// \brief Declares functions that implement mathematical morphology.


namespace dip {


/// \defgroup morphology Mathematical morphology
/// \ingroup filtering
/// \brief Morphological filters for smoothing, sharpening, detection and more, and the watershed transform.
/// \{

/// \brief Objects of class `%StructuringElement` represent the shape and size of a structuring element.
///
/// Many functions in the Mathematical Morphology module require a structuring element definition.
/// There are two ways to define a structuring element: the user can specify the shape name and the size
/// of a structuring element, and the user can pass an image containing the structuring element.
///
/// Objects of type `dip::Image`, `dip::FloatArray` and `dip::String` implicitly convert to
/// a `%dip::StructuringElement`, so it should be convenient to use these various representations in your
/// code.
///
/// To define a structuring element by shape and size, pass a string defining the shape, and a
/// floating-point array with the size along each dimension.
/// These are the valid shape strings, and the corresponding meaning of the size array:
///
/// -  `"rectangular"`, `"elliptic"`, and `"diamond"`: these are unit circles in different metrics. The
///    size array corresponds to the diameter in that metric. `"elliptic"` is the default shape,
///    because it is isotropic, but it also is the slowest to compute. Both `"elliptic"` and `"diamond"`
///    structuring elements always are symmetric. That is, their origin is centered on a pixel.
///    The pixels included in the shape are those at most half of the diameter away from the origin.
///    For the `"rectangular"` structuring element, a box with integer sizes is always generated,
///    but the box can be even in size also, meaning that the origin is in between pixels.
///    Any size array element that is smaller or equal to 1 causes that dimension to not be processed.
///
/// -  `"parabolic"`: the parabolic structuring element is the morphological equivalent to the Gaussian
///    kernel in linear filtering. It is separable and perfectly isotropic. The size array corresponds
///    to the scaling of the parabola (i.e. the \f$a\f$ in \f$a^{-2} x^2\f$). A value equal
///    or smaller to 0 causes that dimension to not be processed. The boundary condition is ignored
///    for operators with this structuring element, and the output image is always a floating-point type.
///
/// -  `"interpolated line"`, `"discrete line"`: these are straight lines, using different implementations.
///    The size array corresponds to the size of the bounding box of the line, with signs indicating
///    the direction. Thus, if the size array is `[2,2]`, the line goes right and down two pixels,
///    meaning that the line is formed by two pixels at an angle of 45 degrees down. If the size array
///    is `[-2,2]`, then the line is again two pixels, but at an angle of 125 degrees. (Note that
///    in images, angles increase clockwise from the x-axis, as the y-axis is inverted).
///
/// To define a structuring element through an image, provide either a binary or grey-value image.
/// If the image is binary, the "set" pixels form the structuring element. If the image is a grey-value
/// image, those grey values are directly used as structuring element values. Set pixels to negative infinity
/// to exclude them from the structuring element (the result would be the same by setting them to
/// a value lower than the range of the input image, but the algorithm should be more efficient if
/// those pixels are excluded).
///
/// Note that the image is directly used as neighborhood (i.e. no mirroring is applied).
/// That is, `dip::Dilation` and `dip::Erosion` will use the same neighborhood. Their composition only
/// leads to an opening or a closing if the structuring element is symmetric. For non-symmetric structuring
/// elements images, you need to mirror it in one of the two function calls:
/// ```cpp
///     dip::Image se = ...;
///     dip::Image out = dip::Erosion( in, se );
///     out = dip::Dilation( out, se.Mirror() );
/// ```
/// (Do note that, in the code above, `se` itself is modified! use `se.QuickCopy().Mirror()` to prevent
/// that.)
///
/// As elsewhere, the origin of the structuring element is in the middle of the image, on the pixel to
/// the right of the center in case of an even-sized image.
///
/// See dip::Kernel, dip::PixelTable
class DIP_NO_EXPORT StructuringElement{
      // TODO: In the old DIPlib, line SEs used filterParam = [length,angle], and only applied to 2D images!
      // TODO: Implement the discrete line for 2D without skews, so it's more efficient.
      // TODO: Implement periodic lines, construct translation-invariant discrete lines using periodic lines
      // TODO: Construct diamond SE and approximations to elliptic SE using lines
   public:
      enum class ShapeCode {
            RECTANGULAR,
            ELLIPTIC,
            DIAMOND,
            PARABOLIC,
            INTERPOLATED_LINE,
            DISCRETE_LINE,
            CUSTOM
      };

      /// \brief The default structuring element is a disk with a diameter of 7 pixels.
      StructuringElement() : shape_( ShapeCode::ELLIPTIC ), params_( { 7 } ) {}

      /// \brief A string implicitly converts to a structuring element, it is interpreted as a shape.
      StructuringElement( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }

      /// \brief A `dip::FloatArray` implicitly converts to a structuring element, it is interpreted as the
      /// parameter of the SE for all dimensions. A second argument specifies the shape.
      StructuringElement( FloatArray const& params, String const& shape = "elliptic" ) : params_( params ) {
         SetShape( shape );
      }

      /// \brief A floating-point value implicitly converts to a structuring element, it is interpreted as the
      /// parameter of the SE along each dimension. A second argument specifies the shape.
      StructuringElement( dfloat param, String const& shape = "elliptic" ) : params_( FloatArray{ param } ) {
         SetShape( shape );
      }

      /// \brief An image implicitly converts to a structuring element.
      StructuringElement( Image const& image ) : shape_( ShapeCode::CUSTOM ), image_( image.QuickCopy() ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( image_.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
      }

      /// \brief Mirrors the structuring element.
      void Mirror() {
         mirror_ = !mirror_;
      }

      /// \brief Converts the Structuring element into a kernel
      // NOTE: When we go to SEs that are sequences of kernels, this function will change!
      dip::Kernel Kernel() const {
         dip::Kernel out;
         switch( shape_ ) {
            case ShapeCode::RECTANGULAR:
               out = { Kernel::ShapeCode::RECTANGULAR, params_ };
               break;
            case ShapeCode::ELLIPTIC:
               out = { Kernel::ShapeCode::ELLIPTIC, params_ };
               break;
            case ShapeCode::DIAMOND:
               out = { Kernel::ShapeCode::DIAMOND, params_ };
               break;
            case ShapeCode::CUSTOM:
               out = { image_ };
               break;
            default:
               DIP_THROW( "Cannot create kernel for this structuring element shape" );
         }
         if( mirror_ ) {
            out.Mirror();
         }
         return out;
      }

      /// \brief Retrieves the size array, adjusted to an image of size `imsz`.
      FloatArray Params( UnsignedArray const& imsz ) const {
         dip::uint nDim = imsz.size();
         DIP_THROW_IF( nDim < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         FloatArray out;
         if( IsCustom() ) {
            DIP_THROW_IF( image_.Dimensionality() > nDim, E::DIMENSIONALITIES_DONT_MATCH );
            out = FloatArray{ image_.Sizes() };
            out.resize( nDim, 1 ); // expand dimensionality by adding singletons
         } else {
            out = params_;
            DIP_START_STACK_TRACE
               ArrayUseParameter( out, nDim, 1.0 );
            DIP_END_STACK_TRACE
         }
         return out;
      }

      /// \brief Returns the structuring element parameters, not adjusted to image dimensionality.
      FloatArray const& Params() const { return params_; }

      /// \brief Returns the structuring element shape
      ShapeCode Shape() const { return shape_; }

      /// \brief Tests to see if the kernel is a custom shape
      bool IsCustom() const { return shape_ == ShapeCode::CUSTOM; }

      /// \brief Tests to see if the structuring element is flat or grey-valued
      bool IsFlat() const {
         if( IsCustom() ) {
            return image_.DataType().IsBinary();
         } else {
            return shape_ != ShapeCode::PARABOLIC;
         }
      }

   private:
      ShapeCode shape_;
      FloatArray params_;
      Image image_;
      bool mirror_ = false;

      void SetShape( String const& shape ) {
         if( shape == "elliptic" ) {
            shape_ = ShapeCode::ELLIPTIC;
         } else if( shape == "rectangular" ) {
            shape_ = ShapeCode::RECTANGULAR;
         } else if( shape == "diamond" ) {
            shape_ = ShapeCode::DIAMOND;
         } else if( shape == "parabolic" ) {
            shape_ = ShapeCode::PARABOLIC;
         } else if( shape == "interpolated line" ) {
            shape_ = ShapeCode::INTERPOLATED_LINE;
         } else if( shape == "discrete line" ) {
            shape_ = ShapeCode::DISCRETE_LINE;
         } else {
            DIP_THROW_INVALID_FLAG( shape );
         }
      }
};


//
// Basic operators
//


/// \brief Applies the dilation.
///
/// `se` defines the structuring element.
///
/// `boundary` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add min"`, but any value can be used.
///
/// \see dip::Erosion, dip::Opening, dip::Closing
DIP_EXPORT void Dilation(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
inline Image Dilation(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Dilation( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the erosion with a standard structuring element.
///
/// `se` defines the structuring element.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add max"`, but any value can be used.
///
/// \see dip::Dilation, dip::Opening, dip::Closing
DIP_EXPORT void Erosion(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
inline Image Erosion(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Erosion( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the closing with a standard structuring element.
///
/// `se` defines the structuring element.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the closing are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see dip::Dilation, dip::Erosion, dip::Opening
DIP_EXPORT void Closing(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
inline Image Closing(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Closing( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the opening with a standard structuring element.
///
/// `se` defines the structuring element.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the opening are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see dip::Dilation, dip::Erosion, dip::Closing
DIP_EXPORT void Opening(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
inline Image Opening(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Opening( in, out, se, boundaryCondition );
   return out;
}


//
// Filters
//


/// \brief The Top-hat operator and its variants
///
/// The top-hat is the difference between a morphological operation and the original image,
/// comparable to a high-pass filter. The flags `edgeType` and `polarity` define which
/// operation is applied.
///
/// `edgeType` can be one of:
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `polarity` can be either `"white"` to indicate objects are brighter than the background, or `"black"` to
/// indicate objects are darker than the background.
///
/// The standard top-hat is defined as the `Opening( in ) - in`. This is the operation obtained
/// with the default values.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void Tophat(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = "texture", // "object", "both", "dynamic"=="both"
      String const& polarity = "white", // "black"
      StringArray const& boundaryCondition = {}
);
inline Image Tophat(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      String const& polarity = "white",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Tophat( in, out, se, edgeType, polarity, boundaryCondition );
   return out;
}

/// \brief A morphological smoothing filter
///
/// Implements a morphological smoothing based on the average of two complementary morphological operations.
/// These can be chosen through the `edgeType` parameter.
///
/// `edgeType` can be one of:
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void MorphologicalThreshold(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalThreshold(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalThreshold( in, out, se, edgeType, boundaryCondition );
   return out;
}

// TODO: Document MorphologicalGist. It was undocumented in the old DIPlib.
DIP_EXPORT void MorphologicalGist(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalGist(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalGist( in, out, se, edgeType, boundaryCondition );
   return out;
}

/// \brief A morphological edge detector
///
/// Implements a morphological edge detector based on the difference of two complementary morphological
/// operations. These can be chosen through the `edgeType` parameter.
///
/// `edgeType` can be one of:
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void MorphologicalRange(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalRange(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalRange( in, out, se, edgeType, boundaryCondition );
   return out;
}

/// \brief The morphological version of the gradient magnitude
///
/// The morphological gradient magnitude is defined as `Dilation( in ) - Erosion( in )`.
///
/// This function is implemented by a call to `dip::MorphologicalRange` with `edgeType` set to `"both"`.
inline void MorphologicalGradientMagnitude(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   MorphologicalRange( in, out, se, "both", boundaryCondition );
}
inline Image MorphologicalGradientMagnitude(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalGradientMagnitude( in, out, se, boundaryCondition );
   return out;
}

/// \brief A morphological edge detector
///
/// Implements a morphological edge detector based on the minimum of two complementary morphological
/// operations. These can be chosen through the `edgeType` parameter.
///
/// `edgeType` can be one of:
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// If `sign` is `"unsigned"`, `%Lee` computes the absolute edge strength. `sign` can also be `"signed"`
/// to compute the signed edge strength.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void Lee(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      String const& sign = "unsigned",
      StringArray const& boundaryCondition = {}
);
inline Image Lee(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = "texture",
      String const& sign = "unsigned",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Lee( in, out, se, edgeType, sign, boundaryCondition );
   return out;
}

/// \brief A morphological smoothing filter
///
/// Implements a morphological smoothing based on the sequence of two complementary morphological
/// operations. These can be chosen through the `mode` parameter.
///
/// `mode` can be one of:
/// - `"open-close"`: applies the opening first, then the closing.
/// - `"close-open"`: applies the closing first, then the opening.
/// - `"average"`: computes the average of the result of the first two modes.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void MorphologicalSmoothing(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& mode = "average", // "open-close", "close-open"
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalSmoothing(
      Image const& in,
      StructuringElement const& se = {},
      String const& mode = "average",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalSmoothing( in, out, se, mode, boundaryCondition );
   return out;
}

/// \brief A morphological edge detector
///
/// This function computes the average morphological gradient over a range of scales bounded by `upperSize`
/// and `lowerSize`. The morphological gradient is computed as the difference of the dilation and erosion
/// of the input image at a particular scale, eroded by an erosion of one size smaller. At the lowest scale,
/// the diameter of the structuring element is `2 * lowerSize + 1`.
///
/// `filterShape` can be either `"rectangular"`, `"elliptic"`, and `"diamond"`, as described in
/// `dip::PixelTable::PixelTable( String const&, FloatArray, dip::uint )`.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
DIP_EXPORT void MultiScaleMorphologicalGradient(
      Image const& in,
      Image& out,
      dip::uint upperSize = 9,
      dip::uint lowerSize = 3,
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);
inline Image MultiScaleMorphologicalGradient(
      Image const& in,
      dip::uint upperSize = 9,
      dip::uint lowerSize = 3,
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MultiScaleMorphologicalGradient( in, out, upperSize, lowerSize, filterShape, boundaryCondition );
   return out;
}

/// \brief The morphological version of the Laplace operator
///
/// This function computes:
/// ```cpp
///     out = ( Dilation( in ) + Erosion( in ) ) / 2 - in;
/// ```
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See `dip::Dilation`, `dip::Erosion`, `dip::Opening` and/or `dip::Closing` for a description
/// of these parameters.
DIP_EXPORT void MorphologicalLaplace(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalLaplace(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalLaplace( in, out, se, boundaryCondition );
   return out;
}


//
// Priority-queue--based algorithms
//


/// \brief Computes the watershed of `in` within `mask`, with on-line merging of regions.
///
/// The watershed is a segmentation algorithm that divides the image according to its grey-value
/// ridges.
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads
/// to vertex-connected watershed lines (i.e. thinnest possible result).
///
/// `flags` determines how the output is computed. There are three options:
/// - "labels" or "binary": returns either the labels used during processing, with the watershed
///   lines as background (value 0), or a binary image where the watershed lines are set and the
///   regions are not set. "binary" is the default. In the "labels" case, labels used are not
///   necessarily consecutive (i.e. not all label values between 1 and the largest label are
///   necessarily used).
/// - "low first" or "high first": determines the sort order of pixels. The default of "low first"
///   yields the normal watershed, where local minima are origin of the basins, and the watershed
///   lines run along the high ridges in the image. "high first" simply inverts the definition,
///   such that local maxima are at the centers of the basins, and the watershed lines run along
///   the low valleys.
/// - "fast" or "correct": determines which algorithm is used: "fast" is an algorithm that takes
///   a few shortcuts, but usually manages to produce good results any way. One shortcut leads to
///   all border pixels being marked as watershed lines. It is possible to extend the image by one
///   pixel before processing to circumvent this. The other shortcut means that plateaus are not
///   handled correctly. A plateau is a region in the image where pixels have exactly the same
///   value. This is usually seen as watershed lines not running through the middle of the
///   plateaus, instead being shifted to one side. Adding a little bit of noise to the image, and
///   setting `maxDepth` to the range of the noise, usually improves the results in these cases
///   a little bit. "correct" is an algorithm that first finds the local minima through
///   `dip::Minima` (or maxima if "high first" is set), and then applies `dip::SeededWatershed`.
///   This always produces correct results, but is significantly slower.
///
/// The on-line region merging works as follows: When two regions first meet, a decision is
/// made on whether to keep the regions separate (and thus put a watershed pixel at that point),
/// or to merge the regions. If one of the regions is no deeper than `maxDepth` (i.e. the intensity
/// difference between the region's minimum and the pixel where the region meets another), and is
/// no larger than `maxSize` (i.e. the number of pixels belonging to the region and that have been
/// seen so far), then it can be merged. The merged region is subsequently treated as a single
/// region, and their labels are considered equal. If `maxSize` is zero, no test for size is done.
/// In this case, the merging is exactly equivalent to applying an H-minima transform to the image
/// before computing the watershed.
///
/// Any pixel that is infinity will be part of the watershed lines, as is any pixel not within
/// `mask`.
///
/// See \ref connectivity for information on the connectivity parameter.
DIP_EXPORT void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 0, // only merging within plateaus
      dip::uint maxSize = 0,
      StringSet flags = {} // "labels" / "binary"(default), "low first"(default) / "high first", "fast"(default) / "correct"
);
inline Image Watershed(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1.0,
      dip::uint maxSize = 0,
      StringSet const& flags = {}
) {
   Image out;
   Watershed( in, mask, out, connectivity, maxDepth, maxSize, flags );
   return out;
}

/// \brief Computes the watershed of `in` within `mask`, starting at `seeds`, with on-line merging of regions.
///
/// `seeds` is a binary or labeled image (if binary, it is labeled using `connectivity`). These labels are
/// iteratively expanded in the watershed order (i.e. pixels that have a low value in `in` go first) until
/// they meet. Pixels where two regions meet are marked as the watershed lines. `seeds` is commonly used
/// to direct the segmentation, and merging is consequently not necessary. However, this algorithm does
/// include on-line merging. Note that two labeled regions in `seeds` that do not have a grey-value ridge
/// between them (i.e. they are on the same plateau) will always be merged. Merged labels will be painted
/// with the label of one of the originating seeds, and the other labels will not be present in the output
/// (only if `flags` contains "labels").
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads
/// to vertex-connected watershed lines (i.e. thinnest possible result).
///
/// See `dip::Watershed` for a description of the merging parameters (`maxDepth`, `maxSize`), and the
/// `flags` parameter.
///
/// See \ref connectivity for information on the connectivity parameter.
DIP_EXPORT void SeededWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1.0,
      dip::uint maxSize = 0,
      StringSet const& flags = {}
);
inline Image SeededWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1.0,
      dip::uint maxSize = 0,
      StringSet const& flags = {}
) {
   Image out;
   SeededWatershed( in, seeds, mask, out, connectivity, maxDepth, maxSize, flags );
   return out;
}

/// \brief Marks significant local minima.
///
/// This algorithm works exactly like `dip::Watershed` with the "fast" flag set. All pixels with a value
/// equal to the lowest value within each watershed basin form a local minimum. Note that they can form
/// disconnected regions, use the "labels" flag to recognize such disconnected regions as a single local
/// minimum. See `dip::Watershed` for a description of all the parameters.
///
/// `output` can be "binary" or "labels", and determines whether the algorithm outputs a binary image or
/// a labeled image. Note that the labels are not necessarily consecutive (i.e. not all label values between
/// 1 and the largest label are necessarily used).
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see  WatershedMaxima, Minima, Maxima.
DIP_EXPORT void WatershedMinima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
);
inline Image WatershedMinima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
) {
   Image out;
   WatershedMinima( in, mask, out, connectivity, maxDepth, maxSize, output );
   return out;
}

/// \brief Marks significant local maxima.
///
/// This algorithm works exactly like `dip::Watershed` with the "fast" flag set. All pixels with a value
/// equal to the highest value within each watershed basin form a local maximum. Note that they can form
/// disconnected regions, use the "labels" flag to recognize such disconnected regions as a single local
/// maximum. See `dip::Watershed` for a description of all the parameters.
///
/// `output` can be "binary" or "labels", and determines whether the algorithm outputs a binary image or
/// a labeled image. Note that the labels are not necessarily consecutive (i.e. not all label values between
/// 1 and the largest label are necessarily used).
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see  WatershedMinima, Maxima, Minima.
DIP_EXPORT void WatershedMaxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
);
inline Image WatershedMaxima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
) {
   Image out;
   WatershedMaxima( in, mask, out, connectivity, maxDepth, maxSize, output );
   return out;
}

/// \brief Marks local minima.
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a higher value. If `output` is "binary", the result is a binary image where these
/// pixels and plateaus are set. If `output` is "labels", the result is a labeled image, where the labels used
/// are not necessarily consecutive (i.e. not all label values between 1 and the largest label are necessarily used).
///
/// If `mask` is given, it restricts the area in the image that is searched.
///
/// For images that have large plateaus (regions of constant value) that are not local minima, this function can
/// be quite slow. For example, an image that is zero everywhere except for a small valley. For such an image
/// it is recommended to use the `mask` input, for example with the output of a threshold operation.
///
/// See \ref connectivity for information on the connectivy parameter.
///
/// \see  Maxima, WatershedMinima, WatershedMaxima.
DIP_EXPORT void Minima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      String const& output = "binary"
);
inline Image Minima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      String const& output = "binary"
) {
   Image out;
   Minima( in, mask, out, connectivity, output );
   return out;
}

/// \brief Marks local maxima.
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a lower value. If `output` is "binary", the result is a binary image where these
/// pixels and plateaus are set. If `output` is "labels", the result is a labeled image, where the labels used
/// are not necessarily consecutive (i.e. not all label values between 1 and the largest label are necessarily used).
///
/// If `mask` is given, it restricts the area in the image that is searched.
///
/// For images that have large plateaus (regions of constant value) that are not local maxima, this function can
/// be quite slow. For example, an image that is zero everywhere except for a small peak. For such an image
/// it is recommended to use the `mask` input, for example with the output of a threshold operation.
///
/// See \ref connectivity for information on the connectivy parameter.
///
/// \see  Minima, WatershedMaxima, WatershedMinima.
DIP_EXPORT void Maxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      String const& output = "binary"
);
inline Image Maxima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      String const& output = "binary"
) {
   Image out;
   Maxima( in, mask, out, connectivity, output );
   return out;
}

// TODO: Port and document UpperEnvelope
DIP_EXPORT void UpperEnvelope(
      Image const& in,
      Image& out,
      Image& bottom,
      Image& labels,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0
);

/// \brief Reconstruction by dilation or erosion.
///
/// Iteratively dilates (erodes) the image `marker` such that it remains lower (higher) than `in` everywhere, until
/// stability. This is implemented with an efficient priority-queue--based method. `direction` indicates which of
/// the two operations to apply ("dilation" or "erosion").
///
/// `out` will have the data type of `in`, and `marker` will be cast to that same type (with clamping to the target
/// range, see `dip::Convert`).
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// Literature: K. Robinson and P.F. Whelan: Efficient morphological reconstruction: a downhill filter,
/// Pattern Recognition Letters 25:1759-1767, 2004.
DIP_EXPORT void MorphologicalReconstruction(
      Image const& marker,
      Image const& in, // grey-value mask
      Image& out,
      dip::uint connectivity = 1,
      String const& direction = "dilation" // alt: "erosion"
);
inline Image MorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      dip::uint connectivity = 1,
      String const& direction = "dilation"
) {
   Image out;
   MorphologicalReconstruction( marker, in, out, connectivity, direction );
   return out;
}

/// \brief Computes the H-Minima filtered image
///
/// The H-Minima filtered image has all local minima with a depth less than `h` removed:
///
/// ```cpp
///     HMinima = dip::MorphologicalReconstruction( in + h, in, connectivity, "erosion" );
/// ```
///
/// \see dip::MorphologicalReconstruction, dip::Minima, dip::HMaxima
inline void HMinima(
      Image const& in,
      Image& out,
      dfloat h,
      dip::uint connectivity = 1
) {
   Image tmp = Add( in, h, in.DataType() );
   MorphologicalReconstruction( tmp, in, out, connectivity, "erosion" );
}
inline Image HMinima(
      Image const& in,
      dfloat h,
      dip::uint connectivity = 1
) {
   Image out;
   HMinima( in, out, h, connectivity );
   return out;
}

/// \brief Computes the H-Maxima filtered image
///
/// The H-Maxima filtered image has all local maxima with a height less than `h` removed:
///
/// ```cpp
///     HMinima = dip::MorphologicalReconstruction( in - h, in, connectivity, "dilation" );
/// ```
///
/// \see dip::MorphologicalReconstruction, dip::Maxima, dip::Hminima
inline void HMaxima(
      Image const& in,
      Image& out,
      dfloat h,
      dip::uint connectivity = 1
) {
   Image tmp = Subtract( in, h, in.DataType() );
   MorphologicalReconstruction( tmp, in, out, connectivity, "dilation" );
}
inline Image HMaxima(
      Image const& in,
      dfloat h,
      dip::uint connectivity = 1
) {
   Image out;
   HMaxima( in, out, h, connectivity );
   return out;
}

DIP_EXPORT void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 1,
      String const& polarity = "opening" // vs "closing"
);

/// \brief Applies a path opening in all possible directions
///
/// `length` is the length of the path. All `filterParam` arguments to `dip::DirectedPathOpening` that yield a
/// lenght of `length` and represent unique directions are generated, and the directed path opening is computed
/// for each of them. The supremum (when `polarity` is `"opening"`) or infimum (when it is `"closing"`) is
/// computed over all results. See `dip::DirectedPathOpening` for a description of the algorithm and the parameters.
DIP_EXPORT void PathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint length = 7,
      String const& polarity = "opening",
      String const& mode = "normal"
);
inline Image PathOpening(
      Image const& in,
      Image const& mask,
      dip::uint length = 7,
      String const& polarity = "opening",
      String const& mode = "normal"
) {
   Image out;
   PathOpening( in, mask, out, length, polarity, mode );
   return out;
}

/// \brief Applies a path opening in a specific direction.
///
/// The path opening is an opening over all possible paths of a specific length and general direction. A path
/// direction represents a 90 degree cone within which paths are generated. The paths are formed by single pixel
/// steps in one of three directions (in 2D): the main direction, or 45 degrees to the left or right. That is,
/// if the main direction is [1,0] (to the right), then [1,-1] and [1,1] (diagonal up or down) are also possible
/// steps. This leads to a number of different paths that is exponential in its lengths. However, the opening over
/// all these paths can be computed in \f$O(n \log(n))\f$ time, with *n* the path length.
///
/// The direction description above can be generalized to any number of dimensions by realizing that the main direction
/// can be specified by any of the neighbors of a central pixel, and then the other allowed steps are the neighbor
/// pixels that are also neighbor to the pixel that represents the main direction. In 3D, this leads to 6 or 8 alternate
/// steps.
///
/// There are 4 possible path directions in 2D, and 13 in 3D. Both length and direction are specified through
/// the `filterParam` argument, see below. Note that the path length is given by the number of pixels in the path,
/// not the Euclidean length of the path.
///
/// When `mode` is `"constrained"`, the path construction described above is modified such that, after every alternate
/// step, a step in the main direction must be taken. This constraint avoids a zig-zag line that causes the path
/// opening to yield much shorter lines for the diagonal directions if the lines in the image are thicker than one pixel.
/// See the paper by Luengo referenced below. It also reduces the cone size from 90 degrees to 45 degrees, making the
/// algorithm more directionally-selective. The constrained mode increases computation time a little, but is highly
/// recommended when using the path opening in a granulometry.
///
/// **Definition of `filterSize`:** `length = max(abs(filterSize))` is the number of pixels in the line.
/// The path direction is determined by translating `filterSize` to an array with -1, 0 and 1 values using
/// `direction = round(filterSize/length)`. For example, if `filterSize=[7,0]`, then `length` is 7, and
/// `direction` is `[1,0]` (to the right), with `[1,1]` and `[1,-1]` as alternate directions.
///
/// Literature:
///  - H. Heijmans, M. Buckley and H. Talbot, "Path Openings and Closings", Journal of Mathematical Imaging and Vision 22:107-119, 2005.
///  - H. Talbot, B. Appleton, "Efficient complete and incomplete path openings and closings", Image and Vision Computing 25:416-425, 2007.
///  - C.L. Luengo Hendriks, "Constrained and dimensionality-independent path openings", IEEE Transactions on Image Processing 19(6):1587–1595, 2010.
DIP_EXPORT void DirectedPathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntegerArray filterParam,
      String const& polarity = "opening", // vs "closing"
      String const& mode = "normal"  // vs "constrained"
);
inline Image DirectedPathOpening(
      Image const& in,
      Image const& mask,
      IntegerArray filterParam,
      String const& polarity = "opening", // vs "closing"
      String const& mode = "normal"  // vs "constrained"
) {
   Image out;
   DirectedPathOpening( in, mask, out, filterParam, polarity, mode );
   return out;
}

// TODO: Have dip::Opening(in,{n,"path"}) call dip::PathOpening(in,n)   --   The SE should not be valid in
// TODO: Have dip::Opening(in,{[n,m],"path"}) call dip::PathOpening(in,[n,m])


// TODO: functions to port:
/*
dip_UpperEnvelope (dip_morphology.h)
dip_AreaOpening (dip_morphology.h)
dip_UpperSkeleton2D (dip_binary.h)
*/

// TODO: opening and closing by reconstruction
// TODO: alternating sequential open-close filter (3 versions: with structural opening, opening by reconstruction, and area opening)
// TODO: hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees (360 degrees means no rotations).
// TODO: thinning & thickening, to be implemented as iterated hit'n'miss.
// TODO: levelling
// TODO: granulometries (isotropic and path opening)

// TODO: link documentation to dip::PercentileFilter (when written) as a rank filter, and implement rank-min and rank-max operators

/// \}

} // namespace dip

#endif // DIP_MORPHOLOGY_H
