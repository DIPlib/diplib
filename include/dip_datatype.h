/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types and support functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_DATATYPE_H
#define DIP_DATATYPE_H

#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc. (C++11 only!)
#include <complex>   // std::complex
#include <vector>    // std::vector

namespace dip {


//
// Integer types for image properties, pixel coordinates, loop indices, etc.
//
typedef std::ptrdiff_t sint;  ///< An integer type to be used for strides and similar measures.
typedef std::size_t    uint;  ///< An integer type to be used for sizes and the like.
            // ptrdiff_t and size_t are signed and unsigned integers of the same length as
            // pointers: 32 bits on 32-bit systems, 64 bits on 64-bit systems.


//
// Types for pixel values
//
typedef std::uint8_t          bin;     // Binary data stored in a single byte (don't use bool for pixels, it has implementation-defined size)
typedef std::uint8_t          uint8;
typedef std::uint16_t         uint16;
typedef std::uint32_t         uint32;
typedef std::int8_t           sint8;
typedef std::int16_t          sint16;
typedef std::int32_t          sint32;
typedef float                 sfloat;
typedef double                dfloat;
typedef std::complex<sfloat>  scomplex;
typedef std::complex<dfloat>  dcomplex;

// if 8 bits is not a byte...
static_assert( sizeof(uint8)==1, "8 bits is not a byte in your system!" );
// Seriously, though. We rely on this property, and there is no guarantee
// that a system actually has 8 bits in a byte. Maybe we should use char
// (which is guaranteed to be size 1) for generic pointer arithmetic?


//
// The DataType class.
//

/// DataType objects are used to indicate what the data type of
/// an image is. It is a simple enumeration type, but will some added
/// member functions that can be used to query the data type. A series
/// of constant expressions have been defined that should be used when
/// specifying a data type or testing for specific data types:
/// * dip::DT_BIN
/// * dip::DT_UINT8
/// * dip::DT_UINT16
/// * dip::DT_UINT32
/// * dip::DT_SINT8
/// * dip::DT_SINT16
/// * dip::DT_SINT32
/// * dip::DT_SFLOAT
/// * dip::DT_DFLOAT
/// * dip::DT_SCOMPLEX
/// * dip::DT_DCOMPLEX
///
/// It is possible to call DataType member functions on these constants:
///
///     dip::DT_BIN.SizeOf();
struct DataType {

   enum class DT {
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
   } dt;

   constexpr DataType() : dt(DT::SFLOAT) {}
   constexpr DataType( DT _dt ) : dt(_dt) {}

   /// DataType objects implicitly convert to the enumeration integer.
   constexpr operator int() const { return static_cast<int>(dt); }   // This one allows the use of DataType in a switch() statement

   /// Returns a C-style string constant with a representation of the data type name.
   const char* Name() const {
      switch( dt ) {
         case DT::BIN:      return "BIN";
         case DT::UINT8:    return "UINT8";
         case DT::UINT16:   return "UINT16";
         case DT::UINT32:   return "UINT32";
         case DT::SINT8:    return "SINT8";
         case DT::SINT16:   return "SINT16";
         case DT::SINT32:   return "SINT32";
         case DT::SFLOAT:   return "SFLOAT";
         case DT::DFLOAT:   return "DFLOAT";
         case DT::SCOMPLEX: return "SCOMPLEX";
         case DT::DCOMPLEX: return "DCOMPLEX";
      };
   }

   /// Returns the size in bytes of the data type.
   uint SizeOf() const {
      switch( dt ) {
         case DT::BIN:      return sizeof(bin);
         case DT::UINT8:    return sizeof(uint8);
         case DT::UINT16:   return sizeof(uint16);
         case DT::UINT32:   return sizeof(uint32);
         case DT::SINT8:    return sizeof(sint8);
         case DT::SINT16:   return sizeof(sint16);
         case DT::SINT32:   return sizeof(sint32);
         case DT::SFLOAT:   return sizeof(sfloat);
         case DT::DFLOAT:   return sizeof(dfloat);
         case DT::SCOMPLEX: return sizeof(scomplex);
         case DT::DCOMPLEX: return sizeof(dcomplex);
      };
   }

