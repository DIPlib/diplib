/*
 * DIPlib 3.0
 * This file contains the definitions for the functions that creates a convex hull from a sequence of chain codes.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 *
 * The algorihtm to convert a simple polygon to a convex hull is from:
 *    A.A. Melkman, "On-Line Construction of the Convex Hull of a Simple Polyline",
 *    Information Processing Letters 25:11-12 (1987).
 * Algorithm to make a polygon from a chain code is home-brewed, concept of using
 *    pixel edge midpoints from Steve Eddins:
 *    http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/
 */


#include <deque>

#include "diplib.h"
#include "diplib/chain_code.h"


namespace dip {


static inline void DecrementMod4( int& k ) {
   k = ( k == 0 ) ? ( 3 ) : ( k - 1 );
}

dip::Polygon ChainCode::Polygon() const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)." );

   std::array< VertexInteger, 8 > dir;
   if( is8connected ) {
      dir[ 0 ] = {  1,  0 };
      dir[ 1 ] = {  1, -1 };
      dir[ 2 ] = {  0, -1 };
      dir[ 3 ] = { -1, -1 };
      dir[ 4 ] = { -1,  0 };
      dir[ 5 ] = { -1,  1 };
      dir[ 6 ] = {  0,  1 };
      dir[ 7 ] = {  1,  1 };
   } else {
      dir[ 0 ] = {  1,  0 };
      dir[ 1 ] = {  0, -1 };
      dir[ 2 ] = { -1,  0 };
      dir[ 3 ] = {  0,  1 };
   }

   std::array< VertexFloat, 8 > pts;
   pts[ 0 ] = {  0.0, -0.5 };
   pts[ 1 ] = { -0.5,  0.0 };
   pts[ 2 ] = {  0.0,  0.5 };
   pts[ 3 ] = {  0.5,  0.0 };

   VertexFloat pos { dfloat( start.x ), dfloat( start.y ) };
   dip::Polygon polygon;

   if( codes.empty() ) {
      /* A 1-pixel oject. */
      polygon.vertices.push_back( pts[ 0 ] + pos );
      polygon.vertices.push_back( pts[ 1 ] + pos );
      polygon.vertices.push_back( pts[ 2 ] + pos );
      polygon.vertices.push_back( pts[ 3 ] + pos );
   } else {
      int m = 0;
      for( auto const& code : codes ) {
         int n = int( code );
         int k, l;
         if( is8connected ) {
            k = ( m + 1 ) / 2;
            if( k == 4 ) {
               k = 0;
            }
            l = ( n / 2 ) - k;
            if( l < 0 ) {
               l += 4;
            }
         } else {
            k = m;
            l = n;
         }
         polygon.vertices.push_back( pts[ k ] + pos );
         if( l != 0 ) {
            DecrementMod4( k );
            polygon.vertices.push_back( pts[ k ] + pos );
            if( l <= 2 ) {
               DecrementMod4( k );
               polygon.vertices.push_back( pts[ k ] + pos );
               if( is8connected && ( l == 1 ) ) {
                  // This case is only possible if n is odd and n==m+4
                  DecrementMod4( k );
                  polygon.vertices.push_back( pts[ k ] + pos );
               }
            }
         }
         pos += dir[ n ];
         m = n;
      }
   }
   return polygon;
}


ConvexHull::ConvexHull( Polygon const&& polygon ) {
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
   while( ParallelogramSignedArea( *v1, *v2, *v3 ) == 0 ) {
      // While the first three vertices are colinear, we discard the middle one and continue
      // Note that this could cause problems if all vertices are in a straight line, we could discard the points
      // at the extrema. But because of the way we generate the vertices, they cannot all be in a straight line.
      v2 = v3;
      ++v3;
      if( v3 == pv.end() ) {
         vertices_.vertices.push_back( *v1 );
         vertices_.vertices.push_back( *v2 );
      }
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
      }
      deque.push_back( *v1 );
      while( ParallelogramSignedArea( *v1, deque.front(), deque.begin()[ 1 ] ) <= 0 ) {
         deque.pop_front();
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
