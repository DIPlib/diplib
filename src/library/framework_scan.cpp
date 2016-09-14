/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <new>
#include <iostream>

#include "diplib.h"
#include "dip_framework.h"
#include "dip_numeric.h"

#include "copybuffer.h"

namespace dip {
namespace Framework {

void Scan(
      ImageConstRefArray const& c_in,
      ImageRefArray& c_out,
      DataTypeArray const& inBufferTypes,
      DataTypeArray const& outBufferTypes,
      DataTypeArray const& outImageTypes,
      UnsignedArray const& nTensorElements,
      ScanFilter lineFilter,
      const void* functionParameters,
      const std::vector< void* >& functionVariables,
      ScanOptions opts
) {
   std::size_t nIn = c_in.size();
   std::size_t nOut = c_out.size();
   if( ( nIn == 0 ) && ( nOut == 0 ) ) { return; } // Duh!

   // Check array sizes
   dip_ThrowIf( inBufferTypes.size() != nIn, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outBufferTypes.size() != nOut, E::ARRAY_ILLEGAL_SIZE );
   dip_ThrowIf( outImageTypes.size() != nOut, E::ARRAY_ILLEGAL_SIZE );

   // NOTE: In this function, we use some DimensionArray objects where the
   // array needs to hold nIn or nOut elements. We expect nIn and nOut to be
   // small in most cases.

   // Make simplified copies of input image headers so we can modify them at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip an output image without destroying
   // the input pixel data.
   PixelSize pixelSize;
   ImageArray in( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      Image const& tmp = c_in[ ii ].get();
      dip_ThrowIf( !tmp.IsForged(), E::IMAGE_NOT_FORGED );
      in[ ii ] = tmp.QuickCopy();
      if( !pixelSize.IsDefined() && tmp.HasPixelSize() ) {
         pixelSize = tmp.PixelSize();
      }
   }
   StringArray colspaces = OutputColorSpaces( c_in, nTensorElements );

   // Do tensor to spatial dimension if necessary
   bool tensorToSpatial = false;
   if( opts == Scan_TensorAsSpatialDim ) {
      bool allscalar = true;
      // We either convert all images, or none (so that dimensions still match).
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         if( !in[ ii ].IsScalar() ) {
            allscalar = false;
            break;
         }
      }
      if( !allscalar ) {
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            in[ ii ].TensorToSpatial( 0 );
         }
         tensorToSpatial = true;
      }
   }

