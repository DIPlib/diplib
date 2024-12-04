/*
 * (c)2017-2021, Cris Luengo.
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

#include <utility>

#include "diplib.h"
#include "diplib/random.h"


/// \file
/// \brief Mathematical morphology operators and filters.
/// See \ref morphology.


namespace dip {


// Forward declaration, from diplib/kernel.h
class DIP_NO_EXPORT Kernel;


/// \group morphology Morphological filtering
/// \ingroup filtering
/// \brief Morphological filters for smoothing, sharpening, detection and more.
/// \addtogroup

/// \brief Represents the shape and size of a structuring element.
///
/// Many functions in the Mathematical Morphology module require a structuring element definition.
/// There are two ways to define a structuring element: the user can specify the shape name and the size
/// of a structuring element, and the user can pass an image containing the structuring element.
///
/// Objects of type \ref dip::Image, \ref dip::FloatArray and \ref dip::String implicitly convert to
/// a \ref dip::StructuringElement, so it should be convenient to use these various representations in your
/// code.
///
/// To define a structuring element by shape and size, pass a string defining the shape, and a
/// floating-point array with the size along each dimension.
/// These are the valid shape strings, and the corresponding meaning of the size array:
///
/// - `"elliptic"`: the isotropic flat structuring element, and the default shape. The size array gives the
///   diameter along each dimension. It is always symmetric. That is, the origin is centered on a pixel.
///   The pixels included in the disk or ellipse are those less than half of the diameter away from the origin.
///   It is implemented through a relatively efficient algorithm that scales with the diameter, not the number
///   of pixels covered. Rectangular, elliptic and octagonal structuring elements are much faster, especially
///   for larger sizes.
///   Any size array element that is smaller than 2 causes that dimension to not be processed.
///
/// - `"rectangular"`: the unit circle in a chessboard metric. The size array gives the diameter (or rather the
///   side lengths). The rectangle can have even sizes, in which case it is not symmetric around the origin pixel.
///   This structuring element is implemented with a one-dimensional pass along each image dimension.
///   This decomposition makes this a highly efficient structuring element, with computation times that are
///   independent of the size.
///   Any size array element that is smaller or equal to 1 causes that dimension to not be processed.
///
/// - `"diamond"`: the unit circles in a city-block metric. The size array gives the diameter (the extent along
///   each image axis). Like the ellipse, it is always symmetric. That is, the origin is centered on a pixel.
///   If all sizes are equal, then this structuring element is decomposed into a unit diamond and 2 diagonal
///   lines, for a 2D diamond. In this case, computation times that are independent of the size, like
///   for the rectangle. However, for smaller diamonds, the decomposition is different: a unit diamond is applied
///   repeatedly; this yielding faster computation times. If any size is different from the others, or if the
///   diamond has more than two dimensions, then the same algorithm as for the elliptic structuring element is used.
///   Any size array element that is smaller than 2 causes that dimension to not be processed.
///
/// - `"octagonal"`: a fast approximation to the ellipse. Octagons (in 2D) are decomposed into a rectangle
///   and a diamond, each one implemented as described above. This makes the octagonal structuring element
///   more expensive than either the diamond or rectangle, but still computable in constant time independent
///   of the diameter. We generalize this structuring element to arbitrary number of dimensions simply by
///   applying those two smaller structuring elements in succession. In 3D this leads to a rhombicuboctahedron.
///   Any size array element that is smaller than 2 causes that dimension to not be processed.
///
/// - `"parabolic"`: the parabolic structuring element is the morphological equivalent to the Gaussian
///   kernel in linear filtering. It is separable and perfectly isotropic. The size array corresponds
///   to the scaling of the parabola (i.e. the $a$ in $a^{-2} x^2$). A value equal
///   or smaller to 0 causes that dimension to not be processed. The boundary condition is ignored
///   for operators with this structuring element, and the output image is always a floating-point type.
///
/// - `"line"`, `"fast line"`, `"periodic line"`, `"discrete line"`, `"interpolated line"`:
///   these are straight lines, using different implementations.
///   The size array corresponds to the size of the bounding box of the line, with signs indicating
///   the direction. Thus, if the size array is `{2,2}`, the line goes right and down two pixels,
///   meaning that the line is formed by two pixels at an angle of 45 degrees down. If the size array
///   is `{-2,2}`, then the line is again two pixels, but at an angle of 135 degrees. (Note that
///   in images, angles increase clockwise from the x-axis, as the y-axis is inverted). For a description
///   of the meaning of these various line implementations, see \ref line_morphology.
///
/// To define a structuring element through an image, provide either a binary or grey-value image.
/// If the image is binary, the set pixels form the structuring element. If the image is a grey-value
/// image, those grey values are directly used as structuring element values. Set pixels to negative infinity
/// to exclude them from the structuring element (the result would be the same by setting them to
/// a value lower than the range of the input image, but the algorithm should be more efficient if
/// those pixels are excluded).
///
/// Note that the image is directly used as neighborhood (i.e. no mirroring is applied).
/// That is, \ref dip::Dilation and \ref dip::Erosion will use the same neighborhood. Their composition only
/// leads to an opening or a closing if the structuring element is symmetric. For non-symmetric structuring
/// element images, you need to mirror it in one of the two function calls:
///
/// ```cpp
/// dip::Image se = ...;
/// dip::Image out = dip::Erosion( in, se );
/// out = dip::Dilation( out, se.Mirror() );
/// ```
///
/// (Do note that, in the code above, `se` itself is modified! use `se.QuickCopy().Mirror()` to prevent
/// that.)
///
/// As elsewhere, the origin of the structuring element is in the middle of the image, on the pixel to
/// the right of the center in case of an even-sized image.
///
/// \see dip::Kernel, dip::PixelTable
///
/// \section line_morphology Line morphology
///
/// There are various different ways of applying dilations, erosions, openings and closings with line
/// structuring elements. The \ref dip::StructuringElement class accepts five different strings each
/// providing a different definition of the line structuring element. Further, there is also the
/// \ref dip::PathOpening function, which provides path openings and closings. Here we describe the five
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
/// A few quick experiments have shown that, depending on the angle and the direction of the line w.r.t.
/// the image storage order, `"discrete line"` can be much faster than `"line"` (or `"fast line"`) for shorter
/// lines (times were equal with around 50px length), or they can be much slower for even the shortest of lines.
/// Predicting which implementation of the line will be faster for a given situation is not trivial.
///
/// !!! literature
///     - P. Soille, E. J. Breen and R. Jones, "Recursive implementation of erosions and dilations along discrete lines
///       at arbitrary angles", IEEE Transactions on Pattern Analysis and Machine Intelligence 18(5):562-567, 1996.
///     - C.L. Luengo Hendriks and L.J. van Vliet, "Using line segments as structuring elements for sampling-invariant
///       measurements", IEEE Transactions on Pattern Analysis and Machine Intelligence 27(11):1826-1831, 2005.
class DIP_NO_EXPORT StructuringElement {
   public:

      /// \brief Possible shapes of a structuring element
      enum class ShapeCode : uint8 {
            RECTANGULAR,       ///< Corresponding to string `"rectangular"`.
            ELLIPTIC,          ///< Corresponding to string `"elliptic"`.
            DIAMOND,           ///< Corresponding to string `"diamond"`.
            OCTAGONAL,         ///< Corresponding to string `"octagonal"`.
            LINE,              ///< Corresponding to string `"line"`.
            FAST_LINE,         ///< Corresponding to string `"fast line"`.
            PERIODIC_LINE,     ///< Corresponding to string `"periodic line"`.
            DISCRETE_LINE,     ///< Corresponding to string `"discrete line"`.
            INTERPOLATED_LINE, ///< Corresponding to string `"interpolated line"`.
            PARABOLIC,         ///< Corresponding to string `"parabolic"`.
            CUSTOM             ///< Defined through an image.
      };

      /// \brief The default structuring element is a disk with a diameter of 7 pixels.
      StructuringElement() : params_( { 7 } ) {}

      /// \brief A string implicitly converts to a structuring element, it is interpreted as a shape.
      StructuringElement( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }
      StructuringElement( char const* shape ) : params_( { 7 } ) {
         SetShape( shape );
      }

      /// \brief A \ref dip::FloatArray implicitly converts to a structuring element, it is interpreted as the
      /// parameter of the SE for all dimensions. A second argument specifies the shape.
      StructuringElement( FloatArray params, String const& shape = S::ELLIPTIC ) : params_( std::move( params )) {
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
      ShapeCode shape_ = ShapeCode::ELLIPTIC;
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

enum class BasicMorphologyOperation : uint8 {
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

/// \brief Applies the dilation with a standard or custom structuring element.
///
/// If the structuring element $S$ is a set (i.e. a binary image, or a footprint), the dilation of image $f$
/// is defined as $(\delta f)(x) = \bigvee\limits_{z \in S} f(x+z)$ (the supremum or maximum over the pixels
/// covered by the structuring element).
/// For gray-scale structuring elements, it is defined as $(\delta f)(x) = \bigvee\limits_{z} f(x+z) + S(z)$.
///
/// `se` defines the structuring element, see \ref dip::StructuringElement for options and details.
///
/// `boundaryCondition` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// The default value, and most meaningful one, is `"add min"`, but any value can be used.
/// For the rectangular, diamond, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds. This is equivalent to `"add min"`.
///
/// `in` must be a scalar image, and not complex-valued. In particular, `in` can be binary; this function
/// is more efficient than \ref dip::BinaryDilation.
///
/// \see dip::Erosion, dip::Opening, dip::Closing, dip::RankFilter, dip::IsotropicDilation
inline void Dilation(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::DILATION );
}
DIP_NODISCARD inline Image Dilation(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Dilation( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the erosion with a standard or custom structuring element.
///
/// If the structuring element $S$ is a set (i.e. a binary image, or a footprint), the erosion of image $f$
/// is defined as $(\epsilon f)(x) = \bigwedge\limits_{z \in S} f(x+z)$ (the infimum or minimum over the pixels
/// covered by the structuring element).
/// For gray-scale structuring elements, it is defined as $(\epsilon f)(x) = \bigwedge\limits_{z} f(x+z) - S(z)$.
///
/// `se` defines the structuring element, see \ref dip::StructuringElement for options and details.
///
/// `boundaryCondition` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// The default value, and most meaningful one, is `"add max"`, but any value can be used.
/// For the rectangular, diamond, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds. This is equivalent to `"add max"`.
///
/// `in` must be a scalar image, and not complex-valued. In particular, `in` can be binary; this function
/// is more efficient than \ref dip::BinaryErosion.
///
/// \see dip::Dilation, dip::Opening, dip::Closing, dip::RankFilter, dip::IsotropicErosion
inline void Erosion(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::EROSION );
}
DIP_NODISCARD inline Image Erosion(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Erosion( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the closing with a standard or custom structuring element.
///
/// The closing is defined as a dilation followed by its complementary erosion (i.e. with the mirrored structuring element).
///
/// `se` defines the structuring element, see \ref dip::StructuringElement for options and details.
///
/// `boundaryCondition` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// Meaningful values for the closing are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
/// For the rectangular, diamond, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds.
///
/// `in` must be a scalar image, and not complex-valued. In particular, `in` can be binary; this function
/// is more efficient than \ref dip::BinaryClosing.
///
/// \see dip::Dilation, dip::Erosion, dip::Opening, dip::RankMinClosing, dip::IsotropicClosing
inline void Closing(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::CLOSING );
}
DIP_NODISCARD inline Image Closing(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Closing( in, out, se, boundaryCondition );
   return out;
}

/// \brief Applies the opening with a standard or custom structuring element.
///
/// The opening is defined as a erosion followed by its complementary dilation (i.e. with the mirrored structuring element).
///
/// `se` defines the structuring element, see \ref dip::StructuringElement for options and details.
///
/// `boundaryCondition` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// Meaningful values for the opening are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
/// For the rectangular, diamond, fast line and periodic line structuring elements, no boundary condition
/// causes the filter to not read outside the image bounds.
///
/// `in` must be a scalar image, and not complex-valued. In particular, `in` can be binary; this function
/// is more efficient than \ref dip::BinaryOpening.
///
/// \see dip::Dilation, dip::Erosion, dip::Closing, dip::RankMaxOpening, dip::IsotropicOpening
inline void Opening(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   detail::BasicMorphology( in, out, se, boundaryCondition, detail::BasicMorphologyOperation::OPENING );
}
DIP_NODISCARD inline Image Opening(
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
///
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
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void Tophat(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image Tophat(
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
///
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalThreshold(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalThreshold(
      Image const& in,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalThreshold( in, out, se, edgeType, boundaryCondition );
   return out;
}

/// \brief Morphological gist operator and its variants
///
/// Similar to the top-hat operator, it computes the difference between the average of
/// two complementary morphological operators and the original image.
///
/// The flags `edgeType` defines which operation is applied:
///
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalGist(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalGist(
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
///
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalRange(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalRange(
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
/// This function is implemented by a call to \ref dip::MorphologicalRange with `edgeType` set to `"both"`.
inline void MorphologicalGradientMagnitude(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   MorphologicalRange( in, out, se, S::BOTH, boundaryCondition );
}
DIP_NODISCARD inline Image MorphologicalGradientMagnitude(
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
///
/// - `"texture"`: response is limited to edges in texture (i.e. scales smaller than the structuring element).
/// - `"object"`: response is limited to object edges (i.e. scales larger than the structuring element).
/// - `"both"` or `"dynamic"`: all edges produce equal response.
///
/// If `sign` is `"unsigned"`, `Lee` computes the absolute edge strength. `sign` can also be `"signed"`
/// to compute the signed edge strength.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void Lee(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& edgeType = S::TEXTURE,
      String const& sign = S::UNSIGNED,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image Lee(
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
/// Implements a morphological smoothing based on the sequence of an opening and a closing. Their order
/// can be chosen through the `polarity` parameter.
///
/// `polarity` can be one of:
///
/// - `"open-close"`: applies the opening first, then the closing.
/// - `"close-open"`: applies the closing first, then the opening.
/// - `"average"`: computes the average of the result of the first two modes.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalSmoothing(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      String const& polarity = S::AVERAGE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalSmoothing(
      Image const& in,
      StructuringElement const& se = {},
      String const& polarity = S::AVERAGE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalSmoothing( in, out, se, polarity, boundaryCondition );
   return out;
}

/// \brief A morphological sharpening filter
///
/// Implements a morphological sharpening based on selecting per pixel the result of the dilation or erosion,
/// whichever is closest to the input image.
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalSharpening(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalSharpening(
      Image const& in,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MorphologicalSharpening( in, out, se, boundaryCondition );
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
/// \ref dip::StructuringElement.
///
/// `boundaryCondition` determines the boundary conditions. See \ref dip::BoundaryCondition.
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
DIP_NODISCARD inline Image MultiScaleMorphologicalGradient(
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
///
/// ```cpp
/// out = ( Dilation( in ) + Erosion( in ) ) / 2 - in;
/// ```
///
/// `se` defines the structuring element, and `boundaryCondition` the boundary conditions.
/// See \ref dip::Dilation for a description of these parameters.
DIP_EXPORT void MorphologicalLaplace(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image MorphologicalLaplace(
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
/// See also \ref dip::PercentileFilter, which does the same thing but uses a percentile instead of
/// a rank as input argument.
///
/// `boundary` determines the boundary conditions. See \ref dip::BoundaryCondition.
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
DIP_NODISCARD inline Image RankFilter(
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
///
/// $$ \phi_{B,\text{rank}} = \bigwedge_i \{ \phi_{B_i} | B_i \subseteq B, \text{card}(B_i) = n-\text{rank} \} \; , $$
///
/// which is identical to
///
/// $$ \phi_{B,\text{rank}} = I \vee \epsilon_{\check{B}} \xi_{B,n-\text{rank}} \; . $$
///
/// `boundary` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// The default empty array causes the function to use `"add min"` with the rank filter,
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// !!! literature
///     - P. Soille, "Morphological Image Analysis", 2^nd^ Edition, section 4.4.3. Springer, 2002.
DIP_EXPORT void RankMinClosing(
      Image const& in,
      Image& out,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image RankMinClosing(
      Image const& in,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   RankMinClosing( in, out, std::move( se ), rank, boundaryCondition );
   return out;
}

/// \brief Applies the rank-max opening, an opening that is somewhat robust to noise.
///
/// `se` defines the structuring element. `rank` determines how many pixels in the SE
/// are ignored. That is, a rank filter with rank equal `rank + 1` is applied instead of an
/// erosion.
///
/// This function uses the definition of Soille (ref!):
///
/// $$ \gamma_{B,\text{rank}} = \bigvee_i \{ \gamma_{B_i} | B_i \subseteq B, \text{card}(B_i) = n-\text{rank} \} \; , $$
///
/// which is identical to
///
/// $$ \gamma_{B,\text{rank}} = I \wedge \delta_{\check{B}} \xi_{B,\text{rank}+1} \; . $$
///
/// `boundary` determines the boundary conditions. See \ref dip::BoundaryCondition.
/// The default empty array causes the function to use `"add min"` with the dilation,
/// and `"add max"` with the rank filter, equivalent to ignoring what's outside the image.
///
/// !!! literature
///     - P. Soille, "Morphological Image Analysis", 2^nd^ Edition, section 4.4.3. Springer, 2002.
DIP_EXPORT void RankMaxOpening(
      Image const& in,
      Image& out,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image RankMaxOpening(
      Image const& in,
      StructuringElement se = {},
      dip::uint rank = 2,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   RankMaxOpening( in, out, std::move( se ), rank, boundaryCondition );
   return out;
}


//
// Priority-queue--based algorithms
//


/// \brief Computes the watershed of `in` within `mask`, with on-line merging of regions.
///
/// \ingroup segmentation
///
/// The watershed is a segmentation algorithm that divides the image according to its grey-value
/// ridges.
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads
/// to vertex-connected watershed lines (i.e. thinnest possible result). See \ref connectivity for
/// information on the connectivity parameter.
///
/// `flags` determines how the output is computed. There are three options:
///
/// - `"labels"` or `"binary"`: returns either the labels used during processing, with the watershed
///   lines as background (value 0), or a binary image where the watershed lines are set and the
///   regions are not set. `"binary"` is the default.
///
/// - `"low first"` or `"high first"`: determines the sort order of pixels. The default of `"low first"`
///   yields the normal watershed, where local minima are origin of the basins, and the watershed
///   lines run along the high ridges in the image. `"high first"` simply inverts the definition,
///   such that local maxima are at the centers of the basins, and the watershed lines run along
///   the low valleys.
///
/// - `"fast"` or `"correct"`: determines which algorithm is used:
///     - `"fast"` (the default) is an algorithm that takes a few shortcuts, but usually manages to produce good results
///       any way. One shortcut leads to all border pixels being marked as watershed lines. It is possible
///       to extend the image by one pixel before processing to circumvent this. The other shortcut means
///       that plateaus are not handled correctly. A plateau is a region in the image where pixels have
///       exactly the same value. This is usually seen as watershed lines not running through the middle of
///       the plateaus, instead being shifted to one side. Adding a little bit of noise to the image, and
///       setting `maxDepth` to the range of the noise, usually improves the results in these cases
///       a little bit.
///
///     - `"correct"` is an algorithm that first finds the local minima through \ref dip::Minima (or maxima if
///       `"high first"` is set), and then applies \ref dip::SeededWatershed. This always produces correct results,
///       but is significantly slower.
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
/// Note that for the `"fast"` algorithm, `maxDepth` is always at least 0 (negative values will be ignored).
/// That is, two regions without a grey-value difference between them (they are on the same plateau) will
/// always be merged. This is necessary to prevent unexpected results (i.e. a plateau being split into
/// multiple regions). For the `"correct"` algorithm, any negative value of `maxDepth` will disable the
/// merging. But note that, due to the way that the region seeds are computed (\ref dip::Minima), setting
/// `maxDepth` to 0 would lead to the exact same result.
///
/// Any pixel that is infinity will be part of the watershed lines, as is any pixel not within
/// `mask`.
///
/// \see dip::SeededWatershed
DIP_EXPORT void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      StringSet flags = {}
);
DIP_NODISCARD inline Image Watershed(
      Image const& in,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      StringSet flags = {}
) {
   Image out;
   Watershed( in, mask, out, connectivity, maxDepth, maxSize, std::move( flags ));
   return out;
}

/// \brief Computes the watershed of `in` within `mask`, starting at `seeds`, with on-line merging of regions.
///
/// \ingroup segmentation
///
/// `seeds` is a binary or labeled image (if binary, it is labeled using `connectivity`). These labels are
/// iteratively expanded in the watershed order (i.e. pixels that have a low value in `in` go first) until
/// they meet. Pixels where two regions meet are marked as the watershed lines. `seeds` is commonly used
/// to direct the segmentation, and merging is consequently not necessary. However, this algorithm does
/// include on-line merging. Note that two labeled regions in `seeds` that do not have a grey-value ridge
/// between them (i.e. they are on the same plateau) will be merged unless merging is disabled (see below).
/// Merged labels will be painted with the label of one of the originating seeds, and the other labels will
/// not be present in the output (only if `flags` contains `"labels"`).
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads to
/// vertex-connected watershed lines (i.e. thinnest possible result). See \ref connectivity for information
/// on the connectivity parameter.
///
/// The region merging and the `flags` parameter work as described in \ref dip::Watershed, with the following
/// additions:
///
/// - If `maxDepth` is negative, regions will never be merged, even if they have no grey-value difference
///   between them.
/// - The `flags` values `"fast"` or `"correct"` are not allowed.
/// - `flags` can contain the string `"no gaps"`, which prevents the formation of watershed lines in between
///   the regions. That is, seeds are grown until they touch. This flag implies the flag `"labels"`, since in
///   a binary image there would be no distinction between initially separate regions.
///   Pixels that have an infinite value in `in`, or a zero value in `mask`, will still be excluded from the
///   region growing process.
/// - `flags` can contain the string `"uphill only"`, which will limit the region growing to be exclusively
///   uphill (or downhill if `"high first"` is also given). This means that regions will grow to fill the
///   local catchment basin, but will not grow into neighboring catchment basins that have no seeds. This
///   flag will also disable any merging.
///
/// \see dip::Watershed, dip::CompactWatershed, dip::GrowRegions, dip::GrowRegionsWeighted
DIP_EXPORT void SeededWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      StringSet const& flags = {}
);
DIP_NODISCARD inline Image SeededWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      StringSet const& flags = {}
) {
   Image out;
   SeededWatershed( in, seeds, mask, out, connectivity, maxDepth, maxSize, flags );
   return out;
}

/// \brief Computes the compact watershed of `in` within `mask`, starting at `seeds`.
///
/// \ingroup segmentation
///
/// `seeds` is a binary or labeled image (if binary, it is labeled using `connectivity`). These labels are
/// iteratively expanded in the watershed order (i.e. pixels that have a low value in `in` go first), modified
/// with a compactness term, until they meet. Pixels where two regions meet are marked as the watershed lines.
///
/// The compactness term modifies the normal watershed order by taking into account the distance to the originating
/// seed. This distance, multiplied by `compactness`, is added to the grey value when determining the processing
/// order. A `compactness` of 0 leads to the normal seeded watershed, and a very large value for `compactness`
/// leads to disregarding the pixel values in `in`, thereby creating a Voronoi diagram.
///
/// `connectivity` determines which pixels are considered neighbors; the default value of 1 leads to
/// vertex-connected watershed lines (i.e. thinnest possible result). See \ref connectivity for information
/// on the connectivity parameter.
///
/// The `flags` parameter work as described in \ref dip::SeededWatershed, except that `"uphill only"` is not supported.
///
/// \see dip::SeededWatershed, dip::Watershed, dip::GrowRegions, dip::GrowRegionsWeighted
///
/// !!! literature
///     - P. Neubert and P. Protzel, "Compact Watershed and Preemptive SLIC: On improving trade-offs of superpixel segmentation algorithms",
///       22^nd^ International Conference on Pattern Recognition, Stockholm, 2014, pp. 996-1001.
DIP_EXPORT void CompactWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat compactness = 1.0,
      StringSet const& flags = {}
);
DIP_NODISCARD inline Image CompactWatershed(
      Image const& in,
      Image const& seeds,
      Image const& mask = {},
      dip::uint connectivity = 1,
      dfloat compactness = 1.0,
      StringSet const& flags = {}
) {
   Image out;
   CompactWatershed( in, seeds, mask, out, connectivity, compactness, flags );
   return out;
}

/// \brief Computes the stochastic watershed of `in`.
///
/// \ingroup segmentation
///
/// The stochastic watershed is computed by applying a watershed with randomly placed seeds `nIterations` times,
/// and adding the results. The output is an image where each pixel's value is the likelihood that it belongs to
/// an edge in the image, the values are in the range [0,`nIterations`]. The input image `in` should contain high
/// grey values at the edges of the regions to be segmented. Thresholding `out` at an appropriate value will yield
/// the relevant edges in the image. Alternatively, apply \ref dip::Watershed to the result, with `maxDepth` set to
/// the appropriate threshold value.
///
/// The number of seeds used is given by `nSeeds`. Actually seeds are chosen with a density of
/// `nSeeds / in.NumberOfPixels()`, the random process causes the actual number of seeds to differ between
/// runs. Seeds are placed either through a Poisson point process (`seeds` is `"poisson"`) or a randomly
/// translated and rotated grid (`seeds` is `"rectangular"` (any number of dimensions), `"hexagonal"` (2D only),
/// or `"bcc"` or `"fcc"` (3D only)). The output contains counts, in the range [0,`nIterations`].
///
/// If `seeds` is `"exact"`, or if `nIterations` is 0, then the exact probabilities are computed
/// (Malmberg and Luengo, 2014). The output contains probabilities, in the range [0,1]. Note that this algorithm
/// requires O(n^2^) space, and is not suitable for very large images.
///
/// The stochastic watershed expects the image to contain roughly equally-sized regions. `nSeeds` should be
/// approximately equal to the number of expected regions. If there is a strong difference in region sizes, larger
/// regions will be split into smaller ones.
///
/// If the image contains regions with different sizes, it is recommended to set `noise` to a value that is larger
/// than the variation within regions, but smaller than the height of the barrier between regions. Uniform noise
/// will be added to the input image for every iteration of the process, causing non-significant edges to be
/// strongly suppressed (Bernander et al., 2013). In the case of the exact stochastic watershed, the
/// operation is applied three times with random noise added to the input, and the geometric mean of the results
/// is returned (Selig et al., 2015).
///
/// `in` must be real-valued and scalar. `out` will be of a suitable unsigned integer type (depending on the number
/// of iterations, but typically \ref dip::DT_UINT8), or of type \ref dip::DT_SFLOAT if the exact stochastic watershed
/// is computed.
///
/// !!! literature
///     - J. Angulo and D. Jeulin, "Stochastic watershed segmentation", Proceedings of the 8th International Symposium on
///       Mathematical Morphology, Instituto Nacional de Pesquisas Espaciais (INPE), São José dos Campos, pp. 265–276, 2007.
///     - K.B. Bernander, K. Gustavsson, B. Selig, I.-M. Sintorn, and C.L. Luengo Hendriks, "Improving the stochastic watershed",
///       Pattern Recognition Letters 34:993-1000, 2013.
///     - F. Malmberg and C.L. Luengo Hendriks, "An efficient algorithm for exact evaluation of stochastic watersheds",
///       Pattern Recognition Letters 47:80-84, 2014.
///     - B. Selig, F, Malmberg and C.L. Luengo Hendriks, "Fast evaluation of the robust stochastic watershed",
///       Proceedings of ISMM 2015, LNCS 9082:705-716, 2015.
DIP_EXPORT void StochasticWatershed(
      Image const& in,
      Image& out,
      Random& random,
      dip::uint nSeeds = 100,
      dip::uint nIterations = 50,
      dfloat noise = 0,
      String const& seeds = S::HEXAGONAL
);
DIP_NODISCARD inline Image StochasticWatershed(
      Image const& in,
      Random& random,
      dip::uint nSeeds = 100,
      dip::uint nIterations = 50,
      dfloat noise = 0,
      String const& seeds = S::HEXAGONAL
) {
   Image out;
   StochasticWatershed( in, out, random, nSeeds, nIterations, noise, seeds );
   return out;
}
/// \brief Like above, using a default-initialized \ref dip::Random object.
///
/// \ingroup segmentation
inline void StochasticWatershed(
      Image const& in,
      Image& out,
      dip::uint nSeeds = 100,
      dip::uint nIterations = 50,
      dfloat noise = 0,
      String const& seeds = S::HEXAGONAL
) {
   Random random;
   StochasticWatershed( in, out, random, nSeeds, nIterations, noise, seeds );
}
DIP_NODISCARD inline Image StochasticWatershed(
      Image const& in,
      dip::uint nSeeds = 100,
      dip::uint nIterations = 50,
      dfloat noise = 0,
      String const& seeds = S::HEXAGONAL
) {
   Image out;
   StochasticWatershed( in, out, nSeeds, nIterations, noise, seeds );
   return out;
}

/// \brief Marks significant local minima.
///
/// \ingroup segmentation
///
/// This algorithm works exactly like \ref dip::Watershed with the `"fast"` flag set. All pixels with a value
/// equal to the lowest value within each watershed basin form a local minimum. Note that they can form
/// disconnected regions, use the `"labels"` flag to recognize such disconnected regions as a single local
/// minimum. See \ref dip::Watershed for a description of all the parameters.
///
/// `output` can be `"binary"` or `"labels"`, and determines whether the algorithm outputs a binary image or
/// a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::WatershedMaxima, dip::Minima, dip::Maxima
DIP_EXPORT void WatershedMinima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
);
DIP_NODISCARD inline Image WatershedMinima(
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
/// \ingroup segmentation
///
/// This algorithm works exactly like \ref dip::Watershed with the `"fast"` flag set. All pixels with a value
/// equal to the highest value within each watershed basin form a local maximum. Note that they can form
/// disconnected regions, use the `"labels"` flag to recognize such disconnected regions as a single local
/// maximum. See \ref dip::Watershed for a description of all the parameters.
///
/// `output` can be `"binary"` or `"labels"`, and determines whether the algorithm outputs a binary image or
/// a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::WatershedMinima, dip::Maxima, dip::Minima
DIP_EXPORT void WatershedMaxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = S::BINARY
);
DIP_NODISCARD inline Image WatershedMaxima(
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
/// \ingroup segmentation
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a higher value. If `output` is `"binary"`, the result is a binary image where these
/// pixels and plateaus are set. If `output` is `"labels"`, the result is a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::Maxima, dip::WatershedMinima, dip::WatershedMaxima
DIP_EXPORT void Minima(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
);
DIP_NODISCARD inline Image Minima(
      Image const& in,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
) {
   Image out;
   Minima( in, out, connectivity, output );
   return out;
}

/// \brief Marks local maxima.
///
/// \ingroup segmentation
///
/// This algorithm finds single pixels or plateaus (connected groups of pixels with identical value) that are
/// surrounded by pixels with a lower value. If `output` is `"binary"`, the result is a binary image where these
/// pixels and plateaus are set. If `output` is `"labels"`, the result is a labeled image.
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// \see dip::Minima, dip::WatershedMaxima, dip::WatershedMinima
DIP_EXPORT void Maxima(
      Image const& in,
      Image& out,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
);
DIP_NODISCARD inline Image Maxima(
      Image const& in,
      dip::uint connectivity = 0,
      String const& output = S::BINARY
) {
   Image out;
   Maxima( in, out, connectivity, output );
   return out;
}


/// \brief Grey-value skeleton (2D only).
///
/// This algorithm finds ridges in the image by, starting at the lowest values, setting values to the minimum possible
/// value for the given data type if that doesn't change the topology of the higher-valued pixels.
/// It uses Hilditch conditions to preserve topology. The unmodified pixels are the grey-value equivalent to a binary
/// skeleton. Note that the minimum possible value is minus infinity for floating-point types.
///
/// The `mask` image optionally restricts the region of the image processed. Pixels not selected by the mask will
/// retain their original value.
///
/// The `endPixelCondition` parameter determines what is considered an "end pixel" in the skeleton, and thus affects
/// how many branches are generated. It is one of the following strings:
///
/// - `"natural"`: "natural" end pixel condition of this algorithm.
/// - `"one neighbor"`: Keep endpoint if it has one neighbor.
/// - `"two neighbors"`: Keep endpoint if it has two neighbors.
/// - `"three neighbors"`: Keep endpoint if it has three neighbors.
///
/// To generate skeletons without end pixels (the equivalent of `"loose ends away"` in \ref dip::EuclideanSkeleton),
/// use \ref dip::Watershed instead.
///
/// `in` must be a real-valued, scalar image. `out` will have the same type.
///
/// !!! attention
///     Pixels in a 1-pixel border around the edge are not processed, and set to the non-skeleton value.
///     If this is an issue, consider adding one pixel on each side of your image.
DIP_EXPORT void UpperSkeleton2D(
      Image const& in,
      Image const& mask,
      Image& out,
      String const& endPixelCondition = S::NATURAL
);
DIP_NODISCARD inline Image UpperSkeleton2D(
      Image const& in,
      Image const& mask = {},
      String const& endPixelCondition = S::NATURAL
) {
   Image out;
   UpperSkeleton2D( in, mask, out, endPixelCondition );
   return out;
}

/// \brief Reconstruction by dilation or erosion, also known as inf-reconstruction and sup-reconstruction
///
/// This function has the same effect as iteratively dilating (eroding) the image `marker` such that it remains lower
/// (higher) than `in` everywhere, until stability. However, this is implemented with a much more efficiently.
/// `direction` indicates which of the two operations to apply (`"dilation"` or `"erosion"`).
///
/// `out` will have the data type of `in`, and `marker` will be cast to that same type (with clamping to the target
/// range, see \ref dip::Convert).
///
/// See \ref connectivity for information on the connectivity parameter.
///
/// The algorithm implemented is a hybrid between the method proposed by Vincent (a forward raster scan, followed
/// by a backward raster scan, followed by a LIFO queue propagation method), and that proposed by Robinson and Whelan
/// (a priority queue method). We implement the forward and backward scan, and follow it by a priority queue propagation.
/// The priority queue method has the advantage of visiting each pixels exactly once.
///
/// For binary images, this function calls \ref dip::BinaryPropagation, which uses the same algorithm but is specialized
/// for the binary case (e.g. using a stack instead of a priority queue).
///
/// This functions is used by \ref dip::LimitedMorphologicalReconstruction, \ref dip::HMinima, \ref dip::HMaxima,
/// \ref dip::Leveling, \ref dip::OpeningByReconstruction, \ref dip::ClosingByReconstruction
///
/// !!! literature
///     - L. Vincent, "Morphological grayscale reconstruction in image analysis: applications and efficient algorithms",
///       IEEE Transactions on Image Processing 2(2):176-201, 1993.
///     - K. Robinson and P.F. Whelan, "Efficient morphological reconstruction: a downhill filter", Pattern Recognition
///       Letters 25:1759-1767, 2004.
DIP_EXPORT void MorphologicalReconstruction(
      Image const& marker,
      Image const& in, // grey-value mask
      Image& out,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
);
DIP_NODISCARD inline Image MorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
) {
   Image out;
   MorphologicalReconstruction( marker, in, out, connectivity, direction );
   return out;
}

/// \brief Reconstruction by dilation or erosion, but with a limited reach.
///
/// Performs the same function as \ref dip::MorphologicalReconstruction, but limiting the
/// reach of the operation to `maxDistance` pixels. This is an Euclidean distance, and
/// determines the zone of influence of each value in `marker`. The limited reach is
/// accomplished by updating `in`, rather than counting propagation steps.
///
/// See \ref dip::MorphologicalReconstruction for the meaning of the rest of the parameters,
/// and more information about the algorithm.
DIP_EXPORT void LimitedMorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      Image& out,
      dfloat maxDistance = 20,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
);
DIP_NODISCARD inline Image LimitedMorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      dfloat maxDistance = 20,
      dip::uint connectivity = 0,
      String const& direction = S::DILATION
) {
   Image out;
   LimitedMorphologicalReconstruction( marker, in, out, maxDistance, connectivity, direction );
   return out;
}

/// \brief Computes the H-Minima filtered image
///
/// The H-Minima filtered image has all local minima with a depth less than `h` removed:
///
/// ```cpp
/// HMinima = dip::MorphologicalReconstruction( in + h, in, connectivity, "erosion" );
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
DIP_NODISCARD inline Image HMinima(
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
/// HMaxima = dip::MorphologicalReconstruction( in - h, in, connectivity, "dilation" );
/// ```
///
/// \see dip::MorphologicalReconstruction, dip::Maxima, dip::HMinima
inline void HMaxima(
      Image const& in,
      Image& out,
      dfloat h,
      dip::uint connectivity = 0
) {
   Image tmp = Subtract( in, h, in.DataType() );
   MorphologicalReconstruction( tmp, in, out, connectivity, S::DILATION );
}
DIP_NODISCARD inline Image HMaxima(
      Image const& in,
      dfloat h,
      dip::uint connectivity = 0
) {
   Image out;
   HMaxima( in, out, h, connectivity );
   return out;
}

/// \brief Impose minima.
///
/// Regions in `marker` will be the only local minima in `in`: `dip::Minima( dip::ImposeMinima( a, b )) == b`, for any `a`.
///
/// The image `in` will be modified such that the regions marked by `marker` obtain the lowest possible value for the
/// given data type, and any other local minima in `in` are filled in to become plateaus. Minimum imposition is typically
/// applied in conjunction with the watershed to reduce the number of regions created. The function \ref dip::SeededWatershed
/// has a similar result, but obtained in a different way, to applying `dip::Watershed` to the output of `ImposeMinima`.
///
/// \see dip::MorphologicalReconstruction, dip::Minima, dip::Watershed, dip::SeededWatershed
DIP_EXPORT void ImposeMinima(
      Image const& in,
      Image const& marker,
      Image& out,
      dip::uint connectivity = 0
);
DIP_NODISCARD inline Image ImposeMinima(
      Image const& in,
      Image const& marker,
      dip::uint connectivity = 0
) {
   Image out;
   ImposeMinima( in, marker, out, connectivity );
   return out;
}

/// \brief The leveling of `in` imposed by `marker`.
///
/// The leveling introduces flat zones in the image, in such a way that, if $g_p > g_q$, then $f_p \geq g_p$
/// and $g_q \geq f_q$, with $g$ the leveling of $f$, and $p$, $q$ any two locations within the image.
/// That is, for any edge remaining in $g$, there exists an edge of equal or larger magnitude in $f$.
///
/// The leveling can be obtained by initializing $g$ to the `marker` image and iteratively applying
///
/// $$ g = (f \wedge \delta g) \vee \epsilon g $$
///
/// until idempotence ($g$ doesn't change any further). However, here it is implemented more efficiently
/// using \ref dip::MorphologicalReconstruction.
///
/// The `marker` image can be a smoothed version of `in`, then the leveling yields a similar simplification as
/// the smoothing, but preserving sharp edges.
///
/// !!! literature
///     - F. Meyer, "The levelings", Mathematical Morphology and its Applications to Image and Signal Processing
///       (proceedings of ISSM'98), pp. 199-206, 1998.
inline void Leveling(
      Image const& in,
      Image const& marker,
      Image& out,
      dip::uint connectivity = 0
) {
   Image tmp = MorphologicalReconstruction( marker, in, connectivity, S::DILATION );
   Image mask = marker < in;
   MorphologicalReconstruction( marker, in, out, connectivity, S::EROSION );
   out.At( mask ).Copy( tmp.At( mask ));
}
DIP_NODISCARD inline Image Leveling(
      Image const& in,
      Image const& marker,
      dip::uint connectivity = 0
) {
   Image out;
   Leveling( in, marker, out, connectivity );
   return out;
}

/// \brief Computes the area opening or closing. This is a parametric opening.
///
/// The area opening removes all local maxima that have an area smaller than the given parameter `filterSize`,
/// and is equivalent to the supremum of openings with all possible connected flat structuring elements of that area.
/// The output has all maxima being connected components with a size of at least `filterSize`. The area closing is the
/// dual operation.
///
/// Note that we refer to "area" here as the number of pixels, which readily extends to any number of dimensions.
///
/// `in` must be scalar and real-valued or binary.
///
/// `mask` restricts the image regions used for the operation.
///
/// `connectivity` determines what a connected component is. See \ref connectivity for information on the
/// connectivity parameter.
///
/// `polarity` can be `"opening"` (the default) or `"closing"`, to compute the area opening or area closing, respectively.
///
/// We use a union-find implementation similar to that described my Meijster and Wilkinson (2002), and is based on
/// the algorithm for our fast watershed (`"fast"` mode to \ref dip::Watershed). For binary images, this function calls
/// \ref dip::BinaryAreaOpening or \ref dip::BinaryAreaClosing.
///
/// \see dip::AreaClosing, dip::VolumeOpening, dip::VolumeClosing, dip::PathOpening, dip::DirectedPathOpening, dip::Opening, dip::Closing, dip::Maxima, dip::Minima, dip::SmallObjectsRemove
///
/// !!! literature
///     - L. Vincent, "Grayscale area openings and closings, their efficient implementation and applications",
///       Mathematical Morphology and Its Applications to Signal Processing, pp. 22-27, 1993.
///     - A. Meijster and M.H.F. Wilkinson, "A Comparison of Algorithms for Connected Set Openings and Closings",
///       IEEE Transactions on Pattern Analysis and Machine Intelligence 24(4):484-494, 2002.
DIP_EXPORT void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0,
      String const& polarity = S::OPENING
);
DIP_NODISCARD inline Image AreaOpening(
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

/// \brief Computes the area closing, calling \ref dip::AreaOpening with `polarity="closing"`.
inline void AreaClosing(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 0
) {
   AreaOpening( in, mask, out, filterSize, connectivity, S::CLOSING );
}
DIP_NODISCARD inline Image AreaClosing(
      Image const& in,
      Image const& mask,
      dip::uint filterSize,
      dip::uint connectivity = 0
) {
   Image out;
   AreaClosing( in, mask, out, filterSize, connectivity );
   return out;
}

/// \brief Computes the volume opening or closing. This is a parametric opening.
///
/// The volume opening removes all local maxima that have a volume smaller than the given parameter `filterSize`. The
/// "volume" is the integral over the pixel values, offset by the graylevel at which the maximum is cut.
/// The volume closing is the dual operation.
///
/// Comparing to the area opening, which removes peaks by the area of their support, this function removes peaks by
/// the volume being removed. The difference of the opening with the input image, in the case of the area opening,
/// is a series of peaks, each of which less than `filterSize` pixels, surrounded by zero-valued pixels.
/// In the case of the volume opening, these peaks all have an integral (sum of pixel values) of less than `filterSize`.
///
/// `in` must be scalar and real-valued. Binary images are not allowed.
///
/// `mask` restricts the image regions used for the operation.
///
/// `connectivity` determines what a connected component is. See \ref connectivity for information on the
/// connectivity parameter.
///
/// `polarity` can be `"opening"` (the default) or `"closing"`, to compute the area opening or area closing, respectively.
///
/// We use a union-find implementation similar to that described my Meijster and Wilkinson (2002), and is based on
/// the algorithm for our fast watershed (`"fast"` mode to \ref dip::Watershed).
///
/// \see dip::VolumeClosing, dip::AreaOpening, dip::AreaClosing, dip::PathOpening, dip::DirectedPathOpening, dip::Opening, dip::Closing, dip::Maxima, dip::Minima, dip::SmallObjectsRemove
///
/// !!! literature
///     - L. Vincent, "Grayscale area openings and closings, their efficient implementation and applications",
///       Mathematical Morphology and Its Applications to Signal Processing, pp. 22-27, 1993.
///     - A. Meijster and M.H.F. Wilkinson, "A Comparison of Algorithms for Connected Set Openings and Closings",
///       IEEE Transactions on Pattern Analysis and Machine Intelligence 24(4):484-494, 2002.
DIP_EXPORT void VolumeOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat filterSize,
      dip::uint connectivity = 0,
      String const& polarity = S::OPENING
);
DIP_NODISCARD inline Image VolumeOpening(
      Image const& in,
      Image const& mask,
      dfloat filterSize,
      dip::uint connectivity = 0,
      String const& polarity = S::OPENING
) {
   Image out;
   VolumeOpening( in, mask, out, filterSize, connectivity, polarity );
   return out;
}

/// \brief Computes the area closing, calling \ref dip::VolumeOpening with `polarity="closing"`.
inline void VolumeClosing(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat filterSize,
      dip::uint connectivity = 0
) {
   VolumeOpening( in, mask, out, filterSize, connectivity, S::CLOSING );
}
DIP_NODISCARD inline Image VolumeClosing(
      Image const& in,
      Image const& mask,
      dfloat filterSize,
      dip::uint connectivity = 0
) {
   Image out;
   VolumeClosing( in, mask, out, filterSize, connectivity );
   return out;
}

/// \brief Applies a path opening or closing in all possible directions
///
/// `length` is the length of the path. All `filterParam` arguments to \ref dip::DirectedPathOpening that yield a
/// length of `length` pixels and represent unique directions are generated, and the directed path opening or closing
/// is computed for each of them. The supremum (when `polarity` is `"opening"`) or infimum (when it is `"closing"`) is
/// computed over all results. See \ref dip::DirectedPathOpening for a description of the algorithm and the parameters.
DIP_EXPORT void PathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint length = 7,
      String const& polarity = S::OPENING,
      StringSet const& mode = {}
);
DIP_NODISCARD inline Image PathOpening(
      Image const& in,
      Image const& mask,
      dip::uint length = 7,
      String const& polarity = S::OPENING,
      StringSet const& mode = {}
) {
   Image out;
   PathOpening( in, mask, out, length, polarity, mode );
   return out;
}

/// \brief Applies a path opening or closing in a specific direction.
///
/// The path opening is an opening over all possible paths of a specific length and general direction. A path
/// direction represents a 90 degree cone within which paths are generated. The paths are formed by single pixel
/// steps in one of three directions (in 2D): the main direction, or 45 degrees to the left or right. That is,
/// if the main direction is [1,0] (to the right), then [1,-1] and [1,1] (diagonal up or down) are also possible
/// steps. This leads to a number of different paths that is exponential in its lengths. However, the opening over
/// all these paths can be computed in $O(n \log(n))$ time, with $n$ the path length.
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
/// The `polarity` parameter can be `"opening"` (the default) or `"closing"`, to compute the path opening and path
/// closing, respectively.
///
/// When `mode` contains `"constrained"`, the path construction described above is modified such that, after every alternate
/// step, a step in the main direction must be taken. This constraint avoids a zig-zag line that causes the path
/// opening to yield much shorter lines for the diagonal directions if the lines in the image are thicker than one pixel.
/// See the paper by Luengo referenced below. It also reduces the cone size from 90 degrees to 45 degrees, making the
/// algorithm more directionally-selective. The constrained mode increases computation time a little, but is highly
/// recommended when using the path opening in a granulometry. The alternate flag is `"unconstrained"`, which is the
/// default and does not need to be given.
///
/// Path openings can be sensitive to noise. If `mode` contains `"robust"`, a robust path opening or closing is
/// obtained. A robust path opening is computed by dilating the image with a 2x2 rectangular structuring element,
/// applying the path opening, then taking the infimum of the result and the input (Merveille, 2018). For a path
/// closing, the erosion and the supremum are used instead.
///
/// !!! par "Definition of `filterSize`"
///     `length = max(abs(filterSize))` is the number of pixels in the line.
///
///     The path direction is determined by translating `filterSize` to an array with -1, 0 and 1 values using
///     `direction = round(filterSize/length)`.
///
///     For example, if `filterSize=[7,0]`, then `length` is 7, and
///     `direction` is `[1,0]` (to the right), with `[1,1]` and `[1,-1]` as alternate directions.
///
/// !!! literature
///     - H. Heijmans, M. Buckley and H. Talbot, "Path Openings and Closings", Journal of Mathematical Imaging and Vision 22:107-119, 2005.
///     - H. Talbot and B. Appleton, "Efficient complete and incomplete path openings and closings", Image and Vision Computing 25:416-425, 2007.
///     - C.L. Luengo Hendriks, "Constrained and dimensionality-independent path openings", IEEE Transactions on Image Processing 19(6):1587–1595, 2010.
///     - O. Merveille, H. Talbot, L. Najman and N. Passat, "Curvilinear Structure Analysis by Ranking the Orientation Responses of Path Operators", IEEE Transactions on Pattern Analysis and Machine Intelligence 40(2):304-317, 2018.
DIP_EXPORT void DirectedPathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      IntegerArray filterParam,
      String const& polarity = S::OPENING,
      StringSet const& mode = {}
);
DIP_NODISCARD inline Image DirectedPathOpening(
      Image const& in,
      Image const& mask,
      IntegerArray filterParam,
      String const& polarity = S::OPENING,
      StringSet const& mode = {}
) {
   Image out;
   DirectedPathOpening( in, mask, out, std::move( filterParam ), polarity, mode );
   return out;
}

/// \brief Opening by reconstruction
///
/// Applies a structural erosion followed by a reconstruction by dilation.
///
/// See \ref dip::Erosion and \ref dip::MorphologicalReconstruction for a description of the parameters.
inline void OpeningByReconstruction(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   Image in_c = in;
   if( out.Aliases( in_c )) {
      out.Strip(); // Make sure we don't overwrite `in` in the first step
   }
   DIP_STACK_TRACE_THIS( Erosion( in_c, out, se, boundaryCondition ));
   DIP_STACK_TRACE_THIS( MorphologicalReconstruction( out, in_c, out, connectivity, S::DILATION ));
}
DIP_NODISCARD inline Image OpeningByReconstruction(
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
/// See \ref dip::Dilation and \ref dip::MorphologicalReconstruction for a description of the parameters.
inline void ClosingByReconstruction(
      Image const& in,
      Image& out,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   Image in_c = in;
   if( out.Aliases( in_c )) {
      out.Strip(); // Make sure we don't overwrite `in` in the first step
   }
   DIP_STACK_TRACE_THIS( Dilation( in_c, out, se, boundaryCondition ));
   DIP_STACK_TRACE_THIS( MorphologicalReconstruction( out, in_c, out, connectivity, S::EROSION ));
}
DIP_NODISCARD inline Image ClosingByReconstruction(
      Image const& in,
      StructuringElement const& se = {},
      dip::uint connectivity = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   ClosingByReconstruction( in, out, se, connectivity, boundaryCondition );
   return out;
}

/// \brief Alternating sequential filters for smoothing
///
/// Applies alternating sequential filters to `in`, using structuring element sizes given by the range `sizes`.
/// Alternating sequential filters are two morphological filters opening and closing, applied in sequence, from
/// a small size to a larger size. This provides an effective smoothing that is less biased than applying an
/// opening and closing of a single size (as in \ref dip::MorphologicalSmoothing).
/// `polarity` can be `"open-close"` or `"close-open"`, and determines which of the operations is applied first.
///
/// For example, if `sizes` is `{3,7,2}` and `polarity` is `"open-close"`, the following operations are applied:
///
/// ```cpp
/// dip::Opening( in,  out, { 3, shape } );
/// dip::Closing( out, out, { 3, shape } );
/// dip::Opening( out, out, { 5, shape } );
/// dip::Closing( out, out, { 5, shape } );
/// dip::Opening( out, out, { 7, shape } );
/// dip::Closing( out, out, { 7, shape } );
/// ```
///
/// `mode` is one of:
///
/// - `"structural"`: uses structural openings and closings (see \ref dip::Opening).
/// - `"reconstruction"`: uses openings and closings by reconstruction (see \ref dip::OpeningByReconstruction).
/// - `"area"`: uses area openings and closings (see \ref dip::AreaOpening) -- `shape` is ignored.
DIP_EXPORT void AlternatingSequentialFilter(
      Image const& in,
      Image& out,
      Range const& sizes = { 3, 7, 2 },
      String const& shape = S::ELLIPTIC,
      String const& mode = S::STRUCTURAL,
      String const& polarity = S::OPENCLOSE,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image AlternatingSequentialFilter(
      Image const& in,
      Range const& sizes = { 3, 7, 2 },
      String const& shape = S::ELLIPTIC,
      String const& mode = S::STRUCTURAL,
      String const& polarity = S::OPENCLOSE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   AlternatingSequentialFilter( in, out, sizes, shape, mode, polarity, boundaryCondition );
   return out;
}

/// \brief The Hit-and-Miss transform, uses two structuring elements, `hit` must be within the structures,
/// `miss` must be without.
///
/// For a binary image, the result is the intersection of the erosion of the image with `hit` and the erosion of the
/// inverted image with `miss`.
///
/// For a grey-value image, there are two definitions of the operator. If `mode` is `"unconstrained"`, the output is
/// the difference of the erosion with `hit` and the dilation with `miss`, with any negative values clipped to 0.
///
/// If `mode` is `"constrained"`, a more restrictive definition is applied (conditions evaluated pixel-wise):
///
/// - If `in == erosion(in,hit) && dilation(in,miss) < in`: `out = in - dilation(in,miss)`.
/// - If `in == dilation(in,miss) && erosion(in,hit) > in`: `out = erosion(in,hit) - in`.
/// - Otherwise: `out = 0`.
///
/// Note that the two structuring elements must be disjoint. If one pixel is set in both structuring elements,
/// the output will be all zeros.
///
/// See also \ref dip::SupGenerating for a function specific to binary images.
///
/// !!! literature
///     - P. Soille, "Morphological Image Analysis", 2^nd^ Edition, sections 5.1.1 and 5.1.2. Springer, 2002.
DIP_EXPORT void HitAndMiss(
      Image const& in,
      Image& out,
      StructuringElement const& hit,
      StructuringElement const& miss,
      String const& mode = S::UNCONSTRAINED,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image HitAndMiss(
      Image const& in,
      StructuringElement const& hit,
      StructuringElement const& miss,
      String const& mode = S::UNCONSTRAINED,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   HitAndMiss( in, out, hit, miss, mode, boundaryCondition );
   return out;
}

/// \brief The Hit-and-Miss transform, uses a single structuring element in the form of a small image that
/// has "hit", "miss" and "don't care" values.
///
/// The `hit` SE is `se == 1`, the `miss` SE is `se == 0`. "Don't care" values are any other value.
///
/// See the description for the other \ref dip::HitAndMiss function for a description of the other parameters.
inline void HitAndMiss(
      Image const& in,
      Image& out,
      Image const& se,
      String const& mode = S::UNCONSTRAINED,
      StringArray const& boundaryCondition = {}
) {
   DIP_THROW_IF( !se.IsForged(), E::IMAGE_NOT_FORGED );
   HitAndMiss( in, out, StructuringElement{ se == 1 }, StructuringElement{ se == 0 }, mode, boundaryCondition );
}
DIP_NODISCARD inline Image HitAndMiss(
      Image const& in,
      Image const& se,
      String const& mode = S::UNCONSTRAINED,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   HitAndMiss( in, out, se, mode, boundaryCondition );
   return out;
}

/// \endgroup

} // namespace dip

#endif // DIP_MORPHOLOGY_H
