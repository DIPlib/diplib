/*
 * DIPlib 3.0
 * This file contains definitions for distributions and related functionality
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

#include "diplib.h"
#include "diplib/distribution.h"
#include "diplib/histogram.h"

namespace dip {

Distribution::Distribution( Histogram const& histogram ) {
   DIP_THROW_IF( histogram.Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
   Image const& histImg = histogram.GetImage();
   DIP_ASSERT( histImg.IsForged() );
   DIP_ASSERT( histImg.Stride( 0 ) == 1 );
   length_ = histImg.Size( 0 );
   nRows_ = nColums_ = 1;
   data_.resize( length_ * 2 );
   auto bins = histogram.BinCenters();
   auto src = static_cast< Histogram::CountType const* >( histImg.Origin() );
   dfloat* dest = data_.data();
   for( dip::uint ii = 0; ii < length_; ++ii ) {
      *dest = bins[ ii ];
      ++dest;
      *dest = static_cast< dfloat >( *src );
      ++dest;
      ++src;
   }
}

Distribution& Distribution::Sort() {
   // -- std::stable_sort doesn't work: it wants to use a temporary variable, it doens't just swap elements
   //std::stable_sort( begin(), end(), []( Sample const& a, Sample const& b ) { return a.X() < b.X(); } );
   // -- Instead using insertion sort because it's easy to implement
   auto first = begin();
   auto last = end();
   for( auto it = first + 1; it != last; ++it ) {
      auto it2 = it;
      auto it3 = it - 1;
      while(( it2 != first ) && ( it3->X() > it2->X() )) {
         swap( *it3, *it2 );
         it2 = it3;
         --it3;
      }
   }
   return *this;
}

Distribution& Distribution::Cumulative() {
   dip::uint nValues = ValuesPerSample();
   dip::uint stride = Stride();
   dfloat* ptr = data_.data();
   for( dip::uint ii = 1; ii < Size(); ++ii ) {
      ++ptr; // skip x values
      for( dip::uint index = 0; index < nValues; ++index, ++ptr ) {
         *ptr += *( ptr - stride );
      }
   }
   return *this;
}

Distribution& Distribution::NormalizeSum() {
   dip::uint nValues = ValuesPerSample();
   for( dip::uint index = 0; index < nValues; ++index ) {
      dfloat sum = Sum( index );
      if( sum != 0 ) {
         for( auto it = Ybegin( index ); it != Yend( index ); ++it ) {
            *it /= sum;
         }
      }
   }
   return *this;
}

Distribution& Distribution::Integrate() {
   DIP_THROW_IF( Empty(), "Attempting to integrate an empty distribution" );
   Container newData = data_;
   dip::uint nValues = ValuesPerSample();
   dip::uint stride = Stride();
   // First output element must be 0
   dfloat const* src = data_.data();
   dfloat* dest = newData.data();
   dfloat prevX = *src;
   ++src; ++dest; // skip x values
   for( dip::uint index = 0; index < nValues; ++index, ++dest ) {
      *dest = 0;
   }
   src += nValues;
   // Next output elements computed by trapezoidal rule
   for( dip::uint ii = 1; ii < Size(); ++ii ) {
      dfloat curX = *src;
      ++src; ++dest; // skip x values
      for( dip::uint index = 0; index < nValues; ++index, ++src, ++dest ) {
         *dest = *( dest - stride ) + ( *src + *( src - stride )) / 2 * ( curX - prevX );
      }
      prevX = curX;
   }
   std::swap( data_, newData );
   return *this;
}

dfloat Distribution::Integral( dip::uint index ) const {
   dip::uint stride = Stride();
   dfloat const* ptr = data_.data() + stride;
   // Computed by trapezoidal rule
   dfloat integral = 0;
   for( dip::uint ii = 1; ii < Size(); ++ii, ptr += stride ) {
      integral += ( ptr[ index + 1 ] + ( ptr - stride )[ index + 1 ] ) / 2 * ( *ptr - *( ptr - stride ));
   }
   return integral;
}

Distribution& Distribution::NormalizeIntegral() {
   dip::uint nValues = ValuesPerSample();
   for( dip::uint index = 0; index < nValues; ++index ) {
      dfloat integral = Integral( index );
      for( auto it = Ybegin( index ); it != Yend( index ); ++it ) {
         *it /= integral;
      }
   }
   return *this;
}

Distribution& Distribution::Differentiate() {
   DIP_THROW_IF( Size() < 2, "Attempting to differentiate a distribution with 0 or 1 sample" );
   Container newData = data_;
   dip::uint nValues = ValuesPerSample();
   dip::uint stride = Stride();
   dfloat const* src = data_.data();
   dfloat* dest = newData.data();
   // First element is right derivative
   dfloat curX = *src;
   dfloat nextX = *( src + stride );
   ++src; ++dest; // skip x values
   for( dip::uint index = 0; index < nValues; ++index, ++src, ++dest ) {
      *dest = ( *( src + stride ) - *src ) / ( nextX - curX );
   }
   dfloat prevX = curX;
   // Next output elements computed by averaging left and right derivatives
   for( dip::uint ii = 1; ii < Size() - 1; ++ii ) {
      curX = *src;
      nextX = *( src + stride );
      ++src; ++dest; // skip x values
      for( dip::uint index = 0; index < nValues; ++index, ++src, ++dest ) {
         *dest = (( *( src + stride ) - *src ) / ( nextX - curX )
                  + ( *src - *( src - stride )) / ( curX - prevX )) * 0.5;
      }
      prevX = curX;
   }
   // Last element is left derivative
   curX = *src;
   ++src; ++dest; // skip x values
   for( dip::uint index = 0; index < nValues; ++index, ++src, ++dest ) {
      *dest = ( *src - *( src - stride ) ) / ( curX - prevX );
   }
   // Done!
   std::swap( data_, newData );
   return *this;
}

Distribution& Distribution::operator+=( Distribution const& other ) {
   DIP_THROW_IF( Size() != other.Size(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF(( Rows() != other.Rows()) || ( Columns() != other.Columns()), E::ARRAY_SIZES_DONT_MATCH );
   dip::uint nValues = ValuesPerSample();
   dfloat* thisPtr = data_.data();
   dfloat const* otherPtr = other.data_.data();
   for( dip::uint ii = 0; ii < Size(); ++ii ) {
      DIP_THROW_IF( *thisPtr != *otherPtr, "Distribution x values don't match" );
      ++thisPtr;
      ++otherPtr;
      for( dip::uint jj = 0; jj < nValues; ++jj, ++thisPtr, ++otherPtr ) {
         *thisPtr += *otherPtr;
      }
   }
   return *this;
}

Distribution& Distribution::operator*=( dfloat scale ) {
   dip::uint nValues = ValuesPerSample();
   dfloat* ptr = data_.data();
   for( dip::uint ii = 0; ii < Size(); ++ii ) {
      ++ptr; // skip *x*
      for( dip::uint jj = 0; jj < nValues; ++jj, ++ptr ) {
         *ptr *= scale;
      }
   }
   return *this;
}

void Distribution::SetSampling(
      PixelSize const& pixelSize,
      dfloat offset,
      dfloat scaling
) {
   if( pixelSize.IsPhysical() && pixelSize.IsIsotropic() ) {
      XUnits() = pixelSize[ 0 ].units;
      scaling *= pixelSize[ 0 ].magnitude;
      offset *= pixelSize[ 0 ].magnitude;
   } else {
      XUnits() = Units::Pixel();
   }
   dfloat value = 0;
   for( auto it = Xbegin(); it != Xend(); ++it, ++value ) {
      *it = offset + value * scaling;
   }
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/random.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Distribution" ) {
   // Scalar distribution
   dip::Distribution dist( 5 );
   DOCTEST_REQUIRE( dist.Size() == 5 );
   dist[ 0 ].X() = 0.2;   dist[ 0 ].Y() = 1.7;
   dist[ 1 ].X() = 0.1;   dist[ 1 ].Y() = 1.5;
   dist[ 2 ].X() = 1.0;   dist[ 2 ].Y() = 0.8;
   dist[ 3 ].X() = 0.7;   dist[ 3 ].Y() = 1.2;
   dist[ 4 ].X() = 0.4;   dist[ 4 ].Y() = 2.4;
   DOCTEST_CHECK( dist[ 2 ].X() == 1.0 ); DOCTEST_CHECK( dist[ 2 ].Y() == 0.8 );
   DOCTEST_CHECK( dist[ 3 ].X() == 0.7 ); DOCTEST_CHECK( dist[ 3 ].Y() == 1.2 );
   DOCTEST_CHECK( dist[ 4 ].X() == 0.4 ); DOCTEST_CHECK( dist[ 4 ].Y() == 2.4 );
   // 2x2 matrix distribution
   dist = dip::Distribution( 5, 2, 2 );
   dist[ 0 ].X() = 0.2;   dist[ 0 ].Y( 0, 0 ) = 1.7;   dist[ 0 ].Y( 1, 0 ) = 1.6;   dist[ 0 ].Y( 0, 1 ) = 1.5;   dist[ 0 ].Y( 1, 1 ) = 1.4;
   dist[ 1 ].X() = 0.1;   dist[ 1 ].Y( 0, 0 ) = 1.5;   dist[ 1 ].Y( 1, 0 ) = 1.4;   dist[ 1 ].Y( 0, 1 ) = 1.3;   dist[ 1 ].Y( 1, 1 ) = 1.2;
   dist[ 2 ].X() = 1.0;   dist[ 2 ].Y( 0, 0 ) = 0.8;   dist[ 2 ].Y( 1, 0 ) = 0.7;   dist[ 2 ].Y( 0, 1 ) = 0.6;   dist[ 2 ].Y( 1, 1 ) = 0.5;
   dist[ 3 ].X() = 0.7;   dist[ 3 ].Y( 0, 0 ) = 1.2;   dist[ 3 ].Y( 1, 0 ) = 1.1;   dist[ 3 ].Y( 0, 1 ) = 1.0;   dist[ 3 ].Y( 1, 1 ) = 0.9;
   dist[ 4 ].X() = 0.4;   dist[ 4 ].Y( 0, 0 ) = 2.4;   dist[ 4 ].Y( 1, 0 ) = 2.3;   dist[ 4 ].Y( 0, 1 ) = 2.2;   dist[ 4 ].Y( 1, 1 ) = 2.1;
   DOCTEST_CHECK( dist[ 2 ].X() == 1.0 );
   DOCTEST_CHECK( dist[ 2 ].Y( 0, 0 ) == 0.8 );
   DOCTEST_CHECK( dist[ 2 ].Y( 1, 0 ) == 0.7 );
   DOCTEST_CHECK( dist[ 2 ].Y( 0, 1 ) == 0.6 );
   DOCTEST_CHECK( dist[ 2 ].Y( 1, 1 ) == 0.5 );
   DOCTEST_CHECK( dist[ 2 ].Y( 0 ) == 0.8 );
   DOCTEST_CHECK( dist[ 2 ].Y( 1 ) == 0.7 );
   DOCTEST_CHECK( dist[ 2 ].Y( 2 ) == 0.6 );
   DOCTEST_CHECK( dist[ 2 ].Y( 3 ) == 0.5 );
   // Sorting
   dist.Sort();
   DOCTEST_CHECK( dist[ 2 ].X() == 0.4 );
   DOCTEST_CHECK( dist[ 2 ].Y( 0 ) == 2.4 );
   DOCTEST_CHECK( dist[ 2 ].Y( 1 ) == 2.3 );
   DOCTEST_CHECK( dist[ 2 ].Y( 2 ) == 2.2 );
   DOCTEST_CHECK( dist[ 2 ].Y( 3 ) == 2.1 );
   DOCTEST_CHECK( dist[ 3 ].X() == 0.7 ); DOCTEST_CHECK( dist[ 3 ].Y( 0 ) == 1.2 );
   DOCTEST_CHECK( dist[ 4 ].X() == 1.0 ); DOCTEST_CHECK( dist[ 4 ].Y( 0 ) == 0.8 );
   // Integration and differentiation
   auto dist2 = dist;
   //std::cout << "INPUT:\n" << dist2 << std::endl;
   dist2.Integrate();
   DOCTEST_CHECK( dist2.Back().X() == 1.0 );
   DOCTEST_CHECK( dist2.Back().Y() == 1.41 );
   DOCTEST_CHECK( dist.Integral() == 1.41 );
   //std::cout << "AFTER INTEGRATE:\n" << dist2 << std::endl;
   dist2.Differentiate();
   DOCTEST_CHECK( dist2.Back().X() == 1.0 );
   DOCTEST_CHECK( dist2.Back().Y() == doctest::Approx( 1.0 ));
   //std::cout << "AFTER DIFFERENTIATE:\n" << dist2 << std::endl;
   // Normalization, scaling
   dist2 = dist;
   dist2.NormalizeSum();
   //std::cout << "AFTER NORMALIZE:\n" << dist2 << std::endl;
   DOCTEST_CHECK( dist2.Sum( 0 ) == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( dist2.Sum( 1 ) == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( dist2.Sum( 2 ) == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( dist2.Sum( 3 ) == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( dist2[ 2 ].Y( 0 ) == doctest::Approx( 2.4 / 7.6 ));
   DOCTEST_CHECK( dist2[ 2 ].Y( 1 ) == doctest::Approx( 2.3 / 7.1 ));
   DOCTEST_CHECK( dist2[ 2 ].Y( 2 ) == doctest::Approx( 2.2 / 6.6 ));
   DOCTEST_CHECK( dist2[ 2 ].Y( 3 ) == doctest::Approx( 2.1 / 6.1 ));
   dist *= 2;
   DOCTEST_CHECK( dist[ 2 ].Y( 0 ) == 2.4 * 2.0 );
   DOCTEST_CHECK( dist[ 2 ].Y( 1 ) == 2.3 * 2.0 );
   DOCTEST_CHECK( dist[ 2 ].Y( 2 ) == 2.2 * 2.0 );
   DOCTEST_CHECK( dist[ 2 ].Y( 3 ) == 2.1 * 2.0 );
   // Addition
   dist += dist2;
   DOCTEST_CHECK( dist[ 2 ].Y( 0 ) == doctest::Approx( 2.4 * ( 2.0 + 1.0 / 7.6 )));
   DOCTEST_CHECK( dist[ 2 ].Y( 1 ) == doctest::Approx( 2.3 * ( 2.0 + 1.0 / 7.1 )));
   DOCTEST_CHECK( dist[ 2 ].Y( 2 ) == doctest::Approx( 2.2 * ( 2.0 + 1.0 / 6.6 )));
   DOCTEST_CHECK( dist[ 2 ].Y( 3 ) == doctest::Approx( 2.1 * ( 2.0 + 1.0 / 6.1 )));
}

#endif // DIP__ENABLE_DOCTEST
