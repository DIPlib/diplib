/*
 * (c)2017, Erik Schuitema.
 * (c)2017-2022, Cris Luengo.
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

#include <stack>

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/neighborlist.h"
#include "binary_support.h"

namespace dip {

namespace {

void BinaryPropagation_Fast (
      Image& out_img_c, // output image with seed input image copied in
      Image& mask_img,  // input, we can't write to it, but we're free to modify the image object
      dip::uint connectivity,
      bool outsideImageIsObject
) {
   // NOTE!!!
   //   Putting all three binary images used in this algorithm into the same image, using bit planes, as the older
   //   algorithm does, leads to much (x2) slower code on my M1 machine.

   // We need `mask_img` to have the same strides as `out_img_c`. Make a copy if needed.
   if( mask_img.Strides() != out_img_c.Strides() ) {
      Image tmp;
      tmp.SetStrides( out_img_c.Strides() );
      tmp.SetExternalInterface( out_img_c.ExternalInterface() ); // If there's an external interface for out_img_c, using it should give us the same strides here.
      tmp.ReForge( out_img_c );
      DIP_THROW_IF( tmp.Strides() != out_img_c.Strides(), "Couldn't allocate an intermediate image (copy of in) with the same strides as out" );
      tmp.Copy( mask_img );
      mask_img.swap( tmp );
   }

   // Prepare border image. This one must also have matching strides.
   Image border_img;
   border_img.SetStrides( out_img_c.Strides() );
   border_img.SetExternalInterface( out_img_c.ExternalInterface() );
   border_img.ReForge( out_img_c );
   DIP_THROW_IF( border_img.Strides() != out_img_c.Strides(), "Couldn't allocate an intermediate image (border) with the same strides as out" );
   border_img.Fill( 0 );
   SetBorder( border_img, { true }, { 1 } );

   // Reorder dimensions to improve iteration -- this has no effect on this algorithm, except to speed it up.
   Image out_img = out_img_c.QuickCopy(); // we don't want to change dimension order of the output image
   mask_img.StandardizeStrides();
   out_img.StandardizeStrides();
   border_img.StandardizeStrides(); // These three function calls do the same computations internally. It's not easy to rewrite things to avoid this.
   DIP_ASSERT( mask_img.Strides() == out_img.Strides() );
   DIP_ASSERT( mask_img.Strides() == border_img.Strides() );

   // Create array with offsets to neighbors
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, out_img.Dimensionality() );
   IntegerArray neighborOffsets = neighborList.ComputeOffsets( out_img.Strides() );
   dip::uint nNeigh = neighborList.Size();
   UnsignedArray const& imsz = out_img.Sizes();
   NeighborList backwardNeighbors = neighborList.SelectBackward();

   // Get pointers to image data
   bin* out = static_cast< bin* >( out_img.Origin() );
   bin* mask = static_cast< bin* >( mask_img.Origin() );
   bin* border = static_cast< bin* >( border_img.Origin() );

   // Step 1: Forward raster pass, propagate values forward (to the right and down)
   {
      IntegerArray backwardOffsets = backwardNeighbors.ComputeOffsets( out_img.Strides() );
      ImageIterator< bin > it( { out_img } );
      do {
         dip::sint offset = it.Offset();
         if( !*it && mask[ offset ] ) {
            if( border[ offset ] ) {
               if( outsideImageIsObject ) {
                  *it = true;
               } else {
                  for( dip::uint ii = 0; ii < backwardOffsets.size(); ++ii ) {
                     if( neighborList.IsInImage( ii, it.Coordinates(), imsz )) {
                        if( it.Pointer()[ backwardOffsets[ ii ]] ) {
                           *it = true;
                           break;
                        }
                     }
                  }
               }
            } else {
               for( auto n: backwardOffsets ) {
                  if( it.Pointer()[ n ] ) {
                     *it = true;
                     break;
                  }
               }
            }
         }
      } while( ++it );
   }

   // Step 2: Backward raster pass, propagate values backward (to the left and up), and enqueue pixels where we could
   //         propagate from in Step 3
   std::stack< dip::sint > Q;
   {
      Image out_img_mirrored = out_img.QuickCopy();
      out_img_mirrored.Mirror(); // a forward raster scan in a mirrored image is a backward raster scan in the original image
      IntegerArray backwardOffsets = backwardNeighbors.ComputeOffsets( out_img_mirrored.Strides() );
      ImageIterator< bin > it( { out_img_mirrored } );
      do {
         dip::sint offset = it.Pointer() - out; // The offset in the original image, so we can use it to index into `border` and `mask`
         if( !*it && mask[ offset ] ) {
            bin hasForegroundNeighbor = false;
            bin hasBackgroundNeighbor = false;
            if( border[ offset ] ) {
               if( outsideImageIsObject ) {
                  hasForegroundNeighbor = true;
               }
               for( dip::uint ii = 0; ii < backwardOffsets.size(); ++ii ) {
                  if( neighborList.IsInImage( ii, it.Coordinates(), imsz )) {
                     bin val = it.Pointer()[ backwardOffsets[ ii ]];
                     hasForegroundNeighbor |= val;
                     hasBackgroundNeighbor |= !val;
                  }
                  if( hasForegroundNeighbor && hasBackgroundNeighbor ) {
                     break;
                  }
               }
            } else {
               for( auto n: backwardOffsets ) {
                  bin val = it.Pointer()[ n ];
                  hasForegroundNeighbor |= val;
                  hasBackgroundNeighbor |= !val;
                  if( hasForegroundNeighbor && hasBackgroundNeighbor ) {
                     break;
                  }
               }
            }
            if( hasForegroundNeighbor ) {
               *it = true;
               // Enqueue only if pixels in the backward direction might be propagated into (the forward pixels we'll
               // be propagating into later in this raster scan).
               if( hasBackgroundNeighbor ) {
                  Q.push( offset );
               }
            }
         }
      } while( ++it );
   }

   // Step 3: Priority queue pass, propagate values in every direction from the pixels on the queue.
   auto coordinatesComputer = out_img.OffsetToCoordinatesComputer();
   BooleanArray skipar( nNeigh );
   while( !Q.empty() ) {
      dip::sint offset =  Q.top();
      Q.pop();
      // Compute coordinates if we're a border pixel
      UnsignedArray coords;
      bool onBorder = border[ offset ];
      if( onBorder ) {
         coords = coordinatesComputer( offset );
      }
      // Iterate over all neighbors
      for( dip::uint jj = 0; jj < nNeigh; ++jj ) {
         if( !onBorder || neighborList.IsInImage( jj, coords, imsz )) { // test IsInImage only for border pixels
            // Propagate this pixel's value to its unfinished neighbors
            dip::sint nOffset = offset + neighborOffsets[ jj ];
            if( !out[ nOffset ] && mask[ nOffset ] ) {
               out[ nOffset ] = true;
               // Add the updated neighbors to the heap
               Q.push( nOffset );
            }
         }
      }
   }

   // Last step: turn off pixels in `out_img` where `mask_img` is not set: If any input seed pixels were set where
   // the mask wasn't, we discard those pixels now. If we discard them at the start, then we don't get the same
   // behaviour as the older `BinaryPropagation_Iterative()`.
   out_img &= mask_img;
}


// Bit planes
constexpr uint8 dataBitmask{ 1u }; // Data mask: the pixel data is in the first 'plane'.
constexpr uint8 borderBitmask{ 1u << 2u };
constexpr uint8 maskBitmask{ 1u << 3u };
constexpr uint8 maskOrDataBitmask = dataBitmask | maskBitmask;


void BinaryPropagation_Iterative(
      Image& out,
      Image const& inMask,
      dip::sint connectivity,
      dip::uint iterations,
      bool outsideImageIsObject
) {
   // Use border mask to mark pixels of the image border
   ApplyBinaryBorderMask( out, borderBitmask );

   // Add mask plane to out image
   JointImageIterator< bin, bin > itInMaskOut( { inMask, out } );
   itInMaskOut.OptimizeAndFlatten();
   do {
      if( itInMaskOut.In() ) {
         SetBits( *reinterpret_cast< uint8* >( itInMaskOut.OutPointer() ), maskBitmask );
      }
   } while( ++itInMaskOut );

   // Create arrays with offsets to neighbours for even iterations
   dip::uint nDims = out.Dimensionality();
   dip::uint iterConnectivity0 = GetAbsBinaryConnectivity( nDims, connectivity, 0 );
   NeighborList neighborList0( { Metric::TypeCode::CONNECTED, iterConnectivity0 }, nDims );
   IntegerArray neighborOffsetsOut0 = neighborList0.ComputeOffsets( out.Strides() );

   // Create arrays with offsets to neighbours for odd iterations
   dip::uint iterConnectivity1 = GetAbsBinaryConnectivity( nDims, connectivity, 1 ); // won't throw
   NeighborList neighborList1( { Metric::TypeCode::CONNECTED, iterConnectivity1 }, nDims );
   IntegerArray neighborOffsetsOut1 = neighborList1.ComputeOffsets( out.Strides() );

   // Initialize the queue by finding all edge pixels of type 'background'
   BinaryFifoQueue edgePixels;
   bool findObjectPixels = false;
   FindBinaryEdgePixels( out, findObjectPixels, neighborList0, neighborOffsetsOut0, dataBitmask, borderBitmask, outsideImageIsObject, edgePixels );

   // First iteration: process all elements in the queue a first time
   dip::uint count = edgePixels.size();
   for( dip::uint jj = 0; jj < count; ++jj ) {
      bin* pPixel = edgePixels.front();
      uint8& pixelByte = *reinterpret_cast< uint8* >( pPixel );
      if(( pixelByte & maskOrDataBitmask ) == maskBitmask ) {
         SetBits( pixelByte, dataBitmask );
         edgePixels.push_back( pPixel );
      }
      // Remove the front pixel from the queue
      edgePixels.pop_front();
   }

   // Create a coordinates computer for bounds checking of border pixels
   CoordinatesComputer const coordsComputer = out.OffsetToCoordinatesComputer();

   // Second and further iterations. Loop stops if the queue is empty
   for( dip::uint ii = 1; ( ii < iterations ) && !edgePixels.empty(); ++ii ) {
      // Obtain neighbor list and offsets for this iteration
      NeighborList const& neighborList = ( ii & 1u ) == 1 ? neighborList1 : neighborList0;
      IntegerArray const& neighborOffsetsOut = ( ii & 1u ) == 1 ? neighborOffsetsOut1 : neighborOffsetsOut0;

      // Process all elements currently in the queue
      count = edgePixels.size();
      for( dip::uint jj = 0; jj < count; ++jj ) {
         // Get front pixel from the queue
         bin* pPixel = edgePixels.front();
         edgePixels.pop_front();
         uint8& pixelByte = *reinterpret_cast< uint8* >( pPixel );
         bool isBorderPixel = TestAnyBit( pixelByte, borderBitmask );

         // Propagate to all neighbours which are not yet processed
         IntegerArray::const_iterator itNeighborOffset = neighborOffsetsOut.begin();
         for( NeighborList::Iterator itNeighbor = neighborList.begin(); itNeighbor != neighborList.end(); ++itNeighbor, ++itNeighborOffset ) {
            if( !isBorderPixel || itNeighbor.IsInImage( coordsComputer( pPixel - static_cast< bin* >( out.Origin() )), out.Sizes() )) { // IsInImage() is not evaluated for non-border pixels
               bin* pNeighbor = pPixel + *itNeighborOffset;
               uint8& neighborByte = *reinterpret_cast< uint8* >( pNeighbor );
               // If the neighbor has the mask-bit (means: propagation allowed)
               // but not the seed-bit (means: not yet processed),
               // process this neighbor.
               if(( neighborByte & maskOrDataBitmask ) == maskBitmask ) {
                  // Propagate to the neighbor pixel
                  SetBits( neighborByte, dataBitmask );
                  // Add neighbor to the queue
                  edgePixels.push_back( pNeighbor );
               }
            }
         }
      }
   }

   // Final step: pixels have their data bit set iff they have both seed-bit and mask-bit
   // The result is stored in a way that resets all bits except bit 0
   // This means that the border mask is also removed
   ImageIterator< bin > itOut( out );
   itOut.OptimizeAndFlatten();
   do {
      *itOut = TestAllBits( static_cast< uint8 >( *itOut ), maskOrDataBitmask );
   } while( ++itOut );
}

} // namespace

void BinaryPropagation(
   Image const& c_inSeed,
   Image const& c_inMask,
   Image& out,
   dip::sint connectivity,
   dip::uint iterations,
   String const& s_edgeCondition
) {
   // Verify that the mask image is forged, scalar and binary
   DIP_THROW_IF( !c_inMask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_inMask.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !c_inMask.IsScalar(), E::IMAGE_NOT_SCALAR );

   // If the seed image is not raw, check that it is scalar and binary and of the same size as inMask
   if( c_inSeed.IsForged() ) {
      DIP_THROW_IF( !c_inSeed.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
      DIP_THROW_IF( !c_inSeed.IsScalar(), E::IMAGE_NOT_SCALAR );
      DIP_THROW_IF( c_inMask.Sizes() != c_inSeed.Sizes(), E::SIZES_DONT_MATCH );
   }

   // Check connectivity validity
   dip::uint nDims = c_inMask.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity > static_cast< dip::sint >( nDims ), E::ILLEGAL_CONNECTIVITY );

   // Edge condition: true means object, false means background
   bool outsideImageIsObject;
   DIP_STACK_TRACE_THIS( outsideImageIsObject = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND ));

   // Make out equal to inSeed
   Image inMask = c_inMask.QuickCopy(); // temporary copy of input image headers, so we can strip/reforge out
   Image inSeed = c_inSeed.QuickCopy();
   PixelSize pixelSize = c_inSeed.HasPixelSize() ? c_inSeed.PixelSize() : c_inMask.PixelSize();
   if( out.Aliases( inMask )) { // make sure we don't overwrite the mask image
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   DIP_STACK_TRACE_THIS( out.ReForge( inMask.Sizes(), 1, DT_BIN ));
   // Copy inSeed plane to output plane if it is non-empty, otherwise clear it
   // Operation takes place directly in the output plane.
   if( inSeed.IsForged() ) {
      out.Copy( inSeed );     // if &c_inSeed == &out, we get here too. Copy won't do anything.
   } else {
      out = 0; // No seed data means: initialize all samples with false
   }
   out.SetPixelSize( std::move( pixelSize ));

   if( iterations == 0 ) {
      // If zero iterations (propagate until stability) use the fast algorithm
      dip::uint conn = static_cast< dip::uint >( std::max( dip::sint( 0 ), connectivity ));
      DIP_STACK_TRACE_THIS( BinaryPropagation_Fast( out, inMask, conn, outsideImageIsObject ));

   } else {
      // Iterate the given number of steps
      DIP_STACK_TRACE_THIS( BinaryPropagation_Iterative( out, inMask, connectivity, iterations, outsideImageIsObject ));
   }
}

} // namespace dip
