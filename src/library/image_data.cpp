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
#include <limits>
#include "dip_numeric.h"

namespace dip {


// --- Internal functions, static ---


// Sort strides smallest to largest (simple bubble sort, assume few elements)
// Dimensions array is sorted the same way.
static void SortByStrides(
      IntegerArray& s,
      UnsignedArray& d
) {
   dip::uint n = s.size();
   for( dip::uint jj=n-1; jj!=0; --jj ) {
      for( dip::uint ii=0; ii!=jj; ++ii ) {
         if( s[ii] > s[ii+1] ) {
            std::swap( s[ii], s[ii+1] );
            std::swap( d[ii], d[ii+1] );
         }
      }
   }
}


// Return the number of pixels defined by the dims array.
// Same as dip::Image::NumberOfPixels() but with check.
static dip::uint FindNumberOfPixels(
      const UnsignedArray& dims
) {
   dip::uint n = 1;
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
      dip_ThrowIf( ( dims[ii] != 0 ) && ( n > std::numeric_limits<dip::uint>::max() / dims[ii] ),
         E::DIMENSIONALITY_EXCEEDS_LIMIT );
      n *= dims[ii];
   }
   return n;
}


// Return the size of the data block needed to store an image given by
// strides and dims, as well as the (negative) offset of the block if any
// of the strides are negative.
static void FindDataBlockSizeAndStart(
      const IntegerArray& strides,
      const UnsignedArray& dims,
      dip::uint& size,
      dip::sint& start
) {
   dip::sint min = 0, max = 0;
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
      dip::sint p = ( dims[ii] - 1 ) * strides[ii];
      if( p < 0 ) {
         min += p;
      } else {
         max += p;
      }
   }
   start = min;
   size = max - min + 1;
}


// Return the simple stride (if it exists) and the start pixel (offset into
// the data block) of pixels defined by strides and dims.
static void FindSimpleStrideSizeAndStart(
      const IntegerArray& strides,
      const UnsignedArray& dims,
      dip::uint& sstride,
      dip::uint& size,
      dip::sint& start
) {
  if( strides.size() == 0 ) {
      sstride = 1;
      start = 0;
      return;
   }
   sstride = std::numeric_limits<dip::sint>::max();
   for( dip::uint ii=0; ii<strides.size(); ++ii ) {
      if( dims[ii]>1 ) {
         sstride = std::min( sstride, static_cast<dip::uint>( std::abs( strides[ii] ) ) );
      }
   }
   FindDataBlockSizeAndStart( strides, dims, size, start );
   if( size != ( FindNumberOfPixels( dims ) - 1 ) * sstride + 1 ) {
      sstride = 0;
   }
}


// Compute coordinates of a pixel from an offset.
// Strides array must be all positive, and sorted in increasing order.
static UnsignedArray OffsetToCoordinates(
      dip::uint offset,
      const IntegerArray& strides
) {
   UnsignedArray coord( strides.size() );
   for( dip::sint ii = (dip::sint)(strides.size()-1); ii >= 0 ; --ii ) {
      coord[ii] = offset / strides[ii];
      offset    = offset % strides[ii];
   }
   return coord;
}


// --- Library functions ---


// Normal strides are the default ones:
// increasing in value, and with contiguous data.
bool Image::HasNormalStrides() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   if( tstride != 1 ) {
      return false;
   }
   dip::sint total = tensor.Elements();
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
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
void Image::GetSimpleStrideAndOrigin( dip::uint& sstride, void*& porigin ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint start;
   dip::uint size;
   FindSimpleStrideSizeAndStart( strides, dims, sstride, size, start );
   if( sstride == 0 ) {
      porigin = nullptr;
   } else {
      porigin = (uint8*)origin + start * datatype.SizeOf();
   }
}


//
bool Image::HasValidStrides() const {
   // We require that |strides[ii+1]| > |strides[ii]|*(dims[ii]-1) (after sorting the absolute strides on size)
   if( dims.size() != strides.size() ) {
      return false;
   }
   // Add tensor dimension and strides to the lists
   IntegerArray  s = strides;
   UnsignedArray d = dims;
   if( tensor.Elements() > 1 ) {
      s.push_back( tstride );
      d.push_back( tensor.Elements() );
   }
   dip::uint n = s.size();
   // Make sure all strides are positive
   for( dip::uint ii=0; ii<n; ++ii ) {
      s[ii] = std::abs( s[ii] );
   }
   SortByStrides( s, d );
   // Test invariant
   for( dip::uint ii=0; ii<n-1; ++ii ) {
      if( s[ii+1] <= s[ii]*(d[ii]-1) )
         return false;
   }
   // It's OK
   return true;
}


