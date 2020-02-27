/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
 *
 * (c)2014-2019, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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


#ifndef DIP_TYPES_H
#define DIP_TYPES_H

#include <cstddef>   // std::size_t, std::ptrdiff_t
#include <cstdint>   // std::uint8_t, etc.
#include <complex>
#include <vector>
#include <string>
#include <set>
#include <cctype>
#include <climits>

#include "diplib/library/dimension_array.h"


/// \file
/// \brief Defines the basic types used throughout the library. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


//
// Integer types for image properties, pixel coordinates, loop indices, etc.
//
// NOTE: `uint` is defined somewhere in some header, so *always* refer to it
// as `dip::uint`, everywhere in the DIPlib code base!
// For consistency, we also use `dip::sint` everywhere we refer to `sint`.
//
// NOTE: It might be better to always use signed integer types everywhere.
//       uint could lead to difficult to catch errors in loops, uint ii<0 is
//       always false. I started with the uint because the standard library
//       uses it for sizes of arrays, and sizeof() is unsigned also. Maybe
//       better to cast these to sint?
using sint = std::ptrdiff_t;  ///< An integer type to be used for strides and similar measures.
using uint = std::size_t;  ///< An integer type to be used for sizes and the like.
// ptrdiff_t and size_t are signed and unsigned integers of the same length as
// pointers: 32 bits on 32-bit systems, 64 bits on 64-bit systems.
// NOTE: We don't allow any integer to be larger than the max value of ptrdiff_t.
//       This is tested for in a few places (at dip::Image creation and when forging)
//       but not everywhere.
constexpr dip::uint maxint = static_cast< dip::uint >( std::numeric_limits< dip::sint >::max() );


/// \}


/// \addtogroup types
/// \{


//
// Types for pixel values
//
class DIP_NO_EXPORT bin;
using uint8 = std::uint8_t;      ///< Type for samples in an 8-bit unsigned integer image; also to be used as single byte for pointer arithmetic
using uint16 = std::uint16_t;    ///< Type for samples in a 16-bit unsigned integer image
using uint32 = std::uint32_t;    ///< Type for samples in a 32-bit unsigned integer image
using uint64 = std::uint64_t;    ///< Type for samples in a 64-bit unsigned integer image
using sint8 = std::int8_t;       ///< Type for samples in an 8-bit signed integer image
using sint16 = std::int16_t;     ///< Type for samples in a 16-bit signed integer image
using sint32 = std::int32_t;     ///< Type for samples in a 32-bit signed integer image
using sint64 = std::int64_t;     ///< Type for samples in a 64-bit signed integer image
using sfloat = float;            ///< Type for samples in a 32-bit floating point (single-precision) image
using dfloat = double;           ///< Type for samples in a 64-bit floating point (double-precision) image
using scomplex = std::complex< sfloat >;   ///< Type for samples in a 64-bit complex-valued (single-precision) image
using dcomplex = std::complex< dfloat >;   ///< Type for samples in a 128-bit complex-valued (double-precision) image

using LabelType = uint32;        ///< Type currently used for all labeled images, see `dip::DT_LABEL`.

namespace detail {

template< typename T > struct IsSampleType { static constexpr bool value = false; };
template<> struct IsSampleType< bin > { static constexpr bool value = true; };
template<> struct IsSampleType< uint8 > { static constexpr bool value = true; };
template<> struct IsSampleType< uint16 > { static constexpr bool value = true; };
template<> struct IsSampleType< uint32 > { static constexpr bool value = true; };
template<> struct IsSampleType< uint64 > { static constexpr bool value = true; };
template<> struct IsSampleType< sint8 > { static constexpr bool value = true; };
template<> struct IsSampleType< sint16 > { static constexpr bool value = true; };
template<> struct IsSampleType< sint32 > { static constexpr bool value = true; };
template<> struct IsSampleType< sint64 > { static constexpr bool value = true; };
template<> struct IsSampleType< sfloat > { static constexpr bool value = true; };
template<> struct IsSampleType< dfloat > { static constexpr bool value = true; };
template<> struct IsSampleType< scomplex > { static constexpr bool value = true; };
template<> struct IsSampleType< dcomplex > { static constexpr bool value = true; };

template< typename T > struct IsNumericType { static constexpr bool value = std::is_arithmetic< T >::value; };
template<> struct IsNumericType< bin > { static constexpr bool value = true; };
template<> struct IsNumericType< scomplex > { static constexpr bool value = true; };
template<> struct IsNumericType< dcomplex > { static constexpr bool value = true; };

template< typename T > struct IsIndexingType { static constexpr bool value = false; };
template<> struct IsIndexingType< signed int > { static constexpr bool value = true; };
template<> struct IsIndexingType< unsigned int > { static constexpr bool value = true; };
#if SIZE_MAX != UINT_MAX // we don't want to compile the next two if `dip::uint == unsigned int`.
template<> struct IsIndexingType< dip::uint > { static constexpr bool value = true; };
template<> struct IsIndexingType< dip::sint > { static constexpr bool value = true; };
#endif
} // namespace detail

