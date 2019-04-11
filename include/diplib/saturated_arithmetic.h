/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the functions dip::saturated_XXX().
 *
 * (c)2016, Cris Luengo.
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


//
// Addition
//

/// \brief Adds two values using saturated arithmetic.
// The base template is good for floats and complex.
template< typename T >
constexpr inline T saturated_add( T lhs, T rhs ) {
   return lhs + rhs;
}

// Unsigned integers overflow by giving a result that is smaller than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template<>
constexpr inline uint64 saturated_add( uint64 lhs, uint64 rhs ) {
   uint64 res = lhs + rhs;
   return res < lhs ? std::numeric_limits< uint64 >::max() : res;
}
template<>
constexpr inline uint32 saturated_add( uint32 lhs, uint32 rhs ) {
   uint32 res = lhs + rhs;
   return res < lhs ? std::numeric_limits< uint32 >::max() : res;
}
template<>
constexpr inline uint16 saturated_add( uint16 lhs, uint16 rhs ) {
   uint16 res = static_cast< uint16 >( lhs + rhs ); // there's an implicit conversion to `unsigned int`
   return res < lhs ? std::numeric_limits< uint16 >::max() : res;
}
template<>
constexpr inline uint8 saturated_add( uint8 lhs, uint8 rhs ) {
   uint8 res = static_cast< uint8 >( lhs + rhs ); // there's an implicit conversion to `unsigned int`
   return res < lhs ? std::numeric_limits< uint8 >::max() : res;
}

// Signed integers are more complex, we simply use a larger integer type to do the operation.
// TODO: overflow only happens if both operands have the same sign, opportunity for improvement?
#ifdef __SIZEOF_INT128__
template<>
constexpr inline sint64 saturated_add( sint64 lhs, sint64 rhs ) {
   return clamp_both< sint64 >( static_cast< __int128_t >( lhs ) + static_cast< __int128_t >( rhs ));
}
#else
// However, if we don't have a 128-bit integer type, we need to do this the hard way
template<>
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
template<>
constexpr inline sint32 saturated_add( sint32 lhs, sint32 rhs ) {
   return clamp_both< sint32 >( static_cast< sint64 >( lhs ) + static_cast< sint64 >( rhs ));
}
template<>
constexpr inline sint16 saturated_add( sint16 lhs, sint16 rhs ) {
   return clamp_both< sint16 >( static_cast< sint32 >( lhs ) + static_cast< sint32 >( rhs ));
}
template<>
constexpr inline sint8 saturated_add( sint8 lhs, sint8 rhs ) {
   return clamp_both< sint8 >( static_cast< sint32 >( lhs ) + static_cast< sint32 >( rhs ));
}

// Binary addition is equivalent to OR.
template<>
constexpr inline bin saturated_add( bin lhs, bin rhs ) {
   return lhs || rhs;
}


//
// Subtraction
//

/// \brief Subtracts two values using saturated arithmetic.
// The base template is good for floats and complex.
template< typename T >
constexpr inline T saturated_sub( T lhs, T rhs ) {
   return lhs - rhs;
}

// Unsigned integers underflow by giving a result that is larger than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template<>
constexpr inline uint64 saturated_sub( uint64 lhs, uint64 rhs ) {
   uint64 res = lhs - rhs;
   return res > lhs ? uint64( 0 ) : res;
}
template<>
constexpr inline uint32 saturated_sub( uint32 lhs, uint32 rhs ) {
   uint32 res = lhs - rhs;
   return res > lhs ? uint32( 0 ) : res;
}
template<>
constexpr inline uint16 saturated_sub( uint16 lhs, uint16 rhs ) {
   uint16 res = static_cast< uint16 >( lhs - rhs ); // there's an implicit conversion to `unsigned int`
   return res > lhs ? uint16( 0 ) : res;
}
template<>
constexpr inline uint8 saturated_sub( uint8 lhs, uint8 rhs ) {
   uint8 res = static_cast< uint8 >( lhs - rhs ); // there's an implicit conversion to `unsigned int`
   return res > lhs ? uint8( 0 ) : res;
}

