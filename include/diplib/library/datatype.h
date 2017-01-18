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
#include <utility>

#include "diplib/library/types.h"


/// \file
/// \brief Declares the dip::DataType class and support functions. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


//
// The DataType class.
//

/// \brief DataType objects are used to indicate what the data type of
/// an image is.
///
/// It is a simple enumeration type, but with some added
/// member functions that can be used to query the data type. A series
/// of constant expressions have been defined that should be used when
/// specifying a data type or testing for specific data types:
/// * `dip::DT_BIN`
/// * `dip::DT_UINT8`
/// * `dip::DT_UINT16`
/// * `dip::DT_UINT32`
/// * `dip::DT_SINT8`
/// * `dip::DT_SINT16`
/// * `dip::DT_SINT32`
/// * `dip::DT_SFLOAT`
/// * `dip::DT_DFLOAT`
/// * `dip::DT_SCOMPLEX`
/// * `dip::DT_DCOMPLEX`
///
/// It is possible to call DataType member functions on these constants:
///
/// ```cpp
///     dip::DT_BIN.SizeOf();
/// ```
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

   constexpr explicit DataType( bin ) : dt( DT::BIN ) {}
   constexpr explicit DataType( uint8 ) : dt( DT::UINT8 ) {}
   constexpr explicit DataType( sint8 ) : dt( DT::SINT8 ) {}
   constexpr explicit DataType( uint16 ) : dt( DT::UINT16 ) {}
   constexpr explicit DataType( sint16 ) : dt( DT::SINT16 ) {}
   constexpr explicit DataType( uint32 ) : dt( DT::UINT32 ) {}
   constexpr explicit DataType( sint32 ) : dt( DT::SINT32 ) {}
   constexpr explicit DataType( sfloat ) : dt( DT::SFLOAT ) {}
   constexpr explicit DataType( dfloat ) : dt( DT::DFLOAT ) {}
   constexpr explicit DataType( scomplex ) : dt( DT::SCOMPLEX ) {}
   constexpr explicit DataType( dcomplex ) : dt( DT::DCOMPLEX ) {}

   /// \brief Swaps the values of `this` and `other`
   void swap( DataType& other ) {
      using std::swap;
      swap( dt, other.dt );
   }

   /// \brief DataType objects implicitly convert to the enumeration integer.
   constexpr operator int() const { return static_cast< int >( dt ); }   // This one allows the use of DataType in a switch() statement

   /// \brief DataType objects can be compared.
   bool operator==( DataType other ) const { return dt == other.dt; }

   /// \brief Returns a C-style string constant with a representation of the data type name.
   char const* Name() const {
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

   /// \brief Returns the size in bytes of the data type.
   dip::uint SizeOf() const {
      switch( dt ) {
         case DT::BIN:      return sizeof( dip::bin );
         case DT::UINT8:    return sizeof( dip::uint8 );
         case DT::SINT8:    return sizeof( dip::sint8 );
         case DT::UINT16:   return sizeof( dip::uint16 );
         case DT::SINT16:   return sizeof( dip::sint16 );
         case DT::UINT32:   return sizeof( dip::uint32 );
         case DT::SINT32:   return sizeof( dip::sint32 );
         case DT::SFLOAT:   return sizeof( dip::sfloat );
         case DT::DFLOAT:   return sizeof( dip::dfloat );
         case DT::SCOMPLEX: return sizeof( dip::scomplex );
         case DT::DCOMPLEX: return sizeof( dip::dcomplex );
      };
   }

   /// \brief Returns `true` if the data type is binary (equal to `dip::DT_BIN`).
   bool IsBinary() const {
      return dt == DT::BIN;
   }

   /// \brief Returns `true` if the data type is an unsigned integer type.
   bool IsUInt() const {
      switch( dt ) {
         case DT::UINT8:
         case DT::UINT16:
         case DT::UINT32:
            return true;
         default:
            return false;
      };
   }

   /// \brief Returns `true` if the data type is a signed integer type.
   bool IsSInt() const {
      switch( dt ) {
         case DT::SINT8:
         case DT::SINT16:
         case DT::SINT32:
            return true;
         default:
            return false;
      };
   }

   /// \brief Returns `true` if the data type is an integer type.
   bool IsInteger() const {
      return IsUInt() || IsSInt();
   }

   /// \brief Returns `true` if the data type is a floating point type.
   bool IsFloat() const {
      switch( dt ) {
         case DT::SFLOAT:
         case DT::DFLOAT:
            return true;
         default:
            return false;
      };
   }

   /// \brief Returns `true` if the data type is real (floating point or integer).
   bool IsReal() const {
      return IsInteger() || IsFloat();
   }

   /// vReturns `true` if the data type is complex.
   bool IsComplex() const {
      switch( dt ) {
         case DT::SCOMPLEX:
         case DT::DCOMPLEX:
            return true;
         default:
            return false;
      };
   }

   /// \brief Returns `true` if the data type is an unsigned type (same as `dip::DataType::IsUInt`).
   bool IsUnsigned() const {
      return IsUInt();
   }

   /// \brief Returns `true` if the data type is a signed type (signed integer, floating point or complex)
   bool IsSigned() const {
      return IsSInt() || IsFloat() || IsComplex();
   }

   /// \class dip::DataType::Classes
   /// \brief Specifies a collection of data types.
   ///
   /// Valid values are:
   ///
   /// Classes constant | Definition
   /// ---------------- | ----------
   /// Class_Bin        | DT_BIN
   /// Class_UInt8      | DT_UINT8
   /// Class_SInt8      | DT_SINT8
   /// Class_UInt16     | DT_UINT16
   /// Class_SInt16     | DT_SINT16
   /// Class_UInt32     | DT_UINT32
   /// Class_SInt32     | DT_SINT32
   /// Class_SFloat     | DT_SFLOAT
   /// Class_DFloat     | DT_DFLOAT
   /// Class_SComplex   | DT_SCOMPLEX
   /// Class_DComplex   | DT_DCOMPLEX
   /// Class_Binary     | Class_Bin;
   /// Class_UInt       | Class_UInt8 + Class_UInt16 + Class_UInt32;
   /// Class_SInt       | Class_SInt8 + Class_SInt16 + Class_SInt32;
   /// Class_Integer    | Class_UInt + Class_SInt;
   /// Class_Float      | Class_SFloat + Class_DFloat;
   /// Class_Real       | Class_Integer + Class_Float;
   /// Class_Complex    | Class_SComplex + Class_DComplex;
   /// Class_Unsigned   | Class_UInt;
   /// Class_Signed     | Class_SInt + Class_Float + Class_Complex;
   /// Class_Any        | Class_Binary + Class_Real + Class_Complex;
   ///
   /// Note that you can add these constants together, for example `dip::Class_Bin + dip::Class_UInt`.
   DIP_DECLARE_OPTIONS( Classes );
   static DIP_DEFINE_OPTION( Classes, Class_Bin, static_cast<dip::uint>( DT::BIN ) );
   static DIP_DEFINE_OPTION( Classes, Class_UInt8, static_cast<dip::uint>( DT::UINT8 ) );
   static DIP_DEFINE_OPTION( Classes, Class_SInt8, static_cast<dip::uint>( DT::SINT8 ) );
   static DIP_DEFINE_OPTION( Classes, Class_UInt16, static_cast<dip::uint>( DT::UINT16 ) );
   static DIP_DEFINE_OPTION( Classes, Class_SInt16, static_cast<dip::uint>( DT::SINT16 ) );
   static DIP_DEFINE_OPTION( Classes, Class_UInt32, static_cast<dip::uint>( DT::UINT32 ) );
   static DIP_DEFINE_OPTION( Classes, Class_SInt32, static_cast<dip::uint>( DT::SINT32 ) );
   static DIP_DEFINE_OPTION( Classes, Class_SFloat, static_cast<dip::uint>( DT::SFLOAT ) );
   static DIP_DEFINE_OPTION( Classes, Class_DFloat, static_cast<dip::uint>( DT::DFLOAT ) );
   static DIP_DEFINE_OPTION( Classes, Class_SComplex, static_cast<dip::uint>( DT::SCOMPLEX ) );
   static DIP_DEFINE_OPTION( Classes, Class_DComplex, static_cast<dip::uint>( DT::DCOMPLEX ) );
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

   /// \brief Implicit conversion to `dip::DataType::Classes` options class.
   operator Classes() const { return { static_cast<dip::uint>( dt ) }; }

   //
   // Functions to suggest an output data type for all types of filters and operators
   //


   /// \brief Returns a suitable floating-point type that can hold the samples of `type`.
   static DataType SuggestFloat( DataType type );

   /// \brief Returns a suitable complex type that can hold the samples of `type`.
   static DataType SuggestComplex( DataType type );

   /// \brief Returns a suitable floating-point or complex type that can hold the samples of `type`.
   static DataType SuggestFlex( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type that can hold the samples of `type`.
   static DataType SuggestFlexBin( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type that can hold the result of an arithmetic computation performed with the two datatypes.
   static DataType SuggestArithmetic( DataType type1, DataType type2 );

   /// \brief Returns a suitable type that can hold any samples of the two datatypes.
   static DataType SuggestDiadicOperation( DataType type1, DataType type2 );

};

inline void swap( DataType& v1, DataType& v2 ) {
   v1.swap( v2 );
}

/// \brief An array to hold data types
using DataTypeArray = std::vector< DataType >;

//
// Constants that people will use where a DataType is needed
//

constexpr DataType DT_BIN{ DataType::DT::BIN };
constexpr DataType DT_UINT8{ DataType::DT::UINT8 };
constexpr DataType DT_SINT8{ DataType::DT::SINT8 };
constexpr DataType DT_UINT16{ DataType::DT::UINT16 };
constexpr DataType DT_SINT16{ DataType::DT::SINT16 };
constexpr DataType DT_UINT32{ DataType::DT::UINT32 };
constexpr DataType DT_SINT32{ DataType::DT::SINT32 };
constexpr DataType DT_SFLOAT{ DataType::DT::SFLOAT };
constexpr DataType DT_DFLOAT{ DataType::DT::DFLOAT };
constexpr DataType DT_SCOMPLEX{ DataType::DT::SCOMPLEX };
constexpr DataType DT_DCOMPLEX{ DataType::DT::DCOMPLEX };

/// \}

} // namespace dip

#endif // DIP_DATATYPE_H
