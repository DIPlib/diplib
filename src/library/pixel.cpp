/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "dip_overload.h"
#include <typeinfo>

using namespace dip;

// Casting a Pixel to dcomplex.

template< typename TPI >
inline dcomplex CastValueComplex( void* p ) {
   return (dcomplex)*((TPI*)p);
}

Pixel::operator dcomplex() const{
   dcomplex x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueComplex, ( origin ), datatype );
   return x;
}

// Casting a Pixel to dfloat.

inline dfloat dip__todfloat( dfloat v ) { return v; }

template< typename T >
inline dfloat dip__todfloat( std::complex< T > v ) { return std::abs(v); }

template< typename TPI >
inline dfloat CastValueDouble( void* p ) {
   return dip__todfloat(*((TPI*)p));
}

Pixel::operator dfloat() const{
   dfloat x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueDouble, ( origin ), datatype );
   return x;
}

// Casting a Pixel to sint.
// I hope sfloat is converted to sint implicitly, rather than to scomplex.

inline sint dip__tosint( sint v )   { return v; }

template< typename T >
inline sint dip__tosint( std::complex< T > v ) { return (sint)std::abs(v); }

template< typename TPI >
inline sint CastValueInteger( void* p ) {
   return dip__tosint(*((TPI*)p));
}

Pixel::operator sint() const {
   sint x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueInteger, ( origin ), datatype );
   return x;
}
