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
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "pixel_table.h"


/// \file
/// \brief Declares functions that implement mathematical morphology.


namespace dip {


/// \defgroup morphology Mathematical morphology
/// \ingroup filtering
/// \brief Functions that implement mathematical morphology
/// \{

// TODO: some functions are \ingroup segmentation! Add some links in that group's documentation to these.

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
///  -  `"rectangular"`, `"elliptic"`, and `"diamond"`: these are unit circles in different metrics. The
///     size array corresponds to the diameter in that metric. `"elliptic"` is the default shape,
///     because it is isotropic, but it also is the slowest to compute. Both `"elliptic"` and `"diamond"`
///     structuring elements always are symmetric. That is, their origin is centered on a pixel.
///     The pixels included in the shape are those at most half of the diameter away from the origin.
///     For the `"rectangular"` structuring element, a box with integer sizes is always generated,
///     but the box can be even in size also, meaning that the origin is in between pixels.
///     Any size array element that is smaller or equal to 1 causes that dimension to not be processed.
///  -  `"parabolic"`: the parabolic structuring element is the morphological equivalent to the Gaussian
///     kernel in linear filtering. It is separable and perfectly isotropic. The size array corresponds
///     to the scaling of the parabola (i.e. the \f$a\f$ in \f$a^{-2} x^2\f$). A value equal
///     or smaller to 0 causes that dimension to not be processed. The boundary condition is ignored
///     for operators with this structuring element, and the output image is always a floating-point type.
///  -  `"interpolated line"`, `"discrete line"`: these are straight lines, using different implementations.
///     The size array corresponds to the size of the bounding box of the line, with signs indicating
///     the direction. Thus, if the size array is `[2,2]`, the line goes right and down two pixels,
///     meaning that the line is formed by two pixels at an angle of 45 degrees down. If the size array
///     is `[-2,2]`, then the line is again two pixels, but at an angle of 125 degrees. (Note that
///     in images, angles increase clockwise from the x-axis, as the y-axis is inverted).
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
/// leads to an opening or a closing if the structuring element is symmetric. For non-symmetric strcturing
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
class DIP_NO_EXPORT StructuringElement{
      // TODO: In the old DIPlib, line SEs used filterParam = [length,angle], and only applied to 2D images!
      // TODO: Implement the discrete line for 2D without skews, so it's more efficient.
      // TODO: Implement periodic lines, construct translation-invariant discrete lines using periodic lines
      // TODO: Construct diamond SE and approximations to elliptic SE using lines
   public:
      /// \brief Default constructor leads to a circle with a diameter of 7 pixels.
      StructuringElement() {}

      /// \brief A string implicitly converts to a structuring element, it is interpreted as a shape.
      StructuringElement( String const& shape ) : shape_( shape ) {}

      /// \brief A `dip::FloatArray` implicitly converts to a structuring element, it is interpreted as the
      /// sizes along each dimension. A second argument specifies the shape.
      StructuringElement( FloatArray size, String const& shape = "elliptic" ) : size_( size ), shape_( shape ) {}

      /// \brief An image implicitly converts to a structuring element.
      StructuringElement( Image const& image ) : image_( image.QuickCopy() ), shape_( "custom" ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
      }

