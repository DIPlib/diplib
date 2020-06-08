/*
 * DIPlib 3.0
 * This file contains support functions for draw_discrete.cpp and draw_bandlimited.cpp.
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

#ifndef DIP_DRAW_SUPPORT_H
#define DIP_DRAW_SUPPORT_H

#include "diplib.h"

namespace dip {

namespace {

// Copy `value` into an array with the right number of elements, and of the right data type
template< typename TPI >
void CopyPixelToVector( Image::Pixel const& in, std::vector< TPI >& out, dip::uint nTensor ) {
   out.resize( nTensor, in[ 0 ].As< TPI >() );
   if( !in.IsScalar() ) {
      for( dip::uint ii = 1; ii < nTensor; ++ii ) {
         out[ ii ] = in[ ii ].As< TPI >();
      }
   }
}

// We can determine ahead of time for which image lines the `DrawEllipsoidLineFilter` line filter should
// be called (potentially a small subset of them!). Here we adjust `out` to be the bounding box for these image
// lines. `origin` is adjusted to match.
// Returns false if there are no pixels to process
inline bool NarrowImageView(
      Image& out,          // adjusted
      FloatArray const& sizes,
      FloatArray& origin   // adjusted
) {
   dip::uint nDims = out.Dimensionality();
   UnsignedArray outOffset( nDims );
   UnsignedArray outSizes( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint start = ceil_cast( origin[ ii ] - sizes[ ii ] / 2.0 );
      dip::sint end = floor_cast( origin[ ii ] + sizes[ ii ] / 2.0 );
      start = std::max( start, dip::sint( 0 ));
      end = std::min( end, static_cast< dip::sint >( out.Size( ii ) - 1 ));
      if( start > end ) {
         return false;
      }
      origin[ ii ] -= static_cast< dfloat >( start );
      outOffset[ ii ] = static_cast< dip::uint >( start );
      outSizes[ ii ] = static_cast< dip::uint >( end - start + 1 );
   }
   out.SetOriginUnsafe( out.Pointer( outOffset ));
   out.SetSizesUnsafe( outSizes );
   return true;
}

} // namespace

} // namespace dip

#endif // DIP_DRAW_SUPPORT_H
