/*
 * (c)2016-2026, Cris Luengo.
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

#include "diplib/boundary.h"

#include <limits>
#include <utility>

#include "diplib.h"
#include "diplib/generic_iterators.h"
#include "diplib/library/copy_buffer.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
void SetValue( Image::Pixel& out, bool toMax ) {
   out = toMax ? std::numeric_limits< TPI >::max() : std::numeric_limits< TPI >::lowest();
}

void SetMaxValue( Image::Pixel& out ) {
   DIP_OVL_CALL_ALL( SetValue, ( out, true ), out.DataType() );
}

void SetMinValue( Image::Pixel& out ) {
   DIP_OVL_CALL_ALL( SetValue, ( out, false ), out.DataType() );
}

} // namespace

Image::Pixel ReadPixelWithBoundaryCondition(
      Image const& img,
      IntegerArray coords, // getting a local copy so we can modify it
      BoundaryConditionArray const& c_bc
) {
   DIP_THROW_IF( coords.size() != img.Dimensionality(), E::ARRAY_PARAMETER_WRONG_LENGTH );
   BoundaryConditionArray bc = c_bc;
   dip::BoundaryArrayUseParameter( bc, img.Dimensionality() );
   bool invert = false;
   Image::Pixel out( DataType::SuggestFlex( img.DataType() ), img.TensorElements() );
   out.ReshapeTensor( img.Tensor() );
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      dip::sint sz = static_cast< dip::sint >( img.Size( ii ));
      if(( coords[ ii ] < 0 ) || ( coords[ ii ] >= sz )) {
         switch( bc[ ii ] ) {
            case BoundaryCondition::SYMMETRIC_MIRROR: {
               dip::sint period = sz - 1;
               coords[ ii ] = modulo( coords[ ii ], 2 * period );
               if( coords[ ii ] >= sz ) {
                  coords[ ii ] = 2 * period - coords[ ii ];
               }
               break;
            }
            case BoundaryCondition::ASYMMETRIC_MIRROR: {
               bool onLeft = coords[ ii ] < 0;
               bool invert_again = false;
               dip::sint period = sz - 1;
               coords[ ii ] = modulo( coords[ ii ], 2 * period );
               if( coords[ ii ] >= sz ) {
                  coords[ ii ] = 2 * period - coords[ ii ];
                  invert_again = true; // In the second half of the 2*period region, we invert
               }
               if( coords[ ii ] == 0 ) {
                  // zero indices on the right are always inverted, on the left they re never
                  invert_again = !onLeft;
               } else if( coords[ ii ] == period ) {
                  // end indices  the left are always inverted, on the right they are never
                  invert_again = onLeft;
               }
               if( invert_again ) {
                  invert = !invert;
               }
               break;
            }
            case BoundaryCondition::PERIODIC:
               coords[ ii ] = modulo( coords[ ii ], sz );
               break;
            case BoundaryCondition::ASYMMETRIC_PERIODIC:
               coords[ ii ] = modulo( coords[ ii ], 2 * sz );
               if( coords[ ii ] >= sz ) {
                  coords[ ii ] -= sz;
                  invert = !invert; // In the second half of the 2*sz region, we invert
               }
               break;
            case BoundaryCondition::ADD_ZEROS:
               out = 0;
               return out; // We're done!
            case BoundaryCondition::ADD_MAX_VALUE:
               SetMaxValue( out );
               return out; // We're done!
            case BoundaryCondition::ADD_MIN_VALUE:
               SetMinValue( out );
               return out; // We're done!
            case BoundaryCondition::ZERO_ORDER_EXTRAPOLATE:
               coords[ ii ] = clamp( coords[ ii ], dip::sint( 0 ), sz - 1 );
               break;
            case BoundaryCondition::FIRST_ORDER_EXTRAPOLATE:
            case BoundaryCondition::SECOND_ORDER_EXTRAPOLATE:
            case BoundaryCondition::THIRD_ORDER_EXTRAPOLATE:
               // The definitions for the first, second and third order extrapolation depend on the size of the boundary region, which the user is not required to decide in this function.
            case BoundaryCondition::ANTISYMMETRIC_REFLECT:
               // TODO. This is hard, we need to compute the edge at each distance of sz-1 that we advance.
               //       And it requires computing multiple values, depending on how many dimensions are outside the domain.
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
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( borderSizes.empty(), E::ARRAY_PARAMETER_EMPTY );

   // The output sizes
   dip::uint nDims = in.Dimensionality();
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

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/random.h"

DOCTEST_TEST_CASE("[DIPlib] comparing the output of ReadPixelWithBoundaryCondition to that of ExtendImage") {
   dip::Image img( { 15, 10 }, 1, dip::DT_SFLOAT );
   img.Fill( 0 );
   dip::Random rng;
   dip::UniformNoise( img, img, rng, 0.5, 3.8 );
   dip::Image ext;
   for( auto bc : {
      dip::BoundaryCondition::SYMMETRIC_MIRROR,
      dip::BoundaryCondition::ASYMMETRIC_MIRROR,
      // dip::BoundaryCondition::ANTISYMMETRIC_REFLECT // Not yet implemented
      dip::BoundaryCondition::PERIODIC,
      dip::BoundaryCondition::ASYMMETRIC_PERIODIC,
      dip::BoundaryCondition::ADD_ZEROS,
      dip::BoundaryCondition::ADD_MAX_VALUE,
      dip::BoundaryCondition::ADD_MIN_VALUE,
      dip::BoundaryCondition::ZERO_ORDER_EXTRAPOLATE
      // dip::BoundaryCondition::FIRST_ORDER_EXTRAPOLATE // Will never be implemented
      // dip::BoundaryCondition::SECOND_ORDER_EXTRAPOLATE // Will never be implemented
      // dip::BoundaryCondition::THIRD_ORDER_EXTRAPOLATE // Will never be implemented
   } ) {
      dip::ExtendImage( img, ext, { 20, 15 }, { bc }, dip::Option::ExtendImage::Masked );
      dip::sfloat const* ext_ptr;
      ext_ptr = static_cast< dip::sfloat const* >( ext.Origin() );
      // We can index into `ext` from x = -20 to 14 + 20 = 34, and y = -15 to 9 + 15 = 24
      for( dip::IntegerArray coords : {
         // Inside image
         dip::IntegerArray{ 0, 0 },
         // Just outside on the left
         dip::IntegerArray{ -1, 0 },
         dip::IntegerArray{ 0, -1 },
         dip::IntegerArray{ -1, -1 },
         // Just outside on the right
         dip::IntegerArray{ 20, 14 },
         dip::IntegerArray{ 19, 15 },
         dip::IntegerArray{ 20, 15 },
         // Around the transition from the first to the second replicate in the case of mirror
         dip::IntegerArray{ 28, -9 },
         dip::IntegerArray{ 29, -9 },
         dip::IntegerArray{ 28, -10 },
         dip::IntegerArray{ 29, -10 },
         // Around the transition from the first to the second replicate in the case of periodic
         dip::IntegerArray{ 30, -10 },
         dip::IntegerArray{ 29, -11 },
         dip::IntegerArray{ 30, -11 },
      } ) {
            dip::Image::Pixel val;
            val = dip::ReadPixelWithBoundaryCondition( img, coords, { bc } );
            DOCTEST_CHECK( val.As< dip::sfloat >() == *( ext_ptr + ext.Offset( coords )));
      }
   }
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
