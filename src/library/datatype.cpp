/*
 * DIPlib 3.0
 * This file contains data-type--related functions.
 *
 * (c)2015-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"


/* These are the dip_DataTypeGetInfo() functions actually used:
DIP_DT_INFO_SIZEOF
DIP_DT_INFO_C2R               // Complex to real
DIP_DT_INFO_R2C               // Real to complex
DIP_DT_INFO_GET_MAXIMUM_PRECISION   // Used with dip_ImagesCheck(DIP_CKIM_MAX_PRECISION_MATCH) and in dip_DetermineDataType() in a way I don't understand.
DIP_DT_INFO_MAXIMUM_VALUE
DIP_DT_INFO_MINIMUM_VALUE
DIP_DT_INFO_PROPS             // Returns flags such as 'is int', 'is real', etc.
DIP_DT_INFO_SUGGEST_2         // A complex type that can hold the data
DIP_DT_INFO_SUGGEST_5         // A non-binary type that can hold the data (bin->sint8, others don't change)
DIP_DT_INFO_SUGGEST_6         // A double-precision type that can hold the data
DIP_DT_INFO_TO_FLEX           // A float or complex type (for result of arithmetic)
DIP_DT_INFO_TO_DIPIMAGE       // A float, complex or bin type (for result of arithmetic)
DIP_DT_INFO_TO_FLOAT          // A float type
 * Will not need:
DIP_DT_INFO_BITS              // SIZEOF * 8, what's the point? Used exactly in one place...
DIP_DT_INFO_BIN_TO_INT        // Bin type to same-size uint type -- we're not using bin16 or bin32...
DIP_DT_INFO_VALID             // won't need equivalent with "enum class".
 * Not used:
DIP_DT_INFO_GET_DATATYPE_NAME // not used
DIP_DT_INFO_GET_SIGNED        // not used; a signed type with same size
DIP_DT_INFO_SIZEOF_PTR        // not used
DIP_DT_INFO_SUGGEST_1         // not used
DIP_DT_INFO_SUGGEST_4         // not used
*/

namespace dip {

DataType DataType::SuggestFloat( DataType type ) {
   switch( type ) {
      default:
         return DT_SFLOAT;
      case DT_UINT32:
      case DT_SINT32:
      case DT_DFLOAT:
      case DT_DCOMPLEX:
         return DT_DFLOAT;
   }
}

DataType DataType::SuggestComplex( DataType type ) {
   switch( type ) {
      default:
         return DT_SCOMPLEX;
      case DT_UINT32:
      case DT_SINT32:
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
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataType::SuggestArithmetic( DataType type1, DataType type2 ) {
   type1 = DataType::SuggestFlexBin( type1 );
   type2 = DataType::SuggestFlexBin( type2 );
   if( type2 > type1 )
      dip::swap( type1, type2 );    // sort the two, it saves us lots of tests
   if( type1 == DT_DCOMPLEX )
      return DT_DCOMPLEX;
   if( ( type1 == DT_SCOMPLEX ) && ( type2 == DT_DFLOAT ) )
      return DT_DCOMPLEX;
   if( type1 == DT_SCOMPLEX )
      return DT_SCOMPLEX;
   if( type1 == DT_DFLOAT )
      return DT_DFLOAT;
   if( type1 == DT_SFLOAT )
      return DT_SFLOAT;
   return DT_BIN;
}

DataType DataType::SuggestDiadicOperation( DataType type1, DataType type2 ) {
   if( type1 == type2 )
      return type1;                 // short-cut
   if( type2 > type1 )
      dip::swap( type1, type2 );    // sort the two, it saves us lots of tests

   if( type1 == DT_DCOMPLEX )
      return DT_DCOMPLEX;
   if( ( type1 == DT_SCOMPLEX ) && ( type2 == DT_DFLOAT ) )
      return DT_DCOMPLEX;
   if( type1 == DT_SCOMPLEX )
      return DT_SCOMPLEX;

   if( type1 == DT_DFLOAT )
      return DT_DFLOAT;
   if( ( type1 == DT_SFLOAT ) && ( ( type2 == DT_UINT32 ) || ( type2 == DT_SINT32 ) ) )
      return DT_DFLOAT;
   if( type1 == DT_SFLOAT )
      return DT_SFLOAT;

   if( ( type1 == DT_SINT32 ) && ( type2 == DT_UINT32 ) )
      return DT_DFLOAT;
   if( type1 == DT_SINT32 )
      return DT_SINT32;
   if( ( type1 == DT_UINT32 ) && ( ( type2 == DT_SINT16 ) || ( type2 == DT_SINT8 ) ) )
      return DT_DFLOAT;
   if( type1 == DT_UINT32 )
      return DT_UINT32;

   if( ( type1 == DT_SINT16 ) && ( type2 == DT_UINT16 ) )
      return DT_SINT32;
   if( type1 == DT_SINT16 )
      return DT_SINT16;
   if( ( type1 == DT_UINT16 ) && ( type2 == DT_SINT8 ) )
      return DT_SINT32;
   if( type1 == DT_UINT16 )
      return DT_UINT16;

   if( ( type1 == DT_SINT8 ) && ( type2 == DT_UINT8 ) )
      return DT_SINT16;
   if( type1 == DT_SINT8 )
      return DT_SINT8;
   //if( type1 == DT_UINT8 ) // is always the case: if it's DT_BIN, then type2 is also DT_BIN, and we returned at the first test in this function.
   return DT_UINT8;
}


} // namespace dip
