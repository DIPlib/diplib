/*
 * DIPlib 3.0
 * This file contains declarations for the Fourier and other transforms
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

#ifndef DIP_TRANSFORM_H
#define DIP_TRANSFORM_H

#include "diplib.h"


/// \file
/// \brief The Fourier and other transforms.
/// \see transform


namespace dip {


/// \defgroup transform Transforms
/// \brief The Fourier and other transforms.
/// \{


/// \brief Computes the forward and inverse Fourier Transform
///
/// The Fourier transform as implemented here places the origin (frequency 0) in the middle of
/// the image. If the image has `N` pixels along a dimension, then the origin will be at pixel `N/2`
/// along that dimension, where `N/2` is the integer division, and hence truncates the result for
/// odd values of `N`. For example, an image of 256 pixels wide will have the origin at pixel 128
/// (right of the center), whereas an image of 255 pixels will have the origin at pixel 127
/// (dead in the middle). The same is true for the spatial domain, which is only obvious when
/// computing the Fourier transform of a convolution kernel.
///
/// As it is commonly defined, the Fourier transform is not normalized, and the inverse transform
/// is normalized by `1/size` for each dimension. This normalization is necessary for the sequence of
/// forward and inverse transform to be idempotent. However, it is possible to change where the
/// normalization is applied. For example, *DIPlib 2* used identical
/// normalization for each of the two transforms. The advantage of using the common
/// definition without normalization in the forward transform is that it is straightforward to
/// transform an image and a convolution kernel, multiply them, and apply the inverse transform, as
/// an efficient way to compute the convolution. With any other normalization, this process would
/// require an extra multiplication by a constant to undo the normalization in the forward transform
/// of the convolution kernel.
///
/// This function will compute the Fourier Transform along the dimensions indicated by `process`. If
/// `process` is an empty array, all dimensions will be processed (normal multi-dimensional transform).
///
/// `options` is an set of strings that indicate how the transform is applied:
///   - "inverse": compute the inverse transform; not providing this string causes the the forward
///     transform to be computed.
///   - "real": assumes that the (complex) input is conjugate symmetric, and returns a real-valued
///     result.
///   - "fast": pads the input to a "nice" size, multiple of 2, 3 and 5, which can be processed faster.
///     Note that "fast" causes the output to be interpolated. This is not always a problem
///     when computing convolutions or correlations, but will introduce e.g. edge effects in the result
///     of the convolution.
///   - "corner": sets the origin to the top-left corner of the image (both in the spatial and the
///     frequency domain). This yields a standard DFT (Discrete Fourier Transform).
///   - "symmetric": the normalization is made symmetric, where both forward and inverse transforms
///     are normalized by the same amount. Each transform is multiplied by `1/sqrt(size)` for each
///     dimension. This makes the transform identical to how it was in *DIPlib 2*.
///
/// For tensor images, each plane is transformed independently.
///
/// **Known Limitation:** the largest size that can be transformed is 2^31-1. In DIPlib, image sizes are
/// represented by a `dip::uint`, which on a 64-bit system can hold values up to 2^64-1. But this function
/// uses `int` internally to represent sizes, and therefore has a more strict limit to image sizes. Note
/// that this limit refers to the size of one image dimension, not to the total number of pixels in the image.
DIP_EXPORT void FourierTransform(
      Image const& in,
      Image& out,
      StringSet const& options = {},
      BooleanArray process = {}
);
inline Image FourierTransform(
      Image const& in,
      StringSet const& options = {},
      BooleanArray const& process = {}
) {
   Image out;
   FourierTransform( in, out, options, process );
   return out;
}

/// \brief Returns the next higher multiple of {2, 3, 5}. The largest value that can be returned is 2125764000
/// (smaller than 2^31-1, the largest possible value of an `int` on most platforms).
DIP_EXPORT dip::uint OptimalFourierTransformSize( dip::uint size );


/// \brief Computes the Riesz transform of a scalar image.
///
/// The Riesz transform is the multi-dimensional generalization of the Hilbert transform, and identical to it for
/// one-dimensional images. It is computed through the Fourier domain by
///
/// \f[ R_j f = \mathcal{F}^{-1} \left\{ -i\frac{x_j}{|x|}(\mathcal{F}f) \right\} \; , \f]
///
/// where \f$f\f$ is the input image and \f$x\f$ is the coordinate vector.
///
/// `out` is a vector image with one element per image dimension. If `process` is given, it specifies which
/// dimensions to include in the output vector image. `in` must be scalar.
///
/// `inRepresentation` and `outRepresentation` can be `"spatial"` or `"frequency"`, and indicate in which domain
/// the input image is, and in which domain the output image should be.
/// If `inRepresentation` is `"frequency"`, the input image must already be in the frequency domain, and will not
/// be transformed again. Likewise, if `outRepresentation` is `"frequency"`, the output image will not be transformed
/// to the spatial domain. Use these flags to prevent redundant back-and-forth transformations if other processing
/// in the frequency domain is necessary.
DIP_EXPORT void RieszTransform(
      Image const& in,
      Image& out,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      BooleanArray process = {}
);
inline Image RieszTransform(
      Image const& in,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      BooleanArray const& process = {}
) {
   Image out;
   RieszTransform( in, out, inRepresentation, outRepresentation, process );
   return out;
}


/// \}

} // namespace dip

#endif // DIP_TRANSFORM_H
