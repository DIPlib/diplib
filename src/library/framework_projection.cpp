/*
 * (c)2017-2024, Cris Luengo.
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

#include <utility>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"

namespace dip {
namespace Framework {

void Projection(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      DataType outImageType,
      BooleanArray process,   // taken by copy so we can modify
      ProjectionFunction& projectionFunction,
      ProjectionOptions opts
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

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image input = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();
   Tensor outTensor = c_in.Tensor();

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( inSizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( inSizes );
         mask.ExpandSingletonTensor( input.TensorElements() ); // We've checked that it has a single tensor element
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // Determine output sizes
   UnsignedArray outSizes = inSizes;
   UnsignedArray procSizes = inSizes;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( inSizes[ ii ] == 1 ) {
         process[ ii ] = true; // We want to process this dimension, even if this means a no-op, because it maximizes the possibility to skip the looping.
      }
      if( process[ ii ] ) {
         outSizes[ ii ] = 1;
      } else {
         procSizes[ ii ] = 1;
      }
   }
   // NOTE: even if all `process` are false now (no projection to apply at all), we still want to do the processing below.
   // Many projection types incur some change to the data that we need. For example, the `MaximumAbs` projection would
   // apply `Abs` to each pixel, or the `SumSquare` projection would apply `Square`.

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      if( out.Aliases( input ) || out.Aliases( mask )) {
         out.Strip();
      }
      out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
      out.ReshapeTensor( outTensor );
   DIP_END_STACK_TRACE
   out.SetPixelSize( std::move( pixelSize ));
   out.SetColorSpace( std::move( colorSpace ));
   Image output = out.QuickCopy();
   output.Fill( 42 );

   // Do tensor to spatial dimension if necessary
   if( outTensor.Elements() > 1 ) {
      input.TensorToSpatial( 0 );
      if( hasMask ) {
         mask.TensorToSpatial( 0 );
      }
      output.TensorToSpatial( 0 );
      process.insert( 0, false );
      outSizes = output.Sizes(); // == outSizes.insert( 0, outTensor.Elements() );
      procSizes.insert( 0, 1 );
      nDims = outSizes.size();
   }

   // Do we need to loop at all?
   if( process.all() ) {
      //std::cout << "Projection framework: no need to loop!\n";
      projectionFunction.SetNumberOfThreads( 1 );
      if( output.DataType() != outImageType ) {
         Image::Sample outBuffer( outImageType );
         projectionFunction.Project( input, mask, outBuffer, 0 );
         output[0][0] = outBuffer;
      } else {
         Image::Sample outBuffer( output.Origin(), outImageType );
         projectionFunction.Project( input, mask, outBuffer, 0 );
      }
      return;
   }

   // Create view over input image, that spans the processing dimensions
   Image tempIn;
   tempIn.CopyProperties( input );
   tempIn.SetSizes( procSizes );
   tempIn.SetOriginUnsafe( input.Origin() );
   // Create view over mask image, identically to input
   Image tempMask;
   if( hasMask ) {
      tempMask.CopyProperties( mask );
      tempMask.SetSizes( procSizes );
      tempMask.SetOriginUnsafe( mask.Origin() );
   }
   // Make sure that projectionFunction.Project() loops over as few dimensions as possible
   if( hasMask ) {
      if( tempIn.Strides() == tempMask.Strides() ) {
         tempIn.FlattenAsMuchAsPossible();
         tempMask.FlattenAsMuchAsPossible(); // will do same transformation as we did for tempIn
      } else {
         tempIn.Squeeze();// we can't be as aggressive here, but at least prevent looping over singleton dimensions
         tempMask.Squeeze();
      }
   } else {
      tempIn.FlattenAsMuchAsPossible();
   }

   // Create view over output image that doesn't contain the processing dimensions or other singleton dimensions,
   // and keep inStride, maskStride, outStride and outSizes in sync.
   // TODO: This is an opportunity for improving performance if the non-processing dimensions in in and mask
   //       have the same layout and simple stride: turn tempOut into a 1D image.
   IntegerArray inStride = input.Strides();
   IntegerArray maskStride( nDims );
   if( hasMask ) {
      maskStride = mask.Strides();
   }
   IntegerArray outStride = output.Strides();
   dip::uint jj = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( outSizes[ ii ] > 1 ) {
         inStride[ jj ] = inStride[ ii ];
         maskStride[ jj ] = maskStride[ ii ];
         outStride[ jj ] = outStride[ ii ];
         outSizes[ jj ] = outSizes[ ii ];
         ++jj;
      }
   }
   nDims = jj;
   inStride.resize( nDims );
   maskStride.resize( nDims );
   outStride.resize( nDims );
   outSizes.resize( nDims );
   // Pre-multiply output strides with data type size to avoid that multiplication later on
   dip::sint psz = static_cast< dip::sint >( output.DataType().SizeOf() );
   for( dip::sint& s : outStride ) {
      s *= psz;
   }
   dip::uint8* output_pointer = static_cast< dip::uint8* >( output.Origin() );

   // TODO: Determine the number of threads we'll be using. The size of the data has an influence, as does the number
   //       of sub-images that we can generate
   projectionFunction.SetNumberOfThreads( 1 );

   // TODO: Start threads, each thread makes its own temp image.
   dip::uint thread = 0;

   // Iterate over the pixels in the output image. For each, we create a view in the input image.
   UnsignedArray position( nDims, 0 );
   bool useOutputBuffer = output.DataType() != outImageType;
   for( ;; ) {

      // Do the thing
      Image::Sample outSample( output_pointer, output.DataType() );
      if( useOutputBuffer ) {
         Image::Sample outBuffer( outImageType );
         projectionFunction.Project( tempIn, tempMask, outBuffer, thread );
         outSample = outBuffer;
      } else {
         projectionFunction.Project( tempIn, tempMask, outSample, thread );
      }

      // Next output pixel
      dip::uint dd = 0;
      for( ; dd < nDims; dd++ ) {
         ++position[ dd ];
         tempIn.ShiftOriginUnsafe( inStride[ dd ] );
         if( hasMask ) {
            tempMask.ShiftOriginUnsafe( maskStride[ dd ] );
         }
         output_pointer += outStride[ dd ];
         // Check whether we reached the last pixel of the line
         if( position[ dd ] != outSizes[ dd ] ) {
            break;
         }
         // Rewind along this dimension
         tempIn.ShiftOriginUnsafe( -inStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         if( hasMask ) {
            tempMask.ShiftOriginUnsafe( -maskStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
         }
         output_pointer -= outStride[ dd ] * static_cast< dip::sint >( position[ dd ] );
         position[ dd ] = 0;
         // Continue loop to increment along next dimension
      }
      if( dd == nDims ) {
         break;            // We're done!
      }
   }

   // TODO: End threads.
}

} // namespace Framework
} // namespace dip
