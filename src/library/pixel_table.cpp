/*
 * DIPlib 3.0
 * This file contains data-type--related functions.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib/pixel_table.h"

namespace dip {

// Construct a pixel table with offsets from a pixel table
PixelTableOffsets::PixelTableOffsets(
      PixelTable const& pt,
      Image const& image
) {
   sizes_ = pt.Sizes();
   origin_ = pt.Origin();
   nPixels_ = pt.NumberOfPixels();
   procDim_ = pt.ProcessingDimension();
   stride_ = image.Stride( procDim_ );
   auto const& inRuns = pt.Runs();
   runs_.resize( inRuns.size() );
   for( dip::uint ii = 0; ii < runs_.size(); ++ii ) {
      runs_[ ii ].offset = image.Offset( inRuns[ ii ].coordinates );
      runs_[ ii ].length = inRuns[ ii ].length;
   }
}

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
   // TODO: the old dip_BinaryImageToPixelTable goes here,
   // but instead of using the scan framework (which doesn't guarantee a whole line is processed at once), we use
   // the new ImageIterator functionality.

}

// Create a binary image from a pixel table
PixelTable::operator dip::Image() const {
   // TODO: the old dip_PixelTableToBinaryImage goes here
   return Image();
}

} // namespace dip
