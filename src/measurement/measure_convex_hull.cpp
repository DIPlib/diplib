/*
 * DIPlib 3.0
 * This file contains definitions for functions that measure polygons and convex hulls.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
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

#include "diplib.h"
#include "diplib/chain_code.h"

namespace dip {

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

   auto const& vertices = Vertices();

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


RadiusValues Polygon::RadiusStatistics() const {
   RadiusValues radius;
   if( vertices.size() < 3 ) {
      return radius; // CLion thinks this is not initialized, but it is.
   }
   VertexFloat centroid = Centroid();
   VarianceAccumulator vacc;
   MinMaxAccumulator macc;
   for( auto const& v : vertices ) {
      dfloat r = Distance( centroid, v );
      vacc.Push( r );
      macc.Push( r );
   }
   radius.mean = vacc.Mean();
   radius.var = vacc.Variance();
   radius.max = macc.Maximum();
   radius.min = macc.Minimum();
   return radius;
}


dfloat Polygon::EllipseVariance( VertexFloat const& g, dip::CovarianceMatrix const& C ) const {
   // Inverse of covariance matrix
   dip::CovarianceMatrix U = C.Inv();
   // Distance of vertex to ellipse is given by sqrt( v' * U * v ), with v' the transpose of v
   VarianceAccumulator acc;
   for( auto v : vertices ) {
      v -= g;
      dfloat d = std::sqrt( U.Project( v ));
      acc.Push( d );
   }
   dfloat m = acc.Mean();
   // Ellipse variance = coefficient of variation of radius
   return m == 0.0 ? 0.0 : acc.StandardDeviation() / m;
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing chain code polygons") {
   dip::ChainCode cc;
   dip::Polygon p = cc.Polygon();
   DOCTEST_CHECK( p.vertices.size() == 4 );
   DOCTEST_CHECK( p.Area() == doctest::Approx( 0.5 ));
   DOCTEST_CHECK( p.Length() == doctest::Approx( 2.0 * std::sqrt( 2.0 )));
   dip::ConvexHull h = p.ConvexHull();
   DOCTEST_CHECK( h.Vertices().size() == 4 );
   DOCTEST_CHECK( h.Area() == doctest::Approx( 0.5 ));
   DOCTEST_CHECK( h.Perimeter() == doctest::Approx( 2.0 * std::sqrt( 2.0 )));
   auto f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( f.minDiameter == doctest::Approx( std::sqrt( 2 ) / 2.0 ));

   cc.codes = { 0, 6, 4, 2 }; // A chain code that is a little square.
   p = cc.Polygon();
   DOCTEST_CHECK( p.vertices.size() == 8 );
   DOCTEST_CHECK( p.Area() == doctest::Approx( 4 - 0.5 ));
   DOCTEST_CHECK( p.Length() == doctest::Approx( 4 + 2 * std::sqrt( 2 )));
   h = p.ConvexHull();
   DOCTEST_CHECK( h.Vertices().size() == 8 );
   DOCTEST_CHECK( h.Area() == doctest::Approx( 4 - 0.5 ));
   DOCTEST_CHECK( h.Perimeter() == doctest::Approx( 4 + 2 * std::sqrt( 2 )));
   f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == doctest::Approx( std::sqrt( 5 )));
   DOCTEST_CHECK( f.minDiameter == doctest::Approx( 2 ));
}

#endif // DIP__ENABLE_DOCTEST
