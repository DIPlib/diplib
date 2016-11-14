/*
 * DIPlib 3.0
 * This file contains definitions for the separable framework.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <new>
#include <iostream>

#include "diplib.h"
#include "diplib/framework.h"

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
      SeparableFilter lineFilter,
      void const* functionParameters,
      std::vector< void* > const& functionVariables,
      SeparableOptions opts
) {
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();

   // Check inputs
   if( process.empty() ) {
      // An empty process array means all dimensions are to be processed
      process.resize( nDims, true );
   } else {
      dip_ThrowIf( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
      if( !process.any() ) {
         // No dimensions to process
         c_out = c_in; // This ignores the Separable_DontResizeOutput option...
         return;
      }
   }
   border = ArrayUseParameter( border, nDims, dip::uint( 0 ));
   boundaryConditions = BoundaryArrayUseParameter( boundaryConditions, nDims );

   // Make simplified copy of input image headers so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();

   // Determine output sizes
   UnsignedArray outSizes;
   if( opts == Separable_DontResizeOutput ) {
      outSizes = c_out.Sizes();
      dip_ThrowIf( outSizes.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
      for( size_t ii = 0; ii < nDims; ++ii ) {
         dip_ThrowIf( !process[ ii ] && ( inSizes[ ii ] != outSizes[ ii ] ), "Output size must match input size for dimensions not being processed" );
      }
   } else {
      outSizes = inSizes;
   }

   // `lookUpTable` is the look-up table for `in`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< dip::sint > lookUpTable;

   // Determine number of tensor elements and do tensor to spatial dimension if necessary
   Tensor outTensor = in.Tensor();
   bool tensorToSpatial = false;
   if( opts == Separable_AsScalarImage ) {
      if( !in.IsScalar() ) {
         in.TensorToSpatial( 0 );
         process.insert( 0, false );
         tensorToSpatial = true;
         ++nDims;
         inSizes = in.Sizes();
      }
   } else {
      if(( opts == Separable_ExpandTensorInBuffer ) && !in.Tensor().HasNormalOrder() ) {
         lookUpTable = in.Tensor().LookUpTable();
         outTensor.SetMatrix( in.Tensor().Rows(), in.Tensor().Columns() );
         colorSpace.clear(); // the output tensor shape is different from the input's, the color space presumably doesn't match
      }
   }

   // Adjust output if necessary (and possible)
   if( c_out.IsForged() && c_out.IsOverlappingView( c_in ) ) {
      c_out.Strip();
   }
   c_out.ReForge( outSizes, outTensor.Elements(), outImageType );
   c_out.ReshapeTensor( outTensor );
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!

   // Make simplified copies of output image headers so we can modify them at will
   Image out = c_out.QuickCopy();

   // Do tensor to spatial dimension if necessary
   if( tensorToSpatial ) {
      out.TensorToSpatial( 0 );
      outSizes = out.Sizes();
   }

   // Determine the order in which dimensions are to be processed.
   // The first dimension to process is the one that most reduces the size of the intermediate image.
   // The last one is the one that most expands the intermediate image.
   FloatArray grow( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         grow[ ii ] = static_cast< double >( outSizes[ ii ] ) / static_cast< double >( inSizes[ ii ] );
      } else {
         grow[ ii ] = 0; // this dimension will not be processed, the order arraw will index it first
      }
   }
   UnsignedArray order = grow.sortedIndices();
   while(( order.size() > 0 ) && ( order[ 0 ] == 0 )) {
      order.erase( 0 );
   }
   // `order` now indexes the dimensions to be processed, in the optimal order (to reduce the amount of
   // computation and intermediate storage)

   // Create a temporary image, if needed, that can hold the result of all the steps minus the last one
   // (which will be written directly in the output data segment).
   // TODO: It might be better to have a different temp image for each step, and flip dimensions so
   // TODO:   that each step reads a line that is consecutive in memory, and writes it out so that
   // TODO:   the next dimension to process is consecutive. (x,y) -> (y,x) -> (x,y) ; (x,y,z) -> (y,z,x) -> (z,x,y) -> (x,y,z)
   Image tmp;
   if( order.size() > 1 ) { // if not, we won't ever user tmp
      UnsignedArray tmpSizes( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         tmpSizes[ ii ] = std::max( inSizes[ ii ], outSizes[ ii ] );
      }
      tmpSizes[ order[ 0 ] ] = outSizes[ order[ 0 ] ];
      tmpSizes[ order[ nDims - 1 ] ] = inSizes[ order[ nDims - 1 ] ];
      if( ( tmpSizes.allSmallerOrEqual( outSizes ) ) && ( bufferType == outImageType ) ) {
         tmp = out;  // we can use the data segment of out for intermediate data
      } else {
         tmp = Image( tmpSizes, tensorToSpatial ? 1 : outTensor.Elements(), bufferType );
      }
   }

   // TODO: Determine the number of threads we'll be using.

   bool useFunctionVariables = functionVariables.size() > 0;

   // TODO: Start threads, each thread makes its own buffers.
   dip::uint thread = 0;

   // The temporary buffers, if needed, will be stored here
   std::vector< uint8 > inBufferStorage;
   std::vector< uint8 > outBufferStorage;

   // TODO: iterate over dimensions
   {
      dip::uint processingDim = 0;
      // in the first iteration: input data = in
      // in the last iteration: output data = out
      // at other times: input data = tmp, output buffer = tmp
      UnsignedArray sizes = inSizes; // TODO: This array will change during each iteration to match the input size

      dip::uint inLength = inSizes[ processingDim ];
      dip::uint inBorder = border[ processingDim ];
      dip::uint outLength = outSizes[ processingDim ];
      dip::uint outBorder = opts == Separable_UseOutBorder ? inBorder : 0;

      // Determine if we need to make a temporary buffer for this dimension
      bool inUseBuffer = ( in.DataType() != bufferType ) || !lookUpTable.empty() || ( inBorder > 0 );
      bool outUseBuffer = ( out.DataType() != bufferType ) || ( outBorder > 0 );

      // Create buffer data structs and (re-)allocate buffers
      SeparableBuffer inSeparableBuf;
      inSeparableBuf.length = inLength;
      inSeparableBuf.border = inBorder;
      if( inUseBuffer ) {
         if( lookUpTable.empty() ) {
            inSeparableBuf.tensorLength = in.TensorElements();
         } else {
            inSeparableBuf.tensorLength = lookUpTable.size();
         }
         inSeparableBuf.tensorStride = 1;
         if( in.Stride( processingDim ) == 0 ) {
            // A stride of 0 means all pixels are the same, allocate space for a single pixel
            inSeparableBuf.stride = 0;
            inBufferStorage.resize( bufferType.SizeOf() * inSeparableBuf.tensorLength );
         } else {
            inSeparableBuf.stride = inSeparableBuf.tensorLength;
            inBufferStorage.resize( ( inLength + 2 * inBorder ) * bufferType.SizeOf() * inSeparableBuf.tensorLength );
         }
         inSeparableBuf.buffer = inBufferStorage.data();
      } else {
         inSeparableBuf.tensorLength = in.TensorElements();
         inSeparableBuf.tensorStride = in.TensorStride();
         inSeparableBuf.stride = in.Stride( processingDim );
         inSeparableBuf.buffer = nullptr;
      }
      SeparableBuffer outSeparableBuf;
      outSeparableBuf.length = outLength;
      outSeparableBuf.border = outBorder;
      if( outUseBuffer ) {
         outSeparableBuf.tensorLength = out.TensorElements();
         outSeparableBuf.tensorStride = 1;
         outSeparableBuf.stride = outSeparableBuf.tensorLength;
         outBufferStorage.resize( ( outLength + 2 * outBorder ) * bufferType.SizeOf() * outSeparableBuf.tensorLength );
         outSeparableBuf.buffer = outBufferStorage.data();
      } else {
         outSeparableBuf.tensorLength = out.TensorElements();
         outSeparableBuf.tensorStride = out.TensorStride();
         outSeparableBuf.stride = out.Stride( processingDim );
         outSeparableBuf.buffer = nullptr;
      }

      // Iterate over lines in the image
      // TODO: use the GenericJointImageIterator here.
      //std::cout << "dip::Framework::Separable -- running\n";
      UnsignedArray position( sizes.size(), 0 );
      dip::sint inIndex = 0;
      dip::sint outIndex = 0;
      for( ;; ) {

         // Get pointers to input and ouput lines
         //std::cout << "      sectionStart = " << sectionStart << std::endl;
         //std::cout << "      inIndex[" << ii << "] = " << inIndex[ii] << std::endl;
         if( inUseBuffer ) {
            // If inIndex[ii] is the same as in the previous iteration, we don't need
            // to copy the buffer over again. This happens with singleton-expanded input images.
            // But it's easier to copy, and also safer as the lineFilter function could be bad and write in its input!
            CopyBuffer(
                  in.Pointer( inIndex ),
                  in.DataType(),
                  in.Stride( processingDim ),
                  in.TensorStride(),
                  inSeparableBuf.buffer,
                  bufferType,
                  inSeparableBuf.stride,
                  inSeparableBuf.tensorStride,
                  inSeparableBuf.stride == 0 ? 1 : inLength, // if stride == 0, copy only a single pixel because they're all the same
                  inSeparableBuf.tensorLength,
                  lookUpTable );
            if(( inBorder > 0 ) && ( inSeparableBuf.stride != 0 )) {
               ExpandBuffer(
                     inSeparableBuf.buffer,
                     bufferType,
                     inSeparableBuf.stride,
                     inSeparableBuf.tensorStride,
                     inLength,
                     inSeparableBuf.tensorLength,
                     inBorder,
                     boundaryConditions[ processingDim ] );
            }
         } else {
            inSeparableBuf.buffer = in.Pointer( inIndex );
         }
         //std::cout << "      outIndex[" << ii << "] = " << outIndex[ii] << std::endl;
         if( !outUseBuffer ) {
            outSeparableBuf.buffer = out.Pointer( outIndex );
         }

         // Filter the line
         lineFilter(
               inSeparableBuf,
               outSeparableBuf,
               processingDim,
               position,
               functionParameters,
               useFunctionVariables ? functionVariables[ thread ] : nullptr
         );

         // Copy back the line from output buffer to the image
         if( outUseBuffer ) {
            CopyBuffer(
                  outSeparableBuf.buffer,
                  bufferType,
                  outSeparableBuf.stride,
                  outSeparableBuf.tensorStride,
                  out.Pointer( outIndex ),
                  out.DataType(),
                  out.Stride( processingDim ),
                  out.TensorStride(),
                  outLength,
                  outSeparableBuf.tensorLength,
                  std::vector< dip::sint > {} );
         }

         // Determine which line to process next until we're done
         position[ processingDim ] = 0; // reset this index
         dip::uint dd;
         for( dd = 0; dd < sizes.size(); dd++ ) {
            if( dd != processingDim ) {
               ++position[ dd ];
               inIndex += in.Stride( dd );
               outIndex += out.Stride( dd );
               // Check whether we reached the last pixel of the line
               if( position[ dd ] != sizes[ dd ] ) {
                  break;
               }
               // Rewind along this dimension
               inIndex -= position[ dd ] * in.Stride( dd );
               outIndex -= position[ dd ] * out.Stride( dd );
               position[ dd ] = 0;
               // Continue loop to increment along next dimension
            }
         }
         if( dd == sizes.size() ) {
            break;            // We're done!
         }
      }
      // Clear the tensor look-up table: if this was set, then the intermediate data now has a full matrix as tensor shape and we don't need it any more.
      lookUpTable.clear();
   }

   // TODO: End threads.

   c_out.SetPixelSize( pixelSize );
   c_out.SetColorSpace( colorSpace );
}

} // namespace Framework
} // namespace dip
