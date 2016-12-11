/*
 * DIPlib 3.0
 * This file contains definitions for functions that measure convex hulls.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 */


#include <limits>

#include "diplib.h"
#include "diplib/chain_code.h"


namespace dip {


dfloat ConvexHull::Area() const {
   if( vertices.size() < 3 ) {
      if( vertices.empty() ) {
         return 0;
      } else if( vertices.size() == 1 ) {
         return 1;
      } else {
         return Distance( vertices[ 0 ], vertices[ 1 ] ) + 1;
      }
   } else {
      dfloat a = 0;
      auto first = vertices.begin();
      auto v1 = first + 1;
      auto v2 = v1 + 1;
      while( v2 != vertices.end() ) {
         a += TriangleArea( *first, *v1, *v2 );
         v1 = v2;
         ++v2;
      }
      // Because the way we define the polygons w.r.t. the boundary pixels,
      // we are always half a pixel under the correct object area.
      return a + 0.5;
   }
}

dfloat ConvexHull::Primeter() const {
   if( vertices.size() < 2 ) {
       return 0;
   } else {
      dfloat p = 0;
      auto v1 = vertices.begin();
      auto v2 = v1 + 1;
      while( v2 != vertices.end() ) {
         p += Distance( *v1, *v2 );
         v1 = v2;
         ++v2;
      }
      p += Distance( vertices.back(), vertices.front() );
      return p;
   }
}


// An operator that increments the iterator but treating the container as a circular one
template< typename Iterator, typename Container >
static inline Iterator Next( Iterator it, Container const& con ) {
   ++it;
   if( it == con.end() ) {
      it = con.begin();
   }
   return it;
}

FeretValues ConvexHull::Feret() const {

   FeretValues feret;

   if( vertices.size() < 3 ) {
      // Nothing to do, give some meaningful values
      if( vertices.size() == 2 ) {
         feret.maxDiameter = Distance( vertices[ 0 ], vertices[ 1 ] );
         feret.minDiameter = 1;
         feret.maxPerpendicular = feret.maxDiameter;
      } else if (vertices.size() == 1 ) {
         feret.maxDiameter = 1;
         feret.minDiameter = 1;
         feret.maxPerpendicular = 1;
      }
      return feret;
   }

   /* Algorithm by Preparata and Shamos (1985) to obtain list of anti-podal pairs
    *    (from http://cgm.cs.mcgill.ca/~orm/rotcal.html)
    *
    * begin
    *   p0:=pn;
    *   q:=NEXT[p];
    *   while (Area(p,NEXT[p],NEXT[q]) > Area(p,NEXT[p],q)) do
    *     q:=NEXT[q];
    *     q0:=q;                                                <= WRONG INDENTING!!!
    *     while (q != p0) do                                    <= TYPO! (p != p0)
    *       begin
    *         p:=NEXT[p];
    *         Print(p,q);
    *         while (Area(p,NEXT[p],NEXT[q]) > Area(p,NEXT[p],q) do
    *           begin
    *             q:=NEXT[q];
    *             if ((p,q) != (q0,p0)) then Print(p,q)
    *             else return                                   <= NEVER SEEMS TO HAPPEN?
    *           end;
    *         if (Area(p,NEXT[p],NEXT[q]) = Area(p,NEXT[p],q)) then
    *           if ((p,q) != (q0,p0)) then Print(p,NEXT[q])
    *           else Print(NEXT[p],q)                           <= NEVER SEEMS TO HAPPEN?
    *       end
    * end.
    */
   auto first = vertices.begin();
   auto p = first;
   auto q = p + 1;
   while( ParallelogramSignedArea( *p, *Next( p, vertices ), *Next( q, vertices )) >
          ParallelogramSignedArea( *p, *Next( p, vertices ), *q ) ) {
      q = Next( q, vertices );
   }

   //auto q0 = q;
   auto p0 = vertices.end() - 1;
   feret.minDiameter = std::numeric_limits< dfloat >::max();
   while( p != p0 ) {
      ++p;
      // (p,q) is an antipodal pair
      dfloat d = Distance( *p, *q );
      if( d > feret.maxDiameter ) {
         feret.maxDiameter = d;
         feret.maxAngle = Angle( *p, *q );
      }
      while( ParallelogramSignedArea( *p, *Next( p, vertices ), *Next( q, vertices )) >
             ParallelogramSignedArea( *p, *Next( p, vertices ), *q ) ) {
         // (p,q+1) is an antipodal pair
         d = TriangleHeight( *q, *Next( q, vertices ), *p );
         if( d < feret.minDiameter ) {
            feret.minDiameter = d;
            feret.minAngle = Angle( *q, *Next( q, vertices ) );
         }
         q = Next( q, vertices );
         d = Distance( *p, *q );
         if( d > feret.maxDiameter ) {
            feret.maxDiameter = d;
            feret.maxAngle = Angle( *p, *q );
         }
      }
      if( ParallelogramSignedArea( *p, *Next( p, vertices ), *Next( q, vertices ) ) ==
          ParallelogramSignedArea( *p, *Next( p, vertices ), *q ) ) {
         // (p,q+1) is an antipodal pair also, but we don't advance q
         d = TriangleHeight( *q, *Next( q, vertices ), *p );
         if( d < feret.minDiameter ) {
            feret.minDiameter = d;
            feret.minAngle = Angle( *q, *Next( q, vertices ) );
         }
         d = Distance( *p, *Next( q, vertices ) );
         if( d > feret.maxDiameter ) {
            feret.maxDiameter = d;
            feret.maxAngle = Angle( *p, *Next( q, vertices ) );
         }
      }
   }

   // Get the diameter perpendicular to feret.minDiameter
   dfloat cos = std::cos( feret.minAngle );
   dfloat sin = std::sin( feret.minAngle );
   dfloat pmin = std::numeric_limits< dfloat >::max();
   dfloat pmax = std::numeric_limits< dfloat >::lowest();
   for( auto const& v : vertices ) {
      dfloat d = v.x * cos + v.y * sin;
      pmin = std::min( pmin, d );
      pmax = std::max( pmax, d );
      ++p;
   }
   feret.maxPerpendicular = pmax - pmin;

   // We want to give the minimum diameter angle correctly
   feret.minAngle = feret.minAngle + pi / 2.0;

   return feret;
}


} // namespace dip