/// \brief For use with `std::enable_if` to enable templates only for types that are valid for image samples.
///
/// One example usage is as follows:
///
/// ```cpp
///     template< typename T, typename = std::enable_if_t< dip::IsSampleType< T >::value >>
///     void MyFunction( T value ) { ... }
/// ```
///
/// When defining different versions of the templated function for `IsSampleType< T >` and `!IsSampleType< T >`,
/// you'll need to use the following form:
///
/// ```cpp
///     template< typename T, typename std::enable_if_t< dip::IsSampleType< T >::value, int > = 0 >
///     void MyFunction( T value ) { ... }
///     template< typename T, typename std::enable_if_t< !dip::IsSampleType< T >::value, int > = 0 >
///     void MyFunction( T value ) { ... }
/// ```
template< typename T > struct IsSampleType : public detail::IsSampleType< typename std::remove_cv_t< std::remove_reference_t< T >>> {};

/// \brief For use with `std::enable_if` to enable templates only for types that are numeric types, similar to
/// `std::is_arithmetic` but also true for complex types. See `dip::IsSampleType` for usage details.
template< typename T > struct IsNumericType : public detail::IsNumericType< typename std::remove_cv_t< std::remove_reference_t< T >>> {};

/// \brief For use with `std::enable_if` to enable templates only for types that are indexing types, true for
/// signed and unsigned integers. See `dip::IsSampleType` for usage details.
template< typename T > struct IsIndexingType : public detail::IsIndexingType< typename std::remove_cv_t< std::remove_reference_t< T >>> {};

/// \brief A templated function to check for positive infinity, which works also for integer types (always returning false)
template< typename TPI, typename std::enable_if_t< !std::numeric_limits< TPI >::has_infinity, int > = 0 >
bool PixelIsInfinity( TPI /*value*/ ) {
   return false;
}
template< typename TPI, typename std::enable_if_t< std::numeric_limits< TPI >::has_infinity, int > = 0 >
bool PixelIsInfinity( TPI value ) {
   return value == std::numeric_limits< TPI >::infinity();
}

/// \brief A templated function to check for negative infinity, which works also for integer types (always returning false)
template< typename TPI, typename std::enable_if_t< !std::numeric_limits< TPI >::has_infinity, int > = 0 >
   bool PixelIsMinusInfinity( TPI /*value*/ ) {
   return false;
}
template< typename TPI, typename std::enable_if_t< std::numeric_limits< TPI >::has_infinity, int > = 0 >
bool PixelIsMinusInfinity( TPI value ) {
   return value == -std::numeric_limits< TPI >::infinity();
}

/// \brief Type for samples in a binary image. Can store 0 or 1. Occupies 1 byte.
class DIP_NO_EXPORT bin {
   // Binary data stored in a single byte (don't use bool for pixels, it has
   // implementation-defined size). We define this class for binary data so
   // that we can overload functions differently for bin and for uint8.
   public:

      // Default copy and move constructors, just in case the templated constructor overrides one of these.
      constexpr bin( bin const& ) = default;
      constexpr bin( bin&& ) = default;
      bin& operator=( bin const& ) = default;
      bin& operator=( bin&& ) = default;

      // Overload constructors to make sure we always write 0 or 1 in the bin.
      /// The default value is 0 (false)
      constexpr bin() = default;

      /// A bool implicitly converts to bin
      constexpr bin( bool v ) : v_( static_cast< uint8 >( v )) {}

