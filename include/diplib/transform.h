/*
 * DIPlib 3.0
 * This file contains declarations for the Fourier and other transforms
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

#ifndef DIP_TRANSFORM_H
#define DIP_TRANSFORM_H

#include "diplib.h"


/// \file
/// \brief Declares functions that implement the Fourier and other transforms.


namespace dip {


/// \defgroup transform Transforms
/// \brief Functions that implement the Fourier and other transforms
/// \{


/// \brief Computes the forward and inverse Fourier Transform
///
/// The Fourier transform as implemented here places the origin (frequency 0) in the middle of
/// the image. If the image has `N` pixels along a dimension, then the origin will be at pixel `N/2`
/// along that dimension, where `N/2` is the integer division, and hence truncates the result for
/// odd values of `N`. For example, an image of 256 pixels wide will have the origin at pixel 128
/// (right of the center), whereas an image of 255 pixels will have the origin at pixel 127
/// (dead in the middle).
///
/// The Fourier transform as implemented here normalizes by `1/sqrt(size)` for each dimension. Both
/// forward and inverse transform use the same normalization. This is different from the more typical
/// normalization where the forward transform is not normalized and the inverse transform is
/// normalized by `1/size`. The person that originally implemented the DIPlib Fourier Transform likes
/// symmetry.
///
/// This function will compute the Fourier Transform along the dimensions indicated by `process`. If
/// `process` is an empty array, all dimensions will be processed (normal multi-dimensional transform).
///
/// `options` is an array of strings that indicate how the transform is applied:
///   - "forward": compute the forward transform (this is the default, so this option does not need to
///     be given.
///   - "inverse": compute the inverse transform
///   - "real": assumes that the (complex) input is conjugate symmetric, and returns a real-valued
///     result.
///   - "fast": pads the input to a "nice" size, multiple of 2, 3 and 5, which can be processed faster.
///     Note that "fast" causes the output to be interpolated. This is not always a problem
///     when computing convolutions or correlations, but will introduce e.g. edge effects in the result
///     of the convolution.
void FourierTransform(
      Image const& in,
      Image& out,
      StringArray const& options = {},
      BooleanArray process = {}
);
inline Image FourierTransform(
      Image const& in,
      StringArray const& options = {},
      BooleanArray const& process = {}
) {
   Image out;
   FourierTransform( in, out, options, process );
   return out;
}


/// \}

} // namespace dip

#endif // DIP_TRANSFORM_H
