/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2018, Cris Luengo.
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

#include <algorithm>
#include <iostream>

#include "diplib.h"

namespace dip {

Image& Image::PermuteDimensions( UnsignedArray const& order ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( order.size() > nd, E::ARRAY_PARAMETER_WRONG_LENGTH );
   BooleanArray keep( nd, false );
   dip::uint newnd = order.size();
   for( dip::uint ii = 0; ii < newnd; ++ii ) {
      DIP_THROW_IF( order[ ii ] >= nd, E::ILLEGAL_DIMENSION );
      DIP_THROW_IF( keep[ order[ ii ]], "Cannot duplicate a dimension" );
      keep[ order[ ii ]] = true;
   }
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      DIP_THROW_IF( !keep[ ii ] && sizes_[ ii ] > 1, "Cannot discard non-singleton dimension" );
   }
   sizes_ = sizes_.permute( order );
   strides_ = strides_.permute( order );
   if( pixelSize_.IsDefined() ) {
      dip::PixelSize newpixelsz;
      for( dip::uint ii = 0; ii < newnd; ++ii ) {
         newpixelsz.Set( ii, pixelSize_[ order[ ii ]] );
      }
      pixelSize_ = newpixelsz;
   }
   return *this;
}


Image& Image::SwapDimensions( dip::uint dim1, dip::uint dim2 ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF(( dim1 >= nd ) || ( dim2 >= nd ), E::ILLEGAL_DIMENSION );
   if( dim1 != dim2 ) {
      std::swap( sizes_[ dim1 ], sizes_[ dim2 ] );
      std::swap( strides_[ dim1 ], strides_[ dim2 ] );
      pixelSize_.SwapDimensions( dim1, dim2 );
   }
   return *this;
}


Image& Image::Flatten() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint stride;
   void* p;
   std::tie( stride, p ) = GetSimpleStrideAndOrigin();
   if( !p ) {
      // The image has no simple stride -- copy the samples over to a new data segment
      Image newimg;
      newimg.CopyProperties( *this );
      newimg.strides_.clear(); // reset strides so Forge() fills out normal strides
      newimg.Forge();
      newimg.Copy( *this ); // TODO: why not directly forge a 1D image?
      std::tie( stride, p ) = newimg.GetSimpleStrideAndOrigin();
      DIP_THROW_IF( !p, "Copying over the image data didn't yield simple strides" );
      this->move( std::move( newimg ));
   }
   strides_ = { stride };
   sizes_ = { NumberOfPixels() };
   origin_ = p;
   if( pixelSize_.IsIsotropic() ) {
      pixelSize_.Resize( 1 );  // if all sizes are identical, keep first one only
   } else {
      pixelSize_.Clear();      // else set the pixel size to 'undefined'
   }
   return *this;
}

Image& Image::FlattenAsMuchAsPossible() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint stride;
   void* p;
   std::tie( stride, p ) = GetSimpleStrideAndOrigin();
   if( p ) {
      strides_ = { stride };
      sizes_ = { NumberOfPixels() };
      origin_ = p;
   } else {
      StandardizeStrides(); // Re-order strides
      UnsignedArray sizes{ sizes_[ 0 ] };
      IntegerArray strides{ strides_[ 0 ] };
      dip::uint jj = 0;
      for( dip::uint ii = 1; ii < sizes_.size(); ++ii ) {
         if( static_cast< dip::sint >( sizes[ jj ] ) * strides[ jj ] == strides_[ ii ] ) {
            sizes[ jj ] *= sizes_[ ii ];
         } else {
            ++jj;
            sizes.push_back( sizes_[ ii ] );
            strides.push_back( strides_[ ii ] ); // Using push_back in the hopes that there are no more than 4 output dimensions, this will be slow otherwise
         }
      }
      sizes_ = std::move( sizes );
      strides_ = std::move( strides );
   }
   if( pixelSize_.IsIsotropic() ) {
      pixelSize_.Resize( 1 );  // if all sizes are identical, keep first one only
   } else {
      pixelSize_.Clear();      // else set the pixel size to 'undefined'
   }
   return *this;
}