      /// Any arithmetic type converts to bin by comparing to zero
      template< typename T, typename = std::enable_if_t< IsNumericType< T >::value >>
      constexpr explicit bin( T v ) : v_( static_cast< uint8 >( v != 0 )) {}

      /// A bin implicitly converts to bool
      constexpr operator bool() const { return v_ != 0; }

      /// Negation operator
      constexpr bin operator!() const { return v_ == 0; }

      /// Bit-wise negation is the same as regular negation
      constexpr bin operator~() const { return v_ == 0; }

      /// And operator, prefer to use this over `std::min`
      constexpr bin operator&( bin other ) const { return { ( v_ != 0 ) && ( other.v_ != 0 ) }; } // Why && and not & ? Short circuit.

      /// Or operator, prefer to use this over `std::max`
      constexpr bin operator|( bin other ) const { return { ( v_ != 0 ) || ( other.v_ != 0 ) }; } // Why || and not | ? Short circuit.

      /// Exclusive-or operator
      constexpr bin operator^( bin other ) const { return bin( v_ ^ other.v_ ); }

      /// And compound operator
      constexpr bin& operator&=( bin other ) { v_ &= other.v_; return *this; }

      /// Or compound operator
      constexpr bin& operator|=( bin other ) { v_ |= other.v_; return *this; }

      /// Exclusive-or compound operator
      constexpr bin& operator^=( bin other ) { v_ ^= other.v_; return *this; }

	  /// Equality operator
	  template< typename T >
	  constexpr bool operator==( T other ) const { return static_cast< bool >( v_ ) == static_cast< bool >( other ); }

	  /// Inequality operator
	  template< typename T >
	  constexpr bool operator!=( T other ) const { return static_cast< bool >( v_ ) != static_cast< bool >( other ); }

      /// Allow explicit casting to a reference to the underlying type (uint8&) for binary image operations
      explicit operator uint8&() { return v_; }

   private:
      uint8 v_ = 0;
};

/// \brief Writes the value as a `bool` to a stream.
/// \relates dip::bin
inline std::ostream& operator<<(
      std::ostream& os,
      bin const& v
) {
   os << static_cast< bool >( v );
   return os;
}

// if 8 bits is not a byte...
static_assert( sizeof( dip::uint8 ) == 1, "8 bits is not a byte in your system!" );
// Seriously, though. We rely on this property, and there is no guarantee
// that a system actually has 8 bits in a byte. Maybe we should use char
// (which is guaranteed to be size 1) for generic pointer arithmetic?

static_assert( sizeof( dip::bin ) == 1, "The binary type is not a single byte!" );


//
// Templates that help get the right types in template functions and template classes
//

