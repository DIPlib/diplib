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

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"

namespace dip {
namespace Framework {

void Full(
      Image const& c_in,
      Image& c_out,
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
   DIP_STACK_TRACE_THIS( kernelSizes = kernel.Sizes( sizes.size() ));

   // Store these because they can get lost when ReForging `c_out` (it could be the same image as `c_in`)
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
      expandTensor = ( opts == Full_ExpandTensorInBuffer ) && !c_in.Tensor().HasNormalOrder();
   }

   // Determine boundary sizes
   UnsignedArray boundary = kernel.Boundary( c_in.Dimensionality() );

   // Copy input if necessary (this is the input buffer!)
   Image input;
   bool dataTypeChange = false;
   if( c_in.DataType() != inBufferType ) {
      dataTypeChange = true;
      input.SetDataType( inBufferType );
      input.Protect();
   }
   bool expandBoundary = boundary.any() && ( opts != Full_BorderAlreadyExpanded );
   if( dataTypeChange || expandTensor || expandBoundary ) {
      Option::ExtendImage options = Option::ExtendImage_Masked;
      if( expandTensor ) {
         options += Option::ExtendImage_ExpandTensor;
      }
      DIP_STACK_TRACE_THIS( ExtendImageLowLevel( c_in, input, boundary, boundaryConditions, options ));
   } else {
      input = c_in.QuickCopy();
   }

   // Adjust c_out if necessary (and possible)
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
   DIP_START_STACK_TRACE
      if( c_out.Aliases( input )) {
         // We cannot work in-place! Note this only happens if we didn't call `ExtendImageLowLevel` earlier.
         c_out.Strip();
      }
      c_out.ReForge( sizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      c_out.ReshapeTensor( outTensor );
      c_out.SetPixelSize( pixelSize );
      if( !colorSpace.empty() ) {
         c_out.SetColorSpace( colorSpace );
      }
   DIP_END_STACK_TRACE
   Image output = c_out.QuickCopy();

   // Create a pixel table suitable to be applied to `input`
   dip::uint processingDim = OptimalProcessingDim( input, kernelSizes );
   PixelTable pixelTable;
   DIP_STACK_TRACE_THIS( pixelTable = kernel.PixelTable( sizes.size(), processingDim ));
   PixelTableOffsets pixelTableOffsets = pixelTable.Prepare( input );

   // Convert input and output to scalar images if needed -- add tensor dimension at end so `processingDim` is not affected.
   if( asScalarImage ) {
      input.TensorToSpatial();
      output.TensorToSpatial();
   }

   // Do we need an output buffer?
   bool useOutBuffer = output.DataType() != outBufferType;

   // How many pixels in a line? How many lines?
   dip::uint lineLength = input.Size( processingDim );
   dip::uint nLines = input.NumberOfPixels() / lineLength; // this must be a round division

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   if( opts != Full_NoMultiThreading ) {
      nThreads = std::min( GetNumberOfThreads(), nLines );
      if( nThreads > 1 ) {
         dip::uint operations;
         DIP_STACK_TRACE_THIS( operations = nLines *
               lineFilter.GetNumberOfOperations( lineLength, input.TensorElements(), pixelTable.NumberOfPixels(), pixelTable.Runs().size() ));
         // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
         if( operations < threadingThreshold ) {
            nThreads = 1;
         }
      }
   }

   //std::cout << "Starting " << nThreads << " threads\n";
   DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads, pixelTableOffsets ));

   // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
   // image lines to process.
   dip::uint nLinesPerThread = div_ceil( nLines, nThreads );
   std::vector< UnsignedArray > startCoords( nThreads );
   dip::uint nDims = input.Dimensionality();
   startCoords[ 0 ] = UnsignedArray( nDims, 0 );
   for( dip::uint ii = 1; ii < nThreads; ++ii ) {
      startCoords[ ii ] = startCoords[ ii - 1 ];
      // To advance the iterator nLinesPerThread times, we increment it in whole-line steps.
      dip::uint firstDim = processingDim == 0 ? 1 : 0;
      dip::uint remaining = nLinesPerThread;
      do {
         for( dip::uint dd = 0; dd < nDims; ++dd ) {
            if( dd == firstDim ) {
               dip::uint n = sizes[ dd ] - startCoords[ ii ][ dd ];
               if (remaining >= n) {
                  // Rewinding, next loop iteration will increment the next coordinate
                  remaining -= n;
                  startCoords[ ii ][ dd ] = 0;
               } else {
                  // Forward by `remaining`, then we're done.
                  startCoords[ ii ][ dd ] += remaining;
                  remaining = 0;
                  break;
               }
            } else if( dd != processingDim ) {
               // Increment coordinate
               ++startCoords[ ii ][ dd ];
               // Check whether we reached the last pixel of the line
               if( startCoords[ ii ][ dd ] < sizes[ dd ] ) {
                  break;
               }
               // Rewind, the next loop iteration will increment the next coordinate
               startCoords[ ii ][ dd ] = 0;
            }
         }
      } while( remaining > 0 );
   }

   // Start threads, each thread makes its own buffers
   AssertionError assertionError;
   ParameterError parameterError;
   RunTimeError runTimeError;
   Error error;
   #pragma omp parallel num_threads( nThreads )
   try {
      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );

      // Create input buffer data struct
      FullBuffer inBuffer;
      inBuffer.tensorLength = input.TensorElements();
      inBuffer.tensorStride = input.TensorStride();
      inBuffer.stride = input.Stride( processingDim );
      inBuffer.buffer = nullptr;

      // Create output buffer data struct and allocate buffer if necessary
      std::vector< uint8 > outputBuffer;
      FullBuffer outBuffer;
      outBuffer.tensorLength = output.TensorElements();
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

      // Loop over nLinesPerThread image lines
      GenericJointImageIterator< 2 > it( { input, output }, processingDim );
      it.SetCoordinates( startCoords[ thread ] );
      FullLineFilterParameters fullLineFilterParameters{
            inBuffer, outBuffer, lineLength, processingDim, it.Coordinates(), pixelTableOffsets, thread
      }; // Takes inBuffer, outBuffer, it.Coordinates(), pixelTableOffsets as references
      for( dip::uint ii = 0; ( ii < nLinesPerThread ) && it; ++ii, ++it ) {
         inBuffer.buffer = it.InPointer();
         if( !useOutBuffer ) {
            // Point output buffer to right line in output image
            outBuffer.buffer = it.OutPointer();
         }
         // Filter the line
         lineFilter.Filter( fullLineFilterParameters );
         if( useOutBuffer ) {
            // Copy output buffer to output image
            detail::CopyBuffer(
                  outBuffer.buffer,
                  outBufferType,
                  outBuffer.stride,
                  outBuffer.tensorStride,
                  it.OutPointer(),
                  output.DataType(),
                  output.Stride( processingDim ),
                  output.TensorStride(),
                  lineLength,
                  outBuffer.tensorLength );
         }
      }
   } catch( dip::AssertionError const& e ) {
      if( !assertionError.IsSet() ) {
         assertionError = e;
         DIP_ADD_STACK_TRACE( assertionError );
      }
   } catch( dip::ParameterError const& e ) {
      if( !parameterError.IsSet() ) {
         parameterError = e;
         DIP_ADD_STACK_TRACE( parameterError );
      }
   } catch( dip::RunTimeError const& e ) {
      if( !runTimeError.IsSet() ) {
         runTimeError = e;
         DIP_ADD_STACK_TRACE( runTimeError );
      }
   } catch( dip::Error const& e ) {
      if( !error.IsSet() ) {
         error = e;
         DIP_ADD_STACK_TRACE( error );
      }
   } catch( std::exception const& stde ) {
      if( !runTimeError.IsSet() ) {
         runTimeError = dip::RunTimeError( stde.what() );
         DIP_ADD_STACK_TRACE( runTimeError );
      }
   }
   if( assertionError.IsSet() ) {
      throw assertionError;
   }
   if( parameterError.IsSet() ) {
      throw parameterError;
   }
   if( runTimeError.IsSet() ) {
      throw runTimeError;
   }
   if( error.IsSet() ) {
      throw error;
   }
}

} // namespace Framework
} // namespace dip
