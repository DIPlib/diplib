/*
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

#ifndef BINARY_SUPPORT_H
#define BINARY_SUPPORT_H

#include <deque>

#include "diplib.h"
#include "diplib/neighborlist.h"

namespace dip {

// A queue (FIFO). Since std::queue doesn't support iterators,
// std::deque is used, accepting a wider interface than strictly necessary.
// Note that (std::queue with underlying) std::deque container might perform poorly.
// If so, this type can be replaced by a more efficient implementation of a queue/circular buffer.
using BinaryFifoQueue = std::deque< bin* >;

// Set the bit or bits specified in `mask`.
inline void SetBits( uint8& value, uint8 const mask ) {
   value |= mask;
}

// Reset the bit or bits specified in `mask`. It'll help if `mask` is a `constexpr`.
inline void ResetBits( uint8& value, uint8 const mask ) {
   value &= uint8( ~mask );
}

// True if any of the bits specified in `mask` are set.
inline bool TestAnyBit( uint8 const value, uint8 const mask ) {
   return ( value & mask ) != 0;
}

// True if all of the bits specified in `mask` are set.
inline bool TestAllBits( uint8 const value, uint8 const mask ) {
   return ( value & mask ) == mask;
}

// Masks all border pixels of a binary image with the given mask
// and resets the mask for non-border pixels.
DIP_NO_EXPORT void ApplyBinaryBorderMask( Image& out, uint8 borderMask );

// Clears the binary border mask
DIP_NO_EXPORT void ClearBinaryBorderMask( Image& out, uint8 borderMask );

// Checks for a binary pixel if any of the neighbors from a NeighborList has a different value
// Bounds checking is optional. It is most efficient to only enable it for pixels on the border of an image.
DIP_NO_EXPORT bool IsBinaryEdgePixel(
      Image const& in,
      dip::sint pixelOffset,
      NeighborList const& neighborList,
      IntegerArray const& neighborOffsets,
      uint8 dataMask,
      bool checkBounds,
      CoordinatesComputer const& coordsComputer
);

// Collect binary pixels that have at least one neighbor with a different value
// If findObjectPixels is true, object pixels are collected that have at least one neighboring background pixel.
// If findObjectPixels is false, background pixels are collected that have at least one neighboring object pixel.
// If treatOutsideImageAsObject is true, the area outside the image borders is treated as object; otherwise it is treated as background
DIP_NO_EXPORT void FindBinaryEdgePixels(
      Image const& in,
      bool findObjectPixels,
      NeighborList const& neighborList,
      IntegerArray const& neighborOffsets,
      uint8 dataMask,
      uint8 borderMask,
      bool treatOutsideImageAsObject, BinaryFifoQueue& edgePixels
);

// This function creates support for alternating connectivities when performing multiple binary operations
// Returns the absolute connectivity based on a signed connectivity number and an iteration number.
// Alternation is only supported for dimensionality 2 and 3. For other dimensionalities, connectivity is returned unaltered.
// The function does not check whether connectivity <= dimensionality.
DIP_NO_EXPORT dip::uint GetAbsBinaryConnectivity( dip::uint dimensionality, dip::sint connectivity, dip::uint iteration );

} // namespace dip

#endif // BINARY_SUPPORT_H