namespace detail {
template< typename T > struct FloatTypeCalculator { using type = sfloat; };
template<> struct FloatTypeCalculator< uint32 > { using type = dfloat; };
template<> struct FloatTypeCalculator< sint32 > { using type = dfloat; };
template<> struct FloatTypeCalculator< uint64 > { using type = dfloat; };
template<> struct FloatTypeCalculator< sint64 > { using type = dfloat; };
template<> struct FloatTypeCalculator< dfloat > { using type = dfloat; };
template<> struct FloatTypeCalculator< dcomplex > { using type = dfloat; };
} // namespace detail
/// \brief The type to use in calculations when a floating-point type is needed. Matches `dip::DataType::SuggestFloat`.
template< typename T > using FloatType = typename detail::FloatTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct DoubleTypeCalculator { using type = dfloat; };
template<> struct DoubleTypeCalculator< scomplex > { using type = dcomplex; };
template<> struct DoubleTypeCalculator< dcomplex > { using type = dcomplex; };
} // namespace detail
/// \brief The double precision floating point type (real or complex) to use when computing large sums of any input type. Matches `dip::DataType::SuggestDouble`.
template< typename T > using DoubleType = typename detail::DoubleTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct ComplexTypeCalculator { using type = scomplex; };
template<> struct ComplexTypeCalculator< uint32 > { using type = dcomplex; };
template<> struct ComplexTypeCalculator< sint32 > { using type = dcomplex; };
template<> struct ComplexTypeCalculator< uint64 > { using type = dcomplex; };
template<> struct ComplexTypeCalculator< sint64 > { using type = dcomplex; };
template<> struct ComplexTypeCalculator< dfloat > { using type = dcomplex; };
template<> struct ComplexTypeCalculator< dcomplex > { using type = dcomplex; };
} // namespace detail
/// \brief The type to use in calculations when a complex type is needed. Matches `dip::DataType::SuggestComplex`.
template< typename T > using ComplexType = typename detail::ComplexTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct FlexTypeCalculator { using type = FloatType< T >; };
template<> struct FlexTypeCalculator< scomplex > { using type = scomplex; };
template<> struct FlexTypeCalculator< dcomplex > { using type = dcomplex; };
} // namespace detail
/// \brief The type to use in calculations. Matches `dip::DataType::SuggestFlex`.
template< typename T > using FlexType = typename detail::FlexTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct FlexBinTypeCalculator { using type = FlexType< T >; };
template<> struct FlexBinTypeCalculator< bin > { using type = bin; };
} // namespace detail
/// \brief The type to use in calculations. Matches `dip::DataType::SuggestFlexBin`.
template< typename T > using FlexBinType = typename detail::FlexBinTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct AbsTypeCalculator { using type = T; };
template<> struct AbsTypeCalculator< sint8 > { using type = uint8; };
template<> struct AbsTypeCalculator< sint16 > { using type = uint16; };
template<> struct AbsTypeCalculator< sint32 > { using type = uint32; };
template<> struct AbsTypeCalculator< sint64 > { using type = uint64; };
template<> struct AbsTypeCalculator< scomplex > { using type = sfloat; };
template<> struct AbsTypeCalculator< dcomplex > { using type = dfloat; };
} // namespace detail
/// \brief The type to use for the output of abs operations. Matches `dip::DataType::SuggestAbs`.
template< typename T > using AbsType = typename detail::AbsTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

namespace detail {
template< typename T > struct RealTypeCalculator { using type = T; };
template<> struct RealTypeCalculator< bin > { using type = uint8; };
template<> struct RealTypeCalculator< scomplex > { using type = sfloat; };
template<> struct RealTypeCalculator< dcomplex > { using type = dfloat; };
} // namespace detail
/// \brief The type to use in calculations when a real-valued type is needed. Matches `dip::DataType::SuggestReal`.
template< typename T > using RealType = typename detail::RealTypeCalculator< std::remove_cv_t< std::remove_reference_t< T >>>::type;

/// \}


/// \addtogroup infrastructure
/// \{


//
// Array types
//

using IntegerArray = DimensionArray< dip::sint >;   ///< An array to hold strides, filter sizes, etc.
using UnsignedArray = DimensionArray< dip::uint >;  ///< An array to hold dimensions, dimension lists, etc.
using FloatArray = DimensionArray< dip::dfloat >;   ///< An array to hold filter parameters.
using BooleanArray = DimensionArray< bool >;        ///< An array used as a dimension selector.

using CoordinateArray = std::vector< UnsignedArray >; ///< An array of pixel coordinates.
using FloatCoordinateArray = std::vector< FloatArray >; ///< An array of subpixel coordinates.


/// \brief Check the length of an array, and extend it if necessary and possible.
///
/// This function is used where a function's
/// input parameter is an array that is supposed to match the image dimensionality `nDims`. The user can give an
/// array of that length, or an array with a single value, which will be used for all dimensions, or an empty array,
/// in which case the default value `defaultValue` will be used for all dimensions.
template< typename T >
inline void ArrayUseParameter( DimensionArray< T >& array, dip::uint nDims, T defaultValue = {} ) {
   if( array.empty() ) {
      array.resize( nDims, defaultValue );
   } else if( array.size() == 1 ) {
      array.resize( nDims, array[ 0 ] );
   } else if( array.size() != nDims ) {
      DIP_THROW( E::ARRAY_PARAMETER_WRONG_LENGTH );
   }
}


//
// Strings, used for parameters and other things
//

using String = std::string;                 ///< A string, used to specify an option
using StringArray = std::vector< String >;  ///< An array of strings, used to specify an option per dimension
using StringSet = std::set< String >;       ///< A collection of strings, used to specify multiple independent options

