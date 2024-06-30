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

#include <limits>

#include "diplib.h"

namespace dip {
namespace {

// The saturated multiplication code below is modified from include/diplib/saturated_arithmetic.h

template< dip::uint maxFactor, typename UIntT >
UIntT NextOptimalSize( UIntT n ) {
   // UIntT should be an unsigned integer type, typically 32-bit or 64-bit.
   // maxFactor should be 5, 7 or 11
   if( n <= maxFactor + 1 ) {
      return n;
   }
   if( n > std::numeric_limits< UIntT >::max() / maxFactor / 2 ) {
      // The algorithm below doesn't work for this value, the multiplication can overflow.
      if( sizeof( UIntT ) == 4 ) {
         // We can try using this algorithm with 64-bit integers:
         uint64 res = NextOptimalSize< maxFactor >( static_cast< uint64 >( n ));
         if( res <= std::numeric_limits< UIntT >::max() ) {
            return static_cast< UIntT >( res );
         }
      }
      // Otherwise, this size is ridiculously large, people shouldn't be computing FFTs this large.
      return 0;
   }
   UIntT best = n * 2;
   for( UIntT f11 = 1; f11 < ( maxFactor >= 11 ? best : 2 ); f11 *= 11 ) {
      for( UIntT f117 = f11; f117 < ( maxFactor >= 7 ? best : 2 ); f117 *= 7 ) {
         for( UIntT f1175 = f117; f1175 < best; f1175 *= 5 ) {
            UIntT x = f1175;
            while( x < n ) {
               x *= 2;
            }
            while( true ) {
               if( x == n ) {
                  return n;
               }
               if( x < n ) {
                  x *= 3;
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
   if( best == std::numeric_limits< UIntT >::max() ) {
      // This means there's no value larger than `n` that is good for FFT.
      return 0;
   }
   return best;
}

template< dip::uint maxFactor, typename UIntT >
UIntT PreviousOptimalSize( UIntT n ) {
   // maxFactor should be 5, 7 or 11
   if( n <= maxFactor + 1 ) {
      return n;
   }
   if( n > std::numeric_limits< UIntT >::max() / maxFactor ) {
      // The algorithm below doesn't work for this value, the multiplication can overflow.
      if( sizeof( UIntT ) == 4 ) {
         // We can try using this algorithm with 64-bit integers:
         uint64 res = PreviousOptimalSize< maxFactor >( static_cast< uint64 >( n ));
         if( res <= std::numeric_limits< UIntT >::max() ) { // This should always be the case
            return static_cast< UIntT >( res );
         }
      }
      // Otherwise, this size is ridiculously large, people shouldn't be computing FFTs this large.
      return 0;
   }
   UIntT best = 1;
   for( UIntT f11 = 1; f11 <= ( maxFactor >= 11 ? n : 1 ); f11 *= 11 ) {
      for( UIntT f117 = f11; f117 <= ( maxFactor >= 7 ? n : 1 ); f117 *= 7 ) {
         for( UIntT f1175 = f117; f1175 <= n; f1175 *= 5 ) {
            UIntT x = f1175;
            auto x2 = x * 2;
            while( x2 <= n ) {
               x = x2;
               x2 = x * 2;
            }
            best = std::max( x, best );
            while( true ) {
               auto x3 = x * 3;
               if( x3 <= n ) {
                  x = x3;
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
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() / 11 / 2 ) == 195230112 ); // does the work in the 32-bit algorithm
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() / 11 / 2 + 1 ) == 195230112 ); // should be elevated to the 64-bit algorithm
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() ) == 0 );
   DOCTEST_CHECK( dip::NextOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() - 1 ) == 0 );
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
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() / 11 ) == 390297600); // does the work in the 32-bit algorithm
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() / 11 + 1 ) == 390297600); // should be elevated to the 64-bit algorithm
   DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint32 >::max() ) == 4293273600ul );
   if( sizeof( dip::uint ) == 8 ) {
      DOCTEST_CHECK( dip::PreviousOptimalSize< 11 >( std::numeric_limits< dip::uint >::max() ) == 0 );
   }

   // Invalid argument
   DOCTEST_CHECK_THROWS( dip::GetOptimalDFTSize( 100, true, 13 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
