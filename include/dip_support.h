/*
 * New DIPlib include file
 * This file contains definitions for support classes and functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_SUPPORT_H
#define DIP_SUPPORT_H

#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc. (C++11 only!)
#include <complex>   // std::complex
#include <vector>    // std::vector
#include <stdexcept> // std::logic_error and other exception classes

namespace dip {

/*
 * Basic Data Types
 */

// Integer types for image properties, pixel coordinates, loop indices, etc.
typedef std::ptrdiff_t sint;  // For strides and similar measures
typedef std::size_t uint;     // For sizes and the like
            // ptrdiff_t and size_t are signed and unsigned integers of the same length as
            // pointers: 32 bits on 32-bit systems, 64 bits on 64-bit systems.

// Types for pixel values
typedef std::uint8_t          bin;     // Binary data stored in a single byte
typedef std::uint8_t          uint8;
typedef std::uint16_t         uint16;
typedef std::uint32_t         uint32;
typedef std::int8_t           sint8;
typedef std::int16_t          sint16;
typedef std::int32_t          sint32;
typedef float                 sfloat;
typedef double                dfloat;  // Also to be used for general floating-point stuff
typedef std::complex<sfloat>  scomplex;
typedef std::complex<dfloat>  dcomplex;

// Corresponding enum types
enum class DataType {
   BIN,
   UINT8,
   UINT16,
   UINT32,
   SINT8,
   SINT16,
   SINT32,
   SFLOAT,
   DFLOAT,
   SCOMPLEX,
   DCOMPLEX,
};

namespace dt {

   const char* Name( DataType );
   uint SizeOf( DataType );

   bool IsBinary( DataType );       // IsNoNBinary(dt) = !IsBinary(dt);
   bool IsUInt( DataType );
   bool IsSInt( DataType );
   bool IsInteger( DataType );
   bool IsFloat( DataType );
   bool IsReal( DataType );
   bool IsComplex( DataType );      // IsNoNComplex(dt) = !IsComplex(dt);
   bool IsUnsigned( DataType );
   bool IsSigned( DataType );

}

// Arrays of signed, unsigned and floating-point values.
// It's probably worth while to create our own short-vector optimized version of std::vector
//    since these represent dimensions, and we usually only have two or three of those.
typedef std::vector<sint> IntegerArray;   // strides
typedef std::vector<uint> UnsignedArray;  // dimensions
typedef std::vector<dfloat> FloatArray;
   // IntegerArray A;
   // IntegerArray A(n);
   // IntegerArray A(n,0);
   // IntegerArray A {10,20,5};
   // A.assign(n,0);
   // A.assign({10,20,5});
   // A = ...;
   // A.size();
   // A[ii];
   // A.data();
   // A.resize(n);
   // A.resize(n,0);
   // A == B;

/*
 * Strings
 */

typedef std::string String;
typedef std::vector<String> StringArray;

/*
 * Exceptions
 */

/*
 * Color spaces
 */

class ColorSpace {
   public:
      String name;               // We use strings to specify color space.
      dfloat whitepoint[3][3];   // This will hold the whitepoint XYZ array.

      // constructors & destructor
      ColorSpace() {}
      explicit ColorSpace( const String & );
      explicit ColorSpace( const String &, const dfloat (&a) [3][3] );
};

/*
 * Physical dimensions
 */

class PhysicalDimensions {
   private:
      //StringArray spatial_value;
      //FloatArray spatial_size;
      //String intensity_unit;
      //dfloat intensity_value = 0;
   public:
      // constructors & destructor
      // getters
      // setters
      // other
            // Some static? methods to multiply and divide units
            // Some knowledge about standard units and unit conversions
};

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

} // namespace dip

#endif // DIP_SUPPORT_H
