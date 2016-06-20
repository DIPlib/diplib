/*
 * DIPlib 3.0
 * This file contains support for writing nD loops.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_NDLOOP_H
#define DIP_NDLOOP_H

#include "dip_types.h"

namespace dip {

namespace ndloop {

/// Initializes the variables for an nD loop.
inline UnsignedArray Init(
      dip::sint& index,
      const UnsignedArray& dims
) {
   index = 0;
   return UnsignedArray( dims.size(), 0 );
}


/// Calling this function will advance `position` to the next pixel. Use it in
/// a simple loop to visit all pixels in an image of arbitrary dimensionality.
/// The function will return `false` if all pixels have been visited, that is,
/// if the `position` now points to a non-existing pixel.
/// The `index` is the offset to the pixel at `position`. Add `index` to the
/// `origin` pointer to access the pixel.
/// The optional value `skipDim` indicates a dimension along which will not be
/// iterated. This is useful when looping over image lines, rather than over
/// pixels.
inline bool Next(
      UnsignedArray& position,
      dip::sint& index,
      const UnsignedArray& dims,
      const IntegerArray& strides,
      dip::sint skipDim = -1
) {
   dip::uint dd;
   for( dd = 0; dd < position.size(); ++dd ) {
      if( (dip::sint)dd != skipDim ) {
         ++position[ dd ];
         index += strides[ dd ];
         // Check whether we reached the last pixel of the line ...
         if( position[ dd ] < dims[ dd ] ) {
            break;
         }
         index -= position[ dd ] * strides[ dd ];
         position[ dd ] = 0;
      }
   }
   return dd != position.size();
}

} // namespace ndloop
} // namespace dip

#endif // DIP_NDLOOP_H