/// \brief Translates a string input parameter that is meant as a boolean value.
inline bool BooleanFromString( String const& input, String const& trueString, String const& falseString ) {
   if( input == trueString ) {
      return true;
   }
   if( input == falseString ) {
      return false;
   }
   DIP_THROW_INVALID_FLAG( input );
}
// An overload so that we don't construct a `String` object from string literals
inline bool BooleanFromString( String const& input, String::value_type const* trueString, String::value_type const* falseString ) {
   if( input == trueString ) {
      return true;
   }
   if( input == falseString ) {
      return false;
   }
   DIP_THROW_INVALID_FLAG( input );
}

/// \brief A case-insensitive string comparison, use only with ASCII characters!
inline bool StringCompareCaseInsensitive( String const& string1, String const& string2 ) {
   if( string1.size() != string2.size() ) {
      return false;
   }
   for( auto it1 = string1.begin(), it2 = string2.begin(); it1 != string1.end(); ++it1, ++it2 ) {
      if( std::tolower( static_cast< unsigned char >( *it1 )) != std::tolower( static_cast< unsigned char >( *it2 ))) {
         return false;
      }
   }
   return true;
}

/// \brief Convert a string to lower case, use only with ASCII characters!
inline void ToLowerCase( String& string ) {
   for( auto& s : string ) {
      s = static_cast< char >( std::tolower( static_cast< unsigned char >( s )));
   }
}

/// \brief Convert a string to upper case, use only with ASCII characters!
inline void ToUpperCase( String& string ) {
   for( auto& s : string ) {
      s = static_cast< char >( std::toupper( static_cast< unsigned char >( s )));
   }
}


//
// Ranges, used for indexing
//

/// \brief Used in indexing to indicate a regular subset of pixels along one
/// image dimension.
///
/// `%dip::Range{ start, stop }` generates a range of pixels where
/// `start` and `stop` are the first and last indices in the range. That is,
/// `stop` is included in the range. `%dip::Range{ start }` generates a range
/// for a single pixel. For example, `%dip::Range{ 0 }` is the first pixel,
/// and is equivalent to `%dip::Range{ 0, 0 }`. `%dip::Range{ 0, N-1 }` is
/// a range of the first N pixels.
///
/// `%dip::Range{ start, stop, step }` generates a range of pixels where
/// `step` is the number of pixels between
/// subsequent indices. The pixels indexed are the ones generated by the
/// following loop:
///
/// ```cpp
///     offset = start;
///     do {
///        // use this offset
///        offset += step;
///     } while( offset <= stop );
/// ```
///
/// That is, it is possible that the range does not include `stop`, if the
/// `step` would make the range step over `stop`.
///
/// Negative `start` and `stop` values indicate offset from the end (-1 is the
/// last pixel, -2 the second to last, etc.): `%dip::Range{ 5, -6 }` indicates
/// a range that skips the first and last five pixels. `%dip::Range{ -1, -1, 1 }`
/// (or simply `%dip::Range{ -1 }` indicates the last pixel only.
///
/// `%dip::Range{ 0, -1 }` is equivalent to `%dip::Range{}`, and indicates all
/// pixels.
///
/// The `dip::Range::Fix` method converts the negative `start` and `stop` values
/// to actual offsets:
///
/// ```cpp
///     dip::Range r{ 5, -6 };
///     r.fix( 50 );
///     // now r.stop == 50 - 6
/// ```
///
/// If `stop` comes before `start`, then the range generates pixel indices in
/// the reverse order. That is, negative steps are taken to go from `start` to `stop`.
/// `step` is always a positive integer, the direction of the steps is given
/// solely by the ordering of `start` and `stop`.
///
/// The `begin` and `end` methods yield an iterator that dereferences to the indices
/// defined by the range.
struct DIP_NO_EXPORT Range {
   dip::sint start = 0;    ///< First index included in range
   dip::sint stop = -1;    ///< Last index included in range
   dip::uint step = 1;     ///< Step size when going from start to stop

   /// Create a range that indicates all pixels
   Range() = default;

   /// Create a range that indicates a single pixel
   explicit Range( dip::sint i ) : start{ i }, stop{ i }, step{ 1 } {}

