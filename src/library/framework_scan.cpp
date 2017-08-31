/*
 * DIPlib 3.0
 * This file contains definitions for the scan framework.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"

namespace dip {
namespace Framework {

void Scan(
      ImageConstRefArray const& c_in,
      ImageRefArray& c_out,
      DataTypeArray const& inBufferTypes,
      DataTypeArray const& outBufferTypes,
      DataTypeArray const& outImageTypes,
      UnsignedArray const& nTensorElements,
      ScanLineFilter& lineFilter,
      ScanOptions opts
) {
   std::size_t nIn = c_in.size();
   std::size_t nOut = c_out.size();
   if(( nIn == 0 ) && ( nOut == 0 )) {
      // Duh!
      return;
   }

   // Check array sizes
   DIP_THROW_IF( inBufferTypes.size() != nIn, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( outBufferTypes.size() != nOut, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( outImageTypes.size() != nOut, E::ARRAY_ILLEGAL_SIZE );

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
      DIP_THROW_IF( !tmp.IsForged(), E::IMAGE_NOT_FORGED );
      in[ ii ] = tmp.QuickCopy();
      if( !pixelSize.IsDefined() && tmp.HasPixelSize() ) {
         pixelSize = tmp.PixelSize();
      }
   }
   StringArray colspaces = OutputColorSpaces( c_in, nTensorElements );

   // Will we convert tensor to spatial dimension?
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
         tensorToSpatial = true;
      }
   }

   // Do singleton expansion if necessary
   UnsignedArray sizes;
   dip::uint tsize = 1;
   Tensor outTensor;
   if( nIn == 1 ) {
      sizes = in[ 0 ].Sizes();
      tsize = in[ 0 ].TensorElements();
      outTensor = in[ 0 ].Tensor();
   } else if( nIn > 1 ) {
      if( opts != Scan_NoSingletonExpansion ) {
         DIP_START_STACK_TRACE
            sizes = SingletonExpandedSize( in );
            if( tensorToSpatial ) {
               tsize = SingletonExpendedTensorElements( in );
            }
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               if( in[ ii ].Sizes() != sizes ) {
                  in[ ii ].ExpandSingletonDimensions( sizes );
               }
               if( outTensor.IsScalar() ) {
                  outTensor = in[ ii ].Tensor();
               }
               if( tensorToSpatial && ( in[ ii ].TensorElements() != tsize ) ) {
                  in[ ii ].ExpandSingletonTensor( tsize );
               }
            }
         DIP_END_STACK_TRACE
      } else {
         sizes = in[ 0 ].Sizes();
         for( dip::uint ii = 1; ii < nIn; ++ii ) {
            if( in[ ii ].Sizes() != sizes ) {
               DIP_THROW( E::SIZES_DONT_MATCH );
            }
            if( outTensor.IsScalar() ) {
               outTensor = in[ ii ].Tensor();
            }
         }
      }
   } else {
      // nOut > 0, as was checked way at the top of this function.
      sizes = c_out[ 0 ].get().Sizes();
      tsize = c_out[ 0 ].get().TensorElements();
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ ii ].get();
      dip::uint nTensor;
      if( opts == Scan_TensorAsSpatialDim ) {
         // Input parameter ignored, output matches singleton-expanded number of tensor elements.
         nTensor = tsize;
      } else {
         nTensor = nTensorElements[ ii ];
      }
      if( tmp.IsForged() && tmp.IsOverlappingView( in )) {
         tmp.Strip();
      }
      tmp.ReForge( sizes, nTensor, outImageTypes[ ii ], Option::AcceptDataTypeChange::DO_ALLOW );
      if( tensorToSpatial && !outTensor.IsScalar() ) {
         tmp.ReshapeTensor( outTensor );
      }
      tmp.SetPixelSize( pixelSize );
      tmp.SetColorSpace( colspaces[ ii ] );
   }
   DIP_END_STACK_TRACE

   // Make simplified copies of output image headers so we can modify them at will
   ImageArray out( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      out[ ii ] = c_out[ ii ].get().QuickCopy();
   }

   // Do tensor to spatial dimension if necessary
   if( tensorToSpatial ) {
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         in[ ii ].TensorToSpatial();
      }
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         out[ ii ].TensorToSpatial();
      }
      sizes.push_back( tsize );
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
            if(( ii > 0 ) && !in[ ii ].HasSameDimensionOrder( in[ 0 ] )) {
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
            if(( ii > 0 ) && !out[ ii ].HasSameDimensionOrder( out[ 0 ] )) {
               scan1D = false;
               break;
            }
         }
      }
   }
   if( scan1D && ( nIn > 0 ) && ( nOut > 0 )) {
      // Input all have the same order, and output all are the same order. But are these the same?
      if( !in[ 0 ].HasSameDimensionOrder( out[ 0 ] )) {
         scan1D = false;
      }
   }

   // If we can treat the images as 1D, convert them to 1D.
   DIP_START_STACK_TRACE
      if( scan1D ) {
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            in[ ii ].Flatten();
         }
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            out[ ii ].Flatten();
         }
         sizes = ( nIn > 0 ? in[ 0 ] : out[ 0 ] ).Sizes();
      }
   DIP_END_STACK_TRACE

   /*
   std::cout << "dip::Framework::Scan -- images\n";
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      std::cout << "- Input image, before: " << ii << std::endl;
      std::cout << "  " << c_in[ii].get(); // NOTE! this one might look like one of the output images now, if they are the same object
      std::cout << "- Input image, after: " << ii << std::endl;
      std::cout << "  " << in[ii];
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      std::cout << "- Output image, before: " << ii << std::endl;
      std::cout << "  " << c_out[ii].get();
      std::cout << "- Output image, after: " << ii << std::endl;
      std::cout << "  " << out[ii];
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
   if(( opts == Scan_ExpandTensorInBuffer ) && ( opts != Scan_TensorAsSpatialDim )) {
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

   // TODO: Determine the number of threads we'll be using.
   //       The size of the data has an influence. We can cut an image line in parts if necessary.
   //       I guess it would be useful to get an idea of the amount of work that
   //       the lineFilter does per pixel. If the caller can provide that estimate,
   //       we'd be able to use that to determine the threading schedule.
   //       Don't forget to check for opts==Scan_NoMultiThreading!

   DIP_START_STACK_TRACE
      lineFilter.SetNumberOfThreads( 1 );
   DIP_END_STACK_TRACE

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // Create buffer data structs and allocate buffers
   std::vector< std::vector< uint8 > > buffers; // The outer one here is not a DimensionArray, because it won't delete() its contents
   std::vector< ScanBuffer > inBuffers( nIn );   // We don't use DimensionArray here either, but we could
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      if( inUseBuffer[ ii ] ) {
         if( lookUpTables[ ii ].empty() ) {
            inBuffers[ ii ].tensorLength = in[ ii ].TensorElements();
         } else {
            inBuffers[ ii ].tensorLength = lookUpTables[ ii ].size();
         }
         inBuffers[ ii ].tensorStride = 1;
         if( in[ ii ].Stride( processingDim ) == 0 ) {
            // A stride of 0 means all pixels are the same, allocate space for a single pixel
            inBuffers[ ii ].stride = 0;
            buffers.emplace_back( inBufferTypes[ ii ].SizeOf() * inBuffers[ ii ].tensorLength );
         } else {
            inBuffers[ ii ].stride = static_cast< dip::sint >( inBuffers[ ii ].tensorLength );
            buffers.emplace_back( bufferSize * inBufferTypes[ ii ].SizeOf() * inBuffers[ ii ].tensorLength );
         }
         inBuffers[ ii ].buffer = buffers.back().data();
      } else {
         inBuffers[ ii ].tensorLength = in[ ii ].TensorElements();
         inBuffers[ ii ].tensorStride = in[ ii ].TensorStride();
         inBuffers[ ii ].stride = in[ ii ].Stride( processingDim );
         inBuffers[ ii ].buffer = nullptr;
      }
   }
   std::vector< ScanBuffer > outBuffers( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      if( outUseBuffer[ ii ] ) {
         outBuffers[ ii ].tensorLength = out[ ii ].TensorElements();
         outBuffers[ ii ].tensorStride = 1;
         outBuffers[ ii ].stride = static_cast< dip::sint >( outBuffers[ ii ].tensorLength );
         buffers.emplace_back( bufferSize * outBufferTypes[ ii ].SizeOf() * outBuffers[ ii ].tensorLength );
         outBuffers[ ii ].buffer = buffers.back().data();
      } else {
         outBuffers[ ii ].tensorLength = out[ ii ].TensorElements();
         outBuffers[ ii ].tensorStride = out[ ii ].TensorStride();
         outBuffers[ ii ].stride = out[ ii ].Stride( processingDim );
         outBuffers[ ii ].buffer = nullptr;
      }
   }

   /*
   std::cout << "dip::Framework::Scan -- buffers\n";
   std::cout << "   sizes = " << sizes << std::endl;
   std::cout << "   processing dimension = " << processingDim << std::endl;
   std::cout << "   buffer size = " << bufferSize << std::endl;
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      std::cout << "   in[" << ii << "]: use buffer: " << ( inUseBuffer[ii] ? "yes" : "no" ) << std::endl;
      std::cout << "      buffer stride: " << inBuffers[ii].stride << std::endl;
      std::cout << "      buffer tensorStride: " << inBuffers[ii].tensorStride << std::endl;
      std::cout << "      buffer tensorLength: " << inBuffers[ii].tensorLength << std::endl;
      std::cout << "      buffer type: " << inBufferTypes[ii].Name() << std::endl;
   }
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      std::cout << "   out[" << ii << "]: use buffer: " << ( outUseBuffer[ii] ? "yes" : "no" ) << std::endl;
      std::cout << "      buffer stride: " << outBuffers[ii].stride << std::endl;
      std::cout << "      buffer tensorStride: " << outBuffers[ii].tensorStride << std::endl;
      std::cout << "      buffer tensorLength: " << outBuffers[ii].tensorLength << std::endl;
      std::cout << "      buffer type: " << outBufferTypes[ii].Name() << std::endl;
   }
   */

   // Iterate over lines in the image
   //std::cout << "dip::Framework::Scan -- running\n";
   UnsignedArray position( sizes.size(), 0 );
   IntegerArray inIndices( nIn, 0 );
   IntegerArray outIndices( nOut, 0 );
   ScanLineFilterParameters scanLineFilterParams{
         inBuffers, outBuffers, 0, processingDim, position, tensorToSpatial, thread
   }; // Takes inBuffers, outBuffers, position as references
   for( ;; ) {

      // Iterate over line sections, if bufferSize < sizes[processingDim]
      for( dip::uint sectionStart = 0; sectionStart < sizes[ processingDim ]; sectionStart += bufferSize ) {
         position[ processingDim ] = sectionStart;
         dip::uint nPixels = std::min( bufferSize, sizes[ processingDim ] - sectionStart );
         scanLineFilterParams.bufferLength = nPixels;

         // Get pointers to input and ouput lines
         //std::cout << "      sectionStart = " << sectionStart << std::endl;
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            //std::cout << "      inIndices[" << ii << "] = " << inIndices[ii] << std::endl;
            if( inUseBuffer[ ii ] ) {
               // If inIndices[ii] and sectionStart are the same as in the previous iteration, we don't need
               // to copy the buffer over again. This happens with singleton-expanded input images.
               // But it's easier to copy, and also safer as the lineFilter function could be bad and write in its input!
               detail::CopyBuffer(
                     in[ ii ].Pointer( inIndices[ ii ] + static_cast< dip::sint >( sectionStart ) * in[ ii ].Stride( processingDim )),
                     in[ ii ].DataType(),
                     in[ ii ].Stride( processingDim ),
                     in[ ii ].TensorStride(),
                     inBuffers[ ii ].buffer,
                     inBufferTypes[ ii ],
                     inBuffers[ ii ].stride,
                     inBuffers[ ii ].tensorStride,
                     nPixels, // if stride == 0, only a single pixel will be copied, because they're all the same
                     inBuffers[ ii ].tensorLength,
                     lookUpTables[ ii ] );
            } else {
               inBuffers[ ii ].buffer = in[ ii ].Pointer( inIndices[ ii ] + static_cast< dip::sint >( sectionStart ) * in[ ii ].Stride( processingDim ));
            }
         }
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            //std::cout << "      outIndices[" << ii << "] = " << outIndices[ii] << std::endl;
            if( !outUseBuffer[ ii ] ) {
               outBuffers[ ii ].buffer = out[ ii ].Pointer( outIndices[ ii ] + static_cast< dip::sint >( sectionStart ) * out[ ii ].Stride( processingDim ));
            }
         }

         // Filter the line
         DIP_START_STACK_TRACE
            lineFilter.Filter( scanLineFilterParams );
         DIP_END_STACK_TRACE

         // Copy back the line from output buffer to the image
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( outUseBuffer[ ii ] ) {
               detail::CopyBuffer(
                     outBuffers[ ii ].buffer,
                     outBufferTypes[ ii ],
                     outBuffers[ ii ].stride,
                     outBuffers[ ii ].tensorStride,
                     out[ ii ].Pointer( outIndices[ ii ] + static_cast< dip::sint >( sectionStart ) * out[ ii ].Stride( processingDim )),
                     out[ ii ].DataType(),
                     out[ ii ].Stride( processingDim ),
                     out[ ii ].TensorStride(),
                     nPixels,
                     outBuffers[ ii ].tensorLength );
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
               inIndices[ ii ] -= static_cast< dip::sint >( position[ dd ] ) * in[ ii ].Stride( dd );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outIndices[ ii ] -= static_cast< dip::sint >( position[ dd ] ) * out[ ii ].Stride( dd );
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
}

} // namespace Framework
} // namespace dip
