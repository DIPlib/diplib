/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <algorithm>
#include <iostream>

#include "diplib.h"

namespace dip {

Image& Image::PermuteDimensions( const UnsignedArray& order ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( order.size() > nd, E::ARRAY_PARAMETER_WRONG_LENGTH );
   BooleanArray keep( nd, false );
   dip::uint newnd = order.size();
   for( dip::uint ii=0; ii<newnd; ++ii ) {
      dip_ThrowIf( order[ii] >= nd, E::ILLEGAL_DIMENSION );
      dip_ThrowIf( keep[ order[ii] ], "Cannot duplicate a dimension" );
      keep[ order[ii] ] = true;
   }
   for( dip::uint ii=0; ii<newnd; ++ii ) {
      dip_ThrowIf( !keep[ii] && dims[ii]>1, "Cannot discard non-singleton dimension" );
   }
   UnsignedArray newdims( newnd, 0 );
   IntegerArray  newstrides( newnd, 0 );
   dip::PixelSize     newpixelsz;
   for( dip::uint ii=0; ii<newnd; ++ii ) {
      newdims   [ii] = dims   [ order[ii] ];
      newstrides[ii] = strides[ order[ii] ];
      newpixelsz.Set( ii, pixelsize[ order[ii] ] );
   }
   dims      = newdims;
   strides   = newstrides;
   pixelsize = newpixelsz;
   return *this;
}


Image& Image::SwapDimensions( dip::uint d1, dip::uint d2 ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( (d1>=nd) || (d2>=nd), E::ILLEGAL_DIMENSION );
   std::swap( dims   [d1], dims   [d2] );
   std::swap( strides[d1], strides[d2] );
   pixelsize.SwapDimensions( d1, d2 );
   return *this;
}


Image& Image::Flatten() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint stride;
   void* p;
   GetSimpleStrideAndOrigin( stride, p );
   if( !p ) {
      // TODO: force a rewrite of the data with normal strides.
      // If there is no external interface:
      //    stride = tensor.Elements();
      //    p = origin;
      // Else:
      //    GetSimpleStrideAndOrigin( stride, p );
      dip_Throw("Not yet implemented for non-simple strides.");
   }
   strides = { dip::sint(stride) };
   dims = { NumberOfPixels() };
   origin = p;
   if( pixelsize.IsIsotropic() ) {
      pixelsize.Resize( 1 );  // if all sizes are identical, keep first one only
   } else {
      pixelsize.Clear();      // else set the pixel size to 'undefined'
   }
   return *this;
}


Image& Image::Squeeze() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint jj = 0;
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
      if( dims[ii] > 1) {
         strides[jj] = strides[ii];
         dims[jj] = dims[ii];
         pixelsize.Set( jj, pixelsize[ii] );
         ++jj;
      }
   }
   strides.resize( jj );
   dims.resize( jj );
   pixelsize.Resize( jj );
   return *this;
}


Image& Image::AddSingleton( dip::uint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( dim > nd, E::INVALID_PARAMETER );
   dims.insert( dim, 1 );
   strides.insert( dim, 0 );
   pixelsize.InsertDimension( dim );
   // We set added singleton dimensions to 0 stride. The value is
   // irrelevant, but we use this as a flag for added singletons
   // in the Image::Aliases() function.
   return *this;
}


Image& Image::ExpandDimensionality( dip::uint n ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( dims.size() < n ) {
      dims   .resize( n, 1 );
      strides.resize( n, 0 ); // follow same convention as in AddSingleton().
      // Not setting the pixel sizes for these dimensions. If the pixel was isotropic,
      // it continues to be. Otherwise, the last dimension's size is repeated for the
      // new dimensions.
   }
   return *this;
}


Image& Image::ExpandSingletonDimension( dip::uint dim, dip::uint sz ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( dims.size() <= dim, E::ILLEGAL_DIMENSION );
   dip_ThrowIf( dims[dim] != 1, E::INVALID_PARAMETER );
   dims   [dim] = sz;
   strides[dim] = 0;
   return *this;
}


Image& Image::Mirror( const BooleanArray& process ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( process.size() != nd, E::ARRAY_ILLEGAL_SIZE);
   for( dip::uint ii=0; ii<nd; ++ii ) {
      if( process[ii] ) {
         origin = Pointer( ( dims[ii] - 1 ) * strides[ii] );
         strides[ii] = -strides[ii];
      }
   }
   return *this;
}


