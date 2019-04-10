/*
 * DIPlib 3.0
 * This file contains data-type--related functions.
 *
 * (c)2015-2019, Cris Luengo.
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

#include "diplib.h"


/* These are the dip_DataTypeGetInfo() functions actually used in the old DIPlib, and what to use in the new infrastructure:
DIP_DT_INFO_SIZEOF                  // DataType::SizeOf
DIP_DT_INFO_C2R                     // DataType::SuggestFloat, FloatType
DIP_DT_INFO_R2C                     // DataType::SuggestComplex, ComplexType
DIP_DT_INFO_GET_MAXIMUM_PRECISION   // Used with dip_ImagesCheck(DIP_CKIM_MAX_PRECISION_MATCH) and in dip_DetermineDataType() in a way I don't understand.
DIP_DT_INFO_MAXIMUM_VALUE           // std::numeric_limits<T>::max()
DIP_DT_INFO_MINIMUM_VALUE           // std::numeric_limits<T>::lowest()
DIP_DT_INFO_PROPS                   // DataType::IsReal, DataType::IsComplex, etc.
DIP_DT_INFO_SUGGEST_2               // DataType::SuggestComplex
DIP_DT_INFO_SUGGEST_5               // A non-binary type that can hold the data (bin->sint8, others don't change) => `if (dt.IsBinary()) { dt = dip::DT_SINT8; }`
DIP_DT_INFO_SUGGEST_6               // DataType::SuggestDouble
DIP_DT_INFO_TO_FLEX                 // DataType::SuggestFlex, FlexType
DIP_DT_INFO_TO_DIPIMAGE             // DataType::SuggestFlexBin, FlexBinType
DIP_DT_INFO_TO_FLOAT                // DataType::SuggestFloat, FloatType
*/

