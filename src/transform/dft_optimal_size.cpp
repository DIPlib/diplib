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

// The saturated multiplication code below is modified from include/diplib/saturated_arithmetic.h

struct saturated_mul_result {
   bool overflow;
   dip::uint result;
};

template< typename T >
constexpr saturated_mul_result saturated_mul_simple( T lhs, T rhs ) {
   T result = lhs * rhs;
   if( result > std::numeric_limits< dip::uint >::max() ) {
      return { true, std::numeric_limits< dip::uint >::max() };
   }
   return { false, static_cast< dip::uint >( result ) };
}

constexpr saturated_mul_result saturated_mul( dip::uint lhs, dip::uint rhs ) {
   static_assert(( sizeof( dip::uint ) == 8 ) || ( sizeof( dip::uint ) == 4 ), "This function expects either a 32-bit system or a 64-bit system." );
   // Note that the sizeof() choice should be elided by an optimizing compiler, only leaving one of the two branches.
   if( sizeof( dip::uint ) == 4 ) {
      // 32-bit system
      return saturated_mul_simple< uint64 >( lhs, rhs );
   }
   #ifdef __SIZEOF_INT128__
      // 64-bit system, using 128-bit arithmetic
      return saturated_mul_simple< __uint128_t >( lhs, rhs );
   #else
      // 64-bit system, the hard (and expensive) way
      dip::uint result = lhs * rhs;
      if(( lhs != 0 ) && ( result / lhs != rhs )) {
         return { true, std::numeric_limits< dip::uint >::max() };
      }
      return { false, result };
   #endif
}

template< dip::uint maxFactor >
dip::uint NextOptimalSize( dip::uint n ) {
   // maxFactor should be 5, 7 or 11
   if( n <= maxFactor + 1 ) {
      return n;
   }
   if( n == std::numeric_limits< dip::uint >::max() ) {
      // The algorithm below doesn't work for this value.
      // No matter how many bits n has, there is no fast number >= n.
      return 0;
   }
   dip::uint best = saturated_mul( n, 2 ).result;
   for( dip::uint f11 = 1; f11 < ( maxFactor >= 11 ? best : 2 ); f11 = saturated_mul( f11, 11 ).result ) {
      for( dip::uint f117 = f11; f117 < ( maxFactor >= 7 ? best : 2 ); f117 = saturated_mul( f117, 7 ).result ) {
         for( dip::uint f1175 = f117; f1175 < best; f1175 = saturated_mul( f1175, 5 ).result ) {
            dip::uint x = f1175;
            while( x < n ) {
               auto res = saturated_mul( x, 2 );
               if( res.overflow ) {
                  break;
               }
               x = res.result;
            }
            while( true ) {
               if( x == n ) {
                  return n;
               }
               if( x < n ) {
                  auto res = saturated_mul( x, 3 );
                  if( res.overflow ) {
                     break;
                  }
                  x = res.result;
               } else { // x > n
                  best = std::min( x, best );
                  if( x & 1 ) {
                     break;
                  }
                  x /= 2;
               }
            }
         }
      }
   }
   if( best == std::numeric_limits< dip::uint >::max() ) {
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
   if( n == std::numeric_limits< dip::uint >::max() ) {
      // The algorithm below is an infinite loop for this value.
      // No matter how many bits n has, this number cannot be nice, so we search for a fast number <= n-1.
      n -= 1;
   }
   dip::uint best = 1;
   for( dip::uint f11 = 1; f11 <= ( maxFactor >= 11 ? n : 1 ); f11 = saturated_mul( f11, 11 ).result ) {
      for( dip::uint f117 = f11; f117 <= ( maxFactor >= 7 ? n : 1 ); f117 = saturated_mul( f117, 7 ).result ) {
         for( dip::uint f1175 = f117; f1175 <= n; f1175 = saturated_mul( f1175, 5 ).result ) {
            dip::uint x = f1175;
            auto x2 = saturated_mul( x, 2 );
            while( x2.result <= n ) {
               x = x2.result;
               x2 = saturated_mul( x, 2 );
            }
            best = std::max( x, best );
            while( true ) {
               auto x3 = saturated_mul( x, 3 );
               if( x3.result <= n ) {
                  x = x3.result;
               }
               else if(( x & 1 ) == 0 ) {
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
   DOCTEST_CHECK( dip::NextOptimalSize< 5 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::NextOptimalSize< 5 >( 11 ) == 12 );
   DOCTEST_CHECK( dip::NextOptimalSize< 5 >( 13 ) == 15 );
   DOCTEST_CHECK( dip::NextOptimalSize< 5 >( 101 ) == 108 );
   DOCTEST_CHECK( dip::NextOptimalSize< 5 >( 2109375001 ) == 2123366400 );
   // larger, 7
   DOCTEST_CHECK( dip::NextOptimalSize< 7 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::NextOptimalSize< 7 >( 11 ) == 12 );
   DOCTEST_CHECK( dip::NextOptimalSize< 7 >( 13 ) == 14 );
   DOCTEST_CHECK( dip::NextOptimalSize< 7 >( 101 ) == 105 );
   DOCTEST_CHECK( dip::NextOptimalSize< 7 >( 2109375001 ) == 2113929216 );
   // larger, 11
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( 11 ) == 11 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( 13 ) == 14 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( 101 ) == 105 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( 2109375001 ) == 2112000000 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint >::max() ) == 0 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint >::max() - 1 ) == 0 );

   // smaller 5
   DOCTEST_CHECK( dip::PreviousOptimalSize< 5 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 5 >( 11 ) == 10 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 5 >( 13 ) == 12 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 5 >( 107 ) == 100 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 5 >( 2123366399 ) == 2109375000 );
   // smaller 7
   DOCTEST_CHECK( dip::PreviousOptimalSize< 7 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 7 >( 11 ) == 10 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 7 >( 13 ) == 12 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 7 >( 107 ) == 105 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 7 >( 2123366399 ) == 2117682000 );
   // smaller 11
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( 10 ) == 10 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( 11 ) == 11 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( 13 ) == 12 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( 107 ) == 105 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( 2123366399 ) == 2122312500 );
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() ) == 4293273600ul );
   if( sizeof( dip::uint ) == 8 ) {
      DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint >::max() ) == 18446613971412049920ull );
   }

   // Invalid argument
   DOCTEST_CHECK_THROWS( dip::GetOptimalDFTSize( 100, true, 13 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
