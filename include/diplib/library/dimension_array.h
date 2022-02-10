/*
 * (c)2016-2021, Cris Luengo.
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


#ifndef DIP_DIMENSIONARRAY_H
#define DIP_DIMENSIONARRAY_H

#include <cstdlib>   // std::malloc, std::realloc, std::free, std::size_t
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <utility>
#include <iostream>
#include <type_traits>
#include <cmath>

#include "diplib/library/error.h"


/// \file
/// \brief The \ref dip::DimensionArray template class. This file is always included through \ref "diplib.h".
/// See \ref supporttypes.


namespace dip {


/// \addtogroup supporttypes


/// \brief A dynamic array type optimized for few elements.
///
/// `dip::DimensionArray` is similar to `std::vector` but optimized for one particular
/// use within the *DIPlib* library: hold one element per image dimension. Most images have
/// only two or three dimensions, and for internal processing we might add the
/// tensor dimension to the mix, yielding up to four dimensions for most
/// applications. However, *DIPlib* does not limit image dimensionality, and we
/// need to be able to hold more than four dimensions if the user needs to do
/// so. We want the array holding the image dimensions to be as efficient in
/// use as a static array of size 4, but without the limitation of a static
/// array. So this version of `std::vector` has a static array of size 4, which
/// is used if that is sufficient, and also a pointer we can use if we need to
/// allocate space on the heap.
///
/// It also differs from std::vector in that it doesn't grow or shrink
/// efficiently, don't use this type when repeatedly using `push_back()` or
/// similar functionality. The *DIPlib* codebase uses dip::DimensionArray only
/// where the array holds one value per image dimension, or when more often
/// than not the array will have very few elements, and `std::vector` everywhere
/// else.
///
/// The interface tries to copy that of the STL containers, but only partially.
/// We do not include some of the `std::vector` functionality, and do include
/// some custom functionality useful for the specific application of the container.
/// We also have some custom algorithms such as dip::DimensionArray::sort() that
/// assumes the array is short.
///
/// This container can only be used with trivially copyable types (this means that
/// the destructor performs no action, and the object can be copied by copying its
/// bit pattern).
///
/// An array of size 0 has space for writing four values in it, `data()` won't
/// return a `nullptr`, and nothing will break if you call front(). But don't
/// do any of these things! The implementation could change. Plus, you're just
/// being silly and making unreadable code.
template< typename T >
class DIP_NO_EXPORT DimensionArray {
      static_assert( std::is_trivially_copyable<T>::value, "DimensionArray can only be used with trivially copyable types." );

   public:
      // Types for consistency with STL containers
      /// Type of values stored in container
      using value_type = T;
      /// Type of container's iterator
      using iterator = T*;
      /// Type of container's const iterator
      using const_iterator = T const*;
      /// Type of container's reverse iterator
      using reverse_iterator = std::reverse_iterator< iterator >;
      /// Type of container's const reverse iterator
      using const_reverse_iterator = std::reverse_iterator< const_iterator >;
      /// Type of index into container
      using size_type = std::size_t;

      /// The default-initialized array has zero size.
      DimensionArray() {}; // Using `=default` causes weird sequence of "constructor required before non-static data member for ‘dip::Histogram::Configuration::lowerBound’ has been parsed" in GCC

      /// Like `std::vector`, you can initialize with a size and a default value.
      explicit DimensionArray( size_type sz, T newval = T() ) {
         resize( sz, newval );
      }

      /// Like `std::vector`, you can initialize with a set of values in braces.
      DimensionArray( std::initializer_list< T > const init ) {
         resize( init.size() );
         std::copy( init.begin(), init.end(), data_ );
      }

      /// Copy constructor, initializes with a copy of `other`.
      DimensionArray( DimensionArray const& other ) {
         resize( other.size_ );
         std::copy( other.data_, other.data_ + size_, data_ );
      }

      /// \brief Cast constructor, initializes with a copy of `other`.
      /// Casting done as default in C++, not through `dip::clamp_cast`.
      template< typename O >
      explicit DimensionArray( DimensionArray< O > const& other ) {
         resize( other.size() );
         std::transform( other.data(), other.data() + size_, data_, []( O const& v ) { return static_cast< value_type >( v ); } );
      }

      /// Move constructor, initializes by stealing the contents of `other`.
      DimensionArray( DimensionArray&& other ) noexcept {
         steal_data_from( other );
      }

      // Destructor, no need for documentation
      ~DimensionArray() noexcept {
         free_array(); // no need to keep status consistent...
      }

      /// Copy assignment, copies over data from `other`.
      DimensionArray& operator=( DimensionArray const& other ) {
         if( this != &other ) {
            resize( other.size_ );
            std::copy( other.data_, other.data_ + size_, data_ );
         }
         return *this;
      }

      /// Move assignment, steals the contents of `other`.
      DimensionArray& operator=( DimensionArray&& other ) noexcept {
         // Self-assignment is not valid for move assignment, not testing for it here.
         free_array();
         steal_data_from( other );
         return *this;
      }

      /// Swaps the contents of two arrays.
      void swap( DimensionArray& other ) noexcept {
         using std::swap;
         if( is_dynamic() ) {
            if( other.is_dynamic() ) {
               // both have dynamic memory
               swap( data_, other.data_ );
            } else {
               // *this has dynamic memory, other doesn't
               other.data_ = data_;
               data_ = stat_;
               std::move( other.stat_, other.stat_ + other.size_, stat_ );
            }
         } else {
            if( other.is_dynamic() ) {
               // other has dynamic memory, *this doesn't
               data_ = other.data_;
               other.data_ = other.stat_;
               std::move( stat_, stat_ + size_, other.stat_ );
            } else {
               // both have static memory
               std::swap_ranges( stat_, stat_ + std::max( size_, other.size_ ), other.stat_ );
            }
         }
         swap( size_, other.size_ );
      }

      /// \brief Resizes the array, making it either larger or smaller. Initializes
      /// new elements with `newval`.
      void resize( size_type newsz, T newval = T() ) {
         if( newsz == size_ ) { return; } // NOP
         if( newsz > static_size_ ) {
            if( is_dynamic() ) {
               // expand or contract heap data
               T* tmp = static_cast< T* >( std::realloc( data_, newsz * sizeof( T )));
               //std::cout << "   DimensionArray realloc\n";
               if( tmp == nullptr ) {
                  throw std::bad_alloc();
               }
               data_ = tmp;
               if( newsz > size_ ) {
                  std::fill( data_ + size_, data_ + newsz, newval );
               }
               size_ = newsz;
            } else {
               // move from static to heap data
               // We use malloc because we want to be able to use realloc; new cannot do this.
               T* tmp = static_cast< T* >( std::malloc( newsz * sizeof( T )));
               //std::cout << "   DimensionArray malloc\n";
               if( tmp == nullptr ) {
                  throw std::bad_alloc();
               }
               std::move( stat_, stat_ + size_, tmp );
               data_ = tmp;
               std::fill( data_ + size_, data_ + newsz, newval );
               size_ = newsz;
            }
         } else {
            if( is_dynamic() ) {
               // move from heap to static data
               if( newsz > 0 ) {
                  std::move( data_, data_ + newsz, stat_ );
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
      void clear() noexcept {
         free_array();
         reset();
      }

      /// Checks whether the array is empty (size is 0).
      DIP_NODISCARD bool empty() const noexcept { return size_ == 0; }

      /// Returns the size of the array.
      size_type size() const noexcept { return size_; }

      /// Accesses an element of the array
      T& operator[]( size_type index ) { return *( data_ + index ); }
      /// Accesses an element of the array
      T const& operator[]( size_type index ) const { return *( data_ + index ); }

      /// Accesses the first element of the array
      T& front() { return *data_; }
      /// Accesses the first element of the array
      T const& front() const { return *data_; }

      /// Accesses the last element of the array
      T& back() { return *( data_ + size_ - 1 ); }
      /// Accesses the last element of the array
      T const& back() const { return *( data_ + size_ - 1 ); }

      /// Returns a pointer to the underlying data
      T* data() { return data_; };
      /// Returns a pointer to the underlying data
      T const* data() const { return data_; };

      /// Returns an iterator to the beginning
      iterator begin() { return data_; }
      /// Returns an iterator to the beginning
      const_iterator begin() const { return data_; }
      /// Returns an iterator to the end
      iterator end() { return data_ + size_; }
      /// Returns an iterator to the end
      const_iterator end() const { return data_ + size_; }
      /// Returns a reverse iterator to the beginning
      reverse_iterator rbegin() { return reverse_iterator( end() ); }
      /// Returns a reverse iterator to the beginning
      const_reverse_iterator rbegin() const { return const_reverse_iterator( end() ); }
      /// Returns a reverse iterator to the end
      reverse_iterator rend() { return reverse_iterator( begin() ); }
      /// Returns a reverse iterator to the end
      const_reverse_iterator rend() const { return const_reverse_iterator( begin() ); }

      /// \brief Adds a value at the given location, moving the current value at that
      /// location and subsequent values forward by one.
      void insert( size_type index, T const& value ) {
         DIP_ASSERT( index <= size_ );
         resize( size_ + 1 ); // we're default-initializing one T here.
         if( index < size_ - 1 ) {
            std::move_backward( data_ + index, data_ + size_ - 1, data_ + size_ );
         }
         *( data_ + index ) = value;
      }

      /// \brief Adds a value to the back. Not efficient -- prefer `std::vector` if you need
      /// to use this repeatedly.
      void push_back( T const& value ) {
         resize( size_ + 1, value );
      }

      /// Adds all values in source array to the back.
      void append( DimensionArray const& values ) {
         size_type index = size_;
         resize( size_ + values.size_ );
         for( size_type ii = 0; ii < values.size_; ++ii ) {
            data_[ index + ii ] = values.data_[ ii ];
         }
      }

      /// Removes the value at the given location, moving subsequent values forward by one.
      void erase( size_type index ) {
         DIP_ASSERT( index < size_ );
         if( index < size_ - 1 ) {
            std::move( data_ + index + 1, data_ + size_, data_ + index );
         }
         resize( size_ - 1 );
      }

      /// Removes the value at the back.
      void pop_back() {
         DIP_ASSERT( size_ > 0 );
         resize( size_ - 1 );
      }

      /// Adds a constant to each element in the array.
      DimensionArray& operator+=( T const& v ) {
         for( size_type ii = 0; ii < size_; ++ii ) {
            data_[ ii ] += v;
         }
         return *this;
      }

      /// \brief Adds an array to `this`, element-wise. `other` must have the same number of elements.
      template< typename S >
      DimensionArray< T >& operator+=( DimensionArray< S > const& other );

      /// Subtracts a constant from each element in the array.
      DimensionArray& operator-=( T const& v ) {
         for( size_type ii = 0; ii < size_; ++ii ) {
            data_[ ii ] -= v;
         }
         return *this;
      }

      /// \brief Subtracts an array from `this`, element-wise. `other` must have the same number of elements.
      template< typename S >
      DimensionArray< T >& operator-=( DimensionArray< S > const& other );

      /// Sort the contents of the array from smallest to largest.
      void sort() {
         // Using insertion sort because we expect the array to be small.
         for( size_type ii = 1; ii < size_; ++ii ) {
            T elem = data_[ ii ];
            size_type jj = ii;
            while(( jj > 0 ) && ( data_[ jj - 1 ] > elem )) {
               data_[ jj ] = data_[ jj - 1 ];
               --jj;
            }
            data_[ jj ] = elem;
         }
      }

      /// Multiplies each element in the array by a constant.
      DimensionArray& operator*=( T const& v ) {
         for( size_type ii = 0; ii < size_; ++ii ) {
            data_[ ii ] *= v;
         }
         return *this;
      }

      /// Divides each element in the array by a constant.
      DimensionArray& operator/=( T const& v ) {
         for( size_type ii = 0; ii < size_; ++ii ) {
            data_[ ii ] /= v;
         }
         return *this;
      }

      /// Sort the contents of the array from smallest to largest, and keeping `other` in the same order.
      template< typename S >
      void sort( DimensionArray< S >& other ) {
         // We cannot access private members of `other` because it's a different class (if S != T).
         DIP_ASSERT( size_ == other.size() );
         // Using insertion sort because we expect the array to be small.
         for( size_type ii = 1; ii < size_; ++ii ) {
            T elem = data_[ ii ];
            S otherelem = other[ ii ];
            size_type jj = ii;
            while(( jj > 0 ) && ( data_[ jj - 1 ] > elem )) {
               data_[ jj ] = data_[ jj - 1 ];
               other[ jj ] = other[ jj - 1 ];
               --jj;
            }
            data_[ jj ] = elem;
            other[ jj ] = otherelem;
         }
      }

      /// Returns an array with indices into the array, sorted from smallest value to largest.
      DIP_NODISCARD DimensionArray< size_type > sorted_indices() const {
         DimensionArray< size_type > out( size_ );
         for( size_type ii = 0; ii < size_; ++ii ) {
            out[ ii ] = ii;
         }
         // Using insertion sort because we expect the array to be small.
         for( size_type ii = 1; ii < size_; ++ii ) {
            size_type elem = out[ ii ];
            size_type jj = ii;
            while(( jj > 0 ) && ( data_[ out[ jj - 1 ]] > data_[ elem ] )) {
               out[ jj ] = out[ jj - 1 ];
               --jj;
            }
            out[ jj ] = elem;
         }
         return out;
      }

      /// \brief Order the elements in the array according to the `order` array, such as returned by `sorted_indices`.
      ///
      /// Postcondition:
      ///
      /// ```cpp
      /// out[ ii ] = (*this)[ order[ ii ] ];
      /// ```
      DIP_NODISCARD DimensionArray permute( DimensionArray< size_type > const& order ) const {
         DimensionArray out( order.size() );
         for( size_type ii = 0; ii < order.size(); ++ii ) {
            out[ ii ] = data_[ order[ ii ]];
         }
         return out;
      }

      /// \brief Inverse orders the elements in the array according to the `order` array, such as returned by `sorted_indices`.
      ///
      /// Postcondition:
      ///
      /// ```cpp
      /// out[ order[ ii ]] = (*this)[ ii ];
      /// ```
      ///
      /// Elements not indexed by `order` will be default-initialized.
      DIP_NODISCARD DimensionArray inverse_permute( DimensionArray< size_type > const& order ) const {
         size_type n = 0;
         for( auto o : order ) {
            n = std::max( n, o + 1 );
         }
         DimensionArray out( n );
         for( size_type ii = 0; ii < order.size(); ++ii ) {
            out[ order[ ii ]] = data_[ ii ];
         }
         return out;
      }

      /// Finds the first occurrence of `value` in the array, returns the index or `size()` if it is not present.
      size_type find( T value ) {
         // Like in `sort`, we expect the array to be small
         size_type ii = 0;
         while(( ii < size_ ) && ( data_[ ii ] != value )) {
            ++ii;
         }
         return ii;
      }

      /// Compute the sum of the elements in the array.
      T sum() const {
         T p = 0;
         for( size_type ii = 0; ii < size_; ++ii ) {
            p += data_[ ii ];
         }
         return p;
      }

      /// Compute the product of the elements in the array. Returns 1 for an empty array.
      T product() const {
         T p = 1;
         for( size_type ii = 0; ii < size_; ++ii ) {
            p *= data_[ ii ];
         }
         return p;
      }

      /// Compute the sum of the squares of the elements in the array.
      double norm_square() const {
         double p = 0;
         for( size_type ii = 0; ii < size_; ++ii ) {
            double d = static_cast< double >( data_[ ii ] );
            p += d * d;
         }
         return p;
      }

      /// Find the minimum element in the array, returns the index or 0 if the array is empty.
      size_type minimum() const {
         if( size_ == 0 ) {
            return 0;
         }
         size_type result = 0;
         size_type ii = 0;
         while( ++ii != size_ ) {
            if( data_[ ii ] < data_[ result ] ) {
               result = ii;
            }
         }
         return result;
      }

      /// Find the maximum element in the array, returns the index or 0 if the array is empty.
      size_type maximum() const {
         if( size_ == 0 ) {
            return 0;
         }
         size_type result = 0;
         size_type ii = 0;
         while( ++ii != size_ ) {
            if( data_[ ii ] > data_[ result ] ) {
               result = ii;
            }
         }
         return result;
      }

      /// \brief Find the minimum element in the array, returns the value. The array must not be empty.
      T minimum_value() const {
         // If the array is empty, returns `data[0]`, which is OK but contains unspecified data.
         return data_[ minimum() ];
      }
      /// \brief Find the minimum element in the array, returns the value. The array must not be empty.
      T& minimum_value() {
         return data_[ minimum() ];
      }

      /// \brief Find the maximum element in the array, returns the value. The array must not be empty.
      T maximum_value() const {
         return data_[ maximum() ];
      }
      /// \brief Find the maximum element in the array, returns the value. The array must not be empty.
      T& maximum_value() {
         return data_[ maximum() ];
      }


      /// True if all elements are non-zero.
      bool all() const {
         for( size_type ii = 0; ii < size_; ++ii ) {
            if( data_[ ii ] == T( 0 )) {
               return false;
            }
         }
         return true;
      }

      /// True if one element is non-zero.
      bool any() const {
         for( size_type ii = 0; ii < size_; ++ii ) {
            if( data_[ ii ] != T( 0 )) {
               return true;
            }
         }
         return false;
      }

      /// Count of number of elements that are non-zero.
      size_type count() const {
         size_type n = 0;
         for( size_type ii = 0; ii < size_; ++ii ) {
            if( data_[ ii ] != T( 0 )) {
               ++n;
            }
         }
         return n;
      }

      /// Assigns one same value to each element in the array
      void fill( T const& value ) {
         for( size_type ii = 0; ii < size_; ++ii ) {
            data_[ ii ] = value;
         }
      }

   private:
      constexpr static size_type static_size_ = 4;
      size_type size_ = 0;
      T* data_ = stat_;
      T stat_[ static_size_ ];
      // The alternate implementation, where data_ and stat_ are in a union
      // to reduce the amount of memory used, requires a test for every data
      // access. Data access is most frequent, it's worth using a little bit
      // more memory to avoid that test.

      bool is_dynamic() const noexcept {
         return data_ != stat_;
      }

      void free_array() noexcept {
         if( is_dynamic() ) {
            std::free( data_ );
            //std::cout << "   DimensionArray free\n";
         }
      }

      void reset() noexcept {
         // This should be called only after `free_array()`.
         size_ = 0;
         data_ = stat_;
      }

      void steal_data_from( DimensionArray& other ) noexcept {
         if( other.is_dynamic() ) {
            size_ = other.size_;
            data_ = other.data_;       // move pointer
            other.reset();             // leave other in consistent, empty state
         } else {
            size_ = other.size_;
            data_ = stat_;
            std::move( other.data_, other.data_ + size_, data_ );
         }
      }
};


//
// Compound assignment operator specializations
//

// The general case: cast rhs to type of lhs
template< typename T >
template< typename S >
DimensionArray< T >& DimensionArray< T >::operator+=( DimensionArray< S > const& other ) {
   DIP_ASSERT( size_ == other.size() );
   for( size_type ii = 0; ii < size_; ++ii ) {
      data_[ ii ] += static_cast< T >( other[ ii ] );
   }
   return *this;
}
// A special case for adding a signed array to an unsigned array
template<>
template<>
inline DimensionArray< std::size_t >& DimensionArray< std::size_t >::operator+=( DimensionArray< std::ptrdiff_t > const& other ) {
   DIP_ASSERT( size_ == other.size() );
   for( size_type ii = 0; ii < size_; ++ii ) {
      data_[ ii ] = static_cast< std::size_t >( static_cast< std::ptrdiff_t >( data_[ ii ] ) + other[ ii ] );
   }
   return *this;
}

template< typename T >
template< typename S >
DimensionArray< T >& DimensionArray< T >::operator-=( DimensionArray< S > const& other ) {
   DIP_ASSERT( size_ == other.size() );
   for( size_type ii = 0; ii < size_; ++ii ) {
      data_[ ii ] -= static_cast< T >( other[ ii ] );
   }
   return *this;
}
// A special case for subtracting a signed array from an unsigned array
template<>
template<>
inline DimensionArray< std::size_t >& DimensionArray< std::size_t >::operator-=( DimensionArray< std::ptrdiff_t > const& other ) {
   DIP_ASSERT( size_ == other.size() );
   for( size_type ii = 0; ii < size_; ++ii ) {
      data_[ ii ] = static_cast< std::size_t >( static_cast< std::ptrdiff_t >( data_[ ii ] ) - other[ ii ] );
   }
   return *this;
}


//
// Comparison operators, array vs array
//

/// \brief Compares two arrays, returns true only if they have the same size and contain the same values.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator==( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   if( lhs.size() != rhs.size() ) {
      return false;
   }
   auto lhsp = lhs.begin();
   auto rhsp = rhs.begin();
   while( lhsp != lhs.end() ) {
      if( *lhsp != *rhsp ) {
         return false;
      }
      ++lhsp;
      ++rhsp;
   };
   return true;
}
/// \brief Compares two arrays, returns true if they have different size and/or contain different values.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator!=( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   return !( lhs == rhs );
}

// Note on ordering operators: These have a non-standard meaning, because they all return false if the arrays
// are not of the same length. Therefore, it is not possible to define operator<= in terms of operator>, etc.

/// \brief Compares two arrays, returns true only if they have the same size and all `lhs` elements are larger
/// than all `rhs` elements.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator>( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   if( lhs.size() != rhs.size() ) {
      return false;
   }
   auto lhsp = lhs.begin();
   auto rhsp = rhs.begin();
   while( lhsp != lhs.end() ) {
      if( *lhsp <= *rhsp ) {
         return false;
      }
      ++lhsp;
      ++rhsp;
   };
   return true;
}
/// \brief Compares two arrays, returns true only if they have the same size and all `lhs` elements are smaller
/// than all `rhs` elements.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator<( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   if( lhs.size() != rhs.size() ) {
      return false;
   }
   auto lhsp = lhs.begin();
   auto rhsp = rhs.begin();
   while( lhsp != lhs.end() ) {
      if( *lhsp >= *rhsp ) {
         return false;
      }
      ++lhsp;
      ++rhsp;
   };
   return true;
}
/// \brief Compares two arrays, returns true only if they have the same size and all `lhs` elements are larger
/// or equal than all `rhs` elements.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator>=( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   if( lhs.size() != rhs.size() ) {
      return false;
   }
   auto lhsp = lhs.begin();
   auto rhsp = rhs.begin();
   while( lhsp != lhs.end() ) {
      if( *lhsp < *rhsp ) {
         return false;
      }
      ++lhsp;
      ++rhsp;
   };
   return true;
}
/// \brief Compares two arrays, returns true only if they have the same size and all `lhs` elements are smaller
/// or equal than all `rhs` elements.
/// \relates dip::DimensionArray
template< typename T >
inline bool operator<=( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   if( lhs.size() != rhs.size() ) {
      return false;
   }
   auto lhsp = lhs.begin();
   auto rhsp = rhs.begin();
   while( lhsp != lhs.end() ) {
      if( *lhsp > *rhsp ) {
         return false;
      }
      ++lhsp;
      ++rhsp;
   };
   return true;
}


//
// Comparison operators, array vs scalar
//

/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator==( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp == rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}
/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator!=( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp != rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}
/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator>( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp > rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}
/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator<( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp < rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}
/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator>=( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp >= rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}
/// \brief Compares an array to a scalar, returns a boolean array.
/// \relates dip::DimensionArray
template< typename T >
inline DimensionArray< bool > operator<=( DimensionArray< T > const& lhs, T const& rhs ) {
   DimensionArray< bool > out( lhs.size() );
   auto lhsp = lhs.begin();
   auto outp = out.begin();
   while( lhsp != lhs.end() ) {
      *outp = *lhsp <= rhs;
      ++lhsp;
      ++outp;
   };
   return out;
}


//
// Other operators and convenience functions
//

/// \brief Writes the array to a stream
/// \relates dip::DimensionArray
template< typename T >
inline std::ostream& operator<<(
      std::ostream& os,
      DimensionArray< T > const& array
) {
   os << '{';
   auto it = array.begin();
   if( it != array.end() ) {
      os << *it;
      while( ++it != array.end() ) {
         os << ", " << *it;
      };
   }
   os << '}';
   return os;
}

template< typename T >
inline void swap( DimensionArray< T >& v1, DimensionArray< T >& v2 ) {
   v1.swap( v2 );
}

/// \brief Sorts the `indices` array with indices into the `data` array, from smallest to largest. The sort is stable.
/// \relates dip::DimensionArray
template< typename T >
inline void sortIndices( DimensionArray< typename DimensionArray< T >::size_type >& indices, DimensionArray< T > const& data ) {
   using size_type = typename DimensionArray< T >::size_type;
   #ifdef DIP_CONFIG_ENABLE_ASSERT
      for( size_type ii = 0; ii < indices.size(); ++ii ) {
         DIP_ASSERT( indices[ ii ] < data.size() );
      }
   #endif
   // Using insertion sort because we expect the array to be small.
   for( size_type ii = 1; ii < indices.size(); ++ii ) {
      size_type elem = indices[ ii ];
      size_type jj = ii;
      while(( jj > 0 ) && ( data[ indices[ jj - 1 ]] > data[ elem ] )) {
         indices[ jj ] = indices[ jj - 1 ];
         --jj;
      }
      indices[ jj ] = elem;
   }
}

/// \brief Computes the Square Euclidean distance between two points.
/// \relates dip::DimensionArray
template< typename T >
inline double SquareDistance( DimensionArray< T > const& v1, DimensionArray< T > const& v2 ) {
   DIP_ASSERT( v1.size() == v2.size() );
   using size_type = typename DimensionArray< T >::size_type;
   double p = 0;
   for( size_type ii = 0; ii < v1.size(); ++ii ) {
      double d = static_cast< double >( v1[ ii ] ) - static_cast< double >( v2[ ii ] );
      p += d * d;
   }
   return p;
}

/// \brief Computes the Square Euclidean distance between two points.
/// \relates dip::DimensionArray
template< typename T >
inline double Distance( DimensionArray< T > const& v1, DimensionArray< T > const& v2 ) {
   return std::sqrt( SquareDistance( v1, v2 ));
}

/// \endgroup

} // namespace dip

#endif // DIP_DIMENSIONARRAY_H
