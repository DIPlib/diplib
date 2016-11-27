/*
 * DIPlib 3.0
 * This file contains definitions of the functions in boundary.h.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/iterators.h"

#include "copy_buffer.h"

namespace dip {

void ExtendImage(
      Image const& in,
      Image& out,
      UnsignedArray borderSizes,
      StringArray const& boundaryCondition,
      bool masked
) {
   // Test input arguments
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( borderSizes.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   dip::uint nDims = in.Dimensionality();
   BoundaryConditionArray bc;
   DIP_TRY
      borderSizes = ArrayUseParameter( borderSizes, nDims );
      bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      bc = BoundaryArrayUseParameter( bc, nDims );
   DIP_CATCH

   // Prepare output image
   UnsignedArray sizes = in.Sizes();
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      sizes[ ii ] += 2 * borderSizes[ ii ];
   }
   DIP_TRY
      out.ReForge( sizes, in.TensorElements(), in.DataType(), true );
   DIP_CATCH

   // The view on the output image that matches the input
   RangeArray ranges( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint b = static_cast< dip::sint >( borderSizes[ ii ] );
      ranges[ ii ] = Range{ b, -b-1 };
   }

   // Copy input data to output
   Image tmp = out.At( ranges );
   tmp.Copy( in );

   // Extend the boundaries, one dimension at a time
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      // Iterate over all image lines along this dimension
      // The iterator iterates over the lines with data only
      GenericImageIterator it( tmp, dim );
      do {
         // This is the function that does the actual boundary extension. It's defined in copy_buffer.cpp
         ExpandBuffer(
               it.Pointer(),
               tmp.DataType(),
               tmp.Stride( dim ),
               tmp.TensorStride(),
               tmp.Size( dim ),
               tmp.TensorElements(),
               borderSizes[ dim ],
               bc[ dim ]
         );
      } while( ++it );
      ranges[ dim ] = Range{}; // expand the tmp image to cover the newly written data
      tmp = out.At( ranges );
   }

   // Produce output by either using tmp directly or making a window of the original size over it.
   if( masked ) {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dip::sint b = static_cast< dip::sint >( borderSizes[ ii ] );
         ranges[ ii ] = Range{ b, -b-1 };
      }
      out = out.At( ranges );
   }

   // Make sure output has all the input image's properties
   out.CopyNonDataProperties( in );
}

} // namespace dip
