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
#include <diplib/iterators.h>

#include "diplib.h"
#include "diplib/framework.h"

#include "copy_buffer.h"

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
      PixelTable const& pixelTable,
      FullLineFilter& lineFilter,
      FullOptions opts
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray sizes = c_in.Sizes();
   dip::uint nDims = sizes.size();

   // Check inputs
   DIP_THROW_IF( pixelTable.Dimensionality() != nDims, "Pixel table dimensionality does not match image" );

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
   UnsignedArray boundary = pixelTable.Boundary();

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
      if( output.IsForged() && output.IsOverlappingView( input ) ) {
         output.Strip();
      }
      output.ReForge( sizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      output.ReshapeTensor( outTensor );
      output.SetPixelSize( pixelSize );
      if( !colorSpace.empty() ) {
         output.SetColorSpace( colorSpace );
      }
   DIP_END_STACK_TRACE

   // Determine the processing dimension.
   dip::uint processingDim = pixelTable.ProcessingDimension();
   dip::uint lineLength = input.Size( processingDim );

   // Create a pixel table suitable to be applied to `input`
   PixelTableOffsets pixelTableOffsets( pixelTable, input );

   // TODO: Determine the number of threads we'll be using. The size of the data
   //       has an influence. We can cut an image line in parts if necessary.
   //       I guess it would be useful to get an idea of the amount of work that
   //       the lineFilter does per pixel. If the caller can provide that estimate,
   //       we'd be able to use that to determine the threading schedule.

   lineFilter.SetNumberOfThreads( 1 );

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
   std::vector< uint8 > outputBuffer;
   bool useOutBuffer = false;
   if( output.DataType() != outBufferType ) {
      useOutBuffer = true;
   }
   FullBuffer outBuffer;
   outBuffer.tensorLength = asScalarImage ? 1 : output.TensorElements();
   if( useOutBuffer ) {
      outBuffer.tensorStride = 1;
      outBuffer.stride = outBuffer.tensorLength;
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
   GenericJointImageIterator it( input, output, processingDim );
   FullLineFilterParameters fullLineFilterParameters{ inBuffer, outBuffer, lineLength, processingDim, it.Coordinates(), pixelTableOffsets, thread }; // Takes inBuffer, outBuffer, it.Coordinates() as references
   do {
      // Loop over all tensor components
      for( dip::uint ii = 0; ii < nTElems; ++ii ) {
         inBuffer.buffer = input.Pointer( it.InOffset() + ii * inBuffer.tensorStride );
         if( !useOutBuffer ) {
            // Point output buffer to right line in output image
            outBuffer.buffer = output.Pointer( it.OutOffset() + ii * outBuffer.tensorStride );
         }
         // Filter the line
         lineFilter.Filter( fullLineFilterParameters );
         if( useOutBuffer ) {
            // Copy output buffer to output image
            CopyBuffer(
                  outBuffer.buffer,
                  outBufferType,
                  outBuffer.stride,
                  outBuffer.tensorStride,
                  output.Pointer( it.OutOffset() + ii * output.TensorStride() ),
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
