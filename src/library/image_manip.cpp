/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include <algorithm>

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
   for( dip::uint ii=0; ii<newnd; ++ii ) {
      newdims   [ii] = dims   [ order[ii] ];
      newstrides[ii] = strides[ order[ii] ];
   }
   dims    = newdims;
   strides = newstrides;
   return *this;
}


Image& Image::SwapDimensions( dip::uint d1, dip::uint d2 ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( (d1>=nd) || (d2>=nd), E::ILLEGAL_DIMENSION );
   std::swap( dims   [d1], dims   [d2] );
   std::swap( strides[d1], strides[d2] );
   return *this;
}


Image& Image::Flatten() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint stride;
   void* p;
   GetSimpleStrideAndOrigin( stride, p );
   if( stride==0 ) {
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
   return *this;
}


Image& Image::Squeeze() {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint jj = 0;
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
      if( dims[ii] > 1) {
         strides[jj] = strides[ii];
         dims[jj] = dims[ii];
         ++jj;
      }
   }
   strides.resize( jj );
   dims.resize( jj );
   return *this;
}


Image& Image::AddSingleton( dip::uint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( dim > nd, E::INVALID_PARAMETER );
   dims.resize( nd+1 );
   strides.resize( nd+1 );
   for( dip::uint ii=nd; ii>dim; --ii ) {
      dims   [ii] = dims   [ii-1];
      strides[ii] = strides[ii-1];
   }
   dims   [dim] = 1;
   strides[dim] = 0;
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
         origin = (uint8*)origin + ( dims[ii] - 1 ) * strides[ii] * datatype.SizeOf();
         strides[ii] = -strides[ii];
      }
   }
   return *this;
}


Image& Image::TensorToSpatial( dip::uint dim ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nd = dims.size();
   dip_ThrowIf( dim > nd, E::INVALID_PARAMETER );
   dims.resize( nd+1 );
   strides.resize( nd+1 );
   for( dip::uint ii=nd; ii>dim; --ii ) {
      dims   [ii] = dims   [ii-1];
      strides[ii] = strides[ii-1];
   }
   dims   [dim] = tensor.Elements();
   strides[dim] = tstride;
   return *this;
}


Image& Image::SpatialToTensor( dip::uint dim, dip::uint rows, dip::uint cols ) {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !tensor.IsScalar(), E::NOT_SCALAR );
   dip::uint nd = dims.size();
   dip_ThrowIf( dim >= nd, E::INVALID_PARAMETER );
   dip_ThrowIf( dims[dim] != rows * cols, E::PARAMETER_OUT_OF_RANGE );
   tensor.SetMatrix( rows, cols );
   tstride = strides[dim];
   --nd;
   for( dip::uint ii=dim; ii<nd; ++ii ) {
      dims   [ii] = dims   [ii+1];
      strides[ii] = strides[ii+1];
   }
   dims.resize( nd );
   strides.resize( nd );
   return *this;
}

} // namespace dip
