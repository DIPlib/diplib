/*
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

#include "diplib/framework.h"

#include <algorithm>
#include <vector>

#include "diplib.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"

#include "framework_support.h"

namespace dip {
namespace Framework {

namespace {

String OutputColorSpace(
      ImageConstRefArray const& c_in,
      dip::uint nTensorElements
) {
   for( dip::uint jj = 0; jj < c_in.size(); ++jj ) {
      Image const& tmp = c_in[ jj ].get();
      if( tmp.IsColor() && ( tmp.TensorElements() == nTensorElements )) {
         return tmp.ColorSpace();
      }
   }
   return "";
}

StringArray OutputColorSpaces(
      ImageConstRefArray const& c_in,
      UnsignedArray const& nTensorElements
) {
   dip::uint nOut = nTensorElements.size();
   StringArray colspaces( nOut );
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      colspaces[ ii ] = OutputColorSpace( c_in, nTensorElements[ ii ] );
   }
   return colspaces;
}

} // namespace

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
   dip::uint nIn = c_in.size();
   dip::uint nOut = c_out.size();
   if(( nIn == 0 ) && ( nOut == 0 )) {
      // Duh!
      return;
   }

   // Check array sizes
   DIP_THROW_IF(( inBufferTypes.size()  != nIn )  ||
                ( outBufferTypes.size() != nOut ) ||
                ( outImageTypes.size()  != nOut ), E::ARRAY_PARAMETER_WRONG_LENGTH );

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

   // Will we convert tensor to spatial dimension?
   bool tensorToSpatial = false;
   if( opts.Contains( ScanOption::TensorAsSpatialDim )) {
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
   dip::uint tSize = 1;
   Tensor outTensor;
   if( nIn == 1 ) {
      sizes = in[ 0 ].Sizes();
      tSize = in[ 0 ].TensorElements();
      outTensor = in[ 0 ].Tensor();
   } else if( nIn > 1 ) {
      if( !opts.Contains( ScanOption::NoSingletonExpansion )) {
         DIP_START_STACK_TRACE
            sizes = SingletonExpandedSize( in );
            if( tensorToSpatial ) {
               tSize = SingletonExpendedTensorElements( in );
            }
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               if( in[ ii ].Sizes() != sizes ) {
                  in[ ii ].ExpandSingletonDimensions( sizes );
               }
               if( outTensor.IsScalar() ) {
                  outTensor = in[ ii ].Tensor();
               }
               if( tensorToSpatial && ( in[ ii ].TensorElements() != tSize ) ) {
                  in[ ii ].ExpandSingletonTensor( tSize );
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
      tSize = c_out[ 0 ].get().TensorElements();
   }

   // Figure out color spaces for the output images
   StringArray colspaces;
   if( nIn > 0 ) {
      if( opts.Contains( ScanOption::TensorAsSpatialDim )) {
         colspaces.resize( 1 );
         colspaces[ 0 ] = OutputColorSpace( c_in, tSize );
      } else {
         colspaces = OutputColorSpaces( c_in, nTensorElements );
      }
   }

   // Ensure we don't do computations along a dimension that is singleton-expanded in all inputs
   BooleanArray isSingletonExpanded( sizes.size(), false );
   bool tIsSingletonExpanded = false;
   UnsignedArray trueSizes = sizes;
   dip::uint trueTSize = tSize;
   if( nIn > 0 ) {
      isSingletonExpanded.fill( true );
      tIsSingletonExpanded = true;
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         for( dip::uint jj = 0; jj < sizes.size(); ++jj ) {
            if( in[ ii ].Stride( jj ) != 0 ) {
               isSingletonExpanded[ jj ] = false;
            }
         }
         if( in[ ii ].TensorStride() != 0 ) {
            tIsSingletonExpanded = false;
         }
      }
      for( dip::uint jj = 0; jj < sizes.size(); ++jj ) {
         if( isSingletonExpanded[ jj ] ) {
            sizes[ jj ] = 1;
         }
      }
      if( tensorToSpatial && tIsSingletonExpanded ) {
         tSize = 1;
      }
   }
   for( dip::uint ii = 0; ii < nIn; ++ii ) {
      for( dip::uint jj = 0; jj < sizes.size(); ++jj ) {
         if( isSingletonExpanded[ jj ] ) {
            in[ ii ].UnexpandSingletonDimension( jj );
         }
      }
      if( tensorToSpatial && tIsSingletonExpanded ) {
         in[ ii ].UnexpandSingletonTensor();
      }
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ ii ].get();
      dip::uint nTensor{};
      if( opts.Contains( ScanOption::TensorAsSpatialDim )) {
         // Input parameter ignored, output matches singleton-expanded number of tensor elements.
         nTensor = tSize;
      } else {
         nTensor = nTensorElements[ ii ];
      }
      if( tmp.IsForged() && tmp.IsOverlappingView( in )) {
         tmp.Strip();
      }
      tmp.ReForge( sizes, nTensor, outImageTypes[ ii ], Option::AcceptDataTypeChange::DO_ALLOW );
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
      sizes.push_back( tSize );
   }

   // Can we treat the images as if they were 1D?
   bool scan1D = sizes.size() <= 1;
   if( !scan1D ) {
      scan1D = !opts.Contains( ScanOption::NeedCoordinates );
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
      if( scan1D && ( nIn > 0 ) && ( nOut > 0 )) {
         // Input all have the same order, and output all are the same order. But are these the same?
         if( !in[ 0 ].HasSameDimensionOrder( out[ 0 ] )) {
            scan1D = false;
         }
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
      if( !outUseBuffer[ ii ] && opts.Contains( ScanOption::NotInPlace )) {
         // Make sure we don't alias
         for( dip::uint jj = 0; jj < nIn; ++jj ) {
            if( !inUseBuffer[ jj ] && Alias( in[ jj ], out[ ii ] )) {
               outUseBuffer[ ii ] = true;
               break;
            }
         }
      }
      needBuffers |= outUseBuffer[ ii ];
   }
   // Temporary buffers are necessary also when expanding the tensor.
   // `lookUpTables[ii]` is the look-up table for `in[ii]`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< std::vector< dip::sint >> lookUpTables( nIn );
   if( opts.Contains( ScanOption::ExpandTensorInBuffer ) && !opts.Contains( ScanOption::TensorAsSpatialDim )) {
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         if( !in[ ii ].Tensor().HasNormalOrder() ) {
            inUseBuffer[ ii ] = true;
            needBuffers = true;
            lookUpTables[ ii ] = in[ ii ].Tensor().LookUpTable();
         }
      }
   }

   dip::uint processingDim = 0;
   dip::uint nLines = 1;
   dip::uint lineLength = 0;
   dip::uint bufferSize = 0;
   dip::uint nThreads = 1;
   if( scan1D ) {

      // One image line --- Iterate over sections of the image if we need large buffers or we want to use parallelism ---

      // Note that this case likely comes from an image that was flattened to 1D for processing. If so, it
      // potentially is much larger than you'd expect for an image line. If copying image lines to a buffer
      // for processing, we could incur a large cost for such a large image line. So we divide the line up
      // into chunks that are easier to copy into a buffer. Also, if we want to do parallel processing, we
      // must divide the line up into a chunk for each thread.

      lineLength = bufferSize = sizes[ processingDim ];

      // Determine the number of threads we'll be using
      if( !opts.Contains( ScanOption::NoMultiThreading )) {
         nThreads = GetNumberOfThreads();
         if( nThreads > 1 ) {
            DIP_START_STACK_TRACE
            dip::uint operations = lineLength * lineFilter.GetNumberOfOperations( nIn, nOut, ( nIn > 0 ? in[ 0 ] : out[ 0 ] ).TensorElements() );
            // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
            if( operations < threadingThreshold ) {
               nThreads = 1;
            }
            DIP_END_STACK_TRACE
         }
      }

      // Chunk size if we use threads
      if( nThreads > 1 ) {
         lineLength = bufferSize = div_ceil( lineLength, nThreads );
      }
      // Chunk size if we'll be copying data to buffers
      if( needBuffers ) {
         if( bufferSize > MAX_BUFFER_SIZE ) {
            // Divide each thread's work into equal chunks, smaller than MAX_BUFFER_SIZE
            nLines = div_ceil( bufferSize, MAX_BUFFER_SIZE );
            bufferSize = div_ceil( bufferSize, nLines );
         }
      }
      nLines *= nThreads;

      // Many of these variables have a slightly different (but equivalent) meaning if `scan1D`
      // For example: `nLines` is the total number of chunks to process.
      // `lineLength` is the number of pixels that each thread will process.

   } else {

      // Multiple image lines --- Iterate over image lines ---

      // Determine the best processing dimension.
      processingDim = OptimalProcessingDim( nIn > 0 ? in[ 0 ] : out[ 0 ] );
      lineLength = bufferSize = sizes[ processingDim ];
      nLines = sizes.product() / bufferSize;

      // Determine the number of threads we'll be using
      if( !opts.Contains( ScanOption::NoMultiThreading )) {
         nThreads = std::min( GetNumberOfThreads(), nLines );
         if( nThreads > 1 ) {
            DIP_START_STACK_TRACE
            dip::uint operations = nLines * lineLength * lineFilter.GetNumberOfOperations( nIn, nOut, ( nIn > 0 ? in[ 0 ] : out[ 0 ] ).TensorElements() );
            // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
            if( operations < threadingThreshold ) {
               nThreads = 1;
            }
            DIP_END_STACK_TRACE
         }
      }

   }

   dip::uint nLinesPerThread = div_ceil( nLines, nThreads );
   if( scan1D ) {
      nThreads = std::min( div_ceil( sizes[ processingDim ], lineLength ), nThreads );
   } else {
      nThreads = std::min( div_ceil( nLines, nLinesPerThread ), nThreads);
   }
   std::vector< UnsignedArray > startCoords;

   // Start threads, each thread makes its own buffers
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      {
         nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
         DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads ));
         // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
         // image lines to process.
         if( scan1D ) {
            startCoords.resize( nThreads );
            startCoords[ 0 ] = UnsignedArray( 1, 0 );
            for( dip::uint ii = 1; ii < nThreads; ++ii ) {
               startCoords[ ii ] = startCoords[ ii - 1 ];
               startCoords[ ii ][ 0 ] += lineLength;        // `lineLength` in this case is the number of pixels per thread
            }
         } else {
            startCoords = SplitImageEvenlyForProcessing( sizes, nThreads, nLinesPerThread, processingDim );
         }
      }
      #pragma omp barrier

      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );
      std::vector< AlignedBuffer > buffers; // The outer one here is not a DimensionArray, because it won't delete() its contents

      // Create input buffer data structs and allocate buffers
      std::vector< ScanBuffer > inBuffers( nIn );  // We don't use DimensionArray here either, but we could
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

      // Create output buffer data structs and allocate buffers
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
         std::cout << "      buffer type: " << inBufferTypes[ii] << std::endl;
      }
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         std::cout << "   out[" << ii << "]: use buffer: " << ( outUseBuffer[ii] ? "yes" : "no" ) << std::endl;
         std::cout << "      buffer stride: " << outBuffers[ii].stride << std::endl;
         std::cout << "      buffer tensorStride: " << outBuffers[ii].tensorStride << std::endl;
         std::cout << "      buffer tensorLength: " << outBuffers[ii].tensorLength << std::endl;
         std::cout << "      buffer type: " << outBufferTypes[ii] << std::endl;
      }
      */

      UnsignedArray position = startCoords[ thread ];
      ScanLineFilterParameters scanLineFilterParams{
            inBuffers, outBuffers, bufferSize, processingDim, position, tensorToSpatial, thread
      }; // Takes inBuffers, outBuffers, position as references
      IntegerArray inOffsets( nIn );
      for( dip::uint ii = 0; ii < nIn; ++ii ) {
         inOffsets[ ii ] = in[ ii ].Offset( position );
      }
      IntegerArray outOffsets( nOut );
      for( dip::uint ii = 0; ii < nOut; ++ii ) {
         outOffsets[ ii ] = out[ ii ].Offset( position );
      }
      dip::uint lastCoord = 0;
      if( scan1D ) {
         lastCoord = position[ 0 ] + lineLength;
         lastCoord = std::min( lastCoord, sizes[ 0 ] );
      }

      // Loop over nLinesPerThread image lines
      for( dip::uint jj = 0; jj < nLinesPerThread ; ++jj ) {

         // Make `bufferSize` smaller if it's the last chunk in a 1D image
         if( scan1D ) {
            if( position[ 0 ] >= lastCoord ) { // This *should* not happen...
               break;
            }
            scanLineFilterParams.bufferLength = std::min( bufferSize, lastCoord - position[ 0 ] );
         }

         // Get pointers to input and output lines
         for( dip::uint ii = 0; ii < nIn; ++ii ) {
            if( inUseBuffer[ ii ] ) {
               // If inOffsets[ii] and is the same as in the previous iteration, we don't need
               // to copy the buffer over again. This happens with singleton-expanded input images.
               // But it's easier to copy, and also safer as the lineFilter function could be bad and write in its input!
               detail::CopyBuffer(
                     in[ ii ].Pointer( inOffsets[ ii ] ),
                     in[ ii ].DataType(),
                     in[ ii ].Stride( processingDim ),
                     in[ ii ].TensorStride(),
                     inBuffers[ ii ].buffer,
                     inBufferTypes[ ii ],
                     inBuffers[ ii ].stride,
                     inBuffers[ ii ].tensorStride,
                     scanLineFilterParams.bufferLength, // if stride == 0, only a single pixel will be copied, because they're all the same
                     inBuffers[ ii ].tensorLength,
                     lookUpTables[ ii ] );
            } else {
               inBuffers[ ii ].buffer = in[ ii ].Pointer( inOffsets[ ii ] );
            }
         }
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( !outUseBuffer[ ii ] ) {
               outBuffers[ ii ].buffer = out[ ii ].Pointer( outOffsets[ ii ] );
            }
         }

         // Filter the line
         lineFilter.Filter( scanLineFilterParams );

         // Copy back the line from output buffer to the image
         for( dip::uint ii = 0; ii < nOut; ++ii ) {
            if( outUseBuffer[ ii ] ) {
               detail::CopyBuffer(
                     outBuffers[ ii ].buffer,
                     outBufferTypes[ ii ],
                     outBuffers[ ii ].stride,
                     outBuffers[ ii ].tensorStride,
                     out[ ii ].Pointer( outOffsets[ ii ] ),
                     out[ ii ].DataType(),
                     out[ ii ].Stride( processingDim ),
                     out[ ii ].TensorStride(),
                     scanLineFilterParams.bufferLength,
                     outBuffers[ ii ].tensorLength );
            }
         }

         // Determine which line to process next until we're done
         if( scan1D ) {
            position[ 0 ] += bufferSize;
            for( dip::uint ii = 0; ii < nIn; ++ii ) {
               inOffsets[ ii ] += static_cast< dip::sint >( bufferSize ) * in[ ii ].Stride( 0 );
            }
            for( dip::uint ii = 0; ii < nOut; ++ii ) {
               outOffsets[ ii ] += static_cast< dip::sint >( bufferSize ) * out[ ii ].Stride( 0 );
            }
         } else {
            dip::uint dd = 0;
            for( ; dd < sizes.size(); dd++ ) {
               if( dd != processingDim ) {
                  ++position[ dd ];
                  for( dip::uint ii = 0; ii < nIn; ++ii ) {
                     inOffsets[ ii ] += in[ ii ].Stride( dd );
                  }
                  for( dip::uint ii = 0; ii < nOut; ++ii ) {
                     outOffsets[ ii ] += out[ ii ].Stride( dd );
                  }
                  // Check whether we reached the last pixel of the line
                  if( position[ dd ] != sizes[ dd ] ) {
                     break;
                  }
                  // Rewind along this dimension
                  for( dip::uint ii = 0; ii < nIn; ++ii ) {
                     inOffsets[ ii ] -= static_cast< dip::sint >( position[ dd ] ) * in[ ii ].Stride( dd );
                  }
                  for( dip::uint ii = 0; ii < nOut; ++ii ) {
                     outOffsets[ ii ] -= static_cast< dip::sint >( position[ dd ] ) * out[ ii ].Stride( dd );
                  }
                  position[ dd ] = 0;
                  // Continue loop to increment along next dimension
               }
            }
            if( dd == sizes.size() ) {
               break;            // We're done!
            }
         }
      }
   DIP_PARALLEL_ERROR_END

   // Correct output image properties
   for( dip::uint ii = 0; ii < nOut; ++ii ) {
      Image& tmp = c_out[ ii ].get();
      tmp.ExpandSingletonDimensions( trueSizes );
      if( tensorToSpatial && tmp.IsScalar() ) {
         tmp.ExpandSingletonTensor( trueTSize );
      }
      if( tensorToSpatial && !outTensor.IsScalar() ) {
         tmp.ReshapeTensor( outTensor );
      }
      tmp.SetPixelSize( pixelSize );
      if( !colspaces.empty() ) {
         tmp.SetColorSpace( colspaces[ colspaces.size() == 1 ? 0 : ii ] );
      }
   }
}

} // namespace Framework
} // namespace dip
