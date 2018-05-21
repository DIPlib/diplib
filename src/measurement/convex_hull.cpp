/*
 * DIPlib 3.0
 * This file contains the definitions for the functions that creates a convex hull from a sequence of chain codes.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 *
 * The algorithm to convert a simple polygon to a convex hull is from:
 *    A.A. Melkman, "On-Line Construction of the Convex Hull of a Simple Polyline",
 *    Information Processing Letters 25:11-12 (1987).
 * Algorithm to make a polygon from a chain code is home-brewed, concept of using
 *    pixel edge midpoints from Steve Eddins:
 *    http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/
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

#include <deque>

#include "diplib.h"
#include "diplib/chain_code.h"

namespace dip {

namespace {

inline void DecrementMod4( unsigned& k ) {
   k = ( k == 0 ) ? ( 3 ) : ( k - 1 );
}

} // namespace

ChainCode ChainCode::ConvertTo8Connected() const {
   if( is8connected ) {
      return *this;
   }
   ChainCode out;
   out.objectID = objectID;
   out.start = start;
   if( codes.size() < 3 ) {
      out.codes = codes;
      for( auto& c : out.codes ) {
         c = Code( c * 2, c.IsBorder() );
      }
   } else {
      Code cur = codes.back();
      bool skipLast = false;
      Code next = codes[ 0 ];
      dip::uint ii = 0;
      if(( cur + 1 ) % 4 == next ) { // If the chain code was created by GetImageChainCodes or GetSingleChainCode, this will not happen.
         out.Push( Code( cur * 2 + 1, false ));
         out.start -= deltas4[ cur ];
         skipLast = true;
         ++ii;
      }
      for( ; ii < codes.size() - 1; ++ii ) {
         cur = codes[ ii ];
         next = codes[ ii + 1 ];
         if(( cur + 1 ) % 4 == next ) {
            out.Push( Code( cur * 2 + 1, false )); // this cannot be along the image edge!
            ++ii;
         } else {
            out.Push( Code( cur * 2, cur.IsBorder() ));
         }
      }
      if(( ii < codes.size() ) && !skipLast ) {
         cur = codes[ ii ];
         out.Push( Code( cur * 2, cur.IsBorder() ));
      }
   }
   return out;
}

dip::Polygon ChainCode::Polygon() const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );

   // This function works only for 8-connected chain codes, convert it if it's 4-connected.
   ChainCode const& cc = is8connected ? *this : ConvertTo8Connected();

   std::array< VertexFloat, 4 > pts;
   pts[ 0 ] = {  0.0, -0.5 };
   pts[ 1 ] = { -0.5,  0.0 };
   pts[ 2 ] = {  0.0,  0.5 };
   pts[ 3 ] = {  0.5,  0.0 };

   VertexFloat pos { dfloat( cc.start.x ), dfloat( cc.start.y ) };
   dip::Polygon polygon;

   if( cc.codes.empty() ) {
      // A 1-pixel object.
      polygon.vertices.push_back( pts[ 0 ] + pos );
      polygon.vertices.push_back( pts[ 3 ] + pos );
      polygon.vertices.push_back( pts[ 2 ] + pos );
      polygon.vertices.push_back( pts[ 1 ] + pos );
   } else {
      unsigned m = cc.codes.back();
      for( unsigned n : cc.codes ) {
         unsigned k = ( m + 1 ) / 2;
         if( k == 4 ) {
            k = 0;
         }
         unsigned l = n / 2;
         if( l < k ) {
            l += 4;
         }
         l -= k;
         polygon.vertices.push_back( pts[ k ] + pos );
         if( l != 0 ) {
            DecrementMod4( k );
            polygon.vertices.push_back( pts[ k ] + pos );
            if( l <= 2 ) {
               DecrementMod4( k );
               polygon.vertices.push_back( pts[ k ] + pos );
               if( l == 1 ) {
                  // This case is only possible if n is odd and n==m+4
                  DecrementMod4( k );
                  polygon.vertices.push_back( pts[ k ] + pos );
               }
            }
         }
         pos += deltas8[ n ];
         m = n;
      }
   }
   return polygon;
}


ConvexHull::ConvexHull( dip::Polygon&& polygon ) {
   auto const& pv = polygon.vertices;
   if( pv.size() <= 3 ) {
      // If there's less than 4 elements, we already have a convex hull
      vertices_ = std::move( polygon );
      return;
   }

   // Melkman's algorithm for the convex hull
   std::deque< VertexFloat > deque;
   auto v1 = pv.begin();
   auto v2 = v1 + 1;
   auto v3 = v2 + 1;         // these elements exist for sure -- we have more than 3 elements!
   dfloat eps = std::max( Distance( *v1, *v2 ), Distance( *v2, *v3 )) * 1e-12;
   while( std::abs( ParallelogramSignedArea( *v1, *v2, *v3 )) < eps ) {
      // While the first three vertices are colinear, we discard the middle one and continue.
      // We ignore a distance that is 12 orders of magnitude smaller than the distance between vertices,
      // to prevent numerical precision errors in this calculation.
      v2 = v3;
      ++v3;
      DIP_THROW_IF( v3 == pv.end(), "All vertices are colinear, cannot compute convex hull" );
      // Note that this error should not occur for any polygon generated from a chain code (i.e. representing a
      // set of pixels in an image). We have this test here in case the polygon has a different source.
      // We could, instead of throwing, return the two vertices that compose the bounding box. But that could
      // cause trouble later on, so this is better.
   }
   if( ParallelogramSignedArea( *v1, *v2, *v3 ) > 0 ) {
      deque.push_back( *v1 );
      deque.push_back( *v2 );
   } else {
      deque.push_back( *v2 );
      deque.push_back( *v1 );
   }
   deque.push_back( *v3 );
   deque.push_front( *v3 );
   v1 = v3;
   while( v1 != pv.end() ) {
      ++v1;
      if( v1 == pv.end() ) {
         break;
      }
      while( ParallelogramSignedArea( *v1, deque.front(), deque.begin()[ 1 ] ) >= 0 &&
             ParallelogramSignedArea( deque.rbegin()[ 1 ], deque.back(), *v1 ) >= 0 ) {
         ++v1;
         if( v1 == pv.end() ) {
            break;
         }
      }
      if( v1 == pv.end() ) {
         break;
      }
      while( ParallelogramSignedArea( deque.rbegin()[ 1 ], deque.back(), *v1 ) <= 0 ) {
         deque.pop_back();
         DIP_THROW_IF( deque.size() < 2, "The polygon is self-intersecting, cannot compute convex hull" );
      }
      deque.push_back( *v1 );
      while( ParallelogramSignedArea( *v1, deque.front(), deque.begin()[ 1 ] ) <= 0 ) {
         deque.pop_front();
         DIP_THROW_IF( deque.size() < 2, "The polygon is self-intersecting, cannot compute convex hull" );
      }
      deque.push_front( *v1 );
   }
   deque.pop_front(); // The deque always has the same point at beginning and end, we only need it once in out polygon.

   // Make a new chain of the relevant polygon vertices.
   for( auto const& v : deque ) {
      vertices_.vertices.push_back( v );
   }
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

// NOTE: polygon to convex hull conversion is tested a bit in `measure_convex_hull.cpp`.

DOCTEST_TEST_CASE("[DIPlib] testing chain code conversion to polygon") {
   dip::ChainCode cc8;
   cc8.codes = { 0, 0, 7, 6, 6, 5, 4, 4, 3, 2, 2, 1 }; // A chain code that is a little circle.
   cc8.is8connected = true;
   dip::ChainCode cc4;
   cc4.codes = { 0, 0, 3, 0, 3, 3, 2, 3, 2, 2, 1, 2, 1, 1, 0, 1 }; // A 4-connected chain code for the same object.
   cc4.is8connected = false;
   auto P8 = cc8.Polygon();
   //for (auto& p : P8.vertices ) {
   //   std::cout << ' ' << p.x << ',' << p.y;
   //}
   //std::cout << '\n';
   auto P4 = cc4.Polygon();
   //for (auto& p : P4.vertices ) {
   //   std::cout << ' ' << p.x << ',' << p.y;
   //}
   //std::cout << '\n';
   DOCTEST_REQUIRE( P8.vertices.size() == P4.vertices.size() );
   for( dip::uint ii = 0; ii < P8.vertices.size(); ++ii ) {
      DOCTEST_CHECK( P8.vertices[ ii ] == P4.vertices[ ii ] );
   }
}

#endif // DIP__ENABLE_DOCTEST
