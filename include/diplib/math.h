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
void Mean( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the sum of the pixel values over all those dimensions which are specified by `process`.
void Sum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the product of the pixel values over all those dimensions which are specified by `process`.
void Product( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the cumulative sum of the pixel values over all those dimensions which are specified by `process`.
void CumulativeSum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the mean of the absolute pixel values over all those dimensions which are specified by `process`.
void MeanAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the sum of the absolute pixel values over all those dimensions which are specified by `process`.
void SumAbs( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the mean of the square pixel values over all those dimensions which are specified by `process`.
void MeanSquare( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the variance of the pixel values over all those dimensions which are specified by `process`.
void Variance( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the standard deviation of the pixel values over all those dimensions which are specified by `process`.
void StandardDeviation( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the maximum of the pixel values over all those dimensions which are specified by `process`.
void Maximum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the minimum of the pixel values over all those dimensions which are specified by `process`.
void Minimum( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the median of the pixel values over all those dimensions which are specified by `process`.
void Median( Image const& in, Image const& mask, Image& out, BooleanArray process = {} );

/// \brief Calculates the percentile of the pixel values over all those dimensions which are specified by `process`.
void Percentile( Image const& in, Image const& mask, Image& out, dfloat percentile, BooleanArray process = {} );


//
//
//



/// \}

} // namespace dip

#endif // DIP_MATH_H