Image& Image::Squeeze() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint jj = 0;
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      if( sizes_[ ii ] > 1 ) {
         strides_[ jj ] = strides_[ ii ];
         sizes_[ jj ] = sizes_[ ii ];
         pixelSize_.Set( jj, pixelSize_[ ii ] );
         ++jj;
      }
   }
   strides_.resize( jj );
   sizes_.resize( jj );
   pixelSize_.Resize( jj );
   return *this;
}


Image& Image::Squeeze( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF(( dim >= nd ) || ( sizes_[ dim ] != 1 ), E::INVALID_PARAMETER );
   for( dip::uint ii = dim + 1; ii < sizes_.size(); ++ii ) {
      strides_[ ii - 1 ] = strides_[ ii ];
      sizes_[ ii - 1 ] = sizes_[ ii ];
      pixelSize_.Set( ii - 1, pixelSize_[ ii ] );
   }
   strides_.resize( nd - 1 );
   sizes_.resize( nd - 1 );
   pixelSize_.Resize( nd - 1 );
   return *this;
}


Image& Image::AddSingleton( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( dim > nd, E::INVALID_PARAMETER );
   sizes_.insert( dim, 1 );
   strides_.insert( dim, 0 );
   pixelSize_.InsertDimension( dim );
   // We set added singleton dimensions to 0 stride. The value is
   // irrelevant, but we use this as a flag for added singletons
   // in the Image::Aliases() function.
   return *this;
}


Image& Image::ExpandDimensionality( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   if( sizes_.size() < dim ) {
      sizes_.resize( dim, 1 );
      strides_.resize( dim, 0 ); // follow same convention as in AddSingleton().
      // Not setting the pixel sizes for these dimensions. If the pixel was isotropic,
      // it continues to be. Otherwise, the last dimension's size is repeated for the
      // new dimensions.
   }
   return *this;
}


Image& Image::ExpandSingletonDimension( dip::uint dim, dip::uint sz ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( sizes_.size() <= dim, E::ILLEGAL_DIMENSION );
   DIP_THROW_IF( sizes_[ dim ] != 1, E::INVALID_PARAMETER );
   sizes_[ dim ] = sz;
   strides_[ dim ] = 0;
   return *this;
}

Image& Image::ExpandSingletonDimensions( UnsignedArray const& newSizes ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint ndims = newSizes.size();
   DIP_THROW_IF( sizes_.size() > ndims, E::DIMENSIONALITIES_DONT_MATCH );
   DIP_THROW_IF( !IsSingletonExpansionPossible( newSizes ), E::SIZES_DONT_MATCH );
   if( sizes_.size() < ndims ) {
      ExpandDimensionality( ndims );
   }
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      if( sizes_[ ii ] != newSizes[ ii ] ) {
         ExpandSingletonDimension( ii, newSizes[ ii ] );
      }
   }
   return *this;
}

Image& Image::UnexpandSingletonDimensions() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   if( tensorStride_ == 0 ) {
      tensor_.SetScalar();
   }
   dip::uint ndims = sizes_.size();
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      if( strides_[ ii ] == 0 ) {
         sizes_[ ii ] = 1; // we leave the stride at 0, it's irrelevant.
      }
   }
   return *this;
}

bool Image::IsSingletonExpansionPossible( UnsignedArray const& newSizes ) const {
   if( sizes_.size() > newSizes.size() ) {
      return false;
   }
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      if(( sizes_[ ii ] != newSizes[ ii ] ) && ( sizes_[ ii ] != 1 )) {
         return false;
      }
   }
   return true;
}

