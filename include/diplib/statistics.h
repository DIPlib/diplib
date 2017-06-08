/*
 * DIPlib 3.0
 * This file contains declarations for image statistics functions.
 *
 * (c)2014-2016, Cris Luengo.
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

#ifndef DIP_STATISTICS_H
#define DIP_STATISTICS_H

#include "diplib.h"


/// \file
/// \brief Declares image statistics functions, including projections.
/// \see math


namespace dip {


/// \addtogroup math
/// \{


//
// Basic image queries
//

/// \defgroup math_statistics Statistics
/// \brief %Image sample statistics, see also \ref math_projection.
/// \{


/// \brief Counts the number of non-zero pixels in a scalar image.
DIP_EXPORT dip::uint Count( Image const& in, Image const& mask = {} );

/// \brief Returns the coordinates of the maximum pixel in the image.
///
/// The image must be scalar and real-valued. If `positionFlag` is `"first"`, the first
/// maximum is found, in linear index order. If it is `"last"`, the last one is found.
DIP_EXPORT UnsignedArray MaximumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = "first" );

/// \brief Returns the coordinates of the minimum pixel in the image.
///
/// The image must be scalar and real-valued. If `positionFlag` is `"first"`, the first
/// minimum is found, in linear index order. If it is `"last"`, the last one is found.
DIP_EXPORT UnsignedArray MinimumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = "first" );

/// \brief Calculates the cumulative sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed. The output is an image of the same size as the input.
/// For tensor images, the output has the same tensor size and shape as the input.
///
/// If `mask` is forged, those pixels not selected by the mask are presumed to be 0.
DIP_EXPORT void CumulativeSum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image CumulativeSum( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   CumulativeSum( in, mask, out, process );
   return out;
}

/// \brief Finds the largest and smallest value in the image, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. In case of a complex
/// samples, treats real and imaginary components as individual samples.
DIP_EXPORT MinMaxAccumulator GetMaximumAndMinimum( Image const& in, Image const& mask = {} );

/// \brief Computes the first four central moments of the pixel intensities, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. The image must be real-valued.
DIP_EXPORT StatisticsAccumulator GetSampleStatistics( Image const& in, Image const& mask = {} );

// TODO: functions to port:
/*
   dip_Moments (dip_math.h)
   dip_CenterOfMass (dip_math.h)
*/

/// \}


//
// The following functions project along one or more (or all) dimensions
//

/// \defgroup math_projection Projection operators
/// \brief Operators that project the image data onto fewer spatial dimensions, computing image statistics.
/// \{

/// \brief Calculates the mean of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean projection along the processing dimensions. To get the mean value of all pixels in the
/// image:
/// ```cpp
///     dip::Mean( img ).As< double >();
/// ```
///
/// If `mode` is `"directional"`, the data in `in` is assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
/// For directional statistics, the input must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Mean( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
inline Image Mean( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray process = {} ) {
   Image out;
   Mean( in, mask, out, mode, process );
   return out;
}

/// \brief Calculates the sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum of pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum projection along the processing dimensions. To get the sum of all pixel values in the
/// image:
/// ```cpp
///     dip::Sum( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Sum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Sum( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Sum( in, mask, out, process );
   return out;
}

/// \brief Calculates the product of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the product of pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the product projection along the processing dimensions. To get the product of all pixel values in the
/// image:
/// ```cpp
///     dip::Product( img ).As< double >();
/// ```
///
/// For tensor images, the product is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Product( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Product( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Product( in, mask, out, process );
   return out;
}

/// \brief Calculates the mean of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean absolute pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean absolute projection along the processing dimensions. To get the mean absolute value of all pixels in the
/// image:
/// ```cpp
///     dip::MeanAbs( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void MeanAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MeanAbs( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   MeanAbs( in, mask, out, process );
   return out;
}

/// \brief Calculates the sum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum absolute pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum absolute projection along the processing dimensions. To get the sum absolute value of all pixels in the
/// image:
/// ```cpp
///     dip::SumAbs( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void SumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image SumAbs( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   SumAbs( in, mask, out, process );
   return out;
}

/// \brief Calculates the mean of the square pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean square projection along the processing dimensions. To get the mean square value of all pixels in the
/// image:
/// ```cpp
///     dip::MeanSquare( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void MeanSquare( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MeanSquare( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   MeanSquare( in, mask, out, process );
   return out;
}

/// \brief Calculates the mean of the square pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum square projection along the processing dimensions. To get the sum square value of all pixels in the
/// image:
/// ```cpp
///     dip::SumSquare( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void SumSquare( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image SumSquare( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   SumSquare( in, mask, out, process );
   return out;
}

