/*
 * (c)2016-2022, Cris Luengo.
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
#include "diplib/library/copy_buffer.h"
#include "diplib/multithreading.h"

#include "framework_support.h"

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
      if( border.any() ) {
         BoundaryArrayUseParameter( boundaryConditions, nDims );
      }
   DIP_END_STACK_TRACE

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image input = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();
   DIP_START_STACK_TRACE
      if( c_out.IsOverlappingView( c_in )) {
         // We can work in-place, but not if the input and output don't match exactly.
         // Stripping c_out makes sure we allocate a new data segment for it.
         c_out.Strip();
      }
   DIP_END_STACK_TRACE
   // NOTE: Don't use c_in any more from here on. It has possibly been stripped!

   // Determine output sizes
   UnsignedArray outSizes;
   if( opts.Contains( SeparableOption::DontResizeOutput )) {
      outSizes = c_out.Sizes();
      DIP_THROW_IF( outSizes.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
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
   if( opts.Contains( SeparableOption::AsScalarImage )) {
      if( !input.IsScalar() ) {
         input.TensorToSpatial();
         process.push_back( false );
         border.push_back( 0 );
         tensorToSpatial = true;
         ++nDims;
         inSizes = input.Sizes();
      }
   } else {
      if( opts.Contains( SeparableOption::ExpandTensorInBuffer ) && !input.Tensor().HasNormalOrder() ) {
         lookUpTable = input.Tensor().LookUpTable();
         outTensor.SetMatrix( input.Tensor().Rows(), input.Tensor().Columns() );
         colorSpace.clear(); // the output tensor shape is different from the input's, the color space presumably doesn't match
      }
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      c_out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      c_out.ReshapeTensor( outTensor );
      c_out.SetPixelSize( std::move( pixelSize ));
      if( !colorSpace.empty() ) {
         c_out.SetColorSpace( std::move( colorSpace ));
      }
   DIP_END_STACK_TRACE

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
   if ( opts.Contains( SeparableOption::DontResizeOutput )) { // else: all `grow` is 1.
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
      intermediate.SetSizes( std::move( intermSizes ));
      intermediate.Forge();
   }

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   if( !opts.Contains( SeparableOption::NoMultiThreading ) && ( GetNumberOfThreads() > 1 )) {
      dip::uint operations = 0;
      dip::uint maxNLines = 0;
      UnsignedArray sizes = input.Sizes();
      for( dip::uint processingDim : order ) {
         dip::uint lineLength = sizes[ processingDim ] = outSizes[ processingDim ];
         dip::uint nLines = sizes.product() / lineLength;
         maxNLines = std::max( maxNLines, nLines );
         if( nLines > 1 ) {
            DIP_STACK_TRACE_THIS( operations += nLines *
                  lineFilter.GetNumberOfOperations( lineLength, input.TensorElements(), border[ processingDim ], processingDim ));
         }
         //std::cout << "lineLength = " << lineLength << ", nLines = " << nLines << ", operations = " << operations << std::endl;
      }
      // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
      //std::cout << "GetNumberOfThreads() = " << GetNumberOfThreads() << ", maxNLines = " << maxNLines << ", operations = " << operations << std::endl;
      if( operations >= threadingThreshold ) {
         // We can't do more threads than the max, and we can't do more threads than lines we have to process
         nThreads = std::min( GetNumberOfThreads(), maxNLines );
      }
      // Note that we pick the number of threads according to the dimension where most threads can be used.
      // It is possible that one dimension has fewer image lines than threads we're starting. We need to deal
      // with this below.
   }

   // Some variables that need to be shared among threads, will be filled out by the master thread after thread construction
   Image inImage;
   Image outImage;
   std::vector< UnsignedArray > startCoords;
   dip::uint nLinesPerThread{};
   dip::uint dThreads{};

   // Start threads, each thread makes its own buffers
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      {
         nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
         DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads ));
      }
      #pragma omp barrier

      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );

      // The temporary buffers, if needed, will be stored here (each thread their own!)
      AlignedBuffer inBufferStorage;
      AlignedBuffer outBufferStorage;

      // Iterate over the dimensions to be processed. This loop should not parallelized!
      for( dip::uint rep = 0; rep < order.size(); ++rep ) {
         dip::uint processingDim = order[ rep ];

         #pragma omp master
         {
            // First step always reads from input, other steps read from outImage, which is either intermediate or output
            inImage = (( rep == 0 ) ? ( input ) : ( outImage )).QuickCopy();
            // Last step always writes to output, other steps write to intermediate or output
            UnsignedArray sizes = inImage.Sizes();
            outImage = (( rep == order.size() - 1 ) ? ( output ) : ( useIntermediate ? intermediate : output )).QuickCopy();
            sizes[ processingDim ] = outSizes[ processingDim ];
            outImage.SetSizesUnsafe( sizes );

            //std::cout << "dip::Framework::Separable(), processingDim = " << processingDim << std::endl;
            //std::cout << "   inImage.Origin() = " << inImage.Origin() << std::endl;
            //std::cout << "   inImage.Sizes() = " << inImage.Sizes() << std::endl;
            //std::cout << "   inImage.Strides() = " << inImage.Strides() << std::endl;
            //std::cout << "   outImage.Origin() = " << outImage.Origin() << std::endl;
            //std::cout << "   outImage.Sizes() = " << outImage.Sizes() << std::endl;
            //std::cout << "   outImage.Strides() = " << outImage.Strides() << std::endl;

            // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
            // image lines to process.
            nLinesPerThread = div_ceil( inImage.NumberOfPixels() / inSizes[ processingDim ], nThreads );
            DIP_ASSERT( nLinesPerThread == div_ceil( outImage.NumberOfPixels() / outSizes[ processingDim ], nThreads ));
            dThreads = std::min( div_ceil( inImage.NumberOfPixels() / inSizes[ processingDim ], nLinesPerThread ), nThreads );
            startCoords = SplitImageEvenlyForProcessing( sizes, dThreads, nLinesPerThread, processingDim );
         }
         #pragma omp barrier

         if( thread < dThreads ) {

            // Some values to use during this iteration
            dip::uint inLength = inSizes[ processingDim ];
            DIP_ASSERT( inLength == inImage.Size( processingDim ));
            dip::uint inBorder = border[ processingDim ];
            dip::uint outLength = outSizes[ processingDim ];
            dip::uint outBorder = opts.Contains( SeparableOption::UseOutputBorder ) ? inBorder : 0;

            // Determine if we need to make a temporary buffer for this dimension
            bool inUseBuffer = ( inImage.DataType() != bufferType ) || !lookUpTable.empty() || ( inBorder > 0 ) || opts.Contains( SeparableOption::UseInputBuffer );
            bool outUseBuffer = ( outImage.DataType() != bufferType ) || ( outBorder > 0 ) || opts.Contains( SeparableOption::UseOutputBuffer );
            if( !inUseBuffer && !outUseBuffer && ( inImage.Origin() == outImage.Origin() )) {
               // If input and output images are the same, we need to use at least one buffer!
               inUseBuffer = !opts.Contains( SeparableOption::CanWorkInPlace );
            }
            bool useRealComponentOfOutput = outUseBuffer && bufferType.IsComplex() && !outImage.DataType().IsComplex()
                                            && opts.Contains( SeparableOption::UseRealComponentOfOutput );

            // Create buffer data structs and (re-)allocate buffers
            SeparableBuffer inBuffer{};
            inBuffer.length = inLength;
            inBuffer.border = inBorder;
            if( inUseBuffer ) {
               if( lookUpTable.empty() ) {
                  inBuffer.tensorLength = inImage.TensorElements();
               } else {
                  inBuffer.tensorLength = lookUpTable.size();
               }
               inBuffer.tensorStride = 1;
               inBuffer.stride = static_cast< dip::sint >( inBuffer.tensorLength );
               inBufferStorage.resize(( inLength + 2 * inBorder ) * bufferType.SizeOf() * inBuffer.tensorLength );
               //std::cout << "   Using input buffer, size = " << inBufferStorage.size() << std::endl;
               inBuffer.buffer = inBufferStorage.data() + inBorder * bufferType.SizeOf() * inBuffer.tensorLength;
            } else {
               inBuffer.tensorLength = inImage.TensorElements();
               inBuffer.tensorStride = inImage.TensorStride();
               inBuffer.stride = inImage.Stride( processingDim );
               inBuffer.buffer = nullptr;
               //std::cout << "   Not using input buffer\n";
            }
            SeparableBuffer outBuffer{};
            outBuffer.length = outLength;
            outBuffer.border = outBorder;
            outBuffer.tensorLength = outImage.TensorElements();
            if( outUseBuffer ) {
               outBuffer.tensorStride = 1;
               outBuffer.stride = static_cast< dip::sint >( outBuffer.tensorLength );
               outBufferStorage.resize(( outLength + 2 * outBorder ) * bufferType.SizeOf() * outBuffer.tensorLength );
               outBuffer.buffer = outBufferStorage.data() + outBorder * bufferType.SizeOf() * outBuffer.tensorLength;
               //std::cout << "   Using output buffer, size = " << outBufferStorage.size() << std::endl;
            } else {
               outBuffer.tensorStride = outImage.TensorStride();
               outBuffer.stride = outImage.Stride( processingDim );
               outBuffer.buffer = nullptr;
               //std::cout << "   Not using output buffer\n";
            }

            // Loop over nLinesPerThread image lines
            GenericJointImageIterator< 2 > it( { inImage, outImage }, processingDim );
            it.SetCoordinates( startCoords[ thread ] );
            SeparableLineFilterParameters separableLineFilterParams{
                  inBuffer, outBuffer, processingDim, rep, order.size(), it.Coordinates(), tensorToSpatial, thread
            }; // Takes inBuffer, outBuffer, it.Coordinates() as references
            for( dip::uint ii = 0; ( ii < nLinesPerThread ) && it; ++ii, ++it ) {
               // Get pointers to input and output lines
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
                        inLength,
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
               lineFilter.Filter( separableLineFilterParams );

               // Copy back the line from output buffer to the image
               if( outUseBuffer ) {
                  if( useRealComponentOfOutput ) {
                     detail::CopyBuffer(
                           outBuffer.buffer,
                           bufferType.Real(),
                           outBuffer.stride * 2,
                           outBuffer.tensorStride * 2,
                           it.OutPointer(),
                           outImage.DataType(),
                           outImage.Stride( processingDim ),
                           outImage.TensorStride(),
                           outLength,
                           outBuffer.tensorLength );
                  } else {
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
               }
            }
         }

         // Wait to start the next iteration until all threads have finished their work
         #pragma omp barrier

         // Clear the tensor look-up table: if it was defined, then the intermediate data now has a full matrix
         // as tensor shape and we don't need it any more.
         #pragma omp master
         lookUpTable.clear();
      }
   DIP_PARALLEL_ERROR_END
}


void OneDimensionalLineFilter(
      Image const& c_in,
      Image& c_out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint processingDim,
      dip::uint border,
      BoundaryCondition boundaryCondition,
      SeparableLineFilter& lineFilter,
      SeparableOptions opts
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();

   // Check inputs
   DIP_THROW_IF( processingDim >= nDims, E::INVALID_PARAMETER );

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image input = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   String colorSpace = c_in.ColorSpace();
   DIP_START_STACK_TRACE
      if( c_out.IsOverlappingView( c_in )) {
         // We can work in-place, but not if the input and output don't match exactly.
         // Stripping c_out makes sure we allocate a new data segment for it.
         c_out.Strip();
      }
   DIP_END_STACK_TRACE
   // NOTE: Don't use c_in any more from here on. It has possibly been stripped!

   // Determine output sizes
   UnsignedArray outSizes;
   if( opts.Contains( SeparableOption::DontResizeOutput )) {
      outSizes = c_out.Sizes();
      DIP_THROW_IF( outSizes.size() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         DIP_THROW_IF(( ii != processingDim ) && ( inSizes[ ii ] != outSizes[ ii ] ), "Output size must match input size for dimensions not being processed" );
      }
   } else {
      outSizes = inSizes;

   }

   DIP_THROW_IF(( inSizes[ processingDim ] == 1 ) && ( outSizes[ processingDim ] == 1 ), "Filtering dimension must have a size larger than 1" );

   // `lookUpTable` is the look-up table for `in`. If it is not an
   // empty array, then the tensor needs to be expanded. If it is an empty
   // array, simply copy over the tensor elements the way they are.
   std::vector< dip::sint > lookUpTable;

   // Determine number of tensor elements and do tensor to spatial dimension if necessary
   Tensor outTensor = input.Tensor();
   bool tensorToSpatial = false;
   if( opts.Contains( SeparableOption::AsScalarImage )) {
      if( !input.IsScalar() ) {
         input.TensorToSpatial();
         tensorToSpatial = true;
         ++nDims;
         inSizes = input.Sizes();
      }
   } else {
      if( opts.Contains( SeparableOption::ExpandTensorInBuffer ) && !input.Tensor().HasNormalOrder() ) {
         lookUpTable = input.Tensor().LookUpTable();
         outTensor.SetMatrix( input.Tensor().Rows(), input.Tensor().Columns() );
         colorSpace.clear(); // the output tensor shape is different from the input's, the color space presumably doesn't match
      }
   }

   // Adjust output if necessary (and possible)
   DIP_START_STACK_TRACE
      c_out.ReForge( outSizes, outTensor.Elements(), outImageType, Option::AcceptDataTypeChange::DO_ALLOW );
      c_out.ReshapeTensor( outTensor );
      c_out.SetPixelSize( std::move( pixelSize ));
      if( !colorSpace.empty() ) {
         c_out.SetColorSpace( std::move( colorSpace ));
      }
   DIP_END_STACK_TRACE

   // Make simplified copies of output image headers so we can modify them at will
   Image output = c_out.QuickCopy();

   // Do tensor to spatial dimension if necessary
   if( tensorToSpatial ) {
      output.TensorToSpatial();
      outSizes = output.Sizes();
   }

   // Determine the number of threads we'll be using
   dip::uint nThreads = 1;
   if( !opts.Contains( SeparableOption::NoMultiThreading ) && ( GetNumberOfThreads() > 1 )) {
      dip::uint operations = 0;
      dip::uint lineLength = outSizes[ processingDim ];
      dip::uint nLines = inSizes.product() / inSizes[ processingDim ];
      if( nLines > 1 ) {
         DIP_STACK_TRACE_THIS( operations = nLines *
                                            lineFilter.GetNumberOfOperations( lineLength, input.TensorElements(), border, processingDim ));
      }
      // Starting threads is only worth while if we'll do at least `threadingThreshold` operations
      if( operations >= threadingThreshold ) {
         // We can't do more threads than the max, and we can't do more threads than lines we have to process
         nThreads = std::min( GetNumberOfThreads(), nLines );
      }
   }
   dip::uint nLinesPerThread = div_ceil( input.NumberOfPixels() / inSizes[ processingDim ], nThreads );
   nThreads = std::min( div_ceil( input.NumberOfPixels() / inSizes[ processingDim ], nLinesPerThread ), nThreads );
   std::vector< UnsignedArray > startCoords;

   // Some values to use
   dip::uint inLength = inSizes[ processingDim ];
   dip::uint inBorder = border;
   dip::uint outLength = outSizes[ processingDim ];
   dip::uint outBorder = opts.Contains( SeparableOption::UseOutputBorder ) ? inBorder : 0;

   // Determine if we need to make a temporary buffer
   bool inUseBuffer = ( input.DataType() != inBufferType ) || !lookUpTable.empty() || ( inBorder > 0 ) || opts.Contains( SeparableOption::UseInputBuffer );
   bool outUseBuffer = ( output.DataType() != outBufferType ) || ( outBorder > 0 ) || opts.Contains( SeparableOption::UseOutputBuffer );
   if( !inUseBuffer && !outUseBuffer && ( input.Origin() == output.Origin() )) {
      // If input and output images are the same, we need to use at least one buffer!
      inUseBuffer = !opts.Contains( SeparableOption::CanWorkInPlace );
   }
   bool useRealComponentOfOutput = outUseBuffer && outBufferType.IsComplex() && !output.DataType().IsComplex()
                                   && opts.Contains( SeparableOption::UseRealComponentOfOutput );

   // Start threads, each thread makes its own buffers
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      {
         nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
         DIP_STACK_TRACE_THIS( lineFilter.SetNumberOfThreads( nThreads ));
         // Divide the image domain into nThreads chunks for split processing. The last chunk will have same or fewer
         // image lines to process.
         startCoords = SplitImageEvenlyForProcessing( outSizes, nThreads, nLinesPerThread, processingDim );
      }
      #pragma omp barrier

      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );

      // The temporary buffers, if needed, will be stored here (each thread their own!)
      AlignedBuffer inBufferStorage;
      AlignedBuffer outBufferStorage;

      // Create buffer data structs and (re-)allocate buffers
      SeparableBuffer inBuffer{};
      inBuffer.length = inLength;
      inBuffer.border = inBorder;
      if( inUseBuffer ) {
         if( lookUpTable.empty() ) {
            inBuffer.tensorLength = input.TensorElements();
         } else {
            inBuffer.tensorLength = lookUpTable.size();
         }
         inBuffer.tensorStride = 1;
         inBuffer.stride = static_cast< dip::sint >( inBuffer.tensorLength );
         inBufferStorage.resize(( inLength + 2 * inBorder ) * inBufferType.SizeOf() * inBuffer.tensorLength );
         //std::cout << "   Using input buffer, size = " << inBufferStorage.size() << std::endl;
         inBuffer.buffer = inBufferStorage.data() + inBorder * inBufferType.SizeOf() * inBuffer.tensorLength;
      } else {
         inBuffer.tensorLength = input.TensorElements();
         inBuffer.tensorStride = input.TensorStride();
         inBuffer.stride = input.Stride( processingDim );
         inBuffer.buffer = nullptr;
         //std::cout << "   Not using input buffer\n";
      }
      SeparableBuffer outBuffer{};
      outBuffer.length = outLength;
      outBuffer.border = outBorder;
      outBuffer.tensorLength = output.TensorElements();
      if( outUseBuffer ) {
         outBuffer.tensorStride = 1;
         outBuffer.stride = static_cast< dip::sint >( outBuffer.tensorLength );
         outBufferStorage.resize(( outLength + 2 * outBorder ) * outBufferType.SizeOf() * outBuffer.tensorLength );
         outBuffer.buffer = outBufferStorage.data() + outBorder * outBufferType.SizeOf() * outBuffer.tensorLength;
         //std::cout << "   Using output buffer, size = " << outBufferStorage.size() << std::endl;
      } else {
         outBuffer.tensorStride = output.TensorStride();
         outBuffer.stride = output.Stride( processingDim );
         outBuffer.buffer = nullptr;
         //std::cout << "   Not using output buffer\n";
      }

      // Loop over nLinesPerThread image lines
      GenericJointImageIterator< 2 > it( { input, output }, processingDim );
      it.SetCoordinates( startCoords[ thread ] );
      SeparableLineFilterParameters separableLineFilterParams{
            inBuffer, outBuffer, processingDim, 0, 1, it.Coordinates(), tensorToSpatial, thread
      }; // Takes inBuffer, outBuffer, it.Coordinates() as references
      for( dip::uint ii = 0; ( ii < nLinesPerThread ) && it; ++ii, ++it ) {
         // Get pointers to input and output lines
         if( inUseBuffer ) {
            detail::CopyBuffer(
                  it.InPointer(),
                  input.DataType(),
                  input.Stride( processingDim ),
                  input.TensorStride(),
                  inBuffer.buffer,
                  inBufferType,
                  inBuffer.stride,
                  inBuffer.tensorStride,
                  inLength,
                  inBuffer.tensorLength,
                  lookUpTable );
            if(( inBorder > 0 ) && ( inBuffer.stride != 0 )) {
               detail::ExpandBuffer(
                     inBuffer.buffer,
                     inBufferType,
                     inBuffer.stride,
                     inBuffer.tensorStride,
                     inLength,
                     inBuffer.tensorLength,
                     inBorder,
                     inBorder,
                     boundaryCondition );
            }
         } else {
            inBuffer.buffer = it.InPointer();
         }
         if( !outUseBuffer ) {
            outBuffer.buffer = it.OutPointer();
         }

         // Filter the line
         lineFilter.Filter( separableLineFilterParams );

         // Copy back the line from output buffer to the image
         if( outUseBuffer ) {
            if( useRealComponentOfOutput ) {
               detail::CopyBuffer(
                     outBuffer.buffer,
                     outBufferType.Real(),
                     outBuffer.stride * 2,
                     outBuffer.tensorStride * 2,
                     it.OutPointer(),
                     output.DataType(),
                     output.Stride( processingDim ),
                     output.TensorStride(),
                     outLength,
                     outBuffer.tensorLength );
            } else {
               detail::CopyBuffer(
                     outBuffer.buffer,
                     outBufferType,
                     outBuffer.stride,
                     outBuffer.tensorStride,
                     it.OutPointer(),
                     output.DataType(),
                     output.Stride( processingDim ),
                     output.TensorStride(),
                     outLength,
                     outBuffer.tensorLength );
            }
         }
      }
   DIP_PARALLEL_ERROR_END
}

} // namespace Framework
} // namespace dip