Image& Image::ExpandSingletonTensor( dip::uint sz ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( tensor_.Elements() != 1, E::INVALID_PARAMETER );
   tensor_.SetVector( sz );
   tensorStride_ = 0;
   return *this;
}


Image& Image::Mirror( BooleanArray process ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( process, nd, true ));
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      if( process[ ii ] ) {
         origin_ = Pointer( static_cast< dip::sint >( sizes_[ ii ] - 1 ) * strides_[ ii ] );
         strides_[ ii ] = -strides_[ ii ];
      }
   }
   return *this;
}


Image& Image::Rotation90( dip::sint n, dip::uint dimension1, dip::uint dimension2 ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF(( dimension1 >= nd ) || ( dimension2 >= nd ) || ( dimension1 == dimension2 ), E::PARAMETER_OUT_OF_RANGE );
   n = n % 4;
   if( n < 0 ) {
      n += 4;
   }
   BooleanArray process( nd, false );
   switch( n ) {
      default:
      case 0:
         // Do nothing
         break;
      case 1: // 90 degrees clockwise
         process[ dimension2 ] = true;
         Mirror( process );
         SwapDimensions( dimension1, dimension2 );
         break;
      case 2: // 180 degrees
         process[ dimension1 ] = true;
         process[ dimension2 ] = true;
         Mirror( process );
         break;
      case 3: // 270 degrees (== 90 degrees counter-clockwise)
         process[ dimension1 ] = true;
         Mirror( process );
         SwapDimensions( dimension1, dimension2 );
         break;
   }
   return *this;
}


Image& Image::StandardizeStrides() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   // Un-expand tensor dimension
   if( tensorStride_ == 0 ) {
      tensor_.SetScalar();
   }
   // Un-mirror and un-expand spatial dimensions, sort strides, and remove singleton dimensions
   UnsignedArray order;
   dip::sint offset;
   std::tie( order, offset ) = StandardizeStrides( strides_, sizes_ );
   // Modify origin
   origin_ = Pointer( offset );
   // Permute all relevant arrays
   sizes_ = sizes_.permute( order );
   strides_ = strides_.permute( order );
   pixelSize_.Permute( order );
   return *this;
}

std::pair< UnsignedArray, dip::sint > Image::StandardizeStrides( IntegerArray& strides, UnsignedArray& sizes ) {
   dip::uint nd = sizes.size();
   DIP_ASSERT( strides.size() == nd );
   // Un-mirror and un-expand
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      if( strides[ ii ] < 0 ) {
         offset += static_cast< dip::sint >( sizes[ ii ] - 1 ) * strides[ ii ];
         strides[ ii ] = -strides[ ii ];
      } else if( strides[ ii ] == 0 ) {
         sizes[ ii ] = 1;
      }
   }
   // Sort strides
   UnsignedArray order = strides.sorted_indices();
   // Remove singleton dimensions
   dip::uint jj = 0;
   for( dip::uint ii = 0; ii < order.size(); ++ii ) {
      if( sizes[ order[ ii ]] > 1 ) {
         order[ jj ] = order[ ii ];
         ++jj;
      }
   }
   order.resize( jj );
   return std::make_pair( order, offset );
};


Image& Image::TensorToSpatial( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( dim > nd, E::INVALID_PARAMETER );
   sizes_.insert( dim, tensor_.Elements() );
   strides_.insert( dim, tensorStride_ );
   pixelSize_.InsertDimension( dim );
   tensor_.SetScalar();
   tensorStride_ = 1;
   ResetColorSpace();
   return *this;
}


Image& Image::SpatialToTensor( dip::uint dim, dip::uint rows, dip::uint cols ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( dim >= nd, E::INVALID_PARAMETER );
   if(( rows == 0 ) && ( cols == 0 )) {
      rows = sizes_[ dim ];
      cols = 1;
   } else if( rows == 0 ) {
      rows = sizes_[ dim ] / cols;
   } else if( cols == 0 ) {
      cols = sizes_[ dim ] / rows;
   }
   DIP_THROW_IF( sizes_[ dim ] != rows * cols, E::PARAMETER_OUT_OF_RANGE );
   tensor_.SetMatrix( rows, cols );
   tensorStride_ = strides_[ dim ];
   sizes_.erase( dim );
   strides_.erase( dim );
   pixelSize_.EraseDimension( dim );
   ResetColorSpace();
   return *this;
}


