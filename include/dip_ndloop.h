/*
 * DIPlib 3.0
 * This file contains support for writing nD loops.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_NDLOOP_H
#define DIP_NDLOOP_H

#include "dip_error.h"
#include "dip_types.h"


/// \file
/// Declares the dip::NDLoop namespace.


namespace dip {

/// An nD loop allows you to loop over all pixels in an image of arbitrary
/// dimensionality and strides. Example:
///
///      dip::sint offset;
///      dip::UnsignedArray pos = dip::NDLoop::Init( img, offset );
///      dip::uint16* ptr = (dip::uint16*)img.Origin();
///      dip::uint ii = 0;
///      do {
///         *(ptr + offset) = ii++;
///      } while( dip::NDLoop::Next( img, pos, offset );
namespace NDLoop {

/// Initializes the variables for an nD loop over an image of size `dims`.
inline UnsignedArray Init(
      const Image& img,
      dip::sint& offset
) {
   offset = 0;
   return UnsignedArray( img.Dimensionality(), 0 );
}

/// Initializes the variables for an nD loop over two images of size `dims`.
inline UnsignedArray Init(
      const Image& img1,
      const Image& img2,
      dip::sint& offset1,
      dip::sint& offset2
) {
   dip_ThrowIf( img1.RefDimensions() != img2.RefDimensions(), E::DIMENSIONS_DONT_MATCH );
   offset1 = 0;
   offset2 = 0;
   return UnsignedArray( img.Dimensionality(), 0 );
}


/// Calling this function will advance `position` to the next pixel. Use it in
/// a simple loop to visit all pixels in an image of arbitrary dimensionality.
/// The function will return `false` if all pixels have been visited, that is,
/// if the `position` now points to a non-existing pixel.
/// The `offset` is the offset to the pixel at `position`. Add `offset` to the
/// `origin` pointer to access the pixel (e.g. through dip::Image::Pointer).
/// The optional value `skipDim` indicates a dimension along which will not be
/// iterated. This is useful when looping over image lines, rather than over
/// pixels.
inline bool Next(
      UnsignedArray& position,
      dip::sint& offset,
      const UnsignedArray& dims,
      const IntegerArray& strides,
      dip::sint skipDim = -1
) {
   dip::uint dd;
   for( dd = 0; dd < position.size(); ++dd ) {
      if( (dip::sint)dd != skipDim ) {
         ++position[ dd ];
         offset += strides[ dd ];
         // Check whether we reached the last pixel of the line ...
         if( position[ dd ] < dims[ dd ] ) {
            break;
         }
         offset -= position[ dd ] * strides[ dd ];
         position[ dd ] = 0;
      }
   }
   return dd != position.size();
}

/// As the other Next, but looping over pixels in two images of the same
/// dimensions.
inline bool Next(
      UnsignedArray& position,
      dip::sint& offset1,
      dip::sint& offset2,
      const UnsignedArray& dims,
      const IntegerArray& strides1,
      const IntegerArray& strides2,
      dip::sint skipDim = -1
) {
   dip::uint dd;
   for( dd = 0; dd < position.size(); ++dd ) {
      if( (dip::sint)dd != skipDim ) {
         ++position[ dd ];
         offset1 += strides1[ dd ];
         offset2 += strides2[ dd ];
         // Check whether we reached the last pixel of the line ...
         if( position[ dd ] < dims[ dd ] ) {
            break;
         }
         offset1 -= position[ dd ] * strides1[ dd ];
         offset2 -= position[ dd ] * strides2[ dd ];
         position[ dd ] = 0;
      }
   }
   return dd != position.size();
}

} // namespace ndloop
} // namespace dip

#endif // DIP_NDLOOP_H
