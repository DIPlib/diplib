/*
 * DIPlib 3.0
 * This file contains definitions for Bessel, gamma and error functions.
 *
 * (c)2017, Cris Luengo.
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

namespace dip {


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
   } else {
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
   } else {
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
}

dfloat BesselJN(
      dfloat x,
      dip::uint n
) {
   if( x == 0.0 ) {
      return 0.0;
   }
   if( n == 0 ) {
      return BesselJ0( x );
   }
   if( n == 1 ) {
      return BesselJ1( x );
   }
   // calculate Bessel function for order 2 and higher
   dfloat ax = std::abs( x );
   dfloat ans = 0.0;
   if( ax > ( dfloat )n ) {
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
   } else {
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
   } else {
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
