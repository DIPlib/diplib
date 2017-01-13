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


/// \brief Finds the eigenvalues of a 2D symmetric matrix.
///
/// `input` is a pointer to 4 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
void SymmetricEigenValues2D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues of a 2D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 3 values: { xx, xy, yy }.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
inline void SymmetricEigenValues2DPacked( double const* input, double* lambdas ) {
   double matrix[ 4 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   // matrix[ 2 ] is never used
   matrix[ 3 ] = input[ 2 ];
   SymmetricEigenValues2D( matrix, lambdas );
}

/// \brief Finds the eigenvalues and eigenvectors of a 2D symmetric matrix.
///
/// `input` is a pointer to 4 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
/// `v1` and `v2` are pointers to space for 2 values each, and will receive the two eigenvectors.
void SymmetricEigenSystem2D( double const* input, double* lambdas, double* v1, double* v2 );

/// \brief Finds the eigenvalues and eigenvectors of a 2D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 3 values: { xx, xy, yy }.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
/// `v1` and `v2` are pointers to space for 2 values each, and will receive the two eigenvectors.
inline void SymmetricEigenSystem2DPacked( double const* input, double* lambdas, double* v1, double* v2 ) {
   double matrix[ 4 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   // matrix[ 2 ] is never used
   matrix[ 3 ] = input[ 2 ];
   SymmetricEigenSystem2D( matrix, lambdas, v1, v2 );
}


/// \brief Finds the eigenvalues of a 3D symmetric matrix.
///
/// `input` is a pointer to 9 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
void SymmetricEigenValues3D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues of a 3D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 6 values: { xx, xy, xz, yy, yz, zz }.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
inline void SymmetricEigenValues3DPacked( double const* input, double* lambdas ) {
   double matrix[ 9 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   matrix[ 2 ] = input[ 2 ];
   // matrix[ 3 ] is never used
   matrix[ 4 ] = input[ 3 ];
   matrix[ 5 ] = input[ 4 ];
   // matrix[ 6 ] is never used
   // matrix[ 7 ] is never used
   matrix[ 8 ] = input[ 5 ];
   SymmetricEigenValues3D( matrix, lambdas );
}

/// \brief Finds the eigenvalues and eigenvectors of a 3D symmetric matrix.
///
/// `input` is a pointer to 9 values, in column-major order; only the lower triangle will be used.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
/// `v1`, `v2` and `v3` are pointers to space for 3 values each, and will receive the three eigenvectors.
void SymmetricEigenSystem3D( double const* input, double* lambdas, double* v1, double* v2, double* v3 );

/// \brief Finds the eigenvalues and eigenvectors of a 3D symmetric matrix, where only the unique values are given.
///
/// `input` is a pointer to 6 values: { xx, xy, xz, yy, yz, zz }.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
/// `v1`, `v2` and `v3` are pointers to space for 3 values each, and will receive the three eigenvectors.
inline void SymmetricEigenSystem3DPacked( double const* input, double* lambdas, double* v1, double* v2, double* v3 ) {
   double matrix[ 9 ];
   matrix[ 0 ] = input[ 0 ];
   matrix[ 1 ] = input[ 1 ];
   matrix[ 2 ] = input[ 2 ];
   // matrix[ 3 ] is never used
   matrix[ 4 ] = input[ 3 ];
   matrix[ 5 ] = input[ 4 ];
   // matrix[ 6 ] is never used
   // matrix[ 7 ] is never used
   matrix[ 8 ] = input[ 5 ];
   SymmetricEigenSystem3D( matrix, lambdas, v1, v2, v3 );
}


/// \brief Finds the eigenvalues of a 2D matrix.
///
/// `input` is a pointer to 4 values, in column-major order.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
void EigenValues2D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues and eigenvectors of a 2D matrix.
///
/// `input` is a pointer to 4 values, in column-major order.
/// `lambdas` is a pointer to space for 2 values, which will be written sorted largest to smallest.
/// `v1` and `v2` are pointers to space for 2 values each, and will receive the two eigenvectors.
void EigenSystem2D( double const* input, double* lambdas, double* v1, double* v2 );


/// \brief Finds the eigenvalues of a 3D matrix.
///
/// `input` is a pointer to 9 values, in column-major order.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
void EigenValues3D( double const* input, double* lambdas );

/// \brief Finds the eigenvalues and eigenvectors of a 3D matrix.
///
/// `input` is a pointer to 9 values, in column-major order.
/// `lambdas` is a pointer to space for 3 values, which will be written sorted largest to smallest.
/// `v1`, `v2` and `v3` are pointers to space for 3 values each, and will receive the three eigenvectors.
void EigenSystem3D( double const* input, double* lambdas, double* v1, double* v2, double* v3 );


/// \brief Finds the eigenvalues of a square matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
void EigenValues( dip::uint n, double const* input, double* lambdas );

/// \brief Finds the eigenvalues and eigenvectors of a square matrix.
///
/// `input` is a pointer to `n`*`n` values, in column-major order.
/// `lambdas` is a pointer to space for `n` values, which will be written sorted largest to smallest.
/// `vectors` is a pointers to space for `n`*`n` values and will receive the `n` eigenvectors. The eigenvectors
/// can be accessed at `&vectors[ 0 ]`, `&vectors[ n ]`, `&vectors[ 2*n ]`, etc.
void EigenSystem( dip::uint n, double const* input, double* lambdas, double* vectors );


/// \}

} // namespace dip

#endif // DIP_NUMERIC_H
