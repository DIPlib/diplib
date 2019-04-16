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


#ifdef __SIZEOF_INT128__
namespace std {
// These seem not defined?
// WARNING! We define only a useful subset of the struct.

template<>
struct numeric_limits< __uint128_t > {
   static constexpr bool is_specialized = true;
   static constexpr int digits = 128;
   static constexpr bool is_signed = false;
   static constexpr bool is_integer = true;
   static constexpr bool has_infinity = false;
   static constexpr bool has_quiet_NaN = false;
   static constexpr bool has_signaling_NaN = false;
   static constexpr __uint128_t max() noexcept { return static_cast< __uint128_t >( __int128_t( -1 )); }
   static constexpr __uint128_t min() noexcept { return 0; }
   static constexpr __uint128_t lowest() noexcept { return min(); }
};

template<>
struct numeric_limits< __int128_t > {
   static constexpr bool is_specialized = true;
   static constexpr int digits = 127;
   static constexpr bool is_signed = true;
   static constexpr bool is_integer = true;
   static constexpr bool has_infinity = false;
   static constexpr bool has_quiet_NaN = false;
   static constexpr bool has_signaling_NaN = false;
   static constexpr __int128_t max() noexcept { return static_cast< __int128_t >( numeric_limits< __uint128_t >::max() >> 1u ); }
   static constexpr __int128_t min() noexcept { return -max() - 1; }
   static constexpr __int128_t lowest() noexcept { return min(); }
};

// This one is used outside of this file too (at least max and min.
template<>
struct numeric_limits< dip::bin > {
   static constexpr bool is_specialized = false; // Set to false, below we test this instead of "is_arithmetic".
   static constexpr int digits = 1;
   static constexpr bool is_signed = false;
   static constexpr bool is_integer = false;
   static constexpr bool has_infinity = false;
   static constexpr bool has_quiet_NaN = false;
   static constexpr bool has_signaling_NaN = false;
   static constexpr dip::bin max() noexcept { return true; }
   static constexpr dip::bin min() noexcept { return false; }
   static constexpr dip::bin lowest() noexcept { return min(); }
};

} // namespace std
#endif // __SIZEOF_INT128__


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

namespace detail {

// These in the std:: namespace don't always work for 128-bit types. Let's make our own based on numeric limits:
template< typename T > struct is_floating_point{ static constexpr bool value =
         std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_specialized
         && !std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_integer; };
template< typename T > struct is_signed{ static constexpr bool value =
         std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_specialized
         && std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_signed; };
template< typename T > struct is_integer{ static constexpr bool value =
         std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_specialized
         && std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_integer; };
template< typename T > struct is_unsigned_integer{ static constexpr bool value =
         std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_specialized
         && std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_integer
         && !std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_signed; };
template< typename T > struct is_signed_integer{ static constexpr bool value =
         std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_specialized
         && std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_integer
         && std::numeric_limits< typename std::remove_cv_t< std::remove_reference_t< T >>>::is_signed; };

template< typename T > struct is_complex_base{ static constexpr bool value = false; };
template<> struct is_complex_base< scomplex >{ static constexpr bool value = true; };
template<> struct is_complex_base< dcomplex >{ static constexpr bool value = true; };
template< typename T > struct is_complex : public is_complex_base< typename std::remove_cv_t< std::remove_reference_t< T >>> {};

template< typename T > struct is_binary_base{ static constexpr bool value = false; };
template<> struct is_binary_base< bin >{ static constexpr bool value = true; };
template< typename T > struct is_binary : public is_binary_base< typename std::remove_cv_t< std::remove_reference_t< T >>> {};

// Clamp `value` on the lower side to `limit`. `value` is either integer or float, `limit` is always an integer.
//
// Logic:
// value is unsigned integer ---> OK
// value is signed integer ---> limit is unsigned integer ---> CLAMP
//                         \--> limit is signed integer ---> limit has fewer digits: CLAMP
//                                                      \--> otherwise: OK
// value is float ---> CLAMP

template< typename ValueType, typename LimitType >
struct NeedsLowerClamping {
   static constexpr bool value =
         is_floating_point< ValueType >::value // `value is a float
         || ( is_signed_integer< ValueType >::value // `value` is a signed integer
              && ( is_unsigned_integer< LimitType >::value // `limit` is an unsigned integer
                   || ( std::numeric_limits< ValueType >::digits > std::numeric_limits< LimitType >::digits ) // `limit` is a signed integer with fewer digits
                 )
            )
   ;
};

template< typename ValueType, typename LimitType, typename std::enable_if_t< NeedsLowerClamping< ValueType, LimitType >::value, int > = 0 >
constexpr inline const ValueType clamp_lower( ValueType value, LimitType limit ) {
   // `value` is a float or a signed integer with more digits than limit. Casting `limit` to the type of `value` is not a problem.
   return std::max( value, static_cast< ValueType >( limit ));
}
template< typename ValueType, typename LimitType, typename std::enable_if_t< !NeedsLowerClamping< ValueType, LimitType >::value, int > = 0 >
constexpr inline const ValueType clamp_lower( ValueType value, LimitType ) {
   // `value` is a signed integer with same or fewer digits than `limit`
   return value;
}

// Clamp 'value' on the upper side to `limit`. `value` is either integer or float, `limit` is always an integer.
//
// Logic:
// value is integer ---> limit has same or more digits: OK
//                       \--> otherwise: CLAMP
// value is float ---> CLAMP

template< typename ValueType, typename LimitType >
struct NeedsUpperClamping {
   static constexpr bool value =
         is_floating_point< ValueType >::value // `value is a float
         || ( std::numeric_limits< ValueType >::digits > std::numeric_limits< LimitType >::digits );
};

template< typename ValueType, typename LimitType, typename std::enable_if_t< NeedsUpperClamping< ValueType, LimitType >::value, int > = 0 >
constexpr inline const ValueType clamp_upper( ValueType value, LimitType limit ) {
   return std::min( value, static_cast< ValueType >( limit ));
}
template< typename ValueType, typename LimitType, typename std::enable_if_t< !NeedsUpperClamping< ValueType, LimitType >::value, int > = 0 >
constexpr inline const ValueType clamp_upper( ValueType value, LimitType ) {
   return value;
}

} // namespace detail

