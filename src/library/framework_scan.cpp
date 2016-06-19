/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_framework.h"
#include "dip_numeric.h"

namespace dip {
namespace Framework {

void Scan(
      const ImageRefArray& c_in,
      ImageRefArray&       c_out,
      const DataTypeArray& inBuffer,
      const DataTypeArray& outBuffer,
      const DataTypeArray& outImage,
      dip::uint            nTensorElements,
      ScanFilter           lineFilter,
      const void*          functionParameters,
      std::vector<void*>&  functionVariables,
      ScanOptions          opts
) {
   std::size_t nIn = c_in.size();
   std::size_t nOut = c_out.size();
   if( (nIn == 0) && (nOut == 0) ) return; // Duh!

   // Check array sizes
   dip_ThrowIf( inBuffer.size()  != nIn,  E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outBuffer.size() != nOut, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outImage.size()  != nOut, E::ARRAY_ILLEGAL_SIZE );

   // Make simplified copies of input image headers so we can modify them at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip an output image without destroying
   // the input pixel data.
   ImageArray in( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii) {
      in[ii] = c_in[ii].get().QuickCopy();
   }

   // Do tensor to spatial dimension if necessary
   bool tensorToSpatial = false;
   if( opts == Scan_TensorAsSpatialDim ) {
      nTensorElements = 1; // Input parameter ignored, output matches singleton-expanded number of tensor elements.
      bool allscalar = true;
      // We either convert all images, or none (so that dimensions still match).
      for( dip::uint ii = 0; ii < nIn; ++ii) {
         if( !in[ii].IsScalar() ) {
            allscalar = false;
            break;
         }
      }
      if( !allscalar ) {
         for( dip::uint ii = 0; ii < nIn; ++ii) {
            in[ii].TensorToSpatial( 0 );
         }
         tensorToSpatial = true;
      }
   }

   // Do singleton expansion if necessary
   UnsignedArray dims;
   if( nIn > 0 ) {
      dims = SingletonExpandedSize( in );
      for( dip::uint ii = 0; ii < nIn; ++ii) {
         if( in[ii].Dimensions() != dims ) {
            SingletonExpansion( in[ii], dims );
         }
      }
   } else {
      // nOut > 0, as was checked way at the top of this function.
      dims = c_out[0].get().Dimensions();
   }

   // Adjust output if necessary (and possible)
   int nTensElems = nTensorElements;
   if( tensorToSpatial ) {
      nTensElems = 1;
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ii].get();
      if( tensorToSpatial ) {
         tmp.TensorToSpatial( 0 );
      }
      bool good = tmp.CheckProperties( dims, nTensElems, outImage[ii], Option::ThrowException::doNotThrow );
      if( !good ) {
         tmp.Strip();   // Will throw if image is protected
         tmp.SetDimensions( dims );
         tmp.SetTensorDimensions( nTensorElements );
         tmp.SetDataType( outImage[ii] );
         tmp.Forge();
      }
   }
   // Make simplified copies of output image headers so we can modify them at will
   ImageArray out( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii) {
      out[ii] = c_out[ii].get().QuickCopy();
   }

   // Can we treat the images as if they were 1D?
   bool scan1D = dims.size() <= 1;
   if( !scan1D ) {
      scan1D = opts != Scan_NeedCoordinates;
      if( scan1D ) {
         for( dip::uint ii = 0; ii < nIn; ++ii) {
            if( !in[ii].HasSimpleStride() ) {
               scan1D = false;
               break;
            }
         }
      }
      if( scan1D ) {
         for( dip::uint ii = 0; ii < nOut; ++ii) {
            if( !out[ii].HasSimpleStride() ) {
               scan1D = false;
               break;
            }
         }
      }
   }

   // If we can treat the images as 1D, convert them 1D.
   // Note we're only converting the copies of the headers, not the original ones.
   // Note also that if we are dealing with a 0D image, it also has a simple
   // stride and hence will be converted into 1D.
   if( scan1D ) {
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         in[ii].Flatten();
      }
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         out[ii].Flatten();
      }
   }

   // For each image, determine if we need to make a temporary buffer.
   bool needBuffers = false;
   std::vector<bool> inNeedBuffer( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      inNeedBuffer[ii] = in[ii].DataType() != inBuffer[ii];
      needBuffers |= inNeedBuffer[ii];
   }
   std::vector<bool> outNeedBuffer( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      outNeedBuffer[ii] = out[ii].DataType() != outBuffer[ii];
      needBuffers |= outNeedBuffer[ii];
   }

   // Determine the best processing dimension, which is the one with the
   // smallest stride, except if that dimension is very small and there's a
   // longer dimension.
   dip::uint processingDim = 0;
   {
      constexpr dip::uint SMALL_IMAGE = 63;  // A good value would depend on the size of cache.
      // We use the strides of the first input image to determine this, if it exists.
      IntegerArray strides;
      if( nIn > 0 ) {
         strides = in[0].Strides();
      } else {
         strides = out[0].Strides();
      }
      for( dip::uint ii = 1; ii < strides.size(); ++ii ) {
         if( strides[ii] < strides[processingDim] ) {
            if(( dims[ii] > SMALL_IMAGE ) || ( dims[ii] > dims[processingDim] )) {
               processingDim = ii;
            }
         }
      }
   }

   // Determine the ideal buffer size
   dip::uint bufferSize = dims[processingDim];
   if( needBuffers ) {
      // Maybe the buffer size should be smaller
      if( bufferSize > MAX_BUFFER_SIZE ) {
         dip::uint n = div_ceil( bufferSize, MAX_BUFFER_SIZE );
         bufferSize = div_ceil( bufferSize, n );
      }
   }

   // TODO: Determine the number of threads we'll be using. The size of the data
   //       has an influence. We can cut an image line in parts if necessary.
   //       I guess it would be useful to get an idea of the amount of work that
   //       the lineFilter does per pixel. If the caller can provide that estimate,
   //       we'd be able to use that to determine the threading schedule.
   //       If we have a 1D image

   // TODO: Start threads, ech thread makes its own buffers.

   // TODO: Iterate over the image, get pointers to data, copy data to buffers
   //       if needed, call the lineFilter, copy data back if necessary

   // TODO: End threads.

   // Return output image tensors if we made them a spatial dimension
   if (tensorToSpatial) {
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         c_out[ii].get().SpatialToTensor( 0 );
      }
   }
}

} // namespace Framework
} // namespace dip
