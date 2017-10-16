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

#ifndef BINARY_SUPPORT_H_INCLUDED
#define BINARY_SUPPORT_H_INCLUDED

#include "diplib/border.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/library/types.h"
#include "diplib/neighborlist.h"
#include <queue>

namespace dip
{

/// A queue (FIFO). Since std::queue doesn't support iterators,
/// std::deque is used, accepting a wider interface than strictly necessary.
/// Note that (std::queue with underlying) std::deque container might perform poorly.
/// If so, this type can be replaced by a more efficient implementation of a queue/circular buffer.
template< typename T >
using queue = std::deque< T >;

/// Masks all border pixels of a binary image with the given mask
/// and resets the mask for non-border pixels.
void ApplyBinaryBorderMask( Image& out, uint8 borderMask );

/// Clears the binary border mask
void ClearBinaryBorderMask( Image& out, uint8 borderMask );

/// Checks for a binary pixel if any of the neighbors from a NeighborList has a different value
/// Bounds checking is optional. It is most efficient to only enable it for pixels on the border of an image.
bool IsBinaryEdgePixel( Image const& in, dip::sint pixelOffset, NeighborList const& neighborList, IntegerArray const& neighborOffsets, uint8 dataMask, bool checkBounds, CoordinatesComputer const* coordsComputer );

/// Collect binary pixels that have at least one neighbor with a different value
/// If findObjectPixels is true, object pixels are collected that have at least one neighboring background pixel.
/// If findObjectPixels is false, background pixels are collected that have at least one neighboring object pixel.
/// If treatOutsideImageAsObject is true, the area outside the image borders is treated as object; otherwise it is treated as background
void FindBinaryEdgePixels( const Image& in, bool findObjectPixels, NeighborList const& neighborList, IntegerArray const& neighborOffsets, uint8 dataMask, uint8 borderMask, bool treatOutsideImageAsObject, queue< dip::bin* >* edgePixels );

/// This function creates support for alternating connectivities when performing multiple binary operations
/// Returns the absolute connectivity based on a signed connectivity number and an iteration number.
/// Alternation is only supported for dimensionality 2 and 3. For other dimensionalities, connectivity is returned unaltered.
/// The function does not check whether connectivity <= dimensionality.
dip::uint GetAbsBinaryConnectivity( dip::uint dimensionality, dip::sint connectivity, dip::uint iteration );

}; // namespace dip

#endif