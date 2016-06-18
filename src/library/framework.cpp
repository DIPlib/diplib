/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"

namespace dip {
namespace Framework {

// Figure out what the size of the images must be.
UnsignedArray SingletonExpandedSize(
      const ImageRefArray& in
) {
   UnsignedArray size = in[0].get().Dimensions();
   dip::uint ndims = size.size();
   for( dip::uint ii=1; ii<in.size(); ++ii ) {
      UnsignedArray size2 = in[ii].get().Dimensions();
      if( ndims < size2.size() ) {
         ndims = size2.size();
         size.resize( ndims, 1 );
      }
      for( dip::uint jj=0; jj<size2.size(); ++jj ) {
         if( size[jj] != size2[jj] ) {
            if( size[jj] == 1 ) {
               size[jj] = size2[jj];
            } else if( size2[jj] != 1 ) {
               dip_Throw( E::DIMENSIONS_DONT_MATCH );
            }
         }
      }
   }
   return size;
}

// Adjust the size of one image.
void SingletonExpansion(
      Image& in,
      const UnsignedArray& size
) {
   dip::uint ndims = size.size();
   if( in.Dimensionality() < ndims ) {
      in.ExpandDimensionality( ndims );
   }
   UnsignedArray size2 = in.Dimensions();
   for( dip::uint ii=0; ii<ndims; ++ii ) {
      if( size2[ii] != size[ii] ) {
         in.ExpandSingletonDimension( ii, size[ii] );
      }
   }
}

} // namespace Framework
} // namespace dip
