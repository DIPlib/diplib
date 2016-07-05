/*
 * DIPlib 3.0
 * This file contains numeric algorithms unrelated to images.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include "dip_types.h"


/// \file
/// Numeric algorithms unrelated to images.


namespace dip {

/// Compute the greatest common denominator of two positive integers.
inline dip::uint gcd( dip::uint a, dip::uint b ) {
   return b == 0 ? a : gcd( b, a % b );
}

/// Integer division, return ceil.
inline dip::uint div_ceil( dip::uint lhs, dip::uint rhs ) {
   if (lhs * rhs == 0) {
      return 0;
   }
   return (lhs - 1) / rhs + 1;
}

/// Integer division, return ceil.
inline dip::sint div_ceil( dip::sint lhs, dip::sint rhs ) {
   if (lhs * rhs == 0) {
      return 0;
   }
   if (lhs * rhs < 0) {
      return lhs / rhs;
   } else {
      if (lhs < 0) {
         return (lhs + 1) / rhs + 1;
      } else {
         return (lhs - 1) / rhs + 1;
      }
   }
}

/// Integer division, return floor.
inline dip::uint div_floor( dip::uint lhs, dip::uint rhs ) {
   if (lhs * rhs == 0) {
      return 0;
   }
   return lhs / rhs;
}

/// Integer division, return floor.
inline dip::sint div_floor( dip::sint lhs, dip::sint rhs ) {
   if (lhs * rhs == 0) {
      return 0;
   }
   if (lhs * rhs < 0) {
      if (lhs < 0) {
         return (lhs + 1) / rhs - 1;
      } else {
         return (lhs - 1) / rhs - 1;
      }
   } else {
      return lhs / rhs;
   }
}

/// Integer division, return rounded.
inline dip::uint div_round( dip::uint lhs, dip::uint rhs ) {
   return div_floor(lhs + rhs/2, rhs);
}

/// Integer division, return rounded.
inline dip::sint div_round( dip::sint lhs, dip::sint rhs ) {
   return div_floor(lhs + rhs/2, rhs);
}

} // namespace dip

#endif // DIP_NUMERIC_H