   /// \brief Create a range using two or three values; it indicates all pixels between `i` and `j`, both inclusive.
   /// The step size defaults to 1.
   Range( dip::sint i, dip::sint j, dip::uint s = 1 ) : start{ i }, stop{ j }, step{ s } {}

   /// \brief Modify a range so that negative values are assigned correct
   /// values according to the given size. Throws if the range falls
   /// out of bounds.
   void Fix( dip::uint size ) {
      // Check step is non-zero
      DIP_THROW_IF( step == 0, E::INVALID_PARAMETER );
      // Compute indices from end
      dip::sint sz = static_cast< dip::sint >( size );
      if( start < 0 ) { start += sz; }
      if( stop < 0 ) { stop += sz; }
      // Check start and stop are within range
      DIP_THROW_IF( ( start < 0 ) || ( start >= sz ) || ( stop < 0 ) || ( stop >= sz ), E::INDEX_OUT_OF_RANGE );
      // Compute stop given start and step
      //stop = start + ((stop-start)/step)*step;
   }

   /// Get the number of pixels addressed by the range (must be fixed first!).
   dip::uint Size() const {
      if( start > stop ) {
         return 1 + static_cast< dip::uint >( start - stop ) / step;
      } else {
         return 1 + static_cast< dip::uint >( stop - start ) / step;
      }
   }

   /// Get the offset for the range (must be fixed first!).
   dip::uint Offset() const {
      return static_cast< dip::uint >( start );
   }

   /// Get the last index in the range (must be fixed first!).
   dip::uint Last() const {
      return static_cast< dip::uint >( stop );
   }

   /// Get the signed step size for the range (must be fixed first!).
   dip::sint Step() const {
      if( start > stop ) {
         return -static_cast< dip::sint >( step );
      } else {
         return static_cast< dip::sint >( step );
      }
   }

   /// An iterator for the range
   class Iterator {
      public:
         using iterator_category = std::forward_iterator_tag; ///< %Iterator category
         using value_type = dip::uint;          ///< Type of value iterator references
         using reference = dip::sint const&;    ///< Type of reference to value
         using pointer = dip::sint const*;      ///< Type of pointer to value

         /// Default constructor
         Iterator() = default;
         /// Constructor
         Iterator( dip::uint index, dip::sint step ) : index_( static_cast< dip::sint >( index )), step_( step ) {}
         Iterator( dip::sint index, dip::sint step ) : index_( index ), step_( step ) {}

         /// Dereference
         value_type operator*() const { return static_cast< value_type >( index_ ); }
         /// Dereference
         pointer operator->() const { return &index_; }

         /// Pre-increment
         Iterator& operator++() {
            index_ += step_;
            return *this;
         }
         /// Post-increment
         Iterator operator++( int ) {
            Iterator tmp( *this );
            index_ += step_;
            return tmp;
         }

         /// Equality comparison
         bool operator==( Iterator const& other ) const { return index_ == other.index_; }
         /// Inequality comparison
         bool operator!=( Iterator const& other ) const { return index_ != other.index_; }

      private:
         dip::sint index_ = 0; // We use signed one here in case step_<0, we need the one-past-the-end to be sensical
         dip::sint step_ = 1;  //    (sure, we could rely on defined unsigned integer overflow, but why?)
   };

   /// Get an iterator to the beginning of the range (must be fixed first!).
   Iterator begin() const {
      return Iterator( start, Step() );
   }

   /// Get an iterator to the end of the range (must be fixed first!).
   Iterator end() const {
      return Iterator( start + static_cast< dip::sint >( Size() ) * Step(), Step() );
   }
};

/// \brief An array of ranges
/// \relates dip::Range
using RangeArray = DimensionArray< Range >;


//
// The following is support for defining an options type, where the user can
// specify multiple options to pass on to a function or class. The class should
// not be used directly, only through the macro defined below it.
//
// With lots of help from Toby Speight: https://codereview.stackexchange.com/a/183260/151754
//

namespace detail {

template< typename Enum, typename = std::enable_if_t< std::is_enum< Enum >::value >>
class DIP_NO_EXPORT dip__Options {
      using value_type = unsigned long;
      using enum_u_type = typename std::underlying_type< Enum >::type;
      value_type values = 0;

   private:
      // Private constructor used by `operator+` and `operator-`.
      explicit constexpr dip__Options( value_type n ) noexcept : values{ n } {}

