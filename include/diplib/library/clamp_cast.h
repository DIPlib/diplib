/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the function dip::clamp_cast<>().
 *
 * (c)2016-2017, Cris Luengo.
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
/// TODO: Do we want to round the float values instead?
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


// Basis of dip::clamp_cast<>
template< typename T, typename S >
constexpr inline const T clamp_both( S const& v ) {
   return static_cast< T >( clamp(
         v,
         static_cast< S >( std::numeric_limits< T >::lowest() ),
         static_cast< S >( std::numeric_limits< T >::max()    ) ));
}
template< typename T, typename S >
constexpr inline const T clamp_lower( S const& v ) {   // T is an unsigned integer type with same or more bits than S
   return static_cast< T >( std::max( v, static_cast< S >( 0 ) ) );
}
template< typename T, typename S >
constexpr inline const T clamp_upper( S const& v ) {   // S is an unsigned integer type with same or more bits than T
   return static_cast< T >( std::min( v, static_cast< S >( std::numeric_limits< T >::max() ) ) );

}

/*
// Another version, instead of clamp_both, clamp_lower and clamp_upper.
// The compiler will optimize away unnecessary tests.
// Not used because casting the limit of T to S might cause overflow if S is
// narrower. How do we test for that case? In the code we use now we've
// manually written out most cases. It looks stupid but it works.
constexpr inline const T clamp_cast_basis( S const& v ) {
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
// We implement these casts as a function `clamp_cast< target type>( value )`.
// The target type is a template parameter, the input value's type is used
// in regular function overload selection. We define specialized version of
// the templates where needed.

/// Casting from a binary value to any other sample type.
// Casting from dip::bin is always OK, except when casting to complex values
template< typename T >
constexpr inline T clamp_cast( dip::bin v ) {
   return T( v );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::bin v ) {
   return { static_cast< dip::sfloat >( v ), 0.0f };
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::bin v ) {
   return { static_cast< dip::dfloat >( v ), 0.0 };
}

/// Casting from a 8-bit unsigned value to any other sample type.
// Casting from dip::uint8, specialize for dip::sint8
template< typename T >
constexpr inline T clamp_cast( dip::uint8 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint8 v ) {
   return clamp_upper< dip::sint8, dip::uint8 >( v );
}

/// Casting from a 16-bit unsigned value to any other sample type.
// Casting from a dip::uint16, specialize for dip::uint8, dip::sint8 and dip::sint16
template< typename T >
constexpr inline T clamp_cast( dip::uint16 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline dip::uint8 clamp_cast< dip::uint8 >( dip::uint16 v ) {
   return clamp_upper< dip::uint8, dip::uint16 >( v );
}
template<>
constexpr inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint16 v ) {
   return clamp_upper< dip::sint8, dip::uint16 >( v );
}
template<>
constexpr inline dip::sint16 clamp_cast< dip::sint16 >( dip::uint16 v ) {
   return clamp_upper< dip::sint16, dip::uint16 >( v );
}

/// Casting from a 32-bit unsigned value to any other sample type.
// Casting from a dip::uint32, specialize for dip::uint8, dip::uint16 and all signed types
template< typename T >
constexpr inline T clamp_cast( dip::uint32 v ) {
   return static_cast< T >( static_cast< FloatType< T >>( v )); // two casts for the case of T = complex.
}
template<>
constexpr inline dip::uint8 clamp_cast< dip::uint8 >( dip::uint32 v ) {
   return clamp_upper< dip::uint8, dip::uint32 >( v );
}
template<>
constexpr inline dip::uint16 clamp_cast< dip::uint16 >( dip::uint32 v ) {
   return clamp_upper< dip::uint16, dip::uint32 >( v );
}
template<>
constexpr inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint32 v ) {
   return clamp_upper< dip::sint8, dip::uint32 >( v );
}
template<>
constexpr inline dip::sint16 clamp_cast< dip::sint16 >( dip::uint32 v ) {
   return clamp_upper< dip::sint16, dip::uint32 >( v );
}
template<>
constexpr inline dip::sint32 clamp_cast< dip::sint32 >( dip::uint32 v ) {
   return clamp_upper< dip::sint32, dip::uint32 >( v );
}

#if SIZE_MAX != UINT32_MAX // we don't want to compile this segment on 32-bit machines, they'd conflict with the segment above.
/// Casting from a machine-width unsigned value to any other sample type.
// Casting from a dip::uint, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( dip::uint v ) {
   return clamp_upper< T, dip::uint >( v );
}
template<>
constexpr inline dip::sfloat clamp_cast< dip::sfloat >( dip::uint v ) {
   return static_cast< dip::sfloat >( v );
}
template<>
constexpr inline dip::dfloat clamp_cast< dip::dfloat >( dip::uint v ) {
   return static_cast< dip::dfloat >( v );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::uint v ) {
   return static_cast< dip::scomplex >( static_cast< dip::sfloat >( v ) );
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::uint v ) {
   return static_cast< dip::dcomplex >( static_cast< dip::dfloat >( v ) );
}
template<>
constexpr inline dip::bin clamp_cast< dip::bin >( dip::uint v ) {
   return static_cast< dip::bin >( v );
}
#endif

/// Casting from a 8-bit signed value to any other sample type.
// Casting from dip::sint8, specialize for all unsigned types
template< typename T >
constexpr inline T clamp_cast( dip::sint8 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint8 v ) {
   return clamp_lower< dip::uint8, dip::sint8 >( v );
}
template<>
constexpr inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint8 v ) {
   return clamp_lower< dip::uint16, dip::sint8 >( v );
}
template<>
constexpr inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint8 v ) {
   return clamp_lower< dip::uint32, dip::sint8 >( v );
}
#if SIZE_MAX != UINT32_MAX // we don't want to compile this segment on 32-bit machines, they'd conflict with the segment above.
template<>
constexpr inline dip::uint clamp_cast< dip::uint >( dip::sint8 v ) {
   return clamp_lower< dip::uint, dip::sint8 >( v );
}
#endif

/// Casting from a 16-bit signed value to any other sample type.
// Casting from a dip::sint16, specialize for dip::sint8 and all unsigned types
template< typename T >
constexpr inline T clamp_cast( dip::sint16 v ) {
   return static_cast< T >( v );
}
template<>
constexpr inline dip::sint8 clamp_cast< dip::sint8 >( dip::sint16 v ) {
   return clamp_both< dip::sint8, dip::sint16 >( v );
}
template<>
constexpr inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint16 v ) {
   return clamp_both< dip::uint8, dip::sint16 >( v );
}
template<>
constexpr inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint16 v ) {
   return clamp_lower< dip::uint16, dip::sint16 >( v );
}
template<>
constexpr inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint16 v ) {
   return clamp_lower< dip::uint32, dip::sint16 >( v );
}
#if SIZE_MAX != UINT32_MAX // we don't want to compile this segment on 32-bit machines, they'd conflict with the segment above.
template<>
constexpr inline dip::uint clamp_cast< dip::uint >( dip::sint16 v ) {
   return clamp_lower< dip::uint, dip::sint16 >( v );
}
#endif

/// Casting from a 32-bit signed value to any other sample type.
// Casting from a dip::sint32, specialize for dip::sint8, dip::sint16 and all unsigned types
template< typename T >
constexpr inline T clamp_cast( dip::sint32 v ) {
   return static_cast< T >( static_cast< FloatType< T >>( v )); // two casts for the case of T = complex.
}
template<>
constexpr inline dip::sint8 clamp_cast< dip::sint8 >( dip::sint32 v ) {
   return clamp_both< dip::sint8, dip::sint32 >( v );
}
template<>
constexpr inline dip::sint16 clamp_cast< dip::sint16 >( dip::sint32 v ) {
   return clamp_both< dip::sint16, dip::sint32 >( v );
}
template<>
constexpr inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint32 v ) {
   return clamp_both< dip::uint8, dip::sint32 >( v );
}
template<>
constexpr inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint32 v ) {
   return clamp_both< dip::uint16, dip::sint32 >( v );
}
template<>
constexpr inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint32 v ) {
   return clamp_lower< dip::uint32, dip::sint32 >( v );
}
#if SIZE_MAX != UINT32_MAX // we don't want to compile this segment on 32-bit machines, they'd conflict with the segment above.
template<>
constexpr inline dip::uint clamp_cast< dip::uint >( dip::sint32 v ) {
   return clamp_lower< dip::uint, dip::sint32 >( v );
}
#endif

#if PTRDIFF_MAX != INT32_MAX // we don't want to compile this segment on 32-bit machines, they'd conflict with the segment above.
/// Casting from a machine-width signed value to any other sample type.
// Casting from a dip::sint, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( dip::sint v ) {
   return clamp_both< T, dip::sint >( v );
}
template<>
constexpr inline dip::uint clamp_cast< dip::uint >( dip::sint v ) {
   return clamp_lower< dip::uint, dip::sint >( v );
}
template<>
constexpr inline dip::sfloat clamp_cast< dip::sfloat >( dip::sint v ) {
   return static_cast< dip::sfloat >( v );
}
template<>
constexpr inline dip::dfloat clamp_cast< dip::dfloat >( dip::sint v ) {
   return static_cast< dip::dfloat >( v );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::sint v ) {
   return static_cast< dip::scomplex >( static_cast< dip::sfloat >( v ));
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::sint v ) {
   return static_cast< dip::dcomplex >( static_cast< dip::dfloat >( v ));
}
template<>
constexpr inline dip::bin clamp_cast< dip::bin >( dip::sint v ) {
   return static_cast< dip::bin >( v );
}
#endif

/// Casting from a single-precision float value to any other sample type.
// Casting from a dip::sfloat, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( dip::sfloat v ) {
   return clamp_both< T, dip::sfloat >( v );
}
template<>
constexpr inline dip::sfloat clamp_cast< dip::sfloat >( dip::sfloat v ) {
   return v;
}
template<>
constexpr inline dip::dfloat clamp_cast< dip::dfloat >( dip::sfloat v ) {
   return static_cast< dip::dfloat >( v );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::sfloat v ) {
   return static_cast< dip::scomplex >( v );
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::sfloat v ) {
   return static_cast< dip::dcomplex >( static_cast< dip::dfloat >( v ));
}
template<>
constexpr inline dip::bin clamp_cast< dip::bin >( dip::sfloat v ) {
   return static_cast< dip::bin >( v );
}

/// Casting from a double-precision float value to any other sample type.
// Casting from a dip::dfloat, we don't do checks if casting to a float, complex or bin
template< typename T >
constexpr inline T clamp_cast( dip::dfloat v ) {
   return clamp_both< T, dip::dfloat >( v );
}
template<>
constexpr inline dip::dfloat clamp_cast< dip::dfloat >( dip::dfloat v ) {
   return v;
}
template<>
constexpr inline dip::sfloat clamp_cast< dip::sfloat >( dip::dfloat v ) {
   return static_cast< dip::sfloat >( v );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::dfloat v ) {
   return static_cast< dip::scomplex >( static_cast< dip::sfloat >( v ));
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::dfloat v ) {
   return static_cast< dip::dcomplex >( v );
}
template<>
constexpr inline dip::bin clamp_cast< dip::bin >( dip::dfloat v ) {
   return static_cast< dip::bin >( v );
}

/// Casting from a single-precision complex value to any other sample type.
// Casting from an dip::scomplex, we take the absolute value and cast as if from a float
template< typename T >
constexpr inline T clamp_cast( dip::scomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::scomplex v ) {
   return v;
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::scomplex v ) {
   return dip::dcomplex{ static_cast< dip::dfloat >( v.real() ), static_cast< dip::dfloat >( v.imag() ) };
}

/// Casting from a double-precision complex value to any other sample type.
// Casting from a dip::dcomplex, we take the absolute value and cast as if from a float
template< typename T >
constexpr inline T clamp_cast( dip::dcomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
constexpr inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::dcomplex v ) {
   return v;
}
template<>
constexpr inline dip::scomplex clamp_cast< dip::scomplex >( dip::dcomplex v ) {
   return dip::scomplex{ static_cast< dip::sfloat >( v.real() ), static_cast< dip::sfloat >( v.imag() ) };
}

/// \}

namespace detail {
template< typename T >
constexpr T CastSample( DataType dataType, void const* data ) {
   switch( dataType ) {
      case dip::DT_BIN      : return clamp_cast< T >( *static_cast< bin const* >( data ));
      case dip::DT_UINT8    : return clamp_cast< T >( *static_cast< uint8 const* >( data ));
      case dip::DT_UINT16   : return clamp_cast< T >( *static_cast< uint16 const* >( data ));
      case dip::DT_UINT32   : return clamp_cast< T >( *static_cast< uint32 const* >( data ));
      case dip::DT_SINT8    : return clamp_cast< T >( *static_cast< sint8 const* >( data ));
      case dip::DT_SINT16   : return clamp_cast< T >( *static_cast< sint16 const* >( data ));
      case dip::DT_SINT32   : return clamp_cast< T >( *static_cast< sint32 const* >( data ));
      case dip::DT_SFLOAT   : return clamp_cast< T >( *static_cast< sfloat const* >( data ));
      case dip::DT_DFLOAT   : return clamp_cast< T >( *static_cast< dfloat const* >( data ));
      case dip::DT_SCOMPLEX : return clamp_cast< T >( *static_cast< scomplex const* >( data ));
      case dip::DT_DCOMPLEX : return clamp_cast< T >( *static_cast< dcomplex const* >( data ));
      default: return 0; // DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }
}
constexpr inline void CastSample( DataType srcType, void const* src, DataType destType, void* dest ) {
   switch( destType ) {
      case dip::DT_BIN      : *static_cast< bin* >( dest ) = CastSample< bin >( srcType, src ); break;
      case dip::DT_UINT8    : *static_cast< uint8* >( dest ) = CastSample< uint8 >( srcType, src ); break;
      case dip::DT_UINT16   : *static_cast< uint16* >( dest ) = CastSample< uint16 >( srcType, src ); break;
      case dip::DT_UINT32   : *static_cast< uint32* >( dest ) = CastSample< uint32 >( srcType, src ); break;
      case dip::DT_SINT8    : *static_cast< sint8* >( dest ) = CastSample< sint8 >( srcType, src ); break;
      case dip::DT_SINT16   : *static_cast< sint16* >( dest ) = CastSample< sint16 >( srcType, src ); break;
      case dip::DT_SINT32   : *static_cast< sint32* >( dest ) = CastSample< sint32 >( srcType, src ); break;
      case dip::DT_SFLOAT   : *static_cast< sfloat* >( dest ) = CastSample< sfloat >( srcType, src ); break;
      case dip::DT_DFLOAT   : *static_cast< dfloat* >( dest ) = CastSample< dfloat >( srcType, src ); break;
      case dip::DT_SCOMPLEX : *static_cast< scomplex* >( dest ) = CastSample< scomplex >( srcType, src ); break;
      case dip::DT_DCOMPLEX : *static_cast< dcomplex* >( dest ) = CastSample< dcomplex >( srcType, src ); break;
      default: return; // DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }
}
} // namespace detail

} // namespace dip

#endif // DIP_CLAMP_CAST_H