Image& Image::TensorToSpatial( dip::sint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint newdim = (dip::uint)dim;
   dip_ThrowIf( newdim > nd, E::INVALID_PARAMETER );
   dims.insert( newdim, tensor.Elements() );
   strides.insert( newdim, tstride );
   pixelsize.InsertDimension( newdim );
   tensor.SetScalar();
   tstride = 1;
   ResetColorSpace();
   return *this;
}


Image& Image::SpatialToTensor( dip::sint dim, dip::uint rows, dip::uint cols ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !tensor.IsScalar(), E::NOT_SCALAR );
   dip::uint nd = dims.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint olddim = (dip::uint)dim;
   dip_ThrowIf( olddim >= nd, E::INVALID_PARAMETER );
   if ((rows == 0) && (cols == 0)) {
      rows = dims[dim];
      cols = 1;
   } else if (rows == 0) {
      rows = dims[olddim] / cols;
   } else if (cols == 0) {
      cols = dims[olddim] / rows;
   }
   dip_ThrowIf( dims[olddim] != rows * cols, E::PARAMETER_OUT_OF_RANGE );
   tensor.SetMatrix( rows, cols );
   tstride = strides[olddim];
   dims.erase( olddim );
   strides.erase( olddim );
   pixelsize.EraseDimension( olddim );
   ResetColorSpace();
   return *this;
}

Image& Image::SplitComplex( dip::sint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !datatype.IsComplex() , E::DATA_TYPE_NOT_SUPPORTED);
   dip::uint nd = dims.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint newdim = (dip::uint)dim;
   dip_ThrowIf( newdim > nd, E::INVALID_PARAMETER );
   // Change data type
   datatype = datatype == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides[ii] *= 2;
   }
   tstride *= 2;
   // Create new spatial dimension
   dims.insert( newdim, 2 );
   strides.insert( newdim, 1 );
   pixelsize.InsertDimension( newdim );
   return *this;
}

Image& Image::MergeComplex( dip::sint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( datatype.IsComplex() , E::DATA_TYPE_NOT_SUPPORTED);
   dip::uint nd = dims.size();
   if( dim < 0 ) {
      dim = nd;
   }
   dip::uint olddim = (dip::uint)dim;
   dip_ThrowIf( olddim >= nd, E::INVALID_PARAMETER );
   dip_ThrowIf( ( dims[olddim] != 2 ) || ( strides[olddim] != 1 ), E::DIMENSIONS_DONT_MATCH );
   // Change data type
   datatype = datatype == DT_SFLOAT ? DT_SCOMPLEX : DT_DCOMPLEX;
   // Delete old spatial dimension
   dims.erase( olddim );
   strides.erase( olddim );
   // Sample size is doubled, meaning all strides must be halved
   for( dip::uint ii = 0; ii < nd; ++ii ) {
      strides[ii] /= 2;
   }
   tstride /= 2;
   pixelsize.EraseDimension( olddim );
   return *this;
}

Image& Image::SplitComplexToTensor() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !tensor.IsScalar(), E::NOT_SCALAR );
   dip_ThrowIf( !datatype.IsComplex() , E::DATA_TYPE_NOT_SUPPORTED);
   // Change data type
   datatype = datatype == DT_SCOMPLEX ? DT_SFLOAT : DT_DFLOAT;
   // Sample size is halved, meaning all strides must be doubled
   for( dip::uint ii = 0; ii < dims.size(); ++ii ) {
      strides[ii] *= 2;
   }
   // Create new tensor dimension
   tensor.SetVector( 2 );
   tstride = 1;
   ResetColorSpace();
   return *this;
}

Image& Image::MergeTensorToComplex() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( ( tensor.Elements() != 2 ) || ( tstride != 1 ), E::TENSORSIZES_DONT_MATCH );
   dip_ThrowIf( datatype.IsComplex() , E::DATA_TYPE_NOT_SUPPORTED);
   // Change data type
   datatype = datatype == DT_SFLOAT ? DT_SCOMPLEX : DT_DCOMPLEX;
   // Delete old tensor dimension
   tensor.SetScalar();
   //tstride = 1; // was already the case
   // Sample size is doubled, meaning all strides must be halved
   for( dip::uint ii = 0; ii < dims.size(); ++ii ) {
      strides[ii] /= 2;
   }
   ResetColorSpace();
   return *this;
}

} // namespace dip