   public:
      constexpr dip__Options() noexcept = default;
      constexpr dip__Options( Enum n ) noexcept : values{ value_type( 1u ) << static_cast< enum_u_type >( n ) } {}

      // Testing
      constexpr bool operator==( dip__Options const other ) const noexcept {
         return values == other.values;
      }
      constexpr bool operator!=( dip__Options const other ) const noexcept {
         return !operator==( other );
      }
      constexpr bool Contains( dip__Options const other ) const noexcept {
         return ( values & other.values ) == other.values;
      }

      // Combining
      constexpr dip__Options operator+( dip__Options const other ) const noexcept {
         return dip__Options{ values | other.values };
      }
      constexpr dip__Options operator-( dip__Options const other ) const noexcept {
         return dip__Options{ values & ~other.values };
      }
      dip__Options& operator+=( dip__Options const other ) noexcept {
         values |= other.values;
         return *this;
      }
      dip__Options& operator-=( dip__Options const other ) noexcept {
         values &= ~other.values;
         return *this;
      }
};

// This operator enables `enum_value + options`, for symmetry with the operator defined
// as a member of `dip__Options`.
template< typename T >
constexpr dip__Options< T > operator+( T a, dip__Options< T > b ) noexcept {
   return b + a;
}

/// \brief Declare a type used to pass enumerated options to a function or class.
///
/// This macro is used as follows:
///
/// ```cpp
///     enum class MyOption { clean, fresh, shine };
///     DIP_DECLARE_OPTIONS( MyOption, MyOptions )
/// ```
///
/// `MyOptions` will be a type that combines one or more values from MyOption.
/// These values can be combined using the `+` operator.
/// A variable of type `MyOptions` can be tested using its `Contains` method
/// which returns a `bool`:
///
/// ```cpp
///     MyOptions opts {};                            // No options are set
///     opts = MyOption::fresh;                       // Set only one option
///     opts = MyOption::clean + MyOption::shine;     // Set only these two options
///     if( opts.Contains( MyOption::clean )) {...}   // Test to see if `MyOption::clean` is set
/// ```
///
/// The `Contains` method returns true only of all flags specified in the input are set.
/// The `==` operator returns true only if the two operands contain exactly the same
/// set of flags.
///
/// Note that there should be no more than 32 options within the enumerator.
///
/// This macro will not work within a class definition -- You will need to manually declare the type alias
/// and define the operator outside of the class.
#define DIP_DECLARE_OPTIONS( EnumType, OptionsType ) \
   using OptionsType = dip::detail::dip__Options< EnumType >; \
   constexpr OptionsType operator+( EnumType a, EnumType b ) noexcept { return OptionsType{ a } + b; }
// The `operator+` allows the addition of two enumerator values to produce an options object.

} // namespace detail


//
// The following are some types for often-used parameters
//

