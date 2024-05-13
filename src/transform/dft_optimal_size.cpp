/*
(c)2024, Cris Luengo

The functions here were modified from the functions good_size_cmplx() and
prev_good_size_cmplx() from PocketFFT.

Original copyright notice:

Copyright (C) 2010-2022 Max-Planck-Society
Copyright (C) 2019-2020 Peter Bell

For the odd-sized DCT-IV transforms:
  Copyright (C) 2003, 2007-14 Matteo Frigo
  Copyright (C) 2003, 2007-14 Massachusetts Institute of Technology

For the prev_good_size search:
  Copyright (C) 2024 Tan Ping Liang, Peter Bell

Authors: Martin Reinecke, Peter Bell

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
* Neither the name of the copyright holder nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "diplib/dft.h"

#include <algorithm>
#include <limits>

#include "diplib.h"

namespace dip {
namespace {

template< dip::uint maxFactor >
dip::uint NextOptimalSize( dip::uint n ) {
   // maxFactor should be 5, 7 or 11
   if( n <= maxFactor + 1 ) {
      return n;
   }
   dip::uint best = 2 * n;
   if( best < n ) {
      // Handle overflow
      best = std::numeric_limits< dip::uint >::max();
   }
   for( dip::uint f11 = 1; f11 < ( maxFactor >= 11 ? best : 2 ); f11 *= 11 ) {
      for( dip::uint f117 = f11; f117 < ( maxFactor >= 7 ? best : 2 ); f117 *= 7 ) {
         for( dip::uint f1175 = f117; f1175 < best; f1175 *= 5 ) {
            dip::uint x = f1175;
            while( x < n ) {
               x *= 2;
            }
            while( true ) {
               if( x < n ) {
                  x *= 3;
               } else if( x > n ) {
                  best = std::min( x, best );
                  if( x & 1 ) {
                     break;
                  }
                  x /= 2;
               } else {
                  return n;
               }
            }
         }
      }
   }
   if( best == std::numeric_limits< dip::uint >::max() ) {
      // We had overflow earlier, and we haven't found a better value.
      // This means there's no value larger than `n` that is good for FFT.
      return 0;
   }
   return best;
}

template< dip::uint maxFactor >
dip::uint PreviousOptimalSize( dip::uint n ) {
   // maxFactor should be 5, 7 or 11
   if( n <= maxFactor + 1 ) {
      return n;
   }
   dip::uint best = 1;
   for( dip::uint f11 = 1; f11 <= ( maxFactor >= 11 ? n : 1 ); f11 *= 11 ) {
      for( dip::uint f117 = f11; f117 <= ( maxFactor >= 7 ? n : 1 ); f117 *= 7 ) {
         for( dip::uint f1175 = f117; f1175 <= n; f1175 *= 5 ) {
            dip::uint x = f1175;
            while( x * 2 <= n ) {
               x *= 2;
            }
            best = std::max( x, best );
            while( true ) {
               if( x * 3 <= n ) {
                  x *= 3;
               }
               else if( x % 2 == 0 ) {
                  x /= 2;
               }
               else {
                  break;
               }
               best = std::max( x, best );
            }
         }
      }
   }
   return best;
}

} // namespace

dip::uint GetOptimalDFTSize( dip::uint size0, bool larger, dip::uint maxFactor ) {
   dip::uint out = 0;
   switch( maxFactor ) {
      case 5:
         out = larger ? NextOptimalSize< 5 >( size0 ) : PreviousOptimalSize< 5 >( size0 );
         break;
      case 7:
         out = larger ? NextOptimalSize< 7 >( size0 ) : PreviousOptimalSize< 7 >( size0 );
         break;
      case 11:
         out = larger ? NextOptimalSize< 11 >( size0 ) : PreviousOptimalSize< 11 >( size0 );
         break;
      default:
         DIP_THROW( "maxFactor must be 5, 7 or 11." );
   }
   return out > maximumDFTSize ? 0 : out;
}

dip::uint MaxFactor( bool complex ) {
   // The logic:
   //  - FFTW is most efficient when maxFactor = 7.
   //  - PocketFFT is most efficient when maxFactor = 11 when doing complex-to-complex transforms.
   //  - PocketFFT is most efficient when maxFactor = 5 when doing real-to-complex or complex-to-real transforms.
   if( usingFFTW ) {
      return 7;
   }
   return complex ? 11 : 5;
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing GetOptimalDFTSize") {
   // larger, 5
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, true, 5 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, true, 5 ) == 12 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, true, 5 ) == 15 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 101, true, 5 ) == 108 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2109375001, true, 5 ) == 2123366400 );
   // larger, 7
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, true, 7 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, true, 7 ) == 12 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, true, 7 ) == 14 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 101, true, 7 ) == 105 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2109375001, true, 7 ) == 2113929216 );
   // larger, 11
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, true, 11 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, true, 11 ) == 11 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, true, 11 ) == 14 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 101, true, 11 ) == 105 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2109375001, true, 11 ) == 2112000000 );

   // smaller 5
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, false, 5 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, false, 5 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, false, 5 ) == 12 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 107, false, 5 ) == 100 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2123366399, false, 5 ) == 2109375000 );
   // smaller 7
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, false, 7 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, false, 7 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, false, 7 ) == 12 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 107, false, 7 ) == 105 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2123366399, false, 7 ) == 2117682000 );
   // smaller 11
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 10, false, 11 ) == 10 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 11, false, 11 ) == 11 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 13, false, 11 ) == 12 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 107, false, 11 ) == 105 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( 2123366399, false, 11 ) == 2122312500 );

   // Invalid argument
   DOCTEST_CHECK_THROWS( dip::GetOptimalDFTSize( 100, true, 13 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
