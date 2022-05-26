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

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"

namespace dip {

Image::Pixel ReadPixelWithBoundaryCondition(
      Image const& img,
      IntegerArray coords, // getting a local copy so we can modify it
      BoundaryConditionArray const& bc
) {
   DIP_THROW_IF( coords.size() != img.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   bool invert = false;
   Image::Pixel out( DataType::SuggestFlex( img.DataType() ), img.TensorElements() );
   out.ReshapeTensor( img.Tensor() );
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      dip::sint sz = static_cast< dip::sint >( img.Size( ii ));
      if(( coords[ ii ] < 0 ) || ( coords[ ii ] >= sz )) {
         switch( bc[ ii ] ) {
            case BoundaryCondition::ASYMMETRIC_MIRROR:
               invert = true;
               // Intentionally falls through
            case BoundaryCondition::SYMMETRIC_MIRROR:
               coords[ ii ] = modulo( coords[ ii ], sz * 2 );
               if( coords[ ii ] >= sz ) {
                  coords[ ii ] = 2 * sz - coords[ ii ] - 1;
               }
               break;
            case BoundaryCondition::ASYMMETRIC_PERIODIC:
               invert = true;
               // Intentionally falls through
            case BoundaryCondition::PERIODIC:
               coords[ ii ] = modulo( coords[ ii ], sz );
               break;
            case BoundaryCondition::ADD_ZEROS:
               out = 0;
               return out; // We're done!
            case BoundaryCondition::ADD_MAX_VALUE:
               out = infinity;
               return out; // We're done!
            case BoundaryCondition::ADD_MIN_VALUE:
               out = -infinity;
               return out; // We're done!
            case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
               coords[ ii ] = clamp( coords[ ii ], dip::sint( 0 ), sz - 1 );
               break;
            case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:  // not implemented, difficult to implement in this framework.
            case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE: // not implemented, difficult to implement in this framework.
            case BoundaryCondition::THIRD_ORDER_EXTRAPOLATE:  // not implemented, difficult to implement in this framework.
            default:
               DIP_THROW("Boundary condition not implemented" );
         }
      }
   }
   Image::Pixel tmp( img.Pointer( coords ), img.DataType(), img.Tensor(), img.TensorStride() );
   out = invert ? -tmp : tmp; // copy pixel values over from `tmp`, which references them.
   return out;
}

namespace {

Option::ExtendImageFlags TranslateExtendImageFlags( StringSet const& options ) {
   Option::ExtendImageFlags opts;
   if( options.count( "masked" ) > 0 ) {
      opts += Option::ExtendImage::Masked;
   }
   if( options.count( "expand tensor" ) > 0 ) {
      opts += Option::ExtendImage::ExpandTensor;
   }
   return opts;
}

void ExtendImage(
      Image const& c_in,
      Image& out,
      UnsignedArray const& sizes,
      RangeArray window,
      BoundaryConditionArray boundaryConditions,
      Option::ExtendImageFlags options
) {
   // This is an internal function, when we call it, we've already ensured `in` is forged.
   // We also know that `sizes` and `window` have the right number of elements.

   // Save input data
   Image in = c_in; // Not quick copy, so we keep the color space info and pixel size info for later

   // Prepare output image
   Tensor tensor = in.Tensor();
   bool expandTensor = false;
   if( !tensor.HasNormalOrder() && options.Contains( Option::ExtendImage::ExpandTensor )) {
      expandTensor = true;
      tensor = { tensor.Rows(), tensor.Columns() };
   }
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, tensor.Elements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW ));
   out.ReshapeTensor( tensor );
   out.SetPixelSize( in.PixelSize() );
   if( !expandTensor ) {
      out.SetColorSpace( in.ColorSpace() );
   }

   // Fix window
   for( dip::uint ii = 0; ii < window.size(); ++ii ) {
      window[ ii ].Fix( sizes[ ii ] );
   }

   // Copy input data to output
   Image tmp = out.At( window );
   tmp.Protect();
   if( expandTensor ) {
      ExpandTensor( in, tmp );
   } else {
      Copy( in, tmp );
   }

   // Extend the boundaries, one dimension at a time
   DIP_STACK_TRACE_THIS( ExtendRegion( out, window, std::move( boundaryConditions )));

