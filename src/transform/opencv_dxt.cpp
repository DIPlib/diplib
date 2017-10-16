// Taken from OpenCV 3.1
// Modifications (c)2017 Cris Luengo:
//    - Using the C++ Standard Library for std::complex, std::vector, std::swap, etc.
//    - Simplified expressions that do complex arithmetic (because we're using std::complex).
//    - Removed all unnecessary functions and functionality, leaving only DFT() and support.
//    - Encapsulated all functionality in a class DFT.
//    - Added anonymous namespaces.
//    - Using std::vector for buffers.
// NOTE!
//    If you are wondering why some complex multiplications are written out:
//    The multiplication for two std::complex values is 3-8 times slower than the equivalent
//    hand-written code. I presume this is because of the tests for NaN that the Std Lib must
//    do because of the definition of complex infinity. With GCC-5 and particular optimization
//    switches turned on, it becomes about 50x slower! (probably a compiler bug, the same is
//    not observed with CLang).


////////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32
   #define _USE_MATH_DEFINES // Needed to define M_PI in <complex>/<cmath>
#endif

#include <complex>
#include <vector>
#include <cstring>

#include "diplib/library/numeric.h"
#include "diplib/dft.h"


#if defined(__GNUG__) || defined(__clang__)
// The sign conversion warnings are all related to indexing into a std::vector using an int instead of an unsigned
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif


namespace dip {

namespace {

constexpr static unsigned char bitrevTab[] = {
      0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
      0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
      0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
      0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
      0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
      0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
      0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
      0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
      0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
      0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
      0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
      0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
      0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
      0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
      0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
      0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static inline int BitRev( int i, int shift ) {
   return (( int )(((( unsigned )bitrevTab[ ( i ) & 255 ] << 24 ) +
                    (( unsigned )bitrevTab[ (( i ) >> 8 ) & 255 ] << 16 ) +
                    (( unsigned )bitrevTab[ (( i ) >> 16 ) & 255 ] << 8 ) +
                    (( unsigned )bitrevTab[ (( i ) >> 24 ) ] )) >> shift ));
}

constexpr static const std::complex< double > DFTTab[] = {
      { 1.00000000000000000,  -0.00000000000000000 },
      { -1.00000000000000000, -0.00000000000000000 },
      { 0.00000000000000000,  -1.00000000000000000 },
      { 0.70710678118654757,  -0.70710678118654746 },
      { 0.92387953251128674,  -0.38268343236508978 },
      { 0.98078528040323043,  -0.19509032201612825 },
      { 0.99518472667219693,  -0.09801714032956060 },
      { 0.99879545620517241,  -0.04906767432741802 },
      { 0.99969881869620425,  -0.02454122852291229 },
      { 0.99992470183914450,  -0.01227153828571993 },
      { 0.99998117528260111,  -0.00613588464915448 },
      { 0.99999529380957619,  -0.00306795676296598 },
      { 0.99999882345170188,  -0.00153398018628477 },
      { 0.99999970586288223,  -0.00076699031874270 },
      { 0.99999992646571789,  -0.00038349518757140 },
      { 0.99999998161642933,  -0.00019174759731070 },
      { 0.99999999540410733,  -0.00009587379909598 },
      { 0.99999999885102686,  -0.00004793689960307 },
      { 0.99999999971275666,  -0.00002396844980842 },
      { 0.99999999992818922,  -0.00001198422490507 },
      { 0.99999999998204725,  -0.00000599211245264 },
      { 0.99999999999551181,  -0.00000299605622633 },
      { 0.99999999999887801,  -0.00000149802811317 },
      { 0.99999999999971945,  -0.00000074901405658 },
      { 0.99999999999992983,  -0.00000037450702829 },
      { 0.99999999999998246,  -0.00000018725351415 },
      { 0.99999999999999567,  -0.00000009362675707 },
      { 0.99999999999999889,  -0.00000004681337854 },
      { 0.99999999999999978,  -0.00000002340668927 },
      { 0.99999999999999989,  -0.00000001170334463 },
      { 1.00000000000000000,  -0.00000000585167232 },
      { 1.00000000000000000,  -0.00000000292583616 }
};

std::vector< int > DFTFactorize( int n ) {
   std::vector< int > factors;
   factors.reserve( 34 ); // seems to be the max number of factors we'll ever get (given 31-bit limit?)

   if( n <= 5 ) {
      factors.push_back( n );
      return factors;
   }

   int f = ((( n - 1 ) ^ n ) + 1 ) >> 1;
   if( f > 1 ) {
      factors.push_back( f );
      n = f == n ? 1 : n / f;
   }
   for( f = 3; n > 1; ) {
      int d = n / f;
      if( d * f == n ) {
         factors.push_back( f );
         n = d;
      } else {
         f += 2;
         if( f * f > n ) {
            break;
         }
      }
   }
   if( n > 1 ) {
      factors.push_back( n );
   }

   f = ( factors[ 0 ] & 1 ) == 0;
   for( int i = f; i < ( int( factors.size()) + f ) / 2; i++ ) {
      std::swap( factors[ i ], factors[ factors.size() - i - 1 + f ] );
   }

   return factors;
}

} // namespace

template< typename T >
void DFT< T >::Initialize( size_t nfft, bool inverse ) {
   DIP_ASSERT( nfft <= maximumDFTSize );
   nfft_ = static_cast< int >( nfft );
   inverse_ = inverse;
   factors_ = DFTFactorize( nfft_ );
   sz_ = 0;
   {
      int ii = factors_.size() > 1 && ( factors_[ 0 ] & 1 ) == 0;
      if(( factors_[ ii ] & 1 ) != 0 && factors_[ ii ] > 5 ) {
         sz_ += ( factors_[ ii ] + 1 );
      }
   }
   itab_.resize( nfft_ );
   wave_.resize( nfft_ );

   int n = factors_[ 0 ];
   int m = 0;
   if( nfft_ <= 5 ) {
      itab_[ 0 ] = 0;
      itab_[ nfft_ - 1 ] = nfft_ - 1;

      if( nfft_ != 4 ) {
         for( int i = 1; i < nfft_ - 1; i++ ) {
            itab_[ i ] = i;
         }
      } else {
         itab_[ 1 ] = 2;
         itab_[ 2 ] = 1;
      }
      if( nfft_ == 5 ) {
         wave_[ 0 ] = { 1., 0. };
      }
      if( nfft_ != 4 ) {
         return;
      }
      m = 2;
   } else {
      // radix[] is initialized from index 'nf' down to zero
      DIP_ASSERT ( factors_.size() < 34 );
      int digits[34];
      int radix[34];
      radix[ factors_.size() ] = 1;
      digits[ factors_.size() ] = 0;
      for( size_t i = 0; i < factors_.size(); i++ ) {
         digits[ i ] = 0;
         radix[ factors_.size() - i - 1 ] = radix[ factors_.size() - i ] * factors_[ factors_.size() - i - 1 ];
      }

      if(( n & 1 ) == 0 ) {
         int a = radix[ 1 ];
         int na2 = n * a >> 1;
         int na4 = na2 >> 1;
         for( m = 0; ( unsigned )( 1 << m ) < ( unsigned )n; m++ ) {}
         if( n <= 2 ) {
            itab_[ 0 ] = 0;
            itab_[ 1 ] = na2;
         } else if( n <= 256 ) {
            int shift = 10 - m;
            for( int i = 0; i <= n - 4; i += 4 ) {
               int j = ( bitrevTab[ i >> 2 ] >> shift ) * a;
               itab_[ i ] = j;
               itab_[ i + 1 ] = j + na2;
               itab_[ i + 2 ] = j + na4;
               itab_[ i + 3 ] = j + na2 + na4;
            }
         } else {
            int shift = 34 - m;
            for( int i = 0; i < n; i += 4 ) {
               int i4 = i >> 2;
               int j = BitRev( i4, shift ) * a;
               itab_[ i ] = j;
               itab_[ i + 1 ] = j + na2;
               itab_[ i + 2 ] = j + na4;
               itab_[ i + 3 ] = j + na2 + na4;
            }
         }

         digits[ 1 ]++;

         if( factors_.size() >= 2 ) {
            for( int i = n, j = radix[ 2 ]; i < nfft_; ) {
               for( int k = 0; k < n; k++ ) {
                  itab_[ i + k ] = itab_[ k ] + j;
               }
               if(( i += n ) >= nfft_ ) {
                  break;
               }
               j += radix[ 2 ];
               for( int k = 1; ++digits[ k ] >= factors_[ k ]; k++ ) {
                  digits[ k ] = 0;
                  j += radix[ k + 2 ] - radix[ k ];
               }
            }
         }
      } else {
         for( int i = 0, j = 0;; ) {
            itab_[ i ] = j;
            if( ++i >= nfft_ ) {
               break;
            }
            j += radix[ 1 ];
            for( int k = 0; ++digits[ k ] >= factors_[ k ]; k++ ) {
               digits[ k ] = 0;
               j += radix[ k + 2 ] - radix[ k ];
            }
         }
      }
   }

   std::complex< double > w, w1;
   if(( nfft_ & ( nfft_ - 1 )) == 0 ) {
      w = w1 = DFTTab[ m ];
   } else {
      double t = sin( -dip::pi * 2 / nfft_ );
      w = w1 = { std::sqrt( 1. - t * t ), t };
   }
   n = ( nfft_ + 1 ) / 2;
   wave_[ 0 ] = { 1., 0. };
   if(( nfft_ & 1 ) == 0 ) {
      wave_[ n ] = { -1., 0. };
   }
   for( int i = 1; i < n; i++ ) {
      wave_[ i ] = w;
      wave_[ nfft_ - i ] = std::conj( w );
      w = { w.real() * w1.real() - w.imag() * w1.imag(), w.real() * w1.imag() + w.imag() * w1.real() };
   }
}

template< typename T >
void DFT< T >::Apply(
      const std::complex< T >* source,
      std::complex< T >* destination,
      std::complex< T >* buffer,
      T scale
) const {
   int n = nfft_;
   int const* itab = itab_.data();
   int nf = int( factors_.size());
   int tab_size = n;
   int n0 = n;
   int dw0 = tab_size;
   //int tab_step = tab_size == n ? 1 : tab_size == n * 2 ? 2 : tab_size / n;
   int tab_step = 1;

   // 0. shuffle data
   if( destination != source ) {
      if( !inverse_ ) {
         int i;
         for( i = 0; i <= n - 2; i += 2, itab += 2 * tab_step ) {
            int k0 = itab[ 0 ], k1 = itab[ tab_step ];
            DIP_ASSERT(( unsigned )k0 < ( unsigned )n && ( unsigned )k1 < ( unsigned )n );
            destination[ i ] = source[ k0 ];
            destination[ i + 1 ] = source[ k1 ];
         }
         if( i < n ) {
            destination[ n - 1 ] = source[ n - 1 ];
         }
      } else {
         int i;
         for( i = 0; i <= n - 2; i += 2, itab += 2 * tab_step ) {
            int k0 = itab[ 0 ], k1 = itab[ tab_step ];
            DIP_ASSERT(( unsigned )k0 < ( unsigned )n && ( unsigned )k1 < ( unsigned )n );
            destination[ i ] = std::conj( source[ k0 ] );
            destination[ i + 1 ] = std::conj( source[ k1 ] );
         }
         if( i < n ) {
            destination[ i ] = std::conj( source[ n - 1 ] );
         }
      }
   } else {
      DIP_ASSERT( factors_[ 0 ] == factors_[ nf - 1 ] );
      if( nf == 1 ) {
         if(( n & 3 ) == 0 ) {
            int n2 = n / 2;
            std::complex< T >* dsth = destination + n2;
            for( int i = 0; i < n2; i += 2, itab += tab_step * 2 ) {
               int j = itab[ 0 ];
               DIP_ASSERT(( unsigned )j < ( unsigned )n2 );

               std::swap( destination[ i + 1 ], dsth[ j ] );
               if( j > i ) {
                  std::swap( destination[ i ], destination[ j ] );
                  std::swap( dsth[ i + 1 ], dsth[ j + 1 ] );
               }
            }
         }
         // else do nothing
      } else {
         for( int i = 0; i < n; i++, itab += tab_step ) {
            int j = itab[ 0 ];
            DIP_ASSERT(( unsigned )j < ( unsigned )n );
            if( j > i ) {
               std::swap( destination[ i ], destination[ j ] );
            }
         }
      }
      if( inverse_ ) {
         int i;
         for( i = 0; i <= n - 2; i += 2 ) {
            T t0 = -destination[ i ].imag();
            T t1 = -destination[ i + 1 ].imag();
            destination[ i ].imag( t0 );
            destination[ i + 1 ].imag( t1 );
         }

         if( i < n ) {
            destination[ n - 1 ].imag( -destination[ n - 1 ].imag());
         }
      }
   }

   n = 1;
   // 1. power-2 transforms
   if(( factors_[ 0 ] & 1 ) == 0 ) {
      // radix-4 transform
      for( ; n * 4 <= factors_[ 0 ]; ) {
         int nx = n;
         n *= 4;
         dw0 /= 4;
         for( int i = 0; i < n0; i += n ) {
            std::complex< T >* v0 = destination + i;
            std::complex< T >* v1 = v0 + nx * 2;
            std::complex< T > t0 = v1[ 0 ];
            std::complex< T > t4 = v1[ nx ];
            std::complex< T > t1 = t0 + t4;
            std::complex< T > t3 = { t0.imag() - t4.imag(), t4.real() - t0.real() };
            std::complex< T > t2 = v0[ 0 ];
            t4 = v0[ nx ];
            t0 = t2 + t4;
            t2 -= t4;
            v0[ 0 ] = t0 + t1;
            v1[ 0 ] = t0 - t1;
            v0[ nx ] = t2 + t3;
            v1[ nx ] = t2 - t3;
            for( int j = 1, dw = dw0; j < nx; j++, dw += dw0 ) {
               v0 = destination + i + j;
               v1 = v0 + nx * 2;
               t2 = {
                     v0[ nx ].real() * wave_[ dw * 2 ].real() - v0[ nx ].imag() * wave_[ dw * 2 ].imag(),
                     v0[ nx ].real() * wave_[ dw * 2 ].imag() + v0[ nx ].imag() * wave_[ dw * 2 ].real()
               };
               t0 = {
                     v1[ 0 ].real() * wave_[ dw ].imag() + v1[ 0 ].imag() * wave_[ dw ].real(),
                     v1[ 0 ].real() * wave_[ dw ].real() - v1[ 0 ].imag() * wave_[ dw ].imag()
               };
               t3 = {
                     v1[ nx ].real() * wave_[ dw * 3 ].imag() + v1[ nx ].imag() * wave_[ dw * 3 ].real(),
                     v1[ nx ].real() * wave_[ dw * 3 ].real() - v1[ nx ].imag() * wave_[ dw * 3 ].imag()
               };
               t1 = { t0.imag() + t3.imag(), t0.real() + t3.real() };
               t3 = std::conj( t0 - t3 );
               t4 = v0[ 0 ];
               t0 = t4 + t2;
               t2 = t4 - t2;
               v0[ 0 ] = t0 + t1;
               v1[ 0 ] = t0 - t1;
               v0[ nx ] = t2 + t3;
               v1[ nx ] = t2 - t3;
            }
         }
      }

      for( ; n < factors_[ 0 ]; ) {
         // do the remaining radix-2 transform
         int nx = n;
         n *= 2;
         dw0 /= 2;
         for( int i = 0; i < n0; i += n ) {
            std::complex< T >* v = destination + i;
            std::complex< T > t0 = v[ 0 ];
            std::complex< T > t1 = v[ nx ];
            v[ 0 ] = t0 + t1;
            v[ nx ] = t0 - t1;
            for( int j = 1, dw = dw0; j < nx; j++, dw += dw0 ) {
               v = destination + i + j;
               t1 = {
                     v[ nx ].real() * wave_[ dw ].real() - v[ nx ].imag() * wave_[ dw ].imag(),
                     v[ nx ].imag() * wave_[ dw ].real() + v[ nx ].real() * wave_[ dw ].imag()
               };
               t0 = v[ 0 ];
               v[ 0 ] = t0 + t1;
               v[ nx ] = t0 - t1;
            }
         }
      }
   }

   // 2. all the other transforms
   for( int f_idx = ( factors_[ 0 ] & 1 ) ? 0 : 1; f_idx < nf; f_idx++ ) {
      int factor = factors_[ f_idx ];
      int nx = n;
      n *= factor;
      dw0 /= factor;
      if( factor == 3 ) {
         // radix-3
         static const T sin_120 = T( 0.86602540378443864676372317075294 );
         for( int i = 0; i < n0; i += n ) {
            std::complex< T >* v = destination + i;
            std::complex< T > t1 = v[ nx ] + v[ nx * 2 ];
            std::complex< T > t0 = v[ 0 ];
            std::complex< T > t2 = {
                  sin_120 * ( v[ nx ].imag() - v[ nx * 2 ].imag()),
                  sin_120 * ( v[ nx * 2 ].real() - v[ nx ].real())
            };
            v[ 0 ] = t0 + t1;
            t0 -= T( 0.5 ) * t1;
            v[ nx ] = t0 + t2;
            v[ nx * 2 ] = t0 - t2;
            for( int j = 1, dw = dw0; j < nx; j++, dw += dw0 ) {
               v = destination + i + j;
               t0 = {
                     v[ nx ].real() * wave_[ dw ].real() - v[ nx ].imag() * wave_[ dw ].imag(),
                     v[ nx ].real() * wave_[ dw ].imag() + v[ nx ].imag() * wave_[ dw ].real()
               };
               t2 = {
                     v[ nx * 2 ].real() * wave_[ dw * 2 ].imag() + v[ nx * 2 ].imag() * wave_[ dw * 2 ].real(),
                     v[ nx * 2 ].real() * wave_[ dw * 2 ].real() - v[ nx * 2 ].imag() * wave_[ dw * 2 ].imag()
               };
               t1 = { t0.real() + t2.imag(), t0.imag() + t2.real() };
               t2 = { t0.imag() - t2.real(), t2.imag() - t0.real() };
               t2 *= sin_120;
               t0 = v[ 0 ];
               v[ 0 ] = t0 + t1;
               t0 -= T( 0.5 ) * t1;
               v[ nx ] = t0 + t2;
               v[ nx * 2 ] = t0 - t2;
            }
         }
      } else if( factor == 5 ) {
         // radix-5
         static const T fft5_2 = T( 0.559016994374947424102293417182819 );
         static const T fft5_3 = T( -0.951056516295153572116439333379382 );
         static const T fft5_4 = T( -1.538841768587626701285145288018455 );
         static const T fft5_5 = T( 0.363271264002680442947733378740309 );
         for( int i = 0; i < n0; i += n ) {
            for( int j = 0, dw = 0; j < nx; j++, dw += dw0 ) {
               std::complex< T >* v0 = destination + i + j;
               std::complex< T >* v1 = v0 + nx * 2;
               std::complex< T >* v2 = v1 + nx * 2;
               std::complex< T > t3 = {
                     v0[ nx ].real() * wave_[ dw ].real() - v0[ nx ].imag() * wave_[ dw ].imag(),
                     v0[ nx ].real() * wave_[ dw ].imag() + v0[ nx ].imag() * wave_[ dw ].real()
               };
               std::complex< T > t2 = {
                     v2[ 0 ].real() * wave_[ dw * 4 ].real() - v2[ 0 ].imag() * wave_[ dw * 4 ].imag(),
                     v2[ 0 ].real() * wave_[ dw * 4 ].imag() + v2[ 0 ].imag() * wave_[ dw * 4 ].real()
               };
               std::complex< T > t1 = t3 + t2;
               t3 -= t2;
               std::complex< T > t4 = {
                     v1[ nx ].real() * wave_[ dw * 3 ].real() - v1[ nx ].imag() * wave_[ dw * 3 ].imag(),
                     v1[ nx ].real() * wave_[ dw * 3 ].imag() + v1[ nx ].imag() * wave_[ dw * 3 ].real()
               };
               std::complex< T > t0 = {
                     v1[ 0 ].real() * wave_[ dw * 2 ].real() - v1[ 0 ].imag() * wave_[ dw * 2 ].imag(),
                     v1[ 0 ].real() * wave_[ dw * 2 ].imag() + v1[ 0 ].imag() * wave_[ dw * 2 ].real()
               };
               t2 = t4 + t0;
               t4 -= t0;
               t0 = v0[ 0 ];
               std::complex< T > t5 = t1 + t2;
               v0[ 0 ] = t0 + t5;
               t0 -= T( 0.25 ) * t5;
               t1 = fft5_2 * ( t1 - t2 );
               t2 = { -fft5_3 * ( t3.imag() + t4.imag()), fft5_3 * ( t3.real() + t4.real()) };
               t3 = { fft5_5 * t3.real(), -fft5_5 * t3.imag() };
               t4 = { fft5_4 * t4.real(), -fft5_4 * t4.imag() };
               t5 = { t2.real() + t3.imag(), t2.imag() + t3.real() };
               t2 = { t2.real() - t4.imag(), t2.imag() - t4.real() };
               t3 = t0 + t1;
               t0 -= t1;
               v0[ nx ] = t3 + t2;
               v2[ 0 ] = t3 - t2;
               v1[ 0 ] = t0 + t5;
               v1[ nx ] = t0 - t5;
            }
         }
      } else {
         // radix-"factor" - an odd number
         int factor2 = ( factor - 1 ) / 2;
         int dw_f = tab_size / factor;
         std::complex< T >* a = buffer;
         std::complex< T >* b = buffer + factor2;
         for( int i = 0; i < n0; i += n ) {
            for( int j = 0, dw = 0; j < nx; j++, dw += dw0 ) {
               std::complex< T >* v = destination + i + j;
               std::complex< T > v_0 = v[ 0 ];
               std::complex< T > vn_0 = v_0;
               if( j == 0 ) {
                  for( int p = 1, k = nx; p <= factor2; p++, k += nx ) {
                     std::complex< T > t0 = v[ k ] + std::conj( v[ n - k ] );
                     std::complex< T > t1 = v[ k ] - std::conj( v[ n - k ] );
                     vn_0 += std::complex< T >{ t0.real(), t1.imag() };
                     a[ p - 1 ] = t0;
                     b[ p - 1 ] = t1;
                  }
               } else {
                  const std::complex< T >* wave = wave_.data() + dw * factor;
                  for( int p = 1, k = nx, d = dw; p <= factor2; p++, k += nx, d += dw ) {
                     std::complex< T > t2 = {
                           v[ k ].real() * wave_[ d ].real() - v[ k ].imag() * wave_[ d ].imag(),
                           v[ k ].real() * wave_[ d ].imag() + v[ k ].imag() * wave_[ d ].real()
                     };
                     std::complex< T > t1 = {
                           v[ n - k ].real() * wave[ -d ].real() - v[ n - k ].imag() * wave[ -d ].imag(),
                           v[ n - k ].real() * wave[ -d ].imag() + v[ n - k ].imag() * wave[ -d ].real()
                     };
                     std::complex< T > t0 = t2 + std::conj( t1 );
                     t1 = t2 - std::conj( t1 );
                     vn_0 += std::complex< T >{ t0.real(), t1.imag() };
                     a[ p - 1 ] = t0;
                     b[ p - 1 ] = t1;
                  }
               }
               v[ 0 ] = vn_0;
               for( int p = 1, k = nx; p <= factor2; p++, k += nx ) {
                  std::complex< T > s0 = v_0;
                  std::complex< T > s1 = v_0;
                  int d = dw_f * p;
                  int dd = d;
                  for( int q = 0; q < factor2; q++ ) {
                     std::complex< T > t0 = { wave_[ d ].real() * a[ q ].real(), wave_[ d ].imag() * a[ q ].imag() };
                     std::complex< T > t1 = { wave_[ d ].real() * b[ q ].imag(), wave_[ d ].imag() * b[ q ].real() };
                     s1 += std::complex< T >{ t0.real() + t0.imag(), t1.real() - t1.imag() };
                     s0 += std::complex< T >{ t0.real() - t0.imag(), t1.real() + t1.imag() };
                     d += dd;
                     d -= -( d >= tab_size ) & tab_size;
                  }
                  v[ k ] = s0;
                  v[ n - k ] = s1;
               }
            }
         }
      }
   }

   if( scale != 1 ) {
      T re_scale = scale;
      T im_scale = scale;
      if( inverse_ ) {
         im_scale = -im_scale;
      }
      for( int i = 0; i < n0; i++ ) {
         destination[ i ] = { destination[ i ].real() * re_scale, destination[ i ].imag() * im_scale };
      }
   } else if( inverse_ ) {
      int i;
      for( i = 0; i <= n0 - 2; i += 2 ) {
         T t0 = -destination[ i ].imag();
         T t1 = -destination[ i + 1 ].imag();
         destination[ i ].imag( t0 );
         destination[ i + 1 ].imag( t1 );
      }
      if( i < n0 ) {
         destination[ n0 - 1 ].imag( -destination[ n0 - 1 ].imag());
      }
   }
}

// Explicit instantiations:
template void DFT< float >::Initialize( size_t nfft, bool inverse );
template void DFT< double >::Initialize( size_t nfft, bool inverse );
template void DFT< float >::Apply(
      const std::complex< float >* src,
      std::complex< float >* dst,
      std::complex< float >* buf,
      float scale
) const;
template void DFT< double >::Apply(
      const std::complex< double >* src,
      std::complex< double >* dst,
      std::complex< double >* buf,
      double scale
) const;

namespace {

constexpr unsigned int optimalDFTSizeTab[] = {
      1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, 24, 25, 27, 30, 32, 36, 40, 45, 48,
      50, 54, 60, 64, 72, 75, 80, 81, 90, 96, 100, 108, 120, 125, 128, 135, 144, 150, 160,
      162, 180, 192, 200, 216, 225, 240, 243, 250, 256, 270, 288, 300, 320, 324, 360, 375,
      384, 400, 405, 432, 450, 480, 486, 500, 512, 540, 576, 600, 625, 640, 648, 675, 720,
      729, 750, 768, 800, 810, 864, 900, 960, 972, 1000, 1024, 1080, 1125, 1152, 1200,
      1215, 1250, 1280, 1296, 1350, 1440, 1458, 1500, 1536, 1600, 1620, 1728, 1800, 1875,
      1920, 1944, 2000, 2025, 2048, 2160, 2187, 2250, 2304, 2400, 2430, 2500, 2560, 2592,
      2700, 2880, 2916, 3000, 3072, 3125, 3200, 3240, 3375, 3456, 3600, 3645, 3750, 3840,
      3888, 4000, 4050, 4096, 4320, 4374, 4500, 4608, 4800, 4860, 5000, 5120, 5184, 5400,
      5625, 5760, 5832, 6000, 6075, 6144, 6250, 6400, 6480, 6561, 6750, 6912, 7200, 7290,
      7500, 7680, 7776, 8000, 8100, 8192, 8640, 8748, 9000, 9216, 9375, 9600, 9720, 10000,
      10125, 10240, 10368, 10800, 10935, 11250, 11520, 11664, 12000, 12150, 12288, 12500,
      12800, 12960, 13122, 13500, 13824, 14400, 14580, 15000, 15360, 15552, 15625, 16000,
      16200, 16384, 16875, 17280, 17496, 18000, 18225, 18432, 18750, 19200, 19440, 19683,
      20000, 20250, 20480, 20736, 21600, 21870, 22500, 23040, 23328, 24000, 24300, 24576,
      25000, 25600, 25920, 26244, 27000, 27648, 28125, 28800, 29160, 30000, 30375, 30720,
      31104, 31250, 32000, 32400, 32768, 32805, 33750, 34560, 34992, 36000, 36450, 36864,
      37500, 38400, 38880, 39366, 40000, 40500, 40960, 41472, 43200, 43740, 45000, 46080,
      46656, 46875, 48000, 48600, 49152, 50000, 50625, 51200, 51840, 52488, 54000, 54675,
      55296, 56250, 57600, 58320, 59049, 60000, 60750, 61440, 62208, 62500, 64000, 64800,
      65536, 65610, 67500, 69120, 69984, 72000, 72900, 73728, 75000, 76800, 77760, 78125,
      78732, 80000, 81000, 81920, 82944, 84375, 86400, 87480, 90000, 91125, 92160, 93312,
      93750, 96000, 97200, 98304, 98415, 100000, 101250, 102400, 103680, 104976, 108000,
      109350, 110592, 112500, 115200, 116640, 118098, 120000, 121500, 122880, 124416, 125000,
      128000, 129600, 131072, 131220, 135000, 138240, 139968, 140625, 144000, 145800, 147456,
      150000, 151875, 153600, 155520, 156250, 157464, 160000, 162000, 163840, 164025, 165888,
      168750, 172800, 174960, 177147, 180000, 182250, 184320, 186624, 187500, 192000, 194400,
      196608, 196830, 200000, 202500, 204800, 207360, 209952, 216000, 218700, 221184, 225000,
      230400, 233280, 234375, 236196, 240000, 243000, 245760, 248832, 250000, 253125, 256000,
      259200, 262144, 262440, 270000, 273375, 276480, 279936, 281250, 288000, 291600, 294912,
      295245, 300000, 303750, 307200, 311040, 312500, 314928, 320000, 324000, 327680, 328050,
      331776, 337500, 345600, 349920, 354294, 360000, 364500, 368640, 373248, 375000, 384000,
      388800, 390625, 393216, 393660, 400000, 405000, 409600, 414720, 419904, 421875, 432000,
      437400, 442368, 450000, 455625, 460800, 466560, 468750, 472392, 480000, 486000, 491520,
      492075, 497664, 500000, 506250, 512000, 518400, 524288, 524880, 531441, 540000, 546750,
      552960, 559872, 562500, 576000, 583200, 589824, 590490, 600000, 607500, 614400, 622080,
      625000, 629856, 640000, 648000, 655360, 656100, 663552, 675000, 691200, 699840, 703125,
      708588, 720000, 729000, 737280, 746496, 750000, 759375, 768000, 777600, 781250, 786432,
      787320, 800000, 810000, 819200, 820125, 829440, 839808, 843750, 864000, 874800, 884736,
      885735, 900000, 911250, 921600, 933120, 937500, 944784, 960000, 972000, 983040, 984150,
      995328, 1000000, 1012500, 1024000, 1036800, 1048576, 1049760, 1062882, 1080000, 1093500,
      1105920, 1119744, 1125000, 1152000, 1166400, 1171875, 1179648, 1180980, 1200000,
      1215000, 1228800, 1244160, 1250000, 1259712, 1265625, 1280000, 1296000, 1310720,
      1312200, 1327104, 1350000, 1366875, 1382400, 1399680, 1406250, 1417176, 1440000,
      1458000, 1474560, 1476225, 1492992, 1500000, 1518750, 1536000, 1555200, 1562500,
      1572864, 1574640, 1594323, 1600000, 1620000, 1638400, 1640250, 1658880, 1679616,
      1687500, 1728000, 1749600, 1769472, 1771470, 1800000, 1822500, 1843200, 1866240,
      1875000, 1889568, 1920000, 1944000, 1953125, 1966080, 1968300, 1990656, 2000000,
      2025000, 2048000, 2073600, 2097152, 2099520, 2109375, 2125764, 2160000, 2187000,
      2211840, 2239488, 2250000, 2278125, 2304000, 2332800, 2343750, 2359296, 2361960,
      2400000, 2430000, 2457600, 2460375, 2488320, 2500000, 2519424, 2531250, 2560000,
      2592000, 2621440, 2624400, 2654208, 2657205, 2700000, 2733750, 2764800, 2799360,
      2812500, 2834352, 2880000, 2916000, 2949120, 2952450, 2985984, 3000000, 3037500,
      3072000, 3110400, 3125000, 3145728, 3149280, 3188646, 3200000, 3240000, 3276800,
      3280500, 3317760, 3359232, 3375000, 3456000, 3499200, 3515625, 3538944, 3542940,
      3600000, 3645000, 3686400, 3732480, 3750000, 3779136, 3796875, 3840000, 3888000,
      3906250, 3932160, 3936600, 3981312, 4000000, 4050000, 4096000, 4100625, 4147200,
      4194304, 4199040, 4218750, 4251528, 4320000, 4374000, 4423680, 4428675, 4478976,
      4500000, 4556250, 4608000, 4665600, 4687500, 4718592, 4723920, 4782969, 4800000,
      4860000, 4915200, 4920750, 4976640, 5000000, 5038848, 5062500, 5120000, 5184000,
      5242880, 5248800, 5308416, 5314410, 5400000, 5467500, 5529600, 5598720, 5625000,
      5668704, 5760000, 5832000, 5859375, 5898240, 5904900, 5971968, 6000000, 6075000,
      6144000, 6220800, 6250000, 6291456, 6298560, 6328125, 6377292, 6400000, 6480000,
      6553600, 6561000, 6635520, 6718464, 6750000, 6834375, 6912000, 6998400, 7031250,
      7077888, 7085880, 7200000, 7290000, 7372800, 7381125, 7464960, 7500000, 7558272,
      7593750, 7680000, 7776000, 7812500, 7864320, 7873200, 7962624, 7971615, 8000000,
      8100000, 8192000, 8201250, 8294400, 8388608, 8398080, 8437500, 8503056, 8640000,
      8748000, 8847360, 8857350, 8957952, 9000000, 9112500, 9216000, 9331200, 9375000,
      9437184, 9447840, 9565938, 9600000, 9720000, 9765625, 9830400, 9841500, 9953280,
      10000000, 10077696, 10125000, 10240000, 10368000, 10485760, 10497600, 10546875, 10616832,
      10628820, 10800000, 10935000, 11059200, 11197440, 11250000, 11337408, 11390625, 11520000,
      11664000, 11718750, 11796480, 11809800, 11943936, 12000000, 12150000, 12288000, 12301875,
      12441600, 12500000, 12582912, 12597120, 12656250, 12754584, 12800000, 12960000, 13107200,
      13122000, 13271040, 13286025, 13436928, 13500000, 13668750, 13824000, 13996800, 14062500,
      14155776, 14171760, 14400000, 14580000, 14745600, 14762250, 14929920, 15000000, 15116544,
      15187500, 15360000, 15552000, 15625000, 15728640, 15746400, 15925248, 15943230, 16000000,
      16200000, 16384000, 16402500, 16588800, 16777216, 16796160, 16875000, 17006112, 17280000,
      17496000, 17578125, 17694720, 17714700, 17915904, 18000000, 18225000, 18432000, 18662400,
      18750000, 18874368, 18895680, 18984375, 19131876, 19200000, 19440000, 19531250, 19660800,
      19683000, 19906560, 20000000, 20155392, 20250000, 20480000, 20503125, 20736000, 20971520,
      20995200, 21093750, 21233664, 21257640, 21600000, 21870000, 22118400, 22143375, 22394880,
      22500000, 22674816, 22781250, 23040000, 23328000, 23437500, 23592960, 23619600, 23887872,
      23914845, 24000000, 24300000, 24576000, 24603750, 24883200, 25000000, 25165824, 25194240,
      25312500, 25509168, 25600000, 25920000, 26214400, 26244000, 26542080, 26572050, 26873856,
      27000000, 27337500, 27648000, 27993600, 28125000, 28311552, 28343520, 28800000, 29160000,
      29296875, 29491200, 29524500, 29859840, 30000000, 30233088, 30375000, 30720000, 31104000,
      31250000, 31457280, 31492800, 31640625, 31850496, 31886460, 32000000, 32400000, 32768000,
      32805000, 33177600, 33554432, 33592320, 33750000, 34012224, 34171875, 34560000, 34992000,
      35156250, 35389440, 35429400, 35831808, 36000000, 36450000, 36864000, 36905625, 37324800,
      37500000, 37748736, 37791360, 37968750, 38263752, 38400000, 38880000, 39062500, 39321600,
      39366000, 39813120, 39858075, 40000000, 40310784, 40500000, 40960000, 41006250, 41472000,
      41943040, 41990400, 42187500, 42467328, 42515280, 43200000, 43740000, 44236800, 44286750,
      44789760, 45000000, 45349632, 45562500, 46080000, 46656000, 46875000, 47185920, 47239200,
      47775744, 47829690, 48000000, 48600000, 48828125, 49152000, 49207500, 49766400, 50000000,
      50331648, 50388480, 50625000, 51018336, 51200000, 51840000, 52428800, 52488000, 52734375,
      53084160, 53144100, 53747712, 54000000, 54675000, 55296000, 55987200, 56250000, 56623104,
      56687040, 56953125, 57600000, 58320000, 58593750, 58982400, 59049000, 59719680, 60000000,
      60466176, 60750000, 61440000, 61509375, 62208000, 62500000, 62914560, 62985600, 63281250,
      63700992, 63772920, 64000000, 64800000, 65536000, 65610000, 66355200, 66430125, 67108864,
      67184640, 67500000, 68024448, 68343750, 69120000, 69984000, 70312500, 70778880, 70858800,
      71663616, 72000000, 72900000, 73728000, 73811250, 74649600, 75000000, 75497472, 75582720,
      75937500, 76527504, 76800000, 77760000, 78125000, 78643200, 78732000, 79626240, 79716150,
      80000000, 80621568, 81000000, 81920000, 82012500, 82944000, 83886080, 83980800, 84375000,
      84934656, 85030560, 86400000, 87480000, 87890625, 88473600, 88573500, 89579520, 90000000,
      90699264, 91125000, 92160000, 93312000, 93750000, 94371840, 94478400, 94921875, 95551488,
      95659380, 96000000, 97200000, 97656250, 98304000, 98415000, 99532800, 100000000,
      100663296, 100776960, 101250000, 102036672, 102400000, 102515625, 103680000, 104857600,
      104976000, 105468750, 106168320, 106288200, 107495424, 108000000, 109350000, 110592000,
      110716875, 111974400, 112500000, 113246208, 113374080, 113906250, 115200000, 116640000,
      117187500, 117964800, 118098000, 119439360, 119574225, 120000000, 120932352, 121500000,
      122880000, 123018750, 124416000, 125000000, 125829120, 125971200, 126562500, 127401984,
      127545840, 128000000, 129600000, 131072000, 131220000, 132710400, 132860250, 134217728,
      134369280, 135000000, 136048896, 136687500, 138240000, 139968000, 140625000, 141557760,
      141717600, 143327232, 144000000, 145800000, 146484375, 147456000, 147622500, 149299200,
      150000000, 150994944, 151165440, 151875000, 153055008, 153600000, 155520000, 156250000,
      157286400, 157464000, 158203125, 159252480, 159432300, 160000000, 161243136, 162000000,
      163840000, 164025000, 165888000, 167772160, 167961600, 168750000, 169869312, 170061120,
      170859375, 172800000, 174960000, 175781250, 176947200, 177147000, 179159040, 180000000,
      181398528, 182250000, 184320000, 184528125, 186624000, 187500000, 188743680, 188956800,
      189843750, 191102976, 191318760, 192000000, 194400000, 195312500, 196608000, 196830000,
      199065600, 199290375, 200000000, 201326592, 201553920, 202500000, 204073344, 204800000,
      205031250, 207360000, 209715200, 209952000, 210937500, 212336640, 212576400, 214990848,
      216000000, 218700000, 221184000, 221433750, 223948800, 225000000, 226492416, 226748160,
      227812500, 230400000, 233280000, 234375000, 235929600, 236196000, 238878720, 239148450,
      240000000, 241864704, 243000000, 244140625, 245760000, 246037500, 248832000, 250000000,
      251658240, 251942400, 253125000, 254803968, 255091680, 256000000, 259200000, 262144000,
      262440000, 263671875, 265420800, 265720500, 268435456, 268738560, 270000000, 272097792,
      273375000, 276480000, 279936000, 281250000, 283115520, 283435200, 284765625, 286654464,
      288000000, 291600000, 292968750, 294912000, 295245000, 298598400, 300000000, 301989888,
      302330880, 303750000, 306110016, 307200000, 307546875, 311040000, 312500000, 314572800,
      314928000, 316406250, 318504960, 318864600, 320000000, 322486272, 324000000, 327680000,
      328050000, 331776000, 332150625, 335544320, 335923200, 337500000, 339738624, 340122240,
      341718750, 345600000, 349920000, 351562500, 353894400, 354294000, 358318080, 360000000,
      362797056, 364500000, 368640000, 369056250, 373248000, 375000000, 377487360, 377913600,
      379687500, 382205952, 382637520, 384000000, 388800000, 390625000, 393216000, 393660000,
      398131200, 398580750, 400000000, 402653184, 403107840, 405000000, 408146688, 409600000,
      410062500, 414720000, 419430400, 419904000, 421875000, 424673280, 425152800, 429981696,
      432000000, 437400000, 439453125, 442368000, 442867500, 447897600, 450000000, 452984832,
      453496320, 455625000, 460800000, 466560000, 468750000, 471859200, 472392000, 474609375,
      477757440, 478296900, 480000000, 483729408, 486000000, 488281250, 491520000, 492075000,
      497664000, 500000000, 503316480, 503884800, 506250000, 509607936, 510183360, 512000000,
      512578125, 518400000, 524288000, 524880000, 527343750, 530841600, 531441000, 536870912,
      537477120, 540000000, 544195584, 546750000, 552960000, 553584375, 559872000, 562500000,
      566231040, 566870400, 569531250, 573308928, 576000000, 583200000, 585937500, 589824000,
      590490000, 597196800, 597871125, 600000000, 603979776, 604661760, 607500000, 612220032,
      614400000, 615093750, 622080000, 625000000, 629145600, 629856000, 632812500, 637009920,
      637729200, 640000000, 644972544, 648000000, 655360000, 656100000, 663552000, 664301250,
      671088640, 671846400, 675000000, 679477248, 680244480, 683437500, 691200000, 699840000,
      703125000, 707788800, 708588000, 716636160, 720000000, 725594112, 729000000, 732421875,
      737280000, 738112500, 746496000, 750000000, 754974720, 755827200, 759375000, 764411904,
      765275040, 768000000, 777600000, 781250000, 786432000, 787320000, 791015625, 796262400,
      797161500, 800000000, 805306368, 806215680, 810000000, 816293376, 819200000, 820125000,
      829440000, 838860800, 839808000, 843750000, 849346560, 850305600, 854296875, 859963392,
      864000000, 874800000, 878906250, 884736000, 885735000, 895795200, 900000000, 905969664,
      906992640, 911250000, 921600000, 922640625, 933120000, 937500000, 943718400, 944784000,
      949218750, 955514880, 956593800, 960000000, 967458816, 972000000, 976562500, 983040000,
      984150000, 995328000, 996451875, 1000000000, 1006632960, 1007769600, 1012500000,
      1019215872, 1020366720, 1024000000, 1025156250, 1036800000, 1048576000, 1049760000,
      1054687500, 1061683200, 1062882000, 1073741824, 1074954240, 1080000000, 1088391168,
      1093500000, 1105920000, 1107168750, 1119744000, 1125000000, 1132462080, 1133740800,
      1139062500, 1146617856, 1152000000, 1166400000, 1171875000, 1179648000, 1180980000,
      1194393600, 1195742250, 1200000000, 1207959552, 1209323520, 1215000000, 1220703125,
      1224440064, 1228800000, 1230187500, 1244160000, 1250000000, 1258291200, 1259712000,
      1265625000, 1274019840, 1275458400, 1280000000, 1289945088, 1296000000, 1310720000,
      1312200000, 1318359375, 1327104000, 1328602500, 1342177280, 1343692800, 1350000000,
      1358954496, 1360488960, 1366875000, 1382400000, 1399680000, 1406250000, 1415577600,
      1417176000, 1423828125, 1433272320, 1440000000, 1451188224, 1458000000, 1464843750,
      1474560000, 1476225000, 1492992000, 1500000000, 1509949440, 1511654400, 1518750000,
      1528823808, 1530550080, 1536000000, 1537734375, 1555200000, 1562500000, 1572864000,
      1574640000, 1582031250, 1592524800, 1594323000, 1600000000, 1610612736, 1612431360,
      1620000000, 1632586752, 1638400000, 1640250000, 1658880000, 1660753125, 1677721600,
      1679616000, 1687500000, 1698693120, 1700611200, 1708593750, 1719926784, 1728000000,
      1749600000, 1757812500, 1769472000, 1771470000, 1791590400, 1800000000, 1811939328,
      1813985280, 1822500000, 1843200000, 1845281250, 1866240000, 1875000000, 1887436800,
      1889568000, 1898437500, 1911029760, 1913187600, 1920000000, 1934917632, 1944000000,
      1953125000, 1966080000, 1968300000, 1990656000, 1992903750, 2000000000, 2013265920,
      2015539200, 2025000000, 2038431744, 2040733440, 2048000000, 2050312500, 2073600000,
      2097152000, 2099520000, 2109375000, 2123366400, 2125764000
};

} // namespace

size_t GetOptimalDFTSize( size_t size0 ) {
   size_t a = 0;
   size_t b = sizeof( optimalDFTSizeTab ) / sizeof( optimalDFTSizeTab[ 0 ] ) - 1;
   if( size0 > optimalDFTSizeTab[ b ] ) {
      return 0; // 0 indicates an error condition -- of course we don't do DFT on an empty array!
   }
   while( a < b ) {
      size_t c = ( a + b ) >> 1;
      if( size0 <= optimalDFTSizeTab[ c ] ) {
         b = c;
      } else {
         a = c + 1;
      }
   }
   return optimalDFTSizeTab[ b ];
}

} // namespace dip


#if defined(__GNUG__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
