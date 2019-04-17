/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the functions dip::saturated_XXX().
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

#ifndef DIP_SATURATED_ARITHMETIC_H
#define DIP_SATURATED_ARITHMETIC_H

#include "diplib/library/types.h"
#include "diplib/library/clamp_cast.h"


/// \file
/// \brief Defines templated functions for saturated arithmetic.
/// \see sample_operators


// NOTE: A different strategy is described in http://locklessinc.com/articles/sat_arithmetic/

// NOTE: GCC and Clang have some interesting compiler intrinsics:
//       https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
//       However, I'm struggling to find a way to use them that leads to better code than we have here.
//       In particular, for signed arithmetic one can overflow in two directions, and additional checks
//       are necessary to disambiguate.


namespace dip {


/// \addtogroup sample_operators
///
/// `dip::saturated_XXX` are templated functions for saturated arithmetic. Most *DIPlib* functions take care
/// of properly clamping the result of operations on pixels by using these functions to perform arithmetic.
/// For example,
///
/// ```cpp
///     10u - 20u == 4294967286u;
///     dip::saturated_sub(10u, 20u) == 0u;
/// ```
///
/// Saturated arithmetic is made available by including `diplib/saturated_arithmetic.h`.
/// \{


namespace detail {

template< typename T > struct LargerType { using type = void; };
template<> struct LargerType< uint8 > { using type = uint32; };
template<> struct LargerType< uint16 > { using type = uint32; };
template<> struct LargerType< uint32 > { using type = uint64; };
#ifdef __SIZEOF_INT128__
template<> struct LargerType< uint64 > { using type = __uint128_t; };
#endif

template<> struct LargerType< sint8 > { using type = sint32; };
template<> struct LargerType< sint16 > { using type = sint32; };
template<> struct LargerType< sint32 > { using type = sint64; };
#ifdef __SIZEOF_INT128__
template<> struct LargerType< sint64 > { using type = __int128_t; };
#endif

} // namespace detail


//
// Addition
//

/// \brief Adds two values using saturated arithmetic.

// Floats and complex don't overflow
template< typename T, typename std::enable_if_t< detail::is_floating_point< T >::value
                                              || detail::is_complex< T >::value, int > = 0 >
constexpr inline T saturated_add( T lhs, T rhs ) {
   return lhs + rhs;
}

// Unsigned integers overflow by giving a result that is smaller than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template< typename T, typename std::enable_if_t< detail::is_unsigned_integer< T >::value, int > = 0 >
constexpr inline T saturated_add( T lhs, T rhs ) {
   T res = static_cast< T >( lhs + rhs ); // There's an implicit conversion to unsigned/int for smaller types
   return res < lhs ? std::numeric_limits< T >::max() : res;
}

// Signed integers are more complex, we simply use a larger integer type to do the operation.
template< typename T, typename std::enable_if_t< detail::is_signed_integer< T >::value, int > = 0 >
constexpr inline T saturated_add( T lhs, T rhs ) {
   return clamp_cast< T >( static_cast< typename detail::LargerType< T >::type >( lhs )
                         + static_cast< typename detail::LargerType< T >::type >( rhs ));
}
#ifndef __SIZEOF_INT128__
// If we don't have a 128-bit integer type, we need to do this the hard way
constexpr inline sint64 saturated_add( sint64 lhs, sint64 rhs ) {
   if(( rhs > 0 ) && ( std::numeric_limits< sint64 >::max() - rhs <= lhs )) {
      return std::numeric_limits< sint64 >::max();
   }
   if(( rhs < 0 ) && ( std::numeric_limits< sint64 >::lowest() - rhs >= lhs )) {
      return std::numeric_limits< sint64 >::lowest();
   }
   return lhs + rhs;
}
#endif // __SIZEOF_INT128__

// Binary addition is equivalent to OR.
constexpr inline bin saturated_add( bin lhs, bin rhs ) {
   return lhs || rhs;
}


//
// Subtraction
//

/// \brief Subtracts two values using saturated arithmetic.

// Floats and complex don't overflow
template< typename T, typename std::enable_if_t< detail::is_floating_point< T >::value
                                              || detail::is_complex< T >::value, int > = 0 >
constexpr inline T saturated_sub( T lhs, T rhs ) {
   return lhs - rhs;
}

// Unsigned integers underflow by giving a result that is larger than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template< typename T, typename std::enable_if_t< detail::is_unsigned_integer< T >::value, int > = 0 >
constexpr inline T saturated_sub( T lhs, T rhs ) {
   T res = static_cast< T >( lhs - rhs ); // There's an implicit conversion to unsigned/int for smaller types
   return res > lhs ? T( 0 ) : res;
}

// Signed integers are more complex, we simply use a larger integer type to do the operation.
template< typename T, typename std::enable_if_t< detail::is_signed_integer< T >::value, int > = 0 >
constexpr inline T saturated_sub( T lhs, T rhs ) {
   return clamp_cast< T >( static_cast< typename detail::LargerType< T >::type >( lhs )
                         - static_cast< typename detail::LargerType< T >::type >( rhs ));
}
#ifndef __SIZEOF_INT128__
// If we don't have a 128-bit integer type, we need to do this the hard way
constexpr inline sint64 saturated_sub( sint64 lhs, sint64 rhs ) {
   if(( rhs < 0 ) && ( std::numeric_limits< sint64 >::max() + rhs <= lhs )) {
      return std::numeric_limits< sint64 >::max();
   }
   if(( rhs > 0 ) && ( std::numeric_limits< sint64 >::lowest() + rhs >= lhs )) {
      return std::numeric_limits< sint64 >::lowest();
   }
   return lhs - rhs;
}
#endif // __SIZEOF_INT128__

// Binary subtraction is equivalent to AND NOT
constexpr inline bin saturated_sub( bin lhs, bin rhs ) {
   return lhs && !rhs;
}


//
// Multiplication
//

/// \brief Multiplies two values using saturated arithmetic.

// Floats and complex don't overflow
template< typename T, typename std::enable_if_t< detail::is_floating_point< T >::value
                                              || detail::is_complex< T >::value, int > = 0 >
constexpr inline T saturated_mul( T lhs, T rhs ) {
   return lhs * rhs;
}

// For signed and unsigned integers we simply use a larger integer type to do the operation.
template< typename T, typename std::enable_if_t< detail::is_integer< T >::value, int > = 0 >
constexpr inline T saturated_mul( T lhs, T rhs ) {
   return clamp_cast< T >( static_cast< typename detail::LargerType< T >::type >( lhs )
                         * static_cast< typename detail::LargerType< T >::type >( rhs ));
}
#ifndef __SIZEOF_INT128__
// However, if we don't have a 128-bit integer type, we need to do this the hard way
constexpr inline uint64 saturated_mul( uint64 lhs, uint64 rhs ) {
   uint64 result = lhs * rhs;
   if(( lhs != 0 ) && ( result / lhs != rhs )) {
      return std::numeric_limits< uint64 >::max();
   }
   return result;
}

constexpr inline sint64 saturated_mul( sint64 lhs, sint64 rhs ) {
   sint64 result = lhs * rhs;
   if(( lhs != 0 ) && ( result / lhs != rhs )) {
      return (( lhs < 0 ) ^ ( rhs < 0 )) ? std::numeric_limits< sint64 >::lowest() : std::numeric_limits< sint64 >::max();
   }
   return result;
}
#endif // __SIZEOF_INT128__

// Binary multiplication is equivalent to AND
constexpr inline bin saturated_mul( bin lhs, bin rhs ) {
   return lhs && rhs;
}


//
// Division
//

/// \brief Divides two values using saturated arithmetic.

// Floats, complex and unsigned integers don't overflow
template< typename T, typename std::enable_if_t< detail::is_floating_point< T >::value
                                              || detail::is_complex< T >::value
                                              || detail::is_unsigned_integer< T >::value, int > = 0 >
constexpr inline T saturated_div( T lhs, T rhs ) {
   return static_cast< T >( lhs / rhs ); // There's an implicit conversion to unsigned/int for smaller types
}

// Signed integer division can overflow if we divide INT_MIN by -1
template< typename T, typename std::enable_if_t< detail::is_signed_integer< T >::value, int > = 0 >
constexpr inline T saturated_div( T lhs, T rhs ) {
   return (( lhs == std::numeric_limits< T >::lowest()) && ( rhs == -1 ))
          ? std::numeric_limits< T >::max() : static_cast< T >( lhs / rhs ); // There's an implicit conversion to unsigned/int for smaller types
}

// Binary division is equivalent to OR NOT (just to pick something... is this meaningful?).
constexpr inline bin saturated_div( bin lhs, bin rhs ) {
   return lhs || !rhs;
}

/// \brief Divides two values using saturated arithmetic. Tests for division
/// by zero, return 0 rather than infinity or NaN (or an exception).
template< typename T >
constexpr inline T saturated_safediv( T lhs, T rhs ) {
   return rhs == T( 0 ) ? T( 0 ) : saturated_div( lhs, rhs );
}

// Binary division doesn't need the test, defer to saturated_div.
constexpr inline bin saturated_safediv( bin lhs, bin rhs ) {
   return saturated_div( lhs, rhs );
}


//
// Inversion
//

/// \brief Inverts a value using saturated arithmetic. This is the same as negation, but not for unsigned values.

// Floats and complex are straight-forward
template< typename T, typename std::enable_if_t< detail::is_floating_point< T >::value
                                                 || detail::is_complex< T >::value, int > = 0 >
constexpr inline T saturated_inv( T v ) {
   return -v;
}

// Unsigned integers invert by subtracting from max value.
template< typename T, typename std::enable_if_t< detail::is_unsigned_integer< T >::value, int > = 0 >
constexpr inline T saturated_inv( T v ) {
   return static_cast< T >( std::numeric_limits< T >::max() - v ); // There's an implicit conversion to unsigned/int for smaller types
}

// Signed integers seem simple but overflow can happen if the value is equal to lowest possible value
template< typename T, typename std::enable_if_t< detail::is_signed_integer< T >::value, int > = 0 >
constexpr inline T saturated_inv( T v ) {
   return v == std::numeric_limits< T >::lowest() ? std::numeric_limits< T >::max() : static_cast< T >( -v ); // There's an implicit conversion to unsigned/int for smaller types
}

// Binary inversion is equivalent to NOT
constexpr inline bin saturated_inv( bin v ) {
   return !v;
}

/// \}

} // namespace dip

#endif // DIP_SATURATED_ARITHMETIC_H
