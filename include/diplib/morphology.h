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


/// \file
/// \brief Mathematical morphology operators and filters.
/// \see morphology


namespace dip {


// Forward declaration, from diplib/kernel.h
class DIP_NO_EXPORT Kernel;


/// \defgroup morphology Mathematical morphology
/// \ingroup filtering
/// \brief Morphological filters for smoothing, sharpening, detection and more, and the watershed transform.
/// \{


/// \brief Represents the shape and size of a structuring element.
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
/// -  `"octagonal"`: an approximation to the ellipse in 2D only.
///
/// -  `"parabolic"`: the parabolic structuring element is the morphological equivalent to the Gaussian
///    kernel in linear filtering. It is separable and perfectly isotropic. The size array corresponds
///    to the scaling of the parabola (i.e. the \f$a\f$ in \f$a^{-2} x^2\f$). A value equal
///    or smaller to 0 causes that dimension to not be processed. The boundary condition is ignored
///    for operators with this structuring element, and the output image is always a floating-point type.
///
/// -  `"line"`, `"fast line"`, `"periodic line"`, `"discrete line"`, `"interpolated line"`:
///    these are straight lines, using different implementations.
///    The size array corresponds to the size of the bounding box of the line, with signs indicating
///    the direction. Thus, if the size array is `{2,2}`, the line goes right and down two pixels,
///    meaning that the line is formed by two pixels at an angle of 45 degrees down. If the size array
///    is `{-2,2}`, then the line is again two pixels, but at an angle of 125 degrees. (Note that
///    in images, angles increase clockwise from the x-axis, as the y-axis is inverted). For a description
///    of the meaning of these various line implementations, see \ref line_morphology.
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
/// element images, you need to mirror it in one of the two function calls:
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
///
/// \section line_morphology Line morphology
///
/// There are various different ways of applying dilations, erosions, openings and closings with line
/// structuring elements. The `dip::StructuringElement` class accepts five different strings each
/// providing a different definition of the line structuring element. Further, there is also the
/// `dip::PathOpening` function, which provides path openings and closings. Here we describe the five
/// different line structuring elements implemented in *DIPlib*.
///
/// - `"line"`: This is an efficient implementation that yields the same results as the traditional line
///   structuring element (`"discrete line"`). It is implemented as a combination of `"periodic line"` and
///   `"discrete line"`, and is called *recursive line* in the literature (see Soille, 1996). If the line
///   parameters are such that the periodic line has a short period, this implementation saves a lot of time.
///   In this case, for a given line angle, the cost of the operation is independent of the length of the line.
///   If the line parameters are such that the periodic line has only one point, this is identical to
///   `"discrete line"`.
///
/// - `"fast line"`: This is a faster algorithm that applies a 1D operation along Bresenham lines, yielding
///   a non-translation-invariant result. The cost of this operation is always independent of the length of the line.
///
/// - `"periodic line"`: This is a line formed of only a subset of the pixels along the Bresenham line,
///   such that it can be computed as a 1D operation along Bresenham lines, but still yields a
///   translation-invariant result (Soille, 1996). It might not be very useful on its own, but when combined
///   with the `"discrete line"`, it provides a more efficient implementation of the traditional line structuring
///   element (see `"line"` above).
///
/// - `"discrete line"`: This is the traditional line structuring element, drawn using the Bresenham algorithm
///   and applied brute-force.
///
/// - `"interpolated line"`: This operation skews the image, using interpolation, such that the line operation
///   can be applied along an image axis; the result of the operation is then skewed back. The result is an
///   operation with a line that uses interpolation to read image intensities in between pixels.
///   This greatly improves the results in e.g. a granulometry when the input image is band limited
///   (Luengo Hendriks, 2005). However, the result of morphological operations is not band limited, and so the
///   second, reverse skew operation will lose some precision. Note that the result of morphological operations
///   with this SE do not strictly satisfy the corresponding properties (only by approximation) because of the
///   interpolated values. Setting the boundary condition to `"periodic"` allows the operation to occur completely
///   in place; other boundary conditions lead to a larger intermediate image, and thus will always require
///   additional, temporary storage.
///
/// For all these lines, if they are an even number of pixels in length, then the origin is placed at the result
/// of the integer division `length/2`. That is, on the pixel that comes just after the true middle of the line.
/// This means that the line `{8,3}` will have the origin on pixel number 4 (when starting counting at 0), as will
/// the line `{-8,-3}`. The difference between these two is that the latter starts on the bottom right and goes
/// left and up, whereas the former starts on the top left and goes right and down. Note that the drawn Bresenham
/// line might have a slightly different configuration also.
///
/// The SE `"line"` is different from the others in that these two lines will be normalized to the exact same line:
/// If the first size component is negative, all size components will be negated, turning `{-8,-3}` into `{8,3}`.
/// This makes it easier to decompose the SE into the two components. Do note that, because of this normalization,
/// there could be a 1 pixel shift for even-sized lines as compared to `"discrete line"` or `"fast line"`.
///
/// In general, a few quick experiments have shown that, depending on the angle and the direction of the line w.r.t.
/// the image storage order, `"discrete line"` can be much faster than `"line"` (or `"fast line"`) for shorter
/// lines (times were equal with around 50px length), or they can be much slower for even the shortest of lines.
/// Predicting which implementation of the line will be faster for a given situation is not trivial.
///
/// **Literature**
///  - P. Soille, E. J. Breen and R. Jones, "Recursive implementation of erosions and dilations along discrete lines
///    at arbitrary angles," IEEE Transactions on Pattern Analysis and Machine Intelligence 18(5):562-567, 1996.
///  - C.L. Luengo Hendriks and L.J. van Vliet, "Using line segments as structuring elements for sampling-invariant
///    measurements," IEEE Transactions on Pattern Analysis and Machine Intelligence 27(11):1826-1831, 2005.
class DIP_NO_EXPORT StructuringElement {
   public:
      enum class ShapeCode {
            RECTANGULAR,
            ELLIPTIC,
            DIAMOND,
            OCTAGONAL,
            LINE,
            FAST_LINE,
            PERIODIC_LINE,
            DISCRETE_LINE,
            INTERPOLATED_LINE,
            PARABOLIC,
            CUSTOM
      };