namespace dip {

DataType DataType::SuggestInteger( DataType type ) {
   switch( type ) {
      case DT_BIN:
         return DT_UINT8;
      case DT_SFLOAT:
      case DT_SCOMPLEX:
         return DT_SINT32;
      case DT_DFLOAT:
      case DT_DCOMPLEX:
         return DT_SINT64;
      default:
         return type;
   }
}

DataType DataType::SuggestSigned( DataType type ) {
   switch( type ) {
      case DT_BIN:
         return DT_SINT8;
      case DT_UINT8:
         return DT_SINT16;
      case DT_UINT16:
         return DT_SINT32;
      case DT_UINT32:
      case DT_UINT64:
         return DT_SINT64;
      default:
         return type;
   }
}

DataType DataType::SuggestFloat( DataType type ) {
   switch( type ) {
      default:
         return DT_SFLOAT;
      case DT_UINT32:
      case DT_SINT32:
      case DT_UINT64:
      case DT_SINT64:
      case DT_DFLOAT:
      case DT_DCOMPLEX:
         return DT_DFLOAT;
   }
}

DataType DataType::SuggestDouble( DataType type ) {
   switch( type ) {
      default:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataType::SuggestComplex( DataType type ) {
   switch( type ) {
      default:
         return DT_SCOMPLEX;
      case DT_UINT32:
      case DT_SINT32:
      case DT_UINT64:
      case DT_SINT64:
      case DT_DFLOAT:
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}


DataType DataType::SuggestFlex( DataType type ) {
   switch( type ) {
      default:
         return DT_SFLOAT;
      case DT_UINT32:
      case DT_SINT32:
      case DT_UINT64:
      case DT_SINT64:
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataType::SuggestFlexBin( DataType type ) {
   switch( type ) {
      case DT_BIN:
         return DT_BIN;
      default:
         return DT_SFLOAT;
      case DT_UINT32:
      case DT_SINT32:
      case DT_UINT64:
      case DT_SINT64:
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataType::SuggestAbs( DataType type ) {
   switch( type ) {
      default:
         return type;
      case DT_SINT8:
         return DT_UINT8;
      case DT_SINT16:
         return DT_UINT16;
      case DT_SINT32:
         return DT_UINT32;
      case DT_SINT64:
         return DT_UINT64;
      case DT_SCOMPLEX:
         return DT_SFLOAT;
      case DT_DCOMPLEX:
         return DT_DFLOAT;
   }
}

DataType DataType::SuggestReal( DataType type ) {
   switch( type ) {
      default:
         return type;
      case DT_BIN:
         return DT_UINT8;
      case DT_SCOMPLEX:
         return DT_SFLOAT;
      case DT_DCOMPLEX:
         return DT_DFLOAT;
   }
}

DataType DataType::SuggestArithmetic( DataType type1, DataType type2 ) {
   type1 = DataType::SuggestFlexBin( type1 );
   type2 = DataType::SuggestFlexBin( type2 );
   if( type2 > type1 ) {
      dip::swap( type1, type2 ); // sort the two, it saves us lots of tests
   }
   if( type1 == DT_DCOMPLEX ) {
      return DT_DCOMPLEX;
   }
   if(( type1 == DT_SCOMPLEX ) && ( type2 == DT_DFLOAT )) {
      return DT_DCOMPLEX;
   }
   if( type1 == DT_SCOMPLEX ) {
      return DT_SCOMPLEX;
   }
   if( type1 == DT_DFLOAT ) {
      return DT_DFLOAT;
   }
   if( type1 == DT_SFLOAT ) {
      return DT_SFLOAT;
   }
   return DT_BIN;
}

DataType DataType::SuggestDyadicOperation( DataType type1, DataType type2 ) {
   if( type1 == type2 ) {
      return type1;                 // short-cut
   }
   if( type2 > type1 ) {            // compares after casting to integer, see `enum class DT` for order
      dip::swap( type1, type2 );    // sort the two, it saves us lots of tests
   }

   if( type1 == DT_DCOMPLEX ) {
      return DT_DCOMPLEX;
   }
   if( type1 == DT_SCOMPLEX ) {
      if(( type2 == DT_DFLOAT ) || ( type2 == DT_UINT64 ) || ( type2 == DT_SINT64 ) || ( type2 == DT_UINT32 ) || ( type2 == DT_SINT32 )) {
         return DT_DCOMPLEX;
      }
      return DT_SCOMPLEX;
   }

   if( type1 == DT_DFLOAT ) {
      return DT_DFLOAT;
   }
   if( type1 == DT_SFLOAT ) {
      if(( type2 == DT_UINT64 ) || ( type2 == DT_SINT64 ) || ( type2 == DT_UINT32 ) || ( type2 == DT_SINT32 )) {
         return DT_DFLOAT;
      }
      return DT_SFLOAT;
   }

   if( type1 == DT_SINT64 ) {
      return DT_SINT64;
   }
   if( type1 == DT_UINT64 ) {
      if(( type2 == DT_SINT32 ) || ( type2 == DT_SINT16 ) || ( type2 == DT_SINT8 )) {
         return DT_SINT64;
      }
      return DT_UINT64;
   }

   if( type1 == DT_SINT32 ) {
      if( type2 == DT_UINT32 ) {
         return DT_SINT64;
      }
      return DT_SINT32;
   }
   if( type1 == DT_UINT32 ) {
      if(( type2 == DT_SINT16 ) || ( type2 == DT_SINT8 )) {
         return DT_SINT64;
      }
      return DT_UINT32;
   }

   if( type1 == DT_SINT16 ) {
      if( type2 == DT_UINT16 ) {
         return DT_SINT32;
      }
      return DT_SINT16;
   }
   if( type1 == DT_UINT16 ) {
      if( type2 == DT_SINT8 ) {
         return DT_SINT32;
      }
      return DT_UINT16;
   }

   if( type1 == DT_SINT8 ) {
      if( type2 == DT_UINT8 ) {
         return DT_SINT16;
      }
      return DT_SINT8;
   }
   //if( type1 == DT_UINT8 ) // is always the case: if it's DT_BIN, then type2 is also DT_BIN, and we returned at the first test in this function.
   return DT_UINT8;
}

// Static data members of dip::DataType need to have a place in memory, which is claimed here.
constexpr DataType::Classes DataType::Class_Bin;
constexpr DataType::Classes DataType::Class_UInt8;
constexpr DataType::Classes DataType::Class_SInt8;
constexpr DataType::Classes DataType::Class_UInt16;
constexpr DataType::Classes DataType::Class_SInt16;
constexpr DataType::Classes DataType::Class_UInt32;
constexpr DataType::Classes DataType::Class_SInt32;
constexpr DataType::Classes DataType::Class_UInt64;
constexpr DataType::Classes DataType::Class_SInt64;
constexpr DataType::Classes DataType::Class_SFloat;
constexpr DataType::Classes DataType::Class_DFloat;
constexpr DataType::Classes DataType::Class_SComplex;
constexpr DataType::Classes DataType::Class_DComplex;
constexpr DataType::Classes DataType::Class_Binary;
constexpr DataType::Classes DataType::Class_UInt;
constexpr DataType::Classes DataType::Class_SInt;
constexpr DataType::Classes DataType::Class_Integer;
constexpr DataType::Classes DataType::Class_IntOrBin;
constexpr DataType::Classes DataType::Class_Float;
constexpr DataType::Classes DataType::Class_Complex;
constexpr DataType::Classes DataType::Class_Flex;
constexpr DataType::Classes DataType::Class_FlexBin;
constexpr DataType::Classes DataType::Class_Unsigned;
constexpr DataType::Classes DataType::Class_Signed;
constexpr DataType::Classes DataType::Class_Real;
constexpr DataType::Classes DataType::Class_SignedReal;
constexpr DataType::Classes DataType::Class_NonBinary;
constexpr DataType::Classes DataType::Class_NonComplex;
constexpr DataType::Classes DataType::Class_All;

} // namespace dip