/// \brief Calculates the variance of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the variance of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the variance projection along the processing dimensions. To get the variance of all pixels in the
/// image:
/// ```cpp
///     dip::Variance( img ).As< double >();
/// ```
///
/// If `mode` is `"directional"`, the data in `in` is assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
/// For directional statistics, the input must be real (integer or floating point).
///
/// For tensor images, the result is computed for each element independently. Input must be not complex, and for
/// directional statistics it must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Variance( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
inline Image Variance( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray process = {} ) {
   Image out;
   Variance( in, mask, out, mode, process );
   return out;
}

/// \brief Calculates the standard deviation of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the standard deviation of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the standard deviation projection along the processing dimensions. To get the standard deviation of all pixels in the
/// image:
/// ```cpp
///     dip::StandardDeviation( img ).As< double >();
/// ```
///
/// If `mode` is `"directional"`, the data in `in` is assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
/// For directional statistics, the input must be real (integer or floating point).
///
/// For tensor images, the result is computed for each element independently. Input must be not complex, and for
/// directional statistics it must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void StandardDeviation( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
inline Image StandardDeviation( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray process = {} ) {
   Image out;
   StandardDeviation( in, mask, out, mode, process );
   return out;
}

/// \brief Calculates the maximum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the maximum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the maximum projection along the processing dimensions. To get the maximum of all pixels in the
/// image:
/// ```cpp
///     dip::Maximum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
DIP_EXPORT void Maximum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Maximum( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Maximum( in, mask, out, process );
   return out;
}

/// \brief Calculates the minimum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the minimum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the minimum projection along the processing dimensions. To get the minimum of all pixels in the
/// image:
/// ```cpp
///     dip::Minimum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
DIP_EXPORT void Minimum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Minimum( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Minimum( in, mask, out, process );
   return out;
}

/// \brief Calculates the maximum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the maximum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the maximum projection along the processing dimensions. To get the maximum of all pixels in the
/// image:
/// ```cpp
///     dip::Maximum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
DIP_EXPORT void MaximumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MaximumAbs( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   MaximumAbs( in, mask, out, process );
   return out;
}

/// \brief Calculates the minimum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the minimum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the minimum projection along the processing dimensions. To get the minimum of all pixels in the
/// image:
/// ```cpp
///     dip::Minimum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
DIP_EXPORT void MinimumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MinimumAbs( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   MinimumAbs( in, mask, out, process );
   return out;
}

/// \brief Calculates the percentile of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the `percentile` percentile of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the percentile projection along the processing dimensions. To get the 30th percentile of all pixels in the
/// image:
/// ```cpp
///     dip::Percentile( img, {}, 30.0 ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Percentile( Image const& in, Image const& mask, Image& out, dfloat percentile, BooleanArray process = {} );
inline Image Percentile( Image const& in, Image const& mask, dfloat percentile, BooleanArray process = {} ) {
   Image out;
   Percentile( in, mask, out, percentile, process );
   return out;
}

/// \brief Calculates the median of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the median (50th percentile) of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the median projection along the processing dimensions. To get the median of all pixels in the
/// image:
/// ```cpp
///     dip::Median( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
inline void Median( Image const& in, Image const& mask, Image& out, BooleanArray process = {} ) {
   Percentile( in, mask, out, 50.0, process );
}
inline Image Median( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Median( in, mask, out, process );
   return out;
}

/// \brief Determines if all pixels have non-zero values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// a boolean value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the "all" projection along the processing dimensions. To test if all the pixels in the image are
/// non-zero:
/// ```cpp
///     dip::All( img ).As< bool >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void All( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image All( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   All( in, mask, out, process );
   return out;
}

/// \brief Determines if any pixel has a non-zero value over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// a boolean value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the "any" projection along the processing dimensions. To test if any pixel in the image is
/// non-zero:
/// ```cpp
///     dip::Any( img ).As< bool >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Any( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Any( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   All( in, mask, out, process );
   return out;
}

// TODO: functions to port:
/*
   dip_PositionMaximum (dip_math.h)
   dip_PositionMinimum (dip_math.h)
   dip_PositionMedian (dip_math.h)
   dip_PositionPercentile (dip_math.h)
   dip_RadialMean (dip_math.h)
   dip_RadialSum (dip_math.h)
   dip_RadialMaximum (dip_math.h)
   dip_RadialMinimum (dip_math.h)
*/

/// \}

/// \}

} // namespace dip

#endif // DIP_STATISTICS_H