      /// \brief The default structuring element is a disk with a diameter of 7 pixels.
      StructuringElement() : shape_( ShapeCode::ELLIPTIC ), params_( { 7 } ) {}

      /// \brief A string implicitly converts to a structuring element, it is interpreted as a shape.
      StructuringElement( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }
      StructuringElement( char const* shape ) : params_( { 7 } ) {
         SetShape( shape );
      }

      /// \brief A `dip::FloatArray` implicitly converts to a structuring element, it is interpreted as the
      /// parameter of the SE for all dimensions. A second argument specifies the shape.
      StructuringElement( FloatArray const& params, String const& shape = S::ELLIPTIC ) : params_( params ) {
         SetShape( shape );
      }

      /// \brief A floating-point value implicitly converts to a structuring element, it is interpreted as the
      /// parameter of the SE along each dimension. A second argument specifies the shape.
      StructuringElement( dfloat param, String const& shape = S::ELLIPTIC ) : params_( FloatArray{ param } ) {
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

      /// \brief True if SE is mirrored.
      bool IsMirrored() const {
         return mirror_;
      }

      /// \brief Converts the Structuring element into a kernel
      // NOTE: When we go to SEs that are sequences of kernels, this function will change!
      DIP_EXPORT dip::Kernel Kernel() const;

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

      /// \brief Tests to see if the structuring element is a custom shape
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
         if( shape == S::ELLIPTIC ) {
            shape_ = ShapeCode::ELLIPTIC;
         } else if( shape == S::RECTANGULAR ) {
            shape_ = ShapeCode::RECTANGULAR;
         } else if( shape == S::DIAMOND ) {
            shape_ = ShapeCode::DIAMOND;
         } else if( shape == S::OCTAGONAL ) {
            shape_ = ShapeCode::OCTAGONAL;
         } else if( shape == S::LINE ) {
            shape_ = ShapeCode::LINE;
         } else if( shape == S::FAST_LINE ) {
            shape_ = ShapeCode::FAST_LINE;
         } else if( shape == S::PERIODIC_LINE ) {
            shape_ = ShapeCode::PERIODIC_LINE;
         } else if( shape == S::DISCRETE_LINE ) {
            shape_ = ShapeCode::DISCRETE_LINE;
         } else if( shape == S::INTERPOLATED_LINE ) {
            shape_ = ShapeCode::INTERPOLATED_LINE;
         } else if( shape == S::PARABOLIC ) {
            shape_ = ShapeCode::PARABOLIC;
         } else {
            DIP_THROW_INVALID_FLAG( shape );
         }
      }
};


//
// Basic operators
//

namespace detail {

enum class BasicMorphologyOperation {
      DILATION,
      EROSION,
      CLOSING,
      OPENING
};

DIP_EXPORT void BasicMorphology(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition,
      BasicMorphologyOperation operation
);

} // namespace detail

/// \brief Applies the dilation.
///
/// `se` defines the structuring element.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add min"`, but any value can be used.
/// For the rectangular, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds. This is equivalent to `"add min"`.
///
/// \see dip::Erosion, dip::Opening, dip::Closing, dip::RankFilter
inline void Dilation(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::DILATION );
}
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
/// For the rectangular, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds. This is equivalent to `"add max"`.
///
/// \see dip::Dilation, dip::Opening, dip::Closing, dip::RankFilter
inline void Erosion(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::EROSION );
}
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
/// For the rectangular, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds.
///
/// \see dip::Dilation, dip::Erosion, dip::Opening, dip::RankMinClosing
inline void Closing(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::CLOSING );
}
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
/// For the rectangular, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds.
///
/// \see dip::Dilation, dip::Erosion, dip::Closing, dip::RankMaxOpening
inline void Opening(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::OPENING );
}
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
      String const& edgeType = S::TEXTURE,
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
);
inline Image Tophat(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      String const& polarity = S::WHITE,
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
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalThreshold(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
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
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalGist(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
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
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalRange(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
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
   MorphologicalRange( in, out, se, S::BOTH, boundaryCondition );
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
      String const& edgeType = S::TEXTURE,
      String const& sign = S::UNSIGNED,
      StringArray const& boundaryCondition = {}
);
inline Image Lee(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      String const& sign = S::UNSIGNED,
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
      String const& mode = S::AVERAGE,
      StringArray const& boundaryCondition = {}
);
inline Image MorphologicalSmoothing(
      Image const& in,
      StructuringElement const& se = {},
      String const& mode = S::AVERAGE,
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
/// `dip::StructuringElement`.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
DIP_EXPORT void MultiScaleMorphologicalGradient(
      Image const& in,
      Image& out,
      dip::uint upperSize = 9,
      dip::uint lowerSize = 3,
      String const& filterShape = S::ELLIPTIC,
      StringArray const& boundaryCondition = {}
);
inline Image MultiScaleMorphologicalGradient(
      Image const& in,
      dip::uint upperSize = 9,
      dip::uint lowerSize = 3,
      String const& filterShape = S::ELLIPTIC,
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
// Rank-order--based filters
//


/// \brief Applies the rank-order filter.
///
/// `se` defines the structuring element. `rank` determines which of the sorted values within
/// the SE should be written to the output. A rank of 1 leads to an erosion, and a rank equal
/// to the number of pixels within the SE leads to a dilation. If `order` is `"decreasing"` (instead
/// of the default `"increasing"`), then `rank` is interpreted in the opposite direction, it
/// counts elements starting at the largest value. In this case, a rank of 1 is equal to a dilation.
///
/// Thus, a small non-zero rank with increasing order leads to an approximation to the dilation
/// that is less sensitive to noise, and a small non-zero rank with decreasing order leads to an
/// approximation of the erosion.
///
/// See also `dip::PercentileFilter`, which does the same thing but uses a percentile instead of
/// a rank as input argument.
///
/// `boundary` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value is the most meaningful one, but any value can be used. By default it is
/// `"add max"` if `rank` is lower than half of the pixels in the SE, or `"add min"` otherwise.
///
/// \see dip::Erosion, dip::Dilation, dip::RankMinClosing, dip::RankMaxOpening, dip::PercentileFilter
// Note that this function is defined in src/nonlinear/percentile.cpp
DIP_EXPORT void RankFilter(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      dip::uint rank = 2,
      String const& order = S::INCREASING,
      StringArray const& boundaryCondition = {}
);
inline Image RankFilter(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint rank = 2,
      String const& order = S::INCREASING,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   RankFilter( in, out, se, rank, order, boundaryCondition );
   return out;
}

/// \brief Applies the rank-min closing, a closing that is somewhat robust to noise.
///
/// `se` defines the structuring element. `rank` determines how many pixels in the SE
/// are ignored. That is, if the SE has `n` pixels, then a rank filter with rank equal
/// to `n - rank` is applied instead of a dilation.
///
/// This function uses the definition of Soille:
/// \f[
///    \phi_{B,\text{rank}} = \bigwedge_i \{ \phi_{B_i} | B_i \subseteq B, \text{card}(B_i) = n-\text{rank} \},
/// \f]
/// which is identical to
/// \f[
///    \phi_{B,\text{rank}} = I \vee \epsilon_{\check{B}} \xi_{B,n-\text{rank}}
/// \f]
///
/// `boundary` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default empty array causes the function to use `"add min"` with the rank filter,
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// **Literature**
///  - P. Soille, "Morphological Image Analysis", 2nd Edition, section 4.4.3. Springer, 2002.
DIP_EXPORT void RankMinClosing(
      Image const& in,
      Image& out,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
);
inline Image RankMinClosing(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   RankMinClosing( in, out, se, rank, boundaryCondition );
   return out;
}

/// \brief Applies the rank-max opening, an opening that is somewhat robust to noise.
///
/// `se` defines the structuring element. `rank` determines how many pixels in the SE
/// are ignored. That is, a rank filter with rank equal `rank + 1` is applied instead of an
/// erosion.
///
/// This function uses the definition of Soille (ref!):
/// \f[
///    \gamma_{B,\text{rank}} = \bigvee_i \{ \gamma_{B_i} | B_i \subseteq B, \text{card}(B_i) = n-\text{rank} \},
/// \f]
/// which is identical to
/// \f[
///    \gamma_{B,\text{rank}} = I \wedge \delta_{\check{B}} \xi_{B,\text{rank}+1}
/// \f]
///
/// `boundary` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default empty array causes the function to use `"add min"` with the dilation,
/// and `"add max"` with the rank filter, equivalent to ignoring what's outside the image.
///
/// **Literature**
///  - P. Soille, "Morphological Image Analysis", 2nd Edition, section 4.4.3. Springer, 2002.
DIP_EXPORT void RankMaxOpening(
      Image const& in,
      Image& out,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
);
inline Image RankMaxOpening(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   RankMaxOpening( in, out, se, rank, boundaryCondition );
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
/// to vertex-connected watershed lines (i.e. thinnest possible result). See \ref connectivity for
/// information on the connectivity parameter.
///
/// `flags` determines how the output is computed. There are three options:
/// - "labels" or "binary": returns either the labels used during processing, with the watershed
///   lines as background (value 0), or a binary image where the watershed lines are set and the
///   regions are not set. "binary" is the default.
/// - "low first" or "high first": determines the sort order of pixels. The default of "low first"
///   yields the normal watershed, where local minima are origin of the basins, and the watershed
///   lines run along the high ridges in the image. "high first" simply inverts the definition,
///   such that local maxima are at the centers of the basins, and the watershed lines run along
///   the low valleys.
/// - "fast" or "correct": determines which algorithm is used:
///   - "fast" is an algorithm that takes a few shortcuts, but usually manages to produce good results
///     any way. One shortcut leads to all border pixels being marked as watershed lines. It is possible
///     to extend the image by one pixel before processing to circumvent this. The other shortcut means
///     that plateaus are not handled correctly. A plateau is a region in the image where pixels have
///     exactly the same value. This is usually seen as watershed lines not running through the middle of
///     the plateaus, instead being shifted to one side. Adding a little bit of noise to the image, and
///     setting `maxDepth` to the range of the noise, usually improves the results in these cases
///     a little bit.
///   - "correct" is an algorithm that first finds the local minima through `dip::Minima` (or maxima if
///     "high first" is set), and then applies `dip::SeededWatershed`. This always produces correct results,
///     but is significantly slower.
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
/// Note that for the "fast" algorithm, `maxDepth` is always at least 0 (negative values will be ignored).
/// That is, two regions without a grey-value difference between them (they are on the same plateau) will
/// always be merged. This is necessary to prevent unexpected results (i.e. a plateau being split into
/// multiple regions). For the "correct" algorithm, any negative value of `maxDepth` will disable the
/// merging. But note that, due to the way that the region seeds are computed (`dip::Minima`), setting
/// `maxDepth` to 0 would lead to the exact same result.
///
/// Any pixel that is infinity will be part of the watershed lines, as is any pixel not within
/// `mask`.
DIP_EXPORT void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 0,
      dip::uint maxSize = 0,
      StringSet flags = {}
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
/// between them (i.e. they are on the same plateau) will be merged unless merging is disabled (see below).
/// Merged labels will be painted with the label of one of the originating seeds, and the other labels will
/// not be present in the output (only if `flags` contains "labels").
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads to
/// vertex-connected watershed lines (i.e. thinnest possible result). See \ref connectivity for information
/// on the connectivity parameter.
///
/// The region merging and the `flags` parameter work as described in `dip::Watershed`, with the following
/// additions:
/// - If `maxDepth` is negative, regions will never be merged, even if they have no grey-value difference
///   between them.
/// - The `flags` values "fast" or "correct" are not allowed.
/// - `flags` can contain the string "no gaps", which prevents the formation of watershed lines in between
///   the regions. That is, seeds are grown until they touch. This is typically useful only in combination
///   with "labels", since in a binary image there will be no distinction between initially separate regions.
///   Pixels that have an infinite value in `in`, or a zero value in `mask`, will still be excluded from the
///   region growing process.
/// - `flags` can contain the string "uphill only", and will limit the region growing to be exclusively
///   uphill (or downhill if "high first" is also given). This means that regions will grow to fill the
///   local catchment basin, but will not grow into neighboring catchment basins that have no seeds. This
///   flag will also disable any merging.
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
/// a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::WatershedMaxima, dip::Minima, dip::Maxima.
DIP_EXPORT void WatershedMinima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
);
inline Image WatershedMinima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
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
/// a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::WatershedMinima, dip::Maxima, dip::Minima.
DIP_EXPORT void WatershedMaxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
);
inline Image WatershedMaxima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
) {
   Image out;
   WatershedMaxima( in, mask, out, connectivity, maxDepth, maxSize, output );
   return out;
}

/// \brief Marks local minima.
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a higher value. If `output` is "binary", the result is a binary image where these
/// pixels and plateaus are set. If `output` is "labels", the result is a labeled image.
///
/// If `mask` is given, it restricts the area in the image that is searched.
///
/// For images that have large plateaus (regions of constant value) that are not local minima, this function can
/// be quite slow. For example, an image that is zero everywhere except for a small valley. For such an image
/// it is recommended to use the `mask` input, for example with the output of a threshold operation.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::Maxima, dip::WatershedMinima, dip::WatershedMaxima.
DIP_EXPORT void Minima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
);
inline Image Minima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 0,
      String const& output = S::BINARY
) {
   Image out;
   Minima( in, mask, out, connectivity, output );
   return out;
}

/// \brief Marks local maxima.
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a lower value. If `output` is "binary", the result is a binary image where these
/// pixels and plateaus are set. If `output` is "labels", the result is a labeled image.
///
/// If `mask` is given, it restricts the area in the image that is searched.
///
/// For images that have large plateaus (regions of constant value) that are not local maxima, this function can
/// be quite slow. For example, an image that is zero everywhere except for a small peak. For such an image
/// it is recommended to use the `mask` input, for example with the output of a threshold operation.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::Minima, dip::WatershedMaxima, dip::WatershedMinima.
DIP_EXPORT void Maxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
);
inline Image Maxima(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 0,
      String const& output = S::BINARY
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

/// \brief Reconstruction by dilation or erosion, also known as inf-reconstruction and sup-reconstruction
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
/// For binary images, `dip::BinaryPropagation` also defines the reconstruction by dilation, and allows limiting
/// the number of reconstruction steps, and supports alternating connectivity, which yields a more isotropic result
/// (note that alternating connectivity does not yield any advantages when reconstructing until idempotence, as this
/// function does).
///
/// **Literature**
///  - K. Robinson and P.F. Whelan: Efficient morphological reconstruction: a downhill filter, Pattern Recognition
///    Letters 25:1759-1767, 2004.
DIP_EXPORT void MorphologicalReconstruction(
      Image const& marker,
      Image const& in, // grey-value mask
      Image& out,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
);
inline Image MorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
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
      dip::uint connectivity = 0
) {
   Image tmp = Add( in, h, in.DataType() );
   MorphologicalReconstruction( tmp, in, out, connectivity, S::EROSION );
}
inline Image HMinima(
      Image const& in,
      dfloat h,
      dip::uint connectivity = 0
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
      dip::uint connectivity = 0
) {
   Image tmp = Subtract( in, h, in.DataType() );
   MorphologicalReconstruction( tmp, in, out, connectivity, S::DILATION );
}
inline Image HMaxima(
      Image const& in,
      dfloat h,
      dip::uint connectivity = 0
) {
   Image out;
   HMaxima( in, out, h, connectivity );
   return out;
}

