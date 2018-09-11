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
#include "diplib/transform.h"
#include "diplib/generation.h"

namespace dip {

namespace {

// Cohen–Sutherland Algorithm
// https://gist.githubusercontent.com/maxkarelov/293b5e4235c1e7dcdb40/raw/d92f331556ff74067a49b0676c35dbbc611ee25a/cohen-sutherland-algorithm.cp
bool clip(IntegerArray &A, IntegerArray &B, IntegerArray const &Pmax) {
   while (true) {
      unsigned int C1 = 0;
      if (A[ 0 ] < 0)         C1 += 1;
      if (A[ 0 ] > Pmax[ 0 ]) C1 += 2;
      if (A[ 1 ] < 0)         C1 += 4;
      if (A[ 1 ] > Pmax[ 1 ]) C1 += 8;

      unsigned int C2 = 0;
      if (B[ 0 ] < 0)         C2 += 1;
      if (B[ 0 ] > Pmax[ 0 ]) C2 += 2;
      if (B[ 1 ] < 0)         C2 += 4;
      if (B[ 1 ] > Pmax[ 1 ]) C2 += 8;

      if ((C1 == C2) && (C1 == 0)) return true;

      if ((C1 & C2) != 0) return false;

      if (C1 == 0) {
         std::swap(A, B);
      }

      if ((C1 & 1) != 0) {
         A[ 1 ] = B[ 1 ] - (B[ 0 ]            ) * (B[ 1 ] - A[ 1 ]) / (B[ 0 ] - A[ 0 ]);
         A[ 0 ] = 0;
      } else if ((C1 & 2) != 0) {
         A[ 1 ] = B[ 1 ] - (B[ 0 ] - Pmax[ 0 ]) * (B[ 1 ] - A[ 1 ]) / (B[ 0 ] - A[ 0 ]);
         A[ 0 ] = Pmax[ 0 ];
      } else if ((C1 & 4) != 0) {
         A[ 0 ] = B[ 0 ] - (B[ 1 ]            ) * (B[ 0 ] - A[ 0 ]) / (B[ 1 ] - A[ 1 ]);
         A[ 1 ] = 0;
      } else if ((C1 & 8) != 0) {
         A[ 0 ] = B[ 0 ] - (B[ 1 ] - Pmax[ 1 ]) * (B[ 0 ] - A[ 0 ]) / (B[ 1 ] - A[ 1 ]);
         A[ 1 ] = Pmax[ 1 ];
      }
   }
}

} // namespace

void HoughTransformCircleCenters(
      Image const& in,
      Image const& gv,
      Image& out,
      UnsignedArray const &range
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !gv.IsForged(), E::IMAGE_NOT_FORGED );

   auto nDims = in.Dimensionality();
   DIP_THROW_IF( nDims != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.DataType() != DT_BIN, E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( gv.Dimensionality() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
   DIP_THROW_IF( gv.TensorElements() != 2, "Only defined for 2-vector images" );
   
   dip::IntegerArray sz{ static_cast< dip::sint >( in.Size(0) - 1), 
                         static_cast< dip::sint >( in.Size(1) - 1) };
   dip::dfloat minsz, maxsz;
   if ( range.empty() ) {
      minsz = 0;
      maxsz = std::sqrt( sz[ 0] * sz[ 0 ] + sz[ 1 ] * sz[ 1 ] );
   } else {
      DIP_THROW_IF( range.size() != 2, E::DIMENSIONALITIES_DONT_MATCH );
      minsz = static_cast< dip::dfloat >( range[ 0 ] );
      maxsz = static_cast< dip::dfloat >( range[ 1 ] );
   }
   
   // Initialize accumulator
   out.ReForge( in.Sizes(), 1, DT_SFLOAT );
   out.Fill(0);
   
   auto coordComp = gv.OffsetToCoordinatesComputer();

   // Iterate over on pixels
   // NOTE: calling end() on View does not work
   for (auto it = gv.At( in ).begin(); it; ++it) {
      auto coord = coordComp( it.Offset() );
      dip::sint x = static_cast< dip::sint >( coord[ 0 ] ),
                y = static_cast< dip::sint >( coord[ 1 ] );
      
      dip::dfloat angle = std::atan2( static_cast< dip::dfloat >( it[ 1 ] ), static_cast< dip::dfloat >( it[ 0 ] ) );

      // TODO: option to select inside or outside
      if (minsz == 0) {
         // Draw single line
         dip::sint cmax = static_cast< dip::sint >( std::cos( angle ) * maxsz ),
                   smax = static_cast< dip::sint >( std::sin( angle ) * maxsz );
         
         IntegerArray start{ x - cmax, y - smax }, end{ x + cmax, y + smax };
         
         if ( clip( start, end, sz ) ) {
            // Note that after clipping we can be sure that all coordinates are positive
            DrawLine( out, UnsignedArray(std::move(start)), UnsignedArray(std::move(end)), { 1 }, S::ADD );
         }
      } else {
         // Draw two line segments
         dip::sint cmin = static_cast< dip::sint >( std::cos( angle ) * minsz ),
                   smin = static_cast< dip::sint >( std::sin( angle ) * minsz ),
                   cmax = static_cast< dip::sint >( std::cos( angle ) * maxsz ),
                   smax = static_cast< dip::sint >( std::sin( angle ) * maxsz );
         
         IntegerArray start1{ x - cmin, y - smin }, end1{ x - cmax, y - smax },
                      start2{ x + cmin, y + smin }, end2{ x + cmax, y + smax };
         
         if ( clip( start1, end1, sz ) ) {
            DrawLine( out, UnsignedArray(std::move(start1)), UnsignedArray(std::move(end1)), { 1 }, S::ADD );
         }
         if ( clip( start2, end2, sz ) ) {
            DrawLine( out, UnsignedArray(std::move(start2)), UnsignedArray(std::move(end2)), { 1 }, S::ADD );
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
   auto a = dip::Image( {1024, 1024}, 1, dip::DT_SFLOAT );
   a.Fill( 0 );
   dip::DrawEllipsoid( a, {200, 200}, {512, 512} );
   
   // Try to find it
   auto gv  = dip::Gradient( a );
   auto gm  = dip::Norm( gv );
   auto bin = dip::IsodataThreshold( gm, { } );
   auto h   = dip::HoughTransformCircleCenters( bin, gv );
   auto f   = dip::Gauss( h, { 5 } );
   auto m   = dip::MaximumPixel( f );

   // Check result   
   DOCTEST_CHECK( m[0] == 512 );
   DOCTEST_CHECK( m[1] == 512 );
}

#endif // DIP__ENABLE_DOCTEST