   // Produce output by either using `out` directly or making a window of the original size over it.
   if( options.Contains( Option::ExtendImage::Masked )) {
      UnsignedArray offset( window.size() );
      for( dip::uint ii = 0; ii < window.size(); ++ii ) {
         offset[ ii ] = window[ ii ].Offset();
      }
      out.ShiftOriginUnsafe( out.Offset( offset )); // TODO: offset calculation does tests that are not necessary.
      out.SetSizesUnsafe( in.Sizes() );
   }
}

} // namespace

void ExtendImage(
      Image const& in,
      Image& out,
      UnsignedArray borderSizes, // by copy so we can modify it
      BoundaryConditionArray boundaryConditions,
      Option::ExtendImageFlags options
) {
   // Test input arguments
   dip::uint nDims;
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( borderSizes.empty(), E::ARRAY_PARAMETER_EMPTY );

   // The output sizes
   nDims = in.Dimensionality();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( borderSizes, nDims ));
   UnsignedArray sizes = in.Sizes();
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      sizes[ ii ] += 2 * borderSizes[ ii ];
   }

   // The view on the output image that matches the input
   RangeArray window( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint b = static_cast< dip::sint >( borderSizes[ ii ] );
      window[ ii ] = Range{ b, -b-1 };
   }

   DIP_STACK_TRACE_THIS( ExtendImage( in, out, sizes, std::move( window ), std::move( boundaryConditions ), options ));
}

void ExtendImage(
      Image const& in,
      Image& out,
      UnsignedArray borderSizes,
      StringArray const& boundaryConditions,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryConditions );
      Option::ExtendImageFlags opts = TranslateExtendImageFlags( options );
      ExtendImage( in, out, std::move( borderSizes ), bc, opts );
   DIP_END_STACK_TRACE
}

void ExtendImageToSize(
      Image const& in,
      Image& out,
      UnsignedArray const& sizes,
      Option::CropLocation cropLocation,
      BoundaryConditionArray boundaryConditions,
      Option::ExtendImageFlags options
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes.size() != in.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_START_STACK_TRACE
      RangeArray window = Image::CropWindow( sizes, in.Sizes(), cropLocation );
      ExtendImage( in, out, sizes, std::move( window ), std::move( boundaryConditions ), options );
   DIP_END_STACK_TRACE
}

void ExtendImageToSize(
      Image const& in,
      Image& out,
      UnsignedArray const& sizes,
      String const& cropLocation,
      StringArray const& boundaryConditions,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes.size() != in.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryConditions );
      Option::ExtendImageFlags opts = TranslateExtendImageFlags( options );
      RangeArray window = Image::CropWindow( sizes, in.Sizes(), cropLocation );
      ExtendImage( in, out, sizes, std::move( window ), bc, opts );
   DIP_END_STACK_TRACE
}

void ExtendRegion(
      Image& image,
      RangeArray ranges,
      BoundaryConditionArray boundaryConditions
) {
   // Test input arguments
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = image.Dimensionality();
   DIP_THROW_IF( ranges.empty(), E::ARRAY_PARAMETER_EMPTY );
   DIP_START_STACK_TRACE
      ArrayUseParameter( ranges, nDims );
      for( dip::uint dim = 0; dim < nDims; ++dim ) {
         ranges[ dim ].step = 1;
         ranges[ dim ].Fix( image.Size( dim )); // could throw
      }
      BoundaryArrayUseParameter( boundaryConditions, nDims );
   DIP_END_STACK_TRACE
   // Extend the boundaries, one dimension at a time
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      dip::uint left = ranges[ dim ].Offset();
      dip::uint right = image.Size( dim ) - 1 - static_cast< dip::uint >( ranges[ dim ].stop );
      if(( left > 0 ) || ( right > 0 )) {
         Image tmp = image.At( ranges );
         // Iterate over all image lines along this dimension
         // The iterator iterates over the lines with data only
         GenericImageIterator<> it( tmp, dim );
         do {
            // This is the function that does the actual boundary extension. It's defined in copy_buffer.cpp
            detail::ExpandBuffer(
                  it.Pointer(),
                  tmp.DataType(),
                  tmp.Stride( dim ),
                  tmp.TensorStride(),
                  tmp.Size( dim ),
                  tmp.TensorElements(),
                  left,
                  right,
                  boundaryConditions[ dim ]
            );
         } while( ++it );
         ranges[ dim ] = Range{}; // expand the tmp image to cover the newly written data
      }
   }
}

} // namespace dip
