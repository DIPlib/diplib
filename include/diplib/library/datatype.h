/*
 * (c)2014-2019, Cris Luengo.
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
// IWYU pragma: private, include "diplib.h"


#ifndef DIP_DATATYPE_H
#define DIP_DATATYPE_H

#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

#include "diplib/library/export.h"
#include "diplib/library/error.h"
#include "diplib/library/dimension_array.h"
#include "diplib/library/types.h"


/// \file
/// \brief The \ref dip::DataType class and support functions. This file is always included through \ref "diplib.h".
/// See \ref pixeltypes.


namespace dip {


/// \group pixeltypes Pixel data types
/// \ingroup infrastructure
/// \brief Types used for image samples (pixels), and related support functionality
///
/// The following table lists all supported sample data types, together with \ref dip::DataType constants
/// and type groups (see \ref dip::DataType::Classes) that they belong to.
///
/// C++ type           | Constant               | String       | Type groups                                       | Size (bytes)
/// ------------------ | ---------------------- | ------------ | ------------------------------------------------- | ------------
/// \ref dip::bin      | \ref dip::DT_BIN       | `"BIN"`      | `Binary`, `IntOrBin`, `FlexBin`, `Unsigned`       | 1
/// \ref dip::uint8    | \ref dip::DT_UINT8     | `"UINT8"`    | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned` | 1
/// \ref dip::sint8    | \ref dip::DT_SINT8     | `"SINT8"`    | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`   | 1
/// \ref dip::uint16   | \ref dip::DT_UINT16    | `"UINT16"`   | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned` | 2
/// \ref dip::sint16   | \ref dip::DT_SINT16    | `"SINT16"`   | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`   | 2
/// \ref dip::uint32   | \ref dip::DT_UINT32    | `"UINT32"`   | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned` | 4
/// \ref dip::sint32   | \ref dip::DT_SINT32    | `"SINT32"`   | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`   | 4
/// \ref dip::uint64   | \ref dip::DT_UINT64    | `"UINT64"`   | `UInt`, `Integer`, `IntOrBin`, `Real`, `Unsigned` | 8
/// \ref dip::sint64   | \ref dip::DT_SINT64    | `"SINT64"`   | `SInt`, `Integer`, `IntOrBin`, `Real`, `Signed`   | 8
/// \ref dip::sfloat   | \ref dip::DT_SFLOAT    | `"SFLOAT"`   | `Float`, `Real`, `Flex`, `FlexBin`, `Signed`      | 4
/// \ref dip::dfloat   | \ref dip::DT_DFLOAT    | `"DFLOAT"`   | `Float`, `Real`, `Flex`, `FlexBin`, `Signed`      | 8
/// \ref dip::scomplex | \ref dip::DT_SCOMPLEX  | `"SCOMPLEX"` | `Complex`, `Flex`, `FlexBin`, `Signed`            | 8 (4 x2)
/// \ref dip::dcomplex | \ref dip::DT_DCOMPLEX  | `"DCOMPLEX"` | `Complex`, `Flex`, `FlexBin`, `Signed`            | 16 (8 x2)
///
/// Note that some functions require specific data types for their input images, and will throw an
/// exception if the data type doesn't match. However, type restrictions typically are meaningful,
/// in the sense that any data type that makes sense for the given operation should be accepted,
/// and is rejected only when an operation is not possible on a given type. For example, it is not
/// possible to find the maximum of a set of complex values, therefore `dip::Maximum` does not accept
/// complex-valued images as input.
///
/// The output images are typically set to a suitable type, which is selected using functions
/// such as \ref dip::DataType::SuggestComplex and \ref dip::DataType::SuggestFlex. The table below lists
/// these functions and their output. See also \ref dip::DataType::SuggestArithmetic and
/// \ref dip::DataType::SuggestDyadicOperation, which help select a suitable data type when combining
/// two images. To manually select an output data type, see \ref protect.
///
/// <div markdown="1" class="m-smaller-font m-block m-flat">
///
/// Input data type | \ref dip::DataType::SuggestInteger "SuggestInteger" | \ref dip::DataType::SuggestSigned "SuggestSigned" | \ref dip::DataType::SuggestAbs "SuggestAbs" | \ref dip::DataType::SuggestFloat "SuggestFloat" | \ref dip::DataType::SuggestDouble "SuggestDouble" | \ref dip::DataType::SuggestReal "SuggestReal" | \ref dip::DataType::SuggestComplex "SuggestComplex" | \ref dip::DataType::SuggestFlex "SuggestFlex" | \ref dip::DataType::SuggestFlexBin "SuggestFlexBin"
/// ------------- | ----------- | ------------- | ----------- | ----------- | ------------- | ----------- | ------------- | ------------- | -------------
/// `DT_BIN`      | `DT_UINT8`  | `DT_SINT8`    | `DT_BIN`    | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_UINT8`  | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_BIN`
/// `DT_UINT8`    | `DT_UINT8`  | `DT_SINT16`   | `DT_UINT8`  | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_UINT8`  | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_SFLOAT`
/// `DT_SINT8`    | `DT_SINT8`  | `DT_SINT8`    | `DT_UINT8`  | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_SINT8`  | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_SFLOAT`
/// `DT_UINT16`   | `DT_UINT16` | `DT_SINT32`   | `DT_UINT16` | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_UINT16` | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_SFLOAT`
/// `DT_SINT16`   | `DT_SINT16` | `DT_SINT16`   | `DT_UINT16` | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_SINT16` | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_SFLOAT`
/// `DT_UINT32`   | `DT_UINT32` | `DT_SINT64`   | `DT_UINT32` | `DT_DFLOAT` | `DT_DFLOAT`   | `DT_UINT32` | `DT_DCOMPLEX` | `DT_DFLOAT`   | `DT_DFLOAT`
/// `DT_SINT32`   | `DT_SINT32` | `DT_SINT32`   | `DT_UINT32` | `DT_DFLOAT` | `DT_DFLOAT`   | `DT_SINT32` | `DT_DCOMPLEX` | `DT_DFLOAT`   | `DT_DFLOAT`
/// `DT_UINT64`   | `DT_UINT64` | `DT_SINT64`   | `DT_UINT64` | `DT_DFLOAT` | `DT_DFLOAT`   | `DT_UINT64` | `DT_DCOMPLEX` | `DT_DFLOAT`   | `DT_DFLOAT`
/// `DT_SINT64`   | `DT_SINT64` | `DT_SINT64`   | `DT_UINT64` | `DT_DFLOAT` | `DT_DFLOAT`   | `DT_SINT64` | `DT_DCOMPLEX` | `DT_DFLOAT`   | `DT_DFLOAT`
/// `DT_SFLOAT`   | `DT_SINT32` | `DT_SFLOAT`   | `DT_SFLOAT` | `DT_SFLOAT` | `DT_DFLOAT`   | `DT_SFLOAT` | `DT_SCOMPLEX` | `DT_SFLOAT`   | `DT_SFLOAT`
/// `DT_DFLOAT`   | `DT_SINT64` | `DT_DFLOAT`   | `DT_DFLOAT` | `DT_DFLOAT` | `DT_DFLOAT`   | `DT_DFLOAT` | `DT_DCOMPLEX` | `DT_DFLOAT`   | `DT_DFLOAT`
/// `DT_SCOMPLEX` | `DT_SINT32` | `DT_SCOMPLEX` | `DT_SFLOAT` | `DT_SFLOAT` | `DT_DCOMPLEX` | `DT_SFLOAT` | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_SCOMPLEX`
/// `DT_DCOMPLEX` | `DT_SINT64` | `DT_DCOMPLEX` | `DT_DFLOAT` | `DT_DFLOAT` | `DT_DCOMPLEX` | `DT_DFLOAT` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX`
///
/// </div>
///
/// \section type_casting Type casting
///
/// \ref dip::clamp_cast is an operator that returns the input value cast to a different
/// type, clamped to the range of values representable by that output type. This
/// is also often referred to as saturated cast. Most *DIPlib* functions take care of properly
/// clamping values when casting pixel values. This typically is more intuitive and useful
/// when processing images than the default C/C++ overflow behavior, which corresponds to
/// modular arithmetic for integer values.
///
/// When casting from complex to non-complex, the absolute value of the complex number is taken.
/// When casting from a floating-point number to an integer, the decimals are truncated, as typically
/// done in C++.
///
/// \ref dip::clamp_cast is defined as a series of overloaded template functions with specializations.
/// The input argument type is used in overload resolution, the output type is the template
/// parameter, and should be specified between angled brackets after the function name,
/// much like the standard `static_cast` and similar:
///
/// ```cpp
/// uint8 u = dip::clamp_cast< dip::uint8 >( -54.6 );
/// ```
///
/// \section saturated_arithmetic Saturated arithmetic
///
/// `dip::saturated_XXX` are templated functions for saturated arithmetic. Most *DIPlib* functions take care
/// of properly clamping the result of operations on pixels by using these functions to perform arithmetic.
/// For example,
///
/// ```cpp
/// 10u - 20u == 4294967286u;
/// dip::saturated_sub(10u, 20u) == 0u;
/// ```
///
/// Saturated arithmetic is made available by including \ref "diplib/saturated_arithmetic.h".
///
/// \addtogroup

//
// The DataType class.
//

/// \brief `DataType` objects are used to indicate what the data type of
/// an image is.
///
/// It is a simple enumeration type, but with some added member functions that can be used
/// to query the data type. A series of constant expressions (`dip::DT_XXX`) have been defined
/// that should be used when specifying a data type, there is never a need to call the
/// constructors to this class explicitly. It is possible to call `DataType` member functions
/// on these constants:
///
/// ```cpp
/// dip::DT_BIN.SizeOf();
/// ```
///
/// See \ref pixeltypes for more information about image sample data types.
struct DIP_NO_EXPORT DataType {

   // Not documented because the user doesn't need these.
   enum class DT : uint8 {
         BIN,
         UINT8,
         SINT8,
         UINT16,
         SINT16,
         UINT32,
         SINT32,
         UINT64,
         SINT64,
         SFLOAT,
         DFLOAT,
         SCOMPLEX,
         DCOMPLEX,
   } dt;

   struct DTString {
      // Undocumented, used to prevent typos in literal strings.
      constexpr static char const* BIN = "BIN";
      constexpr static char const* UINT8 = "UINT8";
      constexpr static char const* SINT8 = "SINT8";
      constexpr static char const* UINT16 = "UINT16";
      constexpr static char const* SINT16 = "SINT16";
      constexpr static char const* UINT32 = "UINT32";
      constexpr static char const* SINT32 = "SINT32";
      constexpr static char const* UINT64 = "UINT64";
      constexpr static char const* SINT64 = "SINT64";
      constexpr static char const* SFLOAT = "SFLOAT";
      constexpr static char const* DFLOAT = "DFLOAT";
      constexpr static char const* SCOMPLEX = "SCOMPLEX";
      constexpr static char const* DCOMPLEX = "DCOMPLEX";
   };

   /// \brief The default data type is single-precision Float (\ref dip::DT_SFLOAT).
   constexpr DataType() : dt( DT::SFLOAT ) {}
   constexpr DataType( DT _dt ) : dt( _dt ) {}

   /// \brief Get the data type value of any expression, as long as that expression is of one of the known data types
   template< typename T >
   constexpr explicit DataType( T /*v*/ );
   constexpr explicit DataType( unsigned long long /*v*/ ); // On some platforms, a long long is not the same as uint64_t!
   constexpr explicit DataType( long long /*v*/ );
   constexpr explicit DataType( unsigned long /*v*/ ); // On some platforms, a long is not the same as uint32_t nor uint64_t!
   constexpr explicit DataType( long /*v*/ );

   /// \brief A string can be cast to a data type. See \ref pixeltypes for recognized strings.
   explicit DataType( String const& name ) {
      if( name == DTString::BIN      ) { dt = DT::BIN;      } else
      if( name == DTString::UINT8    ) { dt = DT::UINT8;    } else
      if( name == DTString::SINT8    ) { dt = DT::SINT8;    } else
      if( name == DTString::UINT16   ) { dt = DT::UINT16;   } else
      if( name == DTString::SINT16   ) { dt = DT::SINT16;   } else
      if( name == DTString::UINT32   ) { dt = DT::UINT32;   } else
      if( name == DTString::SINT32   ) { dt = DT::SINT32;   } else
      if( name == DTString::UINT64   ) { dt = DT::UINT64;   } else
      if( name == DTString::SINT64   ) { dt = DT::SINT64;   } else
      if( name == DTString::SFLOAT   ) { dt = DT::SFLOAT;   } else
      if( name == DTString::DFLOAT   ) { dt = DT::DFLOAT;   } else
      if( name == DTString::SCOMPLEX ) { dt = DT::SCOMPLEX; } else
      if( name == DTString::DCOMPLEX ) { dt = DT::DCOMPLEX; }
      else {
         DIP_THROW( "Illegal data type name: " + name );
      }
   }

   /// \brief Swaps the values of `this` and `other`
   void swap( DataType& other ) noexcept {
      using std::swap;
      swap( dt, other.dt );
   }

   /// \brief `DataType` objects implicitly convert to the enumeration integer.
   constexpr operator int() const { return static_cast< int >( dt ); }   // This one allows the use of DataType in a switch() statement

   /// \brief `DataType` objects can be compared.
   bool operator==( DataType other ) const { return dt == other.dt; }

   /// \brief Returns a C-style string constant with a representation of the data type name. See \ref pixeltypes for returned strings.
   char const* Name() const {
      switch( dt ) {
         case DT::BIN:      return DTString::BIN;
         case DT::UINT8:    return DTString::UINT8;
         case DT::SINT8:    return DTString::SINT8;
         case DT::UINT16:   return DTString::UINT16;
         case DT::SINT16:   return DTString::SINT16;
         case DT::UINT32:   return DTString::UINT32;
         case DT::SINT32:   return DTString::SINT32;
         case DT::UINT64:   return DTString::UINT64;
         case DT::SINT64:   return DTString::SINT64;
         case DT::SFLOAT:   return DTString::SFLOAT;
         case DT::DFLOAT:   return DTString::DFLOAT;
         case DT::SCOMPLEX: return DTString::SCOMPLEX;
         case DT::DCOMPLEX: return DTString::DCOMPLEX;
      }
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
         case DT::UINT64:   return sizeof( dip::uint64 );
         case DT::SINT64:   return sizeof( dip::sint64 );
         case DT::SFLOAT:   return sizeof( dip::sfloat );
         case DT::DFLOAT:   return sizeof( dip::dfloat );
         case DT::SCOMPLEX: return sizeof( dip::scomplex );
         case DT::DCOMPLEX: return sizeof( dip::dcomplex );
      }
      DIP_THROW( "Unknown data type" ); // This should never happen, but GCC complains.
   }

   /// \brief Returns true if the integer `value` is within the range representable by the data type.
   bool IsInRange( dip::sint value ) const {
      switch( dt ) {
         case DT::BIN:
            return ( value >= 0 ) && ( value <= 1 );
         case DT::UINT8:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< uint8 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< uint8 >::max() ));
         case DT::SINT8:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< sint8 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< sint8 >::max() ));
         case DT::UINT16:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< uint16 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< uint16 >::max() ));
         case DT::SINT16:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< sint16 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< sint16 >::max() ));
         case DT::UINT32:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< uint32 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< uint32 >::max() ));
         case DT::SINT32:
            return ( value >= static_cast< dip::sint >( std::numeric_limits< sint32 >::min() ))
                && ( value <= static_cast< dip::sint >( std::numeric_limits< sint32 >::max() ));
         case DT::UINT64:
            return ( value >= 0 );
         //case DT::SINT64, and floating-point types:
         default:
            return true;
      }
   }

   /// \brief Returns true if the integer `value` is within the range representable by the data type.
   bool IsInRange( dip::uint value ) const {
      switch( dt ) {
         case DT::BIN:
            return value <= 1;
         case DT::UINT8:
            return value <= static_cast< dip::uint >( std::numeric_limits< uint8 >::max() );
         case DT::SINT8:
            return value <= static_cast< dip::uint >( std::numeric_limits< sint8 >::max() );
         case DT::UINT16:
            return value <= static_cast< dip::uint >( std::numeric_limits< uint16 >::max() );
         case DT::SINT16:
            return value <= static_cast< dip::uint >( std::numeric_limits< sint16 >::max() );
         case DT::UINT32:
            return value <= static_cast< dip::uint >( std::numeric_limits< uint32 >::max() );
         case DT::SINT32:
            return value <= static_cast< dip::uint >( std::numeric_limits< sint32 >::max() );
         case DT::SINT64:
            return value <= static_cast< dip::uint >( std::numeric_limits< sint64 >::max() );
         //case DT::UINT64, and floating-point types:
         default:
            return true;
      }
   }

   /// \brief Returns the real data type corresponding to a complex data type
   DataType Real() const {
      switch( dt ) {
         case DT::SCOMPLEX:
            return DT::SFLOAT;
         case DT::DCOMPLEX:
            return DT::DFLOAT;
         default:
            return dt;
      }
   }

   /// \brief Specifies a collection of data types.
   ///
   /// Valid values are:
   ///
   /// <div markdown="1" class="m-smaller-font m-grid m-block m-flat">
   ///
   /// Classes constant   | `DT_BIN` | `DT_UINT8` | `DT_SINT8` | `DT_UINT16` | `DT_SINT16` | `DT_UINT32` | `DT_SINT32` | `DT_UINT64` | `DT_SINT64` | `DT_SFLOAT` | `DT_DFLOAT` | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// ------------------ |:--------:|:----------:|:----------:|:-----------:|:-----------:|:-----------:|:-----------:|:-----------:|:-----------:|:-----------:|:-----------:|:-------------:|:-------------:
   /// `Class_Bin`        | ✔        |            |            |             |             |             |             |             |             |             |             |               |
   /// `Class_UInt8`      |          | ✔          |            |             |             |             |             |             |             |             |             |               |
   /// `Class_SInt8`      |          |            | ✔          |             |             |             |             |             |             |             |             |               |
   /// `Class_UInt16`     |          |            |            | ✔           |             |             |             |             |             |             |             |               |
   /// `Class_SInt16`     |          |            |            |             | ✔           |             |             |             |             |             |             |               |
   /// `Class_UInt32`     |          |            |            |             |             | ✔           |             |             |             |             |             |               |
   /// `Class_SInt32`     |          |            |            |             |             |             | ✔           |             |             |             |             |               |
   /// `Class_UInt64`     |          |            |            |             |             |             |             | ✔           |             |             |             |               |
   /// `Class_SInt64`     |          |            |            |             |             |             |             |             | ✔           |             |             |               |
   /// `Class_SFloat`     |          |            |            |             |             |             |             |             |             | ✔           |             |               |
   /// `Class_DFloat`     |          |            |            |             |             |             |             |             |             |             | ✔           |               |
   /// `Class_SComplex`   |          |            |            |             |             |             |             |             |             |             |             | ✔             |
   /// `Class_DComplex`   |          |            |            |             |             |             |             |             |             |             |             |               | ✔
   /// `Class_Binary`     | ✔        |            |            |             |             |             |             |             |             |             |             |               |
   /// `Class_UInt`       |          | ✔          |            | ✔           |             | ✔           |             | ✔           |             |             |             |               |
   /// `Class_SInt`       |          |            | ✔          |             | ✔           |             | ✔           |             | ✔           |             |             |               |
   /// `Class_Integer`    |          | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           |             |             |               |
   /// `Class_IntOrBin`   | ✔        | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           |             |             |               |
   /// `Class_Float`      |          |            |            |             |             |             |             |             |             | ✔           | ✔           |               |
   /// `Class_Complex`    |          |            |            |             |             |             |             |             |             |             |             | ✔             | ✔
   /// `Class_Flex`       |          |            |            |             |             |             |             |             |             | ✔           | ✔           | ✔             | ✔
   /// `Class_FlexBin`    | ✔        |            |            |             |             |             |             |             |             | ✔           | ✔           | ✔             | ✔
   /// `Class_Unsigned`   | ✔        | ✔          |            | ✔           |             | ✔           |             | ✔           |             |             |             |               |
   /// `Class_Signed`     |          |            | ✔          |             | ✔           |             | ✔           |             | ✔           | ✔           | ✔           | ✔             | ✔
   /// `Class_Real`       |          | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           |               |
   /// `Class_SignedReal` |          |            | ✔          |             | ✔           |             | ✔           |             | ✔           | ✔           | ✔           |               |
   /// `Class_NonBinary`  |          | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔             | ✔
   /// `Class_NonComplex` | ✔        | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           |               |
   /// `Class_All`        | ✔        | ✔          | ✔          | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔           | ✔             | ✔
   ///
   /// </div>
   ///
   /// Note that you can add these constants together, for example `dip::DataType::Class_UInt8 + dip::DataType::Class_UInt16`.
   ///
   /// It is possible to see if an image is of a type within a collection using the `Contains` method of
   /// `dip::DataType::Classes` with a \ref dip::DataType as argument:
   ///
   /// ```cpp
   /// if( dip::DataType::Class_Flex.Contains( image.DataType() )) { ... }
   /// ```
   ///
   /// But more convenient is to use the \ref dip::DataType::IsA method:
   ///
   /// ```cpp
   /// if( image.DataType().IsA( dip::DataType::Class_Flex )) { ... }
   /// ```
   ///
   /// This is equivalent to using one of the test functions, if defined for the specific group:
   ///
   /// ```cpp
   /// if( image.DataType().IsFlex() ) { ... }
   /// ```
   ///
   /// The following combination of classes cover all data types, and are non-intersecting:
   ///
   /// - `Class_Unsigned` and `Class_Signed`
   /// - `Class_Complex` and `Class_NonComplex`
   /// - `Class_Binary` and `Class_NonBinary`
   /// - `Class_FlexBin` and `Class_Integer`
   /// - `Class_Flex` and `Class_IntOrBin`
   /// - `Class_Binary`, `Class_Real` and `Class_Complex`
   /// - `Class_Binary`, `Class_Integer`, `Class_Float` and `Class_Complex`
   using Classes = dip::detail::Options< DT >;
   DIP_EXPORT constexpr static Classes Class_Bin = DT::BIN;
   DIP_EXPORT constexpr static Classes Class_UInt8 = DT::UINT8;
   DIP_EXPORT constexpr static Classes Class_SInt8 = DT::SINT8;
   DIP_EXPORT constexpr static Classes Class_UInt16 = DT::UINT16;
   DIP_EXPORT constexpr static Classes Class_SInt16 = DT::SINT16;
   DIP_EXPORT constexpr static Classes Class_UInt32 = DT::UINT32;
   DIP_EXPORT constexpr static Classes Class_SInt32 = DT::SINT32;
   DIP_EXPORT constexpr static Classes Class_UInt64 = DT::UINT64;
   DIP_EXPORT constexpr static Classes Class_SInt64 = DT::SINT64;
   DIP_EXPORT constexpr static Classes Class_SFloat = DT::SFLOAT;
   DIP_EXPORT constexpr static Classes Class_DFloat = DT::DFLOAT;
   DIP_EXPORT constexpr static Classes Class_SComplex = DT::SCOMPLEX;
   DIP_EXPORT constexpr static Classes Class_DComplex = DT::DCOMPLEX;
   DIP_EXPORT constexpr static Classes Class_Binary = Class_Bin;
   DIP_EXPORT constexpr static Classes Class_UInt = Class_UInt8 + Class_UInt16 + Class_UInt32 + Class_UInt64;
   DIP_EXPORT constexpr static Classes Class_SInt = Class_SInt8 + Class_SInt16 + Class_SInt32 + Class_SInt64;
   DIP_EXPORT constexpr static Classes Class_Integer = Class_UInt + Class_SInt;
   DIP_EXPORT constexpr static Classes Class_IntOrBin = Class_Integer + Class_Binary;
   DIP_EXPORT constexpr static Classes Class_Float = Class_SFloat + Class_DFloat;
   DIP_EXPORT constexpr static Classes Class_Complex = Class_SComplex + Class_DComplex;
   DIP_EXPORT constexpr static Classes Class_Flex = Class_Float + Class_Complex;
   DIP_EXPORT constexpr static Classes Class_FlexBin = Class_Flex + Class_Binary;
   DIP_EXPORT constexpr static Classes Class_Unsigned = Class_Binary + Class_UInt;
   DIP_EXPORT constexpr static Classes Class_Signed = Class_SInt + Class_Float + Class_Complex;
   DIP_EXPORT constexpr static Classes Class_Real = Class_Integer + Class_Float;
   DIP_EXPORT constexpr static Classes Class_SignedReal = Class_SInt + Class_Float;
   DIP_EXPORT constexpr static Classes Class_NonBinary = Class_Real + Class_Complex;
   DIP_EXPORT constexpr static Classes Class_NonComplex = Class_Binary + Class_Real;
   DIP_EXPORT constexpr static Classes Class_All = Class_Binary + Class_Real + Class_Complex; // == Class_Unsigned + Class_Signed

   /// \brief Implicit conversion to \ref dip::DataType::Classes options class.
   constexpr operator Classes() const { return dt; }

   //
   // Functions to query the data type class
   //

   /// \brief Returns `true` if the data type is of the given class.
   constexpr bool IsA( Classes cls ) const {
      return cls.Contains( dt );
   }

   /// \brief Returns `true` if the data type is binary.
   constexpr bool IsBinary() const {
      return IsA( DT::BIN );
   }

   /// \brief Returns `true` if the data type is an unsigned integer type.
   constexpr bool IsUInt() const {
      return IsA( Class_UInt );
   }

   /// \brief Returns `true` if the data type is a signed integer type.
   constexpr bool IsSInt() const {
      return IsA( Class_SInt );
   }

   /// \brief Returns `true` if the data type is an integer type.
   constexpr bool IsInteger() const {
      return IsA( Class_Integer );
   }

   /// \brief Returns `true` if the data type is a floating point type.
   constexpr bool IsFloat() const {
      return IsA( Class_Float );
   }

   /// \brief Returns `true` if the data type is real (floating point or integer).
   constexpr bool IsReal() const {
      return IsA( Class_Real );
   }

   /// \brief Returns `true` if the data type is one of the "flex" types (floating point or complex).
   constexpr bool IsFlex() const {
      return IsA( Class_Flex );
   }

   /// \brief Returns `true` if the data type is floating point, complex or binary.
   constexpr bool IsFlexBin() const {
      return IsA( Class_FlexBin );
   }

   /// \brief Returns `true` if the data type is complex.
   constexpr bool IsComplex() const {
      return IsA( Class_Complex );
   }

   /// \brief Returns `true` if the data type is an unsigned type (binary or unsigned integer).
   constexpr bool IsUnsigned() const {
      return IsA( Class_Unsigned );
   }

   /// \brief Returns `true` if the data type is a signed type (signed integer, floating point or complex)
   constexpr bool IsSigned() const {
      return IsA( Class_Signed );
   }

   //
   // Functions to suggest an output data type for all types of filters and operators
   //

   /// \brief Returns an integer type that is most suitable to hold samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestInteger( DataType type );

   /// \brief Returns a signed type that is most suitable to hold samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestSigned( DataType type );

   /// \brief Returns a suitable floating-point type that can hold the samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestFloat( DataType type );

   /// \brief Returns a suitable double precision floating-point type (real or complex) that can hold large sums of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestDouble( DataType type );

   /// \brief Returns a suitable complex type that can hold the samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestComplex( DataType type );

   /// \brief Returns a suitable floating-point or complex type that can hold the samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestFlex( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type that can hold the samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestFlexBin( DataType type );

   /// \brief Returns a suitable type that can hold samples of type `abs(type)`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestAbs( DataType type );

   /// \brief Returns a suitable real type that can hold the samples of `type`. See \ref pixeltypes.
   DIP_EXPORT static DataType SuggestReal( DataType type );

   /// \brief Returns a suitable floating-point, complex or binary type ("FlexBin") that can hold the result of an
   /// arithmetic computation performed with the two data types.
   ///
   /// The output value given `type1` and `type2` is as follows. First the two arguments are promoted using
   /// \ref dip::DataType::SuggestFlexBin (which converts 8 and 16-bit integers to `DT_SFLOAT` and 32 and 64-bit integers
   /// to `DT_DFLOAT`), and the resulting two types are looked up in this table (note that the order of the two
   /// inputs is irrelevant, and the table is symmetric):
   ///
   /// &nbsp;            | `DT_BIN`      | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// ----------------- | ------------- | ------------- | ------------- | ------------- | -------------
   /// **`DT_BIN`**      | `DT_BIN`      | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SFLOAT`**   | `DT_SFLOAT`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_DFLOAT`**   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SCOMPLEX`** | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_DCOMPLEX` | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_DCOMPLEX`** | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   DIP_EXPORT static DataType SuggestArithmetic( DataType type1, DataType type2 );

   /// \brief Returns a suitable type that can hold any samples of the two data types.
   ///
   /// The output value given `type1` and `type2` is as follows (note that the order of the two inputs is
   /// irrelevant, and the table is symmetric):
   ///
   /// <div markdown="1" class="m-text m-tiny m-block m-flat">
   ///
   /// &nbsp;            | `DT_BIN`      | `DT_UINT8`    | `DT_SINT8`    | `DT_UINT16`   | `DT_SINT16`   | `DT_UINT32`   | `DT_SINT32`   | `DT_UINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// ----------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------- | -------------
   /// **`DT_BIN`**      | `DT_BIN`      | `DT_UINT8`    | `DT_SINT8`    | `DT_UINT16`   | `DT_SINT16`   | `DT_UINT32`   | `DT_SINT32`   | `DT_UINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_UINT8`**    | `DT_UINT8`    | `DT_UINT8`    | `DT_SINT16`   | `DT_UINT16`   | `DT_SINT16`   | `DT_UINT32`   | `DT_SINT32`   | `DT_UINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SINT8`**    | `DT_SINT8`    | `DT_SINT16`   | `DT_SINT8`    | `DT_SINT32`   | `DT_SINT16`   | `DT_SINT64`   | `DT_SINT32`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_UINT16`**   | `DT_UINT16`   | `DT_UINT16`   | `DT_SINT32`   | `DT_UINT16`   | `DT_SINT32`   | `DT_UINT32`   | `DT_SINT32`   | `DT_UINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SINT16`**   | `DT_SINT16`   | `DT_SINT16`   | `DT_SINT16`   | `DT_SINT32`   | `DT_SINT16`   | `DT_SINT64`   | `DT_SINT32`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_UINT32`**   | `DT_UINT32`   | `DT_UINT32`   | `DT_SINT64`   | `DT_UINT32`   | `DT_SINT64`   | `DT_UINT32`   | `DT_SINT64`   | `DT_UINT64`   | `DT_SINT64`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SINT32`**   | `DT_SINT32`   | `DT_SINT32`   | `DT_SINT32`   | `DT_SINT32`   | `DT_SINT32`   | `DT_SINT64`   | `DT_SINT32`   | `DT_SINT64`   | `DT_SINT64`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_UINT64`**   | `DT_UINT64`   | `DT_UINT64`   | `DT_SINT64`   | `DT_UINT64`   | `DT_SINT64`   | `DT_UINT64`   | `DT_SINT64`   | `DT_UINT64`   | `DT_SINT64`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SINT64`**   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_SINT64`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SFLOAT`**   | `DT_SFLOAT`   | `DT_SFLOAT`   | `DT_SFLOAT`   | `DT_SFLOAT`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_SFLOAT`   | `DT_DFLOAT`   | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_DFLOAT`**   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DFLOAT`   | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_SCOMPLEX`** | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_SCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_SCOMPLEX` | `DT_DCOMPLEX` | `DT_SCOMPLEX` | `DT_DCOMPLEX`
   /// **`DT_DCOMPLEX`** | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX` | `DT_DCOMPLEX`
   ///
   /// </div>
   DIP_EXPORT static DataType SuggestDyadicOperation( DataType type1, DataType type2 );

};

