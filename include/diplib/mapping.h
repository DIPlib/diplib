/*
 * DIPlib 3.0
 * This file contains declarations for grey-value mapping functions.
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

#ifndef DIP_MAPPING_H
#define DIP_MAPPING_H

#include "diplib.h"


/// \file
/// \brief Image grey-value mapping functions.
/// \see mapping


namespace dip {


/// \defgroup mapping Grey-value mapping
/// \brief Operators that map image grey values.
/// \{


/// \brief Clips the sample values in `in` to a specified range.
///
/// The input values are written unmodified to `out` if they are within the given range, otherwise the closest
/// value within the range is used. Clipping is also known as clamping or thresholding, though in *DIPlib* we
/// use "thresholding" for the process that yields a binary image.
///
/// The output range is given by `low` and `high`. `mode` can be one of the following strings:
///  - `"both"`: any value lower than `low` is set to `low`, and any value higher than `high` is set to `high`.
///  - `"low"`: only the lower bound is enforced, yields same result as setting `high` to infinity.
///  - `"high"`: only the upper bound is enforced, yields same result as setting `low` to infinity.
///  - `"range"`: `low` is interpreted as the middle of the range, and `high` as the length of the range. The
///    output range is given by [`low-high/2`,`low+high/2`].
///
/// `in` must be real-valued.
DIP_EXPORT void Clip(
      Image const& in,
      Image& out,
      dfloat low = 0.0,
      dfloat high = 255.0,
      String const& mode = "both"
);
inline Image Clip(
      Image const& in,
      dfloat low = 0.0,
      dfloat high = 255.0,
      String const& mode = "both"
) {
   Image out;
   Clip( in, out, low, high, mode );
   return out;
}

/// \brief Clips the sample values in `in` to a specified range, using the error function.
///
/// The input values are mapped through the error function. This leads to values in the middle of the range
/// being unaffected, and values larger than `high` asymptotically reaching 1, and values lower than `low`
/// asymptotically reaching 0. This process is also known as soft thresholding, and leads to a quasi-binary
/// image where the slow transition between foreground and background is preserved, thereby avoiding a most
/// of the aliasing that is introduced by binarization (van Vliet, 1993).
///
/// The range to map is given by `low` and `high`. `mode` can be one of the following strings:
///  - `"both"`: any value lower than `low` is set to `low`, and any value higher than `high` is set to `high`.
///  - `"low"`: only the lower bound is enforced, but the value of `high` still affects the mapping.
///  - `"high"`: only the upper bound is enforced, but the value of `low` still affects the mapping.
///  - `"range"`: `low` is interpreted as the middle of the range, and `high` as the length of the range. The
///    input range is given by [`low-high/2`, `low+high/2`]. Note that this is the default mode.
///
/// `in` must be real-valued.
///
/// **Literature**
/// - L.J. van Vliet, "Grey-Scale Measurements in Multi-Dimensional Digitized Images", Ph.D. thesis, Delft University
///   of Technology, The Netherlands, 1993
DIP_EXPORT void ErfClip(
      Image const& in,
      Image& out,
      dfloat low = 128.0,
      dfloat high = 64.0,
      String const& mode = "range"
);
inline Image ErfClip(
      Image const& in,
      dfloat low = 128.0,
      dfloat high = 64.0,
      String const& mode = "range"
) {
   Image out;
   ErfClip( in, out, low, high, mode );
   return out;
}

/// \brief Applies a mapping function according to the input image's range and the given output range.
///
/// The mapping function is defined as follows: sample values higher or equal to `upperBound` are mapped
/// to the `outMax` value, and sample values lower or equal to `lowerBound` are mapped to the `outMin` value.
/// `method` determines how pixel values are mapped in between these limits. Valid strings for `mode` are:
///  - `"linear"`: linear mapping.
///  - `"signed linear"`: linear mapping with zero at fixed value in the middle of the output range.
///  - `"logarithmic"`: logarithmic mapping.
///  - `"signed logarithmic"`: logarithmic mapping with zero at fixed location in the output range.
///  - `"erf"`: error function mapping.
///  - `"decade"`: decade contrast stretch (uses `parameter1`).
///  - `"sigmoid"`: sigmoid function contrast stretch (uses `parameter1` and `parameter2`).
///
/// `in` must be real-valued. `out` will be of an arithmetic type (single or double float), unless it is protected,
/// in which case its data type is preserved (see \ref protect).
///
/// Below follow the equations used for each of the mappings. They all use (with the percentile computed across all
/// samples, not independently for each channel):
/// ```cpp
///     dfloat inMin = dip::Percentile( in, {}, lowerBound ).As< dfloat >();
///     dfloat inMax = dip::Percentile( in, {}, upperBound ).As< dfloat >();
///     in = dip::Clip( in, inMin, inMax );
/// ```
///
/// Next, `"linear"` computes `(( outMax - outMin ) / ( inMax - inMin )) * ( in - inMin ) + outMin`.
///
/// `"signed linear"` computes the same thing, but first sets `inMax = std::max( std::abs(inMax), std::abs(inMin) )`,
/// and `inMin = -inMax`.
///
/// `"logarithmic"` computes:
/// ```cpp
///     dfloat offset = inMin - 1;
///     out = ( outMax - outMin ) * Log( in - offset ) / std::log( inMax - offset ) + outMin;
/// ```
/// whereas `"signed logarithmic"` computes a similar mapping, but first sets
/// `inMax = std::max( std::abs(inMax), std::abs(inMin) )`, and `inMin = -inMax`, and then takes the logarithm
/// of `in+1` for positive `in` or `inMax+in+1` for negative `in`.
///
/// `"erf"` applies a mapping identical to that of the `dip::ErfClip` function with the lower range bound set to
/// `inMin` and the upper one set to `inMax`, and then scales the output to the requested range. Note that in this
/// case, the input is not hard-clipped to the range, but soft-clipped through the error function.
///
/// `"decade"` applies the following mapping to each sample:
/// ```cpp
///     dfloat decade = std::log10(( inMax - inMin ) / ( in - inMin + EPSILON ));
///     if( decade < parameter1 )
///        out = ( outMax - outMin ) * ( 1 + std::floor(decade) - decade ) + outMin;
///     else
///        out = 0;
/// ```
///
/// `"sigmoid"` applies the following mapping to each sample:
/// ```cpp
///     dfloat min = sigmoid( parameter1 * inMin + parameter2 );
///     dfloat max = sigmoid( parameter1 * inMax + parameter2 );
///     out = ( outMax - outMin ) / ( max - min ) * ( sigmoid( parameter1 * in + parameter2 ) - min ) + outMin;
/// ```
/// Here, the `sigmoid` function is `sigmoid(x) = x / ( 1 + std::abs(x) )`. `parameter1` represents the slope and
/// `parameter2` the point around which the sigmoid is centered.
DIP_EXPORT void ContrastStretch(
      Image const& in,
      Image& out,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 100.0,
      dfloat outMin = 0.0,
      dfloat outMax = 255.0,
      String const& method = "linear",
      dfloat parameter1 = 1.0,
      dfloat parameter2 = 0.0
);
inline Image ContrastStretch(
      Image const& in,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 100.0,
      dfloat outMin = 0.0,
      dfloat outMax = 255.0,
      String const& method = "linear",
      dfloat parameter1 = 1.0,
      dfloat parameter2 = 0.0
) {
   Image out;
   ContrastStretch( in, out, lowerBound, upperBound, outMin, outMax, method, parameter1, parameter2 );
   return out;
}


/// \}


} // namespace dip

#endif // DIP_MAPPING_H
