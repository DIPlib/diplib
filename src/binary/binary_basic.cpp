/*
 * DIPlib 3.0
 * This file contains basic binary morphology functions: dilation, erosion, opening, closing.
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

#include "diplib/binary.h"
#include "diplib/library/stringparams.h"
#include "diplib/neighborlist.h"
#include "binary_support.h"

namespace dip
{

/// The BinaryPropagationFunc type is used to define what to do with pixels added
/// to the processing queue during dilation and erosion.
using BinaryPropagationFunc = std::function< void( uint8& pixel, uint8 dataMask ) >;

/// Worker function for both dilation and erosion, since they are very alike
void BinaryDilationErosion(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& s_edgeCondition,
   bool findObjectPixels,
   BinaryPropagationFunc const& propagationOperation
) {
   // Verify that the image is forged, scalar and binary
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( connectivity > (dip::sint)nDims, E::PARAMETER_OUT_OF_RANGE );

   // Edge condition: true means object, false means background
   bool outsideImageIsObject = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND );

   // Copy input plane to output plane. Operation takes place directly in the output plane.
   Image c_in = in; // temporary copy of image header, so we can strip out
   out.ReForge( in.Sizes(), 1, DT_BIN ); // reforging first in case `out` is the right size but a different data type
   out.Copy( c_in );

   // Negative connectivity means: alternate between two different connectivities

   // Create arrays with offsets to neighbours for even iterations
   dip::uint iterConnectivity0 = GetAbsBinaryConnectivity( nDims, connectivity, 0 );
   NeighborList neighborList0( { Metric::TypeCode::CONNECTED, iterConnectivity0 }, nDims );
   IntegerArray neighborOffsetsOut0 = neighborList0.ComputeOffsets( out.Strides() );

   // Create arrays with offsets to neighbours for odd iterations
   dip::uint iterConnectivity1 = GetAbsBinaryConnectivity( nDims, connectivity, 1 );
   NeighborList neighborList1( { Metric::TypeCode::CONNECTED, iterConnectivity1 }, nDims );
   IntegerArray neighborOffsetsOut1 = neighborList1.ComputeOffsets( out.Strides() );

   // Data mask: the pixel data is in the first 'plane'
   uint8 dataMask = 1;

   // Use border mask to mark pixels of the image border
   uint8 borderMask = uint8( 1 << 2 );
   ApplyBinaryBorderMask( out, borderMask );

   // The edge pixel queue
   dip::queue< dip::bin* > edgePixels;

   // Initialize the queue by finding all edge pixels of the type according to findObjectPixels
   FindBinaryEdgePixels( out, findObjectPixels, neighborList0, neighborOffsetsOut0, dataMask, borderMask, outsideImageIsObject, &edgePixels );

   // First iteration: simply process the queue
   if (iterations > 0) {
      for (dip::queue< dip::bin* >::const_iterator itEP = edgePixels.begin(); itEP != edgePixels.end(); ++itEP) {
         propagationOperation( static_cast<uint8&>(**itEP), dataMask );
      }
   }

   // Create a coordinates computer for bounds checking of border pixels
   const CoordinatesComputer coordsComputer = out.OffsetToCoordinatesComputer();

   // Second and further iterations
   for (dip::uint iDilIter = 1; iDilIter < iterations; ++iDilIter) {
      // Obtain neighbor list and offsets for this iteration
      NeighborList const& neighborList = iDilIter & 1 ? neighborList1 : neighborList0;
      IntegerArray const& neighborOffsetsOut = iDilIter & 1 ? neighborOffsetsOut1 : neighborOffsetsOut0;

      // Process all elements currently in the queue
      dip::sint count = edgePixels.size();

      while( --count >= 0 ) {
         // Get front pixel from the queue
         dip::bin* pPixel = edgePixels.front();
         uint8& pixelByte = static_cast<uint8&>(*pPixel);
         bool isBorderPixel = pixelByte & borderMask;

         // Propagate to all neighbours which are not yet processed
         dip::IntegerArray::const_iterator itNeighborOffset = neighborOffsetsOut.begin();
         for( NeighborList::Iterator itNeighbor = neighborList.begin(); itNeighbor != neighborList.end(); ++itNeighbor, ++itNeighborOffset ) {
            if( !isBorderPixel || itNeighbor.IsInImage( coordsComputer( pPixel - static_cast<dip::bin*>(out.Origin()) ), out.Sizes() ) ) { // IsInImage() is not evaluated for non-border pixels
               dip::bin* pNeighbor = pPixel + *itNeighborOffset;
               uint8& neighborByte = static_cast<uint8&>(*pNeighbor);
               bool neighborIsObject = neighborByte & dataMask;
               if( neighborIsObject == findObjectPixels ) {
                  // Propagate to the neighbor pixel
                  propagationOperation( neighborByte, dataMask );
                  // Add neighbor to the queue
                  edgePixels.push_back( pNeighbor );
               }
            }
         }

         // Remove the front pixel from the queue
         edgePixels.pop_front();
      }
   }

   // Clear the edge mask of the image border
   ClearBinaryBorderMask( out, borderMask );
}

void BinaryDilation(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& s_edgeCondition
) {
   bool findObjectPixels = false; // Dilation propagates to background pixels
   auto propagationFunc = []( uint8& pixel, uint8 dataMask ) { pixel |= dataMask; };   // Propagate by adding the data mask
   BinaryDilationErosion( in, out, connectivity, iterations, s_edgeCondition, findObjectPixels, propagationFunc );
}

void BinaryErosion(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& s_edgeCondition
) {
   bool findObjectPixels = true; // Erosion propagates to object pixels
   auto propagationFunc = []( uint8& pixel, uint8 dataMask ) { pixel &= ~dataMask; };  // Propagate by removing the data mask
   BinaryDilationErosion( in, out, connectivity, iterations, s_edgeCondition, findObjectPixels, propagationFunc );
}

void BinaryOpening
(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   if (edgeCondition == S::BACKGROUND || edgeCondition == S::OBJECT) {
      BinaryErosion( in, out, connectivity, iterations, edgeCondition );
      BinaryDilation( out, out, connectivity, iterations, edgeCondition );
   }
   else {
      // "Special handling"
      DIP_THROW_IF( edgeCondition != "special", E::INVALID_PARAMETER );
      BinaryErosion( in, out, connectivity, iterations, S::OBJECT );
      BinaryDilation( out, out, connectivity, iterations, S::BACKGROUND );
   }
}

void BinaryClosing
(
   Image const& in,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& edgeCondition
) {
   if (edgeCondition == S::BACKGROUND || edgeCondition == S::OBJECT) {
      BinaryDilation( in, out, connectivity, iterations, edgeCondition );
      BinaryErosion( out, out, connectivity, iterations, edgeCondition );
   }
   else {
      // "Special handling"
      DIP_THROW_IF( edgeCondition != "special", E::INVALID_PARAMETER );
      BinaryDilation( in, out, connectivity, iterations, S::BACKGROUND );
      BinaryErosion( out, out, connectivity, iterations, S::OBJECT );
   }
}

} // namespace dip