// Cast non-complex value to float
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< detail::is_floating_point< TargetType >::value, int > = 0 >
constexpr inline const TargetType clamp_cast( SourceType v ) {
   return static_cast< TargetType >( v );
}

// Cast non-complex value to complex
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< detail::is_complex< TargetType >::value, int > = 0 >
constexpr inline const TargetType clamp_cast( SourceType v ) {
   return static_cast< TargetType >( static_cast< typename TargetType::value_type >( v ));
}

// Cast non-complex value to integer
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< detail::is_integer< TargetType >::value, int > = 0 >
constexpr inline const TargetType clamp_cast( SourceType v ) {
   static_assert( std::numeric_limits< TargetType >::is_specialized, "It looks like std::numeric_limits is not specialized for the target type." );
   static_assert( std::numeric_limits< SourceType >::is_specialized, "It looks like std::numeric_limits is not specialized for the source type." );
   return static_cast< TargetType >(
         detail::clamp_upper< SourceType, TargetType >(
               detail::clamp_lower< SourceType, TargetType >(
                     v,
                     std::numeric_limits< TargetType >::lowest() ),
               std::numeric_limits< TargetType >::max() ));
}

// Cast non-complex value to bin
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< detail::is_binary< TargetType >::value, int > = 0 >
constexpr inline const TargetType clamp_cast( SourceType v ) {
   return static_cast< TargetType >( v ); // The logic is built into the `dip::bin` class
}

// Cast bin value to anything (except to complex, that's handled below)
template< typename TargetType,
          typename std::enable_if_t< !detail::is_complex< TargetType >::value, int > = 0 >
constexpr inline TargetType clamp_cast( dip::bin v ) {
   return static_cast< TargetType >( v );
}

// Casting from complex to non-complex, we take the absolute value and cast as if from a float
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< !detail::is_complex< TargetType >::value, int > = 0 >
constexpr inline TargetType clamp_cast( std::complex< SourceType > v ) {
   return clamp_cast< TargetType >( std::abs( v ));
}

// Casting from complex to complex
template< typename TargetType, typename SourceType,
          typename std::enable_if_t< detail::is_complex< TargetType >::value, int > = 0 >
constexpr inline TargetType clamp_cast( std::complex< SourceType > v ) {
   return { static_cast< typename TargetType::value_type >( v.real() ), static_cast< typename TargetType::value_type >( v.imag() ) };
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
