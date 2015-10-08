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

template< typename TPI >
inline dcomplex CastValueComplex( void* p ) {
   return (dcomplex)*((TPI*)p);
}

template< typename TPI >
inline dfloat CastValueDouble( void* p ) {
   return (dfloat)*((TPI*)p);
}
template<>
inline dfloat CastValueDouble< dcomplex >( void* p ) {
   return (dfloat)std::abs(*((dcomplex*)p));
}
template<>
inline dfloat CastValueDouble< scomplex >( void* p ) {
   return (dfloat)std::abs(*((scomplex*)p));
}

template< typename TPI >
inline sint CastValueInteger( void* p ) {
   return (sint)*((TPI*)p);
}
template<>
inline sint CastValueInteger< dcomplex >( void* p ) {
   return (sint)std::abs(*((dcomplex*)p));
}
template<>
inline sint CastValueInteger< scomplex >( void* p ) {
   return (sint)std::abs(*((scomplex*)p));
}

Pixel::operator dcomplex() const{
   dcomplex x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueComplex, ( origin ), datatype );
   return x;
}

Pixel::operator dfloat() const{
   dfloat x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueDouble, ( origin ), datatype );
   return x;
}

Pixel::operator sint() const {
   sint x;
   DIP_OVL_CALL_ASSIGN_ALL( x, CastValueInteger, ( origin ), datatype );
   return x;
}
