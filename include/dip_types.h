/*
 * DIPlib 3.0
 * This file contains definitions for the basic data types.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_TYPES_H
#define DIP_TYPES_H

#include <cstdlib>   // std::malloc, std::realloc, std::free
#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t, etc.
#include <complex>
#include <vector>
#include <bitset>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <utility>

#include <iostream> // temporary

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
// Arrays (==vectors) of signed, unsigned and floating-point values.
//

/// We have our own array type, similar to `std::vector` but optimized for our
/// particular use: hold one element per image dimension. Most images have
/// only two or three dimensions, and for internal processing we might add the
/// tensor dimension to the mix, yielding up to four dimensions for most
/// applications. However, DIPlib does not limit image dimensionality, and we
/// need to be able to hold more than four dimensions if the user needs to do
/// so. We want the array holding the image dimensions to be as efficient in
/// use as a static array of size 4, but without the limitation of a static
/// array. So this version of `std::vector` has a static array of size 4, which
/// is used if that is sufficient, and also a pointer we can use if we need to
/// allocate space on the heap.
///
/// It also differs from std::vector in that it doesn't grow or shrink
/// efficiently, don't use this type when repeatedly using `push_back()` or
/// similar functionality. The DIPlib codebase uses dip::DimensionArray only
/// where the array holds one value per image dimension, and `std::vector`
/// everywhere else.
///
/// The interface tries to copy that of the STL containers, but only partially.
/// We do not include some of the `std::vector` functionality, and do include
/// some custom functionality useful for the specific application of the container.
/// We also have some custom algorithms such as dip::DimensionArray::sort() that
/// assumes the array is short.
///
/// You should only use this container with POD types, I won't be held
/// responsible for weird behavior if you use it otherwise. The POD type should
/// have a default constructor, not require being constructed, and not require
/// being destructed.
///
/// An array of size 0 has space for writing four values in it, `data()` won't
/// return a `nullptr`, and nothing will break if you call front(). But don't
/// do any of these things! The implementation could change. Plus, you're just
/// being silly and making unreadable code.
template<typename T>
class DimensionArray {
   public:
      // Types for consistency with STL containers
      using value_type = T;
      using iterator = T*;
      using const_iterator = const T*;
      using reverse_iterator = std::reverse_iterator<iterator>;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
      using size_type = dip::uint;

      /// The default-initialized array has zero size.
      DimensionArray() {}
      /// Like `std::vector`, you can initialize with a size and a default value.
      explicit DimensionArray( size_type sz, T newval = T() ) { resize( sz, newval ); }
      /// Like `std::vector`, you can initialize with a set of values in braces.
      DimensionArray( const std::initializer_list<T> init ) {
         resize( init.size() );
         std::copy( init.begin(), init.end(), data_ );
      }
      /// Copy constructor, initializes with a copy of `other`.
      DimensionArray ( const DimensionArray& other ) {
         resize( other.size_ );
         std::copy( other.data_, other.data_ + size_, data_ );
      }
      /// Move constructor, initializes by stealing the contents of `other`.
      DimensionArray ( DimensionArray&& other ) {
         steal_data_from( other );
      }

      // Destructor, no need for documentation
      ~DimensionArray() { free_array(); } // no need to keep status consistent...

      /// Copy assignment, copies over data from `other`.
      DimensionArray& operator= ( const DimensionArray & other ) {
         if (this != &other) {
            resize( other.size_ );
            std::copy( other.data_, other.data_ + size_, data_ );
         }
         return *this;
      }
      /// Move assignment, steals the contents of `other`.
      DimensionArray& operator= ( DimensionArray && other ) {
         // Self-assignment is not valid for move assignment, not testing for it here.
         free_array();
         steal_data_from( other );
         return *this;
      }

      /// Swaps the contents of two arrays.
      void swap( DimensionArray & other ) {
         if( is_dynamic() ) {
            if( other.is_dynamic() ) {
               // both have dynamic memory
               std::swap( data_, other.data_ );
            } else {
               // *this has dynamic memory, other doesn't
               other.data_ = data_;
               data_ = stat_;
               std::copy( other.stat_, other.stat_ + other.size_, stat_ );
            }
         } else {
            if( other.is_dynamic() ) {
               // other has dynamic memory, *this doesn't
               data_ = other.data_;
               other.data_ = other.stat_;
               std::copy( stat_, stat_ + size_, other.stat_ );
            } else {
               // both have static memory
               std::swap_ranges( stat_, stat_ + std::max( size_, other.size_ ), other.stat_ );
            }
         }
         std::swap( size_, other.size_ );
      }

      /// Resizes the array, making it either larger or smaller; initializes
      /// new elements with `newval`.
      void resize( size_type newsz, T newval = T() ) {
         if( newsz == size_ ) return; // NOP
         if( newsz > static_size ) {
            if( is_dynamic() ) {
               // expand or contract heap data
               T* tmp = static_cast<T*>( std::realloc( data_, newsz * sizeof( T ) ) );
               if( tmp == nullptr ) {
                  throw std::bad_alloc();
               }
               std::cout << "   DimensionArray realloc\n";
               if( newsz > size_ ) {
                  std::fill( tmp + size_, tmp + newsz, newval );
               }
               size_ = newsz;
               data_ = tmp;
            } else {
               // move from static to heap data
               T* tmp = static_cast<T*>( std::malloc( newsz * sizeof( T ) ) );
               std::cout << "   DimensionArray malloc\n";
               if( tmp == nullptr ) {
                  throw std::bad_alloc();
               }
               std::copy( stat_, stat_ + size_, tmp );
               std::fill( tmp + size_, tmp + newsz, newval );
               size_ = newsz;
               data_ = tmp;
            }
         } else {
            if( is_dynamic() ) {
               // move from heap to static data
               if( newsz > 0 ) {
                  std::copy( data_, data_ + newsz, stat_ );
               }
               free_array();
               size_ = newsz;
               data_ = stat_;
            } else {
               // expand or contract static data
               if( newsz > size_ ) {
                  std::fill( stat_ + size_, stat_ + newsz, newval );
               }
               size_ = newsz;
            }
         }
      }

      /// Clears the contents of the array, set its length to 0.
      void clear() { resize( 0 ); }

      /// Checks whether the array is empty (size is 0).
      bool empty() const { return size_ == 0; }

      /// Returns the size of the array.
      size_type size() const { return size_; }

      /// Accesses an element of the array
      T& operator [] ( size_type index ) { return *(data_ + index); }
      /// Accesses an element of the array
      const T& operator [] ( size_type index ) const { return *(data_ + index); }

      /// Accesses the first element of the array
      T& front() { return *data_; }
      /// Accesses the first element of the array
      const T& front() const { return *data_; }

      /// Accesses the last element of the array
      T& back() { return *(data_ + size_ - 1); }
      /// Accesses the last element of the array
      const T& back() const { return *(data_ + size_ - 1); }

      /// Returns a pointer to the underlying data
      T* data() { return data_; };
      /// Returns a pointer to the underlying data
      const T* data() const { return data_; };

      /// Returns an iterator to the beginning
      iterator begin() { return data_; }
      /// Returns an iterator to the beginning
      const_iterator begin() const { return data_; }
      /// Returns an iterator to the end
      iterator end() { return data_ + size_; }
      /// Returns an iterator to the end
      const_iterator end() const { return data_ + size_; }
      /// Returns a reverse iterator to the beginning
      reverse_iterator rbegin() { return reverse_iterator(end()); }
      /// Returns a reverse iterator to the beginning
      const_reverse_iterator rbegin() const { return const_reverse_iterator(begin()); }
      /// Returns a reverse iterator to the end
      reverse_iterator rend() { return reverse_iterator(begin()); }
      /// Returns a reverse iterator to the end
      const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

      /// Adds a value at the given location, moving the current value at that
      /// location and subsequent values forward by one.
      void insert( size_type index, const T& value ) {
         assert( index <= size_ );
         resize( size_ + 1 );
         if( index < size_ - 1 ) {
            std::copy_backward( data_ + index, data_ + size_ - 1, data_ + size_ );
         }
         *(data_ + index) = value;
      }
      /// Adds a value to the back.
      void push_back( const T& value ) {
         resize( size_ + 1 );
         *(data_ + size_ - 1) = value;
      }

      /// Removes the value at the given location, moving subsequent values
      /// forward by one.
      void erase( size_type index ) {
         assert( index < size_ );
         if( index < size_ - 1 ) {
            std::copy( data_ + index + 1, data_ + size_, data_ + index );
         }
         resize( size_ - 1 );
      }
      /// Removes the value at the back.
      void pop_back() {
         assert( size_ > 0 );
         resize( size_ - 1 );
      }

      /// Compares two arrays, returns true only if they have the same size and
      /// contain the same values.
      friend inline bool operator == (const DimensionArray& lhs, const DimensionArray& rhs) {
         if( lhs.size_ != rhs.size_ ) {
            return false;
         }
         const T* lhsp = lhs.data_;
         const T* rhsp = rhs.data_;
         for( size_type ii=0; ii<lhs.size_; ++ii ) {
            if( *(lhsp++) != *(rhsp++) ) {
               return false;
            }
         }
         return true;
      }
      /// Compares two arrays, returns true if they have different size ans/or
      /// contain different values.
      friend inline bool operator != (const DimensionArray& lhs, const DimensionArray& rhs) {
         return !(lhs == rhs);
      }

      /// Sort the contents of the array from smallest to largest.
      void sort() {
         // Using bubble sort because we expect the array to be small.
         if( size_ > 1 ) {
            for( size_type jj = size_ - 1; jj != 0; --jj ) {
               for( size_type ii = 0; ii != jj; ++ii ) {
                  if( data_[ii] > data_[ii+1] ) {
                     std::swap( data_[ii], data_[ii+1] );
                  }
               }
            }
         }
      }
      /// Sort the contents of the array from smallest to largest, and keeping
      /// `other` in the same order.
      template<typename S>
      void sort( DimensionArray<S>& other ) {
         assert( size_ == other.size_ );
         // Using bubble sort because we expect the array to be small.
         if( size_ > 1 ) {
            for( size_type jj = size_ - 1; jj != 0; --jj ) {
               for( size_type ii = 0; ii != jj; ++ii ) {
                  if( data_[ii] > data_[ii+1] ) {
                     std::swap( data_[ii], data_[ii+1] );
                     std::swap( other.data_[ii], other.data_[ii+1] );
                  }
               }
            }
         }
      }

   private:
      constexpr static size_type static_size = 4;
      size_type size_ = 0;
      T* data_ = stat_;
      T  stat_[static_size];
      // The alternate implementation, where data_ and stat_ are in a union
      // to reduce the amount of memory used, requires a test for every data
      // access. Data access is most frequent, it's worth using a little bit
      // more memory to avoid that test.

      bool is_dynamic() {
         return size_ > static_size;
      }

      void free_array() {
         if( is_dynamic() ) {
            std::free( data_ );
            std::cout << "   DimensionArray free\n";
         }
      }

      void steal_data_from( DimensionArray& other ) {
         size_ = other.size_;
         other.size_ = 0; // so if we steal the pointer, other won't deallocate the memory space
         if( is_dynamic() ) {
            data_ = other.data_; // move pointer
            other.data_ = other.stat_; // make sure other is still correct
         } else {
            data_ = stat_;
            std::move( other.data_, other.data_ + size_, data_ );
         }
      }

};

typedef DimensionArray<dip::sint>      IntegerArray;   ///< An array to hold strides, filter sizes, etc.
typedef DimensionArray<dip::uint>      UnsignedArray;  ///< An array to hold dimensions, dimension lists, etc.
typedef DimensionArray<dip::dfloat>    FloatArray;     ///< An array to hold filter parameters.
typedef DimensionArray<dip::dcomplex>  ComplexArray;   //   (used in only one obscure function in the old DIPlib.
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
/// `MyOptions` will by a type that has three non-exclusive flags. Each of the
/// three DIP_DEFINE_OPTION commands defines a `constexpr` variable for the
/// given flag. These values can be combined using the `+` operator.
/// A variable of type `MyOptions` can be tested using the `==` operator,
/// which returns a `bool`:
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
