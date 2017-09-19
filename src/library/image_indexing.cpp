/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2017, Cris Luengo.
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


namespace dip {


Image::View Image::Diagonal() const {
   dip::Tensor tensor = tensor_;
   dip::sint step = 1;
   tensor.ExtractDiagonal( step );
   Range out{ 0, static_cast< dip::sint >( tensor.Elements() - 1 ) * step, static_cast< dip::uint >( step ) };
   DIP_STACK_TRACE_THIS( return Image::View( *this, out ));
}

Image::View Image::TensorRow( dip::uint index ) const {
   dip::Tensor tensor = tensor_;
   dip::sint step = 1;
   dip::sint offset;
   DIP_STACK_TRACE_THIS( offset = tensor.ExtractRow( index, step ));
   Range out{ offset, offset + static_cast< dip::sint >( tensor.Elements() - 1 ) * step, static_cast< dip::uint >( step ) };
   DIP_STACK_TRACE_THIS( return Image::View( *this, out ));
}

Image::View Image::TensorColumn( dip::uint index ) const {
   dip::Tensor tensor = tensor_;
   dip::sint step = 1;
   dip::sint offset;
   DIP_STACK_TRACE_THIS( offset = tensor.ExtractColumn( index, step ));
   Range out{ offset, offset + static_cast< dip::sint >( tensor.Elements() - 1 ) * step, static_cast< dip::uint >( step ) };
   DIP_STACK_TRACE_THIS( return Image::View( *this, out ));
}

Image::Pixel Image::At( dip::uint index ) const {
   if( index == 0 ) { // shortcut to the first pixel
      return Pixel( Origin(), dataType_, tensor_, tensorStride_ );
   } else if( sizes_.size() < 2 ) {
      dip::uint n = sizes_.size() == 0 ? 1 : sizes_[ 0 ];
      DIP_THROW_IF( index >= n, E::INDEX_OUT_OF_RANGE );
      return Pixel( Pointer( static_cast< dip::sint >( index ) * strides_[ 0 ] ),
                    dataType_, tensor_, tensorStride_ );
   } else {
      return At( IndexToCoordinates( index ));
   }
}

Image::Pixel Image::At( dip::uint x_index, dip::uint y_index ) const {
   DIP_THROW_IF( sizes_.size() != 2, E::ILLEGAL_DIMENSIONALITY );
   DIP_THROW_IF( x_index >= sizes_[ 0 ], E::INDEX_OUT_OF_RANGE );
   DIP_THROW_IF( y_index >= sizes_[ 1 ], E::INDEX_OUT_OF_RANGE );
   return Pixel( Pointer( static_cast< dip::sint >( x_index ) * strides_[ 0 ] +
                          static_cast< dip::sint >( y_index ) * strides_[ 1 ] ),
                 dataType_, tensor_, tensorStride_ );
}

Image::Pixel Image::At( dip::uint x_index, dip::uint y_index, dip::uint z_index ) const {
   DIP_THROW_IF( sizes_.size() != 3, E::ILLEGAL_DIMENSIONALITY );
   DIP_THROW_IF( x_index >= sizes_[ 0 ], E::INDEX_OUT_OF_RANGE );
   DIP_THROW_IF( y_index >= sizes_[ 1 ], E::INDEX_OUT_OF_RANGE );
   DIP_THROW_IF( z_index >= sizes_[ 2 ], E::INDEX_OUT_OF_RANGE );
   return Pixel( Pointer( static_cast< dip::sint >( x_index ) * strides_[ 0 ] +
                          static_cast< dip::sint >( y_index ) * strides_[ 1 ] +
                          static_cast< dip::sint >( z_index ) * strides_[ 2 ] ),
                 dataType_, tensor_, tensorStride_ );
}

Image::View Image::Cropped( UnsignedArray const& sizes, Option::CropLocation cropLocation ) const {
   Image tmp = *this;
   tmp.Crop( sizes, cropLocation );
   return View( tmp );
}

Image::View Image::Cropped( UnsignedArray const& sizes, String const& cropLocation ) const {
   Image tmp = *this;
   tmp.Crop( sizes, cropLocation );
   return View( tmp );
}

Image::View Image::Real() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image::View out( *this );
   // Change data type
   out.reference_.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.reference_.strides_[ ii ] *= 2;
   }
   out.reference_.tensorStride_ *= 2;
   return out;
}