      /// \brief Creates a `dip::PixelTable` structure representing the shape of the SE
      ///
      /// The structuring element will be adapted to the the image `in`, by expanding its dimensionality
      /// to match that of `in`, and by choosing an optimal processing dimension.
      ///
      /// This function will throw if the SE is not defined by an image, and the shape string is other than
      /// `"elliptic"` or `"diamond"`.
      ///
      /// Note that the size array will get adapted to the dimensionality of `in`, and therefore this
      /// object cannot be reused with an image of a different dimensionality.
      dip::PixelTable PixelTable( Image const& in, bool mirror = false ) {
         // Tests SE and `in` match in dimensionality, and adjusts `sizes_`.
         Sizes( in ); // Discard the output value, as it is a reference to `sizes_`.
         dip::PixelTable pixelTable;
         if( shape_ == "custom" ) {
            Image se = image_.QuickCopy();
            if( mirror ) {
               se.Mirror();
            }
            dip::uint procDim = Framework::OptimalProcessingDim( in, se.Sizes() );
            if( se.DataType().IsBinary() ) {
               pixelTable = { se, {}, procDim };
            } else {
               // TODO: make a pixel table of IsFinite(se).
               pixelTable = { se > -1e15, {}, procDim };
               pixelTable.AddWeights( se );
            }
            if( mirror ) {
               pixelTable.MirrorOrigin();
            }
         } else if(( shape_ == "elliptic" ) || ( shape_ == "diamond" )) {
            dip::uint procDim = Framework::OptimalProcessingDim( in, UnsignedArray{ size_ } );
            pixelTable = { shape_, size_, procDim };
            if( mirror ) {
               pixelTable.MirrorOrigin();
            }
         } else {
            DIP_THROW( "Cannot produce a pixel table from this structuring element definition." );
         }
         return pixelTable;
      }

      /// \brief Retrieves the size array, adjusted to the image
      FloatArray const& Sizes( Image const& in ) {
         DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
         dip::uint nDim = in.Dimensionality();
         DIP_THROW_IF( nDim < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         if( shape_ == "custom" ) {
            if( image_.Dimensionality() < nDim ) {
               image_.ExpandDimensionality( nDim );
            }
            DIP_THROW_IF( image_.Dimensionality() != nDim, E::DIMENSIONALITIES_DONT_MATCH );
            size_ = FloatArray{ image_.Sizes() };
         } else {
            ArrayUseParameter( size_, in.Dimensionality(), 1.0 );
         }
         return size_;
      }

      /// \brief Tests to see if the structuring element is parabolic
      bool IsParabolic() const { return shape_ == "parabolic"; }

      /// \brief Tests to see if the structuring element is rectangular
      bool IsRectangular() const { return shape_ == "rectangular"; }

      /// \brief Tests to see if the structuring element is an interpolated line
      bool IsInterpolatedLine() const { return shape_ == "interpolated line"; }

      /// \brief Tests to see if the structuring element is a discrete line
      bool IsDiscreteLine() const { return shape_ == "discrete line"; }

   private:
      Image image_;
      FloatArray size_ = { 7 };
      String shape_ = "elliptic";
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


DIP_EXPORT void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
);

DIP_EXPORT void SeededWatershed(
      Image const&,
      Image const&,
      Image const&,
      Image const&,
      dip::uint connectivity = 1,
      String const& sortOrder = "low first",
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
);

DIP_EXPORT void LocalMinima(
      Image const&,
      Image const&,
      Image const&,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      String const& output = "binary"
);

DIP_EXPORT void UpperEnvelope(
      Image const& in,
      Image& out,
      Image& bottom,
      Image& labels,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0
);

DIP_EXPORT void MorphologicalReconstruction(
      Image const& marker,
      Image const& in, // grey-value mask
      Image& out,
      dip::uint connectivity = 1
);
// TODO: MorphologicalReconstruction should have a flag to invert the operation, so it does reconstruction by erosions.

DIP_EXPORT void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 1,
      bool closing = false // should be a string
);


DIP_EXPORT void PathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint length = 7,
      bool closing = false, // should be a string
      bool constrained = true // should be a string
);

DIP_EXPORT void DirectedPathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      FloatArray const& filterParam = {7, 0},
      bool closing = false, // should be a string
      bool constrained = true // should be a string
);

// TODO: Union, Intersection (for binary images), Supremum, Infimum (for grey-value images), Max, Min (idem) -> These are all the same two functions...
// TODO: h-minima & h-maxima, opening and closing by reconstruction
// TODO: alternating sequential open-close filter (3 versions: with structural opening, opening by reconstruction, and area opening)
// TODO: hit'n'miss, where the interval is rotated over 180, 90 or 45 degrees.
// TODO: thinning & thickening, to be implemented as iterated hit'n'miss.
// TODO: levelling

/// \}

} // namespace dip

#endif // DIP_MORPHOLOGY_H
