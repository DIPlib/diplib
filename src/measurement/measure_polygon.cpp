/*
 * (c)2016-2025, Cris Luengo.
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

#include "diplib/chain_code.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "diplib.h"
#include "diplib/accumulators.h"

#if defined(__GNUG__) || defined(__clang__)
   // For Eigen, turn off -Wsign-conversion
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wsign-conversion"
   #ifndef __clang__
      #pragma GCC diagnostic ignored "-Wclass-memaccess"
   #endif
   #if ( __GNUC__ >= 11 ) && ( __GNUC__ <= 14 )
      #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
   #endif
#endif

#include <Eigen/Eigen> // IWYU pragma: keep

#if defined(__GNUG__) || defined(__clang__)
   #pragma GCC diagnostic pop
#endif

namespace dip {

dfloat Polygon::Area() const {
   if( vertices.size() < 3 ) {
      return 0; // Should we generate an error instead?
   }
   dfloat sum = CrossProduct( vertices.back(), vertices[ 0 ] );
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      sum += CrossProduct( vertices[ ii - 1 ], vertices[ ii ] );
   }
   return sum / 2.0;
}

VertexFloat Polygon::Centroid() const {
   if( vertices.size() < 3 ) {
      return { 0, 0 }; // Should we generate an error instead?
   }
   dfloat v = CrossProduct( vertices.back(), vertices[ 0 ] );
   dfloat sum = v;
   dfloat xsum = ( vertices.back().x + vertices[ 0 ].x ) * v;
   dfloat ysum = ( vertices.back().y + vertices[ 0 ].y ) * v;
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      v = CrossProduct( vertices[ ii - 1 ], vertices[ ii ] );
      sum += v;
      xsum += ( vertices[ ii - 1 ].x + vertices[ ii ].x ) * v;
      ysum += ( vertices[ ii - 1 ].y + vertices[ ii ].y ) * v;
   }
   return sum == 0.0 ? VertexFloat{ 0.0, 0.0 } : VertexFloat{ xsum, ysum } / ( 3 * sum );
}

dip::CovarianceMatrix Polygon::CovarianceMatrixVertices( VertexFloat const& g ) const {
   dip::CovarianceMatrix C;
   if( vertices.size() >= 3 ) {
      for( auto v : vertices ) {
         C += dip::CovarianceMatrix( v - g );
      }
      C /= static_cast< dfloat >( vertices.size() );
   }
   return C;
}

namespace {

dip::CovarianceMatrix GreenIntegralCovariance( VertexFloat const& v0, VertexFloat const& v1 ) {
   // See for example https://medium.com/@aleozlx/the-maths-behind-contour-moments-from-opencv-491e5c348b91
   dfloat v = CrossProduct( v0, v1 );
   return dip::CovarianceMatrix{
         /* xx = */ v / 12.0 * ( v0.x * ( v0.x + v1.x ) + v1.x * v1.x ),
         /* yy = */ v / 12.0 * ( v0.y * ( v0.y + v1.y ) + v1.y * v1.y ),
         /* xy = */ v / 24.0 * ( v0.x * ( 2 * v0.y + v1.y ) + v1.x * ( v0.y + 2 * v1.y )),
   };
}

} // namespace

dip::CovarianceMatrix Polygon::CovarianceMatrixSolid( VertexFloat const& g ) const {
   if( vertices.size() < 3 ) {
      return {};
   }
   dip::CovarianceMatrix C = GreenIntegralCovariance( vertices.back() - g, vertices.front() - g );
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      C += GreenIntegralCovariance( vertices[ ii - 1 ] - g, vertices[ ii ] - g );
   }
   C /= Area();
   return C;
}

dfloat Polygon::Length() const {
   if( vertices.size() < 2 ) {
      return 0; // Should we generate an error instead?
   }
   dfloat sum = Distance( vertices.back(), vertices[ 0 ] );
   for( dip::uint ii = 1; ii < vertices.size(); ++ii ) {
      sum += Distance( vertices[ ii - 1 ], vertices[ ii ] );
   }
   return sum;
}

RadiusValues Polygon::RadiusStatistics( VertexFloat const& g ) const {
   RadiusValues radius;
   if( vertices.size() < 3 ) {
      return radius;
   }
   for( auto const& v : vertices ) {
      dfloat r = Distance( g, v );
      radius.Push( r );
   }
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

namespace {

// An operator that increments the iterator but treating the container as a circular one
template< typename Iterator, typename Container >
inline Iterator Next( Iterator it, Container const& con ) {
   ++it;
   if( it == con.end()) {
      it = con.begin();
   }
   return it;
}

} // namespace

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
      } // else if empty, keep the defaults, which are all 0.
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

   auto p = vertices.begin();
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
             ParallelogramSignedArea( *p, *Next( p, vertices ), *q )) {
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
   }
   feret.maxPerpendicular = pmax - pmin;

   // We want to give the minimum diameter angle correctly
   feret.minAngle = feret.minAngle + pi / 2.0;

   return feret;
}

