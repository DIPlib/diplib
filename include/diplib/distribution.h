/*
 * DIPlib 3.0
 * This file contains declarations for histograms and related functionality
 *
 * (c)2018, Cris Luengo.
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

#include "diplib.h"

/// \file
/// \brief Histograms and related functionality.
/// \see histograms


namespace dip {


/// \ingroup analysis
/// \{


/// \brief Holds probability density functions and other types of distribution
///
/// This is a container class to hold results of certain type of analysis that compute
/// a property as a function of scale or intensity. Even though a histogram could fit
/// within this description, the `dip::Histogram` class is specifically meant to
/// hold histograms.
///
/// Distributions represent a function *y* of *x*, where *x* is not necessarily uniformly
/// spaced. Both *x* and *y* are stored as double-precision floating point values.
///
/// Elements can be inserted and modified in such a way that *x* is no longer sorted.
/// The `Sort` method applies a stable sort to restore the order.
class DIP_NO_EXPORT Distribution {
   public:
      struct ValueType {
         dfloat x = 0;
         dfloat y = 0;
      };
      using Container = std::vector< ValueType >;

      /// A distribution can be created by giving a size
      explicit Distribution( dip::uint size = 0 ) : data_( size ) {}
      /// A distribution can be created by giving an array of the x values
      explicit Distribution( std::vector< dfloat > const& x ) : data_( x.size() ) {
         for( dip::uint ii = 0; ii < x.size(); ++ii ) {
            data_[ ii ].x = x[ ii ];
         }
      }
      /// A distribution can be created by giving an array of the x values and one of the y values
      Distribution( std::vector< dfloat > const& x, std::vector< dfloat > const& y ) : data_( x.size() ) {
         DIP_THROW_IF( x.size() != y.size(), E::ARRAY_SIZES_DONT_MATCH );
         for( dip::uint ii = 0; ii < x.size(); ++ii ) {
            data_[ ii ].x = x[ ii ];
            data_[ ii ].y = y[ ii ];
         }
      }

      /// Checks whether the distribution is empty (size is 0)
      bool Empty() const noexcept { return data_.empty(); }
      /// Returns the size of the distribution (number of data points)
      dip::uint Size() const noexcept { return data_.size(); }
      /// Changes the size of the distribution (number of data points)
      void Resize( dip::uint size ) { data_.resize( size ); }
      /// Reserves memory space for the distribution, useful if growing it through `PushBack`
      void Reserve( dip::uint size ) { data_.reserve( size ); }

      /// Gets the *x* and *y* values at location `index`
      ValueType& operator[]( dip::uint index ) {
         DIP_THROW_IF( index >= Size(), E::PARAMETER_OUT_OF_RANGE );
         return data_[ index ];
      }
      /// Gets the *x* and *y* values at location `index`
      ValueType const& operator[]( dip::uint index ) const {
         DIP_THROW_IF( index >= Size(), E::PARAMETER_OUT_OF_RANGE );
         return data_[ index ];
      }

      /// Gets the *x* and *y* values at the end
      ValueType& Back() {
         DIP_THROW_IF( Empty(), "Attempting to access end element in empty distribution" );
         return data_.back();
      }
      /// Gets the *x* and *y* values at the end
      ValueType const& Back() const {
         DIP_THROW_IF( Empty(), "Attempting to access end element in empty distribution" );
         return data_.back();
      }
      /// Appends a pair of *x* and *y* values at the end
      void PushBack( dfloat x, dfloat y ) {
         data_.push_back( { x, y } );
      }

      /// Returns an iterator to the beginning
      Container::iterator begin() noexcept { return data_.begin(); }
      /// Returns an iterator to the beginning
      Container::const_iterator begin() const noexcept { return data_.begin(); }
      /// Returns an iterator to the end
      Container::iterator end() noexcept { return data_.end(); }
      /// Returns an iterator to the end
      Container::const_iterator end() const noexcept { return data_.end(); }

      /// Copies the *x* values to a new array
      std::vector< dfloat > X() const {
         std::vector< dfloat > x( Size() );
         for( dip::uint ii = 0; ii < Size(); ++ii ) {
            x[ ii ] = data_[ ii ].x;
         }
         return x;
      }
      /// Copies the *y* values to a new array
      std::vector< dfloat > Y() const {
         std::vector< dfloat > y( Size() );
         for( dip::uint ii = 0; ii < Size(); ++ii ) {
            y[ ii ] = data_[ ii ].y;
         }
         return y;
      }

      /// Sorts the data in the distribution according to the *x* values.
      Distribution& Sort() {
         std::stable_sort( data_.begin(), data_.end(), []( ValueType const& a, ValueType const& b ) { return a.x < b.x; } );
         return *this;
      }

      /// \brief Converts the distribution to a cumulative distribution, where each element is the sum of
      /// all elements up to that element in the original distribution (i.e. *x* spacing is ignored).
      Distribution& Cumulative() {
         for( dip::uint ii = 1; ii < Size(); ++ii ) {
            data_[ ii ].y += data_[ ii - 1 ].y;
         }
         return *this;
      }
      /// Computes the sum of the *y* values.
      dfloat Sum() const {
         dfloat sum = 0;
         for( auto& d : data_ ) {
            sum += d.y;
         }
         return sum;
      }
      /// Normalizes the sum of the *y* values.
      Distribution& NormalizeSum() {
         return ( *this /= Sum() );
      }

      /// \brief Converts the distribution to a cumulative distribution, where each element is the integral of
      /// the original distribution up to that element. Make sure the data is sorted (see `Sort`).
      Distribution& Integrate() {
         DIP_THROW_IF( Empty(), "Attempting to transform an empty distribution" );
         Container newData = data_;
         newData[ 0 ].y = 0;
         for( dip::uint ii = 1; ii < Size(); ++ii ) {
            newData[ ii ].y = newData[ ii - 1 ].y + ( data_[ ii ].y + data_[ ii - 1 ].y ) / 2 * ( data_[ ii ].x - data_[ ii - 1 ].x );
         }
         std::swap( data_, newData );
         return *this;
      }
      /// Computes the integral of the distribution. Make sure the data is sorted (see `Sort`).
      dfloat Integral() const {
         dfloat integral = 0;
         for( dip::uint ii = 1; ii < Size(); ++ii ) {
            integral += ( data_[ ii ].y + data_[ ii - 1 ].y ) / 2 * ( data_[ ii ].x - data_[ ii - 1 ].x );
         }
         return integral;
      }
      /// Normalizes the integral of the distribution values. Make sure the data is sorted (see `Sort`).
      Distribution& NormalizeIntegral() {
         return ( *this /= Integral() );
      }

      /// \brief Converts the cumulative distribution to a distribution, where each element is the derivative of
      /// the original distribution at that element. The optional `startValue` is the value for the leftmost bin.
      /// Make sure the data is sorted (see `Sort`).
      Distribution& Differentiate( dfloat startValue = 0 ) {
         DIP_THROW_IF( Empty(), "Attempting to transform an empty distribution" );
         Container newData = data_;
         newData[ 0 ].y = startValue;
         for( dip::uint ii = 1; ii < Size(); ++ii ) {
            newData[ ii ].y = 2 * ( data_[ ii ].y - data_[ ii - 1 ].y ) / ( data_[ ii ].x - data_[ ii - 1 ].x ) - newData[ ii - 1 ].y;
         }
         std::swap( data_, newData );
         return *this;
      }

      /// Adds two distributions. Their *x* values must match exactly.
      Distribution& operator+=( Distribution const& other ) {
         DIP_THROW_IF( Size() != other.Size(), E::ARRAY_SIZES_DONT_MATCH );
         for( dip::uint ii = 0; ii < Size(); ++ii ) {
            DIP_THROW_IF( data_[ ii ].x != other.data_[ ii ].x, "Distribution x values don't match" );
         }
         for( dip::uint ii = 0; ii < Size(); ++ii ) {
            data_[ ii ].y += other.data_[ ii ].y;
         }
         return *this;
      }

      /// Scales the distribution, multiplying each *y* value by `scale`.
      Distribution& operator*=( dfloat scale ) {
         for( auto& d : data_ ) {
            d.y *= scale;
         }
         return *this;
      }
      /// Scales the distribution, dividing each *y* value by `scale`.
      Distribution& operator/=( dfloat scale ) {
         return ( *this *= ( 1.0 / scale ));
      }

   private:
      Container data_;
};


/// \brief Writes the distribution to a stream
inline std::ostream& operator<<(
      std::ostream& os,
      Distribution const& distribution
) {
   for( auto const& sample : distribution ) {
      os << sample.x << " -> " << sample.y << '\n';
   }
   return os;
}


/// \}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/random.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Distribution" ) {
   dip::Distribution dist( 5 );
   DOCTEST_REQUIRE( dist.Size() == 5 );
   dist[ 0 ].x = 0.1;   dist[ 0 ].y = 1.5;
   dist[ 1 ].x = 0.2;   dist[ 1 ].y = 1.7;
   dist[ 2 ].x = 0.4;   dist[ 2 ].y = 2.4;
   dist[ 3 ].x = 0.7;   dist[ 3 ].y = 1.2;
   dist[ 4 ].x = 1.0;   dist[ 4 ].y = 0.8;
   DOCTEST_CHECK( dist[ 3 ].x == 0.7 );
   DOCTEST_CHECK( dist[ 3 ].y == 1.2 );
   dist.PushBack( 1.3, 1.3 );
   DOCTEST_CHECK( dist.Size() == 6 );
   auto dist2 = dist;
   dist2.Integrate();
   DOCTEST_CHECK( dist2.Back().x == 1.3 );
   DOCTEST_CHECK( dist2.Back().y == 1.725 );
   DOCTEST_CHECK( dist.Integral() == 1.725 );
   dist2.Differentiate( dist[ 0 ].y );
   DOCTEST_CHECK( dist2.Back().x == 1.3 );
   DOCTEST_CHECK( dist2.Back().y == doctest::Approx( dist.Back().y ));
}

#endif // DIP__ENABLE_DOCTEST

#endif //DIP_DISTRIBUTION_H
