/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>

using namespace dip;

void dip::DefineROI(
   Image& dest,
   const Image& src,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing
){
   // ...
}

// Constructor: Creates a new image pointing to data of 'src'.
Image::Image(
   const Image& src,
   const UnsignedArray& origin,
   const UnsignedArray& dims,
   const IntegerArray& spacing
){
   DefineROI( *this, src, origin, dims, spacing );
}

// Normal strides are the default ones:
// increasing in value, and with contiguous data.
bool Image::HasNormalStrides() const {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( tstride != 1 ) {
      return false;
   }
   sint total = tensor.Elements();
   for( uint ii=0; ii<dims.size(); ++ii ) {
      if( strides[ii] != total ) {
         return false;
      }
      total *= dims[ii];
   }
   return true;
}


// Return a pointer to the start of the data and a single stride to
// walk through all pixels. If this is not possible, stride==0 and
// porigin==nullptr.
void Image::GetSimpleStrideAndOrigin( uint& stride, void*& porigin ) const {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( strides.size() == 0 ) {
      stride = 1;
      porigin = origin;
   }
   sint s = std::abs( strides[0] );
   for( uint ii=1; ii<strides.size(); ++ii ) {
      s = std::min( s, std::abs( strides[ii] ) );
   }
   sint start;
   uint size;
   GetDataBlockSizeAndStart( size, start );
   if( size == (GetNumberOfPixels()-1) * s + 1 ) {
      stride = s;
      porigin = (uint8*)origin + start * datatype.SizeOf();
   }
   else {
      stride = 0;
      porigin = nullptr;
   }
}


void Image::ComputeStrides() {
   ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
   tstride = 1;                       // We set tensor strides to 1 by default.
   uint s = tensor.Elements();
   uint n = dims.size();
   strides.resize( n );
   for( uint ii=0; ii<n; ++ii ) {
      strides[ii] = s;
      s *= dims[ii];
   }
}


void Image::GetDataBlockSizeAndStart( uint& size, sint& start ) const {
   ThrowIf( !HasValidStrides(), "Invalid strides" );
   sint min = 0, max = 0;
      sint p = ( tensor.Elements() - 1 ) * tstride;
      if( p < 0 ) {
         min += p;
      } else {
         max += p;
      }
   for( uint ii=0; ii<dims.size(); ++ii ) {
      sint p = ( dims[ii] - 1 ) * strides[ii];
      if( p < 0 ) {
         min += p;
      } else {
         max += p;
      }
   }
   start = min;
   size = max - min + 1;
}


// Does writing in this image change the data of the other image?
bool Image::Aliases( const Image& other ) const {
   ThrowIf( !IsForged(),       E::IMAGE_NOT_FORGED );
   ThrowIf( !other.IsForged(), E::IMAGE_NOT_FORGED );
   // Different data blocks do not overlap by definition
   if( datablock != other.datablock )
      return false;
   // Quicky: if the origin is the same, they share at least one pixel
   uint origin1 = (uint)origin;        // Pointer to integer conversion.
   uint origin2 = (uint)other.origin;  // Pointer to integer conversion.
   return origin1 == origin2;
   // Non-overlapping portions of the data block
   uint size1,  size2;
   sint start1, start2;
   GetDataBlockSizeAndStart( size1, start1 );
   other.GetDataBlockSizeAndStart( size2, start2 );
   if( ( start1+size1 < start2 ) || ( start2+size2 < start1 ) )
      return false;
   // Lastly, check dimensions and strides
   // *** NOTE: ***
   // We use here the fact that these images can differ in predetermined
   // ways. We do not allow two dimensions to be merged into one, nor
   // do we add dimensions by splitting a dimension. We only add singleton
   // dimensions or subtract dimensions. Thus, if one data block has a
   // dimension with a stride value that the other doesn't have, we can
   // assume that the other has deleted that dimension. Likewise, if
   // one dimension has a stride of 0, it is an added singleton dimension.
   // If a library user decides to fudge with the image object and the
   // data blocks in a way that undermines this assumption, the library
   // might think that data doesn't alias when it does, leading to filters
   // overwriting their own input data and producing undefined output.
   // *** CAREFUL! ***
   IntegerArray  strides1 = strides;
   UnsignedArray dims1    = dims;
   if( tensor.Elements() > 1 ) {
      strides1.push_back( tstride );
      dims1.push_back( tensor.Elements() );
   }
   uint ndims1 = strides1.size();
   IntegerArray  strides2 = other.strides;
   UnsignedArray dims2    = other.dims;
   if( other.tensor.Elements() > 1 ) {
      strides2.push_back( other.tstride );
      dims2.push_back( other.tensor.Elements() );
   }
   uint ndims2 = strides2.size();
   // Make sure all strides are positive
   for( uint ii=0; ii<ndims1; ++ii ) {
      if( strides1[ii] < 0 ) {
         strides1[ii] = -strides1[ii];
         origin1 -= strides1[ii]*dims1[ii];
      }
   }
   for( uint ii=0; ii<ndims2; ++ii ) {
      if( strides2[ii] < 0 ) {
         strides2[ii] = -strides2[ii];
         origin2 -= strides2[ii]*dims2[ii];
      }
   }
   // Sort strides smallest to largest (simple bubble sort, assume few elements)
   for( uint jj=ndims1-1; jj!=0; --jj ) {
      for( uint ii=0; ii!=jj; ++ii ) {
         if( strides1[ii] > strides1[ii+1] ) {
            std::swap( strides1[ii], strides1[ii+1] );
            std::swap( dims1   [ii], dims1   [ii+1] );
         }
      }
   }
   for( uint jj=ndims2-1; jj!=0; --jj ) {
      for( uint ii=0; ii!=jj; ++ii ) {
         if( strides2[ii] > strides2[ii+1] ) {
            std::swap( strides2[ii], strides2[ii+1] );
            std::swap( dims2   [ii], dims2   [ii+1] );
         }
      }
   }
   // Walk through both stride arrays matching up dimensions
   // For each matching dimension, see if the views overlap
   //
   // TODO: matching dimensions can have different strides too if
   // it is subsampled. One invariant is that stride[ii+1]>=stride[ii]*dims[ii].
   //
   uint i1 = 0;
   uint i2 = 0;
   while( strides1[i1]==0 ) ++i1;
   while( strides2[i2]==0 ) ++i2;
   while( i1<ndims1 || i2<ndims2 )
   {
      uint s1 = 0, s2 = 0, d1 = 1, d2 = 1;
      if( i1<ndims1 ) {
         s1 = strides1[i1];
         d1 = dims1[i1];
      }
      if( i2<ndims2 ) {
         s2 = strides2[i2];
         d2 = dims2[i2];
      }
      if( s1 < s2 ) {
         if( s1 != 0 ) {
            // assume img2 has dims==1 in this dimension
            //s2 = s1;
            d2 = 1;
            ++i1;
         } else {
            // we're at the end of dims1
            s1 = s2;
            ++i2;
         }
      } else if ( s1 > s2 ) {
         if( s2 != 0 ) {
            // assume img1 has dims==1 in this dimension
            s1 = s2;
            d1 = 1;
            ++i2;
         } else {
            // we're at the end of dims1
            //s2 = s1;
            ++i1;
         }
      } else {
         // matching strides
         ++i1;
         ++i2;
      }
      if( ( origin1+d1 < origin2 ) ||           // disjoint left
          ( origin2+d2 < origin1 ) ||           // disjoint right
          ( (origin1-origin2) % s1 != 0 ) ) {   // interleaved
            // There is no overlap in this dimension, and hence the views
            // do not overlap;
            return false;
      }
   }
   return true;
}


