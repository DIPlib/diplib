/*
 * New DIPlib include file
 * This file contains macros to simplify overloading on image data type.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_OVERLOAD_H
#define DIP_OVERLOAD_H

namespace dip {

/*
 * Macros to call overloaded functions according to a DataType variable.
 * "paramlist" must include the brackets: DIP_OVL_CALL_ALL( MyFilter, ( in, mask, fsize ), dtype )
 */

#define DIP_OVL_CALL_ALL(fname, paramlist, dtype) { \
   if( dtype == dip::DataType::BIN      ) { fname <dip::bin>      paramlist; } \
   if( dtype == dip::DataType::UINT8    ) { fname <dip::uint8>    paramlist; } \
   if( dtype == dip::DataType::UINT16   ) { fname <dip::uint16>   paramlist; } \
   if( dtype == dip::DataType::UINT32   ) { fname <dip::uint32>   paramlist; } \
   if( dtype == dip::DataType::SINT8    ) { fname <dip::sint8>    paramlist; } \
   if( dtype == dip::DataType::SINT16   ) { fname <dip::sint16>   paramlist; } \
   if( dtype == dip::DataType::SINT32   ) { fname <dip::sint32>   paramlist; } \
   if( dtype == dip::DataType::SFLOAT   ) { fname <dip::sfloat>   paramlist; } \
   if( dtype == dip::DataType::DFLOAT   ) { fname <dip::dfloat>   paramlist; } \
   if( dtype == dip::DataType::SCOMPLEX ) { fname <dip::scomplex> paramlist; } \
   if( dtype == dip::DataType::DCOMPLEX ) { fname <dip::dcomplex> paramlist; } }

// Make similar defines for:
// DIP_OVL_CALL_UINT       // uint8 + uint16 + uint32
// DIP_OVL_CALL_SINT       // sint8 + sint16 + sint32
// DIP_OVL_CALL_INTEGER    // UINT + SINT
// DIP_OVL_CALL_FLOAT      // sfloat + dfloat
// DIP_OVL_CALL_REAL       // INTEGER + FLOAT
// DIP_OVL_CALL_COMPLEX    // scomplex + dcomplex
// DIP_OVL_CALL_NONCOMPLEX // REAL + BINARY
// DIP_OVL_CALL_UNSIGNED   // UINT
// DIP_OVL_CALL_SIGNED     // SINT + FLOAT + COMPLEX
// DIP_OVL_CALL_BINARY     // is just one type, not really necessary, is it?
// DIP_OVL_CALL_NONBINARY  // REAL + COMPLEX

}

#endif
