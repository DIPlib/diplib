/*
 * DIPlib 3.0
 * This file contains functionality related to the boundary condition
 *
 * (c)2016-2018, Cris Luengo.
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

#ifndef DIP_BOUNDARY_H
#define DIP_BOUNDARY_H

#include "diplib.h"


/// \file
/// \brief Functionality implementing boundary conditions.
/// \see infrastructure


namespace dip {


/// \defgroup boundary Boundary
/// \ingroup infrastructure
/// \brief Handling image boundary extension for filtering
/// \{


/// \brief Enumerates various ways of extending image data beyond its boundary.
///
/// This enumerator is used by the framework functions and some internal functions.
/// Externally, the boundary condition is represented by strings.
///
/// Most functions will take a string instead of a `dip::BoundaryCondition` constant.
/// The following table links boundary condition constants and their string representations.
///
/// `BoundaryCondition` constant | String                 | Definition
/// -------------------------- | ------------------------ | ----------
/// `SYMMETRIC_MIRROR`         | "mirror"                 | The data are mirrored, with the value at -1 equal to the value at 0, at -2 equal to at 1, etc.
/// `ASYMMETRIC_MIRROR`        | "asym mirror"            | The data are mirrored and inverted.
/// `PERIODIC`                 | "periodic"               | The data are repeated periodically, with the value at -1 equal to the value of the last pixel.
/// `ASYMMETRIC_PERIODIC`      | "asym periodic"          | The data are repeated periodically and inverted.
/// `ADD_ZEROS`                | "add zeros"              | The boundary is filled with zeros.
/// `ADD_MAX_VALUE`            | "add max"                | The boundary is filled with the max value for the data type.
/// `ADD_MIN_VALUE`            | "add min"                | The boundary is filled with the min value for the data type.
/// `ZERO_ORDER_EXTRAPOLATE`   | "zero order"             | The value at the border is repeated indefinitely.
/// `FIRST_ORDER_EXTRAPOLATE`  | "first order"            | A linear function is defined based on the value closest to the border, the function reaches zero at the end of the extended boundary.
/// `SECOND_ORDER_EXTRAPOLATE` | "second order"           | A quadratic function is defined based on the two values closest to the border, the function reaches zero at the end of the extended boundary.
/// `THIRD_ORDER_EXTRAPOLATE`  | "third order"            | A cubic function is defined based on the two values closest to the border, the function reaches zero with a zero derivative at the end of the extended boundary.
/// `DEFAULT`                  | "default" or ""          | The default value, currently equal to `SYMMETRIC_MIRROR`.
/// `ALREADY_EXPANDED`         | "already expanded"       | The dangerous option. The image is an ROI of a larger image, and the operation cannot reach outside this larger image. Not supported by all functions, and can always be combined with other options.
enum class DIP_NO_EXPORT BoundaryCondition {
      SYMMETRIC_MIRROR,
      ASYMMETRIC_MIRROR,
      PERIODIC,
      ASYMMETRIC_PERIODIC,
      ADD_ZEROS,
      ADD_MAX_VALUE,
      ADD_MIN_VALUE,
      ZERO_ORDER_EXTRAPOLATE,
      FIRST_ORDER_EXTRAPOLATE,
      SECOND_ORDER_EXTRAPOLATE,
      THIRD_ORDER_EXTRAPOLATE,
      DEFAULT = SYMMETRIC_MIRROR,
      ALREADY_EXPANDED
};

using BoundaryConditionArray = DimensionArray< BoundaryCondition >; ///< An array to hold boundary conditions.


/// \brief Convert a string to a boundary condition.
inline BoundaryCondition StringToBoundaryCondition( String const& bc ) {
   if( bc.empty() ) { return BoundaryCondition::DEFAULT; }
   else if( bc == S::DEFAULT ) { return BoundaryCondition::DEFAULT; }
   else if( bc == S::SYMMETRIC_MIRROR ) { return BoundaryCondition::SYMMETRIC_MIRROR; }
   else if( bc == S::ASYMMETRIC_MIRROR ) { return BoundaryCondition::ASYMMETRIC_MIRROR; }
   else if( bc == S::PERIODIC ) { return BoundaryCondition::PERIODIC; }
   else if( bc == S::ASYMMETRIC_PERIODIC ) { return BoundaryCondition::ASYMMETRIC_PERIODIC; }
   else if( bc == S::ADD_ZEROS ) { return BoundaryCondition::ADD_ZEROS; }
   else if( bc == S::ADD_MAX_VALUE ) { return BoundaryCondition::ADD_MAX_VALUE; }
   else if( bc == S::ADD_MIN_VALUE ) { return BoundaryCondition::ADD_MIN_VALUE; }
   else if( bc == S::ZERO_ORDER_EXTRAPOLATE ) { return BoundaryCondition::ZERO_ORDER_EXTRAPOLATE; }
   else if( bc == S::FIRST_ORDER_EXTRAPOLATE ) { return BoundaryCondition::FIRST_ORDER_EXTRAPOLATE; }
   else if( bc == S::SECOND_ORDER_EXTRAPOLATE ) { return BoundaryCondition::SECOND_ORDER_EXTRAPOLATE; }
   else if( bc == S::THIRD_ORDER_EXTRAPOLATE ) { return BoundaryCondition::THIRD_ORDER_EXTRAPOLATE; }
   else if( bc == S::ALREADY_EXPANDED ) { return BoundaryCondition::ALREADY_EXPANDED; }
   else DIP_THROW( "Boundary condition not recognized: " + bc );
}

/// \brief Convert an array of strings to an array of boundary conditions.
inline BoundaryConditionArray StringArrayToBoundaryConditionArray( StringArray const& bc ) {
   BoundaryConditionArray out( bc.size() );
   for( dip::uint ii = 0; ii < bc.size(); ++ii ) {
      out[ ii ] = StringToBoundaryCondition( bc[ ii ] );
   }
   return out;
}

/// \brief Check the length of a `BoundaryConditionArray`, and extend it if necessary and possible.
///
/// The output will
/// have `nDims` elements. If the input has a single value, it will be used for all dimensions. If the
/// input is an empty array, the default boundary condition will be used for all dimensions. If the array
/// has `nDims` elements, it is copied unchanged. For any other length, an exception is thrown.
///
/// \see  ArrayUseParameter
inline void BoundaryArrayUseParameter( BoundaryConditionArray& bc, dip::uint nDims ) {
   ArrayUseParameter( bc, nDims, BoundaryCondition::DEFAULT );
}


/// \brief Returns a pixel with a copy of the sample values at `coords`.
///
/// If `coords` falls outside the image, then the boundary condition `bc` is used to determine what values to write
/// into the output pixel.
///
/// First, second and third order interpolations are not implemented, because their functionality
/// is impossible to reproduce in this simple function. Use `dip::ExtendImage` to get the functionality
/// of these boundary conditions.
DIP_EXPORT Image::Pixel ReadPixelWithBoundaryCondition(
      Image const& img,
      IntegerArray coords, // getting a local copy so we can modify it
      BoundaryConditionArray const& bc
);


namespace Option {

/// \class dip::Option::ExtendImageFlags
/// \brief Determines which properties to compare.
///
/// Valid values are:
///
/// `%ExtendImageFlags` constant   | Definition
/// ------------------------------ | ----------
/// `ExtendImage::Masked`          | The output image is a window on the boundary-extended image of the same size as the input.
/// `ExtendImage::ExpandTensor`    | The output image has normal tensor storage.
///
/// Note that you can add these constants together: `dip::Option::ExtendImage::Masked + dip::Option::ExtendImage::ExpandTensor`.
enum class DIP_NO_EXPORT ExtendImage { Masked, ExpandTensor };
DIP_DECLARE_OPTIONS( ExtendImage, ExtendImageFlags )

} // namespace Option

/// \brief Extends the image `in` by `borderSizes` along each dimension.
///
/// This function is identical to the `%dip::ExtendImage` below, except it uses boundary condition constants and option
/// constants instead of strings. This version is meant to be used by low-level library functions.
DIP_EXPORT void ExtendImage(
      Image const& in,
      Image& out,
      UnsignedArray borderSizes,
      BoundaryConditionArray const& boundaryCondition = {},
      Option::ExtendImageFlags options = {}
);

/// \brief Extends the image `in` by `borderSizes` along each dimension.
///
/// The output image has size `in.Size( ii ) + 2 * borderSizes[ ii ]` along dimension `ii`.
///
/// The new regions are filled using the boundary condition `bc`. If `boundaryCondition` is an empty array, the default
/// boundary condition is used along all dimensions. If `boundaryCondition` has a single element, it is used for all
/// dimensions. Similarly, if `borderSizes` has a single element, it is used for all dimensions.
///
/// If `options` contains `"masked"`, the output image is a window on the boundary-extended image, of the
/// same size as `in`. That is, `out` will be identical to `in` except that it is possible
/// to access pixels outside of its domain.
///
/// If `options` contains `"expand tensor"`, the output image will have normal tensor storage
/// (`dip::Tensor::HasNormalOrder` is true). This affects only those input images that have a transposed, symmetric
/// or triangular matrix as tensor shape.
inline void ExtendImage(
      Image const& in,
      Image& out,
      UnsignedArray const& borderSizes,
      StringArray const& boundaryCondition,
      StringSet const& options = {}
) {
   BoundaryConditionArray bc;
   DIP_START_STACK_TRACE
      bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   DIP_END_STACK_TRACE
   Option::ExtendImageFlags opts;
   if( options.count( "masked" ) > 0 ) {
      opts += Option::ExtendImage::Masked;
   }
   if( options.count( "expand tensor" ) > 0 ) {
      opts += Option::ExtendImage::ExpandTensor;
   }
   ExtendImage( in, out, borderSizes, bc, opts );
}
inline Image ExtendImage(
      Image const& in,
      UnsignedArray const& borderSizes,
      StringArray const& boundaryCondition = {},
      StringSet const& options = {} ) {
   Image out;
   ExtendImage( in, out, borderSizes, boundaryCondition, options );
   return out;
}

/// \brief Fills the pixels outside a region in the image using a boundary condition.
///
/// This function is identical to the `%dip::ExtendRegion` below, except it uses boundary condition constants
/// instead of strings. This version is meant to be used by low-level library functions.
DIP_EXPORT void ExtendRegion(
      Image& image,
      RangeArray ranges,
      BoundaryConditionArray boundaryCondition = {}
);

/// \brief Fills the pixels outside a region in the image using a boundary condition.
///
/// The region that is preserved is specified through `ranges`. The step sizes are ignored, only the `start` and `stop`
/// values of `ranges` are used.
///
/// The pixels outside of the region are filled using the boundary condition `boundaryCondition`, using only those
/// values inside the region. If `boundaryCondition` is an empty array, the default boundary condition is used along
/// all dimensions. If `boundaryCondition` has a single element, it is used for all dimensions. `ranges` is
/// similarly expanded if it has a single element.
inline void ExtendRegion(
      Image& image,
      RangeArray const& ranges,
      StringArray const& boundaryCondition
) {
   BoundaryConditionArray bc;
   DIP_START_STACK_TRACE
      bc = StringArrayToBoundaryConditionArray( boundaryCondition );
   DIP_END_STACK_TRACE
   ExtendRegion( image, ranges, bc );
}

/// \brief Fills the pixels outside a region in the image using a boundary condition.
///
/// The region that is preserved is specified through `origin` and `sizes`.
///
/// The pixels outside of the region are filled using the boundary condition `boundaryCondition`, using only those
/// values inside the region. If `boundaryCondition` is an empty array, the default boundary condition is used along
/// all dimensions. If `boundaryCondition` has a single element, it is used for all dimensions. `origin` and `sizes`
/// are similarly expanded if they have a single element.
inline void ExtendRegion(
      Image& image,
      UnsignedArray origin,
      UnsignedArray sizes,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( origin.empty() || sizes.empty(), E::ARRAY_PARAMETER_EMPTY );
   dip::uint nDims = image.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( origin, nDims );
      ArrayUseParameter( sizes, nDims );
   DIP_END_STACK_TRACE
   RangeArray ranges( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_THROW_IF( sizes[ ii ] < 1, E::INVALID_PARAMETER );
      ranges[ ii ] = { static_cast< dip::sint >( origin[ ii ] ),
                       static_cast< dip::sint >( origin[ ii ] + sizes[ ii ] - 1 ) };
   }
   ExtendRegion( image, ranges, boundaryCondition );
}


/// \}

} // namespace dip

#endif // DIP_BOUNDARY_H
