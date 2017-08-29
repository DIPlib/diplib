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
   DIP_START_STACK_TRACE
      kernelSizes = kernel.Sizes( sizes );
   DIP_END_STACK_TRACE

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
      DIP_START_STACK_TRACE
         ExtendImageLowLevel( c_in, input, boundary, boundaryConditions, options );
      DIP_END_STACK_TRACE
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
   DIP_START_STACK_TRACE
      pixelTable = kernel.PixelTable( sizes.size(), processingDim );
   DIP_END_STACK_TRACE
   PixelTableOffsets pixelTableOffsets = pixelTable.Prepare( input );

   // TODO: Determine the number of threads we'll be using.
   // Don't forget to check for opts==Full_NoMultiThreading!

   DIP_START_STACK_TRACE
      lineFilter.SetNumberOfThreads( 1 );
   DIP_END_STACK_TRACE

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // Create input buffer data struct
   FullBuffer inBuffer;
   dip::uint nTElems = input.TensorElements();
   inBuffer.tensorLength = asScalarImage ? 1 : nTElems;
   inBuffer.tensorStride = input.TensorStride();
   inBuffer.stride = input.Stride( processingDim );
   inBuffer.buffer = nullptr;

   // Create output buffer data struct and allocate buffer if necessary
   dip::uint lineLength = input.Size( processingDim );
   std::vector< uint8 > outputBuffer;
   bool useOutBuffer = false;
   if( output.DataType() != outBufferType ) {
      useOutBuffer = true;
   }
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

   // Determine how many tensor elements to loop over: If the lineFilter wants to get the whole
   // tensor, we loop over only one tensor element. Otherwise we loop over each of the input
   // tensor elements (which will be the same number as output tensor elements).
   if( !asScalarImage ) {
      nTElems = 1;
   }

   // Loop over all image lines
   GenericJointImageIterator< 2 > it( { input, output }, processingDim );
   FullLineFilterParameters fullLineFilterParameters{
         inBuffer, outBuffer, lineLength, processingDim, it.Coordinates(), pixelTableOffsets, thread
   }; // Takes inBuffer, outBuffer, it.Coordinates() as references
   do {
      // Loop over all tensor components
      for( dip::uint ii = 0; ii < nTElems; ++ii ) {
         inBuffer.buffer = input.Pointer( it.InOffset() + static_cast< dip::sint >( ii ) * inBuffer.tensorStride );
         if( !useOutBuffer ) {
            // Point output buffer to right line in output image
            outBuffer.buffer = output.Pointer( it.OutOffset() + static_cast< dip::sint >( ii ) * outBuffer.tensorStride );
         }
         // Filter the line
         DIP_START_STACK_TRACE
            lineFilter.Filter( fullLineFilterParameters );
         DIP_END_STACK_TRACE
         if( useOutBuffer ) {
            // Copy output buffer to output image
            detail::CopyBuffer(
                  outBuffer.buffer,
                  outBufferType,
                  outBuffer.stride,
                  outBuffer.tensorStride,
                  output.Pointer( it.OutOffset() + static_cast< dip::sint >( ii ) * output.TensorStride() ),
                  output.DataType(),
                  output.Stride( processingDim ),
                  output.TensorStride(),
                  lineLength,
                  outBuffer.tensorLength );
         }
      }
   } while( ++it );

   // TODO: End threads.
}

} // namespace Framework
} // namespace dip