Image& Image::SplitComplex( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( dim > nd, E::INVALID_PARAMETER );
   // Change data type
   dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides_[ ii ] *= 2;
   }
   tensorStride_ *= 2;
   // Create new spatial dimension
   sizes_.insert( dim, 2 );
   strides_.insert( dim, 1 );
   pixelSize_.InsertDimension( dim );
   return *this;
}

Image& Image::MergeComplex( dip::uint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( dim >= nd, E::INVALID_PARAMETER );
   DIP_THROW_IF(( sizes_[ dim ] != 2 ) || ( strides_[ dim ] != 1 ), E::SIZES_DONT_MATCH );
   // Change data type
   dataType_ = dataType_ == DT_SFLOAT ? DT_SCOMPLEX : DT_DCOMPLEX;
   // Delete old spatial dimension
   sizes_.erase( dim );
   strides_.erase( dim );
   // Sample size is doubled, meaning all strides must be halved
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides_[ ii ] /= 2;
   }
   tensorStride_ /= 2;
   pixelSize_.EraseDimension( dim );
   return *this;
}

Image& Image::SplitComplexToTensor() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   // Change data type
   dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      strides_[ ii ] *= 2;
   }
   // Create new tensor dimension
   tensor_.SetVector( 2 );
   tensorStride_ = 1;
   ResetColorSpace();
   return *this;
}

Image& Image::MergeTensorToComplex() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF(( tensor_.Elements() != 2 ) || ( tensorStride_ != 1 ), E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   // Change data type
   dataType_ = dataType_ == DT_SFLOAT ? DT_SCOMPLEX : DT_DCOMPLEX;
   // Delete old tensor dimension
   tensor_.SetScalar();
   //tstride = 1; // was already the case
   // Sample size is doubled, meaning all strides must be halved
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      strides_[ ii ] /= 2;
   }
   ResetColorSpace();
   return *this;
}


