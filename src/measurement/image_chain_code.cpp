/*
 * DIPlib 3.0
 * This file contains definitions for functions that create chain codes from object outlines.
 *
 * (c)2016-2017, Cris Luengo.
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

#include <array>
#include <map>

#include "diplib.h"
#include "diplib/chain_code.h"
#include "diplib/regions.h"
#include "diplib/overload.h"

namespace dip {

// We need storage for these tables, as we take pointers to them.
constexpr VertexInteger ChainCode::deltas4[4];
constexpr VertexInteger ChainCode::deltas8[8];

namespace {

struct ObjectData { dip::uint index; bool done; };
using ObjectIdList = std::map< dip::uint, ObjectData >; // key is the objectID (label)

template< typename TPI >
static ChainCode dip__OneChainCode(
      void const* data_ptr,
      VertexInteger coord, // starting coordinates
      VertexInteger const& dims,  // largest coordinates in image
      dip::uint connectivity,
      ChainCode::CodeTable const& codeTable,
      bool startDir0 = false
) {
   TPI const* data = static_cast< TPI const* >( data_ptr );
   dip::uint label = *data;
   DIP_THROW_IF( label == 0, "Start coordinates not on object boundary" );
   // Initialize the chain code of the object
   ChainCode out;
   out.start = coord;
   out.objectID = label;
   out.is8connected = connectivity != 1; // 0 means 8-connected also
   // Follow contour always as left as possible (i.e. ii = ii+2)
   dip::sint offset = 0;
   unsigned dir = 0; // start direction given by how we determine the start position!
   if( !startDir0 ) {
      // In this case, we cannot be sure of the start direction. Let's look for a background pixel first.
      while( true ) {
         VertexInteger nc = coord + codeTable.pos[ dir ];
         dip::sint no = codeTable.offset[ dir ];
         if( ( nc.x < 0 ) || ( nc.x > dims.x ) || ( nc.y < 0 ) || ( nc.y > dims.y ) || ( ( data[ no ] ) != label ) ) {
            break;
         }
         ++dir;
         DIP_THROW_IF( dir == ( out.is8connected ? 8 : 4 ), "Start coordinates not on object boundary" );
      }
   }
   unsigned startdir = dir;
   do {
      VertexInteger nc = coord + codeTable.pos[ dir ];
      dip::sint no = offset + codeTable.offset[ dir ];
      if(( nc.x >= 0 ) && ( nc.x <= dims.x ) && ( nc.y >= 0 ) && ( nc.y <= dims.y ) && (( data[ no ] ) == label ) ) {
         // Add new chain
         bool border = ( nc.x == 0 ) || ( nc.x == dims.x ) || ( nc.y == 0 ) || ( nc.y == dims.y );
         out.Push( { dir, border } );
         // New position
         coord = nc;
         offset = no;
         // Start direction to search
         dir = out.is8connected ? ( dir + 2 ) % 8 : ( dir + 1 ) % 4;
      } else {
         // New direction to search
         if( dir == 0 ) {
            dir = out.is8connected ? 7 : 3;
         } else {
            dir--;
         }
      }
   } while( !(( coord == out.start ) && ( dir == startdir )));
   return out;
}

template< typename TPI >
static ChainCodeArray dip__ChainCodes(
      Image const& labels,
      ObjectIdList& objectIDs,
      dip::uint nObjects, // potentially different from the number of entries in objectIDs, if there were repeated elements in the original list.
      dip::uint connectivity,
      ChainCode::CodeTable const& codeTable
) {
   DIP_ASSERT( labels.DataType() == DataType( TPI( 0 ) ) );
   TPI* data = static_cast< TPI* >( labels.Origin() );
   ChainCodeArray ccArray( nObjects );  // output array
   VertexInteger dims = { static_cast< dip::sint >( labels.Size( 0 ) - 1 ), static_cast< dip::sint >( labels.Size( 1 ) - 1 ) }; // our local copy of `dims` now contains the largest coordinates
   IntegerArray const& strides = labels.Strides();

   // Find first pixel of requested label
   dip::uint label = 0;
   VertexInteger coord;
   for( coord.y = 0; coord.y <= dims.y; ++coord.y ) {
      dip::sint pos = coord.y * strides[ 1 ];
      for( coord.x = 0; coord.x <= dims.x; ++coord.x ) {
         bool process = false;
         dip::uint index = 0;
         dip::uint newlabel = data[ pos ];
         if( ( newlabel != 0 ) && ( newlabel != label ) ) {
            // Check whether newlabel is start of not processed object
            auto it = objectIDs.find( newlabel );
            if( ( it != objectIDs.end() ) && !it->second.done ) {
               it->second.done = true;
               index = it->second.index;
               label = newlabel;
               process = true;
            }
         }
         if( process ) {
            ccArray[ index ] = dip__OneChainCode< TPI >( data + pos, coord, dims, connectivity, codeTable, true );
         }
         pos += strides[ 0 ];
      }
   }
   return ccArray;
}

} // namespace

ChainCodeArray GetImageChainCodes(
      Image const& labels,
      UnsignedArray const& objectIDs,
      dip::uint connectivity
) {
   // Check input image
   DIP_THROW_IF( !labels.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( labels.CheckProperties( 2, 1, DataType::Class_UInt ));
   DIP_THROW_IF( connectivity > 2, E::CONNECTIVITY_NOT_SUPPORTED );

   // Initialize freeman codes
   ChainCode::CodeTable codeTable = ChainCode::PrepareCodeTable( connectivity, labels.Strides() );

   // Create a map for the object IDs
   ObjectIdList objectIdList;
   dip::uint nObjects;
   if (objectIDs.empty()) {
      UnsignedArray allObjectIDs = GetObjectLabels( labels, Image(), "exclude" );
      for( dip::uint ii = 0; ii < allObjectIDs.size(); ++ii ) {
         objectIdList.emplace( allObjectIDs[ ii ], ObjectData{ ii, false } );
      }
      nObjects = allObjectIDs.size();
   } else {
      for( dip::uint ii = 0; ii < objectIDs.size(); ++ii ) {
         objectIdList.emplace( objectIDs[ ii ], ObjectData{ ii, false } );
      }
      nObjects = objectIDs.size();
   }

   // Get the chain code for each label
   ChainCodeArray ccArray;
   DIP_OVL_CALL_ASSIGN_UINT( ccArray,
                             dip__ChainCodes, ( labels, objectIdList, nObjects, connectivity, codeTable ),
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
   DIP_OVL_CALL_ASSIGN_UNSIGNED( cc,
                                 dip__OneChainCode, ( data, coord, dims, connectivity, codeTable ),
                                 labels.DataType() );
   return cc;
}

void ChainCode::Image( dip::Image& out ) const {
   dip::BoundingBox bb = BoundingBox();
   UnsignedArray size = bb.Size();
   out.ReForge( size, 1, DT_BIN );
   out = false; // set all pixels to false
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

ChainCode ChainCode::Offset() const {
   DIP_THROW_IF( !is8connected, "This method is only defined for 8-connected chain codes" );
   ChainCode out;
   out.objectID = objectID;
   out.is8connected = true;
   unsigned prev = codes.back();
   out.start = start + deltas8[ ( prev + ( codes.back().IsEven() ? 2u : 3u )) % 8u ];
   for( auto code : codes ) {
      unsigned n;
      if( code < prev ) {
         n = code + 8u - prev;
      } else {
         n = code - prev;
      }
      if( code.IsEven() ) {
         switch( n ) { // Note we fall through on purpose here
            case 4: // -4
            case 5: // -3
               out.Push( { code + 3, code.IsBorder() } );
            case 6: // -2
            case 7: // -1
               out.Push( { code + 1, code.IsBorder() } );
            case 0:
            case 1:
               out.Push( code );
               break;
            default:
               DIP_THROW_ASSERTION( "Not reachable" );
         }
      } else {
         switch( n ) { // Note we fall through on purpose here
            case 4: // -4
               out.Push( { code + 4, code.IsBorder() } );
            case 5: // -3
            case 6: // -2
               out.Push( { code + 2, code.IsBorder() } );
            case 7: // -1
            case 0:
               out.Push( code );
            case 1:
            case 2:
               // no points to add
               break;
            default:
               DIP_THROW_ASSERTION( "Not reachable" );
         }
      }
      prev = code;
   }
   return out;
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing chain code conversion to image and back") {
   dip::ChainCode cc;
   cc.codes = { 0, 0, 7, 6, 6, 5, 4, 4, 3, 2, 2, 1 }; // A chain code that is a little circle.
   cc.start = { 1, 0 }; // start so that the bounding box has a top-left corner at {0,0}.
   cc.is8connected = true;
   dip::ChainCode cc2;
   dip::Image img = cc.Image();
   cc2 = dip::GetSingleChainCode( img, { 1, 0 }, 2 );
   DOCTEST_CHECK( cc2.start.x == 1 );
   DOCTEST_CHECK( cc2.start.y == 0 );
   DOCTEST_CHECK( cc2.is8connected );
   DOCTEST_REQUIRE( cc2.codes.size() == cc.codes.size() );
   for( dip::uint ii = 0; ii < cc2.codes.size(); ++ii ) {
      DOCTEST_CHECK( cc.codes[ ii ] == cc2.codes[ ii ] );
   }
}

#include "diplib/pixel_table.h"
#include "diplib/morphology.h"

DOCTEST_TEST_CASE("[DIPlib] testing ChainCode::Offset") {
   dip::Image img = dip::PixelTable( "elliptic", { 29, 29 } ).AsImage(); // Currently the easiest way of generating an image...
   img = img.Pad( { 33, 33 } );
   dip::ChainCode cc1 = dip::GetSingleChainCode( img, { 16, 2 }, 2 );
   cc1 = cc1.Offset();
   img = dip::Dilation( img, { 3, "diamond" } );
   dip::ChainCode cc2 = dip::GetSingleChainCode( img, { 16, 1 }, 2 );
   DOCTEST_REQUIRE( cc1.codes.size() == cc2.codes.size());
   for( dip::uint ii = 0; ii < cc1.codes.size(); ++ii ) {
      DOCTEST_CHECK( cc1.codes[ ii ] == cc2.codes[ ii ] );
   }
}

#endif // DIP__ENABLE_DOCTEST
