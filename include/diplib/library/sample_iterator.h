/*
 * DIPlib 3.0
 * This file contains definitions for the sample iterator.
 *
 * (c)2016-2017, Cris Luengo.
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


#ifndef DIP_SAMPLE_ITERATOR_H
#define DIP_SAMPLE_ITERATOR_H

#include "diplib/library/types.h"


/// \file
/// \brief The `dip::SampleIterator` class. This file is always included through `diplib.h`.
/// \see iterators


namespace dip {


/// \addtogroup iterators
/// \{


/// \brief An iterator to iterate over samples in a tensor, or pixels on an image line.
///
/// This is the simplest iterator available in this library, and is most like working with
/// a pointer to a data segment. The only difference with a pointer is that the data
/// stride is taken into account.
///
/// Satisfies all the requirements for a mutable [RandomAccessIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// This means that you can increment and decrement the iterator, add or subtract an
/// integer from it, dereference it, index it using the `[]` operator, as well as compare
/// two iterators or take the difference between them (as long as they reference samples
/// within the same data segment). It is default constructible and swappable, but the
/// default constructed iterator is invalid and should not be dereferenced.
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, JointImageIterator, LineIterator
template< typename T >
class DIP_NO_EXPORT SampleIterator {
   public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = T;               ///< The data type of the sample, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a sample
      using pointer = T*;                 ///< The type of a pointer to a sample

      /// Default constructor yields an invalid iterator that cannot be dereferenced
      SampleIterator() : stride_( 1 ), ptr_( nullptr ) {}
      /// To construct a useful iterator, provide a pointer and a stride
      SampleIterator( pointer ptr, dip::sint stride = 1 ) : stride_( stride ), ptr_( ptr ) {}
      /// Swap
      void swap( SampleIterator& other ) {
         using std::swap;
         swap( stride_, other.stride_ );
         swap( ptr_, other.ptr_ );
      }
      /// Convert from non-const iterator to const iterator
      operator SampleIterator< value_type const >() const {
         return SampleIterator< value_type const >( ptr_, stride_ );
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index
      template< typename I, typename std::enable_if< IsIndexingType< I >::value, int >::type = 0 >
      reference operator[]( I index ) const { return *( ptr_ + static_cast< difference_type >( index ) * stride_ ); }
      /// Increment
      SampleIterator& operator++() {
         ptr_ += stride_;
         return *this;
      }
      /// Decrement
      SampleIterator& operator--() {
         ptr_ -= stride_;
         return *this;
      }
      /// Increment
      SampleIterator operator++( int ) {
         SampleIterator tmp( *this );
         ptr_ += stride_;
         return tmp;
      }
      /// Decrement
      SampleIterator operator--( int ) {
         SampleIterator tmp( *this );
         ptr_ -= stride_;
         return tmp;
      }
      /// Add integer
      template< typename I, typename std::enable_if< IsIndexingType< I >::value, int >::type = 0 >
      SampleIterator& operator+=( I index ) {
         ptr_ += static_cast< difference_type >( index ) * stride_;
         return *this;
      }
      /// Subtract integer
      template< typename I, typename std::enable_if< IsIndexingType< I >::value, int >::type = 0 >
      SampleIterator& operator-=( I index ) {
         ptr_ -= static_cast< difference_type >( index ) * stride_;
         return *this;
      }
      /// Difference between iterators
      difference_type operator-( SampleIterator const& it ) const {
         return ( ptr_ - it.ptr_ ) / stride_;
      }
      // Test returns false if the iterator cannot be dereferenced (is a null pointer)
      explicit operator bool() const { return ptr_ != nullptr; }
      // Returns the stride
      dip::sint Stride() const { return stride_; }
      // Returns the pointer
      pointer Pointer() const { return ptr_; }
      /// Equality comparison
      bool operator==( SampleIterator const& other ) const { return ptr_ == other.ptr_; }
      /// Inequality comparison
      bool operator!=( SampleIterator const& other ) const { return ptr_ != other.ptr_; }
      /// Larger than comparison
      bool operator>( SampleIterator const& other ) const { return ptr_ > other.ptr_; }
      /// Smaller than comparison
      bool operator<( SampleIterator const& other ) const { return ptr_ < other.ptr_; }
      /// Not smaller than comparison
      bool operator>=( SampleIterator const& other ) const { return ptr_ >= other.ptr_; }
      /// Not larger than comparison
      bool operator<=( SampleIterator const& other ) const { return ptr_ <= other.ptr_; }
   private:
      dip::sint stride_;
      pointer ptr_;
};

/// \brief Add integer to a sample iterator
template< typename T, typename I, typename std::enable_if< IsIndexingType< I >::value, int >::type = 0 >
inline SampleIterator< T > operator+( SampleIterator< T > it, I n ) {
   it += n;
   return it;
}
/// \brief Subtract integer from a sample iterator
template< typename T, typename I, typename std::enable_if< IsIndexingType< I >::value, int >::type = 0 >
inline SampleIterator< T > operator-( SampleIterator< T > it, I n ) {
   it -= n;
   return it;
}

template< typename T >
inline void swap( SampleIterator< T >& v1, SampleIterator< T >& v2 ) {
   v1.swap( v2 );
}

/// \brief A const iterator to iterate over samples in a tensor, or pixels on an image line.
///
/// This iterator is identical to `dip::SampleIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [RandomAccessIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstSampleIterator = SampleIterator< T const >;


/// \}

} // namespace dip

#endif // DIP_SAMPLE_ITERATOR_H
