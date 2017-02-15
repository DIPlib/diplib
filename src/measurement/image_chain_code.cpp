/*
 * DIPlib 3.0
 * This file contains definitions for functions that create chain codes from object outlines.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>
#include <map>

#include "diplib.h"
#include "diplib/chain_code.h"
#include "diplib/regions.h"
#include "diplib/overload.h"


namespace dip {


namespace {

struct FreemanCode {
   VertexInteger pos;
   dip::sint offset;
};

using FeemanCodeTable = std::array< FreemanCode, 8 >;

struct ObjectData { dip::uint index; bool done; };
using ObjectIdList = std::map< dip::uint, ObjectData >; // key is the objectID (label)

template< typename DIP_TPI >
static ChainCodeArray dip__ChainCode(
      Image const& labels,
      ObjectIdList& objectIDs,
      dip::uint nObjects, // potentially different from the largest index in objectIDs, if there were repeated elements in the original list.
      dip::uint connectivity,
      FeemanCodeTable const& freeman
) {
   DIP_ASSERT( labels.DataType() == DataType( DIP_TPI( 0 ) ) );
   DIP_TPI* data = static_cast< DIP_TPI* >( labels.Origin() );
   ChainCodeArray ccArray( nObjects );  // output array
   IntegerArray dims( 2 );
   dims[ 0 ] = labels.Size( 0 ) - 1;
   dims[ 1 ] = labels.Size( 1 ) - 1; // our local copy of `dims` now contains the largest coordinates
   IntegerArray const& strides = labels.Strides();

   // Find first pixel of requested label
   dip::uint label = 0;
   VertexInteger coord;
   for( coord.y = 0; coord.y < dims[ 1 ]; ++coord.y ) {
      dip::sint pos = coord.y * strides[ 1 ];
      for( coord.x = 0; coord.x < dims[ 0 ]; ++coord.x ) {
         bool process = false;
         dip::uint index = 0;
         dip::uint newlabel = data[ pos ];
         if( ( newlabel != 0 ) && ( newlabel != label ) ) {
            // Check whether newlabel is start of not processed object
            auto it = objectIDs.find( newlabel );
            if( ( it != objectIDs.end() ) && !it->second.done ) {
               label = newlabel;
               it->second.done = true;
               index = it->second.index;
               process = true;
            }
         }
         if( process ) {
            // Initialize the chaincode of the object
            ccArray[ index ].start = coord;
            ccArray[ index ].objectID = label;
            ccArray[ index ].is8connected = connectivity == 2;

            // Follow contour always as left as possible (i.e. ii = ii+2)
            VertexInteger c = coord;
            dip::sint offset = pos;
            int dir = 0;
            do {

               VertexInteger nc = c + freeman[ dir ].pos;
               dip::sint no = offset + freeman[ dir ].offset;
               if( ( nc.x >= 0 ) && ( nc.x <= dims[ 0 ] ) &&
                   ( nc.y >= 0 ) && ( nc.y <= dims[ 1 ] ) &&
                   ( ( data[ no ] ) == label ) ) {

                  // Add new chain
                  bool border = ( nc.x == 0 ) || ( nc.x == dims[ 0 ] ) ||
                                ( nc.y == 0 ) || ( nc.y == dims[ 1 ] );
                  ccArray[ index ].Push( { dir, border } );

                  // New position
                  c = nc;
                  offset = no;

                  // Start direction to search
                  if( connectivity == 1 ) {
                     dir = ( dir + 1 ) % 4;
                  } else {
                     dir = ( dir + 2 ) % 8;
                  }

               } else {

                  // New direction to search
                  if( dir == 0 ) {
                     dir = connectivity == 2 ? 7 : 3;
                  } else {
                     dir--;
                  }

               }

            } while( !( ( c == coord ) && ( dir == 0 ) ) );
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
   DIP_START_STACK_TRACE
      labels.CheckProperties( 2, 1, DataType::Class_UInt );
   DIP_END_STACK_TRACE
   DIP_THROW_IF( !(connectivity == 1 || connectivity == 2),
          "connectivity not supported");

   // Initialize freeman codes
   FeemanCodeTable freeman;
   dip::sint xS = labels.Stride( 0 );
   dip::sint yS = labels.Stride( 1 );
   if( connectivity == 1 ) {
      freeman[ 0 ].pos.x =  1; freeman[ 0 ].pos.y =  0; freeman[ 0 ].offset =  xS;
      freeman[ 1 ].pos.x =  0; freeman[ 1 ].pos.y = -1; freeman[ 1 ].offset =     - yS;
      freeman[ 2 ].pos.x = -1; freeman[ 2 ].pos.y =  0; freeman[ 2 ].offset = -xS;
      freeman[ 3 ].pos.x =  0; freeman[ 3 ].pos.y =  1; freeman[ 3 ].offset =       yS;
   } else {
      freeman[ 0 ].pos.x =  1; freeman[ 0 ].pos.y =  0; freeman[ 0 ].offset =  xS;
      freeman[ 1 ].pos.x =  1; freeman[ 1 ].pos.y = -1; freeman[ 1 ].offset =  xS - yS;
      freeman[ 2 ].pos.x =  0; freeman[ 2 ].pos.y = -1; freeman[ 2 ].offset =     - yS;
      freeman[ 3 ].pos.x = -1; freeman[ 3 ].pos.y = -1; freeman[ 3 ].offset = -xS - yS;
      freeman[ 4 ].pos.x = -1; freeman[ 4 ].pos.y =  0; freeman[ 4 ].offset = -xS;
      freeman[ 5 ].pos.x = -1; freeman[ 5 ].pos.y =  1; freeman[ 5 ].offset = -xS + yS;
      freeman[ 6 ].pos.x =  0; freeman[ 6 ].pos.y =  1; freeman[ 6 ].offset =       yS;
      freeman[ 7 ].pos.x =  1; freeman[ 7 ].pos.y =  1; freeman[ 7 ].offset =  xS + yS;
   }

   // Create a map for the object IDs
   ObjectIdList objectIdList;
   dip::uint nObjects;
   if (objectIDs.empty()) {
      UnsignedArray allObjectIDs = GetObjectLabels( labels, Image(), false );
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

   // Get the chaincode for each label
   ChainCodeArray ccArray;
   DIP_OVL_CALL_ASSIGN_UINT( ccArray,
                             dip__ChainCode,
                             ( labels, objectIdList, nObjects, connectivity, freeman ),
                             labels.DataType() );
   return ccArray;
}


} // namespace dip
