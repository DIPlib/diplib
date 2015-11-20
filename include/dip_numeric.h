/*
 * DIPlib 3.0
 * This file contains numeric algorithms unrelated to images.
 *
 * (c)2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include "dip_types.h"

namespace dip {

/// Compute the greatest common denominator of two positive integers.
dip::uint gcd( dip::uint a, dip::uint b );

} // namespace dip

#endif // DIP_NUMERIC_H
