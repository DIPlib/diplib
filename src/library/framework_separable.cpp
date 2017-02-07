/*
 * DIPlib 3.0
 * This file contains definitions for the separable framework.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <new>
#include <iostream>
#include <cmath>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"

#include "copy_buffer.h"

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
      SeparableLineFilter* lineFilter,
      SeparableOptions opts
) {
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();

   // Check inputs
   if( process.empty() ) {
      // An empty process array means all dimensions are to be processed
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
      if( !process.any() ) {
         // No dimensions to process
         c_out = c_in; // This ignores the Separable_DontResizeOutput option...
         return;
      }
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

   // `lookUpTable` is the look-up table for `in`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< dip::sint > lookUpTable;

   // Determine number of tensor elements and do tensor to spatial dimension if necessary
   Tensor outTensor = input.Tensor();
   bool tensorToSpatial = false;
   if( opts == Separable_AsScalarImage ) {
      if( !input.IsScalar() ) {
         input.TensorToSpatial( 0 );
         process.insert( 0, false );
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

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
   if( c_out.IsForged() && c_out.IsOverlappingView( c_in ) ) {
      c_out.Strip();
   }
   c_out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
   c_out.ReshapeTensor( outTensor );
   DIP_END_STACK_TRACE
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!

   // Make simplified copies of output image headers so we can modify them at will
   Image output = c_out.QuickCopy();

   // Do tensor to spatial dimension if necessary
   if( tensorToSpatial ) {
      output.TensorToSpatial( 0 );
      outSizes = output.Sizes();
   }

   // Determine the order in which dimensions are to be processed.
   // The first dimension to process is the one that most reduces the size of the intermediate image.
   // The last one is the one that most expands the intermediate image.
   FloatArray grow( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         grow[ ii ] = static_cast< dfloat >( outSizes[ ii ] ) / static_cast< dfloat >( inSizes[ ii ] );
      } else {
         grow[ ii ] = std::numeric_limits< dfloat >::infinity(); // this dimension will not be processed, the order array will index it last
      }
   }
   UnsignedArray order = grow.sortedIndices();
   while(( order.size() > 0 ) && ( std::isinf( grow[ order.back() ] ))) {
      order.pop_back();
   }
   // `order` now indexes the dimensions to be processed, in the optimal order (to reduce the amount of
   // computation and intermediate storage)
   // TODO: for dimensions with equal 'grow' weight (more often than not, grow == 1 for all dimensions), sort them by stride, with lower stride first.

   // TODO: Determine the number of threads we'll be using.

   lineFilter->SetNumberOfThreads( 1 );

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // The temporary buffers, if needed, will be stored here (each process their own!)
   std::vector< uint8 > inBufferStorage;
   std::vector< uint8 > outBufferStorage;

   // The intermediate image, if needed, stored here
   Image intermediate;

   // Iterate over the dimensions to be processed. This loop should be sequential, not parallelized!
   for( dip::uint processingDim : order ) {
      // Where do we read and write data?
      Image& inImage = order.front() == processingDim ? input : intermediate;
      Image tmpImage;
      Image& outImage = order.back() == processingDim ? output : tmpImage;

      // Allocate space for intermediate data
      if( &outImage == &tmpImage ) {
         UnsignedArray tmpSizes = inImage.Sizes();
         tmpSizes[ processingDim ] = outSizes[ processingDim ];
         tmpImage.SetSizes( tmpSizes );
         tmpImage.SetTensorSizes( outImage.TensorElements() );
         tmpImage.SetDataType( bufferType );
         // TODO: Set strides such that the next dimension has a stride of 1 (or rather TensorElements)
         // As in: (x,y) -> (y,x) -> (x,y) ; (x,y,z) -> (y,z,x) -> (z,x,y) -> (x,y,z)
         tmpImage.Forge();
      }

      //std::cout << "dip::Framework::Separable(), processingDim = " << processingDim << std::endl;
      //std::cout << "   inImage.Origin() = " << inImage.Origin() << std::endl;
      //std::cout << "   outImage.Origin() = " << outImage.Origin() << std::endl;

      // Some values to use during this iteration
      dip::uint inLength = inSizes[ processingDim ]; DIP_ASSERT( inLength == inImage.Size( processingDim ) );
      dip::uint inBorder = border[ processingDim ];
      dip::uint outLength = outSizes[ processingDim ];
      dip::uint outBorder = opts == Separable_UseOutBorder ? inBorder : 0;

      // Determine if we need to make a temporary buffer for this dimension
      bool inUseBuffer = ( inImage.DataType() != bufferType ) || !lookUpTable.empty() || ( inBorder > 0 );
      bool outUseBuffer = ( outImage.DataType() != bufferType ) || ( outBorder > 0 );

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
            inBuffer.stride = inBuffer.tensorLength;
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
      if( outUseBuffer ) {
         outBuffer.tensorLength = outImage.TensorElements();
         outBuffer.tensorStride = 1;
         outBuffer.stride = outBuffer.tensorLength;
         outBufferStorage.resize( ( outLength + 2 * outBorder ) * bufferType.SizeOf() * outBuffer.tensorLength );
         outBuffer.buffer = outBufferStorage.data() + outBorder * bufferType.SizeOf() * outBuffer.tensorLength;
         //std::cout << "   Using output buffer, size = " << outBufferStorage.size() << std::endl;
      } else {
         outBuffer.tensorLength = outImage.TensorElements();
         outBuffer.tensorStride = outImage.TensorStride();
         outBuffer.stride = outImage.Stride( processingDim );
         outBuffer.buffer = nullptr;
         //std::cout << "   Not using output buffer\n";
      }

      // Iterate over all lines in the image
      auto it = dip::GenericJointImageIterator( inImage, outImage, processingDim );
      SeparableLineFilterParameters separableLineFilterParams{ inBuffer, outBuffer, processingDim, it.Coordinates(), thread }; // Takes inBuffer, outBuffer, it.Coordinates() as references
      do {
         // Get pointers to input and ouput lines
         if( inUseBuffer ) {
            // If inIndex[ii] is the same as in the previous iteration, we don't need
            // to copy the buffer over again. This happens with singleton-expanded input images.
            // But it's easier to copy, and also safer as the lineFilter function could be bad and write in its input!
            CopyBuffer(
                  it.InPointer(),
                  inImage.DataType(),
                  inImage.Stride( processingDim ),
                  inImage.TensorStride(),
                  inBuffer.buffer,
                  bufferType,
                  inBuffer.stride,
                  inBuffer.tensorStride,
                  inBuffer.stride == 0 ? 1 : inLength, // if stride == 0, copy only a single pixel because they're all the same
                  inBuffer.tensorLength,
                  lookUpTable );
            if(( inBorder > 0 ) && ( inBuffer.stride != 0 )) {
               ExpandBuffer(
                     inBuffer.buffer,
                     bufferType,
                     inBuffer.stride,
                     inBuffer.tensorStride,
                     inLength,
                     inBuffer.tensorLength,
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
         lineFilter->Filter( separableLineFilterParams );

         // Copy back the line from output buffer to the image
         if( outUseBuffer ) {
            CopyBuffer(
                  outBuffer.buffer,
                  bufferType,
                  outBuffer.stride,
                  outBuffer.tensorStride,
                  it.OutPointer(),
                  outImage.DataType(),
                  outImage.Stride( processingDim ),
                  outImage.TensorStride(),
                  outLength,
                  outBuffer.tensorLength,
                  std::vector< dip::sint > {} );
         }
      } while( ++it );

      // Clear the tensor look-up table: if this was set, then the intermediate data now has a full matrix as tensor shape and we don't need it any more.
      lookUpTable.clear();
      // Save the temporary image, if we used it.
      intermediate = std::move( tmpImage );
   }

   // TODO: End threads.

   c_out.SetPixelSize( pixelSize );
   c_out.SetColorSpace( colorSpace );
}

} // namespace Framework
} // namespace dip
