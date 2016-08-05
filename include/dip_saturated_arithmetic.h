/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the functions dip::saturated_XXX().
 *
 * (c)2016, Cris Luengo.
 */

#ifndef DIP_SATURATED_ARITHMETIC_H
#define DIP_SATURATED_ARITHMETIC_H

#include "dip_types.h"
#include "dip_clamp_cast.h"
#include <limits>

namespace dip {

// NOTE: A different strategy is described in http://locklessinc.com/articles/sat_arithmetic/

//
// Addition
//

// The base template is good for floats and complex.
template< typename T >
constexpr inline const T saturated_add( const T& lhs, const T& rhs ) {
   return lhs + rhs;
}
// Unsigned integers overflow by giving a result that is smaller than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template<>
constexpr inline const uint32 saturated_add( const uint32& lhs, const uint32& rhs ) {
   uint32 res = lhs + rhs;
   return res < lhs ? std::numeric_limits< uint32 >::max() : res;
}
template<>
constexpr inline const uint16 saturated_add( const uint16& lhs, const uint16& rhs ) {
   uint16 res = lhs + rhs;
   return res < lhs ? std::numeric_limits< uint16 >::max() : res;
}
template<>
constexpr inline const uint8 saturated_add( const uint8& lhs, const uint8& rhs ) {
   uint8 res = lhs + rhs;
   return res < lhs ? std::numeric_limits< uint8 >::max() : res;
}
// Signed integers are more complex, we simply use a larger integer type to do the operation.
// TODO: overflow only happens if both operands have the same sign, opportunity for improvement?
template<>
constexpr inline const sint32 saturated_add( const sint32& lhs, const sint32& rhs ) {
   return clamp_both< sint32, int64_t >( (int64_t)lhs + (int64_t)rhs );
}
template<>
constexpr inline const sint16 saturated_add( const sint16& lhs, const sint16& rhs ) {
   return clamp_both< sint16, sint32 >( (sint32)lhs + (sint32)rhs );
}
template<>
constexpr inline const sint8 saturated_add( const sint8& lhs, const sint8& rhs ) {
   return clamp_both< sint8, sint16 >( (sint16)lhs + (sint16)rhs );
}
// Binary addition is equivalent to OR.
template<>
constexpr inline const bin saturated_add( const bin& lhs, const bin& rhs ) {
   return lhs | rhs;
}


//
// Subtraction
//

// The base template is good for floats and complex.
template< typename T >
constexpr inline const T saturated_sub( const T& lhs, const T& rhs ) {
   return lhs - rhs;
}
// Unsigned integers underflow by giving a result that is larger than either operand.
// This code is supposed to be branchless, the compiler optimizes using a conditional move.
template<>
constexpr inline const uint32 saturated_sub( const uint32& lhs, const uint32& rhs ) {
   uint32 res = lhs - rhs;
   return res > lhs ? 0 : res;
}
template<>
constexpr inline const uint16 saturated_sub( const uint16& lhs, const uint16& rhs ) {
   uint16 res = lhs - rhs;
   return res > lhs ? 0 : res;
}
template<>
constexpr inline const uint8 saturated_sub( const uint8& lhs, const uint8& rhs ) {
   uint8 res = lhs - rhs;
   return res > lhs ? 0 : res;
}
// Signed integers are more complex, we simply use a larger integer type to do the operation.
// TODO: overflow only happens if both operands have the opposite sign, opportunity for improvement?
template<>
constexpr inline const sint32 saturated_sub( const sint32& lhs, const sint32& rhs ) {
   return clamp_both< sint32, int64_t >( (int64_t)lhs - (int64_t)rhs );
}
template<>
constexpr inline const sint16 saturated_sub( const sint16& lhs, const sint16& rhs ) {
   return clamp_both< sint16, sint32 >( (sint32)lhs - (sint32)rhs );
}
template<>
constexpr inline const sint8 saturated_sub( const sint8& lhs, const sint8& rhs ) {
   return clamp_both< sint8, sint16 >( (sint16)lhs - (sint16)rhs );
}
// Binary subtraction is equivalent to AND NOT
template<>
constexpr inline const bin saturated_sub( const bin& lhs, const bin& rhs ) {
   return lhs & !rhs;
}


//
// Multiplication
//

// The base template is good for floats and complex.
template< typename T >
constexpr inline const T saturated_mul( const T& lhs, const T& rhs ) {
   return lhs * rhs;
}
// For unsigned integers we simply use a larger integer type to do the operation.
template<>
constexpr inline const uint32 saturated_mul( const uint32& lhs, const uint32& rhs ) {
   return clamp_upper< uint32 >( (uint64_t)lhs * (uint64_t)rhs );
}
template<>
constexpr inline const uint16 saturated_mul( const uint16& lhs, const uint16& rhs ) {
   return clamp_upper< uint16 >( (uint32)lhs * (uint32)rhs );
}
template<>
constexpr inline const uint8 saturated_mul( const uint8& lhs, const uint8& rhs ) {
   return clamp_upper< uint8 >( (uint16)lhs * (uint16)rhs );
}
// For signed integers we simply use a larger integer type to do the operation.
template<>
constexpr inline const sint32 saturated_mul( const sint32& lhs, const sint32& rhs ) {
   return clamp_both< sint32, int64_t >( (int64_t)lhs * (int64_t)rhs );
}
template<>
constexpr inline const sint16 saturated_mul( const sint16& lhs, const sint16& rhs ) {
   return clamp_both< sint16, sint32 >( (sint32)lhs * (sint32)rhs );
}
template<>
constexpr inline const sint8 saturated_mul( const sint8& lhs, const sint8& rhs ) {
   return clamp_both< sint8, sint16 >( (sint16)lhs * (sint16)rhs );
}
// Binary multiplication is equivalent to AND
template<>
constexpr inline const bin saturated_mul( const bin& lhs, const bin& rhs ) {
   return lhs & rhs;
}


//
// Division
//

// Division never overflows. We let the system handle division by 0.
template< typename T >
constexpr inline const T saturated_div( const T& lhs, const T& rhs ) {
   return lhs / rhs;
}
// Binary division is equivalent to XOR (just to pick something... is this meaningful?).
template<>
constexpr inline const bin saturated_div( const bin& lhs, const bin& rhs ) {
   return lhs ^ rhs;
}


} // namespace dip

#endif // DIP_SATURATED_ARITHMETIC_H
