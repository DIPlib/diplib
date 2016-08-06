/*
 * DIPlib 3.0
 * This file contains definitions for the DataType class and support functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_DATATYPE_H
#define DIP_DATATYPE_H

#include <vector>

#include "dip_types.h"


/// \file
/// Declares the dip::DataType class and support functions. This file is always included through diplib.h.


namespace dip {

class Image;      // Forward declaration.

//
// The DataType class.
//

/// DataType objects are used to indicate what the data type of
/// an image is. It is a simple enumeration type, but with some added
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
      SINT8,
      UINT16,
      SINT16,
      UINT32,
      SINT32,
      SFLOAT,
      DFLOAT,
      SCOMPLEX,
      DCOMPLEX,
   } dt;

   constexpr DataType() : dt( DT::SFLOAT ) {}
   constexpr DataType( DT _dt ) : dt( _dt ) {}

   constexpr explicit DataType( bin      ) : dt( DT::BIN      ) {}
   constexpr explicit DataType( uint8    ) : dt( DT::UINT8    ) {}
   constexpr explicit DataType( sint8    ) : dt( DT::SINT8    ) {}
   constexpr explicit DataType( uint16   ) : dt( DT::UINT16   ) {}
   constexpr explicit DataType( sint16   ) : dt( DT::SINT16   ) {}
   constexpr explicit DataType( uint32   ) : dt( DT::UINT32   ) {}
   constexpr explicit DataType( sint32   ) : dt( DT::SINT32   ) {}
   constexpr explicit DataType( sfloat   ) : dt( DT::SFLOAT   ) {}
   constexpr explicit DataType( dfloat   ) : dt( DT::DFLOAT   ) {}
   constexpr explicit DataType( scomplex ) : dt( DT::SCOMPLEX ) {}
   constexpr explicit DataType( dcomplex ) : dt( DT::DCOMPLEX ) {}

   /// DataType objects implicitly convert to the enumeration integer.
   constexpr operator int() const { return static_cast<int>(dt); }   // This one allows the use of DataType in a switch() statement

   /// DataType objects can be compared.
   bool operator==(DataType other) const { return dt == other.dt; }

   /// Returns a C-style string constant with a representation of the data type name.
   const char* Name() const {
      switch( dt ) {
         case DT::BIN:      return "BIN";
         case DT::UINT8:    return "UINT8";
         case DT::SINT8:    return "SINT8";
         case DT::UINT16:   return "UINT16";
         case DT::SINT16:   return "SINT16";
         case DT::UINT32:   return "UINT32";
         case DT::SINT32:   return "SINT32";
         case DT::SFLOAT:   return "SFLOAT";
         case DT::DFLOAT:   return "DFLOAT";
         case DT::SCOMPLEX: return "SCOMPLEX";
         case DT::DCOMPLEX: return "DCOMPLEX";
      };
   }

   /// Returns the size in bytes of the data type.
   dip::uint SizeOf() const {
      switch( dt ) {
         case DT::BIN:      return sizeof(dip::bin);
         case DT::UINT8:    return sizeof(dip::uint8);
         case DT::SINT8:    return sizeof(dip::sint8);
         case DT::UINT16:   return sizeof(dip::uint16);
         case DT::SINT16:   return sizeof(dip::sint16);
         case DT::UINT32:   return sizeof(dip::uint32);
         case DT::SINT32:   return sizeof(dip::sint32);
         case DT::SFLOAT:   return sizeof(dip::sfloat);
         case DT::DFLOAT:   return sizeof(dip::dfloat);
         case DT::SCOMPLEX: return sizeof(dip::scomplex);
         case DT::DCOMPLEX: return sizeof(dip::dcomplex);
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

   /// Specifies a collection of data types. Valid values are:
   /// - Class_Bin
   /// - Class_UInt8
   /// - Class_SInt8
   /// - Class_UInt16
   /// - Class_SInt16
   /// - Class_UInt32
   /// - Class_SInt32
   /// - Class_SFloat
   /// - Class_DFloat
   /// - Class_SComplex
   /// - Class_DComplex
   /// - Class_Binary = Class_Bin;
   /// - Class_UInt = Class_UInt8 + Class_UInt16 + Class_UInt32;
   /// - Class_SInt = Class_SInt8 + Class_SInt16 + Class_SInt32;
   /// - Class_Integer = Class_UInt + Class_SInt;
   /// - Class_Float = Class_SFloat + Class_DFloat;
   /// - Class_Real = Class_Integer + Class_Float;
   /// - Class_Complex = Class_SComplex + Class_DComplex;
   /// - Class_Unsigned = Class_UInt;
   /// - Class_Signed = Class_SInt + Class_Float + Class_Complex;
   /// - Class_Any = Class_Binary + Class_Real + Class_Complex;
   DIP_DECLARE_OPTIONS(Classes, 11);
   static DIP_DEFINE_OPTION( Classes, Class_Bin, 0 );
   static DIP_DEFINE_OPTION( Classes, Class_UInt8, 1 );
   static DIP_DEFINE_OPTION( Classes, Class_SInt8, 2 );
   static DIP_DEFINE_OPTION( Classes, Class_UInt16, 3 );
   static DIP_DEFINE_OPTION( Classes, Class_SInt16, 4 );
   static DIP_DEFINE_OPTION( Classes, Class_UInt32, 5 );
   static DIP_DEFINE_OPTION( Classes, Class_SInt32, 6 );
   static DIP_DEFINE_OPTION( Classes, Class_SFloat, 7 );
   static DIP_DEFINE_OPTION( Classes, Class_DFloat, 8 );
   static DIP_DEFINE_OPTION( Classes, Class_SComplex, 9 );
   static DIP_DEFINE_OPTION( Classes, Class_DComplex, 10 );
   static DIP_DEFINE_OPTION( Classes, Class_Binary, Class_Bin );
   static DIP_DEFINE_OPTION( Classes, Class_UInt, Class_UInt8 + Class_UInt16 + Class_UInt32 );
   static DIP_DEFINE_OPTION( Classes, Class_SInt, Class_SInt8 + Class_SInt16 + Class_SInt32 );
   static DIP_DEFINE_OPTION( Classes, Class_Integer, Class_UInt + Class_SInt );
   static DIP_DEFINE_OPTION( Classes, Class_Float, Class_SFloat + Class_DFloat );
   static DIP_DEFINE_OPTION( Classes, Class_Real, Class_Integer + Class_Float );
   static DIP_DEFINE_OPTION( Classes, Class_Complex, Class_SComplex + Class_DComplex );
   static DIP_DEFINE_OPTION( Classes, Class_Unsigned, Class_UInt );
   static DIP_DEFINE_OPTION( Classes, Class_Signed, Class_SInt + Class_Float + Class_Complex );
   static DIP_DEFINE_OPTION( Classes, Class_Any, Class_Binary + Class_Real + Class_Complex );


   //
   // Functions to suggest an output data type for all types of filters and operators
   //


   /// Returns a suitable floating-point type that can hold the samples of `type`.
   static DataType SuggestFloat( DataType type );
   /// Returns a suitable floating-point type that can hold the samples in `img`.
   static DataType SuggestFloat( const Image& img );

   /// Returns a suitable complex type that can hold the samples of `type`.
   static DataType SuggestComplex( DataType type );
   /// Returns a suitable complex type that can hold the samples in `img`.
   static DataType SuggestComplex( const Image& img );

   /// Returns a suitable floating-point or complex type that can hold the samples of `type`.
   static DataType SuggestFlex( DataType type );
   /// Returns a suitable floating-point or complex type that can hold the samples in `img`.
   static DataType SuggestFlex( const Image& img );

   /// Returns a suitable floating-point, complex or binary type that can hold the samples of `type`.
   static DataType SuggestFlexBin( DataType type );
   /// Returns a suitable floating-point, complex or binary type that can hold the samples in `img`.
   static DataType SuggestFlexBin( const Image& img );

   /// Returns a suitable floating-point, complex or binary type that can hold the result of an arithmetic computation performed with the two datatypes.
   static DataType SuggestArithmetic( DataType type1, DataType type2 );
   /// Returns a suitable floating-point, complex or binary type that can hold the result of an arithmetic computation performed on the two images.
   static DataType SuggestArithmetic( const Image& img1, const Image& img2 );

   /// Returns a suitable type that can hold any samples of the two datatypes.
   static DataType SuggestDiadicOperation( DataType type1, DataType type2 );
   /// Returns a suitable type that can hold the samples in the two images.
   static DataType SuggestDiadicOperation( const Image& img1, const Image& img2 );

};

typedef std::vector<DataType> DataTypeArray;   ///< An array to hold data types

//
// Constants that people will use where a DataType is needed
//

constexpr DataType DT_BIN       { DataType::DT::BIN      };
constexpr DataType DT_UINT8     { DataType::DT::UINT8    };
constexpr DataType DT_SINT8     { DataType::DT::SINT8    };
constexpr DataType DT_UINT16    { DataType::DT::UINT16   };
constexpr DataType DT_SINT16    { DataType::DT::SINT16   };
constexpr DataType DT_UINT32    { DataType::DT::UINT32   };
constexpr DataType DT_SINT32    { DataType::DT::SINT32   };
constexpr DataType DT_SFLOAT    { DataType::DT::SFLOAT   };
constexpr DataType DT_DFLOAT    { DataType::DT::DFLOAT   };
constexpr DataType DT_SCOMPLEX  { DataType::DT::SCOMPLEX };
constexpr DataType DT_DCOMPLEX  { DataType::DT::DCOMPLEX };

} // namespace dip

#endif // DIP_DATATYPE_H
