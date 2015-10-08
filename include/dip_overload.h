/*
 * DIPlib 3.0
 * This file contains macros to simplify overloading on image data type.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_OVERLOAD_H
#define DIP_OVERLOAD_H

// TODO: This file requires some love, this will be too much repetition
// of code. Ideally we'd generate the boiler plates automatically,
// otherwise we might add an additional parameter that specifies which
// of the datatypes are allowed (UINT, FLOAT, NONCOMPLEX, ALL, etc.).

/// Calls the overloaded function for dip::DataType `dtype` and ignores
/// any output value.
/// Note that `paramlist` must include brackets:
///
///     DIP_OVL_CALL_ALL( myFunc, ( param1, param2 ), datatype );
///
/// Note also that dip::bin and dip::uint8 are actually the same data
/// type, so if you intend the binary and uint8 processing to be different,
/// you need to pass the data type as a parameter to your function.
#define DIP_OVL_CALL_ALL( fname, paramlist, dtype )                \
   switch( dtype ) {                                               \
   case dip::DT_BIN      : fname <dip::bin>      paramlist; break; \
   case dip::DT_UINT8    : fname <dip::uint8>    paramlist; break; \
   case dip::DT_UINT16   : fname <dip::uint16>   paramlist; break; \
   case dip::DT_UINT32   : fname <dip::uint32>   paramlist; break; \
   case dip::DT_SINT8    : fname <dip::sint8>    paramlist; break; \
   case dip::DT_SINT16   : fname <dip::sint16>   paramlist; break; \
   case dip::DT_SINT32   : fname <dip::sint32>   paramlist; break; \
   case dip::DT_SFLOAT   : fname <dip::sfloat>   paramlist; break; \
   case dip::DT_DFLOAT   : fname <dip::dfloat>   paramlist; break; \
   case dip::DT_SCOMPLEX : fname <dip::scomplex> paramlist; break; \
   case dip::DT_DCOMPLEX : fname <dip::dcomplex> paramlist; break; \
   default: throw dip::Error( dip::E::DATA_TYPE_NOT_SUPPORTED );   \
}

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
// DIP_OVL_CALL_NONBINARY  // REAL + COMPLEX

/// Calls the overloaded function for dip::DataType `dtype` and assigns
/// the output value to variable `x`.
/// Note that `paramlist` must include brackets:
///
///     DIP_OVL_CALL_ALL( myFunc, ( param1, param2 ), datatype );
///
/// Note also that dip::bin and dip::uint8 are actually the same data
/// type, so if you intend the binary and uint8 processing to be different,
/// you need to pass the data type as a parameter to your function.
#define DIP_OVL_CALL_ASSIGN_ALL( x, fname, paramlist, dtype )          \
   switch( dtype ) {                                                   \
   case dip::DT_BIN      : x = fname <dip::bin>      paramlist; break; \
   case dip::DT_UINT8    : x = fname <dip::uint8>    paramlist; break; \
   case dip::DT_UINT16   : x = fname <dip::uint16>   paramlist; break; \
   case dip::DT_UINT32   : x = fname <dip::uint32>   paramlist; break; \
   case dip::DT_SINT8    : x = fname <dip::sint8>    paramlist; break; \
   case dip::DT_SINT16   : x = fname <dip::sint16>   paramlist; break; \
   case dip::DT_SINT32   : x = fname <dip::sint32>   paramlist; break; \
   case dip::DT_SFLOAT   : x = fname <dip::sfloat>   paramlist; break; \
   case dip::DT_DFLOAT   : x = fname <dip::dfloat>   paramlist; break; \
   case dip::DT_SCOMPLEX : x = fname <dip::scomplex> paramlist; break; \
   case dip::DT_DCOMPLEX : x = fname <dip::dcomplex> paramlist; break; \
   default: throw dip::Error( dip::E::DATA_TYPE_NOT_SUPPORTED );       \
}

/// Assigns a pointer to the overloaded function to the variable `x`.
/// Note that dip::bin and dip::uint8 are actually the same data
/// type, so if you intend the binary and uint8 processing to be different,
/// you need to pass the data type as a parameter to your function.
#define DIP_OVL_ASSIGN_ALL( x, fname, dtype )                    \
   switch( dtype ) {                                             \
   case dip::DT_BIN      : x = fname <dip::bin>     ; break;     \
   case dip::DT_UINT8    : x = fname <dip::uint8>   ; break;     \
   case dip::DT_UINT16   : x = fname <dip::uint16>  ; break;     \
   case dip::DT_UINT32   : x = fname <dip::uint32>  ; break;     \
   case dip::DT_SINT8    : x = fname <dip::sint8>   ; break;     \
   case dip::DT_SINT16   : x = fname <dip::sint16>  ; break;     \
   case dip::DT_SINT32   : x = fname <dip::sint32>  ; break;     \
   case dip::DT_SFLOAT   : x = fname <dip::sfloat>  ; break;     \
   case dip::DT_DFLOAT   : x = fname <dip::dfloat>  ; break;     \
   case dip::DT_SCOMPLEX : x = fname <dip::scomplex>; break;     \
   case dip::DT_DCOMPLEX : x = fname <dip::dcomplex>; break;     \
   default: throw dip::Error( dip::E::DATA_TYPE_NOT_SUPPORTED ); \
}

#endif
