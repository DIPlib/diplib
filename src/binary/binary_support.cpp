/*
 * DIPlib 3.0
 * This file contains binary support functions.
 *
 * (c)2017, Erik Schuitema.
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

#include "binary_support.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/border.h"

namespace dip {

void ApplyBinaryBorderMask( Image& out, uint8 const borderMask ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   detail::ProcessBorders< bin, true, true >(
         out,
         [ borderMask ]( bin* ptr, dip::sint ) { // Set border mask bits within the border
            SetBits( static_cast< uint8& >( *ptr ), borderMask );
         },
         [ borderMask ]( bin* ptr, dip::sint ) { // Reset border mask bits elsewhere
            ResetBits( static_cast< uint8& >( *ptr ), borderMask );
         } );
}

void ClearBinaryBorderMask( Image& out, uint8 const borderMask ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   detail::ProcessBorders< bin >(
         out,
         [ borderMask ]( auto* ptr, dip::sint ) { // Reset border mask bits within the border
            ResetBits( static_cast< uint8& >( *ptr ), borderMask );
         } );
}

bool IsBinaryEdgePixel(
      Image const& in,
      dip::sint pixelOffset,
      NeighborList const& neighborList,
      IntegerArray const& neighborOffsets,
      uint8 dataMask,
      bool checkBounds,
      CoordinatesComputer const& coordsComputer
) {
   // Do bounds checking if requested
   dip::UnsignedArray pixelCoords;
   if( checkBounds ) {
      pixelCoords = coordsComputer( pixelOffset );
   }

   uint8* pPixel = static_cast< uint8* >( in.Origin() ) + pixelOffset; // It's a binary image, but we read pixels as uint8.
   // Check for all valid neighbors if any of them has a differing value. If so, break and return true.
   dip::IntegerArray::const_iterator itNeighborOffset = neighborOffsets.begin();
   for( NeighborList::Iterator itNeighbor = neighborList.begin(); itNeighbor != neighborList.end(); ++itNeighbor, ++itNeighborOffset ) {
      if( !checkBounds || itNeighbor.IsInImage( pixelCoords, in.Sizes() )) {
         const uint8 pixelByte = *pPixel;
         const uint8 neighborByte = *( pPixel + *itNeighborOffset ); // Add the neighborOffset to the address (ptr) of pixel, and dereference to get its value
         bool pixelIsObject = TestAnyBit( pixelByte, dataMask );
         bool neighborIsObject = TestAnyBit( neighborByte, dataMask );
         // If the pixel value is different from the neighbor value, it is an edge pixel
         if( pixelIsObject != neighborIsObject ) {
            return true;
         }
      }
   }

   return false;
}

void FindBinaryEdgePixels(
      const Image& in,
      bool findObjectPixels,
      NeighborList const& neighborList,
      IntegerArray const& neighborOffsets,
      uint8 dataMask,
      uint8 borderMask,
      bool treatOutsideImageAsObject,
      BinaryFifoQueue& edgePixels
) {
   // Create a coordinates computer for bounds checking of border pixels
   const CoordinatesComputer coordsComputer = in.OffsetToCoordinatesComputer();

   // Iterate over all pixels: detect edge pixels and add them to the queue
   ImageIterator< bin > itImage( in );
   itImage.OptimizeAndFlatten(); // we get coordinates from the offset, this is not affected by the flattening.
   do {
      uint8& pixelByte = static_cast< uint8& >( *itImage );
      bool isObjectPixel = TestAnyBit( pixelByte, dataMask );   // Does pixel have non-zero data value, i.e., is it part of the object and not the background?
      // Check if the pixel is of the correct type: object or background
      if( isObjectPixel == findObjectPixels ) {
         // Check if the pixel is a border edge pixel due to the edge condition (done outside IsBinaryEdgePixel() to avoid overhead)
         bool isBorderPixel = TestAnyBit( pixelByte, borderMask ); // Is pixel part of the image border?
         bool isBorderEdgePixelForEdgeCondition = isBorderPixel && ( isObjectPixel != treatOutsideImageAsObject );
         // Check if the pixel is an edge pixel due to its neighbors
         if( isBorderEdgePixelForEdgeCondition ||
             IsBinaryEdgePixel( in, itImage.Offset(), neighborList, neighborOffsets, dataMask, isBorderPixel, coordsComputer )) {
            // Add the edge pixel to the queue
            edgePixels.push_back( itImage.Pointer() );
         }
      }
   } while( ++itImage );
}

dip::uint GetAbsBinaryConnectivity( dip::uint dimensionality, dip::sint connectivity, dip::uint iteration ) {
   // No check if connectivity <= dimensionality.
   // It is done automatically when creating a NeighborList.

   if( dimensionality == 2 ) {
      if( connectivity == -1 ) {
         return ( iteration % 2 == 0 ) ? 1 : 2;
      } else if( connectivity == -2 ) {
         return ( iteration % 2 == 0 ) ? 2 : 1;
      }
   } else if( dimensionality == 3 ) {
      if( connectivity == -1 ) {
         return ( iteration % 2 == 0 ) ? 1 : 3;
      } else if( connectivity == -2 || connectivity == -3 ) {
         return ( iteration % 2 == 0 ) ? 3 : 1;
      }
   }

   // All other dimensions and cases: just return connectivity,
   // but throw for negative connectivities since alternation is not supported.
   DIP_THROW_IF( connectivity < 0, "Connectivity can only be negative for dimensionality 2 and 3" );
   return static_cast< dip::uint >( connectivity );
}

}  // namespace dip
