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
   // Some static asserts. This is an internal function, we don't have to worry about these firing because the user does something weird.
   static_assert( sizeof( UIntT ) >= sizeof( dip::uint ), "Template instantiated with type of unexpected size." );
   static_assert( std::numeric_limits< UIntT >::is_integer && !std::numeric_limits< UIntT >::is_signed,
                  "Template instantiated with type that is not unsigned integer." );
   static_assert( maxFactor == 5 || maxFactor == 7 || maxFactor == 11, "Illegal value for maxFactor template argument" );

   if( n <= static_cast< UIntT >( maxFactor + 1 )) {
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
   // Some static asserts. This is an internal function, we don't have to worry about these firing because the user does something weird.
   static_assert( sizeof( UIntT ) >= sizeof( dip::uint ), "Template instantiated with type of unexpected size." );
   static_assert( std::numeric_limits< UIntT >::is_integer && !std::numeric_limits< UIntT >::is_signed,
                  "Template instantiated with type that is not unsigned integer." );
   static_assert( maxFactor == 5 || maxFactor == 7 || maxFactor == 11, "Illegal value for maxFactor template argument" );

   if( n <= static_cast< UIntT >( maxFactor + 1 )) {
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
   // The following two values test proper handling on a 32-bit system
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max() / 11 / 2, true, 11 ) == 195230112 ); // does the work in the 32-bit algorithm
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max() / 11 / 2 + 1, true, 11 ) == 195230112 ); // should be elevated to the 64-bit algorithm
   // The following two values depend on the system's pointer size and the FFT library linked
   dip::uint result = 0;
   if(( sizeof( dip::uint ) == 8 ) && ( dip::maximumDFTSize > std::numeric_limits< dip::uint32 >::max() )) {
      // The `dip::maximumDFTSize > ...` is not exact, but currently, `maximumDFTSize` is either INT_MAX or dip::uint_MAX
      result = 4294967296;
   }
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max(), true, 11 ) == result );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max() - 1, true, 11 ) == result );
   // The following two values are always 0, because they don't fit in a dip::uint
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint >::max(), true, 11 ) == 0 );
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint >::max() - 1, true, 11 ) == 0 );

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
   // The following two values test proper handling on a 32-bit system
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max() / 11, false, 11 ) == 390297600 ); // does the work in the 32-bit algorithm
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max() / 11 + 1, false, 11 ) == 390297600 ); // should be elevated to the 64-bit algorithm
   // The following value depends on the system's pointer size and the FFT library linked
   result = 0;
   if(( sizeof( dip::uint ) == 8 ) && ( dip::maximumDFTSize > std::numeric_limits< dip::uint32 >::max() )) {
      // The `dip::maximumDFTSize > ...` is not exact, but currently, `maximumDFTSize` is either INT_MAX or dip::uint_MAX
      result = 4293273600ul;
   }
   DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint32 >::max(), false, 11 ) == result );
   // The following value is always 0, because this is outside the ability of the algorithm
   if( sizeof( dip::uint ) == 8 ) {
      DOCTEST_CHECK( dip::GetOptimalDFTSize( std::numeric_limits< dip::uint >::max(), false, 11 ) == 0 );
   }

   // Invalid argument
   DOCTEST_CHECK_THROWS( dip::GetOptimalDFTSize( 100, true, 13 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
