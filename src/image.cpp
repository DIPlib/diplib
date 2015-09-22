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

void dip::DefineROI(
   dip::Image & dest,
   const dip::Image & src,
   const dip::UnsignedArray & origin,
   const dip::UnsignedArray & dims,
   const dip::IntegerArray & spacing
){
   // ...
}

// Constructor: Creates a new image pointing to data of 'src'.
dip::Image::Image(
   const dip::Image & src,
   const dip::UnsignedArray & origin,
   const dip::UnsignedArray & dims,
   const dip::IntegerArray & spacing
){
   dip::DefineROI( *this, src, origin, dims, spacing );
}

//

bool dip::Image::HasNormalStrides() const {
   DIPASSERT( IsForged(), dip::E::IMAGE_NOT_FORGED );
   dip::sint total = 1;
   dip::uint n = dims.size();
   for( dip::uint ii=0; ii<n; ii++ ) {
      if( strides[ii] != total ) {
         return false;
      }
      total *= dims[ii];
   }
   n = tensor_dims.size();
   for( dip::uint ii=0; ii<n; ii++ ) {
      if( tensor_strides[ii] != total ) {
         return false;
      }
      total *= tensor_dims[ii];
   }
   return true;
}


void dip::Image::ComputeStrides() {
   DIPTS( IsForged(), dip::E::IMAGE_NOT_RAW );
   dip::uint s = 1;
   dip::uint n = dims.size();
   strides.resize( n );
   for( dip::uint ii=0; ii<n; ii++ ) {
      strides[ii] = s;
      s *= dims[ii];
   }
   n = tensor_dims.size();
   tensor_strides.resize( n );
   for( dip::uint ii=0; ii<n; ii++ ) {
      tensor_strides[ii] = s;
      s *= tensor_dims[ii];
   }
}


void dip::Image::GetDataBlockSizeAndStart( dip::uint & size, dip::sint & start ) const {
   DIPASSERT( HasValidStrides(), "Invalid strides" );
   dip::sint min = 0, max = 0;
   for( dip::uint ii=0; ii<dims.size(); ii++ ) {
      dip::sint p = ( dims[ii] - 1 ) * strides[ii];
      if( p < 0 ) {
         min += p;
      } else {
         max += p;
      }
   }
   for( dip::uint ii=0; ii<tensor_dims.size(); ii++ ) {
      dip::sint p = ( tensor_dims[ii] - 1 ) * tensor_strides[ii];
      if( p < 0 ) {
         min += p;
      } else {
         max += p;
      }
   }
   start = min;
   size = max - min + 1;
}


void dip::Image::Forge() {
   if( !IsForged() ) {
      if( external_interface ) {
         datablock = external_interface->AllocateData( dims, strides, tensor_dims, tensor_strides, datatype );
         dip::uint sz;
         dip::sint start;
         GetDataBlockSizeAndStart( sz, start );
         origin = (void*)( (dip::uint8*)datablock.get() + start*sz );
      } else {
         dip::uint size = GetNumberOfPixels() * GetNumberOfTensorComponents();
                           // todo: check for overflow in computation above!
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
         dip::uint sz = dip::_DataType::SizeOf( datatype );
         void * p = ::malloc( size*sz );
         datablock = std::shared_ptr<void>( p, ::free );
         origin = (void*)( (dip::uint8*)p + start*sz );
      }
   }
}


std::ostream & dip::operator<<(
   std::ostream & os,
   const dip::Image & img
){
   if( img.tensor_dims.size() == 0 ) {
      os << "Scalar image, ";
   } else {
      os << img.tensor_dims[0];
      for( int ii=1; ii<img.tensor_dims.size(); ii++ ) {
         os << "x" << img.tensor_dims[ii];
      }
      os << "-tensor image, ";
   }
   os << img.dims.size() << "-D, " << dip::_DataType::Name(img.datatype) << std::endl;
   os << "   sizes: ";
   for( int ii=0; ii<img.dims.size(); ii++ ) {
      os << img.dims[ii] << ", ";
   }
   os << std::endl;
   os << "   strides: ";
   for( int ii=0; ii<img.strides.size(); ii++ ) {
      os << img.strides[ii] << ", ";
   }
   os << std::endl;
   os << "   tensor strides: ";
   for( int ii=0; ii<img.tensor_strides.size(); ii++ ) {
      os << img.tensor_strides[ii] << ", ";
   }
   os << std::endl;
   if( img.origin ) {
      os << "   origin pointer: " << (dip::uint)img.origin << std::endl;
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
