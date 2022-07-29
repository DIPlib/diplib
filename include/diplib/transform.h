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

#ifndef DIP_TRANSFORM_H
#define DIP_TRANSFORM_H

#include "diplib.h"


/// \file
/// \brief The Fourier and related transforms.
/// See \ref transform.


namespace dip {


/// \group transform Transforms
/// \brief The Fourier and other transforms.
/// \addtogroup


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
///
/// - "inverse": compute the inverse transform; not providing this string causes the the forward
///   transform to be computed.
/// - "real": assumes that the (complex) input is conjugate symmetric, and returns a real-valued
///   result. Only to be used together with "inverse".
/// - "fast": pads the input to a "nice" size, multiple of 2, 3 and 5, which can be processed faster.
///   Note that "fast" causes the output to be interpolated. This is not always a problem
///   when computing convolutions or correlations, but will introduce e.g. edge effects in the result
///   of the convolution.
/// - "corner": sets the origin to the top-left corner of the image (both in the spatial and the
///   frequency domain). This yields a standard DFT (Discrete Fourier Transform).
/// - "symmetric": the normalization is made symmetric, where both forward and inverse transforms
///   are normalized by the same amount. Each transform is multiplied by `1/sqrt(size)` for each
///   dimension. This makes the transform identical to how it was in *DIPlib 2*.
///
/// For tensor images, each plane is transformed independently.
///
/// With the "fast" mode, the input might be padded. If "corner" is given, the padding is to the right.
/// Otherwise it is split evenly on both sides, in such a way that the origin remains in the middle pixel.
/// For the forward transform, the padding applied is the "zero order" boundary condition (see \ref dip::BoundaryCondition).
/// Its effect is similar to padding with zeros, but with reduced edge effects.
/// For the inverse transform, padding is with zeros ("add zeros" boundary condition). However, the combination
/// of "fast", "corner" and "inverse" is not allowed, since padding in that case is non-trivial.
///
/// !!! warning
///     The largest size that can be transformed is given by \ref maximumDFTSize. In *DIPlib*, image sizes are
///     represented by a \ref dip::uint, which on a 64-bit system can hold values up to 2^64^-1. But, depending on
///     which library is used to compute the FFT, this function might use `int` internally to represent sizes, and
///     therefore would be limited to lengths of 2^31^-1. Note that this limit refers to the size of one image
///     dimension, not to the total number of pixels in the image.
DIP_EXPORT void FourierTransform(
      Image const& in,
      Image& out,
      StringSet const& options = {},
      BooleanArray process = {}
);
DIP_NODISCARD inline Image FourierTransform(
      Image const& in,
      StringSet const& options = {},
      BooleanArray process = {}
) {
   Image out;
   FourierTransform( in, out, options, std::move( process ));
   return out;
}

/// \brief Returns the next larger (or smaller) multiple of {2, 3, 5}, an image of this size is more
/// efficient for FFT computations.
///
/// The largest value that can be returned is 2125764000
/// (smaller than 2^31^-1, the largest possible value of an `int` on most platforms).
///
/// By default, `which` is `"larger"`, in which case it returns the next larger value. Set it
/// to `"smaller"` to obtain the next smaller value instead.
///
/// Pad an image with zeros to the next larger size or crop the image to the next smaller size to
/// improve FFT performance.
DIP_EXPORT dip::uint OptimalFourierTransformSize( dip::uint size, dip::String const& which = "larger" );


/// \brief Computes the Riesz transform of a scalar image.
///
/// The Riesz transform is the multi-dimensional generalization of the Hilbert transform, and identical to it for
/// one-dimensional images. It is computed through the Fourier domain by
///
/// $$ R_j f = \mathcal{F}^{-1} \left\{ -i\frac{x_j}{|x|}(\mathcal{F}f) \right\} \; , $$
///
/// where $f$ is the input image and $x$ is the coordinate vector.
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
DIP_NODISCARD inline Image RieszTransform(
      Image const& in,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      BooleanArray process = {}
) {
   Image out;
   RieszTransform( in, out, inRepresentation, outRepresentation, std::move( process ));
   return out;
}


/// \brief Computes a stationary wavelet transform (also called à-trous wavelet decomposition).
///
/// For an *n*-dimensional input image, creates an (*n*+1)-dimensional output image where each
/// slice corresponds to one level of the wavelet transform. The first slice is the lowest level
/// (finest detail), and subsequent slices correspond to increasingly coarser levels. The last
/// slice corresponds to the residue. There are `nLevels + 1` slices in total.
///
/// The filter used to smooth the image for the first level is `[1/16, 1/4, 3/8, 1/4, 1/16]`,
/// applied to each dimension in sequence through \ref dip::SeparableConvolution.
/// For subsequent levels, zeros are inserted into this filter.
///
/// `boundaryCondition` is passed to \ref dip::SeparableConvolution to determine how to extend the
/// input image past its boundary. `process` can be used to exclude some dimensions from the
/// filtering.
///
/// `in` can have any number of dimensions, any number of tensor elements, and any data type.
/// `out` will have the smallest signed data type that can hold all values of `in` (see
/// \ref dip::DataType::SuggestSigned). Note that the first `nLevels` slices will contain negative
/// values, even if `in` is purely positive, as these levels are the difference between two
/// differently smoothed images.
///
/// Summing the output image along its last dimension will yield the input image:
///
/// ```cpp
/// dip::Image img = ...;
/// dip::Image swt = StationaryWaveletTransform( img );
/// dip::BooleanArray process( swt.Dimensionality(), false );
/// process.back() = true;
/// img == Sum( swt, {}, process ).Squeeze();
/// ```
DIP_EXPORT void StationaryWaveletTransform(
      Image const& in,
      Image& out,
      dip::uint nLevels = 4,
      StringArray const& boundaryCondition = {},
      BooleanArray const& process = {}
);
DIP_NODISCARD inline Image StationaryWaveletTransform(
      Image const& in,
      dip::uint nLevels = 4,
      StringArray const& boundaryCondition = {},
      BooleanArray const& process = {}
) {
   Image out;
   StationaryWaveletTransform( in, out, nLevels, boundaryCondition, process );
   return out;
}


/// \brief Computes the Haar wavelet transform or its inverse.
///
/// Recursively splits the image into low-frequency and high-frequency components. Each step splits an *n*-dimensional
/// image into 2^*n*^ smaller blocks, the one in the top-left corner containing the low-frequency components. The
/// low-frequency block is the one recursively split in the next step. The output image has the same sizes as the
/// input image, but is of a floating-point type.
///
/// However, the input must have sizes multiple of 2^`nLevels`^. The image will be padded with zeros for the forward
/// transform if this is not the case. For the inverse transform, an exception will the thrown if the sizes are not
/// as expected.
///
/// `direction` can be `"forward"` or `"inverse"`. Applying a forward transform to any image, and an inverse transform
/// to the result, will yield an image identical to the input image, up to rounding errors, and potentially with some
/// padding to the right and bottom.
///
/// `process` can be used to exclude some dimensions from the filtering.
DIP_EXPORT void HaarWaveletTransform(
      Image const& in,
      Image& out,
      dip::uint nLevels = 4,
      String const& direction = S::FORWARD,
      BooleanArray process = {}
);
DIP_NODISCARD inline Image HaarWaveletTransform(
      Image const& in,
      dip::uint nLevels = 4,
      String const& direction = S::FORWARD,
      BooleanArray process = {}
) {
   Image out;
   HaarWaveletTransform( in, out, nLevels, direction, std::move( process ));
   return out;
}


/// \endgroup

} // namespace dip

#endif // DIP_TRANSFORM_H
