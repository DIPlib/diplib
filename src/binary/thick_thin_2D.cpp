/*
 * DIPlib 3.0
 * This file contains binary propagation.
 *
 * (c)2017, Cris Luengo.
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

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/neighborlist.h"
#include "diplib/iterators.h"
#include "diplib/border.h"
#include "binary_support.h"

namespace dip {

namespace {
// Copied from skeleton.cpp (luthil[0]):
constexpr uint8 luthil[256] = {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0,
      0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1,
      0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1,
      0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0,
      0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1,
      0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1
};

bool CanReset(
      uint8 const* pixel,
      dip::sint strideX,
      dip::sint strideY,
      uint8 bitplane
) {
   dip::uint dirc = 0;
   pixel += strideX;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 1; }
   pixel -= strideX * 2;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 16; }
   pixel -= strideY;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 8; }
   pixel += strideX;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 4; }
   pixel += strideX;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 2; }
   pixel += strideY * 2;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 128; }
   pixel -= strideX;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 64; }
   pixel -= strideX;
   if( TestAnyBit( *pixel, bitplane )) { dirc |= 32; }
   return luthil[ dirc ] == 0;
}
bool CanSet(
      uint8 const* pixel,
      dip::sint strideX,
      dip::sint strideY,
      uint8 bitplane
) {
   dip::uint dirc = 0;
   pixel += strideX;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 1; }
   pixel -= strideX * 2;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 16; }
   pixel -= strideY;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 8; }
   pixel += strideX;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 4; }
   pixel += strideX;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 2; }
   pixel += strideY * 2;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 128; }
   pixel -= strideX;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 64; }
   pixel -= strideX;
   if( !TestAnyBit( *pixel, bitplane )) { dirc |= 32; }
   return luthil[ dirc ] == 0;
}

void SetBorders( Image& out, uint8 const bitmask ) {
   DIP_ASSERT( out.IsForged() );
   detail::ProcessBorders< bin >( out,
         [ bitmask ]( bin* ptr, dip::sint ) {
            SetBits( static_cast< uint8& >( *ptr ), bitmask );
         } );
}

void ResetBorders( Image& out, uint8 const bitmask ) {
   DIP_ASSERT( out.IsForged() );
   detail::ProcessBorders< bin >( out,
         [ bitmask ]( bin* ptr, dip::sint ) {
            ResetBits( static_cast< uint8& >( *ptr ), bitmask );
         } );
}

using Uint8FifoQueue = std::deque< uint8* >;

Uint8FifoQueue EnqueueEdges2D(
      const Image& in,
      bool findObjectPixels,
      uint8 dataBitmask,
      uint8 maskBitmask
) {
   ImageIterator< bin > itImage( in );
   dip::sint strideX = in.Stride( 0 );
   dip::sint strideY = in.Stride( 1 );
   Uint8FifoQueue edgePixels;
   uint8 expectedValue = maskBitmask;
   if( findObjectPixels ) {
      SetBits( expectedValue, dataBitmask );
   }
   do {
      uint8* ptr = reinterpret_cast< uint8* >( itImage.Pointer() );
      if( *ptr == expectedValue ) {
         if( TestAnyBit( *( ptr - strideY ), dataBitmask ) != findObjectPixels ) { goto yes; }
         if( TestAnyBit( *( ptr - strideX ), dataBitmask ) != findObjectPixels ) { goto yes; }
         if( TestAnyBit( *( ptr + strideX ), dataBitmask ) != findObjectPixels ) { goto yes; }
         if( TestAnyBit( *( ptr + strideY ), dataBitmask ) != findObjectPixels ) { goto yes; }
         continue;
         // Add the edge pixel to the queue
         yes:
         edgePixels.push_back( ptr );
      }
   } while( ++itImage );
   return edgePixels;
}

void ConditionalThickeningThinning2D(
   Image const& c_in,
   Image const& c_mask,
   Image& out,
   dip::uint iterations,
   String const& s_endPixelCondition,
   String const& s_edgeCondition,
   bool thicken                     // false for thinning
) {
   DIP_THROW_IF( !c_in.IsForged() || !c_mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.DataType().IsBinary() || !c_mask.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( !c_in.IsScalar() || !c_mask.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( c_in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( c_mask.Sizes() != c_in.Sizes(), E::SIZES_DONT_MATCH );

   if( iterations == 0 ) {
      iterations = std::numeric_limits< dip::uint >::max();
   }
   bool endPixelCondition = BooleanFromString( s_endPixelCondition, S::KEEP, S::LOSE );
   bool edgeCondition = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND );

   // Make out equal to in
   Image mask = c_mask; // temporary copy of input image headers, so we can strip/reforge out
   Image in = c_in;
   if( out.Aliases( mask )) { // make sure we don't overwrite the mask image
      out.Strip();
   }
   out.ReForge( mask.Sizes(), 1, DT_BIN );
   out.Copy( in );     // if &c_in == &out, we get here too. Copy won't do anything.
   if( in.HasPixelSize() ) {
      out.SetPixelSize( in.PixelSize() );
   } else {
      out.SetPixelSize( mask.PixelSize() );
   }

   // Bit planes we'll use
   constexpr uint8 dataBitmask = 1;
   constexpr uint8 maskBitmask = 2;
   constexpr uint8 testedBitmask = 4; // used to mark pixels that cannot be deleted
   if( edgeCondition ) {
      SetBorders( out, dataBitmask ); // If boundary condition is true, set image border to true
   } else {
      ResetBorders( out, dataBitmask );
   }
   ResetBorders( out, maskBitmask ); // set mask border to false so we don't propagate into it

   uint8 expectedValue = maskBitmask;
   if( !thicken ) {
      SetBits( expectedValue, dataBitmask );
   }

   // Add mask plane to out image
   JointImageIterator< bin, bin > it( { mask, out } );
   do {
      if ( it.Sample< 0 >() ) {
         SetBits( static_cast< uint8& >( it.Sample< 1 >()), maskBitmask );
      }
   } while( ++it );

   // Initialize the queue by finding all edge pixels of type 'background'
   Uint8FifoQueue edgePixels = EnqueueEdges2D( out, !thicken, dataBitmask, maskBitmask );

   // Iterate. Loop also stops if the queue is empty.
   dip::sint strideX = out.Stride( 0 );
   dip::sint strideY = out.Stride( 1 );
   for( dip::uint ii = 0; ( ii < iterations ) && !edgePixels.empty(); ++ii ) {
      dip::uint count = edgePixels.size();
      for( dip::uint jj = 0; jj < count; ++jj ) {
         // Get front pixel from the queue
         uint8* ptr = edgePixels.front();
         edgePixels.pop_front();
         if( thicken ? CanSet( ptr, strideX, strideY, dataBitmask )
                     : CanReset( ptr, strideX, strideY, dataBitmask )) {
            thicken ? SetBits( *ptr, dataBitmask )
                    : ResetBits( *ptr, dataBitmask );
            // Enqueue neighbors that have mask bit set, and other bits not set
            uint8* neighbor = ptr - strideY;
            if( *neighbor == expectedValue ) {
               edgePixels.push_back( neighbor );
            }
            neighbor = ptr - strideX;
            if( *neighbor == expectedValue ) {
               edgePixels.push_back( neighbor );
            }
            neighbor = ptr + strideX;
            if( *neighbor == expectedValue ) {
               edgePixels.push_back( neighbor );
            }
            neighbor = ptr + strideY;
            if( *neighbor == expectedValue ) {
               edgePixels.push_back( neighbor );
            }
         } else {
            SetBits( *ptr, testedBitmask ); // don't test this one again
         }
      }
   }

   // Delete end pixels
   if( !endPixelCondition ) {
      // TODO
   }

   // Keep only the data bit
   ImageIterator< dip::bin > itOut( out );
   do {
      *itOut = TestAnyBit( static_cast< uint8& >( *itOut ), dataBitmask );
   } while( ++itOut );
}

} // namespace

void ConditionalThickening2D(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint iterations,
      String const& endPixelCondition,
      String const& edgeCondition
) {
   DIP_STACK_TRACE_THIS( ConditionalThickeningThinning2D( in, mask, out, iterations, endPixelCondition, edgeCondition, true ));
}

void ConditionalThinning2D(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint iterations,
      String const& endPixelCondition,
      String const& edgeCondition
) {
   DIP_STACK_TRACE_THIS( ConditionalThickeningThinning2D( in, mask, out, iterations, endPixelCondition, edgeCondition, false ));
}

} // namespace dip
