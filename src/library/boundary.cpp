/*
 * DIPlib 3.0
 * This file contains definitions of the functions in boundary.h.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/iterators.h"

#include "copy_buffer.h"

namespace dip {

void ExtendImageLowLevel(
      Image const& c_in,
      Image& out,
      UnsignedArray borderSizes, // by copy so we can modify it
      BoundaryConditionArray boundaryConditions, // by copy so we can modify it
      Option::ExtendImage options
) {
   // Test input arguments
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( borderSizes.empty(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   dip::uint nDims = c_in.Dimensionality();
   DIP_START_STACK_TRACE
      borderSizes = ArrayUseParameter( std::move( borderSizes ), nDims );
      boundaryConditions = BoundaryArrayUseParameter( std::move( boundaryConditions ), nDims );
   DIP_END_STACK_TRACE

   // Save input data
   Image in = c_in; // Not quick copy, so we keep the color space info and pixel size info for later

   // Prepare output image
   UnsignedArray sizes = in.Sizes();
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      sizes[ ii ] += 2 * borderSizes[ ii ];
   }
   Tensor tensor = in.Tensor();
   std::vector< dip::sint > lookUpTable;
   bool expandTensor = false;
   if( !tensor.HasNormalOrder() && ( options == Option::ExtendImage_ExpandTensor )) {
      expandTensor = true;
      tensor = { tensor.Rows(), tensor.Columns() };
   }
   DIP_START_STACK_TRACE
      // Note that this can potentially affect `c_in` also, use only `in` from here on.
      out.ReForge( sizes, tensor.Elements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   DIP_END_STACK_TRACE
   out.ReshapeTensor( tensor );
   out.SetPixelSize( in.PixelSize() );
   if( !expandTensor ) {
      out.SetColorSpace( in.ColorSpace() );
   }

   // The view on the output image that matches the input
   RangeArray ranges( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint b = static_cast< dip::sint >( borderSizes[ ii ] );
      ranges[ ii ] = Range{ b, -b-1 };
   }

   // Copy input data to output
   Image tmp = out.At( ranges );
   if( expandTensor ) {
      ExpandTensor( in, tmp );
   } else {
      Copy( in, tmp );
   }

   // Extend the boundaries, one dimension at a time
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      if( borderSizes[ dim ] > 0 ) {
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
                  boundaryConditions[ dim ]
            );
         } while( ++it );
         ranges[ dim ] = Range{}; // expand the tmp image to cover the newly written data
         tmp = out.At( ranges );
      }
   }

   // Produce output by either using out directly or making a window of the original size over it.
   if( options == Option::ExtendImage_Masked ) {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dip::sint b = static_cast< dip::sint >( borderSizes[ ii ] );
         ranges[ ii ] = Range{ b, -b-1 };
      }
      out = out.At( ranges );
   }
}

} // namespace dip
