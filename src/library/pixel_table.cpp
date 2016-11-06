/*
 * DIPlib 3.0
 * This file contains data-type--related functions.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib/pixel_table.h"

namespace dip {

// Construct a pixel table from a unit circle
PixelTable::PixelTable(
      String shape,
      FloatArray size,
      dip::uint procDim
) {
   // TODO: the old dip_PixelTableCreateFilter goes here
}

// Construct a pixel table from a binary image
PixelTable::PixelTable(
      Image mask,
      UnsignedArray origin,
      dip::uint procDim
) {
   // TODO: the old dip_BinaryImageToPixelTable goes here
}

// Create a binary image from a pixel table
PixelTable::operator dip::Image() const {
   // TODO: the old dip_PixelTableToBinaryImage goes here
   return Image();
}

} // namespace dip
