/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_TYPES_H
#define DIP_TYPES_H

#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc.
#include <complex>   // std::complex
#include <vector>    // std::vector
#include <bitset>    // std::bitset

namespace dip {


//
// Integer types for image properties, pixel coordinates, loop indices, etc.
//
// NOTE: `uint` is defined somewhere in some header, so *always* refer to it
// as `dip::uint`, everywhere in the DIPlib code base!
// For consistency, we also use `dip::sint` everywhere we refer to `sint`.
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
static_assert( sizeof(dip::uint8)==1, "8 bits is not a byte in your system!" );
// Seriously, though. We rely on this property, and there is no guarantee
// that a system actually has 8 bits in a byte. Maybe we should use char
// (which is guaranteed to be size 1) for generic pointer arithmetic?


//
// Arrays of signed, unsigned and floating-point values.
//    It's probably worth while to create our own short-vector optimized version of std::vector
//    since these represent dimensions, and we usually only have two or three of those.
//

template<typename T>
using DimensionArray = std::vector<T>; // We define this to indicate which arrays are the ones that would benefit from the short-vector optimization.

typedef DimensionArray<dip::sint>      IntegerArray;   ///< An array to hold strides, filter sizes, etc.
typedef DimensionArray<dip::uint>      UnsignedArray;  ///< An array to hold dimensions, dimension lists, etc.
typedef DimensionArray<dip::dfloat>    FloatArray;     ///< An array to hold filter parameters.
typedef DimensionArray<dip::dcomplex>  ComplexArray;   //   (used in only one obscure function in the old DIPlib.
typedef DimensionArray<bool>           BooleanArray;   ///< An array used as a dimension selector.
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



//
// The following is support for defining an options type, where the user can
// specify multiple options to pass on to a function or class. The class should
// not be used directly, only through the macros defined below it
//

template<typename E, std::size_t N>
class Options {
   std::bitset<N> values;
   public:
   constexpr Options<E,N>() {}
   constexpr Options<E,N>(dip::uint n) : values {1ULL << n} {}
   bool operator& (const Options<E,N> &other) const { return (values & other.values).any(); }
   Options<E,N> operator| (Options<E,N> other) const { other.values |= values; return other; }
};

/// Declare a type used to pass options to a function or class. This macro is used
/// as follows:
///
///        DIP_DECLARE_OPTIONS(MyOptions, 3);
///        DIP_DEFINE_OPTION(MyOptions, Option_clean, 0);
///        DIP_DEFINE_OPTION(MyOptions, Option_fresh, 1);
///        DIP_DEFINE_OPTION(MyOptions, Option_shine, 2);
///
/// `MyOptions` will by a type that has three non-exclusive flags. Each of the
/// three DIP_DEFINE_OPTION commands defines a `constexpr` variable for the
/// given flag. These values can be ORed together (using `|`). A variable of
/// type `MyOptions` can be tested using the AND operator (`&`), which returns
/// a `bool`:
///
///        MyOptions opts = {};                    // No options are set
///        opts = Option_fresh;                    // Set only one option.
///        opts = Option_clean | Option_shine;     // Set only these two options.
///        if (opts & Option_clean) {...}          // Test to see if `Option_clean` is set.
#define DIP_DECLARE_OPTIONS(name, number) class __##name; typedef dip::Options<__##name,number> name;

/// Use in conjunction with DIP_DECLARE_OPTIONS.
#define DIP_DEFINE_OPTION(name, value, index) constexpr name value { index };

// TODO: It would look nice to do (option1 & option2 & option3) for combining
// options, and testing with (options == option1). Though that might be
// counter-intuitive to those used to bit fields.

} // namespace dip

#endif // DIP_TYPES_H