/// \brief You can output a \ref dip::DataType to `std::cout` or any other stream. The result of \ref dip::DataType::Name "`type.Name()`"
/// is written.
/// \relates dip::DataType
inline std::ostream& operator<<( std::ostream& os, DataType type ) {
   os << type.Name();
   return os;
}

inline void swap( DataType& v1, DataType& v2 ) noexcept {
   v1.swap( v2 );
}

// This must be declared outside of the `DataType` class. See the definition of the `DIP_DECLARE_OPTIONS` macro.
constexpr DataType::Classes operator+( DataType::DT a, DataType::DT b ) { return DataType::Classes{ a } + b; }

/// \brief An array to hold data types
/// \relates dip::DataType
using DataTypeArray = DimensionArray< DataType >;

//
// Constructing a DataType object based on the type of an argument
//

namespace detail {

// This is to get the template below to not compile
template< typename T >
struct assert_false : std::false_type {};

// Main template definition, it always bombs out
template< typename T >
constexpr DataType MakeDataType( T /**/ ) {
   static_assert( assert_false< T >::value, "You need to cast your constant to one of the known data types" );
   return DataType{};
}
template<> constexpr DataType MakeDataType( bool /**/     ) { return DataType::DT::BIN; }
template<> constexpr DataType MakeDataType( bin /**/      ) { return DataType::DT::BIN; }
template<> constexpr DataType MakeDataType( uint8 /**/    ) { return DataType::DT::UINT8; }
template<> constexpr DataType MakeDataType( sint8 /**/    ) { return DataType::DT::SINT8; }
template<> constexpr DataType MakeDataType( uint16 /**/   ) { return DataType::DT::UINT16; }
template<> constexpr DataType MakeDataType( sint16 /**/   ) { return DataType::DT::SINT16; }
template<> constexpr DataType MakeDataType( uint32 /**/   ) { return DataType::DT::UINT32; }
template<> constexpr DataType MakeDataType( sint32 /**/   ) { return DataType::DT::SINT32; }
template<> constexpr DataType MakeDataType( uint64 /**/   ) { return DataType::DT::UINT64; }
template<> constexpr DataType MakeDataType( sint64 /**/   ) { return DataType::DT::SINT64; }
template<> constexpr DataType MakeDataType( sfloat /**/   ) { return DataType::DT::SFLOAT; }
template<> constexpr DataType MakeDataType( dfloat /**/   ) { return DataType::DT::DFLOAT; }
template<> constexpr DataType MakeDataType( scomplex /**/ ) { return DataType::DT::SCOMPLEX; }
template<> constexpr DataType MakeDataType( dcomplex /**/ ) { return DataType::DT::DCOMPLEX; }

#if SIZE_MAX == UINT32_MAX
constexpr DataType MakeDataType( dip::uint /**/ ) { return DataType::DT::UINT32; }
constexpr DataType MakeDataType( dip::sint /**/ ) { return DataType::DT::SINT32; }
#elif SIZE_MAX == UINT64_MAX
constexpr DataType MakeDataType( dip::uint /**/ ) { return DataType::DT::UINT64; }
constexpr DataType MakeDataType( dip::sint /**/ ) { return DataType::DT::SINT64; }
#endif

} // namespace detail

