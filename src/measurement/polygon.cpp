/*
 * (c)2016-2022, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 *
 * The algorithm to convert a simple polygon to a convex hull is from:
 *    A.A. Melkman, "On-Line Construction of the Convex Hull of a Simple Polyline",
 *    Information Processing Letters 25:11-12 (1987).
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

#include "diplib/polygon.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <utility>

#include "diplib.h"
#include "diplib/linear.h"

namespace dip {

namespace {

constexpr char const* POLYGON_SELF_INTERSECTS = "The polygon is self-intersecting, cannot compute convex hull";

} // namespace

BoundingBoxFloat Polygon::BoundingBox() const {
   if( vertices.empty() ) {
      return {}; // Should we generate an error instead?
   }
   BoundingBoxFloat bb( vertices[ 0 ] );
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      bb.Expand( vertices[ ii ] );
   }
   return bb;
}

bool Polygon::IsClockWise() const {
   if( vertices.size() < 3 ) {
      return true;
   }
   // Find the topmost point (lowest y value) of the polygon, then compute the
   // cross product of the two incident edges. This avoids computing the signed
   // area of the full polygon.
   dip::uint minIndex = 0;
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      if(( vertices[ ii ].y < vertices[ minIndex ].y ) ||
         (( vertices[ ii ].y == vertices[ minIndex ].y ) && ( vertices[ ii ].x > vertices[ minIndex ].x ))) {
         minIndex = ii;
      }
   }
   dip::uint prev = ( minIndex + vertices.size() - 1 ) % vertices.size();
   dip::uint next = ( minIndex + 1 ) % vertices.size();
   return ParallelogramSignedArea( vertices[ minIndex ], vertices[ next ], vertices[ prev ] ) >= 0; // shouldn't be == 0
}

namespace {

// The next point along a polygon with polygon::size() == N points.
dip::uint Next( dip::uint p, dip::uint N ) {
   return ( p + 1 ) % N;
}

// The most distant point to `index` in the polygon
dip::uint MostDistant( Polygon::Vertices const& vertices, dip::uint index ) {
   dfloat maxDistSq = 0;
   dip::uint maxIndex = index;
   for( dip::uint ii = 0; ii < vertices.size(); ++ii ) {
      dfloat d = DistanceSquare( vertices[ index ], vertices[ ii ] );
      if( d > maxDistSq ) {
         maxDistSq = d;
         maxIndex = ii;
      }
   }
   return maxIndex;
}

void SimplifySection( Polygon::Vertices const& vertices, Polygon::Vertices& out, dip::uint start, dip::uint end, dfloat toleranceSquare ) {
   dip::uint N = vertices.size();
   if( end == start ) {
      // We need at least two points
      return;
   }
   // Find point furthest from the straight line between start and end.
   double maxDistSq = 0;
   dip::uint maxIndex = 0;
   VertexFloat baseVector = vertices[ end ] - vertices[ start ];
   for( dip::uint ii = Next( start, N ); ii != end; ii = Next( ii, N )) {
      // Instead of computing the actual square distance to the line for each point:
      //dfloat area = ParallelogramSignedArea( vertices[ start ], vertices[ end ], vertices[ ii ]  );
      //dfloat d2 = std::abs( area * area / DistanceSquare( vertices[ start ], vertices[ end ] ));
      // we just compute the part relevant for comparison, then compute the computation only for the maximum result:
      dfloat d2 = std::abs( CrossProduct( baseVector, vertices[ ii ] - vertices[ start ] ));
      if( d2 > maxDistSq ) {
         maxDistSq = d2;
         maxIndex = ii;
      }
   }
   maxDistSq = maxDistSq * maxDistSq / NormSquare( baseVector );
   if( maxDistSq > toleranceSquare ) {
      // Split the line at this point, and recursively simplify the two halves.
      // We guarantee here that start != maxIndex != end.
      SimplifySection( vertices, out, start, maxIndex, toleranceSquare );
      out.push_back( vertices[maxIndex ] );
      SimplifySection( vertices, out, maxIndex, end, toleranceSquare );
   }
}

} // namespace

Polygon& Polygon::Simplify( dfloat tolerance ) {
   // With 4 points or fewer there's nothing to simplify
   if(( tolerance <= 0 ) || ( vertices.size() <= 4 )) {
      return *this;
   }
   // Split into two halves using two extreme points that we keep in the polygon
   dip::uint pt1 = MostDistant( vertices, 0 ); // this must be an extreme point, no matter what point 0 is.
   dip::uint pt2 = MostDistant( vertices, pt1 ); // this is a second extreme point on the far side of the polygon.
   // Process each half independently
   dfloat toleranceSquare = tolerance * tolerance;
   Polygon::Vertices newVertices;
   newVertices.push_back( vertices[ pt1 ] );
   SimplifySection( vertices, newVertices, pt1, pt2, toleranceSquare );
   newVertices.push_back( vertices[ pt2 ] );
   SimplifySection( vertices, newVertices, pt2, pt1, toleranceSquare );
   vertices = std::move( newVertices );
   return *this;
}

namespace {

void InsertPoints( Polygon::Vertices& vertices, VertexFloat start, VertexFloat end, double distance ) {
   VertexFloat line = end - start;
   dfloat length = Norm( line );
   dfloat N = std::ceil( length / distance );
   distance = length / N;
   VertexFloat inc = line * ( distance / length );
   for( dip::uint ii = 0; ii < static_cast< dip::uint >( N ); ++ii ) {
      vertices.push_back( start );
      start += inc;
   }
}

} // namespace

Polygon& Polygon::Augment( dfloat distance ) {
   if( !vertices.empty() ) {
      Polygon::Vertices newVertices;
      newVertices.reserve( vertices.size()); // minimum number of points, but we'll probably have more...
      for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
         InsertPoints( newVertices, vertices[ ii - 1 ], vertices[ ii ], distance );
      }
      InsertPoints( newVertices, vertices.back(), vertices.front(), distance );
      vertices = std::move( newVertices );
   }
   return *this;
}

Polygon& Polygon::Smooth( dfloat sigma ) {
   if( !vertices.empty() ) {
      Image img( reinterpret_cast< dfloat* >( vertices.data() ), { 2, vertices.size() } );
      img.Protect();
      GaussFIR( img, img, { 0, sigma }, { 0, 0 }, { S::PERIODIC } );
   }
   return *this;
}

Polygon& Polygon::Rotate( dfloat angle ) {
   dfloat cos_angle = std::cos( angle );
   dfloat sin_angle = std::sin( angle );
   for( auto& v: vertices ) {
      v = { v.x * cos_angle - v.y * sin_angle, v.x * sin_angle + v.y * cos_angle };
   }
   return *this;
}

Polygon& Polygon::Scale( dfloat scale ) {
   for( auto& v: vertices ) {
      v *= scale;
   }
   return *this;
}

Polygon& Polygon::Scale( dfloat scaleX, dfloat scaleY ) {
   VertexFloat scale{ scaleX, scaleY };
   for( auto& v: vertices ) {
      v *= scale;
   }
   return *this;
}

Polygon& Polygon::Translate( VertexFloat shift ) {
   for( auto& v: vertices ) {
      v += shift;
   }
   return *this;
}

ConvexHull::ConvexHull( dip::Polygon const& polygon ) {
   auto const& inVertices = polygon.vertices;
   if( inVertices.size() <= 3 ) {
      // If there's less than 4 elements, we already have a convex hull
      vertices = inVertices;
      return;
   }

   // Find shortest vertex for scale
   dfloat minLength = Distance( inVertices.front(), inVertices.back() );
   for( dip::uint ii = 1; ii < inVertices.size(); ++ii ) {
      minLength = std::min( minLength, Distance( inVertices[ ii ], inVertices[ ii - 1 ] ));
   }
   dfloat eps = minLength * 1e-9;
   // We ignore parallelogram distances that are 9 orders of magnitude smaller than the minimum distance
   // between vertices, to prevent numerical precision errors (vertex locations are rounded to floating-point
   // precision, and there are other numerical errors too).

   // Melkman's algorithm for the convex hull
   std::deque< VertexFloat > deque;
   auto v1 = inVertices.begin();
   auto v2 = v1 + 1;
   auto v3 = v2 + 1;         // these elements exist for sure -- we have more than 3 elements!
   while( std::abs( ParallelogramSignedArea( *v1, *v2, *v3 )) < eps ) {
      // While the first three vertices are colinear, we discard the middle one and continue.
      v2 = v3;
      ++v3;
      DIP_THROW_IF( v3 == inVertices.end(), "All vertices are colinear, cannot compute convex hull" );
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
   while( v1 != inVertices.end() ) {
      ++v1;
      if( v1 == inVertices.end() ) {
         break;
      }
      while( ParallelogramSignedArea( *v1, deque.front(), deque.begin()[ 1 ] ) > -eps &&
             ParallelogramSignedArea( deque.rbegin()[ 1 ], deque.back(), *v1 ) > -eps ) {
         ++v1;
         if( v1 == inVertices.end() ) {
            break;
         }
      }
      if( v1 == inVertices.end() ) {
         break;
      }
      while( ParallelogramSignedArea( deque.rbegin()[ 1 ], deque.back(), *v1 ) < eps ) {
         deque.pop_back();
         DIP_THROW_IF( deque.size() < 2, POLYGON_SELF_INTERSECTS );
      }
      deque.push_back( *v1 );
      while( ParallelogramSignedArea( *v1, deque.front(), deque.begin()[ 1 ] ) < eps ) {
         deque.pop_front();
         DIP_THROW_IF( deque.size() < 2, POLYGON_SELF_INTERSECTS );
      }
      deque.push_front( *v1 );
   }
   deque.pop_front(); // The deque always has the same point at beginning and end, we only need it once in out polygon.

   // Make a new chain of the relevant polygon vertices.
   for( auto const& v : deque ) {
      vertices.push_back( v );
   }
}

bool Polygon::Contains( VertexFloat point ) const {
   // Ray casting algorithm. We count how often the polygon crosses a horizontal line from -infinity to `point`.
   // Each edge can only cross the line once. If the bottom vertex of an edge is on the line, it doesn't count
   // as a crossing, but the top vertex does.
   // Finally, the algorithm returns true if the point is on a vertex or edge (within numerical precision).
   dip::uint count = 0;
   dip::VertexFloat prev = vertices.back();
   for( auto cur : vertices ) {
      if( cur.x == point.x && cur.y == point.y ) {
         return true;
      }
      if(( prev.y <= point.y && cur.y > point.y ) || ( cur.y <= point.y && prev.y > point.y )) {
         if( cur.x <= point.x && prev.x <= point.x ) {
            ++count;
         } else if( !( cur.x > point.x && prev.x > point.x )) {
            VertexFloat edge = cur - prev;
            edge /= Norm(edge);
            edge *= point.y - prev.y;
            edge += prev;
            if( edge.x == point.x ) {
               return true;
            }
            if( edge.x < point.x ) {
               ++count;
            }
         }
      }
      prev = cur;
   }
   return count & 1u;
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing polygon manipulation") {
   dip::Polygon p;
   p.vertices = {{ 0.2,  2 },
                 { 0,    0 },
                 { 5,    0.2 },
                 { 10,   0 },
                 { 10.2, 6 },
                 { 10,   10 },
                 { 5,    9.8 },
                 { 0,    10 },
                 { -0.2, 6 }}; // a noisy square, 10x10
   dip::Polygon p2 = p;
   p2.Simplify( 0.5 );
   DOCTEST_CHECK( p2.vertices.size() == 4 );
   DOCTEST_CHECK( p2.Area() == doctest::Approx( 100 ));
   DOCTEST_CHECK( p2.Length() == doctest::Approx( 40 ));
   dip::Polygon p3 = p.ConvexHull().Polygon();
   dip::dfloat len = p3.Length();
   DOCTEST_CHECK( p3.Area() == doctest::Approx( 102 ));
   DOCTEST_CHECK( len > 40 );
   DOCTEST_CHECK( len < p.Length() );
   dip::Polygon p4 = p3;
   p4.Simplify( 0.5 );
   DOCTEST_CHECK( p4.vertices.size() == 4 );
   DOCTEST_CHECK( p4.Area() == doctest::Approx( 100 ));
   DOCTEST_CHECK( p4.Length() == doctest::Approx( 40 ));
   dip::Polygon p5 = p2;
   p5.Augment( 1.0 );
   DOCTEST_CHECK( p5.vertices.size() == 40 );
   DOCTEST_CHECK( p5.Area() == doctest::Approx( 100 ));
   DOCTEST_CHECK( p5.Length() == doctest::Approx( 40 ));
   p5.Smooth( 2.0 );
   DOCTEST_CHECK( p5.vertices.size() == 40 );
   DOCTEST_CHECK( p5.Area() == doctest::Approx( 92.0977 ));
   DOCTEST_CHECK( p5.Length() == doctest::Approx( 35.0511 ));
   p2.Reverse();
   DOCTEST_CHECK( p.IsClockWise() != p2.IsClockWise() );
}

DOCTEST_TEST_CASE("[DIPlib] testing point-in-polygon algorithm") {
   dip::Polygon p;
   p.vertices = {{ 0.2,  2 },
                 { 0,    0 },
                 { 5,    0.2 },
                 { 10,   0 },
                 { 10.2, 6 },
                 { 10,   10 },
                 { 5,    9.8 },
                 { 0,    10 },
                 { -0.2, 6 }}; // a noisy square, 10x10
   DOCTEST_CHECK( !p.Contains( { -1.0, 5.0 } ));
   DOCTEST_CHECK( !p.Contains( { 5.0, -1.0 } ));
   DOCTEST_CHECK( !p.Contains( { 5.0, 11.0 } ));
   DOCTEST_CHECK( p.Contains( { 0.0, 0.0 } ));
   DOCTEST_CHECK( p.Contains( { 10.0, 10.0 } ));
   DOCTEST_CHECK( p.Contains( { 5.0, 5.0 } ));
   DOCTEST_CHECK( p.Contains( { 1.0, 0.2 } ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
