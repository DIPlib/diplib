/*
 * DIPlib 3.0
 * This file contains definitions for the separable framework.
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
#include <cmath>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

namespace dip {
namespace Framework {

void Separable(
      Image const& c_in,
      Image& c_out,
      DataType bufferType,
      DataType outImageType,
      BooleanArray process,   // taken by copy so we can modify
      UnsignedArray border,   // taken by copy so we can modify
      BoundaryConditionArray boundaryConditions,   // taken by copy so we can modify
      SeparableLineFilter& lineFilter,
      SeparableOptions opts
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();

   // Check inputs
   if( process.empty() ) {
      // An empty process array means all dimensions are to be processed
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }
   DIP_START_STACK_TRACE
      ArrayUseParameter( border, nDims, dip::uint( 0 ));
      BoundaryArrayUseParameter( boundaryConditions, nDims );
   DIP_END_STACK_TRACE

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image input = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();

   // Determine output sizes
   UnsignedArray outSizes;
   if( opts == Separable_DontResizeOutput ) {
      outSizes = c_out.Sizes();
      DIP_THROW_IF( outSizes.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
      for( size_t ii = 0; ii < nDims; ++ii ) {
         DIP_THROW_IF( !process[ ii ] && ( inSizes[ ii ] != outSizes[ ii ] ), "Output size must match input size for dimensions not being processed" );
      }
   } else {
      outSizes = inSizes;
   }

   // Reset `process` for dimensions with size==1
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( inSizes[ ii ] == 1 ) && ( outSizes[ ii ] == 1 )) {
         process[ ii ] = false;
      }
   }

   // `lookUpTable` is the look-up table for `in`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< dip::sint > lookUpTable;

   // Determine number of tensor elements and do tensor to spatial dimension if necessary
   Tensor outTensor = input.Tensor();
   bool tensorToSpatial = false;
   if( opts == Separable_AsScalarImage ) {
      if( !input.IsScalar() ) {
         input.TensorToSpatial();
         process.push_back( false );
         border.push_back( 0 );
         tensorToSpatial = true;
         ++nDims;
         inSizes = input.Sizes();
      }
   } else {
      if(( opts == Separable_ExpandTensorInBuffer ) && !input.Tensor().HasNormalOrder() ) {
         lookUpTable = input.Tensor().LookUpTable();
         outTensor.SetMatrix( input.Tensor().Rows(), input.Tensor().Columns() );
         colorSpace.clear(); // the output tensor shape is different from the input's, the color space presumably doesn't match
      }
   }

   //std::cout << "Input image: " << c_in << std::endl;

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      if( c_out.IsOverlappingView( input ) ) {
         c_out.Strip();
      }
      c_out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
      c_out.ReshapeTensor( outTensor );
      c_out.SetPixelSize( pixelSize );
      if( !colorSpace.empty() ) {
         c_out.SetColorSpace( colorSpace );
      }
   DIP_END_STACK_TRACE

   //std::cout << "Output image: " << c_out << std::endl;

   // Make simplified copies of output image headers so we can modify them at will
   Image output = c_out.QuickCopy();

   // Do tensor to spatial dimension if necessary
   if( tensorToSpatial ) {
      output.TensorToSpatial();
      outSizes = output.Sizes();
   }

   // Determine the order in which dimensions are to be processed.
   //
   // Step 1: create a list of dimension numbers that we'll process.
   UnsignedArray order( nDims );
   { // braces around this code to limit the lifetime of `jj`
      dip::uint jj = 0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( process[ ii ] ) {
            order[ jj ] = ii;
            ++jj;
         }
      }
      if( jj == 0 ) {
         // No dimensions to process.
         output.Copy( input ); // This should always work, as dimensions where the sizes don't match will be processed.
         return;
      }
      order.resize( jj );
   }
   // Step 2: sort the list of dimensions so that the smallest stride comes first
   sortIndices( order, input.Strides() );
   // Step 3: sort the list of dimensions again, so that the dimension that reduces the size of the image
   // the most is processed first.
   if ( opts == Separable_DontResizeOutput ) { // else: all `grow` is 1.
      FloatArray grow( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         grow[ ii ] = static_cast< dfloat >( outSizes[ ii ] ) / static_cast< dfloat >( inSizes[ ii ] );
      }
      sortIndices( order, grow );
   }

   // Processing:
   //  if flipDims [ not used any more ]
   //       input -> temp1 -> temp2 -> temp3 -> ... -> output
   //       - each image tempN has a different dimension with stride==1
   //       - at the end of each pass, we move the tempN image to intermediate
   //       - all but first pass read from intermediate, all but last pass write to a new tempN
   //       - this is actually slower on my computer except with very large 2D images, where the difference is not significant
   // else if useIntermediate
   //       input -> intermediate -> intermediate -> ... -> output
   //       - the intermediate image should be allocated only once
   //       - all but first pass read from intermediate, all but last pass write to intermediate
   //  else
   //       input -> output -> output -> output -> ... -> output
   //       - all but first pass read from output, all passes write in output
   //       - we can do this because output.DataType() == bufferType, so no precision is lost

   // The intermediate image, if needed, stored here
   Image intermediate;
   bool useIntermediate = output.DataType() != bufferType;
   UnsignedArray intermSizes = outSizes;
   for( dip::uint ii = 1; ii < order.size(); ++ii ) { // not using the 1st dimension to be processed
      dip::uint kk = order[ ii ];
      if( inSizes[ kk ] > outSizes[ kk ] ) {
         intermSizes[ kk ] = inSizes[ kk ];
         useIntermediate = true;
      }
   }
   if( useIntermediate ) {
      intermediate.CopyProperties( output );
      intermediate.SetDataType( bufferType );
      intermediate.SetSizes( intermSizes );
      intermediate.Forge();
   }

   // TODO: Determine the number of threads we'll be using.
   // Don't forget to check for opts==Separable_NoMultiThreading!

   DIP_START_STACK_TRACE
      lineFilter.SetNumberOfThreads( 1 );
   DIP_END_STACK_TRACE

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // The temporary buffers, if needed, will be stored here (each process their own!)
   std::vector< uint8 > inBufferStorage;
   std::vector< uint8 > outBufferStorage;

   // Iterate over the dimensions to be processed. This loop should be sequential, not parallelized!
   Image outImage;
   for( dip::uint rep = 0; rep < order.size(); ++rep ) {
      dip::uint processingDim = order[ rep ];
      // First step always reads from input, other steps read from outImage, which is either intermediate or output
      Image inImage = (( rep == 0 ) ? ( input ) : ( outImage )).QuickCopy();
      // Last step always writes to output, other steps write to intermediate or output
      UnsignedArray sizes = inImage.Sizes();
      outImage = (( rep == order.size() - 1 ) ? ( output ) : ( useIntermediate ? intermediate : output )).QuickCopy();
      sizes[ processingDim ] = outSizes[ processingDim ];
      outImage.dip__SetSizes( sizes );

      //std::cout << "dip::Framework::Separable(), processingDim = " << processingDim << std::endl;
      //std::cout << "   inImage.Origin() = " << inImage.Origin() << std::endl;
      //std::cout << "   inImage.Sizes() = " << inImage.Sizes() << std::endl;
      //std::cout << "   inImage.Strides() = " << inImage.Strides() << std::endl;
      //std::cout << "   outImage.Origin() = " << outImage.Origin() << std::endl;
      //std::cout << "   outImage.Sizes() = " << outImage.Sizes() << std::endl;
      //std::cout << "   outImage.Strides() = " << outImage.Strides() << std::endl;

      // Some values to use during this iteration
      dip::uint inLength = inSizes[ processingDim ]; DIP_ASSERT( inLength == inImage.Size( processingDim ) );
      dip::uint inBorder = border[ processingDim ];
      dip::uint outLength = outSizes[ processingDim ];
      dip::uint outBorder = opts == Separable_UseOutputBorder ? inBorder : 0;

      // Determine if we need to make a temporary buffer for this dimension
      bool inUseBuffer = ( inImage.DataType() != bufferType ) || !lookUpTable.empty() || ( inBorder > 0 ) || ( opts == Separable_UseInputBuffer );
      bool outUseBuffer = ( outImage.DataType() != bufferType ) || ( outBorder > 0 ) || ( opts == Separable_UseOutputBuffer );
      if( !inUseBuffer && !outUseBuffer && ( inImage.Origin() == outImage.Origin() )) {
         // If input and output images are the same, we need to use at least one buffer!
         inUseBuffer = true;
      }

      // Create buffer data structs and (re-)allocate buffers
      SeparableBuffer inBuffer;
      inBuffer.length = inLength;
      inBuffer.border = inBorder;
      if( inUseBuffer ) {
         if( lookUpTable.empty() ) {
            inBuffer.tensorLength = inImage.TensorElements();
         } else {
            inBuffer.tensorLength = lookUpTable.size();
         }
         inBuffer.tensorStride = 1;
         if( inImage.Stride( processingDim ) == 0 ) {
            // A stride of 0 means all pixels are the same, allocate space for a single pixel
            inBuffer.stride = 0;
            inBufferStorage.resize( bufferType.SizeOf() * inBuffer.tensorLength );
            //std::cout << "   Using input buffer, stride = 0\n";
         } else {
            inBuffer.stride = static_cast< dip::sint >( inBuffer.tensorLength );
            inBufferStorage.resize( ( inLength + 2 * inBorder ) * bufferType.SizeOf() * inBuffer.tensorLength );
            //std::cout << "   Using input buffer, size = " << inBufferStorage.size() << std::endl;
         }
         inBuffer.buffer = inBufferStorage.data() + inBorder * bufferType.SizeOf() * inBuffer.tensorLength;
      } else {
         inBuffer.tensorLength = inImage.TensorElements();
         inBuffer.tensorStride = inImage.TensorStride();
         inBuffer.stride = inImage.Stride( processingDim );
         inBuffer.buffer = nullptr;
         //std::cout << "   Not using input buffer\n";
      }
      SeparableBuffer outBuffer;
      outBuffer.length = outLength;
      outBuffer.border = outBorder;
      outBuffer.tensorLength = outImage.TensorElements();
      if( outUseBuffer ) {
         outBuffer.tensorStride = 1;
         outBuffer.stride = static_cast< dip::sint >( outBuffer.tensorLength );
         outBufferStorage.resize( ( outLength + 2 * outBorder ) * bufferType.SizeOf() * outBuffer.tensorLength );
         outBuffer.buffer = outBufferStorage.data() + outBorder * bufferType.SizeOf() * outBuffer.tensorLength;
         //std::cout << "   Using output buffer, size = " << outBufferStorage.size() << std::endl;
      } else {
         outBuffer.tensorStride = outImage.TensorStride();
         outBuffer.stride = outImage.Stride( processingDim );
         outBuffer.buffer = nullptr;
         //std::cout << "   Not using output buffer\n";
      }

      // Iterate over all lines in the image. This loop to be parallelized.
      GenericJointImageIterator< 2 > it( { inImage, outImage }, processingDim );
      SeparableLineFilterParameters separableLineFilterParams{
            inBuffer, outBuffer, processingDim, rep, order.size(), it.Coordinates(), tensorToSpatial, thread
      }; // Takes inBuffer, outBuffer, it.Coordinates() as references
      do {
         // Get pointers to input and ouput lines
         if( inUseBuffer ) {
            detail::CopyBuffer(
                  it.InPointer(),
                  inImage.DataType(),
                  inImage.Stride( processingDim ),
                  inImage.TensorStride(),
                  inBuffer.buffer,
                  bufferType,
                  inBuffer.stride,
                  inBuffer.tensorStride,
                  inLength, // if stride == 0, only a single pixel will be copied, because they're all the same
                  inBuffer.tensorLength,
                  lookUpTable );
            if(( inBorder > 0 ) && ( inBuffer.stride != 0 )) {
               detail::ExpandBuffer(
                     inBuffer.buffer,
                     bufferType,
                     inBuffer.stride,
                     inBuffer.tensorStride,
                     inLength,
                     inBuffer.tensorLength,
                     inBorder,
                     inBorder,
                     boundaryConditions[ processingDim ] );
            }
         } else {
            inBuffer.buffer = it.InPointer();
         }
         if( !outUseBuffer ) {
            outBuffer.buffer = it.OutPointer();
         }

         // Filter the line
         DIP_START_STACK_TRACE
            lineFilter.Filter( separableLineFilterParams );
         DIP_END_STACK_TRACE

         // Copy back the line from output buffer to the image
         if( outUseBuffer ) {
            detail::CopyBuffer(
                  outBuffer.buffer,
                  bufferType,
                  outBuffer.stride,
                  outBuffer.tensorStride,
                  it.OutPointer(),
                  outImage.DataType(),
                  outImage.Stride( processingDim ),
                  outImage.TensorStride(),
                  outLength,
                  outBuffer.tensorLength );
         }
      } while( ++it );

      // Clear the tensor look-up table: if this was set, then the intermediate data now has a full matrix as tensor shape and we don't need it any more.
      lookUpTable.clear();
   }

   // TODO: End threads.
}

} // namespace Framework
} // namespace dip
