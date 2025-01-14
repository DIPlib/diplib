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
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/generic_iterators.h"
#include "diplib/kernel.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"
#include "diplib/pixel_table.h"

#include "framework_support.h"

namespace dip {
namespace Framework {

void Full(
      Image const& c_in,
      Image& c_out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint nTensorElements,
      BoundaryConditionArray const& boundaryConditions,
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
   if( opts.Contains( FullOption::AsScalarImage )) {
      outTensor = c_in.Tensor();
      if( !c_in.IsScalar() ) {
         asScalarImage = true;
      }
   } else {
      expandTensor = opts.Contains( FullOption::ExpandTensorInBuffer ) && !c_in.Tensor().HasNormalOrder();
   }

   // Determine boundary sizes
   UnsignedArray boundary = kernel.Boundary( c_in.Dimensionality() );

   // Do we need to adjust the input image?
   bool dataTypeChange = c_in.DataType() != inBufferType;
   bool expandBoundary = boundary.any();
   bool alreadyExpanded = opts.Contains( FullOption::BorderAlreadyExpanded );
   if( !boundaryConditions.empty() ) {
      auto cond = boundaryConditions == BoundaryCondition::ALREADY_EXPANDED;
      if( cond.all() ) {
         alreadyExpanded = true;
      } else {
         DIP_THROW_IF( cond.any(), "\"already expanded\" boundary condition cannot be combined with other boundary conditions" );
      }
   }
   if( !expandBoundary ) {
      alreadyExpanded = false; // we can ignore this flag in this case, we won't be reading outside the image bounds.
   }
   // TODO: We've been passed an input image with borders expanded, meaning we need to use the data there.
   //      But we need to convert the image's type or expand its tensor, meaning that we need to create an
   //      input buffer and copy data into it. This requires copying data from the expanded image, not only
   //      the input image.
   DIP_THROW_IF( alreadyExpanded && ( dataTypeChange || expandTensor ), "Input buffer was already expanded, but I need to expand the tensor or convert data type" );
   bool adjustInput = !alreadyExpanded && ( dataTypeChange || expandTensor || expandBoundary );

   // Adjust c_out if necessary (and possible)
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
   Image cc_in = c_in.QuickCopy(); // Preserve for later
   DIP_START_STACK_TRACE
      if( c_out.Aliases( c_in )) {
         // We cannot work in-place! Note this only happens if we didn't call `ExtendImage` earlier.
         c_out.Strip();
      }
      c_out.ReForge( sizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      c_out.ReshapeTensor( outTensor );
      c_out.SetPixelSize( std::move( pixelSize ));
      if( !colorSpace.empty() ) {
         c_out.SetColorSpace( std::move( colorSpace ));
      }
   DIP_END_STACK_TRACE
   Image output = c_out.QuickCopy();

   // Copy input if necessary (this is the input buffer!)
   // If we do copy the input, we'll adjust its strides to match those of output.
   Image input;
   if( adjustInput ) {
      input.SetDataType( inBufferType );
      if( expandTensor ) {
         input.SetTensorSizes( cc_in.TensorColumns() * cc_in.TensorRows() );
      } else {
         input.SetTensorSizes( cc_in.TensorElements() );
      }
      UnsignedArray bufferSizes = cc_in.Sizes();
      if( expandBoundary ) {
         for( dip::uint ii = 0; ii < bufferSizes.size(); ++ii ) {
            bufferSizes[ ii ] += 2 * boundary[ ii ];
         }
      }
      input.SetSizes( std::move( bufferSizes ));
      input.MatchStrideOrder( output );
      input.Forge(); // This forge will honor the strides we've set, the image does not have an external interface.
      input.Protect(); // make sure it's not reforged by `ExtendImage` or `Copy`.
      if( expandTensor || expandBoundary ) {
         Option::ExtendImageFlags options = Option::ExtendImage::Masked;
         if( expandTensor ) {
            options += Option::ExtendImage::ExpandTensor;
         }
         // TODO: Now that we've got `ExtendRegion`, we could expand boundaries unevenly, e.g. for shifted kernels.
         DIP_STACK_TRACE_THIS( ExtendImage( cc_in, input, boundary, boundaryConditions, options ));
      } else { // if( dataTypeChange )
         input.Copy( cc_in );
      }
      input.Protect( false );
   } else {
      input = cc_in.QuickCopy();
   }
   cc_in.Strip(); // we don't need to keep that around any more

   // Create a pixel table suitable to be applied to `input`
   dip::uint processingDim = OptimalProcessingDim( input, kernelSizes );
   PixelTable pixelTable;
   DIP_STACK_TRACE_THIS( pixelTable = kernel.PixelTable( sizes.size(), processingDim ));
   PixelTableOffsets pixelTableOffsets = pixelTable.Prepare( input );

   // Convert input and output to scalar images if needed -- add tensor dimension at end so `processingDim` is not affected.
   if( asScalarImage ) {
      input.TensorToSpatial();
      output.TensorToSpatial();
      sizes = input.Sizes();
   }

   // Do we need an output buffer?
   bool useOutBuffer = output.DataType() != outBufferType;

   // How many pixels in a line? How many lines?
   dip::uint lineLength = input.Size( processingDim );
   dip::uint nLines = input.NumberOfPixels() / lineLength; // this must be a round division

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   if( !opts.Contains( FullOption::NoMultiThreading )) {
      nThreads = std::min( GetNumberOfThreads(), nLines );
      if( nThreads > 1 ) {
         DIP_START_STACK_TRACE
         dip::uint operations = nLines *
               lineFilter.GetNumberOfOperations( lineLength, input.TensorElements(), pixelTableOffsets.NumberOfPixels(), pixelTableOffsets.Runs().size() );
         // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
         if( operations < threadingThreshold ) {
            nThreads = 1;
         }
         DIP_END_STACK_TRACE
      }
   }
   dip::uint nLinesPerThread = div_ceil( nLines, nThreads );
   nThreads = std::min( div_ceil( nLines, nLinesPerThread ), nThreads );
   std::vector< UnsignedArray > startCoords;

   // Start threads, each thread makes its own buffers
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      {
         nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
         DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads, pixelTableOffsets ));
         // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
         // image lines to process.
         startCoords = SplitImageEvenlyForProcessing( sizes, nThreads, nLinesPerThread, processingDim );
      }
      #pragma omp barrier

      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );

      // Create input buffer data struct
      FullBuffer inBuffer{};
      inBuffer.tensorLength = input.TensorElements();
      inBuffer.tensorStride = input.TensorStride();
      inBuffer.stride = input.Stride( processingDim );
      inBuffer.buffer = nullptr;

      // Create output buffer data struct and allocate buffer if necessary
      AlignedBuffer outputBuffer;
      FullBuffer outBuffer{};
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
   DIP_PARALLEL_ERROR_END
}

} // namespace Framework
} // namespace dip
