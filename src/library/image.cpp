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

namespace dip {

//
bool Image::CompareProperties(
      Image const& src,
      Option::CmpProps cmpProps,
      Option::ThrowException throwException
) const {
   if( cmpProps == Option::CmpProps_DataType ) {
      if( dataType_ != src.dataType_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Data type doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Dimensionality ) {
      if( sizes_.size() != src.sizes_.size() ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Dimensionality doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Sizes ) {
      if( sizes_ != src.sizes_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::SIZES_DONT_MATCH );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_Strides ) {
      if( strides_ != src.strides_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Strides don't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorShape ) {
      if( tensor_ != src.tensor_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Tensor shape doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorElements ) {
      if( tensor_.Elements() != src.tensor_.Elements() ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::NTENSORELEM_DONT_MATCH );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_TensorStride ) {
      if( tensorStride_ != src.tensorStride_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Tensor stride doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_ColorSpace ) {
      if( colorSpace_ != src.colorSpace_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Color space doesn't match" );
         return false;
      }
   }
   if( cmpProps == Option::CmpProps_PixelSize ) {
      if( pixelSize_ != src.pixelSize_ ) {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, "Pixel sizes don't match" );
         return false;
      }
   }
   return true;
}

//
bool Image::CheckProperties(
      dip::uint ndims,
      dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   if( sizes_.size() != ndims ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DIMENSIONALITY_NOT_SUPPORTED );
      return false;
   }
   if( dts != dataType_ ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DATA_TYPE_NOT_SUPPORTED );
      return false;
   }
   return true;
}

bool Image::CheckProperties(
      dip::uint ndims,
      dip::uint tensorElements,
      dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   if( sizes_.size() != ndims ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DIMENSIONALITY_NOT_SUPPORTED );
      return false;
   }
   if( tensor_.Elements() != tensorElements ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW,
                    tensorElements == 1 ? E::NOT_SCALAR : E::NTENSORELEM_DONT_MATCH );
      return false;
   }
   if( dts != dataType_ ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DATA_TYPE_NOT_SUPPORTED );
      return false;
   }
   return true;
}

bool Image::CheckProperties(
      UnsignedArray const& sizes,
      dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   if( sizes_ != sizes ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DIMENSIONALITY_NOT_SUPPORTED );
      return false;
   }
   if( dts != dataType_ ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DATA_TYPE_NOT_SUPPORTED );
      return false;
   }
   return true;
}

bool Image::CheckProperties(
      UnsignedArray const& sizes,
      dip::uint tensorElements,
      dip::DataType::Classes dts,
      Option::ThrowException throwException
) const {
   if( sizes_ != sizes ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DIMENSIONALITY_NOT_SUPPORTED );
      return false;
   }
   if( tensor_.Elements() != tensorElements ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW,
                    tensorElements == 1 ? E::NOT_SCALAR : E::NTENSORELEM_DONT_MATCH );
      return false;
   }
   if( dts != dataType_ ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::DATA_TYPE_NOT_SUPPORTED );
      return false;
   }
   return true;
}

bool Image::CheckIsMask(
      UnsignedArray const& sizes,
      Option::AllowSingletonExpansion allowSingletonExpansion,
      Option::ThrowException throwException
) const {
   if( sizes_ != sizes ) {
      if( allowSingletonExpansion == Option::AllowSingletonExpansion::DO_ALLOW ) {
         if( IsSingletonExpansionPossible( sizes )) {
            DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::MASK_DIMENSIONS_NOT_COMPATIBLE );
            return false;
         }
      } else {
         DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::SIZES_DONT_MATCH );
         return false;
      }
   }
   if( tensor_.Elements() != 1 ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::MASK_NOT_SCALAR );
      return false;
   }
   if( dataType_ != DT_BIN ) {
      DIP_THROW_IF( throwException == Option::ThrowException::DO_THROW, E::MASK_NOT_BINARY );
      return false;
   }
   return true;
}

//
std::ostream& operator<<(
      std::ostream& os,
      Image const& img
) {
   // Shape and other main propertiees
   if( img.TensorElements() == 1 ) {
      os << "Scalar image, ";
   } else {
      os << img.TensorRows() << "x" << img.TensorColumns() << "-tensor image, ";
   }
   os << img.Dimensionality() << "-D, " << img.DataType().Name();
   if( img.IsColor() ) {
      os << ", color image: " << img.ColorSpace();
   }
   os << std::endl;

   // Image size
   os << "   sizes: ";
   dip::UnsignedArray const& sizes = img.Sizes();
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      os << ( ii > 0 ? ", " : "" ) << sizes[ ii ];
   }
   os << std::endl;

   // Pixel size
   if( img.HasPixelSize() ) {
      os << "   pixel size: ";
      dip::PixelSize const& ps = img.PixelSize();
      for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
         os << ( ii > 0 ? " x " : "" ) << ps[ ii ];
      }
      os << std::endl;
   }

   // Strides
   os << "   strides: ";
   dip::IntegerArray const& strides = img.Strides();
   for( dip::uint ii = 0; ii < strides.size(); ++ii ) {
      os << ( ii > 0 ? ", " : "" ) << strides[ ii ];
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

      dip::uint stride;
      void* origin;
      img.GetSimpleStrideAndOrigin( stride, origin );
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
