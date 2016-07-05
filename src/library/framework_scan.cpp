/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <new>

#include "diplib.h"
#include "dip_framework.h"
#include "dip_numeric.h"

#include "copybuffer.h"

namespace dip {
namespace Framework {

void Scan(
      const ImageRefArray& c_in,
      ImageRefArray&       c_out,
      const DataTypeArray& inBufferTypes,
      const DataTypeArray& outBufferTypes,
      const DataTypeArray& outImageTypes,
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
   dip_ThrowIf( inBufferTypes.size()  != nIn,  E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outBufferTypes.size() != nOut, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outImageTypes.size()  != nOut, E::ARRAY_ILLEGAL_SIZE );

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
      if( opts == Scan_NoSingletonExpansion ) {
         dims = SingletonExpandedSize( in );
         for( dip::uint ii = 0; ii < nIn; ++ii) {
            if( in[ii].Dimensions() != dims ) {
               SingletonExpansion( in[ii], dims );
            }
         }
      } else {
         dims = in[0].Dimensions();
         for( dip::uint ii = 1; ii < nIn; ++ii) {
            if( in[ii].Dimensions() != dims ) {
               dip_Throw( E::DIMENSIONS_DONT_MATCH );
            }
         }
      }
   } else {
      // nOut > 0, as was checked way at the top of this function.
      dims = c_out[0].get().Dimensions();
   }

   // Adjust output if necessary (and possible)
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ii].get();
      bool good = false;
      if( tmp.IsForged() ) {
         if( tensorToSpatial ) {
            tmp.TensorToSpatial( 0 );
         }
         good = tmp.CheckProperties( dims, nTensorElements, outImageTypes[ii], Option::ThrowException::doNotThrow );
      }
      if( !good ) {
         tmp.Strip();   // Will throw if image is protected
         tmp.SetDimensions( dims );
         tmp.SetTensorDimensions( nTensorElements );
         tmp.SetDataType( outImageTypes[ii] );
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
   std::vector<bool> inUseBuffer( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      inUseBuffer[ii] = in[ii].DataType() != inBufferTypes[ii];
      needBuffers |= inUseBuffer[ii];
   }
   std::vector<bool> outUseBuffer( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      outUseBuffer[ii] = out[ii].DataType() != outBufferTypes[ii];
      needBuffers |= outUseBuffer[ii];
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
   //       functionVariables.size() is the upper limit to the number of threads,
   //       unless it is zero.

   bool useFunctionVariables = functionVariables.size() > 0;

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // Create buffer data structs and allocate buffers
   // We use `operator new` instead of `std::malloc` here because it throws if out of memory.
   std::vector<void*> buffers;
   std::vector<ScanBuffer> inScanBufs( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      if( inUseBuffer[ii] ) {
         buffers.push_back( operator new( bufferSize * inBufferTypes[ii].SizeOf() * in[ii].TensorElements() ) );
         inScanBufs[ii].buffer = buffers.back();
         inScanBufs[ii].stride = in[ii].TensorElements();
         inScanBufs[ii].tensorStride = 1;
         inScanBufs[ii].tensorLength = in[ii].TensorElements();
      } else {
         inScanBufs[ii].buffer = nullptr;
         inScanBufs[ii].stride = in[ii].Stride( processingDim );
         inScanBufs[ii].tensorStride = in[ii].TensorStride();
         inScanBufs[ii].tensorLength = in[ii].TensorElements();
      }
   }
   std::vector<ScanBuffer> outScanBufs( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      if( outUseBuffer[ii] ) {
         buffers.push_back( operator new( bufferSize * outBufferTypes[ii].SizeOf() * out[ii].TensorElements() ) );
         outScanBufs[ii].buffer = buffers.back();
         outScanBufs[ii].stride = out[ii].TensorElements(); // == nTensorElements
         outScanBufs[ii].tensorStride = 1;
         outScanBufs[ii].tensorLength = out[ii].TensorElements(); // == nTensorElements
      } else {
         outScanBufs[ii].buffer = nullptr;
         outScanBufs[ii].stride = out[ii].Stride( processingDim );
         outScanBufs[ii].tensorStride = out[ii].TensorStride();
         outScanBufs[ii].tensorLength = out[ii].TensorElements(); // == nTensorElements
      }
   }

   // Iterate over lines in the image
   UnsignedArray position( dims.size(), 0 );
   std::vector<dip::sint> inIndices( nIn, 0 );
   std::vector<dip::sint> outIndices( nOut, 0 );
   for(;;) {

      // Iterate over line sections, if bufferSize < dims[processingDim]
      for( dip::uint sectionStart = 0; sectionStart < dims[processingDim]; sectionStart += bufferSize ) {
         position[processingDim] = sectionStart;
         dip::uint nPixels = std::min( bufferSize, dims[processingDim] - sectionStart );

         // Get points to input and ouput lines
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            if( inUseBuffer[ii] ) {
               CopyBuffer(
                     in[ii].Pointer( inIndices[ii] + sectionStart * in[ii].Stride( processingDim ) ),
                     in[ii].DataType(),
                     in[ii].Stride( processingDim ),
                     in[ii].TensorStride(),
                     inScanBufs[ii].buffer,
                     inBufferTypes[ii],
                     inScanBufs[ii].stride,
                     inScanBufs[ii].tensorStride,
                     bufferSize,
                     inScanBufs[ii].tensorLength );
            } else {
               inScanBufs[ii].buffer = in[ii].Pointer( inIndices[ii] + sectionStart * inScanBufs[ii].stride );
            }
         }
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( !outUseBuffer[ii] ) {
               outScanBufs[ii].buffer = out[ii].Pointer( outIndices[ii] + sectionStart * outScanBufs[ii].stride );
            }
         }

         // Filter the line
         lineFilter (
            inScanBufs,
            outScanBufs,
            nPixels,
            processingDim,
            position,
            functionParameters,
            useFunctionVariables ? functionVariables[thread] : nullptr
         );

         // Copy back the line from output buffer to the image
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( outUseBuffer[ii] ) {
               CopyBuffer(
                     outScanBufs[ii].buffer,
                     outBufferTypes[ii],
                     outScanBufs[ii].stride,
                     outScanBufs[ii].tensorStride,
                     out[ii].Pointer( outIndices[ii] + sectionStart * out[ii].Stride( processingDim ) ),
                     out[ii].DataType(),
                     out[ii].Stride( processingDim ),
                     out[ii].TensorStride(),
                     bufferSize,
                     outScanBufs[ii].tensorLength ); // == nTensorElements
            }
         }
      }

      // Determine which line to process next until we're done
      position[processingDim] = 0; // reset this index
      dip::uint dd;
      for ( dd = 0; dd < dims.size(); dd++ ) {
         if( dd != processingDim ) {
            ++position[dd];
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               inIndices[ii] += in[ii].Stride( dd );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outIndices[ii] += out[ii].Stride( dd );
            }
            // Check whether we reached the last pixel of the line
            if( position[dd] != dims[dd] ) {
               break;
            }
            // Rewind along this dimension
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               inIndices[ii] -= position[dd] * in[ii].Stride( dd );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outIndices[ii] -= position[dd] * out[ii].Stride( dd );
            }
            position[dd] = 0;
            // Continue loop to increment along next dimension
         }
      }
      if( dd == dims.size() ) {
         break;            // We're done!
      }
   }

   // Deallocate buffers
   for( dip::uint ii = 0; ii < buffers.size(); ++ii ) {
      operator delete( buffers[ii] );
   }

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
