/*
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

#include "diplib/chain_code.h"

#include <array>

#include "diplib.h"

namespace dip {

ChainCode ChainCode::ConvertTo8Connected() const {
   if( is8connected ) {
      return *this;
   }
   ChainCode out;
   out.objectID = objectID;
   out.start = start;
   if( codes.size() < 3 ) {
      #if __GNUC__ >= 12 && __GNUC__ <= 14
         // A bug in GCC 12 puts out a bogus warning here
         #pragma GCC diagnostic push
         #pragma GCC diagnostic ignored "-Wstringop-overflow="
      #endif
      out.codes = codes;
      for( auto& c : out.codes ) {
         c = Code( c * 2, c.IsBorder() );
      }
      #if __GNUC__ >= 12 && __GNUC__ <= 14
         #pragma GCC diagnostic pop
      #endif
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

ChainCode ChainCode::Offset() const {
   DIP_THROW_IF( !is8connected, "This method is only defined for 8-connected chain codes" );
   ChainCode out;
   out.objectID = objectID;
   out.is8connected = true;
   if( Empty() ) {
      return {}; // There's no chain code, return an empty polygon
   }
   if( codes.empty() ) {
      out.start = start + deltas8[ 2 ];
      out.Push( 7 );
      out.Push( 5 );
      out.Push( 3 );
      out.Push( 1 );
      return out;
   }
   unsigned prev = codes.back();
   out.start = start + deltas8[ ( prev + ( codes.back().IsEven() ? 2u : 3u )) % 8u ];
   for( auto code : codes ) {
      unsigned n = ( code < prev ) ? ( code + 8u - prev ) : ( code - prev );
      if( code.IsEven() ) {
         switch( n ) {
            case 4: // -4
            case 5: // -3
               out.Push( { code + 3, code.IsBorder() } );
               // fallthrough
            case 6: // -2
            case 7: // -1
               out.Push( { code + 1, code.IsBorder() } );
               // fallthrough
            case 0:
            case 1:
               out.Push( code );
               break;
            default:
               DIP_THROW_ASSERTION( E::NOT_REACHABLE );
         }
      } else {
         switch( n ) {
            case 4: // -4
               out.Push( { code + 4, code.IsBorder() } );
               // fallthrough
            case 5: // -3
            case 6: // -2
               out.Push( { code + 2, code.IsBorder() } );
               // fallthrough
            case 7: // -1
            case 0:
               out.Push( code );
               // fallthrough
            case 1:
            case 2:
               // no points to add
               break;
            default:
               DIP_THROW_ASSERTION( E::NOT_REACHABLE );
         }
      }
      prev = code;
   }
   return out;
}

namespace {

inline void DecrementMod4( unsigned& k ) {
   k = ( k == 0 ) ? ( 3 ) : ( k - 1 );
}

} // namespace

// This Algorithm to make a polygon from a chain code is home-brewed.
// The concept of using pixel edge midpoints is from Steve Eddins:
// http://blogs.mathworks.com/steve/2011/10/04/binary-image-convex-hull-algorithm-notes/
dip::Polygon ChainCode::Polygon() const {
   DIP_THROW_IF( codes.size() == 1, "Received a weird chain code as input (N==1)" );

   if( Empty() ) {
      return {}; // There's no chain code, return an empty polygon
   }

   // This function works only for 8-connected chain codes, convert it if it's 4-connected.
   ChainCode const& cc = is8connected ? *this : ConvertTo8Connected();

   std::array< VertexFloat, 4 > pts;
   pts[ 0 ] = {  0.0, -0.5 };
   pts[ 1 ] = { -0.5,  0.0 };
   pts[ 2 ] = {  0.0,  0.5 };
   pts[ 3 ] = {  0.5,  0.0 };

   VertexFloat pos { dfloat( cc.start.x ), dfloat( cc.start.y ) };
   dip::Polygon polygon;
   auto& vertices = polygon.vertices;

   if( cc.codes.empty() ) {
      // A 1-pixel object.
      vertices.push_back( pts[ 0 ] + pos );
      vertices.push_back( pts[ 3 ] + pos );
      vertices.push_back( pts[ 2 ] + pos );
      vertices.push_back( pts[ 1 ] + pos );
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
         vertices.push_back( pts[ k ] + pos );
         if( l != 0 ) {
            DecrementMod4( k );
            vertices.push_back( pts[ k ] + pos );
            if( l <= 2 ) {
               DecrementMod4( k );
               vertices.push_back( pts[ k ] + pos );
               if( l == 1 ) {
                  // This case is only possible if n is odd and n==m+4
                  DecrementMod4( k );
                  vertices.push_back( pts[ k ] + pos );
               }
            }
         }
         pos += deltas8[ n ];
         m = n;
      }
   }
   return polygon;
}

CoordinateArray ChainCode::Coordinates() const {
   if( Empty() ) {
      return {};
   }
   CoordinateArray out;
   out.reserve( codes.size() + 1 );
   VertexInteger const* freeman = is8connected ? deltas8 : deltas4;
   VertexInteger pos = start;
   for( auto code : codes ) {
      out.push_back( { static_cast< dip::uint >( pos.x ), static_cast< dip::uint >( pos.y ) } );
      pos += freeman[ code ];
   }
   if( pos != start ) {
      // The last pixel we only output if it's not the same as the first.
      out.push_back( { static_cast< dip::uint >( pos.x ), static_cast< dip::uint >( pos.y ) } );
   }
   return out;
}

}

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

#include "diplib/pixel_table.h"
#include "diplib/morphology.h"

DOCTEST_TEST_CASE("[DIPlib] testing ChainCode::Offset") {
   dip::Image img = dip::PixelTable( "elliptic", { 29, 29 } ).AsImage(); // Currently the easiest way of generating an image...
   img = img.Pad( { 33, 33 } );
   dip::ChainCode cc1 = dip::GetSingleChainCode( img, { 16, 2 }, 2 );
   cc1 = cc1.Offset();
   img = dip::Dilation( img, { 3, "diamond" } );
   dip::ChainCode cc2 = dip::GetSingleChainCode( img, { 16, 1 }, 2 );
   DOCTEST_REQUIRE( cc1.codes.size() == cc2.codes.size() );
   for( dip::uint ii = 0; ii < cc1.codes.size(); ++ii ) {
      DOCTEST_CHECK( cc1.codes[ ii ] == cc2.codes[ ii ] );
   }
}

DOCTEST_TEST_CASE("[DIPlib] testing chain code conversion to polygon") {
   dip::ChainCode cc8;
   cc8.codes = { 0u, 0u, 7u, 6u, 6u, 5u, 4u, 4u, 3u, 2u, 2u, 1u }; // A chain code that is a little circle.
   cc8.is8connected = true;
   cc8.start = { 10, 12 };
   dip::ChainCode cc4;
   cc4.codes = { 0u, 0u, 3u, 0u, 3u, 3u, 2u, 3u, 2u, 2u, 1u, 2u, 1u, 1u, 0u, 1u }; // A 4-connected chain code for the same object.
   cc4.is8connected = false;
   cc4.start = { 10, 12 };
   auto P8 = cc8.Polygon();
   auto P4 = cc4.Polygon();
   DOCTEST_REQUIRE( P8.vertices.size() == P4.vertices.size() );
   for( dip::uint ii = 0; ii < P8.vertices.size(); ++ii ) {
      DOCTEST_CHECK( P8.vertices[ ii ] == P4.vertices[ ii ] );
   }

   auto C8 = cc8.Coordinates();
   DOCTEST_REQUIRE( C8.size() == cc8.codes.size() );
   DOCTEST_CHECK( C8[ 0 ] == dip::UnsignedArray( { 10, 12 } ));
   DOCTEST_CHECK( C8[ 1 ] == dip::UnsignedArray( { 11, 12 } ));
   DOCTEST_CHECK( C8[ 2 ] == dip::UnsignedArray( { 12, 12 } ));
   DOCTEST_CHECK( C8[ 3 ] == dip::UnsignedArray( { 13, 13 } ));
   DOCTEST_CHECK( C8.back() == dip::UnsignedArray( { 9, 13 } ));

   auto C4 = cc4.Coordinates();
   DOCTEST_REQUIRE( C4.size() == cc4.codes.size() );
   DOCTEST_CHECK( C4[ 0 ] == dip::UnsignedArray( { 10, 12 } ));
   DOCTEST_CHECK( C4[ 1 ] == dip::UnsignedArray( { 11, 12 } ));
   DOCTEST_CHECK( C4[ 2 ] == dip::UnsignedArray( { 12, 12 } ));
   DOCTEST_CHECK( C4[ 3 ] == dip::UnsignedArray( { 12, 13 } ));
   DOCTEST_CHECK( C4.back() == dip::UnsignedArray( { 10, 13 } ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