dfloat Polygon::FractalDimension( dfloat length ) const {
   if( length <= 0 ) {
      length = Length();
   }
   dfloat sigmaMax = length / 16;
   if( sigmaMax <= 2 ) {
      // This ensures that nScales >= 3, and that log2(sigmaMax) is not a negative number
      // We end up here also if the polygon has few or no vertices.
      return 1.0;
   }
   dip::uint nScales = dip::uint( std::ceil( std::log2( sigmaMax ))) + 1; // Guaranteed >= 3.

   // Compute perimeter at all tolerance sizes
   std::vector< dfloat > scale( nScales );
   std::vector< dfloat > perimeter( nScales );
   {
      Polygon P = *this; // Make copy we can modify
      dfloat sigma = 1.0;
      dfloat prevSigma = 0.0;
      for( dip::uint ii = 0; ii < nScales; ++ii ) {
         P.Smooth( std::sqrt( sigma * sigma - prevSigma * prevSigma ));
         scale[ ii ] = sigma;
         perimeter[ ii ] = P.Length();
         prevSigma = sigma;
         sigma *= 2.0;
      }
   }

   // Linear Regression (Least Square Estimation)
   dfloat Sx = std::log( scale[ 0 ] );
   dfloat Sy = std::log( perimeter[ 0 ] );
   dfloat Sxx = Sx * Sx;
   dfloat Sxy = Sx * Sy;
   dfloat N = 1;
   for( dip::uint ii = 1; ii < nScales; ++ii ) {
      dfloat ls = std::log( scale[ ii ] );
      dfloat lp = std::log( perimeter[ ii ] );
      Sx += ls;
      Sy += lp;
      Sxx += ls * ls;
      Sxy += ls * lp;
      ++N;
   }
   dfloat slope = 1.0;
   dfloat d = N * Sxx - Sx * Sx;
   if( d != 0.0 ) {
      slope = (( N * Sxy ) - ( Sx * Sy )) / d;
      slope = clamp( 1.0 - slope, 1.0, 2.0 );
   }
   return slope;
}

namespace {

dfloat AngleDifference( dfloat a, dfloat b ) { // a and b are assumed in the [-pi,pi] range.
   dfloat diff = std::abs( a - b );
   if( diff > pi ) {
      diff = 2 * pi - diff;
   }
   return diff;
}

} // namespace

dfloat Polygon::BendingEnergy() const {
   // BE = sum ( k * k * dist )
   // k = diff / dist
   // => BE = sum ( diff * diff / dist )
   dfloat be = 0;
   if( vertices.size() > 2 ) {
      dfloat prev = dip::Angle( vertices[ 0 ], vertices[ 1 ] );
      dip::uint ii = 1;
      for( ; ii < vertices.size() - 1; ++ii ) {
         dfloat a = Angle( vertices[ ii ], vertices[ ii + 1 ] );
         dfloat diff = AngleDifference( a, prev );
         be += diff * diff * 2.0 / Distance( vertices[ ii - 1 ], vertices[ ii + 1 ] );
         prev = a;
      }
      // Add the two points that require straddling the boundary
      dfloat a = Angle( vertices[ ii ], vertices[ 0 ] );
      dfloat diff = AngleDifference( a, prev );
      be += diff * diff * 2.0 / Distance( vertices[ ii - 1 ], vertices[ 0 ] );
      prev = a;
      a = Angle( vertices[ 0 ], vertices[ 1 ] );
      diff = AngleDifference( a, prev );
      be += diff * diff * 2.0 / Distance( vertices[ ii ], vertices[ 1 ] );
   }
   return be;
}

CircleParameters Polygon::FitCircle() const {
   dip::uint n = vertices.size();
   Eigen::MatrixXd A( n, 3 );
   for( dip::uint ii = 0; ii < n; ++ii ) {
      A.row( static_cast< dip::sint >( ii )) << vertices[ ii ].x, vertices[ ii ].y, 1.0;
   }
   Eigen::VectorXd b = A.col( 0 ).array().square() + A.col( 1 ).array().square();
   Eigen::Vector3d sol = A.colPivHouseholderQr().solve( b );
   dfloat cx = sol( 0 ) / 2;
   dfloat cy = sol( 1 ) / 2;
   dfloat r = std::sqrt( sol( 2 ) + cx * cx + cy * cy );
   return {{ cx, cy }, 2 * r };
}

