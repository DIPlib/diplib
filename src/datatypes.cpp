/*
 * DIPlib 3.0
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

using namespace dip;

DataType DataTypeSuggest_Float( const Image& img ) {
   switch( img.GetDataType() ) {
      default:
         return DT_SFLOAT;
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataTypeSuggest_Complex(const Image& img) {
   switch( img.GetDataType() ) {
      default:
         return DT_SCOMPLEX;
      case DT_DFLOAT:
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataTypeSuggest_Flex(const Image& img) {
   switch( img.GetDataType() ) {
      default:
         return DT_SFLOAT;
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataTypeSuggest_FlexBin(const Image& img) {
   switch( img.GetDataType() ) {
      case DT_BIN:
         return DT_BIN;
      default:
         return DT_SFLOAT;
      case DT_DFLOAT:
         return DT_DFLOAT;
      case DT_SCOMPLEX:
         return DT_SCOMPLEX;
      case DT_DCOMPLEX:
         return DT_DCOMPLEX;
   }
}

DataType DataTypeSuggest_Arithmetic(const Image& img1, const Image& img2) {
   /*** This is the old DIPlib's table:
   DataType combos[][3] = {
      { DT_DCOMPLEX, 0,         DT_DCOMPLEX },
      { DT_SCOMPLEX, DT_DFLOAT, DT_DCOMPLEX },
      { DT_SCOMPLEX, 0,         DT_SCOMPLEX },
      { DT_DFLOAT,   0,         DT_DFLOAT   },
      { DT_SFLOAT,   0,         DT_SFLOAT   },
      { DT_SINT32,   0,         DT_SINT32   },
      { DT_SINT16,   DT_UINT32, DT_SINT32   },
      { DT_SINT16,   0,         DT_SINT16   },
      { DT_SINT8,    DT_UINT32, DT_SINT32   },
      { DT_SINT8,    DT_UINT16, DT_SINT16   },
      { DT_SINT8,    0,         DT_SINT8    },
      { DT_UINT32,   0,         DT_UINT32   },
      { DT_UINT16,   0,         DT_UINT16   },
      { DT_UINT8,    0,         DT_UINT8    },
      { DT_BIN,      0,         DT_BIN      },
      { 0,           0,         0           }, // This line is the terminator marker
   };
    *** The DIPimage-style table would be this, with the DT_SFLOAT as default result:
   DataType combos[][3] = {
      { DT_DCOMPLEX, 0,         DT_DCOMPLEX },
      { DT_SCOMPLEX, DT_DFLOAT, DT_DCOMPLEX },
      { DT_SCOMPLEX, 0,         DT_SCOMPLEX },
      { DT_DFLOAT,   0,         DT_DFLOAT   },
      { DT_BIN,      DT_BIN,    DT_BIN      },
      { 0,           0,         0           }, // This line is the terminator marker
   };
   DataType type1 = img1.GetDataType();
   DataType type2 = img2.GetDataType();
   for( uint ix = 0; combos[ix][0]; ++ix ) {
      if( combos[ix][1] ) {
         if( (( type1 == combos[ix][0] ) && ( type2 == combos[ix][1] )) ||
             (( type2 == combos[ix][0] ) && ( type1 == combos[ix][1] )) )
            return combos[ix][2];
      } else {
         if( ( type1 == combos[ix][0] ) || ( type2 == combos[ix][0] ) )
            return combos[ix][2];
      }
   }
   return DT_SFLOAT;
    *** But we cannot convert 0 to a DataType, and we don't allow illegal
    *** data types in the enum class. */

   DataType type1 = img1.GetDataType();
   DataType type2 = img2.GetDataType();
   if( (type1 == DT_DCOMPLEX) || (type2 == DT_DCOMPLEX) )
      return DT_DCOMPLEX;
   if( ((type1 == DT_SCOMPLEX) && (type2 == DT_DFLOAT)) ||
       ((type2 == DT_SCOMPLEX) && (type1 == DT_DFLOAT)) )
      return DT_DCOMPLEX;
   if( (type1 == DT_SCOMPLEX) || (type2 == DT_SCOMPLEX) )
      return DT_SCOMPLEX;
   if( (type1 == DT_DFLOAT) || (type2 == DT_DFLOAT) )
      return DT_DFLOAT;
   if( (type1 == DT_BIN) && (type2 == DT_BIN) )
      return DT_BIN;
   return DT_SFLOAT;

}
