/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include <algorithm>

using namespace dip;

Image& Image::PermuteDimensions( const UnsignedArray& order ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint nd = dims.size();
   ThrowIf( order.size() > nd, E::ARRAY_PARAMETER_WRONG_LENGTH );
   BooleanArray keep( nd, false );
   uint newnd = order.size();
   for( uint ii=0; ii<newnd; ++ii ) {
      ThrowIf( order[ii] >= nd, E::ILLEGAL_DIMENSION );
      ThrowIf( keep[ order[ii] ], "Cannot duplicate a dimension" );
      keep[ order[ii] ] = true;
   }
   for( uint ii=0; ii<newnd; ++ii ) {
      ThrowIf( !keep[ii] && dims[ii]>1, "Cannot discard non-singleton dimension" );
   }
   UnsignedArray newdims( newnd, 0 );
   IntegerArray  newstrides( newnd, 0 );
   for( uint ii=0; ii<newnd; ++ii ) {
      newdims   [ii] = dims   [ order[ii] ];
      newstrides[ii] = strides[ order[ii] ];
   }
   dims    = newdims;
   strides = newstrides;
   return *this;
}


Image& Image::SwapDimensions( uint d1, uint d2 ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint nd = dims.size();
   ThrowIf( (d1>=nd) || (d2>=nd), E::ILLEGAL_DIMENSION );
   std::swap( dims   [d1], dims   [d2] );
   std::swap( strides[d1], strides[d2] );
   return *this;
}


Image& Image::Flatten() {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint stride;
   void* p;
   GetSimpleStrideAndOrigin( stride, p );
   if( stride==0 ) {
      // TODO: force a rewrite of the data with normal strides.
      // If there is no external interface:
      //    stride = tensor.Elements();
      //    p = origin;
      // Else:
      //    GetSimpleStrideAndOrigin( stride, p );
   }
   strides = { sint(stride) };
   dims = { NumberOfPixels() };
   origin = p;
   return *this;
}


Image& Image::Squeeze() {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint jj = 0;
   for( uint ii=0; ii<dims.size(); ++ii ) {
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


Image& Image::AddSingleton( uint dim ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint nd = dims.size();
   ThrowIf( dim > nd, E::INVALID_PARAMETER );
   dims.resize( nd+1 );
   strides.resize( nd+1 );
   for( uint ii=nd; ii>dim; --ii ) {
      dims   [ii] = dims   [ii-1];
      strides[ii] = strides[ii-1];
   }
   dims   [dim] = 1;
   strides[dim] = 0;
   // We set added singleton dimensions to 0 stride. The value is
   // irrelevant, but maybe we can use this as a flag for added
   // singletons to be used in the Image::Aliases() function.
   return *this;
}


Image& Image::ExpandDimensionality( uint n ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( dims.size() < n ) {
      dims   .resize( n, 1 );
      strides.resize( n, 0 ); // follow same convention as in AddSingleton().
   }
   return *this;
}


Image& Image::Mirror( BooleanArray& process ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint nd = dims.size();
   ThrowIf( process.size() != nd, E::ARRAY_ILLEGAL_SIZE);
   for( uint ii=0; ii<nd; ++ii ) {
      if( process[ii] ) {
         origin = (uint8*)origin + ( dims[ii] - 1 ) * strides[ii] * datatype.SizeOf();
         strides[ii] = -strides[ii];
      }
   }
   return *this;
}