//
void Image::ComputeStrides() {
   dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
   tstride = 1;                       // We set tensor strides to 1 by default.
   dip::uint s = tensor.Elements();
   dip::uint n = dims.size();
   strides.resize( n );
   for( dip::uint ii=0; ii<n; ++ii ) {
      strides[ii] = s;
      s *= dims[ii];
   }
}


//
void Image::GetDataBlockSizeAndStart( dip::uint& size, dip::sint& start ) const {
   // Add tensor dimension and strides to the lists
   IntegerArray  s = strides;
   UnsignedArray d = dims;
   if( tensor.Elements() > 1 ) {
      s.push_back( tstride );
      d.push_back( tensor.Elements() );
   }
   FindDataBlockSizeAndStart( s, d, size, start );
}


// Does writing in this image change the data of the other image?
bool Image::Aliases( const Image& other ) const {
   dip_ThrowIf( !IsForged(),       E::IMAGE_NOT_FORGED );
   dip_ThrowIf( !other.IsForged(), E::IMAGE_NOT_FORGED );

   // Different data blocks do not overlap by definition
   if( datablock != other.datablock )
      return false;

   // Quicky: if the origin is the same, they share at least one pixel
   dip::uint origin1 = (uint8*)      origin - (uint8*)      datablock.get();
   dip::uint origin2 = (uint8*)other.origin - (uint8*)other.datablock.get();
   if( origin1 == origin2 )
      return true;

   // Same data block: expect same data type also!
   dip::uint dts = datatype.SizeOf();       // TODO: what do we do if this is not the case???

   // Make origin in units of data size
   origin1 /= dts;
   origin2 /= dts;

   // Add tensor dimension and strides to the lists
   IntegerArray  strides1 = strides;
   UnsignedArray dims1    = dims;
   if( tensor.Elements() > 1 ) {
      strides1.push_back( tstride );
      dims1.push_back( tensor.Elements() );
   }
   IntegerArray  strides2 = other.strides;
   UnsignedArray dims2    = other.dims;
   if( other.tensor.Elements() > 1 ) {
      strides2.push_back( other.tstride );
      dims2.push_back( other.tensor.Elements() );
   }

   // Quicky: if both have simple strides larger than one, and their offsets
   // do not differ by a multiple of that stride, they don't overlap.
   dip::uint sstride1, sstride2;
   dip::uint size1,    size2;
   dip::sint start1,   start2;
   FindSimpleStrideSizeAndStart( strides1, dims1, sstride1, size1, start1 );
   FindSimpleStrideSizeAndStart( strides2, dims2, sstride2, size2, start2 );
   start1 += origin1;
   start2 += origin2;
   if( ( sstride1 > 1 ) && ( sstride1 == sstride2 ) ) {
      if( ( start1 - start2 ) % sstride1 != 0 )
         return false;
   }

   // Non-overlapping portions of the data block
   if( ( start1+size1 <= start2 ) || ( start2+size2 <= start1 ) )
      return false;

   // Lastly, check dimensions and strides
   // This is a bit complex

   // Make sure all strides are positive (un-mirror)
   dip::uint ndims1 = strides1.size();
   dip::uint ndims2 = strides2.size();
   for( dip::uint ii=0; ii<ndims1; ++ii ) {
      if( strides1[ii] < 0 ) {
         strides1[ii] = -strides1[ii];
         origin1 -= (dims1[ii]-1) * strides1[ii];
      }
   }
   for( dip::uint ii=0; ii<ndims2; ++ii ) {
      if( strides2[ii] < 0 ) {
         strides2[ii] = -strides2[ii];
         origin2 -= (dims2[ii]-1) * strides2[ii];
      }
   }

   // Sort strides smallest to largest
   SortByStrides( strides1, dims1 );
   SortByStrides( strides2, dims2 );

   // Walk through both stride arrays matching up dimensions
   // The assumed invariant is that stride[ii+1]>=stride[ii]*dims[ii]
   // Added singleton dimensions have a stride of 0

   IntegerArray  comstrides;  // common strides
   IntegerArray  newstrides1; // new strides img 1
   IntegerArray  newstrides2; // new strides img 2
   UnsignedArray newdims1;    // new dimensions img 1
   UnsignedArray newdims2;    // new dimensions img 2

   dip::uint i1 = 0;
   dip::uint i2 = 0;
   while( (i1<ndims1) && (strides1[i1]==0) ) ++i1;
   while( (i2<ndims2) && (strides2[i2]==0) ) ++i2;
   while( i1<ndims1 || i2<ndims2 )
   {
      dip::uint s1 = 0, s2 = 0, d1 = 1, d2 = 1;
      if( i1<ndims1 ) {
         s1 = strides1[i1];
         d1 = dims1[i1];
      }
      if( i2<ndims2 ) {
         s2 = strides2[i2];
         d2 = dims2[i2];
      }
      if( s1 == 0 ) {
         // we're at the end of dims1
         s1 = s2;
         ++i2;
      } else if( s2 == 0 ) {           // s1 and s2 cannot be 0 at the same time
         // we're at the end of dims2
         s2 = s1;
         ++i1;
      } else if( (i1+1 < ndims1) && (strides1[i1+1] <= s2*(d2-1)) ) {
         // s2 is too large, assume img2 has dims==1 in this dimension
         s2 = s1;
         d2 = 1;
         ++i1;
      } else if( (i2+1 < ndims2) && (strides2[i2+1] <= s1*(d1-1)) ) {
         // s1 is too large, assume img1 has dims==1 in this dimension
         s1 = s2;
         d1 = 1;
         ++i2;
      } else {
         // matching dimensions
         ++i1;
         ++i2;
      }
      dip::sint cs = comstrides.empty() ? 1 : gcd( s1, s2 ); // The first dimension should have stride==1
      comstrides.push_back( cs );
      newstrides1.push_back( s1/cs );
      newstrides2.push_back( s2/cs );
      newdims1.push_back( d1 );
      newdims2.push_back( d2 );
   }

   // Compute coordinates of origin for both images
   UnsignedArray neworigin1 = OffsetToCoordinates( origin1, comstrides );
   UnsignedArray neworigin2 = OffsetToCoordinates( origin2, comstrides );

   // Compute, for each of the dimensions, if the views overlap. If
   // they don't overlap for any one dimension, there is no aliasing.
   for( dip::uint ii = 0; ii < comstrides.size(); ++ii ) {
      if( neworigin1[ii] + (newdims1[ii]-1)*newstrides1[ii] < neworigin2[ii] )
         return false;
      if( neworigin2[ii] + (newdims2[ii]-1)*newstrides2[ii] < neworigin1[ii] )
         return false;
      if( (newstrides1[ii] == newstrides2[ii]) &&
          (newstrides1[ii] > 1) &&
          (((dip::sint)neworigin1[ii]-(dip::sint)neworigin2[ii]) % newstrides1[ii] != 0) )
         return false;
   }

   return true;
}


