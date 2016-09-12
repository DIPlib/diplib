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
/// Declares image math and statistics functions, except basic arithmetic and comparison.
/// \see dip_operators.h

namespace dip {

/// Contains the return values for the function dip::GetMaximumAndMinimum.
struct MaximumAndMinimum {
   double min;
   double max;
};

// This function here serves as an example of how to use the scan framework
// to do a multi-threaded reduce operation. It also demonstrates how to handle
// optional mask images.
/// Finds the largest and smallest value in the image, within an optional mask.
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. In case of a complex
/// samples, treats real and imaginary components as individual samples.
MaximumAndMinimum GetMaximumAndMinimum(
   Image in,
   Image mask
);


} // namespace dip

#endif // DIP_MATH_H