EllipseParameters Polygon::FitEllipse() const {
   dip::uint n = vertices.size();
   Eigen::Vector< double, 5 > sol;
   {
      Eigen::MatrixXd A( n, 5 );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         dfloat x = vertices[ ii ].x;
         dfloat y = vertices[ ii ].y;
         A.row( static_cast< dip::sint >( ii )) << x * x, x * y, y * y, x, y;
      }
      Eigen::VectorXd b( n );
      b.fill( 1.0 );
      sol = A.colPivHouseholderQr().solve( b );
   }
   dfloat a = sol[ 0 ];
   dfloat b = sol[ 1 ];
   dfloat c = sol[ 2 ];
   dfloat d = sol[ 3 ];
   dfloat e = sol[ 4 ];
   dfloat denom = b * b - 4 * a * c;
   if( denom >= 0 ) {
      // This is not a good fit, the result does not represent an ellipse.
      return {};
   }
   dfloat pt1 = 2 * ( a * e * e + c * d * d - b * d * e - denom );
   dfloat pt2 = a + c;
   dfloat pt3 = std::sqrt(( a - c ) * ( a - c ) + b * b );
   dfloat majorAxis = -std::sqrt( pt1 * ( pt2 + pt3 )) / denom;
   dfloat minorAxis = -std::sqrt( pt1 * ( pt2 - pt3 )) / denom;
   dfloat x = ( 2 * c * d - b * e ) / denom;
   dfloat y = ( 2 * a * e - b * d ) / denom;
   dfloat theta = std::atan2( -b, c - a ) / 2;
   return {
      {x, y},
      2 * majorAxis,
      2 * minorAxis,
      theta,
      std::sqrt(1.0 - minorAxis * minorAxis / (majorAxis * majorAxis ))
   };

}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing chain code polygons") {
   dip::ChainCode cc;
   DOCTEST_CHECK( cc.Empty() );
   DOCTEST_CHECK( cc.Area() == 0.0 );

   dip::Polygon p = cc.Polygon();
   DOCTEST_CHECK( p.vertices.size() == 0 );
   DOCTEST_CHECK( p.Area() == 0.0 );
   DOCTEST_CHECK( p.Length() == 0.0 );
   dip::ConvexHull h = p.ConvexHull();
   DOCTEST_CHECK( h.vertices.size() == 0 );
   DOCTEST_CHECK( h.Area() == 0.0 );
   DOCTEST_CHECK( h.Perimeter() == 0.0 );
   auto f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == 0.0 );
   DOCTEST_CHECK( f.minDiameter == 0.0 );

   cc.start = { 0, 0 };
   DOCTEST_CHECK( !cc.Empty() );
   DOCTEST_CHECK( cc.Area() == doctest::Approx( 1.0 ));
   p = cc.Polygon();
   DOCTEST_CHECK( p.vertices.size() == 4 );
   DOCTEST_CHECK( p.Area() == doctest::Approx( 0.5 ));
   DOCTEST_CHECK( p.Length() == doctest::Approx( 2.0 * std::sqrt( 2.0 )));
   h = p.ConvexHull();
   DOCTEST_CHECK( h.vertices.size() == 4 );
   DOCTEST_CHECK( h.Area() == doctest::Approx( 0.5 ));
   DOCTEST_CHECK( h.Perimeter() == doctest::Approx( 2.0 * std::sqrt( 2.0 )));
   f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( f.minDiameter == doctest::Approx( std::sqrt( 2 ) / 2.0 ));

   cc.codes = { 0u, 6u, 4u, 2u }; // A chain code that is a little square.
   DOCTEST_CHECK( cc.Area() == doctest::Approx( 4.0 ));
   p = cc.Polygon();
   DOCTEST_CHECK( p.vertices.size() == 8 );
   DOCTEST_CHECK( p.Area() == doctest::Approx( 4 - 0.5 ));
   DOCTEST_CHECK( p.Length() == doctest::Approx( 4 + 2 * std::sqrt( 2 )));
   DOCTEST_CHECK( p.IsClockWise() );
   h = p.ConvexHull();
   DOCTEST_CHECK( h.vertices.size() == 8 );
   DOCTEST_CHECK( h.Area() == doctest::Approx( 4 - 0.5 ));
   DOCTEST_CHECK( h.Perimeter() == doctest::Approx( 4 + 2 * std::sqrt( 2 )));
   DOCTEST_CHECK( h.Polygon().IsClockWise() );
   f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == doctest::Approx( std::sqrt( 5 )));
   DOCTEST_CHECK( f.minDiameter == doctest::Approx( 2 ));

   p.vertices = {{ 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0.5, 0.5 }};
   DOCTEST_CHECK( p.Area() == doctest::Approx( -0.75 ));
   DOCTEST_CHECK( p.Length() == doctest::Approx( 3 + std::sqrt( 2 )));
   DOCTEST_CHECK( !p.IsClockWise() );
   h = p.ConvexHull();
   DOCTEST_CHECK( h.vertices.size() == 4 );
   DOCTEST_CHECK( h.Area() == doctest::Approx( 1 ));
   DOCTEST_CHECK( h.Perimeter() == doctest::Approx( 4 ));
   DOCTEST_CHECK( h.Polygon().IsClockWise() );
   f = h.Feret();
   DOCTEST_CHECK( f.maxDiameter == doctest::Approx( std::sqrt( 2 )));
   DOCTEST_CHECK( f.minDiameter == doctest::Approx( 1 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