   // Do singleton expansion if necessary
   UnsignedArray sizes;
   if( nIn > 0 ) {
      if( opts != Scan_NoSingletonExpansion ) {
         sizes = SingletonExpandedSize( in );
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            if( in[ ii ].Sizes() != sizes ) {
               SingletonExpansion( in[ ii ], sizes );
            }
         }
      } else {
         sizes = in[ 0 ].Sizes();
         for( dip::uint ii = 1; ii < nIn; ++ii ) {
            if( in[ ii ].Sizes() != sizes ) {
               dip_Throw( E::DIMENSIONS_DONT_MATCH );
            }
         }
      }
   } else {
      // nOut > 0, as was checked way at the top of this function.
      sizes = c_out[ 0 ].get().Sizes();
   }

   // Adjust output if necessary (and possible)
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ ii ].get();
      dip::uint nTensor = opts == Scan_TensorAsSpatialDim ? 1 : nTensorElements[ ii ]; // Input parameter ignored, output matches singleton-expanded number of tensor elements.
      bool good = false;
      if( tmp.IsForged() ) {
         if( tensorToSpatial ) {
            tmp.TensorToSpatial( 0 );
         }
         good = tmp.CheckProperties( sizes, nTensor, outImageTypes[ ii ], Option::ThrowException::doNotThrow );
      }
      if( !good ) {
         tmp.Strip();   // Will throw if image is protected
         tmp.SetSizes( sizes );
         tmp.SetTensorDimensions( nTensor );
         tmp.SetDataType( outImageTypes[ ii ] );
         tmp.Forge();
      }
   }

   // Make simplified copies of output image headers so we can modify them at will
   ImageArray out( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      out[ ii ] = c_out[ ii ].get().QuickCopy();
   }

   // Can we treat the images as if they were 1D?
   bool scan1D = sizes.size() <= 1;
   if( !scan1D ) {
      scan1D = opts != Scan_NeedCoordinates;
      if( scan1D ) {
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            if( !in[ ii ].HasSimpleStride() ) {
               scan1D = false;
               break;
            }
         }
      }
      if( scan1D ) {
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( !out[ ii ].HasSimpleStride() ) {
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
         in[ ii ].Flatten();
      }
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         out[ ii ].Flatten();
      }
      sizes = ( nIn > 0 ? in[ 0 ] : out[ 0 ] ).Sizes();
   }

   /*
   std::cout << "dip::Framework::Scan -- images\n";
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      std::cout << "   Input image, before: " << ii << std::endl;
      std::cout << c_in[ii].get();
      std::cout << "   Input image, after: " << ii << std::endl;
      std::cout << in[ii];
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      std::cout << "   Output image, before: " << ii << std::endl;
      std::cout << c_out[ii].get();
      std::cout << "   Output image, after: " << ii << std::endl;
      std::cout << out[ii];
   }
   */

   // For each image, determine if we need to make a temporary buffer.
   bool needBuffers = false;
   BooleanArray inUseBuffer( nIn );
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      inUseBuffer[ ii ] = in[ ii ].DataType() != inBufferTypes[ ii ];
      needBuffers |= inUseBuffer[ ii ];
   }
   BooleanArray outUseBuffer( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      outUseBuffer[ ii ] = out[ ii ].DataType() != outBufferTypes[ ii ];
      needBuffers |= outUseBuffer[ ii ];
   }
   // Temporary buffers are necessary also when expanding the tensor.
   // `lookUpTables[ii]` is the look-up table for `in[ii]`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< std::vector< dip::sint >> lookUpTables( nIn );
   if( ( opts == Scan_ExpandTensorInBuffer ) && ( opts != Scan_TensorAsSpatialDim ) ) {
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         if( !in[ ii ].Tensor().HasNormalOrder() ) {
            inUseBuffer[ ii ] = true;
            needBuffers = true;
            lookUpTables[ ii ] = in[ ii ].Tensor().LookUpTable();
         }
      }
   }

   // Determine the best processing dimension.
   dip::uint processingDim = OptimalProcessingDim( nIn > 0 ? in[ 0 ] : out[ 0 ] );

   // Determine the ideal buffer size
   dip::uint bufferSize = sizes[ processingDim ];
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
   std::vector< std::vector< uint8 > > buffers; // The outer one here is not a DimensionArray, because it won't delete() its contents
   std::vector< ScanBuffer > inScanBufs( nIn );   // We don't use DimensionArray here either, but we could
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      if( inUseBuffer[ ii ] ) {
         if( lookUpTables[ ii ].empty() ) {
            inScanBufs[ ii ].tensorLength = in[ ii ].TensorElements();
         } else {
            inScanBufs[ ii ].tensorLength = lookUpTables[ ii ].size();
         }
         inScanBufs[ ii ].tensorStride = 1;
         if( in[ ii ].Stride( processingDim ) == 0 ) {
            // A stride of 0 means all pixels are the same, allocate space for a single pixel
            inScanBufs[ ii ].stride = 0;
            buffers.emplace_back( inBufferTypes[ ii ].SizeOf() * inScanBufs[ ii ].tensorLength );
         } else {
            inScanBufs[ ii ].stride = inScanBufs[ ii ].tensorLength;
            buffers.emplace_back( bufferSize * inBufferTypes[ ii ].SizeOf() * inScanBufs[ ii ].tensorLength );
         }
         inScanBufs[ ii ].buffer = buffers.back().data();
      } else {
         inScanBufs[ ii ].tensorLength = in[ ii ].TensorElements();
         inScanBufs[ ii ].tensorStride = in[ ii ].TensorStride();
         inScanBufs[ ii ].stride = in[ ii ].Stride( processingDim );
         inScanBufs[ ii ].buffer = nullptr;
      }
   }
   std::vector< ScanBuffer > outScanBufs( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      if( outUseBuffer[ ii ] ) {
         outScanBufs[ ii ].tensorLength = out[ ii ].TensorElements();
         outScanBufs[ ii ].tensorStride = 1;
         outScanBufs[ ii ].stride = outScanBufs[ ii ].tensorLength;
         buffers.emplace_back( bufferSize * outBufferTypes[ ii ].SizeOf() * outScanBufs[ ii ].tensorLength );
         outScanBufs[ ii ].buffer = buffers.back().data();
      } else {
         outScanBufs[ ii ].tensorLength = out[ ii ].TensorElements();
         outScanBufs[ ii ].tensorStride = out[ ii ].TensorStride();
         outScanBufs[ ii ].stride = out[ ii ].Stride( processingDim );
         outScanBufs[ ii ].buffer = nullptr;
      }
   }

   /*
   std::cout << "dip::Framework::Scan -- buffers\n";
   std::cout << "   processing dimension = " << processingDim << std::endl;
   std::cout << "   buffer size = " << bufferSize << std::endl;
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      std::cout << "   in[" << ii << "] use buffer: " << ( inUseBuffer[ii] ? "yes" : "no" ) << std::endl;
      std::cout << "   in[" << ii << "] buffer stride: " << inScanBufs[ii].stride << std::endl;
      std::cout << "   in[" << ii << "] buffer tensorStride: " << inScanBufs[ii].tensorStride << std::endl;
      std::cout << "   in[" << ii << "] buffer tensorLength: " << inScanBufs[ii].tensorLength << std::endl;
      std::cout << "   in[" << ii << "] buffer type: " << inBufferTypes[ii].Name() << std::endl;
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      std::cout << "   out[" << ii << "] use buffer: " << ( outUseBuffer[ii] ? "yes" : "no" ) << std::endl;
      std::cout << "   out[" << ii << "] buffer stride: " << outScanBufs[ii].stride << std::endl;
      std::cout << "   out[" << ii << "] buffer tensorStride: " << outScanBufs[ii].tensorStride << std::endl;
      std::cout << "   out[" << ii << "] buffer tensorLength: " << outScanBufs[ii].tensorLength << std::endl;
      std::cout << "   out[" << ii << "] buffer type: " << outBufferTypes[ii].Name() << std::endl;
   }
   */

   // Iterate over lines in the image
   //std::cout << "dip::Framework::Scan -- running\n";
   UnsignedArray position( sizes.size(), 0 );
   IntegerArray inIndices( nIn, 0 );
   IntegerArray outIndices( nOut, 0 );
   for( ;; ) {

      // Iterate over line sections, if bufferSize < sizes[processingDim]
      for( dip::uint sectionStart = 0; sectionStart < sizes[ processingDim ]; sectionStart += bufferSize ) {
         position[ processingDim ] = sectionStart;
         dip::uint nPixels = std::min( bufferSize, sizes[ processingDim ] - sectionStart );

         // Get points to input and ouput lines
         //std::cout << "      sectionStart = " << sectionStart << std::endl;
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            //std::cout << "      inIndices[" << ii << "] = " << inIndices[ii] << std::endl;
            if( inUseBuffer[ ii ] ) {
               // If inIndices[ii] and sectionStart are the same as in the previous iteration, we don't need
               // to copy the buffer over again. This happens with singleton-expanded input images.
               // But it's easier to copy, and also safer as the lineFilter function could be bad and write in its input!
               CopyBuffer(
                     in[ ii ].Pointer( inIndices[ ii ] + sectionStart * in[ ii ].Stride( processingDim ) ),
                     in[ ii ].DataType(),
                     in[ ii ].Stride( processingDim ),
                     in[ ii ].TensorStride(),
                     inScanBufs[ ii ].buffer,
                     inBufferTypes[ ii ],
                     inScanBufs[ ii ].stride,
                     inScanBufs[ ii ].tensorStride,
                     inScanBufs[ ii ].stride == 0 ? 1
                                                  : bufferSize, // if stride == 0, copy only a single pixel because they're all the same
                     inScanBufs[ ii ].tensorLength,
                     lookUpTables[ ii ] );
            } else {
               inScanBufs[ ii ].buffer = in[ ii ].Pointer(
                     inIndices[ ii ] + sectionStart * in[ ii ].Stride( processingDim ) );
            }
         }
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            //std::cout << "      outIndices[" << ii << "] = " << outIndices[ii] << std::endl;
            if( !outUseBuffer[ ii ] ) {
               outScanBufs[ ii ].buffer = out[ ii ].Pointer(
                     outIndices[ ii ] + sectionStart * out[ ii ].Stride( processingDim ) );
            }
         }

         // Filter the line
         lineFilter(
               inScanBufs,
               outScanBufs,
               nPixels,
               processingDim,
               position,
               functionParameters,
               useFunctionVariables ? functionVariables[ thread ] : nullptr
         );

         // Copy back the line from output buffer to the image
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( outUseBuffer[ ii ] ) {
               CopyBuffer(
                     outScanBufs[ ii ].buffer,
                     outBufferTypes[ ii ],
                     outScanBufs[ ii ].stride,
                     outScanBufs[ ii ].tensorStride,
                     out[ ii ].Pointer( outIndices[ ii ] + sectionStart * out[ ii ].Stride( processingDim ) ),
                     out[ ii ].DataType(),
                     out[ ii ].Stride( processingDim ),
                     out[ ii ].TensorStride(),
                     bufferSize,
                     outScanBufs[ ii ].tensorLength,
                     std::vector< dip::sint > {} );
            }
         }
      }

      // Determine which line to process next until we're done
      position[ processingDim ] = 0; // reset this index
      dip::uint dd;
      for( dd = 0; dd < sizes.size(); dd++ ) {
         if( dd != processingDim ) {
            ++position[ dd ];
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               inIndices[ ii ] += in[ ii ].Stride( dd );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outIndices[ ii ] += out[ ii ].Stride( dd );
            }
            // Check whether we reached the last pixel of the line
            if( position[ dd ] != sizes[ dd ] ) {
               break;
            }
            // Rewind along this dimension
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               inIndices[ ii ] -= position[ dd ] * in[ ii ].Stride( dd );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outIndices[ ii ] -= position[ dd ] * out[ ii ].Stride( dd );
            }
            position[ dd ] = 0;
            // Continue loop to increment along next dimension
         }
      }
      if( dd == sizes.size() ) {
         break;            // We're done!
      }
   }

   // TODO: End threads.

   // Return output image tensors if we made them a spatial dimension
   if( tensorToSpatial ) {
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         c_out[ ii ].get().SpatialToTensor( 0 );
      }
   }

   // Set pixel size and color space
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ ii ].get();
      tmp.SetPixelSize( pixelSize );
      tmp.SetColorSpace( colspaces[ ii ] );
   }
}

} // namespace Framework
} // namespace dip
