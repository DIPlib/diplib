/*
 * New DIPlib include file
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include <iostream>
#include <cstdlib>

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

//

bool Image::HasNormalStrides() const {
   DIPASSERT( IsForged(), dip::E::IMAGE_NOT_FORGED );
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


void Image::ComputeStrides() {
   DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
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
   DIPASSERT( HasValidStrides(), "Invalid strides" );
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

void Image::Forge() {
   if( !IsForged() ) {
      if( external_interface ) {
         datablock = external_interface->AllocateData( dims, strides, tensor, tstride, datatype );
         uint sz;
         sint start;
         GetDataBlockSizeAndStart( sz, start );
         origin = (void*)( (uint8*)datablock.get() + start*sz );
      } else {
         uint size = GetNumberOfPixels() * GetTensorElements();
                           // todo: check for overflow in computation above!
         std::cout << size << std::endl;
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
         void* p = ::malloc( size*sz );
         datablock = std::shared_ptr<void>( p, ::free );
         origin = (void*)( (uint8*)p + start*sz );
      }
   }
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
