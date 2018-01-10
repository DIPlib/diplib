/*
 * DIPlib 3.0
 * This file contains infrastructure for changing pixels at the image border.
 *
 * (c)2017, Erik Schuitema, Cris Luengo
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

#ifndef BORDER_H_INCLUDED
#define BORDER_H_INCLUDED

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"

namespace dip {

namespace detail {

/// \brief Generic template to process the border/edges of an image.
///
/// All pixels within `borderWidth` of the image edge are processed through custom functions. These custom
/// functions can read and write from the input pixel.
///
/// `borderPixelFunction` and `nonBorderPixelFunction` are two functions with the following signature:
/// ```cpp
///     void Function( TPI* ptr, dip::sint tensorStride );
/// ```
/// The first one is applied to border pixels (if `ProcessBorder` is true), and the second one to non-border
/// pixels (if `ProcessNonBorder` is true). If either of the boolean template parameters is false, the corresponding
/// function is not called, and thus can be an empty function (for example the lambda `[](auto*,dip::sint){}`).
/// It is recommended that the called functions are lambdas, to allow stronger optimizations.
template< typename TPI, bool ProcessBorder, bool ProcessNonBorder, typename BorderFunc, typename InnerFunc >
void ProcessBorders(
      Image& out,
      BorderFunc borderPixelFunction,
      InnerFunc nonBorderPixelFunction,
      dip::uint borderWidth = 1
) {
   static_assert( ProcessBorder || ProcessNonBorder, "At least one of the two boolean template parameters must be set" );

   // Iterate over all image lines, in the optimal processing dimension
   dip::uint procDim = Framework::OptimalProcessingDim( out );
   dip::uint lineLength = out.Size( procDim );
   dip::sint stride = out.Stride( procDim );
   dip::sint tensorStride = out.TensorStride();

   if( 2 * borderWidth >= lineLength ) {
      // Everything is a border
      if( ProcessBorder ) {
         ImageIterator< TPI > it( out );
         it.OptimizeAndFlatten();
         do {
            borderPixelFunction( it.Pointer(), tensorStride );
         } while( ++it );
      }
      return;
   }

   dip::uint innerLength = lineLength - 2 * borderWidth;
   dip::sint innerOffset = static_cast< dip::sint >( borderWidth ) * stride;
   dip::sint lastOffset = static_cast< dip::sint >( innerLength ) * stride;
   ImageIterator< TPI > it( out, procDim );
   it.Optimize();
   UnsignedArray const& newSizes = it.Sizes();
   procDim = it.ProcessingDimension();
   dip::uint nDim = newSizes.size();
   stride = it.ProcessingDimensionStride(); // could have flipped!
   do {
      // Is this image line inside the image border?
      bool inBorder = false;
      UnsignedArray const& coord = it.Coordinates();
      for( dip::uint ii = 0; ii < nDim; ++ii ) {
         if(( ii != procDim ) &&
            (( coord[ ii ] < borderWidth ) || ( coord[ ii ] >= newSizes[ ii ] - borderWidth ))) {
            inBorder = true;
            break;
         }
      }
      TPI* ptr = it.Pointer();
      if( inBorder ) {
         // Yes, it is: process all pixels on the line
         if( ProcessBorder ) {
            for( dip::uint ii = 0; ii < lineLength; ++ii, ptr += stride ) {
               borderPixelFunction( ptr, tensorStride );
            }
         }
      } else {
         // No, it isn't: process only the first `borderWidth` and last `borderWidth` pixels
         if( ProcessBorder ) {
            for( dip::uint ii = 0; ii < borderWidth; ++ii, ptr += stride ) {
               borderPixelFunction( ptr, tensorStride );
            }
         } else {
            ptr += innerOffset;
         }
         // Optionally process non-border pixels
         if( ProcessNonBorder ) {
            for( dip::uint ii = 0; ii < innerLength; ++ii, ptr += stride ) {
               nonBorderPixelFunction( ptr, tensorStride );
            }
         } else {
            ptr += lastOffset;
         }
         // Process last `borderWidth` pixels
         if( ProcessBorder ) {
            for( dip::uint ii = 0; ii < borderWidth; ++ii, ptr += stride ) {
               borderPixelFunction( ptr, tensorStride );
            }
         }
      }
   } while( ++it );
}

/// \brief Convenience interface to `dip::ProcessBorders` when only border pixels must be processed.
template< typename TPI, typename BorderFunc >
void ProcessBorders(
      Image& out,
      BorderFunc borderPixelFunction,
      dip::uint borderWidth = 1
) {
   ProcessBorders< TPI, true, false >( out, borderPixelFunction, []( TPI*, dip::sint ){}, borderWidth );
}

} // namespace detail

} // namespace dip

#endif
