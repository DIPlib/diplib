/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_TYPES_H
#define DIP_TYPES_H

#include <cstdlib>   // std::malloc, std::realloc, std::free
#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc.
#include <complex>
#include <vector>
#include <bitset>

#include "dip_dimensionarray.h"


/// \file
/// Defines the basic types used throughout the library. This file is always included through diplib.h.

//TODO: We need to document the macros defined here under the file diplib.h. How do we do this?


namespace dip {


//
// Integer types for image properties, pixel coordinates, loop indices, etc.
//
// NOTE: `uint` is defined somewhere in some header, so *always* refer to it
// as `dip::uint`, everywhere in the DIPlib code base!
// For consistency, we also use `dip::sint` everywhere we refer to `sint`.
//
// TODO: It might be better to always used signed integer types everywhere.
//       uint could lead to difficult to catch errors in loops, uint ii<0 is
//       always false. I started with the uint because the standard library
//       uses it for sizes of arrays, and sizeof() is unsigned also. Maybe
//       better to cast these to sint?
typedef std::ptrdiff_t sint;  ///< An integer type to be used for strides and similar measures.
typedef std::size_t    uint;  ///< An integer type to be used for sizes and the like.
            // ptrdiff_t and size_t are signed and unsigned integers of the same length as
            // pointers: 32 bits on 32-bit systems, 64 bits on 64-bit systems.


//
// Types for pixel values
//
typedef std::uint8_t          uint8;      ///< Type for samples in an 8-bit unsigned integer image; also to be used as single byte for pointer arithmetic
typedef std::uint16_t         uint16;     ///< Type for samples in a 16-bit unsigned integer image
typedef std::uint32_t         uint32;     ///< Type for samples in a 32-bit unsigned integer image
typedef std::int8_t           sint8;      ///< Type for samples in an 8-bit signed integer image
typedef std::int16_t          sint16;     ///< Type for samples in a 16-bit signed integer image
typedef std::int32_t          sint32;     ///< Type for samples in a 32-bit signed integer image
typedef float                 sfloat;     ///< Type for samples in a 32-bit floating point (single-precision) image
typedef double                dfloat;     ///< Type for samples in a 64-bit floating point (double-precision) image
typedef std::complex<sfloat>  scomplex;   ///< Type for samples in a 64-bit complex-valued (single-precision) image
typedef std::complex<dfloat>  dcomplex;   ///< Type for samples in a 128-bit complex-valued (double-precision) image
struct bin {
   uint8 v_;                              // Binary data stored in a single byte (don't use bool for pixels,
                                          // it has implementation-defined size).
                                          // We define this struct for binary data so that we can overload
                                          // functions differently for bin and for uint8.
   // Overload constructors to make sure we always write 0 or 1 in the bin.
   bin() : v_( 0 ) {};                    ///< The default value is 0 (false)
   bin( bool v ) : v_( v ) {};            ///< A bool implicitly converts to bin
   template< typename T >
   bin( T v ) : v_( !!v ) {};             ///< Any arithmetic type implicitly converts to bin by comparing to zero
   template< typename T >
   bin( std::complex<T> v ) : v_( !!std::abs( v ) ) {};  ///< A complex value implicity converts to bin by comparing the absolute value to zero
   operator bool() const { return v_; }   ///< Implicitly converts to bool
   // TODO: we probably need to overload some arithmetic operators here? Or is that taken care of by the implicit conversions to bool?
};                                        ///< Type for samples in a binary image. Can store 0 or 1. Ocupies 1 byte.

// if 8 bits is not a byte...
static_assert( sizeof(dip::uint8)==1, "8 bits is not a byte in your system!" );
// Seriously, though. We rely on this property, and there is no guarantee
// that a system actually has 8 bits in a byte. Maybe we should use char
// (which is guaranteed to be size 1) for generic pointer arithmetic?

static_assert( sizeof(dip::bin)==1, "The binary type is not a single byte!" );


//
// Array types
//

// TODO: It's a little confusing that these arrays and others like StringArray or ImageArray work differently.
typedef DimensionArray<dip::sint>      IntegerArray;   ///< An array to hold strides, filter sizes, etc.
typedef DimensionArray<dip::uint>      UnsignedArray;  ///< An array to hold dimensions, dimension lists, etc.
typedef DimensionArray<dip::dfloat>    FloatArray;     ///< An array to hold filter parameters.
typedef DimensionArray<bool>           BooleanArray;   ///< An array used as a dimension selector.
   // IntegerArray A;
   // IntegerArray A(n);
   // IntegerArray A(n,0);
   // IntegerArray A {10,20,5};
   // A = ...;
   // A.size();
   // A[ii];
   // A.data();
   // A.resize(n);
   // A.resize(n,0);
   // A == B;


//
// Strings, used for parameters and other things
//

typedef std::string String;               ///< A string type
typedef std::vector<String> StringArray;  ///< An array of strings


//
// Ranges, used for indexing
//

/// Used in indexing to indicate start, stop and step. Negative start
/// and stop values indicate offset from the end (-1 is the last pixel,
/// -2 the second to last, etc.). If the stop comes before the start,
/// the step is assumed to be negative. No sign is stored for the step.
/// If stop cannot be reached with the given step size, the last pixel
/// in the range will come earlier. That is, stop is never exceeded.
struct Range {
   dip::sint start;    ///< First index included in range
   dip::sint stop;     ///< Last index included in range
   dip::uint step;     ///< Step size when going from start to stop

