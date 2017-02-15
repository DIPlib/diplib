/*
 * DIPlib 3.0
 * This file contains declarations for image math and statistics functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_MATH_H
#define DIP_MATH_H

#include "diplib.h"


/// \file
/// \brief Declares image math and statistics functions, except basic arithmetic and comparison.
/// \see diplib/library/operators.h, math


namespace dip {


//
// Basic image queries
//


/// \defgroup math Image math and statistics functions
/// \brief The image math and statistics functions, except basic arithmetic and comparison, which are in module \ref operators.
/// \{


/// \brief Counts the number of non-zero pixels in a scalar image.
dip::uint Count( Image const& in, Image const& mask = {} );


/// \brief Calculates the cumulative sum of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed. The output is an image of the same size as the input.
///
/// If `mask` is forged, those pixels not selected by the mask are presumed to be 0.
void CumulativeSum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
MinMaxAccumulator GetMaximumAndMinimum( Image const& in, Image const& mask = {} );

// TODO: We need functions dip::All() dip::Any() that apply to samples within a tensor. This combines with equality: dip::All( a == b ), for a, b tensor images.
// TODO: We need similar functions that apply to all pixels in an image.


//
// The following functions project along one or more (or all) dimensions
//


/// \brief Calculates the mean of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the mean pixel value. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the mean projection along the processing dimensions. To get the mean value of all pixels in the
/// image:
/// ```cpp
///     static_cast< double >( dip::Mean( img ));
/// ```
///
/// If `mode` is `"directional"`, the data in `in` is assumed to be angles, and directional statistics are used.
/// If `in` contains orientations, multiply it by 2 before applying this function, and divide the result by 2.
/// For directional statistics, the input must be floating point.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void Mean( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
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
///     static_cast< double >( dip::Sum( img ));
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void Sum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::Product( img ));
/// ```
///
/// For tensor images, the product is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void Product( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::MeanAbs( img ));
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void MeanAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::SumAbs( img ));
/// ```
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void SumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::MeanSquare( img ));
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void MeanSquare( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::SumSquare( img ));
/// ```
///
/// For tensor images, the result is computed for each element independently.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void SumSquare( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::Variance( img ));
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
void Variance( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
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
///     static_cast< double >( dip::StandardDeviation( img ));
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
void StandardDeviation( Image const& in, Image const& mask, Image& out, String const& mode = "", BooleanArray process = {} );
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
///     static_cast< double >( dip::Maximum( img ));
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
void Maximum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
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
///     static_cast< double >( dip::Minimum( img ));
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
///
/// \see dip::GetMaximumAndMinimum
void Minimum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );
inline Image Minimum( Image const& in, Image const& mask = {}, BooleanArray process = {} ) {
   Image out;
   Minimum( in, mask, out, process );
   return out;
}

/// \brief Calculates the percentile of the pixel values over all those dimensions which are specified by `process`.
///
/// If `process` is an empty array, all dimensions are processed, and a 0D output image is generated containing
/// the `percentile` percentile of the pixel values. Otherwise, the output has as many dimensions as elements in `process` that are `false`,
/// and equals the percentile projection along the processing dimensions. To get the 30th percentile of all pixels in the
/// image:
/// ```cpp
///     static_cast< double >( dip::Percentile( img, {}, 30.0 ));
/// ```
///
/// For tensor images, the result is computed for each element independently. Input must be not complex.
///
/// If `mask` is forged, only those pixels selected by the mask image are used.
void Percentile( Image const& in, Image const& mask, Image& out, dfloat percentile, BooleanArray process = {} );
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
///     static_cast< double >( dip::Median( img ));
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


//
//
//



/// \}

} // namespace dip

#endif // DIP_MATH_H