Image& Image::Crop( UnsignedArray const& sizes, Option::CropLocation cropLocation ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = sizes_.size();
   DIP_THROW_IF( sizes.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
   DIP_THROW_IF( sizes > sizes_, E::INDEX_OUT_OF_RANGE );
   UnsignedArray origin( nDims, 0 );
   switch( cropLocation ) {
      case Option::CropLocation::CENTER:
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            dip::uint diff = sizes_[ ii ] - sizes[ ii ];
            origin[ ii ] = diff / 2 + (!( sizes_[ ii ] & 1 ) && ( sizes[ ii ] & 1 )); // add one if input is even in size and output is odd in size
         }
         break;
      case Option::CropLocation::MIRROR_CENTER:
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            dip::uint diff = sizes_[ ii ] - sizes[ ii ];
            origin[ ii ] = diff / 2 + (( sizes_[ ii ] & 1 ) && !( sizes[ ii ] & 1 )); // add one if input is odd in size and output is even in size
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
   origin_ = Pointer( origin );
   sizes_ = sizes;
   return *this;
}

Image& Image::Crop( UnsignedArray const& sizes, String const& cropLocation ) {
   Option::CropLocation flag;
   if( cropLocation == S::CENTER ) {
      flag = Option::CropLocation::CENTER;
   } else if( cropLocation == S::MIRROR_CENTER ) {
      flag = Option::CropLocation::MIRROR_CENTER;
   } else if( cropLocation == S::TOP_LEFT ) {
      flag = Option::CropLocation::TOP_LEFT;
   } else if( cropLocation == S::BOTTOM_RIGHT ) {
      flag = Option::CropLocation::BOTTOM_RIGHT;
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   return Crop( sizes, flag );
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing dip::Image dimension manipulation functions") {
   dip::Image src( { 5, 10, 15, }, 3 );
   dip::Image img = src;
   void* origin = src.Origin();
   DOCTEST_REQUIRE( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_REQUIRE( img.Strides() == dip::IntegerArray{ 3, 15, 150 } );
   DOCTEST_REQUIRE( img.Origin() == origin );
   img.SwapDimensions( 1, 2 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 15, 10 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 150, 15 } );
   DOCTEST_CHECK( img.Origin() == origin );
   img.PermuteDimensions( { 2, 1, 0 } );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 10, 15, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 15, 150, 3 } );
   DOCTEST_CHECK( img.Origin() == origin );
   img.StandardizeStrides();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 15, 150 } );
   DOCTEST_CHECK( img.Origin() == origin );
   DOCTEST_CHECK_THROWS( img.PermuteDimensions( { 0, 1 } ));
   DOCTEST_CHECK_THROWS( img.PermuteDimensions( { 0, 1, 2, 3 } ));
   img.Flatten();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5 * 10 * 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3 } );
   DOCTEST_CHECK( img.Origin() == origin );
   img = src;
   img.Mirror( { true, false, false } );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ -3, 15, 150 } );
   DOCTEST_CHECK( img.Origin() != origin );
   img.Rotation90( 1, 0, 2 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 15, 10, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ -150, 15, -3 } );
   DOCTEST_CHECK( img.Origin() != origin );
   img.StandardizeStrides();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 15, 150 } );
   DOCTEST_CHECK( img.Origin() == origin );
   img.FlattenAsMuchAsPossible();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5 * 10 * 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3 } );
   DOCTEST_CHECK( img.Origin() == origin );
   img = src.At( dip::RangeArray{ {}, { 5, 9 }, { 7, 11 } } );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 5, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 15, 150 } );
   DOCTEST_CHECK( img.Origin() != origin );
   img.FlattenAsMuchAsPossible();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5 * 5, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 150 } );
   DOCTEST_CHECK( img.Origin() != origin );
}

DOCTEST_TEST_CASE("[DIPlib] testing dip::Image tensor dimension manipulation functions") {
   dip::Image src( { 5, 10, 15, }, 3, dip::DT_SCOMPLEX );
   dip::Image img = src;
   DOCTEST_REQUIRE( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_REQUIRE( img.Strides() == dip::IntegerArray{ 3, 15, 150 } );
   DOCTEST_REQUIRE( img.TensorElements() == 3 );
   DOCTEST_REQUIRE( img.TensorStride() == 1 );
   DOCTEST_REQUIRE( img.DataType() == dip::DT_SCOMPLEX );
   img.TensorToSpatial( 1 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 3, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 3, 1, 15, 150 } );
   DOCTEST_CHECK( img.TensorElements() == 1 );
   img.SpatialToTensor( 0 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 3, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 15, 150 } );
   DOCTEST_CHECK( img.TensorElements() == 5 );
   DOCTEST_CHECK( img.TensorStride() == 3 );
   img.SplitComplex();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 3, 10, 15, 2 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1*2, 15*2, 150*2, 1 } );
   DOCTEST_CHECK( img.TensorElements() == 5 );
   DOCTEST_CHECK( img.TensorStride() == 3*2 );
   img.MergeComplex();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 3, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 15, 150 } );
   DOCTEST_CHECK( img.TensorElements() == 5 );
   DOCTEST_CHECK( img.TensorStride() == 3 );
   img.TensorToSpatial();
   img.SplitComplexToTensor();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 3, 10, 15, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1*2, 15*2, 150*2, 3*2 } );
   DOCTEST_CHECK( img.TensorElements() == 2 );
   DOCTEST_CHECK( img.TensorStride() == 1 );
   img.MergeTensorToComplex();
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 3, 10, 15, 5 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 15, 150, 3 } );
   DOCTEST_CHECK( img.TensorElements() == 1 );
}

