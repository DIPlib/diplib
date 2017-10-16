/*
 * DIPlib 3.0
 * This file contains the definition for the dip::DimensionArray template class.
 *
 * (c)2016-2017, Cris Luengo.
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

#ifdef _WIN32
   #define _USE_MATH_DEFINES // Needed to define M_PI in <cstdlib>/<math.h>
#endif

#include <cstdlib>   // std::malloc, std::realloc, std::free, std::size_t
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <utility>
#include <iostream>

#include "diplib/library/error.h"


/// \file
/// \brief The `dip::DimensionArray` template class. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief A dynamic array type optimized for few elements.
///
/// We have our own array type, similar to `std::vector` but optimized for our
/// particular use: hold one element per image dimension. Most images have
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
/// You should only use this container with POD types, I won't be held
/// responsible for weird behavior if you use it otherwise. The POD type should
/// have a default constructor, not require being constructed, and not require
/// being destructed.
///
/// An array of size 0 has space for writing four values in it, `data()` won't
/// return a `nullptr`, and nothing will break if you call front(). But don't
/// do any of these things! The implementation could change. Plus, you're just
/// being silly and making unreadable code.
template< typename T >
class DIP_NO_EXPORT DimensionArray {
   public:
      // Types for consistency with STL containers
      using value_type = T;
      using iterator = T*;
      using const_iterator = T const*;
      using reverse_iterator = std::reverse_iterator< iterator >;
      using const_reverse_iterator = std::reverse_iterator< const_iterator >;
      using size_type = std::size_t;

      /// The default-initialized array has zero size.
      DimensionArray() {}
      /// Like `std::vector`, you can initialize with a size and a default value.
      explicit DimensionArray( size_type sz, T newval = T() ) { resize( sz, newval ); }
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
         std::copy( other.data(), other.data() + size_, data_ );
      }
      /// Move constructor, initializes by stealing the contents of `other`.
      DimensionArray( DimensionArray&& other ) noexcept {
         steal_data_from( other );
      }

      // Destructor, no need for documentation
      ~DimensionArray() { free_array(); } // no need to keep status consistent...

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
      void swap( DimensionArray& other ) {
         using std::swap;
         if( is_dynamic() ) {
            if( other.is_dynamic() ) {
               // both have dynamic memory
               swap( data_, other.data_ );
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
         swap( size_, other.size_ );
      }

      /// \brief Resizes the array, making it either larger or smaller. Initializes
      /// new elements with `newval`.
      void resize( size_type newsz, T newval = T() ) {
         if( newsz == size_ ) { return; } // NOP
         if( newsz > static_size_ ) {
            if( is_dynamic() ) {
               // expand or contract heap data
               T* tmp = static_cast< T* >( std::realloc( data_, newsz * sizeof( T ) ) );
               if( tmp == nullptr ) {
                  throw std::bad_alloc();
               }
               //std::cout << "   DimensionArray realloc\n";
               if( newsz > size_ ) {
                  std::fill( tmp + size_, tmp + newsz, newval );
               }
               size_ = newsz;
               data_ = tmp;
            } else {
               // move from static to heap data
               // We use malloc because we want to be able to use realloc; new cannot do this.
               T* tmp = static_cast<T*>( std::malloc( newsz * sizeof( T ) ) );
               //std::cout << "   DimensionArray malloc\n";
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
      T& operator[]( size_type index ) { return *( data_ + index ); }
      /// Accesses an element of the array
      T const& operator[]( size_type index ) const { return *( data_ + index ); }

      /// Accesses the first element of the array
      T& front() { return * data_; }
      /// Accesses the first element of the array
      T const& front() const { return * data_; }

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
         resize( size_ + 1 );
         if( index < size_ - 1 ) {
            std::copy_backward( data_ + index, data_ + size_ - 1, data_ + size_ );
         }
         *( data_ + index ) = value;
      }
      /// Adds a value to the back.
      void push_back( T const& value ) {
         resize( size_ + 1 );
         back() = value;
      }
      /// Adds all values in source array to the back.
      void push_back( DimensionArray const& values ) {
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
            std::copy( data_ + index + 1, data_ + size_, data_ + index );
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
      DimensionArray< size_type > sorted_indices() const {
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
      /// Order the elements in the array according to the `order` array, such as returned by `sorted_indices`.
      ///
      /// Postcondition:
      /// ```cpp
      ///     out[ ii ] = (*this)[ order[ ii ] ];
      /// ```
      DimensionArray permute( DimensionArray< size_type > const& order ) const {
         DimensionArray out( order.size() );
         for( size_type ii = 0; ii < order.size(); ++ii ) {
            out[ ii ] = data_[ order[ ii ]];
         }
         return out;
      }
      /// Inverse orders the elements in the array according to the `order` array, such as returned by `sorted_indices`.
      ///
      /// Postcondition:
      /// ```cpp
      ///     out[ order[ ii ]] = (*this)[ ii ];
      /// ```
      /// Elements not indexed by `order` will be default-initialized.
      DimensionArray inverse_permute( DimensionArray< size_type > const& order ) const {
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

      /// Compute the sum of the elements in the array.
      T sum() const {
         T p = 0;
         for( size_type ii = 0; ii < size_; ++ii ) {
            p += data_[ ii ];
         }
         return p;
      }

      /// Compute the product of the elements in the array.
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
      T stat_[static_size_];
      // The alternate implementation, where data_ and stat_ are in a union
      // to reduce the amount of memory used, requires a test for every data
      // access. Data access is most frequent, it's worth using a little bit
      // more memory to avoid that test.

      bool is_dynamic() noexcept {
         return size_ > static_size_;
      }

      void free_array() noexcept {
         if( is_dynamic() ) {
            std::free( data_ );
            //std::cout << "   DimensionArray free\n";
         }
      }

      void steal_data_from( DimensionArray& other ) noexcept {
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


/// \brief Compares two arrays, returns true only if they have the same size and contain the same values.
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
template< typename T >
inline bool operator!=( DimensionArray< T > const& lhs, DimensionArray< T > const& rhs ) {
   return !( lhs == rhs );
}

// Note on ordering operators: These have a non-standard meaning, because they all return false if the arrays
// are not of the same length. Therefore, it is not possible to define operator<= in terms of operator>, etc.

/// \brief Compares two arrays, returns true only if they have the same size and all `lhs` elements are larger
/// than all `rhs` elements.
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

/// \brief Writes the array to a stream
template< typename T >
inline std::ostream& operator<<(
      std::ostream& os,
      DimensionArray< T > const& array
) {
   os << "{";
   auto it = array.begin();
   if( it != array.end() ) {
      os << *it;
      while( ++it != array.end() ) {
         os << ", " << *it;
      };
   }
   os << "}";
   return os;
}

template< typename T >
inline void swap( DimensionArray< T >& v1, DimensionArray< T >& v2 ) {
   v1.swap( v2 );
}

/// \brief Sorts the `indices` array with indices into the `data` array, from smallest to largest. The sort is stable.
template< typename T >
inline void sortIndices( DimensionArray< typename DimensionArray< T >::size_type >& indices, DimensionArray< T > const& data ) {
   using size_type = typename DimensionArray< T >::size_type;
   #ifdef DIP__ENABLE_ASSERT
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

/// \}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the dip::DimensionArray class") {
   dip::DimensionArray< int > a{ 1, 2, 4, 8, 16, 32 };
   DOCTEST_REQUIRE( a.size() == 6 );
   DOCTEST_REQUIRE( a.sum() == 63 );

   DOCTEST_SUBCASE("swapping") {
      dip::DimensionArray< int > b{ 5, 4, 3, 2, 1 };
      DOCTEST_REQUIRE( b.size() == 5 );
      DOCTEST_REQUIRE( b.sum() == 15 );
      a.swap( b );
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 15 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("initialization") {
      dip::DimensionArray< int > b( 3, 1 );
      DOCTEST_CHECK( b.size() == 3 );
      DOCTEST_CHECK( b.sum() == 3 );
      b.resize( 6, 2 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 9 );
   }

   DOCTEST_SUBCASE("copy constructor I") {
      dip::DimensionArray< int > b( a );
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("copy constructor II") {
      dip::DimensionArray< int > b = a;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("copy assignment") {
      dip::DimensionArray< int > b;
      b = a;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("move constructor") {
      dip::DimensionArray< int > b( std::move( a ) );
      DOCTEST_CHECK( a.size() == 0 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("move assignment") {
      dip::DimensionArray< int > b;
      b = std::move( a );
      DOCTEST_CHECK( a.size() == 0 );
      DOCTEST_CHECK( b.size() == 6 );
      DOCTEST_CHECK( b.sum() == 63 );
   }

   DOCTEST_SUBCASE("pushing, popping") {
      a.push_back( 1 );
      DOCTEST_CHECK( a.size() == 7 );
      DOCTEST_CHECK( a.sum() == 64 );
      a.pop_back();
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      a.pop_back();
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 31 );
   }

   DOCTEST_SUBCASE("equality") {
      dip::DimensionArray< int > b( a );
      DOCTEST_CHECK( a == b );
      b.back() = 0;
      DOCTEST_CHECK( a != b );
      b.pop_back();
      DOCTEST_CHECK( a != b );
   }

   DOCTEST_SUBCASE("insert, erase, clear") {
      a.insert( 0, 100 );
      DOCTEST_CHECK( a.size() == 7 );
      DOCTEST_CHECK( a.sum() == 163 );
      DOCTEST_CHECK( a.front() == 100 );
      a.erase( 0 );
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 63 );
      DOCTEST_CHECK( a.front() == 1 );
      a.erase( 1 );
      DOCTEST_CHECK( a.size() == 5 );
      DOCTEST_CHECK( a.sum() == 61 );
      DOCTEST_CHECK( a.front() == 1 );
      a.clear();
      DOCTEST_CHECK( a.size() == 0 );
   }

   DOCTEST_SUBCASE("indexing") {
      DOCTEST_CHECK( a[ 3 ] == 8 );
      a[ 3 ] = 0;
      DOCTEST_CHECK( a.size() == 6 );
      DOCTEST_CHECK( a.sum() == 55 );
   }

   DOCTEST_SUBCASE("sorting I") {
      dip::DimensionArray< int > b{ 0, 2, 4, 1, 3, 5 };
      DOCTEST_REQUIRE( b.size() == a.size() );
      b.sort( a ); // sorts b, keeps a in sync. so a should be: { 1, 8, 2, 16, 4, 32 }
      DOCTEST_CHECK( a[ 0 ] == 1 );
      DOCTEST_CHECK( a[ 1 ] == 8 );
      DOCTEST_CHECK( a[ 2 ] == 2 );
      DOCTEST_CHECK( a[ 3 ] == 16 );
      DOCTEST_CHECK( a[ 4 ] == 4 );
      DOCTEST_CHECK( a[ 5 ] == 32 );
   }

   DOCTEST_SUBCASE("sorting II") {
      dip::DimensionArray< int > b{ 0, 2, 4, 1, 3, 5 };
      dip::DimensionArray< size_t > i = b.sorted_indices();
      DOCTEST_CHECK( b.size() == i.size() );
      DOCTEST_CHECK( i[ 0 ] == 0 );
      DOCTEST_CHECK( i[ 1 ] == 3 );
      DOCTEST_CHECK( i[ 2 ] == 1 );
      DOCTEST_CHECK( i[ 3 ] == 4 );
      DOCTEST_CHECK( i[ 4 ] == 2 );
      DOCTEST_CHECK( i[ 5 ] == 5 );
   }
}

#endif // DIP__ENABLE_DOCTEST

#endif // DIP_DIMENSIONARRAY_H
