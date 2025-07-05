/*
 * (c)2014-2025, Cris Luengo.
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

#include <cmath>

#include "diplib.h"
#include "diplib/accumulators.h"


/// \file
/// \brief Declares image statistics functions, including projections.
/// See \ref math.


namespace dip {


//
// Basic image queries
//

/// \group math_statistics Statistics
/// \ingroup math
/// \brief Image sample statistics.
///
/// See also \ref math_projection.
/// \addtogroup

#define DIP_DEFINE_VIEW_FUNCTION( name, ret_type ) \
inline ret_type name( Image::View const& in ) { \
   if( in.Offsets().empty() ) { return name( in.Reference(), in.Mask() ); } \
   return name( Image( in ) ); }

/// \brief Counts the number of non-zero pixels in a scalar image.
///
/// `in` must be scalar, but can have any data type. `mask`, if forged, must be of the same sizes as `in`, or be
/// singleton expandable to that size, and must be binary.
///
/// \see dip::Find
DIP_EXPORT dip::uint Count( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( Count, dip::uint )

/// \brief True if any pixel is NaN.
DIP_EXPORT bool ContainsNotANumber( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( ContainsNotANumber, bool )

/// \brief True if any pixel is infinite.
DIP_EXPORT bool ContainsInfinity( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( ContainsInfinity, bool )

/// \brief True if any pixel is non-finite (infinite or NaN).
DIP_EXPORT bool ContainsNonFiniteValue( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( ContainsNonFiniteValue, bool )

/// \brief Returns the coordinates of the maximum pixel in the image.
///
/// The image must be scalar. If `in` is complex, the modulus of its values are used.
/// If `positionFlag` is `"first"`, the first maximum is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// \see dip::Maximum(Image const&, Image const&, Image&, BooleanArray const&), dip::PositionMaximum, dip::MaximumAndMinimum
DIP_EXPORT UnsignedArray MaximumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = S::FIRST );

/// \brief Returns the coordinates of the minimum pixel in the image.
///
/// The image must be scalar. If `in` is complex, the modulus of its values are used.
/// If `positionFlag` is `"first"`, the first minimum is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// \see dip::Minimum(Image const&, Image const&, Image&, BooleanArray const&), dip::PositionMinimum, dip::MaximumAndMinimum
DIP_EXPORT UnsignedArray MinimumPixel( Image const& in, Image const& mask = {}, String const& positionFlag = S::FIRST );

/// \brief Calculates the cumulative sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed. The output is an image of the same size as the input.
/// For tensor images, the output has the same tensor size and shape as the input.
///
/// If `mask` is forged, those pixels not selected by the mask are presumed to be 0.
DIP_EXPORT void CumulativeSum( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_NODISCARD inline Image CumulativeSum( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) {
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
DIP_DEFINE_VIEW_FUNCTION( MaximumAndMinimum, MinMaxAccumulator )

/// \brief Computes the minimum, lower quartile (25th percentile), median (50th percentile),
/// upper quartile (75th percentile), and maximum.
///
/// Percentiles are always one of the values in the image. The nearest value to a given partition
/// is used, rather than interpolate as classically done. NaN values are ignored.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. In case of a complex
/// samples, treats real and imaginary components as individual samples.
DIP_EXPORT QuartilesResult Quartiles( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( Quartiles, QuartilesResult )

/// \brief Computes the first four central moments of the pixel intensities, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the statistics over all sample values. The image must be real-valued.
DIP_EXPORT StatisticsAccumulator SampleStatistics( Image const& in, Image const& mask = {} );
DIP_DEFINE_VIEW_FUNCTION( SampleStatistics, StatisticsAccumulator )

/// \brief Computes the covariance and correlation between the two images, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of tensor
/// images, returns the covariance over all sample values. The images must be real-valued and
/// have the same number of tensor elements.
///
/// To compute the covariance or correlation between two channels in a multi-channel image (a tensor image):
///
/// ```cpp
/// Covariance( in[ 0 ], in[ 1 ], mask );
/// ```
DIP_EXPORT CovarianceAccumulator Covariance( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Computes the Pearson correlation coefficient. See \ref dip::Covariance.
inline dfloat PearsonCorrelation( Image const& in1, Image const& in2, Image const& mask = {} ) {
   return Covariance( in1, in2, mask ).Correlation();
}

/// \brief Computes the Spearman rank correlation coefficient.
///
/// If `mask` is not forged, all input pixels are considered. In case of tensor
/// images, returns the Spearman rank correlation coefficient over all sample values.
/// The images must be real-valued and have the same number of tensor elements.
///
/// To compute the Spearman rank correlation coefficient between two channels in a multi-channel image (a tensor image):
///
/// ```cpp
/// SpearmanRankCorrelation( in[ 0 ], in[ 1 ], mask );
/// ```
DIP_EXPORT dfloat SpearmanRankCorrelation( Image const& in1, Image const& in2, Image const& mask = {} );

/// \brief Computes the center of mass (first order moments) of the image `in`, optionally using only
/// those pixels selected by `mask`.
///
/// If `mask` is not forged, all input pixels are considered. `in` must be scalar and real-valued.
DIP_EXPORT FloatArray CenterOfMass( Image const& in, Image const& mask = {} );

/// \brief Computes the first order normalized moments and second order normalized central moments of the image `in`,
/// optionally using only those pixels selected by `mask`.
///
/// If `mask` is not forged, all input pixels are considered. `in` must be scalar and real-valued.
///
/// Note that the normalization makes the moments invariant to scaling the image intensities, but not to
/// spatial scaling. Divide each element of \ref MomentAccumulator::PlainSecondOrder by \ref MomentAccumulator::Sum
/// to obtain a value that is invariant also to spatial scaling. The first two Hu moments of a 2D image are obtained
/// as follows:
/// ```cpp
/// auto moments = dip::Moments( img );
/// auto m0 = moments.Sum();
/// auto m2 = moments.PlainSecondOrder();
/// // scale and translation invariant second order moments:
/// double nu20 = m2[ 0 ] / m0;
/// double nu02 = m2[ 1 ] / m0;
/// double nu11 = m2[ 2 ] / m0;
/// // scale, translation and rotation-invariant values:
/// double hu1 = nu20 + nu02;
/// double hu2 = ( nu20 - nu02 ) * ( nu20 - nu02 ) + 4 * nu11 * nu11;
/// ```
DIP_EXPORT MomentAccumulator Moments( Image const& in, Image const& mask = {} );

#undef DIP_DEFINE_VIEW_FUNCTION

/// \endgroup


//
// The following functions project along one or more (or all) dimensions
//

/// \group math_projection Projection operators
/// \ingroup math
/// \brief Operators that project the image data onto fewer spatial dimensions, computing image statistics.
/// \addtogroup

#define DIP_DEFINE_PROJECTION_FUNCTIONS( name ) \
DIP_NODISCARD inline Image name( Image const& in, Image const& mask = {}, BooleanArray const& process = {} ) { \
   Image out; name( in, mask, out, process ); return out; } \
inline void name( Image::View const& in, Image& out ) { \
   if( in.Offsets().empty() ) { name( in.Reference(), in.Mask(), out ); } \
   else { name( Image( in ), {}, out ); }} \
DIP_NODISCARD inline Image name( Image::View const& in ) { \
   Image out; name( in, out ); return out; }

#define DIP_DEFINE_PROJECTION_FUNCTIONS_WITH_MODE( name, default_mode ) \
DIP_NODISCARD inline Image name( Image const& in, Image const& mask = {}, String const& mode = ( default_mode ), BooleanArray const& process = {} ) { \
   Image out; name( in, mask, out, mode, process ); return out; } \
inline void name( Image::View const& in, Image& out, String const& mode = ( default_mode )) { \
   if( in.Offsets().empty() ) { name( in.Reference(), in.Mask(), out, mode ); } \
   else { name( Image( in ), {}, out, mode ); }} \
DIP_NODISCARD inline Image name( Image::View const& in, String const& mode = ( default_mode )) { \
   Image out; name( in, out, mode ); return out; }

/// \brief Calculates the (arithmetic) mean of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean projection along the processing dimensions. To get the mean value of all pixels in the
/// image:
///
/// ```cpp
/// dip::Mean( img ).As< double >();
/// ```
///
/// If `mode` is `"directional"`, the data in `in` are assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
/// For directional statistics, the input must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Mean( img.At( mask ), mode )` is the same as `dip::Mean( img, mask, mode )`.
DIP_EXPORT void Mean( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS_WITH_MODE( Mean, "" )

/// \brief Calculates the sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum of pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum projection along the processing dimensions. To get the sum of all pixel values in the
/// image:
///
/// ```cpp
/// dip::Sum( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Sum( img.At( mask ))` is the same as `dip::Sum( img, mask )`.
DIP_EXPORT void Sum( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( Sum )

/// \brief Calculates the geometric mean of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the product of pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the product projection along the processing dimensions. To get the product of all pixel values in the
/// image:
///
/// ```cpp
/// dip::GeometricMean( img ).As< double >();
/// ```
///
/// For tensor images, the geometric mean is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::GeometricMean( img.At( mask ))` is the same as `dip::GeometricMean( img, mask )`.
DIP_EXPORT void GeometricMean( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( GeometricMean )

/// \brief Calculates the product of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the product of pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the product projection along the processing dimensions. To get the product of all pixel values in the
/// image:
///
/// ```cpp
/// dip::Product( img ).As< double >();
/// ```
///
/// For tensor images, the product is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Product( img.At( mask ))` is the same as `dip::Product( img, mask )`.
DIP_EXPORT void Product( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( Product )

/// \brief Calculates the mean of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean absolute pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean absolute projection along the processing dimensions. To get the mean absolute value of all pixels in the
/// image:
///
/// ```cpp
/// dip::MeanAbs( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MeanAbs( img.At( mask ))` is the same as `dip::MeanAbs( img, mask )`.
DIP_EXPORT void MeanAbs( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MeanAbs )

/// \brief Calculates the mean of the modulus of the pixel values. Alias to \ref dip::MeanAbs.
inline void MeanModulus( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} ) {
   MeanAbs( in, mask, out, process );
}
DIP_DEFINE_PROJECTION_FUNCTIONS( MeanModulus )

/// \brief Calculates the sum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum absolute pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum absolute projection along the processing dimensions. To get the sum absolute value of all pixels in the
/// image:
///
/// ```cpp
/// dip::SumAbs( img ).As< double >();
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::SumAbs( img.At( mask ))` is the same as `dip::SumAbs( img, mask )`.
DIP_EXPORT void SumAbs( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( SumAbs )

/// \brief Calculates the sum of the modulus of the pixel values. Alias to \ref dip::SumAbs.
inline void SumModulus( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} ) {
   SumAbs( in, mask, out, process );
}
DIP_DEFINE_PROJECTION_FUNCTIONS( SumModulus )

/// \brief Calculates the mean of the square pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean square projection along the processing dimensions. To get the mean square value of all pixels in the
/// image:
///
/// ```cpp
/// dip::MeanSquare( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MeanSquare( img.At( mask ))` is the same as `dip::MeanSquare( img, mask )`.
DIP_EXPORT void MeanSquare( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MeanSquare )

/// \brief Calculates the sum of the square pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum square projection along the processing dimensions. To get the sum square value of all pixels in the
/// image:
///
/// ```cpp
/// dip::SumSquare( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::SumSquare( img.At( mask ))` is the same as `dip::SumSquare( img, mask )`.
DIP_EXPORT void SumSquare( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( SumSquare )

/// \brief Calculates the mean of the square modulus of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean square projection along the processing dimensions. To get the mean square modulus value of all
/// pixels in the image:
///
/// ```cpp
/// dip::MeanSquareModulus( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. If `img` is complex, `out` is of the
/// corresponding floating-point type. For other input data types, this function is identical to \ref dip::MeanSquare.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MeanSquareModulus( img.At( mask ))` is the same as `dip::MeanSquareModulus( img, mask )`.
DIP_EXPORT void MeanSquareModulus( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MeanSquareModulus )

/// \brief Calculates the sum of the square modulus of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the sum square pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the sum square projection along the processing dimensions. To get the sum square modulus value of all
/// pixels in the image:
///
/// ```cpp
/// dip::SumSquareModulus( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. If `img` is complex, `out` is of the
/// corresponding floating-point type. For other input data types, this function is identical to \ref dip::SumSquare.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::SumSquareModulus( img.At( mask ))` is the same as `dip::SumSquareModulus( img, mask )`.
DIP_EXPORT void SumSquareModulus( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( SumSquareModulus )

/// \brief Calculates the variance of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the variance of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the variance projection along the processing dimensions. To get the variance of all pixels in the
/// image:
///
/// ```cpp
/// dip::Variance( img ).As< double >();
/// ```
///
/// If `mode` is `"fast"`, a simplistic method to compute variance is used; this method can result in catastrophic
/// cancellation if the variance is very small with respect to the mean. If `mode` is `"stable"`, a stable algorithm
/// is used that avoids catastrophic cancellation, but is slower (see \ref dip::VarianceAccumulator and
/// \ref dip::FastVarianceAccumulator). For 8 and 16-bit integer images, the fast algorithm is always used.
///
/// If `mode` is `"directional"`, the data in `in` are assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex, and for
/// directional statistics it must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Variance( img.At( mask ), mode )` is the same as `dip::Variance( img, mask, mode )`.
DIP_EXPORT void Variance( Image const& in, Image const& mask, Image& out, String mode = S::FAST, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS_WITH_MODE( Variance, S::FAST )

/// \brief Calculates the standard deviation of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the standard deviation of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the standard deviation projection along the processing dimensions. To get the standard deviation of all pixels in the
/// image:
///
/// ```cpp
/// dip::StandardDeviation( img ).As< double >();
/// ```
///
/// If `mode` is `"fast"`, a simplistic method to compute standard deviation is used; this method can result in
/// catastrophic cancellation if the variance is very small with respect to the mean. If `mode` is `"stable"`, a
/// stable algorithm is used that avoids catastrophic cancellation, but is slower (see \ref dip::VarianceAccumulator
/// and \ref dip::FastVarianceAccumulator). For 8 and 16-bit integer images, the fast algorithm is always used.
///
/// If `mode` is `"directional"`, the data in `in` are assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex, and for
/// directional statistics it must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::StandardDeviation( img.At( mask ), mode )` is the same as `dip::StandardDeviation( img, mask, mode )`.
DIP_EXPORT void StandardDeviation( Image const& in, Image const& mask, Image& out, String mode = S::FAST, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS_WITH_MODE( StandardDeviation, S::FAST )

/// \brief Calculates the maximum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the maximum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the maximum projection along the processing dimensions. To get the maximum of all pixels in the
/// image:
///
/// ```cpp
/// dip::Maximum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// To compute the sample-wise maximum over two or more images, use \ref dip::Supremum.
///
/// An alias is defined such that `dip::Maximum( img.At( mask ))` is the same as `dip::Maximum( img, mask )`.
///
/// \see dip::MaximumAndMinimum, dip::MaximumPixel, dip::PositionMaximum, dip::Supremum
DIP_EXPORT void Maximum( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( Maximum )

/// \brief Calculates the minimum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the minimum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the minimum projection along the processing dimensions. To get the minimum of all pixels in the
/// image:
///
/// ```cpp
/// dip::Minimum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// To compute the sample-wise minimum over two or more images, use \ref dip::Infimum.
///
/// An alias is defined such that `dip::Minimum( img.At( mask ))` is the same as `dip::Minimum( img, mask )`.
///
/// \see dip::MaximumAndMinimum, dip::MinimumPixel, dip::PositionMinimum, dip::Infimum
DIP_EXPORT void Minimum( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( Minimum )

/// \brief Calculates the maximum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the maximum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the maximum projection along the processing dimensions. To get the maximum of all pixels in the
/// image:
///
/// ```cpp
/// dip::Maximum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MaximumAbs( img.At( mask ))` is the same as `dip::MaximumAbs( img, mask )`.
DIP_EXPORT void MaximumAbs( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MaximumAbs )

/// \brief Calculates the minimum of the absolute pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the minimum of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the minimum projection along the processing dimensions. To get the minimum of all pixels in the
/// image:
///
/// ```cpp
/// dip::Minimum( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MinimumAbs( img.At( mask ))` is the same as `dip::MinimumAbs( img, mask )`.
DIP_EXPORT void MinimumAbs( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MinimumAbs )

/// \brief Calculates the percentile of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the `percentile` percentile of the pixel values. Otherwise, the output has as many dimensions as elements
/// in `process` that are `false`, and equals the percentile projection along the processing dimensions.
/// To get the 30^th^ percentile of all pixels in the image:
///
/// ```cpp
/// dip::Percentile( img, {}, 30.0 ).As< double >();
/// ```
///
/// Note that the sample nearest the partition is picked, values are not interpolated if the partition falls in
/// between samples. NaN values are ignored.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Percentile( img.At( mask ))` is the same as `dip::Percentile( img, mask )`.
///
/// \see dip::PositionPercentile
DIP_EXPORT void Percentile( Image const& in, Image const& mask, Image& out, dfloat percentile = 50, BooleanArray const& process = {} );
DIP_NODISCARD inline Image Percentile( Image const& in, Image const& mask = {}, dfloat percentile = 50, BooleanArray const& process = {} ) {
   Image out;
   Percentile( in, mask, out, percentile, process );
   return out;
}
inline void Percentile( Image::View const& in, Image& out, dfloat percentile = 50 ) {
   if( in.Offsets().empty() ) {
      Percentile( in.Reference(), in.Mask(), out, percentile );
   } else {
      Percentile( Image( in ), {}, out, percentile );
   }
}
DIP_NODISCARD inline Image Percentile( Image::View const& in, dfloat percentile = 50 ) {
   Image out;
   Percentile( in, out, percentile );
   return out;
}


/// \brief Calculates the median of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the median (50^th^ percentile) of the pixel values. Otherwise, the output has as many dimensions as elements
/// in `process` that are `false`, and equals the median projection along the processing dimensions. To get the
/// median of all pixels in the image:
///
/// ```cpp
/// dip::Median( img ).As< double >();
/// ```
///
/// Note that the sample nearest the median is picked, values are not interpolated if there is an even number of
/// pixels. NaN values are ignored.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::Median( img.At( mask ))` is the same as `dip::Median( img, mask )`.
///
/// \see dip::PositionMedian, dip::Percentile(Image const&, Image const&, Image&, dfloat, BooleanArray const&)
inline void Median( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} ) {
   Percentile( in, mask, out, 50.0, process );
}
DIP_DEFINE_PROJECTION_FUNCTIONS( Median )

/// \brief Computes the median absolute deviation (MAD)
///
/// The MAD is a measure of statistical dispersion. It can be used as a robust estimate of the standard deviation.
/// For normally distributed data, the standard deviation equals `1.4826 * MAD`. It is computed as if by
///
/// ```cpp
/// dip::Median( dip::Abs( img - dip::Median( img )));
/// ```
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the MAD of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the MAD projection along the processing dimensions. To get the MAD of all pixels in the
/// image:
///
/// ```cpp
/// dip::MedianAbsoluteDeviation( img ).As< double >();
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// An alias is defined such that `dip::MedianAbsoluteDeviation( img.At( mask ))` is the same as `dip::MedianAbsoluteDeviation( img, mask )`.
///
/// \see dip::Median(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void MedianAbsoluteDeviation( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( MedianAbsoluteDeviation )

/// \brief Determines if all pixels have non-zero values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// a boolean value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the "all" projection along the processing dimensions. To test if all the pixels in the image are
/// non-zero:
///
/// ```cpp
/// dip::All( img ).As< bool >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void All( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( All )

/// \brief Determines if any pixel has a non-zero value over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// a boolean value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the "any" projection along the processing dimensions. To test if any pixel in the image is
/// non-zero:
///
/// ```cpp
/// dip::Any( img ).As< bool >();
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
DIP_EXPORT void Any( Image const& in, Image const& mask, Image& out, BooleanArray const& process = {} );
DIP_DEFINE_PROJECTION_FUNCTIONS( Any )

#undef DIP_DEFINE_PROJECTION_FUNCTIONS
#undef DIP_DEFINE_PROJECTION_FUNCTIONS_WITH_MODE

/// \brief Calculates the position of the maximum of the pixel values in a single dimension specified by `dim`.
///
/// The `out` image has size 1 in the `dim` dimension and is equally sized as `in` in the other dimensions.
/// For each image line in the `dim` dimension, the position of the maximum is computed
/// and its `dim`-coordinate is stored in `out` at the coordinates of that image line.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// If `mode` is `"first"`, the first maximum is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// \see dip::PositionMinimum, dip::MaximumPixel, dip::Maximum(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void PositionMaximum( Image const& in, Image const& mask, Image& out, dip::uint dim = 0, String const& mode = S::FIRST );
DIP_NODISCARD inline Image PositionMaximum( Image const& in, Image const& mask = {}, dip::uint dim = 0, String const& mode = S::FIRST ) {
   Image out;
   PositionMaximum( in, mask, out, dim, mode );
   return out;
}

/// \brief Calculates the position of the minimum of the pixel values in a single dimension specified by `dim`.
///
/// The `out` image has size 1 in the `dim` dimension and is equally sized as `in` in the other dimensions.
/// For each image line in the `dim` dimension, the position of the minimum is computed
/// and its `dim`-coordinate is stored in `out` at the coordinates of that image line.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// If `mode` is `"first"`, the first minimum is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// \see dip::PositionMaximum, dip::MinimumPixel, dip::Minimum(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void PositionMinimum( Image const& in, Image const& mask, Image& out, dip::uint dim = 0, String const& mode = S::FIRST );
DIP_NODISCARD inline Image PositionMinimum( Image const& in, Image const& mask = {}, dip::uint dim = 0, String const& mode = S::FIRST ) {
   Image out;
   PositionMinimum( in, mask, out, dim, mode );
   return out;
}

/// \brief Calculates the position of the percentile of the pixel values in a single dimension specified by `dim`.
///
/// The `out` image has size 1 in the `dim` dimension and is equally sized as `in` in the other dimensions.
/// For each image line in the `dim` dimension, the position of the percentile is computed
/// and its `dim`-coordinate is stored in `out` at the coordinates of that image line.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// `percentile` must be between 0.0 and 100.0. NaN values are ignored.
///
/// If `mode` is `"first"`, the first pixel with the percentile value is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// A call to this function with `percentile` set to 0.0 redirects to \ref dip::PositionMinimum and
/// a value of 100.0 redirects to \ref dip::PositionMaximum.
///
/// \see dip::PositionMedian, dip::PositionMinimum, dip::PositionMaximum, dip::Percentile(dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::BooleanArray const&)
DIP_EXPORT void PositionPercentile( Image const& in, Image const& mask, Image& out, dfloat percentile = 50, dip::uint dim = 0, String const& mode = S::FIRST );
DIP_NODISCARD inline Image PositionPercentile( Image const& in, Image const& mask = {}, dfloat percentile = 50, dip::uint dim = 0, String const& mode = S::FIRST ) {
   Image out;
   PositionPercentile( in, mask, out, percentile, dim, mode );
   return out;
}

/// \brief Calculates the position of the median of the pixel values in a single dimension specified by `dim`.
///
/// The `out` image has size 1 in the `dim` dimension and is equally sized as `in` in the other dimensions.
/// For each image line in the `dim` dimension, the position of the median is computed
/// and its `dim`-coordinate is stored in `out` at the coordinates of that image line.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// NaN values are ignored.
///
/// If `mode` is `"first"`, the first pixel with the median value is found, in linear index order.
/// If it is `"last"`, the last one is found.
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// This function redirects to \ref dip::PositionPercentile with `percentile` set to 50.0.
///
/// \see dip::PositionPercentile, dip::Median(Image const&, Image const&, Image&, BooleanArray const&)
inline void PositionMedian( Image const& in, Image const& mask, Image& out, dip::uint dim = 0, String const& mode = S::FIRST ) {
   PositionPercentile( in, mask, out, 50.0, dim, mode );
}
DIP_NODISCARD inline Image PositionMedian( Image const& in, Image const& mask = {}, dip::uint dim = 0, String const& mode = S::FIRST ) {
   Image out;
   PositionMedian( in, mask, out, dim, mode );
   return out;
}

/// \brief Computes the radial projection of the sum of the pixel values of `in`.
///
/// If the radial distance of a pixel to `center` is `r`, then the sum of
/// the intensities of all pixels with `n * binSize <= r < (n + 1) * binSize` is
/// stored at position `n` in the radial dimension of `out`.
///
/// `binSize` sets the size of the bins (pixels) in the radial output dimension.
/// If `maxRadius` is set to `"inner radius"`, the maximum radius that is projected is equal to
/// the smallest distance from the given `center` to any edge pixel of the image. Otherwise, when
/// `maxRadius` is set to `"outer radius"`, the maximum radius is set to largest distance from the given
/// `center` to any edge pixel.
///
/// `center` defaults to `in.GetCenter()`.
///
/// The output data type is \ref DT_DFLOAT for non-complex inputs and \ref DT_DCOMPLEX for complex inputs.
///
/// \see dip::RadialMean, dip::Image::GetCenter, dip::Sum(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void RadialSum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
);
inline Image RadialSum(
      Image const& in,
      Image const& mask = {},
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
) {
   Image out;
   RadialSum( in, mask, out, binSize, maxRadius, center );
   return out;
}

/// \brief Computes the radial projection of the mean of the pixel values of `in`.
///
/// If the radial distance of a pixel to `center` is `r`, then the mean of
/// the intensities of all pixels with `n * binSize <= r < (n + 1) * binSize` is
/// stored at position `n` in the radial dimension of `out`.
///
/// `binSize` sets the size of the bins (pixels) in the radial output dimension.
/// If `maxRadius` is set to `"inner radius"`, the maximum radius that is projected is equal to
/// the smallest distance from the given `center` to any edge pixel of the image. Otherwise, when
/// `maxRadius` is set to `"outer radius"`, the maximum radius is set to largest distance from the given
/// `center` to any edge pixel.
///
/// `center` defaults to `in.GetCenter()`.
///
/// The output data type is \ref DT_DFLOAT for non-complex inputs and \ref DT_DCOMPLEX for complex inputs.
///
/// \see dip::RadialSum, dip::Image::GetCenter, dip::Mean
DIP_EXPORT void RadialMean(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
);
inline Image RadialMean(
      Image const& in,
      Image const& mask = {},
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
) {
   Image out;
   RadialMean( in, mask, out, binSize, maxRadius, center );
   return out;
}

/// \brief Computes the radial projection of the minimum of the pixel values of `in`.
///
/// If the radial distance of a pixel to `center` is `r`, then the minimum of
/// the intensities of all pixels with `n * binSize <= r < (n + 1) * binSize` is
/// stored at position `n` in the radial dimension of `out`.
///
/// `binSize` sets the size of the bins (pixels) in the radial output dimension.
/// If `maxRadius` is set to `"inner radius"`, the maximum radius that is projected is equal to
/// the smallest distance from the given `center` to any edge pixel of the image. Otherwise, when
/// `maxRadius` is set to `"outer radius"`, the maximum radius is set to largest distance from the given
/// `center` to any edge pixel.
///
/// `center` defaults to `in.GetCenter()`.
///
/// The output data type is equal to the input data type.
///
/// \see dip::RadialMaximum, dip::Image::GetCenter, dip::Minimum(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void RadialMinimum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
);
inline Image RadialMinimum(
      Image const& in,
      Image const& mask = {},
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
) {
   Image out;
   RadialMinimum( in, mask, out, binSize, maxRadius, center );
   return out;
}

/// \brief Computes the radial projection of the maximum of the pixel values of `in`.
///
/// If the radial distance of a pixel to `center` is `r`, then the maximum of
/// the intensities of all pixels with `n * binSize <= r < (n + 1) * binSize` is
/// stored at position `n` in the radial dimension of `out`.
///
/// `binSize` sets the size of the bins (pixels) in the radial output dimension.
/// If `maxRadius` is set to `"inner radius"`, the maximum radius that is projected is equal to
/// the smallest distance from the given `center` to any edge pixel of the image. Otherwise, when
/// `maxRadius` is set to `"outer radius"`, the maximum radius is set to largest distance from the given
/// `center` to any edge pixel.
///
/// `center` defaults to `in.GetCenter()`.
///
/// The output data type is equal to the input data type.
///
/// \see dip::RadialMinimum, dip::Image::GetCenter, dip::Maximum(Image const&, Image const&, Image&, BooleanArray const&)
DIP_EXPORT void RadialMaximum(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
);
inline Image RadialMaximum(
      Image const& in,
      Image const& mask = {},
      dfloat binSize = 1,
      String const& maxRadius = S::OUTERRADIUS,
      FloatArray const& center = {}
) {
   Image out;
   RadialMaximum( in, mask, out, binSize, maxRadius, center );
   return out;
}

/// \endgroup


//
// Error measures
//

/// \group math_error Error measures
/// \ingroup math
/// \brief Quantifying the difference between images.
/// \addtogroup

/// \brief Calculates the mean error difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed, use \ref dip::MeanAbsoluteError instead.
DIP_EXPORT dfloat MeanError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the mean square error difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
DIP_EXPORT dfloat MeanSquareError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the root mean square (RMS) error difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
inline dfloat RootMeanSquareError( Image const& in, Image const& reference, Image const& mask = {} ) {
   return std::sqrt( MeanSquareError( in, reference, mask ));
}

/// \brief Calculates the mean absolute error difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MeanAbsoluteError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the maximum absolute error difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MaximumAbsoluteError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the mean relative error difference between corresponding sample values of `in` and `reference`.
///
/// The relative error is defined as
///
/// $$ E_\text{relative}(x,y) = \frac{ 2 |x-y| }{ |x|+|y| } \; . $$
///
/// This definition is symmetric, and avoids issues caused by `reference` having a zero value.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MeanRelativeError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the maximum relative error difference between corresponding sample values of `in` and `reference`.
///
/// The relative error is defined as
///
/// $$ E_\text{relative}(x,y) = \frac{ 2 |x-y| }{ |x|+|y| } \; . $$
///
/// This definition is symmetric, and avoids issues caused by `reference` having a zero value.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat MaximumRelativeError( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the I-divergence between corresponding sample values of `in` and `reference`.
///
/// The I-Divergence is defined as
///
/// $$ \text{I-Divergence} = x \ln(x/y) - (x-y) \; , $$
///
/// and is divided by the number of pixels.
/// It is the -log of a Poisson distribution $p(x,y) = e^{-y} / x! - y^x$ with the stirling approximation for
/// $\ln x!$. For $x=0$, where the stirling approximation would fail, $y$ is returned.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed.
///
/// !!! literature
///     - I. Csiszar, "Why Least Squares and Maximum Entropy? An axiomatic approach to inference for linear inverse problems",
///       The Annals of Statistics 19:2032-2066, 1991.
DIP_EXPORT dfloat IDivergence( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the sum of the product of corresponding sample values of `in` and `reference`.
///
/// The sum of the product of `in` and `reference` corresponds to the value of the cross-correlation function at zero
/// displacement (see \ref dip::CrossCorrelationFT) and is a measure of correlation between the two images.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// Complex input is not allowed.
DIP_EXPORT dfloat InProduct( Image const& in, Image const& reference, Image const& mask = {} );

/// \brief Calculates the `order` norm difference between corresponding sample values of `in` and `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
/// For complex input, uses the modulus of the differences.
DIP_EXPORT dfloat LnNormError( Image const& in, Image const& reference, Image const& mask = {}, dfloat order = 2.0 );

/// \brief Calculates the peak signal-to-noise ratio, in dB.
///
/// PSNR is defined as
///
/// $$ \text{PSNR} = 10 \log_{10} \left( \frac{ p^2 }{ \text{MSE} } \right)
///                = 20 \log_{10} \left( \frac{ p }{ \text{RMSE} } \right) \; , $$
///
/// with the peak signal $p$ given by `peakSignal`, MSE being the mean square error and RMSE the root mean square error.
/// If `peakSignal<=0`, computes the peak signal as the difference between maximum and minimum in `reference`.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// Singleton expansion is applied if the image sizes don't match.
DIP_EXPORT dfloat PSNR( Image const& in, Image const& reference, Image const& mask = {}, dfloat peakSignal = 0.0 );

/// \brief Calculates the structural similarity index (a visual similarity measure)
///
/// Returns the average SSIM, computed locally in a Gaussian window of size `sigma`, using constants
/// `K1` and `K2`. These two constants should be small (<<1) positive values and serve to avoid instabilities.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// The two input images must be real-valued. Singleton expansion is applied if the image sizes don't match.
///
/// !!! literature
///     - Z. Wang, A.C. Bovik, H.R. Sheikh and E.P. Simoncelli, "Image quality assessment: from error visibility to
///       structural similarity", IEEE Transactions on Image Processing 13(4):600-612, 2004.
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

/// \brief Holds return values for the function \ref dip::SpatialOverlap.
struct SpatialOverlapMetrics {
   dfloat truePositives;   ///< Number of true positives
   dfloat trueNegatives;   ///< Number of true negatives
   dfloat falsePositives;  ///< Number of false positives
   dfloat falseNegatives;  ///< Number of false negatives
   dfloat diceCoefficient; ///< The F1-measure, harmonic mean between `precision` (PPV) and `sensitivity` (recall, TPR).
   dfloat jaccardIndex;    ///< The ratio of the intersection to the union.
   dfloat sensitivity;     ///< Also called recall, true positive rate (TPR).
   dfloat specificity;     ///< Also called true negative rate (TNR).
   dfloat fallout;         ///< Also called false positive rate (FPR), equal to 1-specificity.
   dfloat accuracy;        ///< Fraction of correctly segmented pixels.
   dfloat precision;       ///< Also called positive predictive value (PPV).
};
/// \brief Compares a segmentation result `in` to the ground truth `reference`.
///
/// Both `in` and `reference` are expected to be either binary images, or real-valued images in the range [0,1]
/// indicating likelihoods (i.e. a fuzzy segmentation). If images contain values outside that range, the results
/// will be meaningless. Both images must be scalar and of the same sizes.
///
/// If only one measure is of interest, it will be more efficient to use one of the specialized functions:
/// \ref dip::DiceCoefficient, \ref dip::JaccardIndex, \ref dip::Specificity, \ref dip::Sensitivity, \ref dip::Accuracy,
/// or \ref dip::Precision.
DIP_EXPORT SpatialOverlapMetrics SpatialOverlap( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the Dice coefficient.
///
/// The Dice coefficient (also known as Sørensen–Dice coefficient) is defined as twice the area of the intersection
/// of `in` and `reference` divided by the sum of their areas,
///
/// $$ \text{Dice} = \frac{2 |A \cap B|}{|A|+|B|} = \frac{2\text{TP}}{2\text{TP}+\text{FP}+\text{FN}} \; . $$
///
/// The Dice coefficient is equivalent to the harmonic mean between precision and sensitivity or recall (i.e. the
/// F~1~ score):
///
/// ```cpp
/// dfloat dice = dip::DiceCoefficient( a, b );
/// dfloat alsoDice = 2.0 / ( 1.0 / dip::Precision( a, b ) + 1.0 / dip::Sensitivity( a, b ));
/// ```
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
DIP_EXPORT dfloat DiceCoefficient( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the Jaccard index.
///
/// The Jaccard index is defined as the area of the intersection of `in` and `reference` divided by their union,
///
/// $$ \text{Jaccard} = \frac{|A \cap B|}{|A \cup B|} = \frac{\text{TP}}{\text{TP}+\text{FP}+\text{FN}} \; . $$
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
DIP_EXPORT dfloat JaccardIndex( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the specificity of the segmentation.
///
/// Specificity is also referred to as True Negative Rate, and is computed as the ratio of true negatives to the
/// total amount of negatives in the `reference` image,
///
/// $$ \text{specificity} = \frac{|\neg A \cap \neg B|}{|\neg B|} = \frac{\text{TN}}{\text{TN}+\text{FP}} \; . $$
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
DIP_EXPORT dfloat Specificity( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the sensitivity of the segmentation.
///
/// Sensitivity, also referred to as recall or True Positive Rate, is computed as the ratio of the true positives
/// to the total amount of positives in the `reference` image,
///
/// $$ \text{sensitivity} = \frac{|A \cap B|}{|B|} = \frac{\text{TP}}{\text{TP}+\text{FN}} \; . $$
///
/// Note that precision and sensitivity are each others mirror, that is, precision yields the same result as
/// sensitivity with switched input images.
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
DIP_EXPORT dfloat Sensitivity( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the accuracy of the segmentation.
///
/// Accuracy is defined as the ratio of correctly classified pixels to the total number of pixels,
///
/// $$ \text{accuracy} = \frac{|A \cap B| + |\neg A \cap \neg B|}{|A| + |\neg A|} = \frac{\text{TP}+\text{TN}}{\text{TP}+\text{FP}+\text{TN}+\text{FN}} \; . $$
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
DIP_EXPORT dfloat Accuracy( Image const& in, Image const& reference );

/// \brief Compares a segmentation result `in` to the ground truth `reference`, determining the precision of the segmentation.
///
/// Precision, or Positive Predictive Value, is defined as the ratio of the true positives to the total amount of
/// positives in the `in` image,
///
/// $$ \text{precision} = \frac{|A \cap B|}{|A|} = \frac{\text{TP}}{\text{TP}+\text{FP}} \; . $$
///
/// Note that precision and sensitivity are each others mirror, that is, precision yields the same result as
/// sensitivity with switched input images.
///
/// The two input images must have the same sizes, be scalar, and either binary or real-valued. Real-valued inputs
/// will be considered as fuzzy segmentations, and expected to be in the range [0,1].
inline dfloat Precision( Image const& in, Image const& reference ) {
   return Sensitivity( reference, in ); // Note! Reversing the order of the parameters on purpose!
}

/// \brief Computes the Hausdorff distance between two binary images.
///
/// The Hausdorff distance is the largest distance one can find between a point in one set and the nearest point
/// in the other set.
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and binary.
DIP_EXPORT dfloat HausdorffDistance( Image const& in, Image const& reference );

/// \brief Computes the modified Hausdorff distance between two binary images.
///
/// The modified Hausdorff distance is the average distance between a point in one set and the nearest point
/// in the other set. The measure is made symmetric by swapping the two sets and using the largest obtained
/// result.
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and binary.
///
/// !!! literature
///     - M.P. Dubuisson and A.K. Jain, "A modified Hausdoff distance for object matching",
///       Proc. 12^th^ Intl. Conf. on Pattern Recognition, Jerusalem, Israel, pp. 566-568, 1994.
DIP_EXPORT dfloat ModifiedHausdorffDistance( Image const& in, Image const& reference );

/// \brief Computes the sum of minimal distances (SMD) between two binary images.
///
/// The sum of minimal distances is the sum of distances between a point in one set and the nearest point
/// in the other set. The measure is made symmetric by swapping the two sets and averaging the results.
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and binary.
///
/// !!! literature
///     - T. Eiter and H. Mannila, "Distance measures for point sets and their computation", Acta Informatica 34(2):109–133, 1997.
DIP_EXPORT dfloat SumOfMinimalDistances( Image const& in, Image const& reference );

/// \brief Computes the complement weighted sum of minimal distances (CWSMD) between two binary images.
///
/// The complement weighted sum of minimal distances is the weighted sum of distances between a point in the first
/// set and the nearest point in the second set. The weights are given by the distance of the point in the
/// first set to its boundary. The measure is made symmetric by swapping the two sets and summing the results.
///
/// Note that this measure is symmetric, that is, it yields the same result if one switches the two images.
///
/// The two input images must have the same sizes, be scalar, and binary.
///
/// !!! literature
///     - V. Ćurić, J. Lindblad, N. Sladoje, H. Sarve, and G. Borgefors, "A new set distance and its application to shape registration",
///       Pattern Analysis and Applications 17:141-152, 2014.
DIP_EXPORT dfloat ComplementWeightedSumOfMinimalDistances( Image const& in, Image const& reference );

/// \brief Calculates the entropy, in bits, using a histogram with `nBins` bins.
///
/// Optionally the `mask` image can be used to exclude pixels from the calculation by setting the value of
/// these pixels in `mask` to zero.
///
/// The input image must be real-valued and scalar.
///
/// \see dip::Entropy(Histogram const&)
DIP_EXPORT dfloat Entropy( Image const& in, Image const& mask = {}, dip::uint nBins = 256 );
inline dfloat Entropy( Image::View const& in, dip::uint nBins = 256 ) {
   if( in.Offsets().empty() ) {
      return Entropy( in.Reference(), in.Mask(), nBins );
   }
   return Entropy( Image( in ), {}, nBins );
}

/// \brief Estimates the variance of white Gaussian noise in an image.
///
/// The method assumes white (uncorrelated) noise, with a Gaussian distribution and zero mean. It may fail if the
/// image contains complex or fine-grained texture.
///
/// If `mask` is not given, creates a mask that avoids edge regions.
///
/// !!! literature
///     - J. Immerk&aelig;r, "Fast Noise Variance Estimation", Computer Vision and Image Understanding 64(2):300-302, 1996.
DIP_EXPORT dfloat EstimateNoiseVariance( Image const& in, Image const& mask = {} );
inline dfloat EstimateNoiseVariance( Image::View const& in ) {
   if( in.Offsets().empty() ) {
      return EstimateNoiseVariance( in.Reference(), in.Mask() );
   }
   return EstimateNoiseVariance( Image( in ) );
}

/// \endgroup

} // namespace dip

#endif // DIP_STATISTICS_H