void Image::Forge() {
   if( !IsForged() ) {
      uint size = GetNumberOfPixels();
      ThrowIf( size==0, "Cannot forge an image without pixels (dimensions must be > 0)" );
      ThrowIf( ( size != 0 ) &&
               ( GetTensorElements() > std::numeric_limits<uint>::max() / size ),
               E::DIMENSIONALITY_EXCEEDS_LIMIT );
      size *= GetTensorElements();
      if( external_interface ) {
         datablock = external_interface->AllocateData( dims, strides, tensor, tstride, datatype );
         uint sz;
         sint start;
         GetDataBlockSizeAndStart( sz, start );
         origin = (uint8*)datablock.get() + start * datatype.SizeOf();
      } else {
         // std::cout << size << std::endl;
         sint start = 0;
         if( HasValidStrides() ) {
            uint sz;
            GetDataBlockSizeAndStart( sz, start );
            if( sz != size ) {
               ComputeStrides();
            }
         } else {
            ComputeStrides();
         }
         uint sz = datatype.SizeOf();
         void* p = ::malloc( size * sz );
         datablock = std::shared_ptr<void>( p, ::free );
         origin = (uint8*)p + start * sz;
      }
   }
}


// Permute dimensions
// Example: {3,1} -> 3rd dimension becomes 1st, 1st dimension becomes 2nd,
//                   2nd dimension is removed (only possible if dims[1]==1).
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


// Swap dimensions d1 and d2
Image& Image::SwapDimensions( uint d1, uint d2 ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   uint nd = dims.size();
   ThrowIf( (d1>=nd) || (d2>=nd), E::ILLEGAL_DIMENSION );
   std::swap( dims   [d1], dims   [d2] );
   std::swap( strides[d1], strides[d2] );
   return *this;
}


// Make image 1D, if !HasSimpleStride(), data block will be copied
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
   dims = { GetNumberOfPixels() };
   origin = p;
   return *this;
}


// Removes singleton dimensions (dimensions with size==1)
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


// Adds a singleton dimension (with size==1), dimensions dim to
// last are shifted up.
// Example: an image with dims {4,5,6}, we add singleton dimension
// dim=1, leaves the image with dims {4,1,5,6}.
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


// Appends singleton dimensions to increase the image dimensionality
// to n. If the image already has n or more dimensions, nothing happens.
Image& Image::ExpandDimensionality( uint n ) {
   ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( dims.size() < n ) {
      dims   .resize( n, 1 );
      strides.resize( n, 0 ); // follow same convention as in AddSingleton().
   }
   return *this;
}


// Mirror de image about selected axes
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


std::ostream& dip::operator<<(
   std::ostream& os,
   const Image& img
){
   if( img.tensor.Elements() == 1 ) {
      os << "Scalar image, ";
   } else {
      os << img.tensor.Rows() << "x" << img.tensor.Columns() << "-tensor image, ";
   }
   os << img.dims.size() << "-D, " << img.datatype.Name() << std::endl;
   os << "   sizes: ";
   for( int ii=0; ii<img.dims.size(); ++ii ) {
      os << img.dims[ii] << ", ";
   }
   os << std::endl;
   os << "   strides: ";
   for( int ii=0; ii<img.strides.size(); ++ii ) {
      os << img.strides[ii] << ", ";
   }
   os << std::endl;
   os << "   tensor stride: " << img.tstride << std::endl;
   if( img.origin ) {
      os << "   origin pointer: " << (uint)img.origin << std::endl;
      if( img.HasContiguousData() ) {
         if( img.HasNormalStrides() ) {
            os << "   strides are normal" << std::endl;
         } else {
            os << "   strides are contiguous but not normal" << std::endl;
         }
      }
   } else {
      os << "   not forged" << std::endl;
   }
   return os;
}
