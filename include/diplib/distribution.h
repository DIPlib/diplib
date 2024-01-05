/*
 * (c)2018-2024, Cris Luengo.
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

#ifndef DIP_DISTRIBUTION_H
#define DIP_DISTRIBUTION_H

#include <algorithm>
#include <iterator>
#include <numeric>
#include <ostream>
#include <type_traits>
#include <vector>

#include "diplib.h"

/// \file
/// \brief Distributions and related functionality.
/// See \ref analysis.


namespace dip {

class DIP_NO_EXPORT Histogram;

/// \addtogroup analysis


/// \brief Holds probability density functions and other types of distribution
///
/// This is a container class to hold results of certain type of analysis that compute
/// a property as a function of scale or intensity. Even though a histogram could fit
/// within this description, the \ref dip::Histogram class is specifically meant to
/// hold histograms, and purposefully kept separate from this class. A `dip::Histogram`
/// with a 1D histogram can be cast to a `dip::Distribution`.
///
/// Distributions represent a function *y* of *x*, where *x* is not necessarily uniformly
/// spaced. Both *x* and *y* are stored as double-precision floating point values.
/// The distribution can also be a multi-valued function, with multiple *y* values for
/// every *x* value. In this case, the *y* values for each *x* are arranged as a 2D matrix
/// *NxM*, where *M* is 1 for a vector-like set of values.
///
/// Elements can be modified in such a way that *x* is no longer sorted.
/// The `Sort` method applies a stable sort to restore the order.
class DIP_NO_EXPORT Distribution {
   public:
      /// Data type of values stored in container
      using ValueType = dfloat;
      /// Container used internally to store the data
      using Container = std::vector< ValueType >;

      /// \brief One sample of a distribution.
      ///
      /// A sample implicitly converts to \ref dip::dfloat, evaluating to the value of *y* for
      /// the sample. For multi-valued samples, evaluates to the first *y* value.
      ///
      /// Given a `Sample` `s`, `s.X()` accesses the *x*-value of the sample, `s.Y()` accesses
      /// the (first) *y*-value, `s.Y(ii)` accesses the `(ii+1)`-th *y* value, and `s.Y(ii,jj)` accesses
      /// the element at (`ii`,`jj`). `s.Y(ii,jj)` is equivalent to `s.Y(ii+jj*nRows)`, where `nRows`
      /// is the number of rows in the matrix. None of these accessors test for out-of-bounds accesses.
      ///
      /// !!! warning
      ///     Note that `Sample` references the \ref dip::Distribution it is created from, which must therefore
      ///     exist while the sample is used. Copying a `Sample` does not create new storage, the copy will
      ///     reference the same data. Therefore, things like the following will not work:
      ///
      ///         :::cpp
      ///         Distribution d = ...;
      ///         auto tmp = d[ 2 ];
      ///         d[ 2 ] = d[ 3 ];    // `tmp` changes along with `d[ 2 ]`, since they reference the same data
      ///         d[ 3 ] = tmp;
      ///
      ///     Instead, use `swap`. Do note that some algorithms in the C++ Standard Library expect the above
      ///     to work.
      class DIP_NO_EXPORT Sample {
            friend class Distribution;
            friend class Iterator;
         public:
            /// Not default constructable
            Sample() = delete;
            /// Move constructor
            Sample( Sample&& ) = default;
            /// \brief Copy constructor, references the same data. Careful!
            Sample( Sample const& ) = default;
            /// Move assignment actually copies value over to sample referenced
            Sample& operator=( Sample&& other ) { // NOLINT(*-noexcept-move-operations, *-noexcept-move-constructor, *-exception-escape)
               *this = other; // Call copy assignment
               return *this;
            }
            /// Copy assignment, data is copied to sample referenced
            Sample& operator=( Sample const& other ) { // NOLINT(*-unhandled-self-assignment)
               DIP_ASSERT( distribution_->ValuesPerSample() == other.distribution_->ValuesPerSample() );
               ValueType const* src = other.distribution_->data_.data() + other.index_;
               ValueType* dest = distribution_->data_.data() + index_;
               std::copy( src, src + distribution_->ValuesPerSample() + 1, dest );
               return *this;
            };
            // Destructor
            ~Sample() = default;
            /// Returns reference to sample's *x* value.
            ValueType& X() {
               return distribution_->data_[ index_ ];
            }
            /// Returns reference to sample's `(index+1)`-th *y* value.
            ValueType& Y( dip::uint index = 0 ) {
               return distribution_->data_[ index_ + index + 1 ];
            }
            /// Returns reference to sample's *y* value at (`row`,`col`).
            ValueType& Y( dip::uint row, dip::uint col ) {
               return distribution_->data_[ index_ + 1 + row + col * distribution_->nRows_ ];
            }
            /// Implicitly casts to sample's first *y* value.
            operator dfloat() const {
               return distribution_->data_[ index_ + 1 ];
            }
            /// \brief Swaps two samples, copying the data from `other` to `*this`, and that from `*this` to `other`.
            /// Both must have the same number of values.
            void swap( Sample& other ) { // NOLINT(*-noexcept-swap, *-exception-escape)
               DIP_ASSERT( distribution_->ValuesPerSample() == other.distribution_->ValuesPerSample() );
               ValueType* ptr1 = distribution_->data_.data() + index_;
               ValueType* ptr2 = other.distribution_->data_.data() + other.index_;
               for( dip::uint ii = 0; ii < distribution_->ValuesPerSample() + 1; ++ii ) {
                  std::swap( ptr1[ ii ], ptr2[ ii ] );
               }
            }
            /// \brief Swaps two samples, copying the data from `other` to `*this`, and that from `*this` to `other`.
            /// Both must have the same number of values.
            friend void swap( Sample& a, Sample& b ) { // NOLINT(*-noexcept-swap, *-exception-escape)
               a.swap( b );
            }
         private:
            Distribution* distribution_;
            dip::uint index_; // points to the first data element for this sample
            // Private constructor, only friend classes can make one of these.
            Sample( Distribution* distribution, dip::uint index )
                  : distribution_( distribution ), index_( index * distribution_->Stride() ) {}
      };

      /// \brief One unmutable sample of a distribution, see \ref Sample for details.
      class DIP_NO_EXPORT ConstSample {
            friend class Distribution;
            friend class Iterator;
         public:
            /// Not default constructable
            ConstSample() = delete;
            /// Move constructor
            ConstSample( ConstSample&& ) = default;
            /// \brief Copy constructor, references the same data. Careful!
            ConstSample( ConstSample const& ) = default;
            /// Not assignable
            ConstSample& operator=( ConstSample&& other ) = delete;
            /// Not assignable
            ConstSample& operator=( ConstSample const& other ) = delete;
            // Destructor
            ~ConstSample() = default;
            /// Returns sample's *x* value.
            ValueType X() const {
               return distribution_->data_[ index_ ];
            }
            /// Returns sample's `(index+1)`-th *y* value.
            ValueType Y( dip::uint index = 0 ) const {
               return distribution_->data_[ index_ + index + 1 ];
            }
            /// Returns sample's *y* value at (`row`,`col`).
            ValueType Y( dip::uint row, dip::uint col ) const {
               return distribution_->data_[ index_ + 1 + row + col * distribution_->nRows_ ];
            }
            /// Implicitly casts to sample's first *y* value.
            operator dfloat() const {
               return distribution_->data_[ index_ + 1 ];
            }
         private:
            Distribution const* distribution_;
            dip::uint index_; // points to the first data element for this sample
            // Private constructor, only friend classes can make one of these.
            ConstSample( Distribution const* distribution, dip::uint index )
                  : distribution_( distribution ), index_( index * distribution_->Stride() ) {}
      };

      /// \brief An iterator for `dip::Distribution`. Dereferences into a `Sample` or a `ConstSample` (the value of `T`).
      template< typename T >
      class DIP_NO_EXPORT IteratorTemplate {
            friend class Distribution;

         public:
            /// Iterator category
            using iterator_category = std::random_access_iterator_tag;
            /// The data type obtained when dereferencing the iterator
            using value_type = T;
            /// The type of difference between iterators
            using difference_type = dip::sint;
            /// The type of a reference to a sample
            using reference = value_type&;
            /// The type of a pointer to a sample
            using pointer = value_type*;

            /// Not default constructable
            IteratorTemplate() = delete; // TODO: random access iterator requires default initialization
            //IteratorTemplate() : sample_{ nullptr, 0 }, stride_( 0 ) {}
            /// Move constructor
            IteratorTemplate( IteratorTemplate&& ) = default;
            /// Copy constructor
            IteratorTemplate( IteratorTemplate const& ) = default;
            /// Move assignment, identical to copy assignment
            IteratorTemplate& operator=( IteratorTemplate&& other ) noexcept {
               operator=( other ); // Call copy assignment
               return *this;
            }
            /// Copy assignment
            IteratorTemplate& operator=( IteratorTemplate const& other ) noexcept {
               // Copy assignment cannot be defaulted because copy assignment of Sample is not wanted here
               sample_.distribution_ = other.sample_.distribution_;
               sample_.index_ = other.sample_.index_;
               stride_ = other.stride_;
               return *this;
            };
            // Destructor
            ~IteratorTemplate() = default;

            /// Dereference
            value_type& operator*() noexcept { return sample_; }
            /// Dereference
            value_type* operator->() noexcept { return &sample_; }

            /// Return sample `index` values away from the current location
            template< typename I, typename = std::enable_if_t< IsIndexingType< I >::value >>
            value_type operator[]( I index ) const {
               auto out = sample_;
               out.index_ = static_cast< dip::uint >( static_cast< dip::sint >( out.index_ ) + static_cast< dip::sint >( index ) * stride_ );
               return out;
            }

            /// Pre-increment
            IteratorTemplate& operator++() {
               sample_.index_ += stride_;
               return *this;
            }
            /// Pre-decrement
            IteratorTemplate& operator--() {
               sample_.index_ -= stride_;
               return *this;
            }
            /// Post-increment
            IteratorTemplate operator++( int ) {
               IteratorTemplate tmp( *this );
               operator++();
               return tmp;
            }
            /// Post-decrement
            IteratorTemplate operator--( int ) {
               IteratorTemplate tmp( *this );
               operator++();
               return tmp;
            }

            /// Move iterator forward by `index` elements
            template< typename I, typename = std::enable_if_t< IsIndexingType< I >::value >>
            IteratorTemplate& operator+=( I index ) {
               sample_.index_ = static_cast< dip::uint >(
                     static_cast< dip::sint >( sample_.index_ )
                     + static_cast< dip::sint >( index ) * static_cast< dip::sint >( stride_ ));
               return *this;
            }
            /// Move iterator backward by `index` elements
            template< typename I, typename = std::enable_if_t< IsIndexingType< I >::value >>
            IteratorTemplate& operator-=( I index ) {
               return operator+=( -static_cast< dip::sint >( index ));
            }
            /// Returns new iterator moved forward by `n` elements
            template< typename I, typename = std::enable_if_t< IsIndexingType< I >::value >>
            IteratorTemplate operator+( I n ) {
               IteratorTemplate tmp( *this );
               tmp += n;
               return tmp;
            }
            /// Returns new iterator moved backward by `n` elements
            template< typename I, typename = std::enable_if_t< IsIndexingType< I >::value >>
            IteratorTemplate operator-( I n ) {
               IteratorTemplate tmp( *this );
               tmp -= n;
               return tmp;
            }
            /// Returns distance between two iterators
            difference_type operator-( IteratorTemplate const& it ) const {
               return ( static_cast< dip::sint >( sample_.index_ ) - static_cast< dip::sint >( it.sample_.index_ )) / static_cast< dip::sint >( stride_ );
            }

            /// Equality comparison
            bool operator==( IteratorTemplate const& other ) const {
               return ( sample_.distribution_ == other.sample_.distribution_ ) && ( sample_.index_ == other.sample_.index_ );
            }
            /// Inequality comparison
            bool operator!=( IteratorTemplate const& other ) const {
               return !operator==( other );
            }

         private:
            value_type sample_;
            dip::uint stride_;
            using DistributionType = std::remove_pointer_t< decltype( value_type::distribution_ ) >;
            explicit IteratorTemplate( DistributionType& distribution ) noexcept
                  : sample_( &distribution, 0 ), stride_( distribution.Stride() ) {}
            explicit IteratorTemplate( DistributionType& distribution, int /*isEnd*/) noexcept // A second argument, ignored, creates an end iterator
                  : sample_( &distribution, distribution.Size() ), stride_( distribution.Stride() ) {}
      };

      /// \brief An iterator for `dip::Distribution`. Dereferences into a `Sample`.
      using Iterator = IteratorTemplate< Sample >;

      /// \brief An iterator for `const dip::Distribution`. Dereferences into a `ConstSample`.
      using ConstIterator = IteratorTemplate< ConstSample >;

      /// A zero-initialized distribution can be created by giving a size, and number of values (or rows and columns) per sample
      explicit Distribution( dip::uint size = 0, dip::uint rows = 1, dip::uint columns = 1 )
            : length_( size ), nRows_( rows ), nColumns_( columns ), data_( length_ * ( rows * columns + 1 )) {}
      /// A zero-initialized distribution can be created by giving an array of the *x* values, and number of values (or rows and columns) per sample
      explicit Distribution( std::vector< dfloat > const& x, dip::uint rows = 1, dip::uint columns = 1 )
            : length_( x.size() ), nRows_( rows ), nColumns_( columns ), data_( length_ * ( rows * columns + 1 )) {
         dip::uint stride = Stride();
         for( dip::uint ii = 0; ii < length_; ++ii ) {
            data_[ ii * stride ] = x[ ii ];
         }
      }
      /// A distribution can be created by giving an array of the *x* values and an array of the *y* values
      Distribution( std::vector< dfloat > const& x, std::vector< dfloat > const& y )
            : length_( x.size() ), nRows_( 1 ), nColumns_( 1 ), data_( length_ * 2 ) {
         DIP_THROW_IF( x.size() != y.size(), E::ARRAY_SIZES_DONT_MATCH );
         for( dip::uint ii = 0; ii < length_; ++ii ) {
            data_[ ii * 2 ] = x[ ii ];
            data_[ ii * 2 + 1 ] = y[ ii ];
         }
      }
      /// A 1D \ref dip::Histogram can be cast to a `dip::Distribution`
      DIP_EXPORT explicit Distribution( Histogram const& histogram );

      /// Checks whether the distribution is empty (size is 0)
      bool Empty() const noexcept {
         return length_ == 0;
      }
      /// Returns the size of the distribution (number of data points)
      dip::uint Size() const noexcept {
         return length_;
      }

      /// Returns the number of *y* values per sample
      dip::uint ValuesPerSample() const {
         return nRows_ * nColumns_;
      }
      /// Returns the number of rows in the matrix of *y* values
      dip::uint Rows() const {
         return nRows_;
      }
      /// Returns the number of columns in the matrix of *y* values
      dip::uint Columns() const {
         return nColumns_;
      }

      /// Returns the units used along the *x* axis.
      Units const& XUnits() const {
         return units_;
      }
      /// Returns a modifiable reference to the units used along the *x* axis.
      Units& XUnits() {
         return units_;
      }

      /// Gets the *x* and *y* values at location `index`
      Sample operator[]( dip::uint index ) {
         DIP_THROW_IF( index >= Size(), E::INDEX_OUT_OF_RANGE );
         return { this, index };
      }
      /// Gets the *x* and *y* values at location `index`
      ConstSample operator[]( dip::uint index ) const {
         DIP_THROW_IF( index >= Size(), E::INDEX_OUT_OF_RANGE );
         return { this, index };
      }

      /// Gets the *x* and *y* values at the end
      Sample Back() {
         DIP_THROW_IF( Empty(), "Attempting to access last element in an empty distribution" );
         return { this, ( Size() - 1 ) };
      }
      /// Gets the *x* and *y* values at the end
      ConstSample Back() const {
         DIP_THROW_IF( Empty(), "Attempting to access last element in an empty distribution" );
         return { this, ( Size() - 1 ) };
      }

      /// Returns an iterator to the beginning
      Iterator begin() noexcept { return Iterator( *this ); }
      /// Returns an iterator to the beginning
      ConstIterator begin() const noexcept { return ConstIterator( *this ); }
      /// Returns an iterator to the end
      Iterator end() noexcept { return Iterator( *this, 0 ); }
      /// Returns an iterator to the end
      ConstIterator end() const noexcept { return ConstIterator( *this, 0 ); }

      /// Returns an *x*-value iterator to the beginning
      SampleIterator< ValueType > Xbegin() noexcept {
         return { data_.data(), static_cast< dip::sint >( Stride() ) };
      }
      /// Returns an *x*-value iterator to the beginning
      ConstSampleIterator< ValueType > Xbegin() const noexcept {
         return { data_.data(), static_cast< dip::sint >( Stride() ) };
      }
      /// Returns an *x*-value iterator to the end
      SampleIterator< ValueType > Xend() noexcept {
         return  Xbegin() + Size();
      }
      /// Returns an *x*-value iterator to the end
      ConstSampleIterator< ValueType > Xend() const noexcept {
         return  Xbegin() + Size();
      }

      /// Returns an *y*-value iterator to the beginning
      SampleIterator< ValueType > Ybegin( dip::uint index = 0 ) {
         DIP_THROW_IF( index >= ValuesPerSample(), E::INDEX_OUT_OF_RANGE );
         return { data_.data() + index + 1, static_cast< dip::sint >( Stride() ) };
      }
      /// Returns an *y*-value iterator to the beginning
      ConstSampleIterator< ValueType > Ybegin( dip::uint index = 0 ) const {
         DIP_THROW_IF( index >= ValuesPerSample(), E::INDEX_OUT_OF_RANGE );
         return { data_.data() + index + 1, static_cast< dip::sint >( Stride() ) };
      }
      /// Returns an *y*-value iterator to the end
      SampleIterator< ValueType > Yend( dip::uint index = 0 ) {
         return  Ybegin( index ) + Size();
      }
      /// Returns an *y*-value iterator to the end
      ConstSampleIterator< ValueType > Yend( dip::uint index = 0 ) const {
         return  Ybegin( index ) + Size();
      }

      /// Copies the *x* values to a new array
      std::vector< dfloat > X() const {
         std::vector< dfloat > x( Size() );
         std::copy( Xbegin(), Xend(), x.begin() );
         return x;
      }
      /// Copies the *y* values to a new array
      std::vector< dfloat > Y( dip::uint index = 0 ) const {
         std::vector< dfloat > y( Size() );
         std::copy( Ybegin( index ), Yend( index ), y.begin() );
         return y;
      }

      /// Sorts the data in the distribution according to the *x* values.
      DIP_EXPORT Distribution& Sort();

      /// \brief Converts the distribution to a cumulative distribution, where each element is the sum of
      /// all elements up to that element in the original distribution (i.e. *x* spacing is ignored).
      DIP_EXPORT Distribution& Cumulative();

      /// Computes the sum of the *y* values.
      dfloat Sum( dip::uint index = 0 ) const {
         return std::accumulate( Ybegin( index ), Yend( index ), 0.0 );
      }

      /// Normalizes the sum of the *y* values.
      DIP_EXPORT Distribution& NormalizeSum();

      /// \brief Converts the distribution to a cumulative distribution, where each element is the integral of
      /// the original distribution up to that element. Make sure the data are sorted (see `Sort`).
      DIP_EXPORT Distribution& Integrate();

      /// Computes the integral of the distribution. Make sure the data are sorted (see `Sort`).
      DIP_EXPORT dfloat Integral( dip::uint index = 0 ) const;

      /// Normalizes the integral of the distribution values. Make sure the data are sorted (see `Sort`).
      DIP_EXPORT Distribution& NormalizeIntegral();

      /// \brief Converts the cumulative distribution to a distribution, where each element is the derivative of
      /// the original distribution at that element. Make sure the data are sorted (see `Sort`).
      DIP_EXPORT Distribution& Differentiate();
      
      /// \brief Computes the most likely *x* values. In essence, returns the *x* values which maximize
      /// the corresponding *y* value.
      DIP_EXPORT Container MaximumLikelihood();

      /// Adds two distributions. Their *x* values must match exactly.
      DIP_EXPORT Distribution& operator+=( Distribution const& other );

      /// Scales the distribution, multiplying each *y* value by `scale`.
      DIP_EXPORT Distribution& operator*=( dfloat scale );

      /// Scales the distribution, dividing each *y* value by `scale`.
      Distribution& operator/=( dfloat scale ) {
         return ( *this *= ( 1.0 / scale ));
      }

      /// \brief Fills the values for the *x* axis, starting with `pixelSize[0].magnitude * offset`,
      /// and linear increments of `pixelSize[0].magnitude * scaling`, if the `pixelSize` is isotropic and has
      /// physical units. The `XUnits` are also set to the units of the isotropic `pixelSize`, or otherwise
      /// to "px".
      DIP_EXPORT void SetSampling(
            PixelSize const& pixelSize = {},
            dfloat offset = 0.0,
            dfloat scaling = 1.0
      );


   private:
      dip::uint length_;   // Number of samples
      dip::uint nRows_;    // Number of rows in the matrix of y values, 1 for scalar distributions
      dip::uint nColumns_; // Number of columns in the matrix of y values, 1 for scalar or vector-valued distributions
      Container data_; // `( 1 + nRows_ * nColumns_ ) * length_` elements
      Units units_ = Units{};
      // data_[ ii * Stride() ] -> x
      // data_[ ii * Stride() + 1 ] -> y[0,0]
      // data_[ ii * Stride() + nRows_ * nColumns_ ] -> y[N,N]
      // matrix stored column-wise, as usual

      dip::uint Stride() const {
         return 1 + nRows_ * nColumns_;
      }
};


/// \brief Writes the distribution to a stream
/// \relates dip::Distribution
inline std::ostream& operator<<(
      std::ostream& os,
      Distribution const& distribution
) {
   for( auto const& sample : distribution ) {
      dip::uint nValues = distribution.ValuesPerSample();
      os << sample.X() << ' ' <<  distribution.XUnits() << " -> " << sample.Y( 0 );
      for( dip::uint jj = 1; jj < nValues; ++jj ) {
         os << ", " << sample.Y( jj );
      }
      os << '\n';
   }
   return os;
}


/// \endgroup

} // namespace dip

#endif //DIP_DISTRIBUTION_H