   /// Returns `true` if the data type is binary (equal to dip::DT_BIN).
   bool IsBinary() const {
      switch( dt ) {
         case DT::BIN:      return true;
         default:           return false;
      };
   }

   /// Returns `true` if the data type is an unsigned integer type.
   bool IsUInt() const {
      switch( dt ) {
         case DT::UINT8:    return true;
         case DT::UINT16:   return true;
         case DT::UINT32:   return true;
         default:           return false;
      };
   }

   /// Returns `true` if the data type is a signed integer type.
   bool IsSInt() const {
      switch( dt ) {
         case DT::SINT8:    return true;
         case DT::SINT16:   return true;
         case DT::SINT32:   return true;
         default:           return false;
      };
   }

   /// Returns `true` if the data type is an integer type.
   bool IsInteger() const {
      return IsUInt() || IsSInt();
   }

   /// Returns `true` if the data type is a floating point type.
   bool IsFloat() const {
      switch( dt ) {
         case DT::SFLOAT:   return true;
         case DT::DFLOAT:   return true;
         default:           return false;
      };
   }

   /// Returns `true` if the data type is real (floating point or integer).
   bool IsReal() const {
      return IsInteger() || IsFloat();
   }

   /// Returns `true` if the data type is complex.
   bool IsComplex() const {
      switch( dt ) {
         case DT::SCOMPLEX: return true;
         case DT::DCOMPLEX: return true;
         default:           return false;
      };
   }

   /// Returns `true` if the data type is an unsigned type (same as IsUInt()).
   bool IsUnsigned() const {
      return IsUInt();
   }

   /// Returns `true` if the data type is a signed type (signed integer, floating point or complex)
   bool IsSigned() const {
      return IsSInt() || IsFloat() || IsComplex();
   }

   /// DataType objects can be compared.
   bool operator==(DataType other) const {
      return dt == other.dt;
   }
};

//
// Constants that people will use where a DataType is needed
//

constexpr DataType DT_BIN       { DataType::DT::BIN      };
constexpr DataType DT_UINT8     { DataType::DT::UINT8    };
constexpr DataType DT_UINT16    { DataType::DT::UINT16   };
constexpr DataType DT_UINT32    { DataType::DT::UINT32   };
constexpr DataType DT_SINT8     { DataType::DT::SINT8    };
constexpr DataType DT_SINT16    { DataType::DT::SINT16   };
constexpr DataType DT_SINT32    { DataType::DT::SINT32   };
constexpr DataType DT_SFLOAT    { DataType::DT::SFLOAT   };
constexpr DataType DT_DFLOAT    { DataType::DT::DFLOAT   };
constexpr DataType DT_SCOMPLEX  { DataType::DT::SCOMPLEX };
constexpr DataType DT_DCOMPLEX  { DataType::DT::DCOMPLEX };


//
// Functions to suggest an output data type for all types of filters and operators
//

/// Returns a suitable floating-point type that can hold the data in `img`.
DataType DataTypeSuggest_Float(const Image& img);

/// Returns a suitable complex type that can hold the data in `img`.
DataType DataTypeSuggest_Complex(const Image& img);

/// Returns a suitable floating-point or complex type that can hold the data in `img`.
DataType DataTypeSuggest_Flex(const Image& img);

/// Returns a suitable floating-point, complex or binary type that can hold the data in `img`.
DataType DataTypeSuggest_FlexBin(const Image& img);

/// Returns a suitable floating-point, complex or binary type that can hold the result of a computation performed on the two images.
DataType DataTypeSuggest_Arithmetic(const Image& img1, const Image& img2);

//
// Arrays of signed, unsigned and floating-point values.
//    It's probably worth while to create our own short-vector optimized version of std::vector
//    since these represent dimensions, and we usually only have two or three of those.
//

typedef std::vector<sint>     IntegerArray;   ///< An array to hold strides, filter sizes, etc.
typedef std::vector<uint>     UnsignedArray;  ///< An array to hold dimensions, dimension lists, etc.
typedef std::vector<dfloat>   FloatArray;     ///< An array to hold filter parameters.
typedef std::vector<dcomplex> ComplexArray;   //   (used in only one obscure function in the old DIPlib.
typedef std::vector<bool>     BooleanArray;   ///< An array used as a dimension selector.
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


} // namespace dip

#endif // DIP_DATATYPE_H
