/*
 * DIPlib 3.0
 * This file contains functionality related to the boundary condition
 *
 * (c)2016-2017, Cris Luengo.
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


/// \addtogroup infrastructure
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
/// `SYMMETRIC_MIRROR`         | "mirror"                 | The data is mirrored, with the value at -1 equal to the value at 0, at -2 equal to at 1, etc.
/// `ASYMMETRIC_MIRROR`        | "asym mirror"            | The data is mirrored and inverted.
/// `PERIODIC`                 | "periodic"               | The data is repeated periodically, with the value at -1 equal to the value of the last pixel.
/// `ASYMMETRIC_PERIODIC`      | "asym periodic"          | The data is repeated periodically and inverted.
/// `ADD_ZEROS`                | "add zeros"              | The boundary is filled with zeros.
/// `ADD_MAX_VALUE`            | "add max"                | The boundary is filled with the max value for the data type.
/// `ADD_MIN_VALUE`            | "add min"                | The boundary is filled with the min value for the data type.
/// `ZERO_ORDER_EXTRAPOLATE`   | "zero order"             | The value at the border is repeated indefinitely.
/// `FIRST_ORDER_EXTRAPOLATE`  | "first order"            | A linear function is defined based on the two values closest to the border.
/// `SECOND_ORDER_EXTRAPOLATE` | "second order"           | A quadratic function is defined based on the two values closest to the border, the function reaches zero at the end of the extended boundary.
/// `THIRD_ORDER_EXTRAPOLATE`  | "third order"            | A cubic function is defined based on the two values closest to the border, the function reaches zero with a zero derivative at the end of the extended boundary.
/// `DEFAULT`                  | "default" or ""          | The default value, currently equal to `SYMMETRIC_MIRROR`.
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
      DEFAULT = SYMMETRIC_MIRROR
};

using BoundaryConditionArray = DimensionArray< BoundaryCondition >; ///< An array to hold boundary conditions.


/// \brief Convert a string to a boundary condition.
inline BoundaryCondition StringToBoundaryCondition( String const& bc ) {
   if( bc.empty() ) { return BoundaryCondition::DEFAULT; }
   else if( bc == "default" ) { return BoundaryCondition::DEFAULT; }
   else if( bc == "mirror" ) { return BoundaryCondition::SYMMETRIC_MIRROR; }
   else if( bc == "asym mirror" ) { return BoundaryCondition::ASYMMETRIC_MIRROR; }
   else if( bc == "periodic" ) { return BoundaryCondition::PERIODIC; }
   else if( bc == "asym periodic" ) { return BoundaryCondition::ASYMMETRIC_PERIODIC; }
   else if( bc == "add zeros" ) { return BoundaryCondition::ADD_ZEROS; }
   else if( bc == "add max" ) { return BoundaryCondition::ADD_MAX_VALUE; }
   else if( bc == "add min" ) { return BoundaryCondition::ADD_MIN_VALUE; }
   else if( bc == "zero order" ) { return BoundaryCondition::ZERO_ORDER_EXTRAPOLATE; }
   else if( bc == "first order" ) { return BoundaryCondition::FIRST_ORDER_EXTRAPOLATE; }
   else if( bc == "second order" ) { return BoundaryCondition::SECOND_ORDER_EXTRAPOLATE; }
   else if( bc == "third order" ) { return BoundaryCondition::THIRD_ORDER_EXTRAPOLATE; }
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
/// \class dip::Option::ExtendImage
/// \brief Determines which properties to compare.
///
/// Valid values are:
///
/// `%ExtendImage` constant        | Definition
/// ------------------------------ | ----------
/// `ExtendImage_Masked`           | The output image is a window on the boundary-extended image of the same size as the input.
/// `ExtendImage_ExpandTensor`     | The output image has normal tensor storage.
/// `ExtendImage_FillBoundaryOnly` | Data will not be copied, only the boundary extension will be filled.
///
/// Note that you can add these constants together: `dip::Option::ExtendImage_Masked + dip::Option::ExtendImage_ExpandTensor`.
///
/// `ExtendImage_FillBoundaryOnly` ignores the input image `in`. In this case, if `ExtendImage_Masked` is also given,
/// `out` must be a view of a larger data segment, such as the one that would be produced by this function if
/// `ExtendImage_FillBoundaryOnly` were not given. Otherwise, a boundary of size `borderSizes` is filled using data
/// from further inside the image. If the image is smaller than twice the border size, an exception is thrown.
/// `ExtendImage_ExpandTensor` is also ignored.
DIP_DECLARE_OPTIONS( ExtendImage );
static DIP_DEFINE_OPTION( ExtendImage, ExtendImage_Masked, 0 );
static DIP_DEFINE_OPTION( ExtendImage, ExtendImage_ExpandTensor, 1 );
static DIP_DEFINE_OPTION( ExtendImage, ExtendImage_FillBoundaryOnly, 2 );

} // namespace Option

/// \brief Extends the image `in` by `boundary` along each dimension.
///
/// This function is identical to `dip::ExtendImage`, except it uses boundary condition constants andoption constants
/// instead of strings, and it gives access to the `dip::Option::ExtendImage_FillBoundaryOnly` option, which is not
/// available in the high-level interface. This version is meant to be used by low-level library functions.
DIP_EXPORT void ExtendImageLowLevel(
      Image const& in,
      Image& out,
      UnsignedArray borderSizes,
      BoundaryConditionArray boundaryCondition,
      Option::ExtendImage options
);

/// \brief Extends the image `in` by `boundary` along each dimension.
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
   Option::ExtendImage opts;
   if( options.count( "masked" ) > 0 ) {
      opts += Option::ExtendImage_Masked;
   }
   if( options.count( "expand tensor" ) > 0 ) {
      opts += Option::ExtendImage_ExpandTensor;
   }
   ExtendImageLowLevel( in, out, borderSizes, bc, opts );
}
inline Image ExtendImage(
      Image const& in,
      UnsignedArray const& borderSizes,
      StringArray const& boundaryCondition,
      StringSet const& options = {} ) {
   Image out;
   ExtendImage( in, out, borderSizes, boundaryCondition, options );
   return out;
}

/// \}

} // namespace dip

#endif // DIP_BOUNDARY_H
