/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the Kuwahara-Nagao operator.
 *
 * (c)2017, Cris Luengo.
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
#include "diplib/nonlinear.h"
#include "diplib/linear.h"
#include "diplib/framework.h"
#include "diplib/generic_iterators.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"

namespace dip {

namespace {

struct SelectionLineFilterParameters {
   void const* inBuffer;
   dfloat const* controlBuffer;
   void* outBuffer;
   dip::sint inStride;
   dip::sint inTensorStride; // == 1
   dip::sint controlStride;
   dip::sint outStride;
   dip::sint outTensorStride;
   dip::uint tensorLength;
   dip::uint bufferLength;
   std::vector< dip::sint > const& pixelTableOffsets;
   std::vector< dfloat > const& pixelTableWeights;
   dfloat threshold;
   bool minimum;
};

class SelectionLineFilterBase {
   public:
      virtual void Filter( SelectionLineFilterParameters const& params ) = 0;
};

template< typename TPI >
class SelectionLineFilter : public SelectionLineFilterBase {
   public:
      virtual void Filter( SelectionLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer );
         dfloat const* control = params.controlBuffer;
         TPI* out = static_cast< TPI* >( params.outBuffer );
         // For each pixel on the line:
         for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
            // Iterate over the pixel table and find optimal offset
            auto it = params.pixelTableOffsets.begin();
            auto d = params.pixelTableWeights.begin();
            dfloat centerValue = *control;
            dfloat bestValue = params.minimum ? std::numeric_limits< dfloat >::max() : std::numeric_limits< dfloat >::lowest();
            dfloat bestDistance = std::numeric_limits< dfloat >::max();
            dip::sint bestOffset = 0;
            do {
               dfloat value = control[ *it ];
               if(( params.minimum ? ( value < bestValue ) : ( value > bestValue )) ||
                     (( value == bestValue ) && ( *d < bestDistance ))) {
                  bestValue = value;
                  bestDistance = *d;
                  bestOffset = *it;
               }
               ++it;
               ++d;
            } while( it != params.pixelTableOffsets.end() );
            if( params.minimum ? bestValue + params.threshold < centerValue
                               : bestValue - params.threshold > centerValue ) {
               bestOffset *= static_cast< dip::sint >( params.tensorLength );
            } else {
               bestOffset = 0;
            }
            // Copy the tensor at that offset over the the output
            out[ 0 ] = in[ bestOffset ];
            for( dip::sint jj = 1; jj < static_cast< dip::sint >( params.tensorLength ); ++jj ) {
               out[ jj * params.outTensorStride ] = in[ bestOffset + jj * params.inTensorStride ];
            }
            // Next pixel
            in += params.inStride;
            control += params.controlStride;
            out += params.outStride;
         }
      }
};

} // namespace

void SelectionFilter(
      Image const& c_in,
      Image const& c_control,
      Image& out,
      Kernel const& kernel,
      dfloat threshold,
      String const& mode,
      StringArray const& boundaryCondition
) {
   // We are not using a framework here, because this is the only pixel table filter that uses two input images.
   // So we've copied things over from Framework::Full, and changed (simplified) them a bit. There's no multi-threading
   // here yet.
   // TODO: add multithreading.

   DIP_THROW_IF( !c_in.IsForged() || !c_control.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( c_in.Sizes() != c_control.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( !c_control.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_control.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( kernel.HasWeights(), E::KERNEL_NOT_BINARY );
   bool minimum;
   DIP_STACK_TRACE_THIS( minimum = BooleanFromString( mode, S::MINIMUM, S::MAXIMUM ));

   // Determine boundary sizes
   UnsignedArray kernelSizes;
   DIP_STACK_TRACE_THIS( kernelSizes = kernel.Sizes( c_in.Dimensionality() ));
   UnsignedArray boundary = kernel.Boundary( c_in.Dimensionality() );

   // Copy input images with boundary extension
   Image in;
   Image control;
   control.SetDataType( DT_DFLOAT );
   control.Protect();
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      ExtendImage( c_in, in, boundary, bc, Option::ExtendImage::Masked );
      ExtendImage( c_control, control, boundary, bc, Option::ExtendImage::Masked );
   DIP_END_STACK_TRACE
#ifdef DIP__ENABLE_ASSERT
   // We have created a new `in` and `control`, so we expect normal strides here. Let's make sure this is the case!
   DIP_ASSERT( in.TensorStride() == 1 );
   for( dip::uint ii = 0; ii < in.Dimensionality(); ++ii ) {
      DIP_ASSERT( in.Stride( ii ) == control.Stride( ii ) * static_cast< dip::sint >( in.TensorElements() ));
   }
#endif

   // Adjust output if necessary (and possible)
   // NOTE: Don't use c_in any more from here on. It has possibly been reforged!
   DIP_START_STACK_TRACE
      out.ReForge( in.Sizes(), in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DONT_ALLOW );
      out.ReshapeTensor( in.Tensor() );
      out.SetPixelSize( in.PixelSize() );
      if( !in.IsColor() ) {
         out.SetColorSpace( in.ColorSpace() );
      }
   DIP_END_STACK_TRACE
   DIP_ASSERT( in.DataType() == out.DataType() );

   // Create a pixel table suitable to be applied to `input`
   dip::uint processingDim = Framework::OptimalProcessingDim( in, kernelSizes );
   PixelTable pixelTable;
   DIP_STACK_TRACE_THIS( pixelTable = kernel.PixelTable( in.Dimensionality(), processingDim ));
   pixelTable.AddDistanceToOriginAsWeights();
   PixelTableOffsets pixelTableOffsets = pixelTable.Prepare( control ); // offsets are for the `control` image, multiply by `in.TensorElements()` to get offsets into `in`.

   // Get the line filter of the right type
   std::unique_ptr< SelectionLineFilterBase > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, SelectionLineFilter, (), in.DataType() );

   // Loop over all image lines
   SelectionLineFilterParameters params = {
         nullptr,
         nullptr,
         nullptr,
         in.Stride( processingDim ),
         in.TensorStride(),
         control.Stride( processingDim ),
         out.Stride( processingDim ),
         out.TensorStride(),
         in.TensorElements(),
         in.Size( processingDim ),
         pixelTableOffsets.Offsets(),
         pixelTableOffsets.Weights(),
         threshold,
         minimum
   };
   GenericJointImageIterator< 3 > it( { in, control, out }, processingDim );
   it.OptimizeAndFlatten();
   do {
      params.inBuffer = in.Pointer( it.Offset< 0 >() );
      params.controlBuffer = static_cast< dfloat* >( control.Pointer( it.Offset< 1 >() ));
      params.outBuffer = out.Pointer( it.Offset< 2 >() );
      DIP_STACK_TRACE_THIS( lineFilter->Filter( params ));
   } while( ++it );
}

void Kuwahara(
      Image const& in,
      Image& out,
      Kernel kernel,
      dfloat threshold,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      Image value = dip::Uniform( in, kernel, boundaryCondition );
      Image control = dip::VarianceFilter( in, kernel, boundaryCondition );
      kernel.Mirror();
      SelectionFilter( value, control, out, kernel, threshold, S::MINIMUM, boundaryCondition );
   DIP_END_STACK_TRACE
}

} // namespace dip