/// \brief Computes the area opening or closing
///
/// The area opening removes all local maxima that have an area smaller than the given parameter `filterSize`,
/// and is equivalent to the supremum of openings with all possible connected flat structuring elements of that area.
/// The output has all maxima being connected components with a size of at least `filterSize`.
///
/// `mask` restricts the image regions used for the operation.
///
/// `connectivity` determines what a connected component is. See \ref connectivity for information on the
/// connectivity parameter.
///
/// When `polarity` is set to `"closing"`, the area closing is computed instead.
///
/// We use a union-find implementation similar to that described my Meijster and Wilkinson (2002), and is based on
/// the algorithm for our fast watershed (`"fast"` mode to `dip::Watershed`). For binary images, this function calls
/// `dip::BinaryAreaOpening` or `dip::BinaryAreaClosing`.
///
/// **Literature**
///  - L. Vincent, "Grayscale area openings and closings, their efficient implementation and applications",
///    Mathematical Morphology and Its Applications to Signal Processing, pp. 22-27, 1993.
///  - A. Meijster and M.H.F. Wilkinson, "A Comparison of Algorithms for Connected Set Openings and Closings",
///    IEEE Transactions on Pattern Analysis and Machine Intelligence 24(4):484-494, 2002.
///
/// \see dip::PathOpening, dip::DirectedPathOpening, dip::Opening, dip::Closing, dip::Maxima, dip::Minima, dip::SmallObjectsRemove
DIP_EXPORT void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      // TODO: we could use a boundary condition parameter here (keep or preserve smaller areas at the image edge)
      String const& polarity = S::OPENING
);
inline Image AreaOpening(
      Image const& in,
      Image const& mask,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      String const& polarity = S::OPENING
) {
   Image out;
   AreaOpening( in, mask, out, filterSize, connectivity, polarity );
   return out;
}