DOCTEST_TEST_CASE("[DIPlib] testing dip::Image singleton dimensions") {
   dip::Image src( { 5, 10, 15, }, 1 );
   dip::Image img = src;                                                      // Point 0
   DOCTEST_REQUIRE( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_REQUIRE( img.Strides() == dip::IntegerArray{ 1, 5, 50 } );
   DOCTEST_REQUIRE( img.TensorElements() == 1 );
   img.AddSingleton( 1 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 1, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50 } );
   img.ExpandDimensionality( 5 );                                             // Point A
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 1, 10, 15, 1 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50, 0 } );
   img.ExpandSingletonDimension( 1, 20 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 20, 10, 15, 1 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50, 0 } );
   img.ExpandSingletonDimension( 4, 25 );
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 20, 10, 15, 25 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50, 0 } );
   img.ExpandSingletonTensor( 3 );                                            // Point B
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 20, 10, 15, 25 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50, 0 } );
   DOCTEST_CHECK( img.TensorElements() == 3 );
   DOCTEST_CHECK( img.TensorStride() == 0 );
   img.UnexpandSingletonDimensions();                                         // Point A
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 1, 10, 15, 1 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 0, 5, 50, 0 } );
   DOCTEST_CHECK( img.TensorElements() == 1 );
   img.Squeeze();                                                             // Point 0
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 5, 50 } );
   DOCTEST_CHECK( img.TensorElements() == 1 );
   img.AddSingleton( 1 );
   img.ExpandDimensionality( 5 );
   img.ExpandSingletonDimension( 1, 20 );
   img.ExpandSingletonDimension( 4, 25 );
   img.ExpandSingletonTensor( 3 );                                            // Point B
   img.StandardizeStrides();                                                  // Point 0
   DOCTEST_CHECK( img.Sizes() == dip::UnsignedArray{ 5, 10, 15 } );
   DOCTEST_CHECK( img.Strides() == dip::IntegerArray{ 1, 5, 50 } );
   DOCTEST_CHECK( img.TensorElements() == 1 );
}

#include "diplib/generation.h"

DOCTEST_TEST_CASE("[DIPlib] testing dip::Image::Crop") {
   // We test all 4 combinations of cropping {even|odd} input to {even|odd} output.
   dip::Image src( { 7, 8, 9, 10 }, 1 );
   dip::FillDelta( src, "right" );
   DOCTEST_REQUIRE( src.At<float>( dip::UnsignedArray{ 3, 4, 4, 5 } ) == 1.0 );
   DOCTEST_REQUIRE( src.At<float>( dip::UnsignedArray{ 3, 3, 4, 4 } ) == 0.0 );
   dip::Image img = src.QuickCopy();
   img.Crop( { 6, 6, 7, 7 }, dip::Option::CropLocation::CENTER ); // keeps the pixel right of center on its place
   DOCTEST_CHECK( img.At<float>( dip::UnsignedArray{ 3, 3, 3, 3 } ) == 1.0 );
   dip::FillDelta( src, "left" );
   DOCTEST_REQUIRE( src.At<float>( dip::UnsignedArray{ 3, 4, 4, 5 } ) == 0.0 );
   DOCTEST_REQUIRE( src.At<float>( dip::UnsignedArray{ 3, 3, 4, 4 } ) == 1.0 );
   img = src.QuickCopy();
   img.Crop( { 6, 6, 7, 7 }, dip::Option::CropLocation::MIRROR_CENTER ); // keeps the pixel left of center on its place
   DOCTEST_CHECK( img.At<float>( dip::UnsignedArray{ 2, 2, 3, 3 } ) == 1.0 );
}

#endif // DIP__ENABLE_DOCTEST