Image::View Image::Imaginary() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image::View out( *this );
   // Change data type
   out.reference_.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.reference_.strides_[ ii ] *= 2;
   }
   out.reference_.tensorStride_ *= 2;
   // Change the offset
   out.reference_.origin_ = out.reference_.Pointer( 1 );
   return out;
}

void DefineROI(
      Image const& src,
      Image& dest,
      UnsignedArray origin,
      UnsignedArray sizes,
      UnsignedArray spacing
) {
   DIP_THROW_IF( !src.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint n = src.Dimensionality();
   ArrayUseParameter( origin, n, dip::uint( 0 ));
   if( sizes.empty() ) {
      // computing: sizes = src.Sizes() - origin;
      sizes.resize( n );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         sizes[ ii ] = src.Size( ii ) - origin[ ii ];
      }
   } else {
      ArrayUseParameter( sizes, n, dip::uint( 1 ) );
   }
   ArrayUseParameter( spacing, n, dip::uint( 1 ));
   RangeArray ranges( n );
   for( dip::uint ii = 0; ii < n; ++ii ) {
      ranges[ ii ] = Range( static_cast< dip::sint >( origin[ ii ] ), static_cast< dip::sint >( sizes[ ii ] + origin[ ii ] - 1 ), spacing[ ii ] );
   }
   dest.Strip(); // strip output image to make sure data is not copied into it.
   dest = src.At( ranges );
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing image indexing") {
   // Note that this also tests parts of dip::Image::View and dip::Image::Pixel functionality, which is
   // tested more thoroughly in image_views.cpp

   dip::Image img{ dip::UnsignedArray{ 15, 20, 10 }, 3 };
   DOCTEST_REQUIRE( img.IsForged() );
   DOCTEST_REQUIRE( img.Size( 0 ) == 15 );
   DOCTEST_REQUIRE( img.Size( 1 ) == 20 );
   DOCTEST_REQUIRE( img.Size( 2 ) == 10 );
   DOCTEST_REQUIRE( img.NumberOfPixels() == 15 * 20 * 10 );
   DOCTEST_REQUIRE( img.TensorElements() == 3 );
   img.Fill( 0 );
   // Tensor indexing and linear spatial indexing
   img[ 0 ].At( 6 ) = 4.0;
   img[ 1 ].At( 6 ) = 5.0;
   img[ 0 ].At( 7 ) = 6.0;
   DOCTEST_CHECK( img.At( 6 )[ 0 ] == 4 );
   DOCTEST_CHECK( img.At( 6 )[ 1 ] == 5 );
   DOCTEST_CHECK( img.At( 7 )[ 0 ] == 6 );
   DOCTEST_CHECK_THROWS( img.At( img.NumberOfPixels() ));
   DOCTEST_CHECK_THROWS( img[ 4 ] );
   // Indexing in a 1D image, also tests range with negative values
   dip::Image img2 = img;
   img2.Flatten();
   img2.At( dip::Range{ -1 } ) = 8.0;
   DOCTEST_CHECK( img2.At( img2.NumberOfPixels() - 1 ) == 8 );
   // Creating a window
   img2 = img.At( dip::Range{ 5, 10 }, dip::Range{ 0, -1, 2 }, dip::Range{ -1, 6 } );
   DOCTEST_CHECK( img2.Sizes() == dip::UnsignedArray( { 6, 10, 4 } ));
   DOCTEST_CHECK( img2.TensorElements() == 3 );
   img2.Fill( 20 );
   // Tests that the window shares data, and that indexing with coordinates works
   DOCTEST_CHECK( img.At( 6, 2, 6 ) == 20 );
   DOCTEST_CHECK( img.At( 6, 1, 6 ) == 0 );
   // Tests Cropped
   img.Fill( 0 );
   img.At( 15/2, 20/2, 10/2 ) = 1;
   dip::Image cropped = img.Cropped( { 10, 10, 9 } );
   DOCTEST_CHECK( cropped.At( 10/2, 10/2, 9/2 ) == 1 );
   // Tests Real/Imaginary
   img = dip::Image{ dip::UnsignedArray{ 15, 20, 10 }, 3, dip::DT_SCOMPLEX };
   img.Real().Fill( 45.2 );
   img.Imaginary().Fill( 24.5 );
   DOCTEST_CHECK( img.At( 0 )[ 0 ] == dip::scomplex{ 45.2f, 24.5f } );
   DOCTEST_CHECK( img.At( 10, 15, 2 )[ 0 ] == dip::scomplex{ 45.2f, 24.5f } );
}

#endif // DIP__ENABLE_DOCTEST