// Signed integers are more complex, we simply use a larger integer type to do the operation.
// TODO: overflow only happens if both operands have the opposite sign, opportunity for improvement?
#ifdef __SIZEOF_INT128__
template<>
constexpr inline sint64 saturated_sub( sint64 lhs, sint64 rhs ) {
   return clamp_both< sint64 >( static_cast< __int128_t >( lhs ) - static_cast< __int128_t >( rhs ));
}
#else
// However, if we don't have a 128-bit integer type, we need to do this the hard way
template<>
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
template<>
constexpr inline sint32 saturated_sub( sint32 lhs, sint32 rhs ) {
   return clamp_both< sint32 >( static_cast< sint64 >( lhs ) - static_cast< sint64 >( rhs ));
}
template<>
constexpr inline sint16 saturated_sub( sint16 lhs, sint16 rhs ) {
   return clamp_both< sint16 >( static_cast< sint32 >( lhs ) - static_cast< sint32 >( rhs ));
}
template<>
constexpr inline sint8 saturated_sub( sint8 lhs, sint8 rhs ) {
   return clamp_both< sint8 >( static_cast< sint32 >( lhs ) - static_cast< sint32 >( rhs ));
}

// Binary subtraction is equivalent to AND NOT
template<>
constexpr inline bin saturated_sub( bin lhs, bin rhs ) {
   return lhs && !rhs;
}


//
// Multiplication
//

/// \brief Multiplies two values using saturated arithmetic.
// The base template is good for floats and complex.
template< typename T >
constexpr inline T saturated_mul( T lhs, T rhs ) {
   return lhs * rhs;
}

// For unsigned integers we simply use a larger integer type to do the operation.
#ifdef __SIZEOF_INT128__
template<>
constexpr inline uint64 saturated_mul( uint64 lhs, uint64 rhs ) {
   return clamp_both< uint64 >( static_cast< __uint128_t >( lhs ) * static_cast< __uint128_t >( rhs ));
}
#else
// However, if we don't have a 128-bit integer type, we need to do this the hard way
template<>
constexpr inline uint64 saturated_mul( uint64 lhs, uint64 rhs ) {
   uint64 result = lhs * rhs;
   if(( lhs != 0 ) && ( result / lhs != rhs )) {
      return std::numeric_limits< uint64 >::max();
   }
   return result;
}
#endif // __SIZEOF_INT128__
template<>
constexpr inline uint32 saturated_mul( uint32 lhs, uint32 rhs ) {
   return clamp_upper< uint32 >( static_cast< uint64 >( lhs ) * static_cast< uint64 >( rhs ));
}
template<>
constexpr inline uint16 saturated_mul( uint16 lhs, uint16 rhs ) {
   return clamp_upper< uint16 >( static_cast< uint32 >( lhs ) * static_cast< uint32 >( rhs ));
}
template<>
constexpr inline uint8 saturated_mul( uint8 lhs, uint8 rhs ) {
   return clamp_upper< uint8 >( static_cast< uint32 >( lhs ) * static_cast< uint32 >( rhs ));
}

// For signed integers we simply use a larger integer type to do the operation.
#ifdef __SIZEOF_INT128__
template<>
constexpr inline sint64 saturated_mul( sint64 lhs, sint64 rhs ) {
   return clamp_both< sint64 >( static_cast< __int128_t >( lhs ) * static_cast< __int128_t >( rhs ));
}
#else
// However, if we don't have a 128-bit integer type, we need to do this the hard way
template<>
constexpr inline sint64 saturated_mul( sint64 lhs, sint64 rhs ) {
   sint64 result = lhs * rhs;
   if(( lhs != 0 ) && ( result / lhs != rhs )) {
      return (( lhs < 0 ) ^ ( rhs < 0 )) ? std::numeric_limits< sint64 >::lowest() : std::numeric_limits< sint64 >::max();
   }
   return result;
}
#endif // __SIZEOF_INT128__
template<>
constexpr inline sint32 saturated_mul( sint32 lhs, sint32 rhs ) {
   return clamp_both< sint32 >( static_cast< sint64 >( lhs ) * static_cast< sint64 >( rhs ));
}
template<>
constexpr inline sint16 saturated_mul( sint16 lhs, sint16 rhs ) {
   return clamp_both< sint16 >( static_cast< sint32 >( lhs ) * static_cast< sint32 >( rhs ));
}
template<>
constexpr inline sint8 saturated_mul( sint8 lhs, sint8 rhs ) {
   return clamp_both< sint8 >( static_cast< sint32 >( lhs ) * static_cast< sint32 >( rhs ));
}

// Binary multiplication is equivalent to AND
template<>
constexpr inline bin saturated_mul( bin lhs, bin rhs ) {
   return lhs && rhs;
}


//
// Division
//

