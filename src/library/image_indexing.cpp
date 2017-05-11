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


Image Image::operator[]( UnsignedArray const& indices ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_START_STACK_TRACE
      dip::uint index = tensor_.Index( indices );
      return operator[]( index );
   DIP_END_STACK_TRACE
}

Image Image::operator[]( dip::uint index ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( index >= tensor_.Elements(), E::INDEX_OUT_OF_RANGE );
   Image out = *this;
   out.tensor_.SetScalar();
   out.origin_ = Pointer( static_cast< dip::sint >( index ) * tensorStride_ );
   out.ResetColorSpace();
   return out;
}

Image Image::Diagonal() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   Image out = *this;
   out.tensor_.ExtractDiagonal( out.tensorStride_ );
   if( out.tensor_.Elements() != tensor_.Elements() ) {
      out.ResetColorSpace();
   }
   return out;
}

Image Image::TensorRow( dip::uint index ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( index >= tensor_.Rows(), E::INDEX_OUT_OF_RANGE );
   Image out = *this;
   dip::sint offset = out.tensor_.ExtractRow( index, out.tensorStride_ );
   out.origin_ = Pointer( offset );
   if( out.tensor_.Elements() != tensor_.Elements() ) {
      out.ResetColorSpace();
   }
   return out;
}

Image Image::TensorColumn( dip::uint index ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( index >= tensor_.Columns(), E::INDEX_OUT_OF_RANGE );
   Image out = *this;
   dip::sint offset = out.tensor_.ExtractColumn( index, out.tensorStride_ );
   out.origin_ = Pointer( offset );
   if( out.tensor_.Elements() != tensor_.Elements() ) {
      out.ResetColorSpace();
   }
   return out;
}

Image Image::At( Range x_range ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes_.size() != 1, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   Image out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.pixelSize_.Scale( 0, static_cast< dfloat >( x_range.Step() ));
   out.origin_ = Pointer( static_cast< dip::sint >( x_range.Offset() ) * strides_[ 0 ] );
   return out;
}

Image Image::At( Range x_range, Range y_range ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes_.size() != 2, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   y_range.Fix( sizes_[ 1 ] );
   Image out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.sizes_[ 1 ] = y_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.strides_[ 1 ] *= y_range.Step();
   out.pixelSize_.Scale( 0, static_cast< dfloat >( x_range.Step() ));
   out.pixelSize_.Scale( 1, static_cast< dfloat >( y_range.Step() ));
   out.origin_ = Pointer(
         static_cast< dip::sint >( x_range.Offset() ) * strides_[ 0 ] +
         static_cast< dip::sint >( y_range.Offset() ) * strides_[ 1 ] );
   return out;
}

Image Image::At( Range x_range, Range y_range, Range z_range ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes_.size() != 3, E::ILLEGAL_DIMENSIONALITY );
   x_range.Fix( sizes_[ 0 ] );
   y_range.Fix( sizes_[ 1 ] );
   z_range.Fix( sizes_[ 2 ] );
   Image out = *this;
   out.sizes_[ 0 ] = x_range.Size();
   out.sizes_[ 1 ] = y_range.Size();
   out.sizes_[ 2 ] = z_range.Size();
   out.strides_[ 0 ] *= x_range.Step();
   out.strides_[ 1 ] *= y_range.Step();
   out.strides_[ 2 ] *= z_range.Step();
   out.pixelSize_.Scale( 0, static_cast< dfloat >( x_range.Step() ));
   out.pixelSize_.Scale( 1, static_cast< dfloat >( y_range.Step() ));
   out.pixelSize_.Scale( 2, static_cast< dfloat >( z_range.Step() ));
   out.origin_ = Pointer(
         static_cast< dip::sint >( x_range.Offset() ) * strides_[ 0 ] +
         static_cast< dip::sint >( y_range.Offset() ) * strides_[ 1 ] +
         static_cast< dip::sint >( z_range.Offset() ) * strides_[ 2 ] );
   return out;
}

Image Image::At( RangeArray ranges ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes_.size() != ranges.size(), E::ARRAY_ILLEGAL_SIZE );
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      ranges[ ii ].Fix( sizes_[ ii ] );
   }
   Image out = *this;
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      out.sizes_[ ii ] = ranges[ ii ].Size();
      out.strides_[ ii ] *= ranges[ ii ].Step();
      out.pixelSize_.Scale( ii, static_cast< dfloat >( ranges[ ii ].Step() ));
      offset += static_cast< dip::sint >( ranges[ ii ].Offset() ) * strides_[ ii ];
   }
   out.origin_ = Pointer( offset );
   return out;
}

Image Image::Crop( UnsignedArray const& sizes, Option::CropLocation cropLocation ) const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = sizes_.size();
   DIP_THROW_IF( sizes.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( sizes > sizes_, E::INDEX_OUT_OF_RANGE );
   Image out = *this;
   UnsignedArray origin( nDims, 0 );
   switch( cropLocation ) {
      case Option::CropLocation::CENTER:
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            origin[ ii ] = ( sizes_[ ii ] - sizes[ ii ] + ( sizes[ ii ] & 1 ) ) / 2; // add one if output is odd in size
         }
         break;
      case Option::CropLocation::MIRROR_CENTER:
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            origin[ ii ] = ( sizes_[ ii ] - sizes[ ii ] + ( sizes_[ ii ] & 1 ) ) / 2; // add one if input is odd in size
         }
         break;
      case Option::CropLocation::TOP_LEFT:
         // Origin stays at 0
         break;
      case Option::CropLocation::BOTTOM_RIGHT:
         origin = sizes_;
         origin -= sizes;
         break;
   }
   out.origin_ = out.Pointer( origin );
   out.sizes_ = sizes;
   return out;
}

Image Image::Real() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image out = *this;
   // Change data type
   out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.strides_[ ii ] *= 2;
   }
   out.tensorStride_ *= 2;
   return out;
}

Image Image::Imaginary() const {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   Image out = *this;
   // Change data type
   out.dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < strides_.size(); ++ii ) {
      out.strides_[ ii ] *= 2;
   }
   out.tensorStride_ *= 2;
   // Change the offset
   out.origin_ = out.Pointer( 1 );
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
   // Tests Crop
   img.Fill( 0 );
   img.At( 15/2, 20/2, 10/2 ) = 1;
   dip::Image cropped = img.Crop( { 10, 10, 9 } );
   DOCTEST_CHECK( cropped.At( 10/2, 10/2, 9/2 ) == 1 );
}

#endif // DIP__ENABLE_DOCTEST
