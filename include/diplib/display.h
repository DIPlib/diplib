/*
 * DIPlib 3.0
 * This file contains declarations for the ImageDisplay function
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

#ifndef DIP_DISPLAY_H
#define DIP_DISPLAY_H

#include "diplib.h"


namespace dip {


// TODO: ImageDisplay to be a class
//   - It holds a copy of the image, so we're not dependent on the original image still existing. This is where data
//     is fetched when slice mode, direction or location is changed.
//   - It holds the 2D slice to be displayed (could be either shared data with the image, or in case of a projection,
//     owning its own data). This is where intensity lookup is performed.
//   - It holds another 2D slice, either identical to the previous, or converted to RGB if it was a color image.
//     This prevents repeated (possibly expensive) conversion to RGB when changing mapping modes.
//   - It holds a 2D UINT8 image (output). Must be able to set the external interface! The external interface controls
//     how the color channel dimension is configured.
//   - It holds display flags, and methods to change flags. Changing the flag does not cause immediate update of output.
//       - projection/slice direction
//       - projection/slice mode
//       - complex->real mapping
//       - intensity mapping (linear, logarithmic, based, and limits)
//   - Each of the stored images has a "dirty" flag. Changing display flags causes one or more "dirty" flags to be set.
//   - It holds information about the image:
//       - max and min intensity, 5 and 95 percentiles, etc.
//       - per complex mapping mode
//       - for the current slice and for the image as a whole (global stretch)
//       - only stores the stuff that was needed, don't compute things prematurely!
//   - A method to get the output, which causes updates of (intermediate and output) images flagged as dirty.
//   - A method to get input image intensity at a given 2D point (automatically finds corresponding 3D location).
// By sticking all this functionality in DIPlib, and removing it from DIPimage, we'll improve performance, and
// we'll prevent code duplication when we need this functionality for a Python image display or some other display.
//
// Note that for MATLAB it is probably advantageous to use a color map in the display itself, but we could still
// add a color mapping to this class, where output has a color channel
//
// The MATLAB interface will contain a MATLAB handle class with a pointer to the C++ object stored in a MEX-file.
// The MEX-file will be locked in memory. Instead of pointer, the handle class could have an ID, which is used
// in a std::map to get the dip::ImageDisplay object (safer?). Each image display window will store this handle
// object in its UserData.
//
// imagedisplay.mex:
//    handle = imagedisplay(image)
//    imagedisplay(handle, 'mappingmode', 'lin')
//    mode = imagedisplay(handle, 'mappingmode')
//           also: 'globalstretch', 'complexmapping', 'slicemode', 'slicedirection', 'coordinates'
//    out = imagedisplay(handle)
//    value = imagedisplay(handle, coords) % using 2D coords, automatically translated


/// \brief Parameters to the `dip::ImageDisplay` function.
struct DIP_NO_EXPORT ImageDisplayParams {
   String mode; ///< "lin" (for linear), "log" (for logarithmic), "based" (for based at 0, where 0 is anchored at grey value 128.
   String complex; ///< "mag" (for magnitude) or "abs", "phase", "real", "imag".
   String projection; ///< "slice", "max", "mean".
   dfloat lowerBound; ///< grey value to set to 0.
   dfloat upperBound; ///< grey value to set to 255.
};

/// \brief Transform the image to make it suitable for display.
///
/// The image `in` will be transformed from nD to 2D according to `params.projection`, such that original dimension
/// `dim1` becomes the first dimension and `dim2` becomes the second. In the case of "slice" projection, `coordinates`
/// indicates which slice to extract; it gives the coordinates to a pixel that will be visible in the output.
/// `in` must have at least two dimensions. If `in` has exactly two dimensions, `coordinates`, `dim1` and `dim2` are
/// ignored.
///
/// `params.lowerBound` and `params.upperBound` indicate how the grey-values will be stretched to the range of the
/// `dip::DT_UINT8` output data type. In case `in` is complex, it will be converted to real values through
/// `params.complex`. Finally, if `params.mode` is "log", a logarithmic stretching will be applied, instead of linear.
/// With the "based" mode, the lower and upper bounds will be adjusted such that 0 is mapped to middle grey. Note that
/// the upper bound must be stricly greater than the lower bound.
///
/// If `in` has up to three tensor elements, each tensor element will be scaled identically. This will work for RGB
/// images, but the color space is not checked. An exception will be thrown for images with more than three tensor
/// elements.
DIP_EXPORT void ImageDisplay(
      Image const& in,
      Image& out,
      UnsignedArray const& coordinates,
      dip::uint dim1,
      dip::uint dim2,
      ImageDisplayParams const& params
);

inline Image ImageDisplay (
      Image const& in,
      UnsignedArray const& coordinates,
      dip::uint dim1,
      dip::uint dim2,
      ImageDisplayParams const& params
) {
   Image out;
   ImageDisplay( in, out, coordinates, dim1, dim2, params );
   return out;
}


} // namespace dip

#endif // DIP_DISPLAY_H
