/*
 * (c)2026, Tolga Ciftcicelik.
 * (c)2026, Cris Luengo.
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
#include "diplib/chain_code.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "diplib.h"
#include "diplib/geometry.h"
#include "diplib/linear.h"
#include "diplib/overload.h"


namespace dip {

namespace {

dfloat EvalCubic( dfloat p_1, dfloat p0, dfloat p1, dfloat p2, dfloat t ) {
   return(( ( -0.5 * p_1 + 1.5 * p0 - 1.5 * p1 + 0.5 * p2 ) * t +
            p_1 - 2.5 * p0 + 2.0 * p1 - 0.5 * p2 ) * t + 0.5 * ( p1 - p_1 )) * t + p0;
}

dfloat EvalCubicDerivative( dfloat p_1, dfloat p0, dfloat p1, dfloat p2, dfloat t ) {
   return( 3.0 * ( -0.5 * p_1 + 1.5 * p0 - 1.5 * p1 + 0.5 * p2 ) * t +
           2.0 * ( p_1 - 2.5 * p0 + 2.0 * p1 - 0.5 * p2 )) * t + 0.5 * ( p1 - p_1 );
}

// Finds t in [0,1] such that the Catmull-Rom spline through p_1, p0, p1 and p2 crosses zero between
// p0 (t=0) and p1 (t=1). Assumes p0 and p1 have different signs (or one is exactly zero).
dfloat FindZeroCubic( dfloat p_1, dfloat p0, dfloat p1, dfloat p2, int maxIter = 50, dfloat tol = 1e-12 ) {
   dfloat lo = 0.0, hi = 1.0;
   dfloat flo = p0, fhi = p1;
   // Handle extreme cases first
   if( flo == 0 ) {
      return 0;
   }
   if( fhi == 0 ) {
      return 1;
   }
   // if(( flo > 0.0 ) == ( fhi > 0.0 )) { return nan; } // This was tested for by the caller
   // Newton steps to find the zero crossing
   dfloat t = flo / ( flo - fhi ); // initial guess: linear interpolation
   dfloat f = EvalCubic( p_1, p0, p1, p2, t );
   for( int iter = 0; iter < maxIter; ++iter ) {
      dfloat df = EvalCubicDerivative( p_1, p0, p1, p2, t );
      dfloat tNext = t - f / df;
      // Accept Newton step only if it stays inside the bracket and the derivative isn't degenerate; otherwise bisect.
      if( !( std::isfinite( tNext ) && ( tNext > lo ) && ( tNext < hi ))) {
         tNext = 0.5 * ( lo + hi );
      }
      if( std::abs( t - tNext ) < tol ) {
         break;
      }
      t = tNext;
      f = EvalCubic( p_1, p0, p1, p2, t );
      if(( f > 0.0 ) == ( flo > 0.0 )) {
         lo = t;
         flo = f;
      } else {
         hi = t;
         fhi = f;
      }
      if(( hi - lo ) < tol ) {
         break;
      }
   }
   return t;
}

// Finds t in [0,1] such that the linear spline through p0 and p1 crosses zero between
// p0 (t=0) and p1 (t=1). Assumes p0 and p1 have different signs (or one is exactly zero).
dfloat FindZeroLinear( dfloat p0, dfloat p1 ) {
   // if(( p0 > 0.0 ) == ( p1 > 0.0 )) { return nan; } // This was tested for by the caller
   return p0 / ( p0 - p1 );
}

// Reads `out.size()` values starting at `startCoords`, every `step` pixels. Does not check that the coordinates are inside the image.
template< typename TPI >
void SamplePixels( Image const& in, FloatArray& out, VertexInteger const& startCoords, VertexInteger const& step ) {
   DIP_ASSERT( in.DataType() == DataType( TPI( 0 )));
   TPI const* ptr = static_cast< TPI const* >( in.Data() );
   auto coords = startCoords;
   for( dfloat& value : out ) {
      DIP_ASSERT( coords.x >= 0 && coords.x < static_cast< dip::sint >( in.Size( 0 )));
      DIP_ASSERT( coords.y >= 0 && coords.y < static_cast< dip::sint >( in.Size( 1 )));
      value = static_cast< dfloat >( ptr[ coords.x * in.Stride( 0 ) + coords.y * in.Stride( 1 ) ] );
      coords += step;
   }
}

typedef void ( *SampleFunction )( Image const&, FloatArray&, VertexInteger const&, VertexInteger const& );

VertexFloat Interpolate(
   Image const& in,
   VertexInteger const& startCoords,
   VertexInteger const& step,
   SampleFunction sampleFunction,
   dfloat threshold,
   bool cubic,
   FloatArray& samples
) {
   dfloat t{};
   if( cubic ) {
      sampleFunction( in, samples, startCoords - step, step );
      samples -= threshold;
      if(( samples[ 1 ] > 0.0 ) == ( samples[ 2 ] > 0.0 )) {
         t = 0; // We cannot interpolate, leave the vertex where it is. This should not happen if the chain code is obtained from the image at the given threshold.
      } else {
         t = FindZeroCubic( samples[ 0 ], samples[ 1 ], samples[ 2 ], samples[ 3 ] );
      }
   } else {
      sampleFunction( in, samples, startCoords, step );
      samples -= threshold;
      if(( samples[ 0 ] > 0.0 ) == ( samples[ 1 ] > 0.0 )) {
         t = 0; // We cannot interpolate, leave the vertex where it is. This should not happen if the chain code is obtained from the image at the given threshold.
      } else {
         t = FindZeroLinear( samples[ 0 ], samples[ 1 ] );
      }
   }
   return {
      static_cast< dfloat >( startCoords.x ) + t * static_cast< dfloat >( step.x ),
      static_cast< dfloat >( startCoords.y ) + t * static_cast< dfloat >( step.y )
   };
}

dfloat SamplePixelInterpolated(
   Image const& gray,
   VertexFloat position,
   InterpolationFunctionPointer interpolationFunction
) {
   return ResampleAtUnchecked( gray, { position.x, position.y }, interpolationFunction ).As< dfloat >();
}

} // namespace

dip::Polygon ChainCode::Polygon( dip::Image const& gray, dfloat threshold, String const& interpolation ) const {
   DIP_THROW_IF( !gray.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !gray.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( gray.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );
   DIP_THROW_IF( Empty() || codes.empty(), "Received empty chain code" );
   bool cubic{};
   DIP_STACK_TRACE_THIS( cubic = BooleanFromString( interpolation, S::CUBIC_ORDER_3, S::LINEAR ));
   SampleFunction sampleFunction{};
   DIP_OVL_ASSIGN_REAL( sampleFunction, SamplePixels, gray.DataType() );
   FloatArray samples( cubic ? 4 : 2 );

   // This function works only for 8-connected chain codes, convert it if it's 4-connected.
   ChainCode const& cc = is8connected ? *this : ConvertTo8Connected();

   static constexpr VertexInteger cardinal[ 4 ] = {{ 0, -1 }, { -1, 0 }, { 0, 1 }, { 1, 0 }};
   auto DecrementMod4 = []( unsigned& k ) { k = ( k == 0 ) ? 3 : ( k - 1 ); };

   VertexInteger pos = cc.start;
   dip::Polygon polygon;
   auto& vertices = polygon.vertices;
   Code m = cc.codes.back();
   for( Code n : cc.codes ) {
      if( !( n.IsBorder() && m.IsBorder() )) {
         unsigned k = ( m + 1 ) / 2;
         if( k == 4 ) {
            k = 0;
         }
         unsigned l = n / 2;
         if( l < k ) {
            l += 4;
         }
         l -= k;
         vertices.push_back( Interpolate( gray, pos, cardinal[ k ], sampleFunction, threshold, cubic, samples ));
         // TODO: Every time we go around the corner, we could add a vertex at the corner, interpolating diagonally.
         if( l != 0 ) {
            DecrementMod4( k );
            vertices.push_back( Interpolate( gray, pos, cardinal[ k ], sampleFunction, threshold, cubic, samples ));
            if( l <= 2 ) {
               DecrementMod4( k );
               vertices.push_back( Interpolate( gray, pos, cardinal[ k ], sampleFunction, threshold, cubic, samples ));
               if( l == 1 ) {
                  // This case is only possible if n is odd and n==m+4
                  DecrementMod4( k );
                  vertices.push_back( Interpolate( gray, pos, cardinal[ k ], sampleFunction, threshold, cubic, samples ));
               }
            }
         }
      }
      pos += deltas8[ n ];
      m = n;
   }
   return polygon;
}

dip::uint RefinePolygon(
      Polygon& polygon,
      Image const& gray,
      Image const& c_gradient,
      dfloat threshold,
      String const& interpolation,
      dfloat stepSize
) {
   DIP_THROW_IF( !gray.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !gray.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( gray.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::Image gradient;
   if( c_gradient.IsForged() ) {
      gradient = c_gradient.QuickCopy();
      DIP_THROW_IF( gradient.TensorElements() != 2, E::NTENSORELEM_DONT_MATCH );
      DIP_THROW_IF( gradient.Sizes() != gray.Sizes(), E::SIZES_DONT_MATCH );
   } else {
      gradient = dip::Gradient( gray );
   }
   DIP_THROW_IF( stepSize <= 0, E::INVALID_PARAMETER );
   bool cubic{};
   DIP_STACK_TRACE_THIS( cubic = BooleanFromString( interpolation, S::CUBIC_ORDER_3, S::LINEAR ));
   dip::sint margin = ceil_cast( stepSize * ( cubic ? 2 : 1 ));
   BoundingBoxInteger validArea{{ margin, margin },
                                { static_cast< dip::sint >( gray.Size( 0 )) - margin - 1, static_cast< dip::sint >( gray.Size( 1 )) - margin - 1 }};
   InterpolationFunctionPointer grayInterpolator = PrepareResampleAtUnchecked( gray, interpolation );
   InterpolationFunctionPointer gradientInterpolator = PrepareResampleAtUnchecked( gradient, interpolation );
   dip::uint nFailed = 0;
   for( dip::uint ii = 0; ii < polygon.vertices.size(); ++ii ) {
      VertexFloat& vertex = polygon.vertices[ ii ];
      if( !validArea.Contains( vertex )) {
         // We cannot refine the polygon because it's too close to the image edge, or outside the image altogether.
         ++nFailed;
         continue;
      }
      auto gradPixel = ResampleAtUnchecked( gradient, { vertex.x, vertex.y }, gradientInterpolator );
      VertexFloat normal = { gradPixel[ 0 ].As< dfloat >(), gradPixel[ 1 ].As< dfloat >() };
      if( normal.x == 0 && normal.y == 0 ) {
         // No gradient, cannot move the vertex
         ++nFailed;
         continue;
      }
      VertexFloat step = normal / Norm( normal ) * stepSize;
      dfloat p0 = SamplePixelInterpolated( gray, vertex, grayInterpolator ) - threshold;
      if( p0 > 0 ) {
         // We need to step downhill, not up
         step = -step;
      }
      dfloat p1 = SamplePixelInterpolated( gray, vertex + step, grayInterpolator ) - threshold;
      if(( p0 > 0 ) == ( p1 > 0 )) {
         // We're too far away from the threshold value, try again with a larger step size
         step *= 1.5;  // Note that this will reduce the accuracy of the interpolation
         p1 = SamplePixelInterpolated( gray, vertex + step, grayInterpolator ) - threshold;
         if(( p0 > 0 ) == ( p1 > 0 )) {
            // We're still too far away, we cannot move the vertex
            ++nFailed;
            continue;
         }
      }
      dfloat t{};
      if( cubic ) {
         dfloat p_1 = SamplePixelInterpolated( gray, vertex - step, grayInterpolator ) - threshold;
         dfloat p2 = SamplePixelInterpolated( gray, vertex + step * 2, grayInterpolator ) - threshold;
         t = FindZeroCubic( p_1, p0, p1, p2 );
      } else {
         t = FindZeroLinear( p0, p1 );
      }
      if( std::isnan( t )) {
         // The vertex is not within stepSize of threshold
         ++nFailed;
         continue;
      }
      vertex += step * t;
   }
   return nFailed;
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing FindZeroCubic()") {
   dip::dfloat t = dip::FindZeroCubic( -1.5, -0.5, 0.5, 1.5 );
   DOCTEST_CHECK( t == doctest::Approx( 0.5 ));
   t = dip::FindZeroCubic( -1.1, -0.1, 0.9, 1.9 );
   DOCTEST_CHECK( t == doctest::Approx( 0.1 ));
}

DOCTEST_TEST_CASE("[DIPlib] testing FindZeroLinear()") {
   dip::dfloat t = dip::FindZeroLinear( -0.5, 0.5 );
   DOCTEST_CHECK( t == doctest::Approx( 0.5 ));
   t = dip::FindZeroLinear( -0.1, 0.9 );
   DOCTEST_CHECK( t == doctest::Approx( 0.1 ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