   /// Create a range that indicates all pixels
   Range() : start{0}, stop{-1}, step{1} {}
   /// Create a range that indicates a single pixel
   Range(dip::sint i) : start{i}, stop{i}, step{1} {}
   /// Create a range that indicates all pixels between `i` and `j`
   Range(dip::sint i, dip::sint j) : start{i}, stop{j}, step{1} {}
   /// Create a range with all thee values set
   Range(dip::sint i, dip::sint j, dip::uint s) : start{i}, stop{j}, step{s} {}

   /// Modify a range so that negative values are assigned correct
   /// values according to the given size; throws if the range falls
   /// out of bounds.
   void Fix( dip::uint size ) {
      // Check step is non-zero
      dip_ThrowIf( step==0, E::PARAMETER_OUT_OF_RANGE );
      // Compute indices from end
      if( start<0 ) start += size;
      if( stop <0 ) stop  += size;
      // Check start and stop are within range
      dip_ThrowIf( (start<0)||(start>=size)||(stop<0)||(stop>=size),
               E::INDEX_OUT_OF_RANGE );
      // Compute stop given start and step
      //stop = start + ((stop-start)/step)*step;
   }

   /// Get the number of pixels addressed by the range (must be fixed first!).
   dip::uint Size() const {
      if( start > stop )
         return 1 + (start-stop)/step;
      else
         return 1 + (stop-start)/step;
   }

   /// Get the offset for the range (must be fixed first!).
   dip::uint Offset() const {
      return start;
   }

   /// Get the signed step size for the range (must be fixed first!).
   dip::sint Step() const {
      if( start > stop )
         return -step;
      else
         return step;
   }

};

typedef DimensionArray<Range> RangeArray;  ///< An array of ranges


//
// The following is support for defining an options type, where the user can
// specify multiple options to pass on to a function or class. The class should
// not be used directly, only through the macros defined below it.
//

template<typename E, std::size_t N>
class Options {
   std::bitset<N> values;
   public:
   constexpr Options<E,N>() {}
   constexpr Options<E,N>(dip::uint n) : values {1ULL << n} {}
   bool operator== (const Options<E,N>& other) const { return (values & other.values).any(); }
   bool operator!= (const Options<E,N>& other) const { return !(values & other.values).any(); }
   Options<E,N> operator+ (Options<E,N> other) const { other.values |= values; return other; }
};

/// Declare a type used to pass options to a function or class. This macro is used
/// as follows:
///
///        DIP_DECLARE_OPTIONS(MyOptions, 3);
///        DIP_DEFINE_OPTION(MyOptions, Option_clean, 0);
///        DIP_DEFINE_OPTION(MyOptions, Option_fresh, 1);
///        DIP_DEFINE_OPTION(MyOptions, Option_shine, 2);
///
/// `MyOptions` will be a type that has three non-exclusive flags. Each of the
/// three DIP_DEFINE_OPTION commands defines a `constexpr` variable for the
/// given flag. These values can be combined using the `+` operator.
/// A variable of type `MyOptions` can be tested using the `==` and `!=`
/// operators, which return a `bool`:
///
///        MyOptions opts = {};                    // No options are set
///        opts = Option_fresh;                    // Set only one option.
///        opts = Option_clean + Option_shine;     // Set only these two options.
///        if (opts == Option_clean) {...}         // Test to see if `Option_clean` is set.
#define DIP_DECLARE_OPTIONS(name, number) class __##name; typedef dip::Options<__##name,number> name;

/// Use in conjunction with DIP_DECLARE_OPTIONS.
#define DIP_DEFINE_OPTION(name, value, index) constexpr name value { index };


//
// The following are some types for often-used parameters
//

/// Enumerated options are defined in the namespace dip::Option, unless they
/// are specific to some other sub-namespace.
namespace Option {

/// Some functions that check for a condition optionally throw an exception
/// if that condition is not met.
enum class ThrowException {
   doNotThrow, ///< Do not throw and exception, return false if the condition is not met.
   doThrow     ///< Throw an exception if the condition is not met.
};

} // namespace Option
} // namespace dip

#endif // DIP_TYPES_H
