/*
 * DIPlib 3.0
 * This file contains declarations for geometric transformations
 *
 * (c)2017-2018, Cris Luengo.
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

#ifndef DIP_GEOMETRY_H
#define DIP_GEOMETRY_H

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Functions for geometric image transformations
/// \see geometry


namespace dip {


/// \defgroup geometry Geometric transformations
/// \brief Geometric image transformations.
///
/// \section interpolation_methods Interpolation methods
///
/// Many of the functions in this group resample the image using an interpolation method. They all perform
/// interpolation separately (i.e. along one dimension at the time). These functions have an input argument
/// called `interpolationMethod`, which determines how image data is interpolated. It can be set to one of
/// the following strings:
///
///  - '"3-cubic"' (or `""`): Third-order cubic spline interpolation (Keys, 1981), using 4 input samples to
///    compute each output sample. This is the default method for most functions.
///
///  - '"4-cubic"': Fourth-order cubic spline interpolation (Keys, 1981), using 6 input samples to compute
///    compute each output sample.
///
///  - '"linear"': Linear interpolation, using 2 input samples to compute each output sample.
///
///  - '"nearest"' (or '"nn"'): Nearest neighbor interpolation, samples are simply shifted and replicated.
///
///  - '"inverse nearest"' (or '"nn2"'): Nearest neighbor interpolation, but resolves the rounding of x.5 in
///    the opposite direction that `"nearest"` does. This is useful when applying the inverse of an earlier
///    transform, to be able to obtain the original geometry back.
///
///  - '"bspline"': A third-order cardinal B-spline is computed for all samples on an image line, which is
///    sampled anew to obtain the interpolated sample values. All input samples are used to compute all
///    output samples, but only about 10 input samples significantly influence each output value.
///
///  - '"lanczos8"': Lanczos interpolation with *a* = 8, using 16 input samples to compute each output sample.
///    The Lanczos kernel is a sinc function windowed by a larger sinc function, where *a* is the width of
///    the larger sinc function. The kernel is normalized.
///
///  - '"lanczos6"': Lanczos interpolation with *a* = 6, using 12 input samples to compute each output sample.
///
///  - '"lanczos4"': Lanczos interpolation with *a* = 4, using 8 input samples to compute each output sample.
///
///  - '"lanczos3"': Lanczos interpolation with *a* = 3, using 6 input samples to compute each output sample.
///
///  - '"lanczos2"': Lanczos interpolation with *a* = 2, using 4 input samples to compute each output sample.
///
///  - '"ft"': Interpolation through padding and cropping, and/or modifying the phase component of the
///    Fourier transform of the image line. Padding with zeros increases the sampling density, cropping
///    reduces the sampling density, and multiplying the phase component by \f$-j s \omega\f$ shifts
///    the image. Equivalent to interpolation with a sinc kernel. All input samples are used to compute
///    all output samples. The boundary condition is ignored, as the Fourier transform imposes a periodic
///    boundary condition.
///
/// Not all methods are available for all functions. If so, this is described in the function's documentation.
/// For operations on binary images, the interpolation method is ignored, and `"nearest"` is always used.
///
/// Interpolation methods require a boundary extension of half the number of input samples used to compute
/// each output sample. For B-spline interpolation, a boundary extension of 5 is used. For the nearest neighbor
/// and Fourier interpolation methods, no boundary extension is needed.
///
/// **Literature**
/// - R.G. Keys, "Cubic Convolution Interpolation for Digital Image Processing", IEEE Transactions on
///   Acoustics, Speech, and Signal Processing 29(6):1153-1160, 1981.
/// \{

/// \brief Shifts the input image by an integer number of pixels, wrapping the pixels around.
///
/// `%dip::Wrap` is equivalent to `dip::Shift` with nearest neighbor interpolation and periodic
/// boundary condition, but faster.
DIP_EXPORT void Wrap(
      Image const& in,
      Image& out,
      IntegerArray wrap
);
inline Image Wrap(
      Image const& in,
      IntegerArray const& wrap
) {
   Image out;
   Wrap( in, out, wrap );
   return out;
}


/// \brief Subsamples the input image.
///
/// The input image is subsampled by `sample[ ii ]` along dimension `ii`. The output image shares
/// the data segment of the input image, meaning that no data is copied. If a data copy is required,
/// calling `dip::Image::ForceContiguousData` after subsampling should trigger a data copy.
inline void Subsampling(
      Image const& in,
      Image& out,
      UnsignedArray const& sample
) {
   DefineROI( in, out, {}, {}, sample );
}
inline Image Subsampling(
      Image const& in,
      UnsignedArray const& sample
) {
   Image out;
   Subsampling( in, out, sample );
   return out;
}


/// \brief Resamples an image with the given zoom factor and sub-pixel shift.
///
/// The shift is applied first, and causes part of the image to shift out of the field of view.
/// Thus, `shift` is in input pixels. `boundaryCondition` determines how the new areas are filled in.
/// See `dip::BoundaryCondition`. Note that the shift can be in fractional pixels. There is no largest
/// possible shift, but applying very large shifts is not optimized for, and will use more computation
/// and temporary memory than necessary. This is true even for the periodic boundary condition; use
/// `dip::Wrap` to apply the integer shift with periodic boundary condition, then use `%Resampling` for
/// the remaining sub-pixel shift. The exception is the `"ft"` interpolation method, which uses the same
/// memory and time for large as for small shifts.
///
/// The scaling is applied next. If `zoom` is larger than 1, the output image will be larger than
/// the input, if it is smaller than 1, it will be smaller. The output image has size
/// `std::floor( in.Sizes( ii ) * zoom[ ii ] )` along dimension `ii`. For the `"ft"` method, the zoom
/// factor is back-computed from this output image size, whereas for the other methods the `zoom` is used
/// as given. This stems from the very different approach to interpolation.
///
/// The pixel at coordinates `pos` in the output image is interpolated from the position
/// `pos[ ii ] / zoom[ ii ] - shift[ ii ]` along dimension `ii`. Thus, with `zoom` smaller
/// than 1, no low-pass filtering is applied first. Again, the `"ft"` interpolation method is different:
/// the output image is generated by inverse Fourier transform from the manipulated frequency spectrum
/// of the input image. The `"ft"` method thus has low-pass filtering built-in when zooming out.
///
/// The output image has the same data type as the input image.
///
/// See \ref interpolation_methods for information on the `interpolationMethod` parameter.
///
/// **Note** that the current implementation doesn't handle the "asym" boundary conditions properly.
/// For unsigned types, resulting samples outside the original image domain are clamped to 0, instead
/// of properly using the saturated inversion.
DIP_EXPORT void Resampling(
      Image const& in,
      Image& out,
      FloatArray zoom,
      FloatArray shift = { 0.0 },
      String const& interpolationMethod = "",
      StringArray const& boundaryCondition = {}
);
inline Image Resampling(
      Image const& in,
      FloatArray const& zoom,
      FloatArray const& shift = { 0.0 },
      String const& interpolationMethod = "",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Resampling( in, out, zoom, shift, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Shift an image. Calls `dip::Resampling` with `zoom` set to 1, and uses the "ft" method by default.
inline void Shift(
      Image const& in,
      Image& out,
      FloatArray const& shift,
      String const& interpolationMethod = S::FOURIER,
      StringArray const& boundaryCondition = {}
) {
   Resampling( in, out, { 1.0 }, shift, interpolationMethod, boundaryCondition );
}
inline Image Shift(
      Image const& in,
      FloatArray const& shift,
      String const& interpolationMethod = S::FOURIER,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Shift( in, out, shift, interpolationMethod, boundaryCondition );
   return out;
}


// Undocumented internal function called by the other forms of Skew.
// Each sub-volume perpendicular to axis is shifted with sub-pixel precision, according to `shearArray`.
// That is, if `axis` is 1, then the sub-volume `in[:,ii,:,:,...]`, with all possible `ii`, is shifted
// by some different amount. No shift happens along the direction `axis`. All image sizes where `shearArray[ii]`
// is non-zero increase except `in.Size(axis)==out.Size(axis)`. `shearArray[axis]` is ignored.
// `origin` indicates which of the sub-volumes perpendicular to `axis` is not shifted. That is, `in[:,origin,:,:,...]`
// is not shifted (or rather, it is shifted by some integer amount to allow negative shifts of other sub-volumes).
// Returns `ret` the location of `in[0,origin,0,0,...]` in the output image. The return array `ret` will always have
// `ret[axis]==origin` since no change happens along that dimension.
DIP_EXPORT dip::UnsignedArray Skew(
      Image const& in,
      Image& out,
      FloatArray const& shearArray, // value along `axis` is ignored
      dip::uint axis,
      dip::uint origin, // where along axis the origin of the skew is
      String const& interpolationMethod = "",
      BoundaryConditionArray boundaryCondition = {} // if it is "periodic", does periodic skew
);

/// \brief Skews an image
///
/// The image is skewed such that a straight line along dimension `axis` is tilted by an angle of
/// `atan( shearArray[ ii ] )` radian in the direction of dimension `ii`. `shearArray[ ii ]` thus represents
/// the sub-pixel shift of a line in the direction `ii` with respect to the previous line along `axis`.
/// Each image sub-volume perpendicular
/// to `axis` is shifted by a different amount. The output image has the same dimension as `in` in the `axis`
/// direction, and larger dimensions in all other dimensions, such that no data is lost. The value of `shearArray[ axis ]`
/// is ignored. The origin of the skew is the origin pixel (as defined in `dip::FourierTransform` and other other
/// places).
///
/// The output image has the same data type as the input image.
///
/// See \ref interpolation_methods for information on the `interpolationMethod` parameter.
///
/// `boundaryCondition` determines how data outside of the input image domain is filled in. See
/// `dip::BoundaryCondition`. If it is `"periodic"`, a periodic skew is applied. This means that
/// image lines are shifted using a periodic boundary condition, and wrap around. The
/// output image does not grow along dimension `skew`.
///
/// **Note** that the current implementation doesn't handle the "asym" boundary conditions properly.
/// For unsigned types, resulting samples outside the original image domain are clamped to 0, instead
/// of properly using the saturated inversion.
///
/// **Note**: The `"ft"` interpolation method is not (yet?) supported.
inline void Skew(
      Image const& in,
      Image& out,
      FloatArray shearArray,
      dip::uint axis,
      String const& interpolationMethod = "",
      StringArray const& boundaryCondition = {}
) {
   DIP_THROW_IF( axis >= in.Dimensionality(), E::ILLEGAL_DIMENSION );
   dip::uint origin = in.Size( axis ) / 2;
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      Skew( in, out, shearArray, axis, origin, interpolationMethod, bc );
   DIP_END_STACK_TRACE
}
inline Image Skew(
      Image const& in,
      FloatArray shearArray, // value along `axis` is ignored
      dip::uint axis,
      String const& interpolationMethod = "",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Skew( in, out, shearArray, axis, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Skews an image
///
/// The image is skewed such that a straight line along dimension `axis` is tilted by an
/// angle of `shear` radian in the direction of dimension `skew`. Each image line along dimension
/// `skew` is shifted by a different amount. The output image has the same dimensions as
/// `in`, except for dimension `skew`, which will be larger, such that no data is lost.
/// The origin of the skew is the origin pixel (as defined in `dip::FourierTransform` and other other
/// places).
///
/// The output image has the same data type as the input image.
///
/// `shear` must have a magnitude smaller than pi/2. Note that the definition of `shear` is different
/// from that of `shearArray` in the other version of `dip::Skew`, documented above.
///
/// See \ref interpolation_methods for information on the `interpolationMethod` parameter.
///
/// `boundaryCondition` determines how data outside of the input image domain is filled in. See
/// `dip::BoundaryCondition`. If it is `"periodic"`, a periodic skew is applied. This means that
/// image lines are shifted using a periodic boundary condition, and wrap around. The
/// output image does not grow along dimension `skew`.
///
/// **Note** that the current implementation doesn't handle the "asym" boundary conditions properly.
/// For unsigned types, resulting samples outside the original image domain are clamped to 0, instead
/// of properly using the saturated inversion.
///
/// **Note**: The `"ft"` interpolation method is not (yet?) supported.
inline void Skew(
      Image const& in,
      Image& out,
      dfloat shear,
      dip::uint skew,
      dip::uint axis,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {} // if it is "periodic", does periodic skew
) {
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( axis == skew, E::INVALID_PARAMETER );
   DIP_THROW_IF(( axis >= nDims ) || ( skew >= nDims ), E::ILLEGAL_DIMENSION );
   DIP_THROW_IF(( shear <= -pi / 2.0 ) | ( shear >= pi / 2.0 ), E::PARAMETER_OUT_OF_RANGE );
   FloatArray shearArray( nDims, 0.0 );
   shearArray[ skew ] = std::tan( shear );
   dip::uint origin = in.Size( axis ) / 2;
   BoundaryCondition bc;
   DIP_START_STACK_TRACE
      bc = StringToBoundaryCondition( boundaryCondition );
   DIP_END_STACK_TRACE
   BoundaryConditionArray bca( 1, bc );
   Skew( in, out, shearArray, axis, origin, interpolationMethod, bca );
}
inline Image Skew(
      Image const& in,
      dfloat shear,
      dip::uint skew,
      dip::uint axis,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {} // if it is "periodic", does periodic skew
) {
   Image out;
   Skew( in, out, shear, skew, axis, interpolationMethod, boundaryCondition );
   return out;
}


/// \brief Rotates an image in one orthogonal plane, over the center of the image.
///
/// Rotates an image in the plane defined by `dimension1` and `dimension2`, over an angle `angle`, in radian.
/// The origin of the rotation is the origin pixel (as defined in `dip::FourierTransform` and other other
/// places).
/// The function implements the rotation in the mathematical sense; **note** the y-axis is positive downwards!
///
/// The output image has the same data type as the input image.
///
/// The rotation is computed by three consecutive calls to `dip::Skew`. See that function for the meaning of
/// `interpolationMethod` and `boundaryCondition`.
///
/// **Note** that the `"periodic"` boundary condition currently produces an output image of the same size as
/// the input, where the corners of the image that rotate out of the field of view are cut off and fill the
/// sections that were outside of the input field of view. This is due to the way that `dip::Skew` handles
/// the `"periodic"` boundary condition. TODO: This is something that we probably want to fix at some point.
///
/// **Note**: The `"ft"` interpolation method is not (yet?) supported.
DIP_EXPORT void Rotation(
      Image const& in,
      Image& out,
      dfloat angle,
      dip::uint dimension1,
      dip::uint dimension2,
      String const& interpolationMethod = "",
      String const& boundaryCondition = { "add zeros" }
);
inline Image Rotation(
      Image const& in,
      dfloat angle,
      dip::uint dimension1,
      dip::uint dimension2,
      String const& interpolationMethod = "",
      String const& boundaryCondition = { "add zeros" }
) {
   Image out;
   Rotation( in, out, angle, dimension1, dimension2, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Rotates a 2D image
///
/// Calls `dip::Rotation`, setting the dimension parameters to 0 and 1. Provides a simplified
/// interface for 2D images.
inline void Rotation2d(
      Image const& in,
      Image& out,
      dfloat angle,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   Rotation( in, out, angle, 0, 1, interpolationMethod, boundaryCondition );
}
inline Image Rotation2d(
      Image const& in,
      dfloat angle,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   Image out;
   Rotation2d( in, out, angle, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Rotates a 3D image in one orthogonal plane
///
/// Calls `dip::Rotation`, setting the dimension parameters according to `axis`. Provides a simplified
/// interface for 3D images.
inline void Rotation3d(
      Image const& in,
      Image& out,
      dfloat angle,
      dip::uint axis = 2,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::uint dim1, dim2;
   switch( axis ) {
      case 0: // x-axis
         dim1 = 1;
         dim2 = 2;
         break;
      case 1: // y-axis
         dim1 = 2;
         dim2 = 0;
         break;
      case 2: // z-axis
         dim1 = 0;
         dim2 = 1;
         break;
      default:
         DIP_THROW( E::PARAMETER_OUT_OF_RANGE );
   }
   Rotation( in, out, angle, dim1, dim2, interpolationMethod, boundaryCondition );
}
inline Image Rotation3d(
      Image const& in,
      dfloat angle,
      dip::uint axis = 2,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   Image out;
   Rotation3d( in, out, angle, axis, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Applies an arbitrary 3D rotation to a 3D image
///
/// Rotates a 3D image over the Euler angles `alpha`, `beta` and `gamma`, by calling `dip::Rotation`
/// three times (i.e. using nine skews).
/// The first rotation is over `alpha` radian around the initial z-axis. The second rotation is over `beta` radian
/// around the intermediate y-axis. The last rotation is over `gamma` radian around the final z-axis.
///
/// The function implements the rotation in the mathematical sense; **note** the y-axis is positive downwards!
///
/// The rotation is over the center of the image.
inline void Rotation3d(
      Image const& in,
      Image& out,
      dfloat alpha,
      dfloat beta,
      dfloat gamma,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   Rotation( in,  out, alpha, 0, 1, interpolationMethod, boundaryCondition );
   Rotation( out, out, beta,  2, 0, interpolationMethod, boundaryCondition );
   Rotation( out, out, gamma, 0, 1, interpolationMethod, boundaryCondition );
}
inline Image Rotation3d(
      Image const& in,
      dfloat alpha,
      dfloat beta,
      dfloat gamma,
      String const& interpolationMethod = "",
      String const& boundaryCondition = {}
) {
   Image out;
   Rotation3d( in, out, alpha, beta, gamma, interpolationMethod, boundaryCondition );
   return out;
}

// TODO: functions to port:
/*
   dip_AffineTransform (dip_interpolation.h) => was never implemented, but we do have affine_trans in DIPimage.
   dip_ResampleAt (dip_interpolation.h)
*/

// TODO: port also rot_euler_low.c and affine_trans_low.c from DIPimage

/// \}

} // namespace dip

#endif // DIP_GEOMETRY_H
