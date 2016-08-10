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
      Option::CmpProps cmpProps,
      Option::ThrowException throwException
) const {
   if( cmpProps == Option::CmpProps_DataType ) {
      if( datatype != src.datatype ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Data type doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Dimensionality ) {
      if( dims.size() != src.dims.size() ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Dimensionality doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Dimensions ) {
      if( dims != src.dims ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, E::DIMENSIONS_DONT_MATCH );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Strides ) {
      if( strides != src.strides ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Strides don't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorShape ) {
      if( tensor != src.tensor ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Tensor shape doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorElements ) {
      if( tensor.Elements() != src.tensor.Elements() ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, E::TENSORSIZES_DONT_MATCH );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorStride ) {
      if( tstride != src.tstride ) {
         dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Tensor stride doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_ColorSpace ) { // TODO
      //if( ... ) {
      //   dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Color space doesn't match" );
      //   return false;
      //}
   }
   if( cmpProps == Option::CmpProps_PhysDims ) { // TODO
      //if( ... ) {
      //   dip_ThrowIf( throwException == Option::ThrowException::doThrow, "Physical dimensions don't match" );
      //   return false;
      //}
   }
   return true;
}

//
bool Image::CheckProperties(
      const dip::uint ndims,
      const dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   bool result = dims.size() == ndims;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   result &= dts == datatype;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DATA_TYPE_NOT_SUPPORTED );
   }
   return result;
}

bool Image::CheckProperties(
      const UnsignedArray& dimensions,
      const dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   bool result = dims == dimensions;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DIMENSIONS_DONT_MATCH );
   }
   result &= dts == datatype;
   if( !result && (throwException == Option::ThrowException::doThrow) ) {
      dip_Throw( E::DATA_TYPE_NOT_SUPPORTED );
   }
   return result;
}

bool Image::CheckProperties(
      const UnsignedArray& dimensions,
      dip::uint tensorElements,
      const dip::DataType::Classes dts,
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
   result &= dts == datatype;
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
      os << "   data pointer:   " << img.Data() << " (shared among " << img.ShareCount() << " images)" << std::endl;
      os << "   origin pointer: " << img.Origin() << std::endl;
      if( img.HasContiguousData() ) {
         if( img.HasNormalStrides() ) {
            os << "   strides are normal" << std::endl;
         } else {
            os << "   data are contiguous but strides are not normal" << std::endl;
         }
      }
      dip::uint stride; void* origin;
      img.GetSimpleStrideAndOrigin(stride, origin);
      if( origin ) {
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