/// \brief Divides two values using saturated arithmetic.
template< typename T >
constexpr inline T saturated_div( T lhs, T rhs ) {
   return static_cast< T >( lhs / rhs ); // There's an implicit conversion to unsigned/int for smaller types
}

// Signed integer division can overflow if we divide INT_MIN by -1
template<>
constexpr inline sint64 saturated_div( sint64 lhs, sint64 rhs ) {
   return (( lhs == std::numeric_limits< sint64 >::lowest() ) && ( rhs == -1 ))
          ? std::numeric_limits< sint64 >::max() : ( lhs / rhs );
}
template<>
constexpr inline sint32 saturated_div( sint32 lhs, sint32 rhs ) {
   return (( lhs == std::numeric_limits< sint32 >::lowest() ) && ( rhs == -1 ))
          ? std::numeric_limits< sint32 >::max() : lhs / rhs;
}
template<>
constexpr inline sint16 saturated_div( sint16 lhs, sint16 rhs ) {
   return (( lhs == std::numeric_limits< sint16 >::lowest() ) && ( rhs == -1 ))
          ? std::numeric_limits< sint16 >::max() : static_cast< sint16 >( lhs / rhs ); // There's an implicit conversion to unsigned/int for smaller types
}
template<>
constexpr inline sint8 saturated_div( sint8 lhs, sint8 rhs ) {
   return (( lhs == std::numeric_limits< sint8 >::lowest() ) && ( rhs == -1 ))
          ? std::numeric_limits< sint8 >::max() : static_cast< sint8 >( lhs / rhs ); // There's an implicit conversion to unsigned/int for smaller types
}

// Binary division is equivalent to OR NOT (just to pick something... is this meaningful?).
template<>
constexpr inline bin saturated_div( bin lhs, bin rhs ) {
   return lhs || !rhs;
}

/// \brief Divides two values using saturated arithmetic. Tests for division
/// by zero, return 0 rather than infinity or NaN (or an exception).
template< typename T >
constexpr inline T saturated_safediv( T lhs, T rhs ) {
   return rhs == T( 0 ) ? T( 0 ) : saturated_div< T >( lhs, rhs );
}

// Binary division doesn't need the test, defer to saturated_div.
template<>
constexpr inline bin saturated_safediv( bin lhs, bin rhs ) {
   return saturated_div( lhs, rhs );
}


//
// Inversion
//

/// \brief Inverts a value using saturated arithmetic. This is the same as negation, but not for unsigned values.
// The base template is good for floats and complex.
template< typename T >
constexpr inline T saturated_inv( T v ) {
   return -v;
}

// Unsigned integers invert by subtracting from max value.
template<>
constexpr inline uint64 saturated_inv( uint64 v ) {
   return std::numeric_limits< uint64 >::max() - v;
}
template<>
constexpr inline uint32 saturated_inv( uint32 v ) {
   return std::numeric_limits< uint32 >::max() - v;
}
template<>
constexpr inline uint16 saturated_inv( uint16 v ) {
   return static_cast< uint16 >( static_cast< uint32 >(std::numeric_limits< uint16 >::max() ) - static_cast< uint32 >( v ));
}
template<>
constexpr inline uint8 saturated_inv( uint8 v ) {
   return static_cast< uint8 >( static_cast< uint32 >( std::numeric_limits< uint8 >::max() ) - static_cast< uint32 >( v ));
}

// Signed integers seem simple but overflow can happen if the value is equal to lowest possible value
template<>
constexpr inline sint64 saturated_inv( sint64 v ) {
   return v == std::numeric_limits< sint64 >::lowest() ? std::numeric_limits< sint64 >::max() : -v;
}
template<>
constexpr inline sint32 saturated_inv( sint32 v ) {
   return v == std::numeric_limits< sint32 >::lowest() ? std::numeric_limits< sint32 >::max() : -v;
}
template<>
constexpr inline sint16 saturated_inv( sint16 v ) {
   return v == std::numeric_limits< sint16 >::lowest() ? std::numeric_limits< sint16 >::max() : static_cast< sint16 >( -v );
}
template<>
constexpr inline sint8 saturated_inv( sint8 v ) {
   return v == std::numeric_limits< sint8 >::lowest() ? std::numeric_limits< sint8 >::max() : static_cast< sint8 >( -v );
}
// Binary inversion is equivalent to NOT
template<>
constexpr inline bin saturated_inv( bin v ) {
   return !v;
}

/// \}

} // namespace dip

#endif // DIP_SATURATED_ARITHMETIC_H
