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

#include "diplib/framework.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/multithreading.h"

#include "framework_support.h"

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
   // output.Fill( 42 ); // for debugging, to see if all output samples get written to

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
   dip::uint8* outputPointer = static_cast< dip::uint8* >( output.Origin() );
   bool useOutputBuffer = output.DataType() != outImageType;

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   dip::uint nLoop = output.NumberOfPixels();
   if( !opts.Contains( ProjectionOption::NoMultiThreading )) {
      nThreads = std::min( GetNumberOfThreads(), nLoop );
      if( nThreads > 1 ) {
         DIP_START_STACK_TRACE
         dip::uint operations = nLoop * projectionFunction.GetNumberOfOperations( tempIn.NumberOfPixels() );
         // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
         if( operations < threadingThreshold ) {
            nThreads = 1;
         }
         DIP_END_STACK_TRACE
      }
   }
   dip::uint nLoopPerThread = div_ceil( nLoop, nThreads );
   nThreads = std::min( div_ceil( nLoop, nLoopPerThread ), nThreads );
   std::vector< UnsignedArray > startCoords;

   // Start threads
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      {
         nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
         DIP_STACK_TRACE_THIS( projectionFunction.SetNumberOfThreads( nThreads ));
         // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
         // image lines to process.
         startCoords = SplitImageEvenlyForProcessing( outSizes, nThreads, nLoopPerThread, nDims );
      }
      #pragma omp barrier

      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );
      UnsignedArray position = startCoords[ thread ];
      IntegerArray startPosition{ position };
      dip::Image localTempIn = tempIn.QuickCopy();
      localTempIn.ShiftOriginUnsafe( Image::Offset( startPosition, inStride ));
      dip::Image localTempMask = tempMask.QuickCopy();
      if( hasMask ) {
         localTempMask.ShiftOriginUnsafe( Image::Offset( startPosition, maskStride ));
      }
      dip::uint8* localOutputPointer = outputPointer + Image::Offset( startPosition, outStride );

      // Iterate over the pixels in the output image. For each, we create a view in the input image.
      for( dip::uint ii = 0; ii < nLoopPerThread; ++ii ) {

         // Do the thing
         Image::Sample outSample( localOutputPointer, output.DataType() );
         if( useOutputBuffer ) {
            Image::Sample outBuffer( outImageType );
            projectionFunction.Project( localTempIn, localTempMask, outBuffer, thread );
            outSample = outBuffer;
         } else {
            projectionFunction.Project( localTempIn, localTempMask, outSample, thread );
         }

         // Next output pixel
         dip::uint dd = 0;
         for( ; dd < nDims; dd++ ) {
            ++position[ dd ];
            localTempIn.ShiftOriginUnsafe( inStride[ dd ] );
            if( hasMask ) {
               localTempMask.ShiftOriginUnsafe( maskStride[ dd ] );
            }
            localOutputPointer += outStride[ dd ];
            // Check whether we reached the last pixel of the line
            if( position[ dd ] != outSizes[ dd ] ) {
               break;
            }
            // Rewind along this dimension
            localTempIn.ShiftOriginUnsafe( -inStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
            if( hasMask ) {
               localTempMask.ShiftOriginUnsafe( -maskStride[ dd ] * static_cast< dip::sint >( position[ dd ] ));
            }
            localOutputPointer -= outStride[ dd ] * static_cast< dip::sint >( position[ dd ] );
            position[ dd ] = 0;
            // Continue loop to increment along next dimension
         }
         if( dd == nDims ) {
            break;            // We're done!
         }
      }
   DIP_PARALLEL_ERROR_END
}

} // namespace Framework
} // namespace dip
