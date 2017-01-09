/*
 * DIPlib 3.0
 * This file contains numeric algorithms unrelated to images.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_NUMERIC_H
#define DIP_NUMERIC_H

#include <cmath>
#include <algorithm>

#include "diplib/library/types.h"

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#endif


/// \file
/// \brief Numeric algorithms and constants unrelated to images. This file is always included through `diplib.h`.
/// \see numeric


namespace dip {


/// \defgroup numeric Numeric algorithms and constants
/// \ingroup infrastructure
/// \brief Functions and constants to be used in numeric computation, unrelated to images.
///
/// These functions and constants are made available when including `diplib.h`.
/// \{


/// \brief The constant pi.
constexpr double pi = 3.14159265358979323846264338327950288;
// std::acos( -1 ) is the good way of definig pi, but it's not constexpr.

/// \brief Compute the greatest common denominator of two positive integers.
// `std::gcd` will be available in C++17.
inline dip::uint gcd( dip::uint a, dip::uint b ) {
   return b == 0 ? a : gcd( b, a % b );
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::gcd function") {
   DOCTEST_CHECK( gcd( 10, 10 ) == 10 );
   DOCTEST_CHECK( gcd( 10, 5 ) == 5 );
   DOCTEST_CHECK( gcd( 10, 1 ) == 1 );
   DOCTEST_CHECK( gcd( 10, 12 ) == 2 );
   DOCTEST_CHECK( gcd( 10, 15 ) == 5 );
   DOCTEST_CHECK( gcd( 15, 10 ) == 5 );
}

#endif

/// \brief Integer division, return ceil.
inline dip::uint div_ceil( dip::uint lhs, dip::uint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return ( lhs - 1 ) / rhs + 1;
}

/// \brief Integer division, return ceil.
inline dip::sint div_ceil( dip::sint lhs, dip::sint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   if( lhs * rhs < 0 ) {
      return lhs / rhs;
   } else {
      if( lhs < 0 ) {
         return ( lhs + 1 ) / rhs + 1;
      } else {
         return ( lhs - 1 ) / rhs + 1;
      }
   }
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_ceil function") {
   DOCTEST_CHECK( div_ceil( 11l, 11l ) == 1 );
   DOCTEST_CHECK( div_ceil( 11l, 6l ) == 2 );
   DOCTEST_CHECK( div_ceil( 11l, 5l ) == 3 );
   DOCTEST_CHECK( div_ceil( 11l, 4l ) == 3 );
   DOCTEST_CHECK( div_ceil( 11l, 3l ) == 4 );
   DOCTEST_CHECK( div_ceil( -11l, 3l ) == -3 );
   DOCTEST_CHECK( div_ceil( -11l, 4l ) == -2 );
   DOCTEST_CHECK( div_ceil( -11l, 5l ) == -2 );
   DOCTEST_CHECK( div_ceil( -11l, 6l ) == -1 );
   DOCTEST_CHECK( div_ceil( 11l, -3l ) == -3 );
   DOCTEST_CHECK( div_ceil( 11l, -4l ) == -2 );
   DOCTEST_CHECK( div_ceil( 11l, -5l ) == -2 );
   DOCTEST_CHECK( div_ceil( 11l, -6l ) == -1 );
   DOCTEST_CHECK( div_ceil( -11l, -6l ) == 2 );
   DOCTEST_CHECK( div_ceil( -11l, -5l ) == 3 );
   DOCTEST_CHECK( div_ceil( -11l, -4l ) == 3 );
   DOCTEST_CHECK( div_ceil( -11l, -3l ) == 4 );
}

#endif

/// \brief Integer division, return floor.
inline dip::uint div_floor( dip::uint lhs, dip::uint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   return lhs / rhs;
}

/// \brief Integer division, return floor.
inline dip::sint div_floor( dip::sint lhs, dip::sint rhs ) {
   if( lhs * rhs == 0 ) {
      return 0;
   }
   if( lhs * rhs < 0 ) {
      if( lhs < 0 ) {
         return ( lhs + 1 ) / rhs - 1;
      } else {
         return ( lhs - 1 ) / rhs - 1;
      }
   } else {
      return lhs / rhs;
   }
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_floor function") {
   DOCTEST_CHECK( div_floor( 10l, 10l ) == 1 );
   DOCTEST_CHECK( div_floor( 11l, 6l ) == 1 );
   DOCTEST_CHECK( div_floor( 11l, 5l ) == 2 );
   DOCTEST_CHECK( div_floor( 11l, 4l ) == 2 );
   DOCTEST_CHECK( div_floor( 11l, 3l ) == 3 );
   DOCTEST_CHECK( div_floor( -11l, 3l ) == -4 );
   DOCTEST_CHECK( div_floor( -11l, 4l ) == -3 );
   DOCTEST_CHECK( div_floor( -11l, 5l ) == -3 );
   DOCTEST_CHECK( div_floor( -11l, 6l ) == -2 );
   DOCTEST_CHECK( div_floor( 11l, -3l ) == -4 );
   DOCTEST_CHECK( div_floor( 11l, -4l ) == -3 );
   DOCTEST_CHECK( div_floor( 11l, -5l ) == -3 );
   DOCTEST_CHECK( div_floor( 11l, -6l ) == -2 );
   DOCTEST_CHECK( div_floor( -11l, -6l ) == 1 );
   DOCTEST_CHECK( div_floor( -11l, -5l ) == 2 );
   DOCTEST_CHECK( div_floor( -11l, -4l ) == 2 );
   DOCTEST_CHECK( div_floor( -11l, -3l ) == 3 );
}

#endif

/// \brief Integer division, return rounded.
inline dip::uint div_round( dip::uint lhs, dip::uint rhs ) {
   return div_floor( lhs + rhs / 2, rhs );
}

/// \brief Integer division, return rounded.
inline dip::sint div_round( dip::sint lhs, dip::sint rhs ) {
   return div_floor( lhs + rhs / 2, rhs );
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_round function") {
   DOCTEST_CHECK( div_round( 10l, 10l ) == 1 );
   DOCTEST_CHECK( div_round( 11l, 6l ) == 2 );
   DOCTEST_CHECK( div_round( 11l, 5l ) == 2 );
   DOCTEST_CHECK( div_round( 11l, 4l ) == 3 );
   DOCTEST_CHECK( div_round( 11l, 3l ) == 4 );
   DOCTEST_CHECK( div_round( -11l, 3l ) == -4 );
   DOCTEST_CHECK( div_round( -11l, 4l ) == -3 );
   DOCTEST_CHECK( div_round( -11l, 5l ) == -2 );
   DOCTEST_CHECK( div_round( -11l, 6l ) == -2 );
   DOCTEST_CHECK( div_round( 11l, -3l ) == -4 );
   DOCTEST_CHECK( div_round( 11l, -4l ) == -3 );
   DOCTEST_CHECK( div_round( 11l, -5l ) == -2 );
   DOCTEST_CHECK( div_round( 11l, -6l ) == -2 );
   DOCTEST_CHECK( div_round( -11l, -6l ) == 2 );
   DOCTEST_CHECK( div_round( -11l, -5l ) == 2 );
   DOCTEST_CHECK( div_round( -11l, -4l ) == 3 );
   DOCTEST_CHECK( div_round( -11l, -3l ) == 4 );
}

#endif

/// \brief Clamps a value between a min and max value (a.k.a. clip, saturate, etc.).
// `std::clamp` will be available in C++17.
template< typename T >
constexpr inline const T& clamp( const T& v, const T& lo, const T& hi ) {
   return std::min( std::max( v, lo ), hi );
}

/// \brief Computes integer powers of 10, assuming the power is relatively small.
inline double pow10( dip::sint power ) {
   switch( power ) {
      case -6: return 1e-6;
      case -5: return 1e-5;
      case -4: return 1e-4;
      case -3: return 1e-3;
      case -2: return 1e-2;
      case -1: return 1e-1;
      case  0: return 1;
      case  1: return 1e1;
      case  2: return 1e2;
      case  3: return 1e3;
      case  4: return 1e4;
      case  5: return 1e5;
      case  6: return 1e6;
      default:
         if( power > 0 ) {
            return 1e6 * pow10( power - 6 );
         } else {
            return 1e-6 * pow10( power + 6 );
         }
   }
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the dip::pow10 function") {
   DOCTEST_CHECK( pow10( 25 ) == doctest::Approx( std::pow( 10, 25 )) );
   DOCTEST_CHECK( pow10( 10 ) == std::pow( 10, 10 ) );
   DOCTEST_CHECK( pow10( 1 ) == std::pow( 10, 1 ) );
   DOCTEST_CHECK( pow10( 0 ) == std::pow( 10, 0 ) );
   DOCTEST_CHECK( pow10( -5 ) == std::pow( 10, -5 ) );
   DOCTEST_CHECK( pow10( -21 ) == doctest::Approx( std::pow( 10, -21 )) );
}

#endif

/// \}

} // namespace dip

#endif // DIP_NUMERIC_H
