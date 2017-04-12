/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
                    tensorElements == 1 ? E::IMAGE_NOT_SCALAR : E::NTENSORELEM_DONT_MATCH );
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
                    tensorElements == 1 ? E::IMAGE_NOT_SCALAR : E::NTENSORELEM_DONT_MATCH );
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
   // Shape and other main properties
   if( img.IsColor() ) {
      os << "Color image (" << img.Tensor() << ", " << img.ColorSpace() << "):";
   } else if( !img.IsScalar() ) {
      os << "Tensor image (" << img.Tensor() << "):";
   } else {
      os << "Scalar image:";
   }
   os << std::endl;
   // Image size and pixel size
   os << "    data type " << img.DataType().Name() << std::endl;
   if( img.Dimensionality() == 0 ) {
      os << "    sizes {} (0D)" << std::endl;
   } else {
      os << "    sizes {" << img.Sizes() << "} (" << img.Dimensionality() << "D)" << std::endl;
      if( img.HasPixelSize() ) {
         os << "    pixel size " << img.PixelSize( 0 );
         for( dip::uint ii = 1; ii < img.Dimensionality(); ++ii ) {
            os << " x " << img.PixelSize( ii );
         }
         os << std::endl;
      }
   }
   // Data storage
   os << "    strides {" << img.Strides() << "}, tensor stride " << img.TensorStride() << std::endl;
   if( img.IsForged() ) {
      os << "    data pointer:   " << img.Data() << " (shared among " << img.ShareCount() << " images)" << std::endl;
      os << "    origin pointer: " << img.Origin() << std::endl;
      /*
      dip::sint sstride;
      void* porigin;
      img.GetSimpleStrideAndOrigin( sstride, porigin );
      if( porigin ) {
         os << "    simple stride: " << sstride << ", origin: " << porigin << std::endl;
      } else {
         os << "    no simple stride" << std::endl;
      }
      */
   } else {
      os << "    not forged" << std::endl;
   }
   return os;
}

} // namespace dip
