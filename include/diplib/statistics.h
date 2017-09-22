/*
 * DIPlib 3.0
 * This file contains declarations for image statistics functions.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *                                (c)2011, Cris Luengo.
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
/// The image must be scalar. If `in` is complex, the modulus of its values are used.
/// If `positionFlag` is `"first"`, the first maximum is found, in linear index order.
/// If it is `"last"`, the last one is found.
DIP_EXPORT UnsignedArray MaximumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = "first" );

/// \brief Returns the coordinates of the minimum pixel in the image.
///
/// The image must be scalar. If `in` is complex, the modulus of its values are used.
/// If `positionFlag` is `"first"`, the first minimuim is found, in linear index order.
/// If it is `"last"`, the last one is found.
DIP_EXPORT UnsignedArray MinimumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = "first" );

/// \brief Calculates the cumulative sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed. The output is an image of the same size as the input.
/// For tensor images, the output has the same tensor size and shape as the input.
///
/// If `mask` is forged, those pixels not selected by the mask are presumed to be 0.
DIP_EXPORT void CumulativeSum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image CumulativeSum( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
   Image out;
   CumulativeSum( in, mask, out, process );
   return out;
}

/// \brief Finds the largest and smallest value in the image, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. In case of a complex
/// samples, treats real and imaginary components as individual samples.
DIP_EXPORT MinMaxAccumulator MaximumAndMinimum( Image const& in, Image const& mask = {} );

/// \brief Computes the first four central moments of the pixel intensities, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the statistics over all sample values. The image must be real-valued.
DIP_EXPORT StatisticsAccumulator SampleStatistics( Image const& in, Image const& mask = {} );

/// \brief Computes the covariance and correlation between the two images, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of tensor
/// images, returns the covariance over all sample values. The images must be real-valued and
/// have the same number of tensor elements.
///
/// To compute the covariance or correlation between two channels in a multi-channel image (a tensor image):
/// ```cpp
///     Covariance( in[ 0 ], in[ 1 ], mask );
/// ```
DIP_EXPORT CovarianceAccumulator Covariance( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Computes the center of mass (first order moments) of the image `in`, optionally using only
/// those pixels selected by `mask`.
///
/// If `mask` is not forged, all input pixels are considered. `in` must be scalar and real-valued.
DIP_EXPORT FloatArray CenterOfMass( Image const& in, Image const& mask = {} );

/// \brief Computes the first order moments and second order central moments of the image `in`,
/// optionally using only those pixels selected by `mask`.
///
/// If `mask` is not forged, all input pixels are considered. `in` must be scalar and real-valued.
DIP_EXPORT MomentAccumulator Moments( Image const& in, Image const& mask = {} );

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
inline Image Mean( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray const& process = {} ) {
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
inline Image Sum( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image Product( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image MeanAbs( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image SumAbs( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image MeanSquare( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image SumSquare( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image Variance( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray const& process = {} ) {
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
inline Image StandardDeviation( Image const& in, Image const& mask = {}, String const& mode = "", BooleanArray const& process = {} ) {
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
/// \see dip::MaximumAndMinimum
DIP_EXPORT void Maximum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Maximum( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
/// \see dip::MaximumAndMinimum
DIP_EXPORT void Minimum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Minimum( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
/// \see dip::MaximumAndMinimum
DIP_EXPORT void MaximumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MaximumAbs( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
/// \see dip::MaximumAndMinimum
DIP_EXPORT void MinimumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image MinimumAbs( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image Percentile( Image const& in, Image const& mask, dfloat percentile, BooleanArray const& process = {} ) {
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
inline void Median( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} ) {
   Percentile( in, mask, out, 50.0, process );
}
inline Image Median( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image All( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
inline Image Any( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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


//
// Basic image queries
//

/// \defgroup math_error Error operators
/// \brief Quantifying the difference between images.
/// \{

/// \brief Calculates the mean error difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed, use `dip::MeanAbsoluteError` instead.
DIP_EXPORT dfloat MeanError( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the mean square error difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
DIP_EXPORT dfloat MeanSquareError( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the root mean square (RMS) error difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
inline dfloat RootMeanSquareError( Image const& in1, Image const& in2, Image const& mask = {} ) {
   return std::sqrt( MeanSquareError( in1, in2, mask ));
}

/// \brief Calculates the mean absolute error difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MeanAbsoluteError( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the maximum absolute error difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MaximumAbsoluteError( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the I-divergence between corresponding sample values of `in1` and `in2`.
///
/// The I-Divergence is defined as \f$I(x,y) = x \ln(x/y) - (x - y)\f$ and is divided by the number of pixels.
/// It is the -log of a Poisson distribution \f$p(x,y) = e^{-y} / x! - y^x\f$ with the stirling approximation for
/// \f$\ln x!\f$. For *x* = 0, the stirling approximation would fail, *y* is returned.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed.
///
/// **Literature**
/// - I. Csiszar, "Why Least Squares and Maximum Entropy? An axiomatic approach to inference for linear inverse problems",
///   The Annals of Statistics 19:2032-2066, 1991.
DIP_EXPORT dfloat IDivergence( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the sum of the product of corresponding sample values of `in1` and `in2`.
///
/// The sum of the product of `in1` and `in2` corresponds to the value of the cross-correlation function at zero
/// displacement (see `dip::CrossCorrelation`) and is a measure of correlation between the two images.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed.
DIP_EXPORT dfloat InProduct( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Calculates the `order` norm difference between corresponding sample values of `in1` and `in2`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
DIP_EXPORT dfloat LnNormError( Image const& in1, Image const& in2, Image const& mask = {}, dfloat order = 2.0 );

/// \brief Calculates the peak signal-to-noise ratio, in dB.
///
/// If `peakSignal<=0`, computes the peak signal as the difference between maximum and minimum in `reference`.
/// PSNR is defined as `20 * log10( peakSignal / RootMeanSquareError( in, reference, mask ))`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat PSNR( Image const& in, Image const& reference, Image const& mask = {}, dfloat peakSignal = 0.0 );

/// \brief Calculates the structural similarity index (a visual similarity measure)
///
/// Returns the average SSIM, computed locally in a Gausian window of size `sigma`, using constants
/// `K1` and `K2`. These two constants should be small (<<1) positive values and serve to avoid instabilities.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// The two input images must be real-valued. Singleton expansion is applied if the image sizes don't match.
///
/// **Literature**
/// - Z. Wang, A.C. Bovik, H.R. Sheikh and E.P. Simoncelli, "Image quality assessment: from error visibility to
///   structural similarity", IEEE Transactions on Image Processing 13(4):600-612, 2004.
DIP_EXPORT dfloat SSIM( Image const& in, Image const& reference, Image const& mask = {}, dfloat sigma = 1.5, dfloat K1 = 0.01, dfloat K2 = 0.03 );

/// \brief Calculates the mutual information, in bits, using a histogram with `nBins`-by-`nBins` bins.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// The two input images must be real-valued and scalar. Singleton expansion is applied if the image sizes don't match.
///
/// \see dip::MutualInformation(Histogram const&)
DIP_EXPORT dfloat MutualInformation( Image const& in, Image const& reference, Image const& mask = {}, dip::uint nBins = 256 );

/// \brief Calculates the entropy, in bits, using a histogram with `nBins` bins.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// The input image must be real-valued and scalar.
///
/// \see dip::Entropy(Histogram const&)
DIP_EXPORT dfloat Entropy( Image const& in, Image const& mask = {}, dip::uint nBins = 256 );

/// \brief Estimates the variance of white Gaussian noise in an image.
///
/// The method assumes white (uncorrelated) noise, with a Gaussian distribution and zero mean. It may fail if the
/// image contains complex or fine-grained texture.
///
/// If `mask` is not given, creates a mask that avoids edge regions.
///
/// **Literature**
/// - J. Immerk&aelig;r, "Fast Noise Variance Estimation", Computer Vision and Image Understanding 64(2):300-302, 1996.
DIP_EXPORT dfloat EstimateNoiseVariance( Image const& in, Image const& mask = {} );

/// \}

/// \}

} // namespace dip

#endif // DIP_STATISTICS_H