template< typename T >
constexpr DataType::DataType( T v ) : dt( detail::MakeDataType( v ).dt ) {}

static_assert( sizeof( unsigned long long ) == 8, "unsigned long long type is not 64 bits" );
constexpr DataType::DataType( unsigned long long /*v*/ ) : dt( DataType::DT::UINT64 ) {}
static_assert( sizeof( long long ) == 8, "long long type is not 64 bits" );
constexpr DataType::DataType( long long /*v*/ ) : dt( DataType::DT::SINT64 ) {}

static_assert( sizeof( unsigned long ) == 8 || sizeof( unsigned long ) == 4, "unsigned long type is not 32 nor 64 bits" );
constexpr DataType::DataType( unsigned long /*v*/ ) : dt( sizeof( unsigned long ) == 8 ? DataType::DT::UINT64 : DataType::DT::UINT32 ) {}
static_assert( sizeof( long ) == 8 || sizeof( long ) == 4, "long type is not 32 nor 64 bits" );
constexpr DataType::DataType( long /*v*/ ) : dt( sizeof( long ) == 8 ? DataType::DT::SINT64 : DataType::DT::SINT32 ) {}

//
// Constants that people will use where a DataType is needed
//

/// Constant representing the type \ref dip::bin.
constexpr DataType DT_BIN{ DataType::DT::BIN };
/// Constant representing the type \ref dip::uint8.
constexpr DataType DT_UINT8{ DataType::DT::UINT8 };
/// Constant representing the type \ref dip::sint8.
constexpr DataType DT_SINT8{ DataType::DT::SINT8 };
/// Constant representing the type \ref dip::uint16.
constexpr DataType DT_UINT16{ DataType::DT::UINT16 };
/// Constant representing the type \ref dip::sint16.
constexpr DataType DT_SINT16{ DataType::DT::SINT16 };
/// Constant representing the type \ref dip::uint32.
constexpr DataType DT_UINT32{ DataType::DT::UINT32 };
/// Constant representing the type \ref dip::sint32.
constexpr DataType DT_SINT32{ DataType::DT::SINT32 };
/// Constant representing the type \ref dip::uint64.
constexpr DataType DT_UINT64{ DataType::DT::UINT64 };
/// Constant representing the type \ref dip::sint64.
constexpr DataType DT_SINT64{ DataType::DT::SINT64 };
/// Constant representing the type \ref dip::sfloat.
constexpr DataType DT_SFLOAT{ DataType::DT::SFLOAT };
/// Constant representing the type \ref dip::dfloat.
constexpr DataType DT_DFLOAT{ DataType::DT::DFLOAT };
/// Constant representing the type \ref dip::scomplex.
constexpr DataType DT_SCOMPLEX{ DataType::DT::SCOMPLEX };
/// Constant representing the type \ref dip::dcomplex.
constexpr DataType DT_DCOMPLEX{ DataType::DT::DCOMPLEX };

/// Type currently used for all labeled images, see \ref dip::LabelType.
constexpr DataType DT_LABEL = DT_UINT32;

/// \endgroup

} // namespace dip

#endif // DIP_DATATYPE_H
