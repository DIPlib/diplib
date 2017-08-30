/*
 * DIPlib 3.0
 * This file contains definitions for the full framework.
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

#include <new>
#include <iostream>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

#ifdef _OPENMP
#include <omp.h>
#else
// We don't have OpenMP, this OpenMP function stub prevents conditional compilation within the `Full` function.
inline int omp_get_thread_num() { return 0; }
#endif

namespace dip {
namespace Framework {

void Full(
      Image const& c_in,
      Image& output,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint nTensorElements,
      BoundaryConditionArray boundaryConditions,
      Kernel const& kernel,
      FullLineFilter& lineFilter,
      FullOptions opts
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray sizes = c_in.Sizes();

   // Check inputs
   UnsignedArray kernelSizes;
   DIP_STACK_TRACE_THIS( kernelSizes = kernel.Sizes( sizes ));

   // Store these because they can get lost when ReForging `output` (it could be the same image as `c_in`)
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();

   // Determine output tensor shape
   Tensor outTensor( nTensorElements );
   bool expandTensor = false;
   bool asScalarImage = false;
   if( opts == Full_AsScalarImage ) {
      outTensor = c_in.Tensor();
      asScalarImage = true;
   } else {
      if(( opts == Full_ExpandTensorInBuffer ) && !c_in.Tensor().HasNormalOrder() ) {
         expandTensor = true;
         outTensor.SetMatrix( c_in.Tensor().Rows(), c_in.Tensor().Columns() );
         colorSpace.clear(); // the output tensor shape is different from the input's, the color space presumably doesn't match
      }
   }

   // Determine boundary sizes
   UnsignedArray boundary = kernelSizes;
   for( dip::uint& b : boundary ) {
      b /= 2;
   }
   IntegerArray shift = kernel.Shift();
   if( !shift.empty() ) {
      dip::uint n = std::min( shift.size(), boundary.size() );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         boundary[ ii ] += static_cast< dip::uint >( std::abs( shift[ ii ] ));
      }
   }

   // Copy input if necessary (this is the input buffer!)
   Image input;
   bool dataTypeChange = false;
   if( c_in.DataType() != inBufferType ) {
      dataTypeChange = true;
      input.SetDataType( inBufferType );
      input.Protect();
   }
   if( dataTypeChange || expandTensor || boundary.any() ) {
      Option::ExtendImage options = Option::ExtendImage_Masked;
      if( expandTensor ) {
         options += Option::ExtendImage_ExpandTensor;
      }
      DIP_STACK_TRACE_THIS( ExtendImageLowLevel( c_in, input, boundary, boundaryConditions, options ));
   } else {
      input = c_in.QuickCopy();
   }

   // Adjust output if necessary (and possible)
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
   DIP_START_STACK_TRACE
      if( output.IsOverlappingView( input ) ) {
         output.Strip();
      }
      output.ReForge( sizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      output.ReshapeTensor( outTensor );
      output.SetPixelSize( pixelSize );
      if( !colorSpace.empty() ) {
         output.SetColorSpace( colorSpace );
      }
   DIP_END_STACK_TRACE

   // Create a pixel table suitable to be applied to `input`
   dip::uint processingDim = OptimalProcessingDim( input, kernelSizes );
   PixelTable pixelTable;
   DIP_STACK_TRACE_THIS( pixelTable = kernel.PixelTable( sizes.size(), processingDim ));
   PixelTableOffsets pixelTableOffsets = pixelTable.Prepare( input );

   // Determine how many tensor elements to loop over: If the lineFilter wants to get the whole
   // tensor, we loop over only one tensor element. Otherwise we loop over each of the input
   // tensor elements (which will be the same number as output tensor elements).
   dip::uint nTElems = !asScalarImage ? 1 : input.TensorElements();

   // Do we need an output buffer?
   bool useOutBuffer = output.DataType() != outBufferType;

   // How many pixels in a line?
   dip::uint lineLength = input.Size( processingDim );

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   if( opts != Full_NoMultiThreading ) {
      // This is an estimate for the number of clock cycles we'll use
      // TODO: Query `lineFilter` for how much work it'll do
      dip::uint operations = input.NumberOfSamples() * pixelTable.NumberOfPixels();
      // Starting a new thread is only worth while if it'll use 10,000 clock cycles
      nThreads = clamp( operations / nClockCyclesPerThread, dip::uint( 1 ), GetNumberOfThreads() );
   }
   std::cout << "Starting " << nThreads << " threads\n";

   DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads ));

   // Start threads, each thread makes its own buffers
   #pragma omp parallel num_threads( nThreads )
   {
      // Create input buffer data struct
      FullBuffer inBuffer;
      inBuffer.tensorLength = asScalarImage ? 1 : input.TensorElements();
      inBuffer.tensorStride = input.TensorStride();
      inBuffer.stride = input.Stride( processingDim );
      inBuffer.buffer = nullptr;

      // Create output buffer data struct and allocate buffer if necessary
      std::vector< uint8 > outputBuffer;
      FullBuffer outBuffer;
      outBuffer.tensorLength = asScalarImage ? 1 : output.TensorElements();
      if( useOutBuffer ) {
         outBuffer.tensorStride = 1;
         outBuffer.stride = static_cast< dip::sint >( outBuffer.tensorLength );
         outputBuffer.resize( lineLength * outBufferType.SizeOf() * outBuffer.tensorLength );
         outBuffer.buffer = outputBuffer.data();
      } else {
         outBuffer.tensorStride = output.TensorStride();
         outBuffer.stride = output.Stride( processingDim );
         outBuffer.buffer = nullptr;
      }

      // Create data structure that works as input argument to the lineFilter
      FullLineFilterParameters fullLineFilterParameters{
            inBuffer, outBuffer, lineLength, processingDim, {}, pixelTableOffsets, static_cast< dip::uint >( omp_get_thread_num() )
      }; // Takes inBuffer, outBuffer, and pixelTableOffsets as references

      #pragma omp critical
      {
         std::cout << "Initiation, thread # " << fullLineFilterParameters.thread << std::endl;
         std::cout << "   bufferLength = " << fullLineFilterParameters.bufferLength << std::endl;
         std::cout << "   dimension = " << fullLineFilterParameters.dimension << std::endl;
         std::cout << "   inBuffer.buffer = " << fullLineFilterParameters.inBuffer.buffer << std::endl;
         std::cout << "   outBuffer.buffer = " << fullLineFilterParameters.outBuffer.buffer << std::endl;
         std::cout << "   &inBuffer = " << &(inBuffer) << std::endl;
         std::cout << "   &params.inBuffer = " << &(fullLineFilterParameters.inBuffer) << std::endl;
         std::cout << "   &outBuffer = " << &(outBuffer) << std::endl;
         std::cout << "   &params.outBuffer = " << &(fullLineFilterParameters.outBuffer) << std::endl;
      }

      // Loop over all image lines
      #pragma omp master
      {
         GenericJointImageIterator< 2 > it( { input, output }, processingDim );
         do {
            // Loop over all tensor components
            for( dip::uint ii = 0; ii < nTElems; ++ii ) {
               dip::sint inOffset = it.InOffset();
               dip::sint outOffset = it.OutOffset();
               UnsignedArray coords = it.Coordinates();
               #pragma omp task firstprivate( inOffset, outOffset, coords )
               {
                  fullLineFilterParameters.position = coords; // the coordinate array is copied twice, but this is better than copying the whole `it` object.
                  inBuffer.buffer = input.Pointer( inOffset + static_cast< dip::sint >( ii ) * inBuffer.tensorStride );
                  if( !useOutBuffer ) {
                     // Point output buffer to right line in output image
                     outBuffer.buffer = output.Pointer( outOffset + static_cast< dip::sint >( ii ) * outBuffer.tensorStride );
                  }
                  #pragma omp critical
                  {
                     std::cout << "Processing line, thread # " << fullLineFilterParameters.thread << std::endl;
                     std::cout << "   bufferLength = " << fullLineFilterParameters.bufferLength << std::endl;
                     std::cout << "   dimension = " << fullLineFilterParameters.dimension << std::endl;
                     std::cout << "   position = " << fullLineFilterParameters.position << std::endl;
                     std::cout << "   inBuffer.buffer = " << fullLineFilterParameters.inBuffer.buffer << std::endl;
                     std::cout << "   outBuffer.buffer = " << fullLineFilterParameters.outBuffer.buffer << std::endl;
                     std::cout << "   &inBuffer = " << &(inBuffer) << std::endl;
                     std::cout << "   &params.inBuffer = " << &(fullLineFilterParameters.inBuffer) << std::endl;
                     std::cout << "   &outBuffer = " << &(outBuffer) << std::endl;
                     std::cout << "   &params.outBuffer = " << &(fullLineFilterParameters.outBuffer) << std::endl;
                  }
                  // Filter the line
                  DIP_STACK_TRACE_THIS( lineFilter.Filter( fullLineFilterParameters ));
                  if( useOutBuffer ) {
                     // Copy output buffer to output image
                     detail::CopyBuffer(
                           outBuffer.buffer,
                           outBufferType,
                           outBuffer.stride,
                           outBuffer.tensorStride,
                           output.Pointer( outOffset + static_cast< dip::sint >( ii ) * output.TensorStride()),
                           output.DataType(),
                           output.Stride( processingDim ),
                           output.TensorStride(),
                           lineLength,
                           outBuffer.tensorLength );
                  }
               }
            }
         } while( ++it );
      }
      #pragma omp taskwait
   }
}

} // namespace Framework
} // namespace dip
