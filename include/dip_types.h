/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_TYPES_H
#define DIP_TYPES_H

#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc. (C++11 only!)
#include <complex>   // std::complex
#include <vector>    // std::vector

namespace dip {


//
// Integer types for image properties, pixel coordinates, loop indices, etc.
//
typedef std::ptrdiff_t sint;  ///< An integer type to be used for strides and similar measures.
typedef std::size_t    uint;  ///< An integer type to be used for sizes and the like.
            // ptrdiff_t and size_t are signed and unsigned integers of the same length as
            // pointers: 32 bits on 32-bit systems, 64 bits on 64-bit systems.


//
// Types for pixel values
//
typedef std::uint8_t          bin;        ///< Type for pixels in a binary image
            // Binary data stored in a single byte (don't use bool for pixels,
            // it has implementation-defined size)
typedef std::uint8_t          uint8;      ///< Type for pixels in an 8-bit unsigned integer image; also to be used as single byte for pointer arithmetic
typedef std::uint16_t         uint16;     ///< Type for pixels in a 16-bit unsigned integer image
typedef std::uint32_t         uint32;     ///< Type for pixels in a 32-bit unsigned integer image
typedef std::int8_t           sint8;      ///< Type for pixels in an 8-bit signed integer image
typedef std::int16_t          sint16;     ///< Type for pixels in a 16-bit signed integer image
typedef std::int32_t          sint32;     ///< Type for pixels in a 32-bit signed integer image
typedef float                 sfloat;     ///< Type for pixels in a 32-bit floating point (single-precision) image
typedef double                dfloat;     ///< Type for pixels in a 64-bit floating point (double-precision) image
typedef std::complex<sfloat>  scomplex;   ///< Type for pixels in a 64-bit complex-valued (single-precision) image
typedef std::complex<dfloat>  dcomplex;   ///< Type for pixels in a 128-bit complex-valued (double-precision) image

// if 8 bits is not a byte...
static_assert( sizeof(uint8)==1, "8 bits is not a byte in your system!" );
// Seriously, though. We rely on this property, and there is no guarantee
// that a system actually has 8 bits in a byte. Maybe we should use char
// (which is guaranteed to be size 1) for generic pointer arithmetic?


//
// Arrays of signed, unsigned and floating-point values.
//    It's probably worth while to create our own short-vector optimized version of std::vector
//    since these represent dimensions, and we usually only have two or three of those.
//

typedef std::vector<sint>     IntegerArray;   ///< An array to hold strides, filter sizes, etc.
typedef std::vector<uint>     UnsignedArray;  ///< An array to hold dimensions, dimension lists, etc.
typedef std::vector<dfloat>   FloatArray;     ///< An array to hold filter parameters.
typedef std::vector<dcomplex> ComplexArray;   //   (used in only one obscure function in the old DIPlib.
typedef std::vector<bool>     BooleanArray;   ///< An array used as a dimension selector.
   // IntegerArray A;
   // IntegerArray A(n);
   // IntegerArray A(n,0);
   // IntegerArray A {10,20,5};
   // A.assign(n,0);
   // A.assign({10,20,5});
   // A = ...;
   // A.size();
   // A[ii];
   // A.data();
   // A.resize(n);
   // A.resize(n,0);
   // A == B;


} // namespace dip

#endif // DIP_TYPES_H
