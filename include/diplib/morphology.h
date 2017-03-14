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


/// \file
/// \brief Declares functions that implement mathematical morphology.


namespace dip {


/// \defgroup morphology Mathematical morphology
/// \ingroup filtering
/// \brief Functions that implement mathematical morphology
/// \{

// TODO: some functions are \ingroup segmentation! Add some links in that group's documentation to these.

/// \brief Applies the dilation with a standard structuring element.
///
/// `filterParam` is the size of the structuring element, and `filterShape` is its shape.
/// See \ref structuringelement.
///
/// `boundary` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add min"`, but any value can be used.
///
/// \see Dilation(Image const&, Image const&, Image&, StringArray const&), Erosion
// TODO: In the old DIPlib, line SEs used filterParam = [length,angle], and only applied to 2D images!
// TODO: Implement the discrete line for 2D without skews, so it's more efficient.
// TODO: Implement periodic lines, construct translation-invariant discrete lines using periodic lines
// TODO: Construct diamond SE and approximations to elliptic SE using lines
void Dilation(
      Image const& in,
      Image& out,
      FloatArray const& filterParam = { 7 },
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the dilation with a custom structuring element.
///
/// `se` is the structuring element. See \ref structuringelement.
///
/// Note that `se` is directly used as neighborhood (i.e. no mirroring is applied).
/// That is, `%dip::Dilation` and `dip::Erosion` will use the same neighborhood. Their composition only
/// leads to an opening or a closing if `se` is symmetric. For non-symmetric `se`, you need to mirror
/// it in one of the two function calls:
/// ```cpp
///     dip::Image out = dip::Dilation( dip::Erosion( in, se ), se.Mirror() );
/// ```
/// (Do note that, in the code above, `se` itself is modified! use `se.QuickCopy().Mirror()` to prevent
/// that.)
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add min"`, but any value can be used.
///
/// \see Dilation(Image const&, Image&, FloatArray, String const&, StringArray const&), Erosion
void Dilation(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the erosion with a standard structuring element.
///
/// `filterParam` is the size of the structuring element, and `filterShape` is its shape.
/// See \ref structuringelement.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add max"`, but any value can be used.
///
/// \see Erosion(Image const&, Image const&, Image&, StringArray const&), Dilation
void Erosion(
      Image const& in,
      Image& out,
      FloatArray const& filterParam = { 7 },
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the erosion with a custom structuring element.
///
/// Note that `se` is directly used as neighborhood (i.e. no mirroring is applied).
/// That is, `%dip::Dilation` and `dip::Erosion` will use the same neighborhood. Their composition only
/// leads to an opening or a closing if `se` is symmetric. For non-symmetric `se`, you need to mirror
/// it in one of the two function calls:
/// ```cpp
///     dip::Image out = dip::Dilation( dip::Erosion( in, se ), se.Mirror() );
/// ```
/// (Do note that, in the code above, `se` itself is modified! use `se.QuickCopy().Mirror()` to prevent
/// that.)
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// The default value, and most meaningful one, is `"add max"`, but any value can be used.
///
/// \see Erosion(Image const&, Image&, FloatArray, String const&, StringArray const&), Dilation
void Erosion(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the closing with a standard structuring element.
///
/// `filterParam` is the size of the structuring element, and `filterShape` is its shape.
/// See \ref structuringelement.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the closing are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see Erosion, Dilation, Opening
void Closing(
      Image const& in,
      Image& out,
      FloatArray const& filterParam = { 7 },
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the closing with a custom structuring element.
///
/// `se` is the structuring element. See \ref structuringelement.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the closing are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see Erosion, Dilation, Opening
void Closing(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the opening with a standard structuring element.
///
/// `filterParam` is the size of the structuring element, and `filterShape` is its shape.
/// See \ref structuringelement.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the opening are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see Erosion, Dilation, Closing
void Opening(
      Image const& in,
      Image& out,
      FloatArray const& filterParam = { 7 },
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

/// \brief Applies the opening with a custom structuring element.
///
/// `se` is the structuring element. See \ref structuringelement.
///
/// `boundaryCondition` determines the boundary conditions. See `dip::BoundaryCondition`.
/// Meaningful values for the opening are `"add max"` and `"add min"`, but any value can
/// be used. The default empty array causes the function to use `"add min"` with the dilation
/// and `"add max"` with the erosion, equivalent to ignoring what's outside the image.
///
/// \see Erosion, Dilation, Closing
void Opening(
      Image const& in,
      Image const& se,
      Image& out,
      StringArray const& boundaryCondition = {}
);


// --- TODO functions below here ---

enum dip_MphEdgeType {
   DIP_MPH_TEXTURE = 1,
   DIP_MPH_OBJECT = 2,
   DIP_MPH_BOTH = 3,
   DIP_MPH_DYNAMIC = 3
};

enum dip_MphTophatPolarity {
   DIP_MPH_BLACK = 1,
   DIP_MPH_WHITE = 2
};

void Tophat(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      dip_MphTophatPolarity = DIP_MPH_WHITE, // should be a string
      StringArray const& boundaryCondition = {}
);

void MorphologicalThreshold(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      StringArray const& boundaryCondition = {}
);

void MorphologicalGist(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      StringArray const& boundaryCondition = {}
);

void MorphologicalRange(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      StringArray const& boundaryCondition = {}
);

void MorphologicalGradientMagnitude(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      StringArray const& boundaryCondition = {}
);

enum dipf_LeeSign {
   DIP_LEE_UNSIGNED = 1,
   DIP_LEE_SIGNED = 2
};

void Lee(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dip_MphEdgeType edgeType = DIP_MPH_TEXTURE, // should be a string
      dipf_LeeSign flags = DIP_LEE_UNSIGNED, // should be a string
      StringArray const& boundaryCondition = {}
);

enum dipf_MphSmoothing {
   DIP_MPH_OPEN_CLOSE = 1,
   DIP_MPH_CLOSE_OPEN = 2,
   DIP_MPH_AVERAGE = 3
};

void MorphologicalSmoothing(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      dipf_MphSmoothing = DIP_MPH_AVERAGE, // should be a string
      StringArray const& boundaryCondition = {}
);

void MultiScaleMorphologicalGradient(
      Image const& in,
      Image const& se,
      Image& out,
      dip::uint upperSize = 9,
      dip::uint lowerSize = 3,
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

void MorphologicalLaplace(
      Image const& in,
      Image const& se,
      Image& out,
      FloatArray const& filterParam = {7},
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);

void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      bool binaryOutput = true // should be a string
);

enum dipf_GreyValueSortOrder {A};
void SeededWatershed(
      Image const&,
      Image const&,
      Image const&,
      Image const&,
      dip::uint connectivity = 1,
      dipf_GreyValueSortOrder sortOrder = A, // should be a string
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      bool binaryOutput = true // should be a string
);

void LocalMinima(
      Image const&,
      Image const&,
      Image const&,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0,
      bool binaryOutput = true // should be a string
);

void UpperEnvelope(
      Image const& in,
      Image& out,
      Image& bottom,
      Image& labels,
      dip::uint connectivity = 1,
      dfloat maxDepth = 1,
      dip::uint maxSize = 0
);

void MorphologicalReconstruction(
      Image const& marker,
      Image const& in, // grey-value mask
      Image& out,
      dip::uint connectivity = 1
);
// TODO: MorphologicalReconstruction should have a flag to invert the operation, so it does reconstruction by erosions.

void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity = 1,
      bool closing = false // should be a string
);

void PathOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint length = 7,
      bool closing = false, // should be a string
      bool constrained = true // should be a string
);

void DirectedPathOpening(
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
