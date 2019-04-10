/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the function dip::clamp_cast<>().
 *
 * (c)2016-2019, Cris Luengo.
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


#ifndef DIP_CLAMP_CAST_H
#define DIP_CLAMP_CAST_H

#include <limits>

#include "diplib/library/numeric.h"
#include "diplib/library/datatype.h"


/// \file
/// \brief Defines `dip::clamp_cast`. This file is always included through `diplib.h`.
/// \see sample_operators


namespace dip {


/// \defgroup sample_operators Saturated arithmetic and casts
/// \ingroup infrastructure
/// \brief These operators operate on single sample values, implementing saturated arithmetic and type casting.
///
/// `%dip::clamp_cast` is an operator that returns the input value cast to a different
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
/// `%dip::clamp_cast` is defined as a series of overloaded template functions with specializations.
/// The input argument type is used in overload resolution, the output type is the template
/// parameter, and should be specified between angled brackets after the function name,
/// much like the standard `static_cast` and similar:
///
/// ```cpp
///     uint8 u = dip::clamp_cast< dip::uint8 >( -54.6 );
/// ```
///
/// `%dip::clamp_cast` is made available when including `diplib.h`.
/// \{
// TODO: Do we want to round the float values instead?

// Basis of dip::clamp_cast<>
template< typename T, typename S >
constexpr inline const T clamp_both( S v ) {
   return static_cast< T >( clamp(
         v,
         static_cast< S >( std::numeric_limits< T >::lowest() ),
         static_cast< S >( std::numeric_limits< T >::max()    ) ));
}
template< typename T, typename S >
constexpr inline const T clamp_lower( S v ) {   // T is an unsigned integer type with same or more bits than S
   return static_cast< T >( std::max( v, static_cast< S >( 0 ) ) );
}
template< typename T, typename S >
constexpr inline const T clamp_upper( S v ) {   // S is an unsigned integer type with same or more bits than T
   return static_cast< T >( std::min( v, static_cast< S >( std::numeric_limits< T >::max() ) ) );

}

/*
// Another version, instead of clamp_both, clamp_lower and clamp_upper.
// The compiler will optimize away unnecessary tests.
// Not used because casting the limit of T to S might cause overflow if S is
// narrower. How do we test for that case? In the code we use now we've
// manually written out most cases. It looks stupid but it works.
constexpr inline const T clamp_cast_basis( S v ) {
   // Check lower bound
   if ( std::numeric_limits< S >::lowest() < static_cast< S >( std::numeric_limits< T >::lowest() ) ) {
      if ( v < static_cast< S >( std::numeric_limits< T >::lowest() ) ) {
         return std::numeric_limits< T >::lowest();
      }
   }
   // Check upper bound
   if ( std::numeric_limits< S >::max() > static_cast< S >( std::numeric_limits< T >::max() ) ) {
      if ( v > static_cast< S >( std::numeric_limits< T >::max() ) ) {
         return std::numeric_limits< T >::max();
      }
   }
   // Cast
   return static_cast< T >( v );
}
*/

// Here we define functions to cast any pixel data type to any other pixel
// data type. When casting to an integer type, the value is first clamped
// (clipped, saturated) to the target type's range. For floating point types,
// we don't care about overflow and underflow, and let the IEEE spec take
// care of things.
//
// We implement these casts as a function `clamp_cast<target_type>( value )`.
// The target type is a template parameter, the input value's type is used
// in regular function overload selection. We define specialized version of
// the templates where needed.

/// Casting from a binary value to any other sample type.
// Casting from bin is always OK, except when casting to complex values
template< typename T >
constexpr inline T clamp_cast( bin v ) {
   return T( v );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( bin v ) {
   return { static_cast< sfloat >( v ), 0.0f };
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( bin v ) {
   return { static_cast< dfloat >( v ), 0.0 };
}

/// Casting from a 8-bit unsigned value to any other sample type.
// Casting from uint8, specialize for sint8
template< typename T >
constexpr inline T clamp_cast( uint8 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline sint8 clamp_cast< sint8 >( uint8 v ) {
   return clamp_upper< sint8, uint8 >( v );
}

/// Casting from a 16-bit unsigned value to any other sample type.
// Casting from a uint16, specialize for uint8, sint8 and sint16
template< typename T >
constexpr inline T clamp_cast( uint16 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline uint8 clamp_cast< uint8 >( uint16 v ) {
   return clamp_upper< uint8, uint16 >( v );
}
template<>
constexpr inline sint8 clamp_cast< sint8 >( uint16 v ) {
   return clamp_upper< sint8, uint16 >( v );
}
template<>
constexpr inline sint16 clamp_cast< sint16 >( uint16 v ) {
   return clamp_upper< sint16, uint16 >( v );
}

/// Casting from a 32-bit unsigned value to any other sample type.
// Casting from a uint32, specialize for uint8, uint16 and all signed types
template< typename T >
constexpr inline T clamp_cast( uint32 v ) {
   return static_cast< T >( static_cast< FloatType< T >>( v )); // two casts for the case of T = complex.
}
template<>
constexpr inline uint8 clamp_cast< uint8 >( uint32 v ) {
   return clamp_upper< uint8, uint32 >( v );
}
template<>
constexpr inline uint16 clamp_cast< uint16 >( uint32 v ) {
   return clamp_upper< uint16, uint32 >( v );
}
template<>
constexpr inline sint8 clamp_cast< sint8 >( uint32 v ) {
   return clamp_upper< sint8, uint32 >( v );
}
template<>
constexpr inline sint16 clamp_cast< sint16 >( uint32 v ) {
   return clamp_upper< sint16, uint32 >( v );
}
template<>
constexpr inline sint32 clamp_cast< sint32 >( uint32 v ) {
   return clamp_upper< sint32, uint32 >( v );
}

/// Casting from a 64-bit unsigned value to any other sample type.
// Casting from a uint64, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( uint64 v ) {
   return clamp_upper< T, uint64 >( v );
}
template<>
constexpr inline sfloat clamp_cast< sfloat >( uint64 v ) {
   return static_cast< sfloat >( v );
}
template<>
constexpr inline dfloat clamp_cast< dfloat >( uint64 v ) {
   return static_cast< dfloat >( v );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( uint64 v ) {
   return static_cast< scomplex >( static_cast< sfloat >( v ) );
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( uint64 v ) {
   return static_cast< dcomplex >( static_cast< dfloat >( v ) );
}
template<>
constexpr inline bin clamp_cast< bin >( uint64 v ) {
   return static_cast< bin >( v );
}

template< typename T >
constexpr inline T clamp_cast( dip::uint v ) {
   return clamp_cast< T >( static_cast< uint64 >( v )); // TODO: I don't like this, it makes assumptions about the size of the pointer...
}

/// Casting from a 8-bit signed value to any other sample type.
// Casting from sint8, specialize for all unsigned types
template< typename T >
constexpr inline T clamp_cast( sint8 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline uint8 clamp_cast< uint8 >( sint8 v ) {
   return clamp_lower< uint8, sint8 >( v );
}
template<>
constexpr inline uint16 clamp_cast< uint16 >( sint8 v ) {
   return clamp_lower< uint16, sint8 >( v );
}
template<>
constexpr inline uint32 clamp_cast< uint32 >( sint8 v ) {
   return clamp_lower< uint32, sint8 >( v );
}
template<>
constexpr inline uint64 clamp_cast< uint64 >( sint8 v ) {
   return clamp_lower< uint64, sint8 >( v );
}

/// Casting from a 16-bit signed value to any other sample type.
// Casting from a sint16, specialize for sint8 and all unsigned types
template< typename T >
constexpr inline T clamp_cast( sint16 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline sint8 clamp_cast< sint8 >( sint16 v ) {
   return clamp_both< sint8, sint16 >( v );
}
template<>
constexpr inline uint8 clamp_cast< uint8 >( sint16 v ) {
   return clamp_both< uint8, sint16 >( v );
}
template<>
constexpr inline uint16 clamp_cast< uint16 >( sint16 v ) {
   return clamp_lower< uint16, sint16 >( v );
}
template<>
constexpr inline uint32 clamp_cast< uint32 >( sint16 v ) {
   return clamp_lower< uint32, sint16 >( v );
}
template<>
constexpr inline uint64 clamp_cast< uint64 >( sint16 v ) {
   return clamp_lower< uint64, sint16 >( v );
}

/// Casting from a 32-bit signed value to any other sample type.
// Casting from a sint32, specialize for sint8, sint16 and all unsigned types
template< typename T >
constexpr inline T clamp_cast( sint32 v ) {
   return static_cast< T >( static_cast< FloatType< T >>( v )); // two casts for the case of T = complex.
}
template<>
constexpr inline sint8 clamp_cast< sint8 >( sint32 v ) {
   return clamp_both< sint8, sint32 >( v );
}
template<>
constexpr inline sint16 clamp_cast< sint16 >( sint32 v ) {
   return clamp_both< sint16, sint32 >( v );
}
template<>
constexpr inline uint8 clamp_cast< uint8 >( sint32 v ) {
   return clamp_both< uint8, sint32 >( v );
}
template<>
constexpr inline uint16 clamp_cast< uint16 >( sint32 v ) {
   return clamp_both< uint16, sint32 >( v );
}
template<>
constexpr inline uint32 clamp_cast< uint32 >( sint32 v ) {
   return clamp_lower< uint32, sint32 >( v );
}
template<>
constexpr inline uint64 clamp_cast< uint64 >( sint32 v ) {
   return clamp_lower< uint64, sint32 >( v );
}

/// Casting from a 64-bit signed value to any other sample type.
// Casting from a sint64, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( sint64 v ) {
   return clamp_both< T, sint64 >( v );
}
template<>
constexpr inline uint64 clamp_cast< uint64 >( sint64 v ) {
   return clamp_lower< uint64, sint64 >( v );
}
template<>
constexpr inline sfloat clamp_cast< sfloat >( sint64 v ) {
   return static_cast< sfloat >( v );
}
template<>
constexpr inline dfloat clamp_cast< dfloat >( sint64 v ) {
   return static_cast< dfloat >( v );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( sint64 v ) {
   return static_cast< scomplex >( static_cast< sfloat >( v ));
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( sint64 v ) {
   return static_cast< dcomplex >( static_cast< dfloat >( v ));
}
template<>
constexpr inline bin clamp_cast< bin >( sint64 v ) {
   return static_cast< bin >( v );
}

template< typename T >
constexpr inline T clamp_cast( dip::sint v ) {
   return clamp_cast< T >( static_cast< sint64 >( v )); // TODO: I don't like this, it makes assumptions about the size of the pointer...
}

/// Casting from a single-precision float value to any other sample type.
// Casting from a sfloat, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( sfloat v ) {
   return clamp_both< T, sfloat >( v );
}
template<>
constexpr inline sfloat clamp_cast< sfloat >( sfloat v ) {
   return v;
}
template<>
constexpr inline dfloat clamp_cast< dfloat >( sfloat v ) {
   return static_cast< dfloat >( v );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( sfloat v ) {
   return static_cast< scomplex >( v );
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( sfloat v ) {
   return static_cast< dcomplex >( static_cast< dfloat >( v ));
}
template<>
constexpr inline bin clamp_cast< bin >( sfloat v ) {
   return static_cast< bin >( v );
}

/// Casting from a double-precision float value to any other sample type.
// Casting from a dfloat, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( dfloat v ) {
   return clamp_both< T, dfloat >( v );
}
template<>
constexpr inline dfloat clamp_cast< dfloat >( dfloat v ) {
   return v;
}
template<>
constexpr inline sfloat clamp_cast< sfloat >( dfloat v ) {
   return static_cast< sfloat >( v );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( dfloat v ) {
   return static_cast< scomplex >( static_cast< sfloat >( v ));
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( dfloat v ) {
   return static_cast< dcomplex >( v );
}
template<>
constexpr inline bin clamp_cast< bin >( dfloat v ) {
   return static_cast< bin >( v );
}

/// Casting from a single-precision complex value to any other sample type.
// Casting from an scomplex, we take the absolute value and cast as if from a float
template< typename T >
constexpr inline T clamp_cast( scomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( scomplex v ) {
   return v;
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( scomplex v ) {
   return dcomplex{ static_cast< dfloat >( v.real() ), static_cast< dfloat >( v.imag() ) };
}

/// Casting from a double-precision complex value to any other sample type.
// Casting from a dcomplex, we take the absolute value and cast as if from a float
template< typename T >
constexpr inline T clamp_cast( dcomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
constexpr inline dcomplex clamp_cast< dcomplex >( dcomplex v ) {
   return v;
}
template<>
constexpr inline scomplex clamp_cast< scomplex >( dcomplex v ) {
   return scomplex{ static_cast< sfloat >( v.real() ), static_cast< sfloat >( v.imag() ) };
}

/// \}

namespace detail {
template< typename T >
constexpr T CastSample( DataType dataType, void const* data ) {
   switch( dataType ) {
      case DT_BIN      : return clamp_cast< T >( *static_cast< bin const* >( data ));
      case DT_UINT8    : return clamp_cast< T >( *static_cast< uint8 const* >( data ));
      case DT_UINT16   : return clamp_cast< T >( *static_cast< uint16 const* >( data ));
      case DT_UINT32   : return clamp_cast< T >( *static_cast< uint32 const* >( data ));
      case DT_UINT64   : return clamp_cast< T >( *static_cast< uint64 const* >( data ));
      case DT_SINT8    : return clamp_cast< T >( *static_cast< sint8 const* >( data ));
      case DT_SINT16   : return clamp_cast< T >( *static_cast< sint16 const* >( data ));
      case DT_SINT32   : return clamp_cast< T >( *static_cast< sint32 const* >( data ));
      case DT_SINT64   : return clamp_cast< T >( *static_cast< sint64 const* >( data ));
      case DT_SFLOAT   : return clamp_cast< T >( *static_cast< sfloat const* >( data ));
      case DT_DFLOAT   : return clamp_cast< T >( *static_cast< dfloat const* >( data ));
      case DT_SCOMPLEX : return clamp_cast< T >( *static_cast< scomplex const* >( data ));
      case DT_DCOMPLEX : return clamp_cast< T >( *static_cast< dcomplex const* >( data ));
      default: return 0; // DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }
}
constexpr inline void CastSample( DataType srcType, void const* src, DataType destType, void* dest ) {
   switch( destType ) {
      case DT_BIN      : *static_cast< bin* >( dest ) = CastSample< bin >( srcType, src ); break;
      case DT_UINT8    : *static_cast< uint8* >( dest ) = CastSample< uint8 >( srcType, src ); break;
      case DT_UINT16   : *static_cast< uint16* >( dest ) = CastSample< uint16 >( srcType, src ); break;
      case DT_UINT32   : *static_cast< uint32* >( dest ) = CastSample< uint32 >( srcType, src ); break;
      case DT_UINT64   : *static_cast< uint64* >( dest ) = CastSample< uint64 >( srcType, src ); break;
      case DT_SINT8    : *static_cast< sint8* >( dest ) = CastSample< sint8 >( srcType, src ); break;
      case DT_SINT16   : *static_cast< sint16* >( dest ) = CastSample< sint16 >( srcType, src ); break;
      case DT_SINT32   : *static_cast< sint32* >( dest ) = CastSample< sint32 >( srcType, src ); break;
      case DT_SINT64   : *static_cast< sint64* >( dest ) = CastSample< sint64 >( srcType, src ); break;
      case DT_SFLOAT   : *static_cast< sfloat* >( dest ) = CastSample< sfloat >( srcType, src ); break;
      case DT_DFLOAT   : *static_cast< dfloat* >( dest ) = CastSample< dfloat >( srcType, src ); break;
      case DT_SCOMPLEX : *static_cast< scomplex* >( dest ) = CastSample< scomplex >( srcType, src ); break;
      case DT_DCOMPLEX : *static_cast< dcomplex* >( dest ) = CastSample< dcomplex >( srcType, src ); break;
      default: return; // DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }
}
} // namespace detail

} // namespace dip

#endif // DIP_CLAMP_CAST_H
