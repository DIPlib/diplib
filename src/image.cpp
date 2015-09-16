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

// Constructor: empty image of given sizes.
dip::Image::Image( dip::UnsignedArray size, dip::DataType dt ) {
   // ...   
}

// Constructor: Empty image of same size as 'src' image.
dip::Image::Image( const dip::Image & src, dip::DataType dt ) {
   // ...
}
                                       
// Constructor: Creates a 0-D image with the value of 'p' and of data type 'dt'.
dip::Image::Image( double p, dip::DataType dt ) {
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

void dip::DefineROI(
   dip::Image & dest,
   const dip::Image & src,
   const dip::UnsignedArray & origin,
   const dip::UnsignedArray & dims,
   const dip::IntegerArray & spacing
){
   // ...
}

void * dip::Image::GetData() const {
   DIPASSERT( IsForged(), DIP_E_IMAGE_NOT_FORGED );
   return origin;
}

dip::DataType dip::Image::GetDataType() const {
   return datatype;
}

dip::uint dip::Image::GetDimensionality() const {
   return dims.size();
}

dip::UnsignedArray dip::Image::GetDimensions() const {
   return dims;
}

dip::uint dip::Image::GetNumberOfPixels() const {
   dip::uint n = 1;
   for( dip::uint ii=0; ii<dims.size(); ii++ ) {
      n *= dims[ii];
      // We should add a test here to make sure we don't get overflow in the computation.
   }
   return n;
}

dip::IntegerArray dip::Image::GetStrides() const {
   return strides;
}

dip::uint dip::Image::GetTensorDimensionality() const {
   return tensor_dims.size();
}

dip::UnsignedArray dip::Image::GetTensorDimensions() const {
   return tensor_dims;
}

dip::uint dip::Image::GetNumberOfTensorComponents() const {
   dip::uint n = 1;
   for( dip::uint ii=0; ii<tensor_dims.size(); ii++ ) {
      n *= tensor_dims[ii];
   }
   return n;
}

dip::IntegerArray dip::Image::GetTensorStrides() const {
   return tensor_strides;
}

void dip::Image::SetDataType( dip::DataType dt ) {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
   datatype = dt;
}

void dip::Image::SetDimensions( const dip::UnsignedArray & d ) {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
   dims = d;
}

void dip::Image::SetStrides( const dip::IntegerArray & s ) {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
   strides = s;
}

void dip::Image::SetTensorDimensions( const dip::UnsignedArray & td ) {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
   tensor_dims = td;
   while( !tensor_dims.empty() && tensor_dims[tensor_dims.size()] == 1 ) {
      tensor_dims.pop_back();
   }
}

void dip::Image::SetTensorStrides( const dip::IntegerArray & ts ) {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
   tensor_strides = ts;
}

bool dip::Image::IsForged() const {
   if( origin )
      return true;
   else
      return false;
}

bool dip::Image::HasContiguousData() const {
   DIPASSERT( IsForged(), DIP_E_IMAGE_NOT_FORGED );
   dip::uint size = GetNumberOfPixels() * GetNumberOfTensorComponents();
   dip::sint start;
   dip::uint sz;
   GetDataBlockSizeAndStart( sz, start );
   return sz == size;
}

bool dip::Image::HasNormalStrides() const {
   DIPASSERT( IsForged(), DIP_E_IMAGE_NOT_FORGED );
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

bool dip::Image::HasValidStrides() const {
   if( dims.size() != strides.size() ) {
      return false;
   }
   if( tensor_dims.size() != tensor_strides.size() ) {
      return false;
   }
   return true;
}

void dip::Image::ComputeStrides() {
   DIPTS( IsForged(), DIP_E_IMAGE_NOT_RAW );
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
      dip::uint sz = dip::dt::SizeOf( datatype );
      void * p = ::malloc( size*sz );
      datablock = std::shared_ptr<void>( p , std::ptr_fun( ::free ) );
      origin = (void*)( (dip::uint8*)p + start*sz );
   }
}

void dip::Image::Strip() {
   if( IsForged() ) {
      datablock = nullptr; // Automatically frees old memory if no other pointers to it exist.
      origin = nullptr;    // Keep this one in sync!
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
   os << img.dims.size() << "-D, " << dip::dt::Name(img.datatype) << std::endl;
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




// Just a test!

template <typename TPI>
static void dip__MyFunction( void* vin ) {
   TPI* in = static_cast <TPI*> ( vin );
   std::cout << "Data type = " << typeid(*in).name() << std::endl;
}

void dip::MyFunction( dip::Image image ) {
   dip::DataType dt = dip::DataType::DFLOAT;
   double data;
   void* in = &data;
   DIP_OVL_CALL_ALL( dip__MyFunction, (in), dt );
}