/// \brief Computes the area closing, calling `dip::AreaOpening` with `polarity="closing"`.
inline void AreaClosing(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0
) {
   AreaOpening( in, mask, out, filterSize, connectivity, S::CLOSING );
}
inline Image AreaClosing(
      Image const& in,
      Image const& mask,
      dip::uint filterSize,
      dip::uint connectivity = 0
) {
   Image out;
   AreaClosing( in, mask, out, filterSize, connectivity );
   return out;
}

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
      String const& polarity = S::OPENING,
      String const& mode = S::NORMAL
);
inline Image PathOpening(
      Image const& in,
      Image const& mask,
      dip::uint length = 7,
      String const& polarity = S::OPENING,
      String const& mode = S::NORMAL
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
/// **Literature**
///  - H. Heijmans, M. Buckley and H. Talbot, "Path Openings and Closings", Journal of Mathematical Imaging and Vision 22:107-119, 2005.
///  - H. Talbot, B. Appleton, "Efficient complete and incomplete path openings and closings", Image and Vision Computing 25:416-425, 2007.
///  - C.L. Luengo Hendriks, "Constrained and dimensionality-independent path openings", IEEE Transactions on Image Processing 19(6):1587–1595, 2010.
DIP_EXPORT void DirectedPathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntegerArray filterParam,
      String const& polarity = S::OPENING,
      String const& mode = S::NORMAL
);
inline Image DirectedPathOpening(
      Image const& in,
      Image const& mask,
      IntegerArray const& filterParam,
      String const& polarity = S::OPENING,
      String const& mode = S::NORMAL
) {
   Image out;
   DirectedPathOpening( in, mask, out, filterParam, polarity, mode );
   return out;
}

