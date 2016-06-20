/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <iostream>
#include <algorithm>

#include "diplib.h"
#include "dip_numeric.h"

namespace dip {

//
bool Image::CompareProperties(
      const Image& src,
      Option::ThrowException throwException
) const {
   // TODO
   return true;
}

//
bool Image::CheckProperties(
      const dip::uint ndims,
      const struct DataType dt,
      Option::ThrowException throwException
) const {
   bool result = dims.size() == ndims;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   result &= datatype == dt;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DATA_TYPE_NOT_SUPPORTED );
   }
   return result;
}

bool Image::CheckProperties(
      const UnsignedArray& dimensions,
      const struct DataType dt,
      Option::ThrowException throwException
) const {
   bool result = dims == dimensions;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DIMENSIONS_DONT_MATCH );
   }
   result &= datatype == dt;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DATA_TYPE_NOT_SUPPORTED );
   }
   return result;
}

bool Image::CheckProperties(
      const UnsignedArray& dimensions,
      dip::uint tensorElements,
      const struct DataType dt,
      Option::ThrowException throwException
) const {
   bool result = dims == dimensions;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DIMENSIONS_DONT_MATCH );
   }
   result &= tensor.Elements() == tensorElements;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::TENSORSIZES_DONT_MATCH );
   }
   result &= datatype == dt;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DATA_TYPE_NOT_SUPPORTED );
   }
   return result;
}

//
std::ostream& operator<<(
      std::ostream& os,
      const Image& img
) {
   // Size and shape
   if( img.TensorElements() == 1 ) {
      os << "Scalar image, ";
   } else {
      os << img.TensorRows() << "x" << img.TensorColumns() << "-tensor image, ";
   }
   os << img.Dimensionality() << "-D, " << img.DataType().Name() << std::endl;
   os << "   sizes: ";
   dip::UnsignedArray dims = img.Dimensions();
   for( dip::uint ii=0; ii<dims.size(); ++ii ) {
      os << dims[ii] << ", ";
   }
   os << std::endl;
   // Strides
   os << "   strides: ";
   dip::IntegerArray strides = img.Strides();
   for( dip::uint ii=0; ii<strides.size(); ++ii ) {
      os << strides[ii] << ", ";
   }
   os << std::endl;
   os << "   tensor stride: " << img.TensorStride() << std::endl;
   // Data segment
   if( img.IsForged() ) {
      os << "   data pointer:   " << (dip::uint)img.Data() << " (shared among " << img.ShareCount() << " images)" << std::endl;
      os << "   origin pointer: " << (dip::uint)img.Origin() << std::endl;
      if( img.HasContiguousData() ) {
         if( img.HasNormalStrides() ) {
            os << "   strides are normal" << std::endl;
         } else {
            os << "   data are contiguous but strides are not normal" << std::endl;
         }
      }
      dip::uint stride; void* origin;
      img.GetSimpleStrideAndOrigin(stride, origin);
      if( stride != 0 ) {
         os << "   simple stride: " << stride << std::endl;
      } else {
         os << "   strides are not simple" << std::endl;
      }
   } else {
      os << "   not forged" << std::endl;
   }
   return os;
}

} // namespace dip
