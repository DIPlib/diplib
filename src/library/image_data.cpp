/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <cstdlib>   // std::malloc, std::realloc, std::free
#include <iostream>
#include <algorithm>
#include <limits>

#include "diplib.h"
#include "dip_numeric.h"
#include "dip_ndloop.h"
#include "dip_framework.h"
#include "dip_overload.h"
#include "dip_clamp_cast.h"

#include "copybuffer.h"

namespace dip {


// --- Internal functions, static ---


// Compute a normal stride array.
static void ComputeStrides(
      const UnsignedArray& dims,
      dip::uint s,               // set to tensor.Elements()
      IntegerArray& strides
) {
   dip::uint n = dims.size();
   strides.resize( n );
   for( dip::uint ii=0; ii<n; ++ii ) {
      strides[ii] = s;
      s *= dims[ii];
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


// Return the simple stride (if it exists), the start pixel (offset into the
// data block), and number of pixels defined by strides and dims.
// If there is no simple stride, sets sstride==0, and returns false.
// Note that sstride==0 does not indicate an error condition, it is possible
// that the image was singleton-expanded from a 0D image.
static bool FindSimpleStrideSizeAndStart(
      const IntegerArray& strides,
      const UnsignedArray& dims,
      dip::uint& sstride,
      dip::uint& size,
      dip::sint& start
) {
   if( strides.size() == 0 ) {
      // Special case
      sstride = 1;
      size = 1;
      start = 0;
   } else {
      // Find the simple stride
      sstride = std::numeric_limits<dip::sint>::max();
      for( dip::uint ii=0; ii<strides.size(); ++ii ) {
         if( dims[ii]>1 ) {
            sstride = std::min( sstride, static_cast<dip::uint>( std::abs( strides[ii] ) ) );
         }
      }
      FindDataBlockSizeAndStart( strides, dims, size, start );
      if( size != ( FindNumberOfPixels( dims ) - 1 ) * sstride + 1 ) {
         sstride = 0;
         return false;
      }
   }
   return true;
}


// Compute coordinates of a pixel from an offset.
// Strides array must be all positive, and sorted in increasing order.
static UnsignedArray OffsetToCoordinates(
      dip::uint offset,
      const IntegerArray& strides
) {
   UnsignedArray coord( strides.size() );
   for( dip::uint ii = strides.size(); ii > 0; ) {
      // This loop increases its counter at the start, a disadvantage of using
      // unsigned integers as loop counters.
      --ii;
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
// walk through all pixels. If this is not possible, porigin==nullptr.
void Image::GetSimpleStrideAndOrigin( dip::uint& sstride, void*& porigin ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint start;
   dip::uint size;
   if( FindSimpleStrideSizeAndStart( strides, dims, sstride, size, start )) {
      porigin = Pointer( start );
   } else {
      porigin = nullptr;
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
   s.sort( d );
   // Test invariant
   for( dip::uint ii=0; ii<n-1; ++ii ) {
      if( s[ii+1] <= s[ii]*(d[ii]-1) )
         return false;
   }
   // It's OK
   return true;
}


//
void Image::SetNormalStrides() {
   dip_ThrowIf( IsForged(), E::IMAGE_NOT_RAW );
   tstride = 1;                       // We set tensor strides to 1 by default.
   ComputeStrides( dims, tensor.Elements(), strides );
}


//
void Image::GetDataBlockSizeAndStart( dip::uint& size, dip::sint& start ) const {
   FindDataBlockSizeAndStart( strides, dims, size, start );
}
void Image::GetDataBlockSizeAndStartWithTensor( dip::uint& size, dip::sint& start ) const {
   if( tensor.Elements() > 1 ) {
      UnsignedArray d = dims;
      d.push_back( tensor.Elements() );
      IntegerArray s = strides;
      s.push_back( tstride );
      FindDataBlockSizeAndStart( s, d, size, start );
   } else {
      FindDataBlockSizeAndStart( strides, dims, size, start );
   }
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

   // Copy size and stride arrays, add tensor dimension
   IntegerArray  strides1 = strides;
   UnsignedArray dims1    = dims;
   if( tensor.Elements() > 1 ) {
      strides1.push_back( tstride );
      dims1.push_back( tensor.Elements() );
   }
   dip::uint ndims1 = strides1.size();
   IntegerArray  strides2 = other.strides;
   UnsignedArray dims2    = other.dims;
   if( other.tensor.Elements() > 1 ) {
      strides2.push_back( other.tstride );
      dims2.push_back( other.tensor.Elements() );
   }
   dip::uint ndims2 = strides2.size();

   // Check sample sizes
   dip::uint dts1 = datatype.SizeOf();
   dip::uint dts2 = other.datatype.SizeOf();
   dip::uint dts = dts1;
   if( dts1 > dts2 ) {
      // Split the samples of 1, adding a new dimension
      dts = dts2;
      dip::uint n = dts1 / dts; // this is always an integer value, samples have size 1, 2, 4, 8 or 16.
      for( dip::uint ii=0; ii<ndims1; ++ii ) {
         strides1[ii] *= n;
      }
      strides1.push_back( 1 );
      dims1.push_back( n );
      ++ndims1;
   } else if( dts1 < dts2 ) {
      // Split the samples of 2, adding a new dimension
      dip::uint n = dts2 / dts; // this is always an integer value, samples have size 1, 2, 4, 8 or 16.
      for( dip::uint ii=0; ii<ndims2; ++ii ) {
         strides2[ii] *= n;
      }
      strides2.push_back( 1 );
      dims2.push_back( n );
      ++ndims2;
   } // else, the samples have the same size

   // Make origin in units of data size
   origin1 /= dts;
   origin2 /= dts;

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

   // Sort strides smallest to largest, keeping dims in sync.
   strides1.sort( dims1 );
   strides2.sort( dims2 );

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
   UnsignedArray neworigin1 = dip::OffsetToCoordinates( origin1, comstrides );
   UnsignedArray neworigin2 = dip::OffsetToCoordinates( origin2, comstrides );

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
      dip_ThrowIf( TensorElements() > std::numeric_limits<dip::uint>::max() / size,
               E::DIMENSIONALITY_EXCEEDS_LIMIT );
      size *= TensorElements();
      if( externalInterface ) {
         datablock = externalInterface->AllocateData( dims, strides, tensor, tstride, datatype );
         // AllocateData() can fail by returning a nullptr. If so, we allocate data in the normal way because we remain raw.
         if( datablock ) {
            dip::uint sz;
            dip::sint start;
            GetDataBlockSizeAndStartWithTensor( sz, start );
            origin = (uint8*)datablock.get() + start * datatype.SizeOf();
            //std::cout << "   Successfully forged image with external interface\n";
         }
      }
      if( !IsForged() ) {
         dip::sint start = 0;
         if( HasValidStrides() ) {
            dip::uint sz;
            GetDataBlockSizeAndStartWithTensor( sz, start );
            if( sz != size ) {
               SetNormalStrides();
            }
         } else {
            SetNormalStrides();
         }
         dip::uint sz = datatype.SizeOf();
         void* p = std::malloc( size * sz );
         dip_ThrowIf( !p, "Failed to allocate memory" );
         datablock = std::shared_ptr<void>( p, std::free );
         origin = (uint8*)p + start * sz;
         //std::cout << "   Successfully forged image\n";
      }
   }
}

//
dip::sint Image::Offset( const UnsignedArray& coords ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( coords.size() != dims.size(), E::ARRAY_ILLEGAL_SIZE );
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < dims.size(); ++ii ) {
      dip_ThrowIf( coords[ii] >= dims[ii], E::INDEX_OUT_OF_RANGE );
      offset += coords[ii] * strides[ii];
   }
   return offset;
}

//
UnsignedArray Image::OffsetToCoordinates( dip::uint offset ) const {
   // TODO: we need to sort the strides, but then the coordinates array will be in wrong order
   // TODO: this does not work if there are negative strides!?
   return dip::OffsetToCoordinates( offset, strides );
}

//
dip::uint Image::Index( const UnsignedArray& coords ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip_ThrowIf( coords.size() != dims.size(), E::ARRAY_ILLEGAL_SIZE );
   dip::uint index = 0;
   for( dip::uint ii = dims.size(); ii > 0; ) {
      --ii;
      dip_ThrowIf( coords[ii] >= dims[ii], E::INDEX_OUT_OF_RANGE );
      index *= dims[ii];
      index += coords[ii];
   }
   return index;
}

//
UnsignedArray Image::IndexToCoordinates( dip::uint index ) const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray coords( dims.size() );
   IntegerArray fake_strides;
   ComputeStrides( dims, 1, fake_strides );
   return dip::OffsetToCoordinates( index, fake_strides );
}

//
void Image::Copy( const Image& src ) {
   if( IsForged() ) {
      // Forged image, check to make sure number of samples is correct
      CompareProperties( src, Option::CmpProps_Dimensions + Option::CmpProps_TensorElements );
      // Change the tensor shape to match that of `src`
      tensor.ChangeShape( src.tensor );
      // The data type is not changed, the copy will convert the data type
   } else {
      // Non-forged image, make properties identical to `src`
      CopyProperties( src );
      Forge();
   }
   dip::uint sstride_d;
   void* porigin_d;
   GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( sstride_d != 0 ) {
      dip::uint sstride_s;
      void* porigin_s;
      src.GetSimpleStrideAndOrigin( sstride_s, porigin_s );
      if( sstride_s != 0 ) {
         // No need to loop
         CopyBuffer(
            porigin_s,
            src.datatype,
            sstride_s,
            src.tstride,
            porigin_d,
            datatype,
            sstride_d,
            tstride,
            NumberOfPixels(),
            tensor.Elements(),
            std::vector< dip::sint > {}
         );
         return;
      }
   }
   // Make nD loop
   dip::uint processingDim = Framework::OptimalProcessingDim( src );
   dip::sint offset_s;
   dip::sint offset_d;
   UnsignedArray coords = NDLoop::Init( src, *this, offset_s, offset_d );
   do {
      CopyBuffer(
         src.Pointer( offset_s ),
         src.datatype,
         src.strides[processingDim],
         src.tstride,
         Pointer( offset_d ),
         datatype,
         strides[processingDim],
         tstride,
         dims[processingDim],
         tensor.Elements(),
         std::vector< dip::sint > {}
      );
   } while( NDLoop::Next( coords, offset_s, offset_d, dims, src.strides, strides, processingDim ));
}

//
template<typename inT>
static inline void InternSet( Image& dest, inT v ) {
   dip_ThrowIf( !dest.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint sstride_d;
   void* porigin_d;
   dest.GetSimpleStrideAndOrigin( sstride_d, porigin_d );
   if( sstride_d != 0 ) {
      // No need to loop
      FillBuffer(
         porigin_d,
         dest.DataType(),
         sstride_d,
         dest.TensorStride(),
         dest.NumberOfPixels(),
         dest.TensorElements(),
         v
      );
   } else {
      // Make nD loop
      dip::uint processingDim = Framework::OptimalProcessingDim( dest );
      dip::sint offset_d;
      UnsignedArray coords = NDLoop::Init( dest, offset_d );
      do {
         FillBuffer(
            dest.Pointer( offset_d ),
            dest.DataType(),
            dest.Stride( processingDim ),
            dest.TensorStride(),
            dest.Dimension( processingDim ),
            dest.TensorElements(),
            v
         );
      } while( NDLoop::Next( coords, offset_d, dest.Dimensions(), dest.Strides(), processingDim ));
   }
}

void Image::Set( dip::sint v ) {
   InternSet( *this, v );
}

void Image::Set( dfloat v ) {
   InternSet( *this, v );
}

void Image::Set( dcomplex v ) {
   InternSet( *this, v );
}

// Casting the first sample (the first tensor component of the first pixel) to dcomplex.
template< typename TPI >
static inline dcomplex CastValueComplex( void* p ) {
   return clamp_cast< dcomplex >( *((TPI*)p) );
}
Image::operator dcomplex() const{
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dcomplex x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueComplex, ( origin ), datatype );
   return x;
}

// Casting the first sample (the first tensor component of the first pixel) to dfloat.
template< typename TPI >
static inline dfloat CastValueDouble( void* p ) {
   return clamp_cast< dfloat >( *((TPI*)p) );
}
Image::operator dfloat() const{
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dfloat x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueDouble, ( origin ), datatype );
   return x;
}

// Casting the first sample (the first tensor component of the first pixel) to sint.
template< typename TPI >
static inline dip::sint CastValueInteger( void* p ) {
   return clamp_cast< dip::sint >( *((TPI*)p) );
}
Image::operator dip::sint() const {
   dip_ThrowIf( !IsForged(), E::IMAGE_NOT_FORGED );
   dip::sint x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueInteger, ( origin ), datatype );
   return x;
}

} // namespace dip
