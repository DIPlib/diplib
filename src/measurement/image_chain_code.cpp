/*
 * (c)2016-2021, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#include <vector>

#include "diplib.h"
#include "diplib/overload.h"
#include "diplib/private/robin_map.h"
#include "diplib/regions.h"

namespace dip {

// We need storage for these tables, as we take pointers to them.
constexpr VertexInteger ChainCode::deltas4[4];
constexpr VertexInteger ChainCode::deltas8[8];

namespace {

struct ObjectData { dip::uint index; bool done; };
using ObjectIdList = tsl::robin_map< LabelType, ObjectData >; // key is the objectID (label)

template< typename TPI >
ChainCode GetOneChainCode(
      void const* data_ptr,
      VertexInteger coord, // starting coordinates
      VertexInteger const& dims,  // largest coordinates in image
      dip::uint connectivity,
      ChainCode::CodeTable const& codeTable,
      bool startDir0 = false
) {
   TPI const* data = static_cast< TPI const* >( data_ptr );
   LabelType label = CastLabelType( *data );
   DIP_THROW_IF( label == 0, "Start coordinates not on object boundary" );
   // Initialize the chain code of the object
   ChainCode out;
   out.start = coord;
   out.objectID = label;
   out.is8connected = connectivity != 1; // 0 means 8-connected also
   // Directions
   unsigned right = 0;
   unsigned up = out.is8connected ? 2 : 1;
   unsigned left = out.is8connected ? 4 : 2;
   unsigned down = out.is8connected ? 6 : 3;
   unsigned last = out.is8connected ? 7 : 3;
   // Follow contour always as left as possible (i.e. ii = ii+2)
   dip::sint offset = 0;
   unsigned dir = 0; // start direction given by how we determine the start position!
   if( !startDir0 ) {
      // In this case, we cannot be sure of the start direction. Let's look for a background pixel first.
      while( true ) {
         VertexInteger nc = coord + codeTable.pos[ dir ];
         dip::sint no = codeTable.offset[ dir ];
         if(( nc.x < 0 ) || ( nc.x > dims.x ) || ( nc.y < 0 ) || ( nc.y > dims.y ) || ( data[ no ] != label )) {
            break;
         }
         ++dir;
         DIP_THROW_IF( dir == last + 1, "Start coordinates not on object boundary" );
      }
   }
   unsigned startdir = dir;
   do {
      VertexInteger nc = coord + codeTable.pos[ dir ];
      dip::sint no = offset + codeTable.offset[ dir ];
      bool outOfBounds = ( nc.x < 0 ) || ( nc.x > dims.x ) || ( nc.y < 0 ) || ( nc.y > dims.y );
      if( !outOfBounds && ( data[ no ] == label )) {
         // Add new chain
         bool isBorder = (( nc.x == 0 ) && ( dir == up )) ||
                         (( nc.x == dims.x ) && ( dir == down )) ||
                         (( nc.y == 0 ) && ( dir == right )) ||
                         (( nc.y == dims.y ) && ( dir == left ) );
         out.Push( { dir, isBorder } );
         // New position
         coord = nc;
         offset = no;
         // Start direction to search
         dir = out.is8connected ? ( dir + 2 ) % 8 : ( dir + 1 ) % 4;
      } else {
         // New direction to search
         if( dir == 0 ) {
            dir = last;
         } else {
            dir--;
         }
      }
   } while( !(( coord == out.start ) && ( dir == startdir )));
   return out;
}

template< typename TPI >
ChainCodeArray GetImageChainCodesInternal(
      Image const& labels,
      ObjectIdList& objectIDs,
      dip::uint nObjects, // potentially different from the number of entries in objectIDs, if there were repeated elements in the original list.
      dip::uint connectivity,
      ChainCode::CodeTable const& codeTable
) {
   DIP_ASSERT( labels.DataType() == DataType( TPI( 0 )));
   TPI* data = static_cast< TPI* >( labels.Origin() );
   ChainCodeArray ccArray( nObjects );  // output array
   VertexInteger dims = { static_cast< dip::sint >( labels.Size( 0 ) - 1 ), static_cast< dip::sint >( labels.Size( 1 ) - 1 ) }; // our local copy of `dims` now contains the largest coordinates
   IntegerArray const& strides = labels.Strides();

   // Find first pixel of requested label
   LabelType label = 0;
   VertexInteger coord;
   for( coord.y = 0; coord.y <= dims.y; ++coord.y ) {
      dip::sint pos = coord.y * strides[ 1 ];
      for( coord.x = 0; coord.x <= dims.x; ++coord.x ) {
         bool process = false;
         dip::uint index = 0;
         LabelType newlabel = CastLabelType( data[ pos ] );
         if(( newlabel != 0 ) && ( newlabel != label )) {
            // Check whether newlabel is start of not processed object
            auto it = objectIDs.find( newlabel );
            if(( it != objectIDs.end() ) && !it.value().done ) {
               it.value().done = true;
               index = it.value().index;
               label = newlabel;
               process = true;
            }
         }
         if( process ) {
            ccArray[ index ] = GetOneChainCode< TPI >( data + pos, coord, dims, connectivity, codeTable, true );
         }
         pos += strides[ 0 ];
      }
   }
   return ccArray;
}

} // namespace

ChainCodeArray GetImageChainCodes(
      Image const& labels,
      std::vector< LabelType > const& objectIDs,
      dip::uint connectivity
) {
   // Check input image
   DIP_THROW_IF( !labels.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( labels.CheckProperties( 2, 1, DataType::Class_UInt ));
   DIP_THROW_IF( connectivity > 2, E::CONNECTIVITY_NOT_SUPPORTED );

   // Initialize freeman codes
   ChainCode::CodeTable codeTable = ChainCode::PrepareCodeTable( connectivity, labels.Strides() );

   // Create a map for the object IDs
   std::vector< LabelType > const& ids = objectIDs.empty() ? ListObjectLabels( labels, Image(), S::EXCLUDE ) : objectIDs;
   ObjectIdList objectIdList;
   objectIdList.reserve( ids.size() * 2 );
   for( dip::uint ii = 0; ii < ids.size(); ++ii ) {
      objectIdList.emplace( ids[ ii ], ObjectData{ ii, false } );
   }
   dip::uint nObjects = ids.size();

   // Get the chain code for each label
   ChainCodeArray ccArray;
   DIP_OVL_CALL_ASSIGN_UINT( ccArray,
                             GetImageChainCodesInternal, ( labels, objectIdList, nObjects, connectivity, codeTable ),
                             labels.DataType() );
   return ccArray;
}

ChainCode GetSingleChainCode(
      Image const& labels,
      UnsignedArray const& startCoord,
      dip::uint connectivity
) {
   // Check input image
   DIP_THROW_IF( !labels.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( labels.CheckProperties( 2, 1, DataType::Class_Unsigned ));
   DIP_THROW_IF( connectivity > 2, E::CONNECTIVITY_NOT_SUPPORTED );

   // Initialize freeman codes
   ChainCode::CodeTable codeTable = ChainCode::PrepareCodeTable( connectivity, labels.Strides() );

   // Get the chain code for each label
   void const* data = labels.Pointer( startCoord );
   VertexInteger coord = { static_cast< dip::sint >( startCoord[ 0 ] ), static_cast< dip::sint >( startCoord[ 1 ] ) };
   VertexInteger dims = { static_cast< dip::sint >( labels.Size( 0 ) - 1 ), static_cast< dip::sint >( labels.Size( 1 ) - 1 ) };
   ChainCode cc;
   DataType dtype = labels.DataType();
   if( dtype.IsBinary() ) {
      dtype = DT_UINT8;  // same memory layout
   }
   DIP_OVL_CALL_ASSIGN_UINT( cc, GetOneChainCode, ( data, coord, dims, connectivity, codeTable ), dtype );
   return cc;
}

void ChainCode::Image( dip::Image& out ) const {
   dip::BoundingBoxInteger bb = BoundingBox();
   UnsignedArray size { bb.Size() };
   out.ReForge( size, 1, DT_BIN );
   out = false; // set all pixels to false
   if( Empty() ) {
      return;
   }
   ChainCode::CodeTable freeman = PrepareCodeTable( out.Strides() );
   VertexInteger coord = start - bb.topLeft;
   dip::sint offset = coord.x * out.Stride( 0 ) + coord.y * out.Stride( 1 );
   dip::bin* ptr = static_cast< dip::bin* >( out.Origin() ) + offset;
   *ptr = true; // Set the start pixel, even if it will be set again by the last chain code, because it's possible that the chain code is not closed.
   for( auto code : codes ) {
      ptr += freeman.offset[ code ];
      *ptr = true;
   }
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing chain code conversion to image and back") {
   dip::ChainCode cc;
   cc.codes = { 0u, 0u, 7u, 6u, 6u, 5u, 4u, 4u, 3u, 2u, 2u, 1u }; // A chain code that is a little circle.
   cc.start = { 1, 0 }; // start so that the bounding box has a top-left corner at {0,0}.
   cc.is8connected = true;
   dip::Image img = cc.Image();
   dip::ChainCode cc2 = dip::GetSingleChainCode( img, { 1, 0 }, 2 );
   DOCTEST_CHECK( cc2.start == cc.start );
   DOCTEST_CHECK( cc2.is8connected );
   DOCTEST_REQUIRE( cc2.codes.size() == cc.codes.size() );
   for( dip::uint ii = 0; ii < cc2.codes.size(); ++ii ) {
      DOCTEST_CHECK( cc.codes[ ii ] == cc2.codes[ ii ] );
   }
}

DOCTEST_TEST_CASE("[DIPlib] testing chain codes on image border") {
   dip::Image img( { 9, 9 }, 1, dip::DT_BIN );
   img.Fill( false );
   img.At( dip::Range( 3, -1 ), dip::Range( 2, -3 )) = true;
   // 8-connected
   dip::ChainCode cc = dip::GetSingleChainCode( img, { 3, 2 }, 2 );
   dip::uint borderCodes = 0;
   for( auto code : cc.codes ) {
      borderCodes += code.IsBorder();
   }
   DOCTEST_CHECK( borderCodes == 4 );
   dip::Polygon p1 = cc.Polygon();
   dip::Polygon p2 = cc.Polygon( "lose" );
   DOCTEST_CHECK( p1.vertices.size() == p2.vertices.size() + 3 );
   // 4-connected
   cc = dip::GetSingleChainCode( img, { 3, 2 }, 1 );
   borderCodes = 0;
   for( auto code : cc.codes ) {
      borderCodes += code.IsBorder();
   }
   DOCTEST_CHECK( borderCodes == 4 );
   p1 = cc.Polygon();
   p2 = cc.Polygon( "lose" );
   DOCTEST_CHECK( p1.vertices.size() == p2.vertices.size() + 3 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
