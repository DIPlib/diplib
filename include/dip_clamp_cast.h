/*
 * DIPlib 3.0
 * This file contains overloaded definitions for the function dip::clamp_cast<>().
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_CLAMP_CAST_H
#define DIP_CLAMP_CAST_H

#include "dip_types.h"
#include <limits>
//#include <algorithm> // defines std::clamp in C++17

namespace dip {

// A replacement for std::clamp that will be available in C++17.
template< typename T >
constexpr inline const T& clamp( const T& v, const T& lo, const T& hi ) {
    return std::min( std::max( v, lo ), hi );
}

// Basis of dip::clamp_cast<>
template< typename T, typename S >
constexpr inline const T clamp_both( const S& v ) {
    return static_cast< T >( clamp( v,
                                    static_cast< S >( std::numeric_limits< T >::lowest() ),
                                    static_cast< S >( std::numeric_limits< T >::max()    ) ));
}
template< typename T, typename S >
constexpr inline const T clamp_lower( const S& v ) {   // T is an unsigned integer type with same or more bits than S
   return static_cast< T >( std::max( v, static_cast< S >( 0 ) ) );
}
template< typename T, typename S >
constexpr inline const T clamp_upper( const S& v ) {   // S is an unsigned integer type with fewer bits than T
    return static_cast< T >( std::min( v, static_cast< S >( std::numeric_limits< T >::max() ) ) );

}

/*
// Another version, instead of clamp_both, clamp_lower and clamp_upper.
// The compiler will optimize away unnecessary tests.
// Not used because casting the limit of T to S might cause overflow if S is
// narrower. How do we test for that case? In the code we use now we've
// manually written out most cases. It looks stupid but it works.
constexpr inline const T clamp_cast_basis( const S& v ) {
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

// Casting from dip::bin is always OK, except when casting to complex values
template< typename T >
inline T clamp_cast( dip::bin v ) {
   return T( v );
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::bin v ) {
   return { static_cast< dip::sfloat >( v ), 0.0f };
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::bin v ) {
   return { static_cast< dip::dfloat >( v ), 0.0 };
}

// Casting from dip::uint8, specialize for dip::sint8
template< typename T >
inline T clamp_cast( dip::uint8 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint8 v ) {
   return clamp_upper< dip::sint8, dip::uint8 >( v );
}

// Casting from a dip::uint16, specialize for dip::uint8, dip::sint8 and dip::sint16
template< typename T >
inline T clamp_cast( dip::uint16 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::uint8 clamp_cast< dip::uint8 >( dip::uint16 v ) {
   return clamp_upper< dip::uint8, dip::uint16 >( v );
}
template<>
inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint16 v ) {
   return clamp_upper< dip::sint8, dip::uint16 >( v );
}
template<>
inline dip::sint16 clamp_cast< dip::sint16 >( dip::uint16 v ) {
   return clamp_upper< dip::sint16, dip::uint16 >( v );
}

// Casting from a dip::uint32, specialize for dip::uint8, dip::uint16 and all signed types
template< typename T >
inline T clamp_cast( dip::uint32 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::uint8 clamp_cast< dip::uint8 >( dip::uint32 v ) {
   return clamp_upper< dip::uint8, dip::uint32 >( v );
}
template<>
inline dip::uint16 clamp_cast< dip::uint16 >( dip::uint32 v ) {
   return clamp_upper< dip::uint16, dip::uint32 >( v );
}
template<>
inline dip::sint8 clamp_cast< dip::sint8 >( dip::uint32 v ) {
   return clamp_upper< dip::sint8, dip::uint32 >( v );
}
template<>
inline dip::sint16 clamp_cast< dip::sint16 >( dip::uint32 v ) {
   return clamp_upper< dip::sint16, dip::uint32 >( v );
}
template<>
inline dip::sint32 clamp_cast< dip::sint32 >( dip::uint32 v ) {
   return clamp_upper< dip::sint32, dip::uint32 >( v );
}

// Casting from dip::sint8, specialize for all unsigned types
template< typename T >
inline T clamp_cast( dip::sint8 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint8 v ) {
   return clamp_lower< dip::uint8, dip::sint8 >( v );
}
template<>
inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint8 v ) {
   return clamp_lower< dip::uint16, dip::sint8 >( v );
}
template<>
inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint8 v ) {
   return clamp_lower< dip::uint32, dip::sint8 >( v );
}

// Casting from a dip::sint16, specialize for dip::sint8 and all unsigned types
template< typename T >
inline T clamp_cast( dip::sint16 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::sint8 clamp_cast< dip::sint8 >( dip::sint16 v ) {
   return clamp_both< dip::sint8, dip::sint16 >( v );
}
template<>
inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint16 v ) {
   return clamp_both< dip::uint8, dip::sint16 >( v );
}
template<>
inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint16 v ) {
   return clamp_lower< dip::uint16, dip::sint16 >( v );
}
template<>
inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint16 v ) {
   return clamp_lower< dip::uint32, dip::sint16 >( v );
}

// Casting from a dip::sint32, specialize for dip::sint8, dip::sint16 and all unsigned types
template< typename T >
inline T clamp_cast( dip::sint32 v ) {
   return static_cast< T >( v );
}
template<>
inline dip::sint8 clamp_cast< dip::sint8 >( dip::sint32 v ) {
   return clamp_both< dip::sint8, dip::sint32 >( v );
}
template<>
inline dip::sint16 clamp_cast< dip::sint16 >( dip::sint32 v ) {
   return clamp_both< dip::sint16, dip::sint32 >( v );
}
template<>
inline dip::uint8 clamp_cast< dip::uint8 >( dip::sint32 v ) {
   return clamp_both< dip::uint8, dip::sint32 >( v );
}
template<>
inline dip::uint16 clamp_cast< dip::uint16 >( dip::sint32 v ) {
   return clamp_both< dip::uint16, dip::sint32 >( v );
}
template<>
inline dip::uint32 clamp_cast< dip::uint32 >( dip::sint32 v ) {
   return clamp_lower< dip::uint32, dip::sint32 >( v );
}

// Casting from a dip::sint, we don't do checks if casting to a float, complex or bin
template< typename T >
inline T clamp_cast( dip::sint v ) {
   return clamp_both< T, dip::sint >( v );
}
template<>
inline dip::sfloat clamp_cast< dip::sfloat >( dip::sint v ) {
   return static_cast< dip::sfloat >( v );
}
template<>
inline dip::dfloat clamp_cast< dip::dfloat >( dip::sint v ) {
   return static_cast< dip::dfloat >( v );
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::sint v ) {
   return static_cast< dip::scomplex >( v );
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::sint v ) {
   return static_cast< dip::dcomplex >( v );
}
template<>
inline dip::bin clamp_cast< dip::bin >( dip::sint v ) {
   return static_cast< dip::bin >( v );
}

// Casting from a float, we don't do checks if casting to a float, complex or bin
template< typename T >
inline T clamp_cast( dip::sfloat v ) {
   return clamp_both< T, dip::sfloat >( v );
}
template<>
inline dip::sfloat clamp_cast< dip::sfloat >( dip::sfloat v ) {
   return v;
}
template<>
inline dip::dfloat clamp_cast< dip::dfloat >( dip::sfloat v ) {
   return static_cast< dip::dfloat >( v );
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::sfloat v ) {
   return static_cast< dip::scomplex >( v );
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::sfloat v ) {
   return static_cast< dip::dcomplex >( v );
}
template<>
inline dip::bin clamp_cast< dip::bin >( dip::sfloat v ) {
   return static_cast< dip::bin >( v );
}

// Casting from a float, we don't do checks if casting to a float, complex or bin
template< typename T >
inline T clamp_cast( dip::dfloat v ) {
   return clamp_both< T, dip::dfloat >( v );
}
template<>
inline dip::dfloat clamp_cast< dip::dfloat >( dip::dfloat v ) {
   return v;
}
template<>
inline dip::sfloat clamp_cast< dip::sfloat >( dip::dfloat v ) {
   return static_cast< dip::sfloat >( v );
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::dfloat v ) {
   return static_cast< dip::scomplex >( v );
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::dfloat v ) {
   return static_cast< dip::dcomplex >( v );
}
template<>
inline dip::bin clamp_cast< dip::bin >( dip::dfloat v ) {
   return static_cast< dip::bin >( v );
}

// Casting from a complex, we take the absolute value and cast as if from a float
template< typename T >
inline T clamp_cast( dip::scomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::scomplex v ) {
   return v;
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::scomplex v ) {
   return static_cast< dip::dcomplex >( v );
}

// Casting from a complex, we take the absolute value and cast as if from a float
template< typename T >
inline T clamp_cast( dip::dcomplex v ) {
   return clamp_cast< T >( std::abs( v ) );
}
template<>
inline dip::dcomplex clamp_cast< dip::dcomplex >( dip::dcomplex v ) {
   return v;
}
template<>
inline dip::scomplex clamp_cast< dip::scomplex >( dip::dcomplex v ) {
   return static_cast< dip::scomplex >( v );
}


} // namespace dip

#endif // DIP_CLAMP_CAST_H
