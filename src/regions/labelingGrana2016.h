// Copyright(c) 2016 - 2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// * Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of YACCLAB nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Taken from YACCLAB: Yet Another Connected Components Labeling Benchmark
// https://github.com/prittt/YACCLAB
// Modifications (c)2017 Cris Luengo:
//    - Only keept the function that does the first pass
//    - Renamed it to dip::LabelFirstPass_Grana2016, added namespaces
//    - Replaced OpenCV types with DIPlib types
//    - Replaced indexing with pre-computed offsets and pointer arithmetic, and using strides
//    - Using my own dip::UnionFind class instead of the functions provided with YACCLAB
//    - Recording region size

#include <functional>

#include "diplib.h"
#include "diplib/regions.h"
#include "diplib/union_find.h"

namespace dip {

using LabelRegionList = UnionFind< LabelType, dip::uint, std::plus< dip::uint >>;

namespace {

void LabelFirstPass_Grana2016( Image const& c_in, Image& c_out, LabelRegionList& regions ) {
   dip::sint w = static_cast< dip::sint >( c_in.Size( 0 ));
   dip::sint h = static_cast< dip::sint >( c_in.Size( 1 ));
   dip::sint xInStride = c_in.Stride( 0 );
   dip::sint yInStride = c_in.Stride( 1 );
   dip::sint xOutStride = c_out.Stride( 0 );
   dip::sint yOutStride = c_out.Stride( 1 );
   if( yInStride < xInStride ) {
      // Let's process the image the other way around, it'll be faster
      std::swap( w, h );
      std::swap( xInStride, yInStride );
      std::swap( xOutStride, yOutStride );
   }
   bin const* in = static_cast< bin const* >( c_in.Origin() );
   LabelType* out = static_cast< LabelType* >( c_out.Origin() );

   dip::sint offsetIn_p = -yInStride - xInStride;
   dip::sint offsetIn_q = -yInStride;
   dip::sint offsetIn_r = -yInStride + xInStride;
   dip::sint offsetOut_s = -xOutStride;
   dip::sint offsetOut_p = -yOutStride - xOutStride;
   dip::sint offsetOut_q = -yOutStride;
   dip::sint offsetOut_r = -yOutStride + xOutStride;

   dip::sint x = -1;
   dip::sint y = 0;

   bin const* pin = in - xInStride;
   LabelType* pout = out - xOutStride;

tree_A0:
   if( pin += xInStride, pout += xOutStride, ++x >= w ) {
      goto break_A0;
   }
   if( *pin ) {
      // x = new label
      *pout = regions.Create( 1 );
      goto tree_B0;
   } else {
      // nothing
      goto tree_A0;
   }
tree_B0:
   if( pin += xInStride, pout += xOutStride, ++x >= w ) {
      goto break_B0;
   }
   if( *pin ) {
      ++regions.Value( *pout = pout[ offsetOut_s ] ); // x = s
      goto tree_B0;
   } else {
      // nothing
      goto tree_A0;
   }
break_A0:
break_B0:
   ;

   for( y = 1; y < h; ++y ) {
      in += yInStride;
      out += yOutStride;
      // First column
      x = 0;
      pin = in;
      pout = out;
      if( *pin ) {
         if( pin[ offsetIn_q ] ) {
            ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
            goto tree_A;
         } else {
            if( pin[ offsetIn_r ] ) {
               ++regions.Value( *pout = pout[ offsetOut_r ] ); // x = r
               goto tree_B;
            } else {
               // x = new label
               *pout = regions.Create( 1 );
               goto tree_C;
            }
         }
      } else {
         // nothing
         goto tree_D;
      }

   tree_A:
      if( pin += xInStride, pout += xOutStride, ++x >= w - 1 ) {
         goto break_A;
      }
      if( *pin ) {
         if( pin[ offsetIn_q ] ) {
            ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
            goto tree_A;
         } else {
            if( pin[ offsetIn_r ] ) {
               ++regions.Value( *pout = regions.Union( pout[ offsetOut_r ], pout[ offsetOut_s ] )); // x = r + s
               goto tree_B;
            } else {
               ++regions.Value( *pout = pout[ offsetOut_s ] ); // x = s
               goto tree_C;
            }
         }
      } else {
         // nothing
         goto tree_D;
      }
   tree_B:
      if( pin += xInStride, pout += xOutStride, ++x >= w - 1 ) {
         goto break_B;
      }
      if( *pin ) {
         ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
         goto tree_A;
      } else {
         // nothing
         goto tree_D;
      }
   tree_C:
      if( pin += xInStride, pout += xOutStride, ++x >= w - 1 ) {
         goto break_C;
      }
      if( *pin ) {
         if( pin[ offsetIn_r ] ) {
            ++regions.Value( *pout = regions.Union( pout[ offsetOut_r ], pout[ offsetOut_s ] )); // x = r + s
            goto tree_B;
         } else {
            ++regions.Value( *pout = pout[ offsetOut_s ] ); // x = s
            goto tree_C;
         }
      } else {
         // nothing
         goto tree_D;
      }
   tree_D:
      if( pin += xInStride, pout += xOutStride, ++x >= w - 1 ) {
         goto break_D;
      }
      if( *pin ) {
         if( pin[ offsetIn_q ] ) {
            ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
            goto tree_A;
         } else {
            if( pin[ offsetIn_r ] ) {
               if( pin[ offsetIn_p ] ) {
                  ++regions.Value( *pout = regions.Union( pout[ offsetOut_p ], pout[ offsetOut_r ] )); // x = p + r
                  goto tree_B;
               } else {
                  ++regions.Value( *pout = pout[ offsetOut_r ] ); // x = r
                  goto tree_B;
               }
            } else {
               if( pin[ offsetIn_p ] ) {
                  ++regions.Value( *pout = pout[ offsetOut_p ] ); // x = p
                  goto tree_C;
               } else {
                  // x = new label
                  *pout = regions.Create( 1 );
                  goto tree_C;
               }
            }
         }
      } else {
         // nothing
         goto tree_D;
      }

      // Last column
   break_A:
      if( *pin ) {
         if( pin[ offsetIn_q ] ) {
            ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
         } else {
            ++regions.Value( *pout = pout[ offsetOut_s ] ); // x = s
         }
      }
      continue;
   break_B:
      if( *pin ) {
         ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
      }
      continue;
   break_C:
      if( *pin ) {
         ++regions.Value( *pout = pout[ offsetOut_s ] ); // x = s
      }
      continue;
   break_D:
      if( *pin ) {
         if( pin[ offsetIn_q ] ) {
            ++regions.Value( *pout = pout[ offsetOut_q ] ); // x = q
         } else {
            if( pin[ offsetIn_p ] ) {
               ++regions.Value( *pout = pout[ offsetOut_p ] ); // x = p
            } else {
               // x = new label
               *pout = regions.Create( 1 );
            }
         }
      }
   } //End rows's for
}

} // namespace

} // namespace dip
