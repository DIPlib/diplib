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
      DIP_THROW_IF( keep[ order[ ii ] ], "Cannot duplicate a dimension" );
      keep[ order[ ii ] ] = true;
   }
   for( dip::uint ii = 0; ii < newnd; ++ii ) {
      DIP_THROW_IF( !keep[ ii ] && sizes_[ ii ] > 1, "Cannot discard non-singleton dimension" );
   }
   UnsignedArray newsizes( newnd, 0 );
   IntegerArray newstrides( newnd, 0 );
   dip::PixelSize newpixelsz;
   for( dip::uint ii = 0; ii < newnd; ++ii ) {
      newsizes[ ii ] = sizes_[ order[ ii ] ];
      newstrides[ ii ] = strides_[ order[ ii ] ];
      newpixelsz.Set( ii, pixelSize_[ order[ ii ] ] );
   }
   sizes_ = newsizes;
   strides_ = newstrides;
   pixelSize_ = newpixelsz;
   return *this;
}


Image& Image::SwapDimensions( dip::uint dim1, dip::uint dim2 ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( ( dim1 >= nd ) || ( dim2 >= nd ), E::ILLEGAL_DIMENSION );
   if( dim1 != dim2 ) {
      std::swap( sizes_[ dim1 ], sizes_[ dim2 ] );
      std::swap( strides_[ dim1 ], strides_[ dim2 ] );
      pixelSize_.SwapDimensions( dim1, dim2 );
   }
   return *this;
}


Image& Image::Flatten() {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint stride;
   void* p;
   GetSimpleStrideAndOrigin( stride, p );
   if( !p ) {
      // The image has no simple stride -- copy the samples over to a new data segment
      Image newimg;
      newimg.CopyProperties( *this );
      newimg.strides_.clear(); // reset strides so Forge() fills out normal strides
      newimg.Forge();
      newimg.Copy( *this );
      newimg.GetSimpleStrideAndOrigin( stride, p );
      DIP_THROW_IF( !p, "Copying over the image data didn't yield simple strides" );
      swap( newimg );
   }
   strides_ = { dip::sint( stride ) };
   sizes_ = { NumberOfPixels() };
   origin_ = p;
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
   if( sizes_.size() < ndims ) {
      ExpandDimensionality( ndims );
   }
   DIP_THROW_IF( !IsSingletonExpansionPossible( newSizes ), E::SIZES_DONT_MATCH );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      if( sizes_[ ii ] != newSizes[ ii ] ) {
         ExpandSingletonDimension( ii, newSizes[ ii ] );
      }
   }
   return *this;
}


bool Image::IsSingletonExpansionPossible( UnsignedArray const& newSizes ) const {
   if( sizes_.size() > newSizes.size() ) {
      return false;
   }
   for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
      if( ( sizes_[ ii ] != newSizes[ ii ] ) && ( sizes_[ ii ] != 1 ) ) {
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


Image& Image::Mirror( BooleanArray const& process ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   DIP_THROW_IF( process.size() != nd, E::ARRAY_ILLEGAL_SIZE );
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      if( process[ ii ] ) {
         origin_ = Pointer( ( sizes_[ ii ] - 1 ) * strides_[ ii ] );
         strides_[ ii ] = -strides_[ ii ];
      }
   }
   return *this;
}


Image& Image::TensorToSpatial( dip::sint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = sizes_.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint newdim = ( dip::uint )dim;
   DIP_THROW_IF( newdim > nd, E::INVALID_PARAMETER );
   sizes_.insert( newdim, tensor_.Elements() );
   strides_.insert( newdim, tensorStride_ );
   pixelSize_.InsertDimension( newdim );
   tensor_.SetScalar();
   tensorStride_ = 1;
   ResetColorSpace();
   return *this;
}


Image& Image::SpatialToTensor( dip::sint dim, dip::uint rows, dip::uint cols ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nd = sizes_.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint olddim = ( dip::uint )dim;
   DIP_THROW_IF( olddim >= nd, E::INVALID_PARAMETER );
   if( ( rows == 0 ) && ( cols == 0 ) ) {
      rows = sizes_[ dim ];
      cols = 1;
   } else if( rows == 0 ) {
      rows = sizes_[ olddim ] / cols;
   } else if( cols == 0 ) {
      cols = sizes_[ olddim ] / rows;
   }
   DIP_THROW_IF( sizes_[ olddim ] != rows * cols, E::PARAMETER_OUT_OF_RANGE );
   tensor_.SetMatrix( rows, cols );
   tensorStride_ = strides_[ olddim ];
   sizes_.erase( olddim );
   strides_.erase( olddim );
   pixelSize_.EraseDimension( olddim );
   ResetColorSpace();
   return *this;
}

Image& Image::SplitComplex( dip::sint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nd = sizes_.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint newdim = ( dip::uint )dim;
   DIP_THROW_IF( newdim > nd, E::INVALID_PARAMETER );
   // Change data type
   dataType_ = dataType_ == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides_[ ii ] *= 2;
   }
   tensorStride_ *= 2;
   // Create new spatial dimension
   sizes_.insert( newdim, 2 );
   strides_.insert( newdim, 1 );
   pixelSize_.InsertDimension( newdim );
   return *this;
}

Image& Image::MergeComplex( dip::sint dim ) {
   DIP_THROW_IF( !IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( dataType_.IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nd = sizes_.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint olddim = static_cast< dip::uint >( dim );
   DIP_THROW_IF( olddim >= nd, E::INVALID_PARAMETER );
   DIP_THROW_IF( ( sizes_[ olddim ] != 2 ) || ( strides_[ olddim ] != 1 ), E::SIZES_DONT_MATCH );
   // Change data type
   dataType_ = dataType_ == DT_SFLOAT ? DT_SCOMPLEX : DT_DCOMPLEX;
   // Delete old spatial dimension
   sizes_.erase( olddim );
   strides_.erase( olddim );
   // Sample size is doubled, meaning all strides must be halved
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides_[ ii ] /= 2;
   }
   tensorStride_ /= 2;
   pixelSize_.EraseDimension( olddim );
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
   DIP_THROW_IF( ( tensor_.Elements() != 2 ) || ( tensorStride_ != 1 ), E::NTENSORELEM_DONT_MATCH );
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

} // namespace dip
