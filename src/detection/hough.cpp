/*
 * DIPlib 3.0
 * This file contains definitions of various Hough Transform functions.
 *
 * (c)2017, Wouter Caarls
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
#include "diplib/detection.h"
#include "diplib/generation.h"

namespace dip {

namespace {

struct IntegerCoords{
   dip::sint x;
   dip::sint y;

   operator UnsignedArray() {
      return { static_cast< dip::uint >( x ), static_cast< dip::uint >( y ) };
   }
};

IntegerCoords operator+( IntegerCoords lhs, IntegerCoords rhs ) {
   lhs.x += rhs.x;
   lhs.y += rhs.y;
   return lhs;
}

IntegerCoords operator-( IntegerCoords lhs, IntegerCoords rhs ) {
   lhs.x -= rhs.x;
   lhs.y -= rhs.y;
   return lhs;
}

// Cohen–Sutherland Algorithm
// https://gist.githubusercontent.com/maxkarelov/293b5e4235c1e7dcdb40/raw/d92f331556ff74067a49b0676c35dbbc611ee25a/cohen-sutherland-algorithm.cp
bool clip( IntegerCoords& A, IntegerCoords& B, IntegerCoords Pmax ) {
   while( true ) {
      unsigned int C1 = 0;
      if( A.x < 0 )      { C1 += 1; }
      if( A.x > Pmax.x ) { C1 += 2; }
      if( A.y < 0 )      { C1 += 4; }
      if( A.y > Pmax.y ) { C1 += 8; }

      unsigned int C2 = 0;
      if( B.x < 0 )      { C2 += 1; }
      if( B.x > Pmax.x ) { C2 += 2; }
      if( B.y < 0 )      { C2 += 4; }
      if( B.y > Pmax.y ) { C2 += 8; }

      if(( C1 == C2 ) && ( C1 == 0 )) { return true; }
      if(( C1 & C2 ) != 0) { return false; }
      if( C1 == 0 ) { std::swap( A, B ); }

      if(( C1 & 1 ) != 0 ) {
         A.y = B.y - ( B.x          ) * ( B.y - A.y ) / ( B.x - A.x );
         A.x = 0;
      } else if(( C1 & 2 ) != 0 ) {
         A.y = B.y - ( B.x - Pmax.x ) * ( B.y - A.y ) / ( B.x - A.x );
         A.x = Pmax.x;
      } else if(( C1 & 4 ) != 0 ) {
         A.x = B.x - ( B.y          ) * ( B.x - A.x ) / ( B.y - A.y );
         A.y = 0;
      } else if(( C1 & 8 ) != 0 ) {
         A.x = B.x - ( B.y - Pmax.y ) * ( B.x - A.x ) / ( B.y - A.y );
         A.y = Pmax.y;
      }
   }
}

} // namespace

void HoughTransformCircleCenters(
      Image const& in,
      Image const& gv,
      Image& out,
      UnsignedArray const& range
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !gv.IsForged(), E::IMAGE_NOT_FORGED );

   auto nDims = in.Dimensionality();
   DIP_THROW_IF( nDims != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.DataType() != DT_BIN, E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( gv.Dimensionality() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
   DIP_THROW_IF( gv.TensorElements() != 2, "Only defined for 2-vector images" );

   IntegerCoords sz{ static_cast< dip::sint >( in.Size( 0 ) - 1 ),
                     static_cast< dip::sint >( in.Size( 1 ) - 1 ) };
   dfloat minsz;
   dfloat maxsz;
   if( range.empty() ) {
      minsz = 0;
      maxsz = std::hypot( sz.x, sz.y );
   } else {
      DIP_THROW_IF( range.size() != 2, E::DIMENSIONALITIES_DONT_MATCH );
      minsz = static_cast< dfloat >( range[ 0 ] );
      maxsz = static_cast< dfloat >( range[ 1 ] );
   }

   // Initialize accumulator
   out.ReForge( in.Sizes(), 1, DT_SFLOAT );
   out.Fill( 0 );

   auto coordComp = gv.OffsetToCoordinatesComputer();

   // Iterate over on pixels
   // NOTE: calling end() on View does not work
   for( auto it = gv.At( in ).begin(); it; ++it ) {
      auto coord = coordComp( it.Offset() );
      IntegerCoords c{ static_cast< dip::sint >( coord[ 0 ] ),
                       static_cast< dip::sint >( coord[ 1 ] ) };
      dfloat angle = std::atan2( static_cast< dfloat >( it[ 1 ] ), static_cast< dfloat >( it[ 0 ] ));
      // TODO: option to select inside or outside
      IntegerCoords max = { static_cast< dip::sint >( std::round( std::cos( angle ) * maxsz )),
                            static_cast< dip::sint >( std::round( std::sin( angle ) * maxsz )) };
      if( minsz == 0 ) {
         // Draw single line
         IntegerCoords start = c - max;
         IntegerCoords end = c + max;
         if( clip( start, end, sz )) {
            // Note that after clipping we can be sure that all coordinates are positive
            DrawLine( out, start, end, { 1 }, S::ADD );
         }
      } else {
         // Draw two line segments
         IntegerCoords min = { static_cast< dip::sint >( std::round( std::cos( angle ) * minsz )),
                               static_cast< dip::sint >( std::round( std::sin( angle ) * minsz )) };
         IntegerCoords start = c - min;
         IntegerCoords end = c - max;
         if( clip( start, end, sz )) {
            DrawLine( out, start, end, { 1 }, S::ADD );
         }
         start = c + min;
         end = c + max;
         if( clip( start, end, sz )) {
            DrawLine( out, start, end, { 1 }, S::ADD );
         }
      }
   }
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/linear.h"
#include "diplib/segmentation.h"
#include "diplib/math.h"
#include "diplib/statistics.h"

DOCTEST_TEST_CASE("[DIPlib] testing the HoughTransformCircleCenters function") {
   // Draw a circle
   auto a = dip::Image( { 512, 512 }, 1, dip::DT_SFLOAT );
   a.Fill( 0 );
   dip::DrawEllipsoid( a, { 200, 200 }, { 256, 256 } );

   // Try to find it
   auto gv  = dip::Gradient( a );
   auto gm  = dip::Norm( gv );
   auto bin = dip::IsodataThreshold( gm, {} );
   auto h   = dip::HoughTransformCircleCenters( bin, gv );
   auto f   = dip::Gauss( h, { 5 } );
   auto m   = dip::MaximumPixel( f );

   // Check result
   DOCTEST_CHECK( m[0] == 256 );
   DOCTEST_CHECK( m[1] == 256 );
}

#endif // DIP__ENABLE_DOCTEST