/// \brief Opening by reconstruction
///
/// Applies a structural erosion followed by a reconstruction by dilation.
///
/// See `dip::Erosion` and `dip::MorphologicalReconstruction` for a description of the parameters.
inline void OpeningByReconstruction(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   DIP_START_STACK_TRACE
      Erosion( in, out, se, boundaryCondition );
      MorphologicalReconstruction( out, in, out, connectivity, S::DILATION );
   DIP_END_STACK_TRACE

}
inline Image OpeningByReconstruction(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   OpeningByReconstruction( in, out, se, connectivity, boundaryCondition );
   return out;
}

/// \brief Closing by reconstruction
///
/// Applies a structural dilation followed by a reconstruction by erosion.
///
/// See `dip::Dilation` and `dip::MorphologicalReconstruction` for a description of the parameters.
inline void ClosingByReconstruction(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   DIP_START_STACK_TRACE
      Dilation( in, out, se, boundaryCondition );
      MorphologicalReconstruction( out, in, out, connectivity, S::EROSION );
   DIP_END_STACK_TRACE
}
inline Image ClosingByReconstruction(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   ClosingByReconstruction( in, out, se, connectivity, boundaryCondition );
   return out;
}



// TODO: functions to port:
/*
dip_UpperEnvelope (dip_morphology.h)
dip_UpperSkeleton2D (dip_binary.h)
*/

// TODO: alternating sequential open-close filter (3 versions: with structural opening, opening by reconstruction, and area opening)
// TODO: hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees (360 degrees means no rotations).
// TODO: levelling
// TODO: granulometries (isotropic and path opening)

/// \}

} // namespace dip

#endif // DIP_MORPHOLOGY_H
