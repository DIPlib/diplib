/*
 * DIPlib 3.0
 * This file contains definitions for the DataType class and support functions.
 *
 * (c)2014-2017, Cris Luengo.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_DATATYPE_H
#define DIP_DATATYPE_H

#include "diplib/library/types.h"


/// \file
/// \brief The `dip::DataType` class and support functions. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \defgroup types Pixel data types
/// \brief Types used for image samples (pixels), and related support functionality
///
/// The following table lists all supported sample data types, together with `dip::DataType` constants and type
/// groups (see `dip::DataType::Classes`) that they belong to.
///
/// C++ type        | Constant                 | Type groups
/// --------------- | ------------------------ | -------------------------------------------------
/// `dip::bin`      | `dip::DT_BIN`            | `Binary`, `IntOrBin`, `FlexBin`, `Unsigned`
/// `dip::uint8`    | `dip::DT_UINT8`          | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned`
/// `dip::uint16`   | `dip::DT_UINT16`         | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned`
/// `dip::uint32`   | `dip::DT_UINT32`         | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned`
/// `dip::sint8`    | `dip::DT_SINT8`          | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`
/// `dip::sint16`   | `dip::DT_SINT16`         | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`
/// `dip::sint32`   | `dip::DT_SINT32`         | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`
/// `dip::sfloat`   | `dip::DT_SFLOAT`         | `Float`, `Real`, `Flex`, `FlexBin`, `Signed`
/// `dip::dfloat`   | `dip::DT_DFLOAT`         | `Float`, `Real`, `Flex`, `FlexBin`, `Signed`
/// `dip::scomplex` | `dip::DT_SCOMPLEX`       | `Complex`, `Flex`, `FlexBin`, `Signed`
/// `dip::dcomplex` | `dip::DT_DCOMPLEX`       | `Complex`, `Flex`, `FlexBin`, `Signed`
///
/// \ingroup infrastructure
/// \{


//
// The DataType class.
//

/// \brief `%DataType` objects are used to indicate what the data type of
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
/// It is possible to call `%DataType` member functions on these constants:
///
/// ```cpp
///     dip::DT_BIN.SizeOf();
/// ```
struct DIP_NO_EXPORT DataType {

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

   // This is to get the template below to not compile -- please ignore
   template< typename T >
   struct assert_false : std::false_type {};

   /// \brief Get the data type value of any expression, as long as that expression is of one of the known data types
   template< typename T >
   constexpr explicit DataType( T ) {
      // This constructor is not valid. Only the specializations towards the bottom of this file are.
      static_assert( assert_false< T >::value, "You need to cast your constant to one of the known data types" );
   }

   /// \brief A string can be cast to a data type. The recognized names are identical to the enumerator names in `dip::DataType::DT`.
   explicit DataType( String name ) {
      if( name == "BIN"      ) { dt = DT::BIN;      } else
      if( name == "UINT8"    ) { dt = DT::UINT8;    } else
      if( name == "SINT8"    ) { dt = DT::SINT8;    } else
      if( name == "UINT16"   ) { dt = DT::UINT16;   } else
      if( name == "SINT16"   ) { dt = DT::SINT16;   } else
      if( name == "UINT32"   ) { dt = DT::UINT32;   } else
      if( name == "SINT32"   ) { dt = DT::SINT32;   } else
      if( name == "SFLOAT"   ) { dt = DT::SFLOAT;   } else
      if( name == "DFLOAT"   ) { dt = DT::DFLOAT;   } else
      if( name == "SCOMPLEX" ) { dt = DT::SCOMPLEX; } else
      if( name == "DCOMPLEX" ) { dt = DT::DCOMPLEX; }
      else {
         DIP_THROW( "Illegal data type name: " + name );
      }
   }

   /// \brief Swaps the values of `this` and `other`
   void swap( DataType& other ) {
      using std::swap;
      swap( dt, other.dt );
   }

   /// \brief `%DataType` objects implicitly convert to the enumeration integer.
   constexpr operator int() const { return static_cast< int >( dt ); }   // This one allows the use of DataType in a switch() statement

   /// \brief `%DataType` objects can be compared.
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
      DIP_THROW( "Unknown data type" ); // This should never happen, but GCC complains.
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
      DIP_THROW( "Unknown data type" ); // This should never happen, but GCC complains.
   }

   /// \brief Returns the real data type corresponding to a complex data type
   DataType Real() {
      switch( dt ) {
         case DT::SCOMPLEX:
            return DT::SFLOAT;
         case DT::DCOMPLEX:
            return DT::DFLOAT;
         default:
            return dt;
      };
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
   /// Class_UInt       | Class_UInt8 + Class_UInt16 + Class_UInt32
   /// Class_SInt       | Class_SInt8 + Class_SInt16 + Class_SInt32
   /// Class_Integer    | Class_UInt + Class_SInt
   /// Class_IntOrBin   | Class_Integer + Class_Binary
   /// Class_Float      | Class_SFloat + Class_DFloat
   /// Class_Complex    | Class_SComplex + Class_DComplex
   /// Class_Flex       | Class_Float + Class_Complex
   /// Class_FlexBin    | Class_Flex + Class_Binary
   /// Class_Unsigned   | Class_Binary + Class_UInt
   /// Class_Signed     | Class_SInt + Class_Float + Class_Complex
   /// Class_Real       | Class_Integer + Class_Float
   /// Class_SignedReal | Class_SInt + Class_Float
   /// Class_NonBinary  | Class_Real + Class_Complex
   /// Class_NonComplex | Class_Binary + Class_Real
   /// Class_All        | Class_Binary + Class_Real + Class_Complex
   ///
   /// Note that you can add these constants together, for example `dip::DataType::Class_UInt8 + dip::DataType::Class_UInt16`.
   ///
   /// It is possible to see if an image is of a type within a collection using the equality operator between a
   /// `%dip::DataType::Classes` on the left hand side and a `dip::DataType` on the right hand side:
   /// ```cpp
   ///     if( dip::DataType::Class_Flex == image.DataType() ) { ... }
   /// ```
   /// This is equivalent to using one of the test functions, if defined for the specific group:
   /// ```cpp
   ///     if( image.DataType().IsFlex() ) { ... }
   /// ```
   ///
   /// The following combination of classes cover all data types, and are non-intersecting:
   /// - `Class_Unsigned` and `Class_Signed`
   /// - `Class_Complex` and `Class_NonComplex`
   /// - `Class_Binary` and `Class_NonBinary`
   /// - `Class_FlexBin` and `Class_Integer`
   /// - `Class_Flex` and `Class_IntOrBin`
   /// - `Class_Binary`, `Class_Real` and `Class_Complex`
   /// - `Class_Binary`, `Class_Integer`, `Class_Float` and `Class_Complex`
   DIP_DECLARE_OPTIONS( Classes );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Bin, static_cast< dip::uint >( DT::BIN ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_UInt8, static_cast< dip::uint >( DT::UINT8 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SInt8, static_cast< dip::uint >( DT::SINT8 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_UInt16, static_cast< dip::uint >( DT::UINT16 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SInt16, static_cast< dip::uint >( DT::SINT16 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_UInt32, static_cast< dip::uint >( DT::UINT32 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SInt32, static_cast< dip::uint >( DT::SINT32 ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SFloat, static_cast< dip::uint >( DT::SFLOAT ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_DFloat, static_cast< dip::uint >( DT::DFLOAT ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SComplex, static_cast< dip::uint >( DT::SCOMPLEX ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_DComplex, static_cast< dip::uint >( DT::DCOMPLEX ) );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Binary, Class_Bin );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_UInt, Class_UInt8 + Class_UInt16 + Class_UInt32 );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SInt, Class_SInt8 + Class_SInt16 + Class_SInt32 );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Integer, Class_UInt + Class_SInt );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_IntOrBin, Class_Integer + Class_Binary );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Float, Class_SFloat + Class_DFloat );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Complex, Class_SComplex + Class_DComplex );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Flex, Class_Float + Class_Complex );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_FlexBin, Class_Flex + Class_Binary );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Unsigned, Class_Binary + Class_UInt );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Signed, Class_SInt + Class_Float + Class_Complex );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_Real, Class_Integer + Class_Float );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_SignedReal, Class_SInt + Class_Float );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_NonBinary, Class_Real + Class_Complex );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_NonComplex, Class_Binary + Class_Real );
   DIP_EXPORT static DIP_DEFINE_OPTION( Classes, Class_All, Class_Binary + Class_Real + Class_Complex ); // == Class_Unsigned + Class_Signed

   /// \brief Implicit conversion to `dip::DataType::Classes` options class.
   constexpr operator Classes() const { return { static_cast< dip::uint >( dt ) }; }

   //
   // Functions to query the data type class
   //

   /// \brief Returns `true` if the data type is binary.
   constexpr bool IsBinary() const {
      return dt == DT::BIN;
   }

   /// \brief Returns `true` if the data type is an unsigned integer type.
   constexpr bool IsUInt() const {
      return Class_UInt == *this;
   }

   /// \brief Returns `true` if the data type is a signed integer type.
   constexpr bool IsSInt() const {
      return Class_SInt == *this;
   }

   /// \brief Returns `true` if the data type is an integer type.
   constexpr bool IsInteger() const {
      return Class_Integer == *this;
   }

   /// \brief Returns `true` if the data type is a floating point type.
   constexpr bool IsFloat() const {
      return Class_Float == *this;
   }

   /// \brief Returns `true` if the data type is real (floating point or integer).
   constexpr bool IsReal() const {
      return Class_Real == *this;
   }

   /// \brief Returns `true` if the data type is one of the "flex" types (floating point or complex).
   constexpr bool IsFlex() const {
      return Class_Flex == *this;
   }

   /// \brief Returns `true` if the data type is floating point, complex or binary.
   constexpr bool IsFlexBin() const {
      return Class_FlexBin == *this;
   }

   /// \brief Returns `true` if the data type is complex.
   constexpr bool IsComplex() const {
      return Class_Complex == *this;
   }

   /// \brief Returns `true` if the data type is an unsigned type (binary or unsigned integer).
   constexpr bool IsUnsigned() const {
      return Class_Unsigned == *this;
   }

   /// \brief Returns `true` if the data type is a signed type (signed integer, floating point or complex)
   constexpr bool IsSigned() const {
      return Class_Signed == *this;
   }

   //
   // Functions to suggest an output data type for all types of filters and operators
   //

   /// \brief Returns an integer type that is most suitable to hold samples of `type`.
   DIP_EXPORT static DataType SuggestInteger( DataType type );

   /// \brief Returns an integer type that is most suitable to hold samples of `type`.
   DIP_EXPORT static DataType SuggestSigned( DataType type );

   /// \brief Returns a suitable floating-point type that can hold the samples of `type`.
   DIP_EXPORT static DataType SuggestFloat( DataType type );

   /// \brief Returns a suitable complex type that can hold the samples of `type`.
   DIP_EXPORT static DataType SuggestComplex( DataType type );

   /// \brief Returns a suitable floating-point or complex type that can hold the samples of `type`.
   DIP_EXPORT static DataType SuggestFlex( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type that can hold the samples of `type`.
   DIP_EXPORT static DataType SuggestFlexBin( DataType type );

   /// \brief Returns a suitable type that can hold samples of type `abs(type)`.
   DIP_EXPORT static DataType SuggestAbs( DataType type );

   /// \brief Returns a suitable real type that can hold the samples of `type`.
   DIP_EXPORT static DataType SuggestReal( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type (FlexBin) that can hold the result of an arithmetic computation performed with the two data types.
   DIP_EXPORT static DataType SuggestArithmetic( DataType type1, DataType type2 );

   /// \brief Returns a suitable type that can hold any samples of the two data types.
   DIP_EXPORT static DataType SuggestDyadicOperation( DataType type1, DataType type2 );

};

inline void swap( DataType& v1, DataType& v2 ) {
   v1.swap( v2 );
}

// Here are the specializations for the templated constructor
template<> constexpr DataType::DataType( bin      ) : dt( DT::BIN      ) {}
template<> constexpr DataType::DataType( uint8    ) : dt( DT::UINT8    ) {}
template<> constexpr DataType::DataType( sint8    ) : dt( DT::SINT8    ) {}
template<> constexpr DataType::DataType( uint16   ) : dt( DT::UINT16   ) {}
template<> constexpr DataType::DataType( sint16   ) : dt( DT::SINT16   ) {}
template<> constexpr DataType::DataType( uint32   ) : dt( DT::UINT32   ) {}
template<> constexpr DataType::DataType( sint32   ) : dt( DT::SINT32   ) {}
template<> constexpr DataType::DataType( sfloat   ) : dt( DT::SFLOAT   ) {}
template<> constexpr DataType::DataType( dfloat   ) : dt( DT::DFLOAT   ) {}
template<> constexpr DataType::DataType( scomplex ) : dt( DT::SCOMPLEX ) {}
template<> constexpr DataType::DataType( dcomplex ) : dt( DT::DCOMPLEX ) {}

/// \brief An array to hold data types
using DataTypeArray = DimensionArray< DataType >;

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

constexpr DataType DT_LABEL = DT_UINT32; ///< Type currently used for all labeled images, see `dip::LabelType`.

/// \}

} // namespace dip

#endif // DIP_DATATYPE_H
