/*
 * (c)2017-2021, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#include "diplib/library/numeric.h"

#include <cmath>

#include "diplib.h"

namespace dip {

// NOTE: C++17 introduces std::cyl_bessel_j() and std::cyl_neumann() for dip::BesselJN() and dip::BesselYN().
//       The implementations here are anywhere between 30 and 300 times faster (depending on the input arguments)
//       than these new functions in the C++ std lib (the GCC implementation, on an M1 Mac). But these algorithms
//       here are designed for single-float precision (witness the number of digits). This more than enough precision
//       for the purposes in DIPlib, though.
//
//       Also, Clang (Apple clang version 14.0.0) does not define the Bessel functions.

dfloat BesselJ0(
      dfloat x
) {
   dfloat ax = std::abs( x );
   if( ax < 8.0 ) {
      dfloat y = x * x;
      dfloat ans1 = 57568490574.0 + y * ( -13362590354.0
                                  + y * (    651619640.7
                                  + y * (   -11214424.18
                                  + y * (    77392.33017
                                  + y * (   -184.9052456 )))));
      dfloat ans2 = 57568490411.0 + y * (   1029532985.0
                                  + y * (    9494680.718
                                  + y * (    59272.64853
                                  + y * (    267.8532712 + y * 1.0 ))));
      return ans1 / ans2;
   }
   dfloat z = 8.0 / ax;
   dfloat y = z * z;
   dfloat xx = ax - 0.785398164;
   dfloat ans1 = 1.0 + y * ( -0.1098628627e-2
                     + y * (  0.2734510407e-4
                     + y * ( -0.2073370639e-5
                     + y *    0.2093887211e-6 )));
   dfloat ans2 = -0.1562499995e-1 + y * (  0.1430488765e-3
                                  + y * ( -0.6911147651e-5
                                  + y * (  0.7621095161e-6
                                  - y *     0.934935152e-7 )));
   return std::sqrt( 0.636619772 / ax ) * ( std::cos( xx ) * ans1 - z * std::sin( xx ) * ans2 );
}

dfloat BesselJ1(
      dfloat x
) {
   dfloat ax = std::abs( x );
   if( ax < 8.0 ) {
      dfloat y = x * x;
      dfloat ans1 = x * ( 72362614232.0 + y * ( -7895059235.0
                                        + y * ( 242396853.1
                                        + y * ( -2972611.439
                                        + y * ( 15704.48260
                                        + y * ( -30.16036606 ))))));
      dfloat ans2 = 144725228442.0 + y * ( 2300535178.0
                                   + y * ( 18583304.74
                                   + y * ( 99447.43394
                                   + y * ( 376.9991397 + y * 1.0 ))));
      return ans1 / ans2;
   }
   dfloat z = 8.0 / ax;
   dfloat y = z * z;
   dfloat xx = ax - 2.356194491;
   dfloat ans1 = 1.0 + y * ( 0.183105e-2 + y * ( -0.3516396496e-4
                                         + y * ( 0.2457520174e-5
                                         + y * ( -0.240337019e-6 ))));
   dfloat ans2 = 0.04687499995 + y * ( -0.2002690873e-3
                               + y * ( 0.8449199096e-5
                               + y * ( -0.88228987e-6
                               + y * 0.105787412e-6 )));
   dfloat ans = std::sqrt( 0.636619772 / ax ) * ( std::cos( xx ) * ans1 - z * std::sin( xx ) * ans2 );
   if( x < 0.0 ) { ans = -ans; }
   return ans;
}

dfloat BesselJN(
      dfloat x,
      dip::uint n
) {
   if( n == 0 ) {
      return BesselJ0( x );
   }
   if( n == 1 ) {
      return BesselJ1( x );
   }
   // calculate Bessel function for order 2 and higher
   if( x == 0.0 ) {
      return 0.0;
   }
   dfloat ax = std::abs( x );
   dfloat ans = 0.0;
   if( ax > static_cast< dfloat >( n )) {
      dfloat tox = 2.0 / ax;
      dfloat bjm = BesselJ0( ax );
      dfloat bj = BesselJ1( ax );
      for( dip::uint j = 1; j < n; j++ ) {
         dfloat tmp = static_cast< dfloat >( j ) * tox * bj - bjm;
         bjm = bj;
         bj = tmp;
      }
      ans = bj;
   } else {
      constexpr dfloat ACC = 40.0;
      constexpr dfloat BIGNO = 1.0e10;
      constexpr dfloat BIGNI = 1.0e-10;
      dfloat tox = 2.0 / ax;
      dip::uint m = 2 * (( n + static_cast< dip::uint >( std::sqrt( ACC * static_cast< dfloat >( n )))) / 2 );
      bool jsum = false;
      dfloat bjp = 0.0;
      dfloat sum = 0.0;
      dfloat bj = 1.0;
      for( dip::uint j = m; j > 0; j-- ) {
         dfloat tmp = static_cast< dfloat >( j ) * tox * bj - bjp;
         bjp = bj;
         bj = tmp;
         if( std::abs( bj ) > BIGNO ) {
            bj *= BIGNI;
            bjp *= BIGNI;
            ans *= BIGNI;
            sum *= BIGNI;
         }
         if( jsum ) { sum += bj; }
         jsum = !jsum;
         if( j == n ) { ans = bjp; }
      }
      sum = 2.0 * sum - bj;
      ans /= sum;
   }
   return x < 0.0 && ( n & 1 ) ? -ans : ans;
}


dfloat BesselY0(
      dfloat x
) {
   if( x < 8.0 ) {
      dfloat y = x * x;
      dfloat ans1 = -2957821389.0 + y * ( 7062834065.0
                                  + y * ( -512359803.6
                                  + y * (  10879881.29
                                  + y * ( -86327.92757
                                  + y *    228.4622733 ))));
      dfloat ans2 = 40076544269.0 + y * (  745249964.8
                                  + y * (  7189466.438
                                  + y * (  47447.26470
                                  + y * (  226.1030244 + y * 1.0 ))));
      return ( ans1 / ans2 ) + 0.636619772 * BesselJ0( x ) * std::log( x );
   }
   dfloat z = 8.0 / x;
   dfloat y = z * z;
   dfloat xx = x - 0.785398164;
   dfloat ans1 = 1.0 + y * ( -0.1098628627e-2
                     + y * (  0.2734510407e-4
                     + y * ( -0.2073370639e-5
                     + y *    0.2093887211e-6 )));
   dfloat ans2 = -0.1562499995e-1 + y * (  0.1430488765e-3
                                  + y * ( -0.6911147651e-5
                                  + y * (  0.7621095161e-6
                                  + y * (  -0.934945152e-7 ))));
   return std::sqrt( 0.636619772 / x ) * ( std::sin( xx ) * ans1 + z * std::cos( xx ) * ans2 );
}

dfloat BesselY1(
      dfloat x
) {
   if( x < 8.0 ) {
      dfloat y = x * x;
      dfloat ans1 = x * ( -0.4900604943e13 + y * (  0.1275274390e13
                                           + y * ( -0.5153438139e11
                                           + y * (   0.7349264551e9
                                           + y * (  -0.4237922726e7
                                           + y *     0.8511937935e4 )))));
      dfloat ans2 = 0.2499580570e14 + y * ( 0.4244419664e12
                                    + y * ( 0.3733650367e10
                                    + y * (  0.2245904002e8
                                    + y * (  0.1020426050e6
                                    + y * (  0.3549632885e3 + y )))));
      return ( ans1 / ans2 ) + 0.636619772 * ( BesselJ1( x ) * std::log( x ) - 1.0 / x );
   }
   dfloat z = 8.0 / x;
   dfloat y = z * z;
   dfloat xx = x - 2.356194491;
   dfloat ans1 = 1.0 + y * (      0.183105e-2
                     + y * ( -0.3516396496e-4
                     + y * (  0.2457520174e-5
                     + y * (  -0.240337019e-6 ))));
   dfloat ans2 = 0.04687499995 + y * ( -0.2002690873e-3
                               + y * (  0.8449199096e-5
                               + y * (   -0.88228987e-6
                               + y *     0.105787412e-6 )));
   return std::sqrt( 0.636619772 / x ) * ( std::sin( xx ) * ans1 + z * std::cos( xx ) * ans2 );
}

dfloat BesselYN(
      dfloat x,
      dip::uint n
) {
   if( n == 0 ) {
      return BesselY0( x );
   }
   if( n == 1 ) {
      return BesselY1( x );
   }
   dfloat tox = 2.0 / x;
   dfloat by = BesselY1( x );
   dfloat bym = BesselY0( x );
   for( dip::uint j = 1; j < n; ++j ) {
      dfloat tmp = static_cast< dfloat >( j ) * tox * by - bym;
      bym = by;
      by = tmp;
   }
   return by;
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

#include <limits>

#include "diplib/library/clamp_cast.h"
#include "diplib/saturated_arithmetic.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::clamp_cast functions") {
   // We pick a few cases, it's difficult to do an exhaustive test here, and not really necessary.
   // Cast up:
   DOCTEST_CHECK( dip::clamp_cast< dip::uint32 >( dip::uint8( 50 )) == dip::uint32( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint32 >( dip::uint8( 50 )) == dip::sint32( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint32 >( dip::sint8( 50 )) == dip::uint32( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint32 >( dip::sint8( 50 )) == dip::sint32( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sfloat >( dip::sint8( 50 )) == dip::sfloat( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::dcomplex >( dip::uint32( 50 )) == dip::dcomplex{ 50, 0 } );
   DOCTEST_CHECK( dip::clamp_cast< dip::dcomplex >( dip::scomplex{ 50,0 } ) == dip::dcomplex{ 50, 0 } );
   // Cast down:
   DOCTEST_CHECK( dip::clamp_cast< dip::uint32 >( dip::sfloat( 50 )) == dip::uint32( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint32 >( dip::sfloat( -50 )) == dip::uint32( 0 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint16 >( dip::sfloat( 1e20 )) == dip::uint16( 65535 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::sfloat( -50 )) == dip::sint16( -50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::sfloat( 1e20 )) == dip::sint16( 32767 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sfloat >( dip::dcomplex{ 4, 3 } ) == dip::sfloat( 5 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint8 >( dip::scomplex{ 4, 3 } ) == dip::uint8( 5 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint8 >( dip::uint( 300 ) ) == dip::uint8( 255 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint8 >( dip::sint( -300 ) ) == dip::uint8( 0 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint8 >( dip::uint( 300 ) ) == dip::sint8( 127 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint8 >( dip::sint( -300 ) ) == dip::sint8( -128 ));
   // Signed/unsigned casts:
   DOCTEST_CHECK( dip::clamp_cast< dip::uint16 >( dip::sint16( -50 )) == dip::uint16( 0 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::uint16 >( dip::sint16( 50 )) == dip::uint16( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::uint16( 50 )) == dip::sint16( 50 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::uint16( 50000 )) == dip::sint16( 32767 ));
   // To/from dip::bin
   DOCTEST_CHECK( dip::clamp_cast< dip::bin >( dip::sint16( -50 )) == dip::bin( 1 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::bin >( dip::sint16( 0 )) == dip::bin( 0 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::bin >( dip::sint16( 50 )) == dip::bin( 1 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::bin >( dip::scomplex{ 0, 1 } ) == dip::bin( 1 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::bin( 0 )) == dip::sint16( 0 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::sint16 >( dip::bin( 1 )) == dip::sint16( 1 ));
   DOCTEST_CHECK( dip::clamp_cast< dip::scomplex >( dip::bin( 1 )) == dip::scomplex{ 1, 0 } );
   // Cast dip::infinity to integer1, 0
   DOCTEST_CHECK( dip::clamp_cast< dip::uint8 >( dip::infinity ) == dip::uint8{ 255 } );
   DOCTEST_CHECK( dip::clamp_cast< dip::uint8 >( -dip::infinity ) == dip::uint8{ 0 } );
   DOCTEST_CHECK( dip::clamp_cast< dip::sint32 >( dip::infinity ) == std::numeric_limits< dip::sint32 >::max() );
   DOCTEST_CHECK( dip::clamp_cast< dip::sint32 >( -dip::infinity ) == std::numeric_limits< dip::sint32 >::lowest() );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::saturatedXXX functions") {
   // Addition

   DOCTEST_CHECK( dip::saturated_add( dip::uint8( 200 ), dip::uint8( 5 )) == dip::uint8( 205 ));
   DOCTEST_CHECK( dip::saturated_add( dip::uint8( 200 ), dip::uint8( 50 )) == dip::uint8( 250 ));
   DOCTEST_CHECK( dip::saturated_add( dip::uint8( 200 ), dip::uint8( 60 )) == dip::uint8( 255 ));

   DOCTEST_CHECK( dip::saturated_add( dip::sint8( 100 ), dip::sint8( 5 )) == dip::sint8( 105 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( 100 ), dip::sint8( 20 )) == dip::sint8( 120 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( 100 ), dip::sint8( 30 )) == dip::sint8( 127 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( 100 ), dip::sint8( -30 )) == dip::sint8( 70 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( -100 ), dip::sint8( -5 )) == dip::sint8( -105 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( -100 ), dip::sint8( -20 )) == dip::sint8( -120 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint8( -100 ), dip::sint8( -30 )) == dip::sint8( -128 ));

   DOCTEST_CHECK( dip::saturated_add( dip::uint16( 1u<<15u ), dip::uint16( 1u<<15u )) == std::numeric_limits< dip::uint16 >::max() );
   DOCTEST_CHECK( dip::saturated_add( dip::sint16( 1u<<14u ), dip::sint16( 1u<<14u )) == std::numeric_limits< dip::sint16 >::max() );
   DOCTEST_CHECK( dip::saturated_add( -dip::sint16( 1u<<14u ), -dip::sint16( 1u<<14u )) == std::numeric_limits< dip::sint16 >::lowest() );

   DOCTEST_CHECK( dip::saturated_add( dip::uint32( 1u<<31u ), dip::uint32( 1u<<31u )) == std::numeric_limits< dip::uint32 >::max() );
   DOCTEST_CHECK( dip::saturated_add( dip::sint32( 1u<<30u ), dip::sint32( 1u<<30u )) == std::numeric_limits< dip::sint32 >::max() );
   DOCTEST_CHECK( dip::saturated_add( -dip::sint32( 1u<<30u ), -dip::sint32( 1u<<30u )) == std::numeric_limits< dip::sint32 >::lowest() );

   DOCTEST_CHECK( dip::saturated_add( dip::uint64( 9000000000000000000ull ), dip::uint64( 9000000000000000000ull )) == dip::uint64( 18000000000000000000ull ));
   DOCTEST_CHECK( dip::saturated_add( dip::uint64( 10000000000000000000ull ), dip::uint64( 10000000000000000000ull )) == std::numeric_limits< dip::uint64 >::max() );

   DOCTEST_CHECK( dip::saturated_add( dip::sint64( 9000000000000000000ll ), dip::sint64( 5 )) == dip::sint64( 9000000000000000005ll ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint64( 9000000000000000000ll ), dip::sint64( 9000000000000000000ll )) == std::numeric_limits< dip::sint64 >::max() );
   DOCTEST_CHECK( dip::saturated_add( dip::sint64( 100 ), dip::sint64( -30 )) == dip::sint64( 70 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint64( -100 ), dip::sint64( 30 )) == dip::sint64( -70 ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint64( -9000000000000000000ll ), dip::sint64( -5 )) == dip::sint64( -9000000000000000005ll ));
   DOCTEST_CHECK( dip::saturated_add( dip::sint64( -9000000000000000000ll ), dip::sint64( -9000000000000000000ll )) == std::numeric_limits< dip::sint64 >::lowest() );
   DOCTEST_CHECK( dip::saturated_add( 5.0, 3.0 ) == 8.0 );

   // Subtraction

   DOCTEST_CHECK( dip::saturated_sub( dip::uint8( 200 ), dip::uint8( 5 )) == dip::uint8( 195 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::uint8( 5 ), dip::uint8( 200 )) == dip::uint8( 0 ));

   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( 100 ), dip::sint8( 5 )) == dip::sint8( 95 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( 100 ), dip::sint8( 20 )) == dip::sint8( 80 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( 20 ), dip::sint8( 100 )) == dip::sint8( -80 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( 100 ), dip::sint8( -30 )) == dip::sint8( 127 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( -100 ), dip::sint8( -5 )) == dip::sint8( -95 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( -100 ), dip::sint8( 20 )) == dip::sint8( -120 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint8( -100 ), dip::sint8( 30 )) == dip::sint8( -128 ));

   DOCTEST_CHECK( dip::saturated_sub( dip::uint16( 100 ), dip::uint16( 20000 )) == 0 );
   DOCTEST_CHECK( dip::saturated_sub( dip::sint16( 20000 ), dip::sint16( -20000 )) == std::numeric_limits< dip::sint16 >::max() );
   DOCTEST_CHECK( dip::saturated_sub( dip::sint16( -20000 ), dip::sint16( 20000 )) == std::numeric_limits< dip::sint16 >::lowest() );

   DOCTEST_CHECK( dip::saturated_sub( dip::uint32( 100 ), dip::uint32( 1u<<31u )) == 0 );
   DOCTEST_CHECK( dip::saturated_sub( dip::sint32( 1u<<30u ), -dip::sint32( 1u<<30u )) == std::numeric_limits< dip::sint32 >::max() );
   DOCTEST_CHECK( dip::saturated_sub( -dip::sint32( 1u<<30u ), dip::sint32( 1u<<30u )) == std::numeric_limits< dip::sint32 >::lowest() );

   DOCTEST_CHECK( dip::saturated_sub( dip::uint64( 9000000000000000000ull ), dip::uint64( 1000000000000000000ull )) == dip::uint64( 8000000000000000000ull ));
   DOCTEST_CHECK( dip::saturated_sub( dip::uint64( 0ull ), dip::uint64( 9000000000000000000ull )) == dip::uint64( 0ull ));

   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( 9000000000000000000ll ), dip::sint64( 1000000000000000000ll )) == dip::sint64( 8000000000000000000ll ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( 9000000000000000000ll ), dip::sint64( 9000000000000000000ll )) == 0 );
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( 8000000000000000000ll ), dip::sint64( 9000000000000000000ll )) == dip::sint64( -1000000000000000000ll ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( 9000000000000000000ll ), dip::sint64( -9000000000000000000ll )) == std::numeric_limits< dip::sint64 >::max() );
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( 100 ), dip::sint64( -30 )) == dip::sint64( 130 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( -100 ), dip::sint64( 30 )) == dip::sint64( -130 ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( -9000000000000000000ll ), dip::sint64( 5 )) == dip::sint64( -9000000000000000005ll ));
   DOCTEST_CHECK( dip::saturated_sub( dip::sint64( -9000000000000000000ll ), dip::sint64( 9000000000000000000ll )) == std::numeric_limits< dip::sint64 >::lowest() );

   DOCTEST_CHECK( dip::saturated_sub( 5.0, 3.0 ) == 2.0 );

   // Multiplication

   DOCTEST_CHECK( dip::saturated_mul( dip::uint8( 5 ), dip::uint8( 5 )) == dip::uint8( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::uint8( 20 ), dip::uint8( 20 )) == dip::uint8( 255 ));

   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( 5 ), dip::sint8( 5 )) == dip::sint8( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( -5 ), dip::sint8( 5 )) == dip::sint8( -25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( 5 ), dip::sint8( -5 )) == dip::sint8( -25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( -5 ), dip::sint8( -5 )) == dip::sint8( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( 20 ), dip::sint8( 20 )) == dip::sint8( 127 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( -20 ), dip::sint8( 20 )) == dip::sint8( -128 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( 20 ), dip::sint8( -20 )) == dip::sint8( -128 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint8( -20 ), dip::sint8( -20 )) == dip::sint8( 127 ));

   DOCTEST_CHECK( dip::saturated_mul( dip::uint16( 300 ), dip::uint16( 300 )) == std::numeric_limits< dip::uint16 >::max() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint16( 300 ), dip::sint16( 300 )) == std::numeric_limits< dip::sint16 >::max() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint16( 300 ), dip::sint16( -300 )) == std::numeric_limits< dip::sint16 >::lowest() );

   DOCTEST_CHECK( dip::saturated_mul( dip::uint32( 70000 ), dip::uint32( 70000 )) == std::numeric_limits< dip::uint32 >::max() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint32( 70000 ), dip::sint32( 70000 )) == std::numeric_limits< dip::sint32 >::max() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint32( 70000 ), dip::sint32( -70000 )) == std::numeric_limits< dip::sint32 >::lowest() );

   DOCTEST_CHECK( dip::saturated_mul( dip::uint64( 5 ), dip::uint64( 5 )) == dip::uint64( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::uint64( 5000000000ll ), dip::uint64( 5000000000ll )) == std::numeric_limits< dip::uint64 >::max() );

   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( 5 ), dip::sint64( 5 )) == dip::sint64( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( -5 ), dip::sint64( 5 )) == dip::sint64( -25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( 5 ), dip::sint64( -5 )) == dip::sint64( -25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( -5 ), dip::sint64( -5 )) == dip::sint64( 25 ));
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( 5000000000ll ), dip::sint64( 5000000000ll )) == std::numeric_limits< dip::sint64 >::max() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( -5000000000ll ), dip::sint64( 5000000000ll )) == std::numeric_limits< dip::sint64 >::lowest() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( 5000000000ll ), dip::sint64( -5000000000ll )) == std::numeric_limits< dip::sint64 >::lowest() );
   DOCTEST_CHECK( dip::saturated_mul( dip::sint64( -5000000000ll ), dip::sint64( -5000000000ll )) == std::numeric_limits< dip::sint64 >::max() );

   DOCTEST_CHECK( dip::saturated_mul( 5.0, 3.0 ) == 15.0 );

   // Division

   DOCTEST_CHECK( dip::saturated_div( dip::sint16( 300 ), dip::sint16( 10 )) == dip::sint16( 30 ));
   DOCTEST_CHECK( dip::saturated_div( dip::sint64( 300 ), dip::sint64( 10 )) == dip::sint64( 30 ));
   DOCTEST_CHECK( dip::saturated_div( 15.0, 5.0 ) == 3.0 );

   // Inversion

   DOCTEST_CHECK( dip::saturated_inv( dip::uint8( 5 )) == dip::uint8( 250 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint8( 5 )) == dip::sint8( -5 ));

   DOCTEST_CHECK( dip::saturated_inv( dip::uint16( 300 )) == dip::uint16( 65235 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint16( 300 )) == dip::sint16( -300 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint16( -32768 )) == dip::sint16( 32767 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint16( -32767 )) == dip::sint16( 32767 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint16( -32766 )) == dip::sint16( 32766 ));

   DOCTEST_CHECK( dip::saturated_inv( dip::uint32( 5 )) == std::numeric_limits< dip::uint32 >::max() - dip::uint32( 5 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint32( 5 )) == dip::sint32( -5 ));

   DOCTEST_CHECK( dip::saturated_inv( dip::uint64( 5 )) == std::numeric_limits< dip::uint64 >::max() - dip::uint64( 5 ));
   DOCTEST_CHECK( dip::saturated_inv( dip::sint64( 5 )) == dip::sint64( -5 ));

   DOCTEST_CHECK( dip::saturated_inv( 5.0 ) == -5.0 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::maximum_gauss_truncation function") {
   double truncation = dip::maximum_gauss_truncation< dip::dfloat >() * 0.999999; // avoid fp rounding errors
   double sigma = 10.0;
   double x = truncation * sigma;
   double result = std::exp( -0.5 * ( x * x ) / ( sigma * sigma ));
   DOCTEST_CHECK( result >= std::numeric_limits< dip::dfloat >::epsilon() );
   x += 0.1 * sigma;
   double result2 = std::exp( -0.5 * ( x * x ) / ( sigma * sigma ));
   DOCTEST_CHECK( result2 < std::numeric_limits< dip::dfloat >::epsilon() );

   truncation = dip::maximum_gauss_truncation< dip::sfloat >() * 0.999999; // avoid fp rounding errors
   x = truncation * sigma;
   result = std::exp( -0.5 * ( x * x ) / ( sigma * sigma ));
   DOCTEST_CHECK( result >= std::numeric_limits< dip::sfloat >::epsilon() );
   x += 0.1 * sigma;
   result2 = std::exp( -0.5 * ( x * x ) / ( sigma * sigma ));
   DOCTEST_CHECK( result2 < std::numeric_limits< dip::sfloat >::epsilon() );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::gcd function") {
   DOCTEST_CHECK( dip::gcd( 10, 10 ) == 10 );
   DOCTEST_CHECK( dip::gcd( 10, 5 ) == 5 );
   DOCTEST_CHECK( dip::gcd( 10, 1 ) == 1 );
   DOCTEST_CHECK( dip::gcd( 10, 12 ) == 2 );
   DOCTEST_CHECK( dip::gcd( 10, 15 ) == 5 );
   DOCTEST_CHECK( dip::gcd( 15, 10 ) == 5 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::div_{floor|ceil|round} functions") {
   DOCTEST_CHECK( dip::div_ceil( 11l, 11l ) == 1 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 6l ) == 2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 5l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 4l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, 3l ) == 4 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 3l ) == -3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 4l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 5l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, 6l ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -3l ) == -3 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -4l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -5l ) == -2 );
   DOCTEST_CHECK( dip::div_ceil( 11l, -6l ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -6l ) == 2 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -5l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -4l ) == 3 );
   DOCTEST_CHECK( dip::div_ceil( -11l, -3l ) == 4 );

   DOCTEST_CHECK( dip::div_floor( 10l, 10l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( 11l, 6l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( 11l, 5l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( 11l, 4l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( 11l, 3l ) == 3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 3l ) == -4 );
   DOCTEST_CHECK( dip::div_floor( -11l, 4l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 5l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( -11l, 6l ) == -2 );
   DOCTEST_CHECK( dip::div_floor( 11l, -3l ) == -4 );
   DOCTEST_CHECK( dip::div_floor( 11l, -4l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( 11l, -5l ) == -3 );
   DOCTEST_CHECK( dip::div_floor( 11l, -6l ) == -2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -6l ) == 1 );
   DOCTEST_CHECK( dip::div_floor( -11l, -5l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -4l ) == 2 );
   DOCTEST_CHECK( dip::div_floor( -11l, -3l ) == 3 );

   DOCTEST_CHECK( dip::div_round( 10l, 10l ) == 1 );
   DOCTEST_CHECK( dip::div_round( 11l, 6l ) == 2 );
   DOCTEST_CHECK( dip::div_round( 11l, 5l ) == 2 );
   DOCTEST_CHECK( dip::div_round( 11l, 4l ) == 3 );
   DOCTEST_CHECK( dip::div_round( 11l, 3l ) == 4 );
   DOCTEST_CHECK( dip::div_round( -11l, 3l ) == -4 );
   DOCTEST_CHECK( dip::div_round( -11l, 4l ) == -3 );
   DOCTEST_CHECK( dip::div_round( -11l, 5l ) == -2 );
   DOCTEST_CHECK( dip::div_round( -11l, 6l ) == -2 );
   DOCTEST_CHECK( dip::div_round( 11l, -3l ) == -4 );
   DOCTEST_CHECK( dip::div_round( 11l, -4l ) == -3 );
   DOCTEST_CHECK( dip::div_round( 11l, -5l ) == -2 );
   DOCTEST_CHECK( dip::div_round( 11l, -6l ) == -2 );
   DOCTEST_CHECK( dip::div_round( -11l, -6l ) == 2 );
   DOCTEST_CHECK( dip::div_round( -11l, -5l ) == 2 );
   DOCTEST_CHECK( dip::div_round( -11l, -4l ) == 3 );
   DOCTEST_CHECK( dip::div_round( -11l, -3l ) == 4 );
   DOCTEST_CHECK( dip::div_round( 0l, 5l ) == 0 );
   DOCTEST_CHECK( dip::div_round( 0l, -5l ) == 0 );
   DOCTEST_CHECK( dip::div_round( 5l, 5l ) == 1 );
   DOCTEST_CHECK( dip::div_round( 5l, -5l ) == -1 );
   DOCTEST_CHECK( dip::div_round( -5l, 5l ) == -1 );
   DOCTEST_CHECK( dip::div_round( -5l, -5l ) == 1 );

   // Integer overflow caused all these operations below to return 0 before.
   dip::uint m = std::numeric_limits< dip::uint >::max();
   m = m / 2 + 1;
   DOCTEST_CHECK( dip::div_ceil( m, m ) == 1 );
   DOCTEST_CHECK( dip::div_floor( m, m ) == 1 );
   dip::sint s = std::numeric_limits< dip::sint >::max();
   s = s / 2 + 1;
   DOCTEST_CHECK( dip::div_ceil( s, s ) == 1 );
   DOCTEST_CHECK( dip::div_ceil( -s, s ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( s, -s ) == -1 );
   DOCTEST_CHECK( dip::div_ceil( -s, -s ) == 1 );
   DOCTEST_CHECK( dip::div_floor( s, s ) == 1 );
   DOCTEST_CHECK( dip::div_floor( -s, s ) == -1 );
   DOCTEST_CHECK( dip::div_floor( s, -s ) == -1 );
   DOCTEST_CHECK( dip::div_floor( -s, -s ) == 1 );
   DOCTEST_CHECK( dip::div_round( s, s ) == 1 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::{floor|ceil|round}_cast functions") {
   DOCTEST_CHECK( dip::floor_cast( 11.0 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( 11.000000001 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( 11.4 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( 11.5 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( 11.9 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( 11.999999999 ) == 11 );
   DOCTEST_CHECK( dip::floor_cast( -11.0 ) == -11 );
   DOCTEST_CHECK( dip::floor_cast( -11.000000001 ) == -12 );
   DOCTEST_CHECK( dip::floor_cast( -11.4 ) == -12 );
   DOCTEST_CHECK( dip::floor_cast( -11.5 ) == -12 );
   DOCTEST_CHECK( dip::floor_cast( -11.9 ) == -12 );
   DOCTEST_CHECK( dip::floor_cast( -11.999999999 ) == -12 );

   DOCTEST_CHECK( dip::ceil_cast( 11.0 ) == 11 );
   DOCTEST_CHECK( dip::ceil_cast( 11.000000001 ) == 12 );
   DOCTEST_CHECK( dip::ceil_cast( 11.4 ) == 12 );
   DOCTEST_CHECK( dip::ceil_cast( 11.5 ) == 12 );
   DOCTEST_CHECK( dip::ceil_cast( 11.9 ) == 12 );
   DOCTEST_CHECK( dip::ceil_cast( 11.999999999 ) == 12 );
   DOCTEST_CHECK( dip::ceil_cast( -11.0 ) == -11 );
   DOCTEST_CHECK( dip::ceil_cast( -11.000000001 ) == -11 );
   DOCTEST_CHECK( dip::ceil_cast( -11.4 ) == -11 );
   DOCTEST_CHECK( dip::ceil_cast( -11.5 ) == -11 );
   DOCTEST_CHECK( dip::ceil_cast( -11.9 ) == -11 );
   DOCTEST_CHECK( dip::ceil_cast( -11.999999999 ) == -11 );

   DOCTEST_CHECK( dip::round_cast( 11.0 ) == 11 );
   DOCTEST_CHECK( dip::round_cast( 11.000000001 ) == 11 );
   DOCTEST_CHECK( dip::round_cast( 11.4 ) == 11 );
   DOCTEST_CHECK( dip::round_cast( 11.499999999 ) == 11 );
   DOCTEST_CHECK( dip::round_cast( 11.5 ) == 12 );
   DOCTEST_CHECK( dip::round_cast( 11.500000001 ) == 12 );
   DOCTEST_CHECK( dip::round_cast( 11.9 ) == 12 );
   DOCTEST_CHECK( dip::round_cast( 11.999999999 ) == 12 );
   DOCTEST_CHECK( dip::round_cast( -11.0 ) == -11 );
   DOCTEST_CHECK( dip::round_cast( -11.000000001 ) == -11 );
   DOCTEST_CHECK( dip::round_cast( -11.4 ) == -11 );
   DOCTEST_CHECK( dip::round_cast( -11.499999999 ) == -11 );
   DOCTEST_CHECK( dip::round_cast( -11.5 ) == -11 );
   DOCTEST_CHECK( dip::round_cast( -11.500000001 ) == -12 );
   DOCTEST_CHECK( dip::round_cast( -11.9 ) == -12 );
   DOCTEST_CHECK( dip::round_cast( -11.999999999 ) == -12 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::pow10 function") {
   DOCTEST_CHECK( dip::pow10( 25 ) == doctest::Approx( std::pow( 10, 25 )));
   DOCTEST_CHECK( dip::pow10( 10 ) == std::pow( 10, 10 ));
   DOCTEST_CHECK( dip::pow10( 1 ) == std::pow( 10, 1 ));
   DOCTEST_CHECK( dip::pow10( 0 ) == std::pow( 10, 0 ));
   DOCTEST_CHECK( dip::pow10( -5 ) == doctest::Approx( std::pow( 10, -5 )));
   DOCTEST_CHECK( dip::pow10( -21 ) == doctest::Approx( std::pow( 10, -21 )));
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::abs function") {
   DOCTEST_CHECK( dip::abs( 25.0 ) == 25.0 );
   DOCTEST_CHECK( dip::abs( -25.0 ) == 25.0 );
   DOCTEST_CHECK( dip::abs( 0.0 ) == 0.0 );
   DOCTEST_CHECK( dip::abs( 25.6f ) == 25.6f );
   DOCTEST_CHECK( dip::abs( -25.6f ) == 25.6f );
   DOCTEST_CHECK( dip::abs( 25 ) == 25 );
   DOCTEST_CHECK( dip::abs( -25 ) == 25 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::ApproximatelyEquals function") {
   DOCTEST_CHECK( dip::ApproximatelyEquals( 1.0, 1.0 ));
   DOCTEST_CHECK( !dip::ApproximatelyEquals( 1.0, 1.1 ));
   DOCTEST_CHECK( dip::ApproximatelyEquals( 1.0, 1.05, 0.1 ));
   DOCTEST_CHECK( dip::ApproximatelyEquals( 1.0, 1.0, 0.0 ));
   DOCTEST_CHECK( !dip::ApproximatelyEquals( 1.0, 1.0 + 1e-12, 0.0 ));
   DOCTEST_CHECK( dip::ApproximatelyEquals( 1.0, 1.0 + 1e-12 ));
}

DOCTEST_TEST_CASE("[DIPlib] testing the dip::RankFromPercentile function") {
   DOCTEST_CHECK( dip::RankFromPercentile( 0.0, 100 ) == 0 );
   DOCTEST_CHECK( dip::RankFromPercentile( 25.0, 100 ) == 25 );
   DOCTEST_CHECK( dip::RankFromPercentile( 50.0, 100 ) == 50 );
   DOCTEST_CHECK( dip::RankFromPercentile( 75.0, 100 ) == 74 );
   DOCTEST_CHECK( dip::RankFromPercentile( 100.0, 100 ) == 99 );
   DOCTEST_CHECK( dip::RankFromPercentile( 0.0, 101 ) == 0 );
   DOCTEST_CHECK( dip::RankFromPercentile( 25.0, 101 ) == 25 );
   DOCTEST_CHECK( dip::RankFromPercentile( 50.0, 101 ) == 50 );
   DOCTEST_CHECK( dip::RankFromPercentile( 75.0, 101 ) == 75 );
   DOCTEST_CHECK( dip::RankFromPercentile( 100.0, 101 ) == 100 );
   DOCTEST_CHECK( dip::RankFromPercentile( 0.0, 1 ) == 0 );
   DOCTEST_CHECK( dip::RankFromPercentile( 50.0, 1 ) == 0 );
   DOCTEST_CHECK( dip::RankFromPercentile( 100.0, 1 ) == 0 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