//
void Image::Forge() {
   if( !IsForged() ) {
      dip::uint size = FindNumberOfPixels( dims );
      dip_ThrowIf( size==0, "Cannot forge an image without pixels (dimensions must be > 0)" );
      dip_ThrowIf( ( size != 0 ) &&
               ( TensorElements() > std::numeric_limits<dip::uint>::max() / size ),
               E::DIMENSIONALITY_EXCEEDS_LIMIT );
      size *= TensorElements();
      if( externalInterface ) {
         datablock = externalInterface->AllocateData( dims, strides, tensor, tstride, datatype );
         dip::uint sz;
         dip::sint start;
         GetDataBlockSizeAndStart( sz, start );
         origin = (uint8*)datablock.get() + start * datatype.SizeOf();
      } else {
         // std::cout << size << std::endl;
         dip::sint start = 0;
         if( HasValidStrides() ) {
            dip::uint sz;
            GetDataBlockSizeAndStart( sz, start );
            if( sz != size ) {
               ComputeStrides();
            }
         } else {
            ComputeStrides();
         }
         dip::uint sz = datatype.SizeOf();
         void* p = ::malloc( size * sz );
         datablock = std::shared_ptr<void>( p, ::free );
         origin = (uint8*)p + start * sz;
      }
   }
}

//
void Image::Set( dip::sint v ) { // TODO, calling Framework::Scan()
}

void Image::Set( dfloat v) { // TODO, calling Framework::Scan()
}

void Image::Set( dcomplex v) { // TODO, calling Framework::Scan()
}

} // namespace dip
