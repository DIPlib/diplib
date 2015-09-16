/*
 * New DIPlib include file
 * This file contains data-type--related functions.
 *
 * (c)2015, Cris Luengo.
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


const char* dip::dt::Name( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::BIN:      return "BIN";
      case dip::DataType::UINT8:    return "UINT8";
      case dip::DataType::UINT16:   return "UINT16";
      case dip::DataType::UINT32:   return "UINT32";
      case dip::DataType::SINT8:    return "SINT8";
      case dip::DataType::SINT16:   return "SINT16";
      case dip::DataType::SINT32:   return "SINT32";
      case dip::DataType::SFLOAT:   return "SFLOAT";
      case dip::DataType::DFLOAT:   return "DFLOAT";
      case dip::DataType::SCOMPLEX: return "SCOMPLEX";
      case dip::DataType::DCOMPLEX: return "DCOMPLEX";
   };
}

dip::uint dip::dt::SizeOf( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::BIN:      return sizeof(dip::bin);
      case dip::DataType::UINT8:    return sizeof(dip::uint8);
      case dip::DataType::UINT16:   return sizeof(dip::uint16);
      case dip::DataType::UINT32:   return sizeof(dip::uint32);
      case dip::DataType::SINT8:    return sizeof(dip::sint8);
      case dip::DataType::SINT16:   return sizeof(dip::sint16);
      case dip::DataType::SINT32:   return sizeof(dip::sint32);
      case dip::DataType::SFLOAT:   return sizeof(dip::sfloat);
      case dip::DataType::DFLOAT:   return sizeof(dip::dfloat);
      case dip::DataType::SCOMPLEX: return sizeof(dip::scomplex);
      case dip::DataType::DCOMPLEX: return sizeof(dip::dcomplex);
   };
}


bool dip::dt::IsBinary( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::BIN:      return true;
      default:                      return false;
   };
}
bool dip::dt::IsUInt( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::UINT8:    return true;
      case dip::DataType::UINT16:   return true;
      case dip::DataType::UINT32:   return true;
      default:                      return false;
   };
}
bool dip::dt::IsSInt( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::SINT8:    return true;
      case dip::DataType::SINT16:   return true;
      case dip::DataType::SINT32:   return true;
      default:                      return false;
   };
}
bool dip::dt::IsInteger( dip::DataType dt ) {
   return IsUInt( dt ) || IsSInt( dt );
}
bool dip::dt::IsFloat( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::SFLOAT:   return true;
      case dip::DataType::DFLOAT:   return true;
      default:                      return false;
   };
}
bool dip::dt::IsReal( dip::DataType dt ) {
   return IsInteger( dt ) || IsFloat( dt );
}
bool dip::dt::IsComplex( dip::DataType dt ) {
   switch( dt ) {
      case dip::DataType::SCOMPLEX: return true;
      case dip::DataType::DCOMPLEX: return true;
      default:                      return false;
   };
}
bool dip::dt::IsUnsigned( dip::DataType dt ) {
   return IsUInt( dt );
}
bool dip::dt::IsSigned( dip::DataType dt ) {
   return IsSInt( dt ) || IsReal( dt ) || IsComplex( dt );
}
