/*
 * DIPlib 3.0
 * This file contains definitions for functions that create chain codes from object outlines.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/chain_code.h"
#include "diplib/regions.h"
#include "diplib/overload.h"


namespace dip {


struct FreemanCode {
   VertexInteger pos;
   dip::sint offset;
};

using FeemanCodeTable = std::array< FreemanCode, 8 >;

template< typename DIP_TPI >
static ChainCodeArray dip__ChainCode(
      Image const& labels,
      UnsignedArray const& objectIDs,
      dip::uint connectivity,
      FeemanCodeTable const& freeman
) {
   DIP_ASSERT( labels.DataType() == DataType( DIP_TPI( 0 )));
   DIP_TPI *data = static_cast< DIP_TPI* >( labels.Origin() );
   BooleanArray done( objectIDs.size(), false ); // mark labels as done
   ChainCodeArray ccArray( objectIDs.size() );  // output array
   UnsignedArray dims = labels.Sizes();
   --dims[ 0 ]; --dims[ 1 ]; // our local copy of `dims` now contains the largest coordinates
   IntegerArray const& strides = labels.Strides();

   // Find first pixel of requested label
   dip::uint label = 0;
   VertexInteger coord;
   for( coord.y = 0; coord.y < dims[ 1 ]; ++coord.y ) {
      dip::sint pos = coord.y * strides[ 1 ];
      for( coord.x = 0; coord.x < dims[ 0 ]; ++coord.x ) {
         bool process = false;
         dip::uint index = 0;
         if( data[ pos ] != label || coord.x == 0 ) {
            // Check whether data[pos] is start of not processed object
            for( index = 0; index < objectIDs.size(); ++index ) {
               if( ( data[ pos ] ) == objectIDs[ index ] ) {
                  if( !done[ index ] ) {
                     done[ index ] = true;
                     label = objectIDs[ index ];
                     process = true;
                  }
                  break;
               }
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
               if(( nc.x >= 0 ) && ( nc.x <= dims[ 0 ] ) &&
                  ( nc.y >= 0 ) && ( nc.y <= dims[ 1 ] ) &&
                  (( data[ no ] ) == label )) {

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

            } while( !(( c == coord ) && ( dir == 0 )));
         }
         pos += strides[ 0 ];
      }
   }
   return ccArray;
}


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

   // Get the chaincode for each label
   ChainCodeArray ccArray;
   DIP_OVL_CALL_ASSIGN_UNSIGNED( ccArray, dip__ChainCode, (
               labels,
               objectIDs.empty() ? GetObjectLabels( labels, Image(), false ) : objectIDs,
               connectivity,
               freeman
         ), labels.DataType() );
   return ccArray;
}


} // namespace dip