/// \brief Enumerated options are defined in the namespace `dip::Option`, unless they
/// are specific to some other sub-namespace.
namespace Option {

/// \brief Some functions that check for a condition optionally throw an exception
/// if that condition is not met.
enum class DIP_NO_EXPORT ThrowException {
   DONT_THROW, ///< Do not throw and exception, return false if the condition is not met.
   DO_THROW    ///< Throw an exception if the condition is not met.
};

/// \brief The function `dip::Image::CheckIsMask` takes this option to control how sizes are compared.
enum class DIP_NO_EXPORT AllowSingletonExpansion {
   DONT_ALLOW, ///< Do not allow singleton expansion.
   DO_ALLOW    ///< Allow singleton expansion.
};

/// \brief The function `dip::Image::ReForge` takes this option to control how to handle protected images.
enum class DIP_NO_EXPORT AcceptDataTypeChange {
   DONT_ALLOW, ///< Do not allow data type change, the output image is always of the requested type.
   DO_ALLOW    ///< Allow data type change, if the output image is protected, it will be used as is.
};

/// \brief The function `dip::Image::Crop` takes this option to control which pixels are taken.
enum class DIP_NO_EXPORT CropLocation {
   CENTER,        ///< The pixel at the origin of the input image is also at the origin in the output image.
   MIRROR_CENTER, ///< Same as `%CENTER`, but for even-sized images, the origin is presumed to be left of center, rather than right of center.
   TOP_LEFT,      ///< The corner of the image at coordinates {0,0,0...} is kept in the corner.
   BOTTOM_RIGHT,  ///< The corner of the image opposite that of `%TOP_LEFT` is kept in the corner.
};

/// \class dip::Option::CmpPropFlags
/// \brief Determines which properties to compare.
///
/// Valid values are:
///
/// `%CmpPropFlags` constant   | Definition
/// -------------------------- | ----------
/// `CmpProp::DataType`        | Compares data type
/// `CmpProp::Dimensionality`  | Compares number of dimensions
/// `CmpProp::Sizes`           | Compares image size
/// `CmpProp::Strides`         | Compares image strides
/// `CmpProp::TensorShape`     | Compares tensor size and shape
/// `CmpProp::TensorElements`  | Compares number of tensor elements
/// `CmpProp::TensorStride`    | Compares tensor stride
/// `CmpProp::ColorSpace`      | Compares color space
/// `CmpProp::PixelSize`       | Compares pixel size
/// `CmpProp::Samples`         | `CmpProp::DataType` + `CmpProp::Sizes` + `CmpProp::TensorElements`
/// `CmpProp::Shape`           | `CmpProp::DataType` + `CmpProp::Sizes` + `CmpProp::TensorShape`
/// `CmpProp::Full`            | `CmpProp::Shape` + `CmpProp::Strides` + `CmpProp::TensorStride`
/// `CmpProp::All`             | `CmpProp::Shape` + `CmpProp::ColorSpace` + `CmpProp::PixelSize`
///
/// Note that you can add these constants together, for example `dip::Option::CmpProp::Sizes + dip::Option::CmpProp::Strides`.
enum class DIP_NO_EXPORT CmpPropEnumerator {
      DataType,
      Dimensionality,
      Sizes,
      Strides,
      TensorShape,
      TensorElements,
      TensorStride,
      ColorSpace,
      PixelSize
};
DIP_DECLARE_OPTIONS( CmpPropEnumerator, CmpPropFlags )
namespace CmpProp {
constexpr CmpPropFlags DataType = CmpPropEnumerator::DataType;
constexpr CmpPropFlags Dimensionality = CmpPropEnumerator::Dimensionality;
constexpr CmpPropFlags Sizes = CmpPropEnumerator::Sizes;
constexpr CmpPropFlags Strides = CmpPropEnumerator::Strides;
constexpr CmpPropFlags TensorShape = CmpPropEnumerator::TensorShape;
constexpr CmpPropFlags TensorElements = CmpPropEnumerator::TensorElements;
constexpr CmpPropFlags TensorStride = CmpPropEnumerator::TensorStride;
constexpr CmpPropFlags ColorSpace = CmpPropEnumerator::ColorSpace;
constexpr CmpPropFlags PixelSize = CmpPropEnumerator::PixelSize;
constexpr CmpPropFlags Samples = CmpPropEnumerator::DataType + CmpPropEnumerator::Sizes + CmpPropEnumerator::TensorElements;
constexpr CmpPropFlags Shape = CmpPropEnumerator::DataType + CmpPropEnumerator::Sizes + CmpPropEnumerator::TensorShape;
constexpr CmpPropFlags Full = Shape + CmpPropEnumerator::Strides + CmpPropEnumerator::TensorStride;
constexpr CmpPropFlags All = Shape + CmpPropEnumerator::ColorSpace + CmpPropEnumerator::PixelSize;
}

} // namespace Option

/// \brief Represents the result of a 2D regression analysis: `y = intercept + x * slope`.
struct RegressionParameters {
   dfloat intercept = 0.0; ///< intercept
   dfloat slope = 0.0;     ///< slope
};

/// \}

} // namespace dip


namespace std {

// Template specialization of `std::max` for `dip::bin` types
// Note that we cannot do `return a | b;` because std::max needs to return a reference.
template<>
inline dip::bin const& max( dip::bin const& a, dip::bin const& b ) {
   return a ? a : b;
}

// Template specialization of `std::min` for `dip::bin` types
template<>
inline dip::bin const& min( dip::bin const& a, dip::bin const& b ) {
   return a ? b : a;
}

} // namespace std

#endif // DIP_TYPES_H
