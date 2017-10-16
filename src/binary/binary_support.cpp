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
#include "diplib/library/error.h"

namespace dip {

namespace {
/// Masks the first sample of each border pixel with the given mask
/// and resets the mask for each (first sample of each) non-border pixel.
class BinaryBorderMasker : public BorderProcessor< dip::bin >
{
public:
   BinaryBorderMasker( Image& out, uint8 borderMask, dip::uint borderWidth = 1 ) : BorderProcessor< dip::bin >( out, borderWidth ), borderMask_( borderMask ), invBorderMask_( ~borderMask ) {}

protected:
   uint8 borderMask_;
   uint8 invBorderMask_;

   // Bitwise-OR the border mask onto the first sample of the pixel
   virtual void ProcessBorderPixel( dip::bin* pPixel ) {
      static_cast<uint8&>(*pPixel) |= borderMask_;
   }

   // Bitwise-AND the inverted border mask onto the first sample of the pixel
   virtual void ProcessNonBorderPixel( dip::bin* pPixel ) {
      static_cast<uint8&>(*pPixel) &= invBorderMask_;
   }

};

class BinaryBorderMaskClearer : public BinaryBorderMasker
{
public:
   BinaryBorderMaskClearer( Image& out, uint8 borderMask, dip::uint borderWidth = 1 ) : BinaryBorderMasker( out, borderMask, borderWidth ) {}
protected:
   /// Bitwise-AND the inverted border mask onto the first sample of the pixel to clear the border mask
   virtual void ProcessBorderPixel( dip::bin* pPixel ) {
      static_cast<uint8&>(*pPixel) &= invBorderMask_;
   }

   /// No operation on non-border pixels
   virtual void ProcessNonBorderPixel( dip::bin* pPixel ) {}
};
}

void ApplyBinaryBorderMask( Image& out, uint8 borderMask ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );

   BinaryBorderMasker( out, borderMask, 1 ).Process();
}

void ClearBinaryBorderMask( Image& out, uint8 borderMask ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );

   BinaryBorderMaskClearer( out, borderMask, 1 ).Process();
}

bool IsBinaryEdgePixel( Image const& in, dip::sint pixelOffset, NeighborList const& neighborList, IntegerArray const& neighborOffsets, uint8 dataMask, bool checkBounds, CoordinatesComputer const* coordsComputer ) {
   // Do bounds checking if requested
   dip::UnsignedArray pixelCoords;
   if( checkBounds ) {
      DIP_ASSERT( coordsComputer != NULL );   // We need a valid CoordinatesComputer for bounds checking
      pixelCoords = (*coordsComputer)(pixelOffset);
   }

   bool isEdgePixel = false;  // Not an edge pixel until proven otherwise
   dip::bin* pPixel = static_cast<dip::bin*>(in.Pointer( pixelOffset ));
   // Check for all valid neighbors if any of them has a differing value. If so, break and return true.
   dip::IntegerArray::const_iterator itNeighborOffset = neighborOffsets.begin();
   for( NeighborList::Iterator itNeighbor = neighborList.begin(); itNeighbor != neighborList.end(); ++itNeighbor, ++itNeighborOffset ) {
      if( !checkBounds || itNeighbor.IsInImage( pixelCoords, in.Sizes() ) ) {
         const uint8 pixelByte = static_cast<uint8>(*pPixel);
         const uint8 neighborByte = static_cast<uint8>(*(pPixel + *itNeighborOffset));   // Add the neighborOffset to the address (ptr) of pixel, and dereference to get its value
         bool pixelIsObject = pixelByte & dataMask;
         bool neighborIsObject = neighborByte & dataMask;
         // If the pixel value is different from the neighbor value, it is an edge pixel
         if( pixelIsObject != neighborIsObject ) {
            isEdgePixel = true;
            break;   // Once true, don't check other neighbors
         }
      }
   }

   return isEdgePixel;
}

void FindBinaryEdgePixels( const Image& in, bool findObjectPixels, NeighborList const& neighborList, IntegerArray const& neighborOffsets, uint8 dataMask, uint8 borderMask, bool treatOutsideImageAsObject, queue< dip::bin* >* edgePixels ) {
   // Create a coordinates computer for bounds checking of border pixels
   const CoordinatesComputer coordsComputer = in.OffsetToCoordinatesComputer();

   // Iterate over all pixels: detect edge pixels and add them to the queue
   ImageIterator< dip::bin > itImage( in );
   do {
      uint8& pixelByte = static_cast<uint8&>(*itImage);
      bool isBorderPixel = pixelByte & borderMask; // Is pixel part of the image border?
      bool isObjectPixel = pixelByte & dataMask;   // Does pixel have non-zero data value, i.e., is it part of the object and not the background?
      // Check if the pixel is of the correct type: object or background
      if( (findObjectPixels && isObjectPixel) || (!findObjectPixels && !isObjectPixel) ) {
         // Check if the pixel is a border edge pixel due to the edge condition (done outside IsBinaryEdgePixel() to avoid overhead)
         bool isBorderEdgePixelForEdgeCondition = isBorderPixel && (isObjectPixel != treatOutsideImageAsObject);
         // Check if the pixel is an edge pixel due to its neighbors
         if( isBorderEdgePixelForEdgeCondition || IsBinaryEdgePixel( in, itImage.Offset(), neighborList, neighborOffsets, dataMask, isBorderPixel, &coordsComputer ) ) {
            // Add the edge pixel to the queue
            edgePixels->push_back( &*itImage );
         }
      }
   } while( ++itImage );
}

dip::uint GetAbsBinaryConnectivity( dip::uint dimensionality, dip::sint connectivity, dip::uint iteration ) {
   // No check if connectivity <= dimensionality.
   // It is done automatically when creating a NeighborList.

   // Alternation for dim 2
   if( dimensionality == 2 ) {
      if( connectivity == -1 ) {
         if( iteration % 2 == 0 )
            return 1;
         else
            return 2;
      }
      else if( connectivity == -2 ) {
         if( iteration % 2 == 0 )
            return 2;
         else
            return 1;
      }
   }
   // Alternation for dim 3
   else if( dimensionality == 3 ) {
      if( connectivity == -1 ) {
         if( iteration % 2 == 0 )
            return 1;
         else
            return 3;
      }
      else if( connectivity == -2 || connectivity == -3 ) {
         if( iteration % 2 == 0 )
            return 3;
         else
            return 1;
      }
   }

   // All other dimensions and cases: just return connectivity,
   // but throw for negative connectivities since alternation is not supported.
   DIP_THROW_IF( connectivity < 0, "Connectivity can only be negative for dimensionality 2 and 3" );
   return static_cast<dip::uint>(connectivity);
}

}  // namespace dip
