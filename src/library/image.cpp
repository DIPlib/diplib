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


// Sort strides smallest to largest (simple bubble sort, assume few elements)
static void SortByStrides( IntegerArray& s, UnsignedArray& d ) {
   // I got compiler errors because `uint` seems to be defined in sys/types.h
   // Solution is to specify the namespace for our `uint`.
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
   if( size == (NumberOfPixels()-1) * s + 1 ) {
      stride = s;
      porigin = (uint8*)origin + start * datatype.SizeOf();
   }
   else {
      stride = 0;
      porigin = nullptr;
   }
}


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
   uint n = s.size();
   // Make sure all strides are positive
   for( uint ii=0; ii<n; ++ii ) {
      s[ii] = std::abs( s[ii] );
   }
   SortByStrides( s, d );
   // Test invariant
   for( uint ii=0; ii<n-1; ++ii ) {
      if( s[ii+1] <= s[ii]*(d[ii]-1) )
         return false;
   }
   // It's OK
   return true;
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
   uint origin1 = (uint)origin;        // Pointer to integer conversion
   uint origin2 = (uint)other.origin;  // Pointer to integer conversion
   return origin1 == origin2;

   // Non-overlapping portions of the data block
   uint size1,  size2;
   sint start1, start2;
   GetDataBlockSizeAndStart( size1, start1 );
   other.GetDataBlockSizeAndStart( size2, start2 );
   if( ( start1+size1 <= start2 ) || ( start2+size2 <= start1 ) )
      return false;

   // Lastly, check dimensions and strides
   // This is a bit complex

   // Add tensor dimension and strides to the lists
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

   // Sort strides smallest to largest
   SortByStrides( strides1, dims1 );
   SortByStrides( strides2, dims2 );

   // Walk through both stride arrays matching up dimensions
   // For each matching dimension, see if the views overlap

   // The assumed invariant is that stride[ii+1]>=stride[ii]*dims[ii]
   // Added singleton dimensions have a stride of 0

   uint i1 = 0;
   uint i2 = 0;
   while( (i1<ndims1) && (strides1[i1]==0) ) ++i1;
   while( (i2<ndims2) && (strides2[i2]==0) ) ++i2;
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
      if( ( origin1+s1*(d1-1) < origin2 ) ||           // disjoint left
          ( origin2+s2*(d2-1) < origin1 ) ||           // disjoint right
          ( (s1==s2) && ((origin1-origin2) % s1 != 0) ) ) {   // interleaved
            // There is no overlap in this dimension, and hence the views
            // do not overlap;
            return false;
      }
   }
   return true;
}


//
// Forge()
//

void Image::Forge() {
   if( !IsForged() ) {
      uint size = NumberOfPixels();
      ThrowIf( size==0, "Cannot forge an image without pixels (dimensions must be > 0)" );
      ThrowIf( ( size != 0 ) &&
               ( TensorElements() > std::numeric_limits<uint>::max() / size ),
               E::DIMENSIONALITY_EXCEEDS_LIMIT );
      size *= TensorElements();
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

//
// Operator<<()
//

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
   for( uint ii=0; ii<img.dims.size(); ++ii ) {
      os << img.dims[ii] << ", ";
   }
   os << std::endl;
   os << "   strides: ";
   for( uint ii=0; ii<img.strides.size(); ++ii ) {
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
