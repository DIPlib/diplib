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


/// \defgroup math Image math and statistics functions
/// \brief The image math and statistics functions, except basic arithmetic and comparison, which are in module \ref operators.
/// \{

/// \brief Counts the number of non-zero pixels in a scalar image.
dip::uint Count(
      Image const& in
);

/// \brief Contains the return values for the function dip::GetMaximumAndMinimum.
struct MaximumAndMinimum {
   double min;    ///< Maximum sample value
   double max;    ///< Minimum sample value
   MaximumAndMinimum() : min( std::numeric_limits< double >::max() ), max( std::numeric_limits< double >::lowest() ) {}
   void Push( double value ) {
      if( value < min ) {
         min = value;
      }
      if( value > max ) {
         max = value;
      }
   }
};

// This function here serves as an example of how to use the scan framework
// to do a multi-threaded reduce operation. It also demonstrates how to handle
// optional mask images.
/// \brief Finds the largest and smallest value in the image, within an optional mask.
///
/// If `mask` is not forged, all input pixels are considered. In case of a tensor
/// image, returns the maximum and minimum sample values. In case of a complex
/// samples, treats real and imaginary components as individual samples.
MaximumAndMinimum GetMaximumAndMinimum(
      Image const& in,
      Image const& mask
);

MaximumAndMinimum GetMaximumAndMinimum2(
      Image const& in,
      Image const& mask
);

// TODO: We need functions dip::All() dip::Any() that apply to samples within a tensor. This combines with equality: dip::All( a == b ), for a, b tensor images.
// TODO: We need similar functions that apply to all pixels in an image.

/// \}

} // namespace dip

#endif // DIP_MATH_H
