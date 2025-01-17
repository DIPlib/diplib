/*
 * (c)2017-2024, Cris Luengo.
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

#include "diplib/measurement.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/accumulators.h"
#include "diplib/label_map.h"
#include "diplib/statistics.h"

namespace dip {

std::ostream& operator<<(
      std::ostream& os,
      Measurement const& msr
) {
   if( msr.Features().empty() ) {
      os << "(Raw dip::Measurement object)\n";
      return os;
   }
   // Figure out column widths
   constexpr int separatorWidth = 3;
   constexpr int minimumColumnWidth = 10; // we format numbers with at least this many spaces: '-' + 4 digits of precision + '.' + 'e+NN'
   dip::uint maxID = msr.Objects().empty() ? 0 : *std::max_element( msr.Objects().begin(), msr.Objects().end() );
   int const firstColumnWidth = maxID > 0 ? static_cast< int >( std::floor( std::log10( maxID ))) + 1 : 1;
   auto const& values = msr.Values();
   std::vector< int > valueWidths( values.size(), 0 );
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::string units = values[ ii ].units.StringUnicode();
      valueWidths[ ii ] = static_cast< int >( LengthUnicode( units )) + 2; // + 2 for the brackets we'll add later
      valueWidths[ ii ] = std::max( valueWidths[ ii ], static_cast< int >( values[ ii ].name.size() ));
      valueWidths[ ii ] = std::max( valueWidths[ ii ], minimumColumnWidth );
   }
   auto const& features = msr.Features();
   std::vector< int > featureWidths( values.size(), 0 );
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      featureWidths[ ii ] = valueWidths[ features[ ii ].startColumn ];
      for( dip::uint jj = 1; jj < features[ ii ].numberValues; ++jj ) {
         featureWidths[ ii ] += valueWidths[ features[ ii ].startColumn + jj ] + separatorWidth;
      }
      int minWidth = static_cast< int >( features[ ii ].name.size() );
      if( featureWidths[ ii ] < minWidth ) {
         // Increase the width of the first valueWidths[] so that the columns together have the same width as the feature
         valueWidths[ features[ ii ].startColumn ] += minWidth - featureWidths[ ii ];
         featureWidths[ ii ] = minWidth;
      }
   }
   // Write out the header: feature names
   os << std::setw( firstColumnWidth ) << ' ' << " | ";
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      os << std::setw( featureWidths[ ii ] ) << features[ ii ].name << " | ";
   }
   os << '\n';
   // Write out the header: horizontal line
   os << std::setfill( '-' ) << std::setw( firstColumnWidth ) << '-' << " | ";
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      os << std::setw( featureWidths[ ii ] ) << '-' << " | ";
   }
   os << std::setfill( ' ' ) << '\n';
   // Write out the header: value names
   os << std::setw( firstColumnWidth ) << ' ' << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      os << std::setw( valueWidths[ ii ] ) << values[ ii ].name << " | ";
   }
   os << '\n';
   // Write out the header: value units
   os << std::setw( firstColumnWidth ) << ' ' << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::string units = values[ ii ].units.StringUnicode();
      if( !units.empty() ) {
         // ostream does not correctly align UTF-8 encoded strings
         dip::uint len = LengthUnicode( units );
         os << String( static_cast< dip::uint >( valueWidths[ ii ] ) - len - 2, ' ' ) << '(' << units << ") | ";
      } else {
         os << std::setw( valueWidths[ ii ] ) << ' ' << " | ";
      }
   }
   os << '\n';
   // Write out the header: horizontal line
   os << std::setfill( '-' ) << std::setw( firstColumnWidth ) << '-' << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      os << std::setw( valueWidths[ ii ] ) << '-' << " | ";
   }
   os << std::setfill( ' ' ) << '\n';
   // Write out the object IDs and associated values
   auto const& objects = msr.Objects();
   if( !objects.empty() ) {
      Measurement::ValueIterator data = msr.Data();
      for( auto object : objects ) {
         os << std::setw( firstColumnWidth ) << object << " | ";
         for( dip::uint ii = 0; ii < values.size(); ++ii ) {
            os << std::setw( valueWidths[ ii ] ) << std::setprecision( 4 ) << std::showpoint << *data << " | ";
            ++data;
         }
         os << '\n';
      }
   }
   return os;
}

void MeasurementWriteCSV(
      Measurement const& msr,
      String const& filename,
      StringSet const& options
) {
   bool simple = false;
   bool unicode = false;
   for( auto const& option : options ) {
      if( option == "simple" ) {
         simple = true;
      } else if( option == "unicode" ) {
         unicode = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }
   std::ofstream file( filename, std::ios_base::trunc );
   DIP_THROW_IF( !file.is_open(), "Could not open file for writing" );
   if( simple ) {
      // Write out the header: feature value (units)
      file << "ObjectID";
      auto values = msr.Values().begin();
      for( auto const& feature : msr.Features() ) {
         for( dip::uint ii = 0; ii < feature.numberValues; ++ii ) {
            file << ", " << feature.name;
            if( !values->name.empty() ) {
               file << ' ' << values->name;
            }
            auto units = unicode ? values->units.StringUnicode() : values->units.String();
            if( !units.empty() ) {
               file << " (" << units << ')';
            }
            ++values;
         }
      }
      file << '\n';
   } else {
      // Write out the header: feature names
      file << "ObjectID";
      for( auto const& feature : msr.Features() ) {
         file << ", " << feature.name;
         for( dip::uint ii = 1; ii < feature.numberValues; ++ii ) {
            file << ", "; // Empty columns for feature values past the first one
         }
      }
      file << '\n';
      // Write out the header: value names
      for( auto const& value : msr.Values() ) {
         file << ", " << value.name;
      }
      file << '\n';
      // Write out the header: value units
      for( auto const& value : msr.Values() ) {
         file << ", " << ( unicode ? value.units.StringUnicode() : value.units.String() );
      }
      file << '\n';
   }
   // Write out the object IDs and associated values
   auto const& objects = msr.Objects();
   if( !objects.empty() ) {
      Measurement::ValueIterator data = msr.Data();
      for( auto object : objects ) {
         file << object;
         for( dip::uint ii = 0; ii < msr.NumberOfValues(); ++ii ) {
            file << ", " << *data++;
         }
         file << '\n';
      }
   }
   file.close(); // Not really necessary, but we're used to it...
}

Measurement operator+( Measurement const& lhs, Measurement const& rhs ) {
   DIP_THROW_IF(( lhs.NumberOfObjects() > 0 ) && !lhs.IsForged(), E::MEASUREMENT_NOT_FORGED );
   DIP_THROW_IF(( rhs.NumberOfObjects() > 0 ) && !rhs.IsForged(), E::MEASUREMENT_NOT_FORGED );
   constexpr dip::uint NOT_THERE = std::numeric_limits< dip::uint >::max();
   // Create output object with the union of rows and columns of the input objects
   Measurement out;
   std::vector< dip::uint > lhsColumnIndex( lhs.NumberOfValues(), NOT_THERE );
   std::vector< dip::uint > rhsColumnIndex( lhs.NumberOfValues(), NOT_THERE ); // Yes, `lhs`, this is not a typo!
   dip::uint index = 0;
   for( auto const& f : lhs.features_ ) {
      auto b = lhs.values_.begin() + static_cast< dip::sint >( f.startColumn );
      out.AddFeature_( f.name, b, b + static_cast< dip::sint >( f.numberValues ));
      for( dip::uint ii = 0; ii < out.features_.back().numberValues; ++ii ) {
         lhsColumnIndex[ index++ ] = out.features_.back().startColumn + ii;
      }
   }
   for( auto const& f : rhs.features_ ) {
      auto it = out.featureIndices_.find( f.name );
      if( it == out.featureIndices_.end() ) {
         // Add the feature
         auto b = rhs.values_.begin() + static_cast< dip::sint >( f.startColumn );
         out.AddFeature_( f.name, b, b + static_cast< dip::sint >( f.numberValues ));
         for( dip::uint ii = 0; ii < f.numberValues; ++ii ) {
            lhsColumnIndex.push_back( NOT_THERE );
            rhsColumnIndex.push_back( f.startColumn + ii );
         }
      } else {
         // Check that they have the same number of values
         DIP_THROW_IF( out.features_[ it.value() ].numberValues != f.numberValues,
                       "Number of values for feature " + f.name + " doesn't match" );
         for( dip::uint ii = 0; ii < f.numberValues; ++ii ) {
            rhsColumnIndex[ it.value() + ii ] = f.startColumn + ii;
         }
      }
   }
   //for( dip::uint ii = 0; ii < lhsColumnIndex.size(); ++ii ) {
   //   std::cout << "ii = " << ii << ", lhsColumnIndex[ii] = " << lhsColumnIndex[ii] << ", rhsColumnIndex[ii] = " << rhsColumnIndex[ii] << '\n';
   //}
   std::vector< dip::uint > lhsRowIndex( lhs.objects_.size(), NOT_THERE );
   std::vector< dip::uint > rhsRowIndex( lhs.objects_.size(), NOT_THERE ); // Yes, `lhs`, this is not a typo!
   {
      UnsignedArray objectIDs( lhs.objects_.size() + rhs.objects_.size(), NOT_THERE );
      dip::uint jj = 0;
      for( dip::uint ii = 0; ii < lhs.objects_.size(); ++ii ) { // auto& o : lhs.objects_
         auto o = lhs.objects_[ ii ];
         out.objectIndices_.emplace( o, jj );
         objectIDs[ jj++ ] = o;
         lhsRowIndex[ ii ] = ii;
      }
      for( dip::uint ii = 0; ii < rhs.objects_.size(); ++ii ) { // auto& o : rhs.objects_
         auto o = rhs.objects_[ ii ];
         auto it = out.objectIndices_.find( o );
         if( it == out.objectIndices_.end()) {
            out.objectIndices_.emplace( o, jj );
            objectIDs[ jj++ ] = o;
            lhsRowIndex.push_back( NOT_THERE );
            rhsRowIndex.push_back( ii );
         } else {
            rhsRowIndex[ it.value() ] = ii;
         }
      }
      DIP_ASSERT( jj <= objectIDs.size() );
      objectIDs.resize( jj );
      out.SetObjectIDs( std::move( objectIDs ));
   }
   //for( dip::uint ii = 0; ii < lhsRowIndex.size(); ++ii  ) {
   //   std::cout << "ii = " << ii << ", lhsRowIndex[ii] = " << lhsRowIndex[ii] << ", rhsRowIndex[ii] = " << rhsRowIndex[ii] << '\n';
   //}
   out.Forge();
   // Copy data over from the two inputs
   Measurement::ValueType* lhsPtr = lhs.data_.data();
   Measurement::ValueType* rhsPtr = rhs.data_.data();
   Measurement::ValueType* outPtr = out.data_.data();
   dip::uint lhsStride = lhs.values_.size();
   dip::uint rhsStride = rhs.values_.size();
   for( dip::uint jj = 0; jj < out.NumberOfObjects(); ++jj ) {
      if( rhsRowIndex[ jj ] == NOT_THERE ) {
         // Copy from LHS
         auto lhsRowPtr = lhsPtr + lhsStride * lhsRowIndex[ jj ];
         for( dip::uint ii = 0; ii < out.NumberOfValues(); ++ii ) {
            if( lhsColumnIndex[ ii ] == NOT_THERE ) {
               *outPtr = nan;
            } else {
               *outPtr = lhsRowPtr[ lhsColumnIndex[ ii ]];
            }
            ++outPtr;
         }
      } else if( lhsRowIndex[ jj ] == NOT_THERE ) {
         // Copy from RHS
         auto rhsRowPtr = rhsPtr + rhsStride * rhsRowIndex[ jj ];
         for( dip::uint ii = 0; ii < out.NumberOfValues(); ++ii ) {
            if( rhsColumnIndex[ ii ] == NOT_THERE ) {
               *outPtr = nan;
            } else {
               *outPtr = rhsRowPtr[ rhsColumnIndex[ ii ]];
            }
            ++outPtr;
         }
      } else {
         // Merge
         auto lhsRowPtr = lhsPtr + lhsStride * lhsRowIndex[ jj ];
         auto rhsRowPtr = rhsPtr + rhsStride * rhsRowIndex[ jj ];
         for( dip::uint ii = 0; ii < out.NumberOfValues(); ++ii ) {
            if( lhsColumnIndex[ ii ] != NOT_THERE ) {
               *outPtr = lhsRowPtr[ lhsColumnIndex[ ii ]];
            } else if( rhsColumnIndex[ ii ] != NOT_THERE ) {
               *outPtr = rhsRowPtr[ rhsColumnIndex[ ii ]];
            } else {
               *outPtr = nan; // Can this even happen???
            }
            ++outPtr;
         }
      }
   }
   return out;
}

Measurement::ValueType Minimum( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   }
   auto it = featureValues.FirstObject();
   Measurement::ValueType minVal = *it;
   while( ++it ) {
      minVal = std::min( minVal, *it );
   }
   return minVal;
}

Measurement::ValueType Maximum( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   }
   auto it = featureValues.FirstObject();
   Measurement::ValueType maxVal = *it;
   while( ++it ) {
      maxVal = std::max( maxVal, *it );
   }
   return maxVal;
}

Measurement::ValueType Percentile( Measurement::IteratorFeature const& featureValues, dfloat percentile ) {
   dip::uint N = featureValues.NumberOfObjects();
   if( N == 0 ) {
      return 0.0;
   }
   dip::uint rank = RankFromPercentile( percentile, N );
   if( rank == 0 ) {
      return Minimum( featureValues );
   }
   if( rank == N - 1 ) {
      return Maximum( featureValues );
   }
   std::vector< Measurement::ValueType > buffer( N );
   auto begin = buffer.begin();
   auto leftIt = begin;
   auto rightIt = buffer.rbegin();
   Measurement::ValueType pivot{};
   auto it = featureValues.FirstObject();
   pivot = *( it++ );
   do {
      Measurement::ValueType v = *it;
      if( v < pivot ) {
         *( leftIt++ ) = v;
      } else {
         *( rightIt++ ) = v;
      }
   } while( ++it );
   DIP_ASSERT( &*leftIt == &*rightIt ); // They should both be pointing to the same array element.
   *leftIt = pivot;
   auto ourGuy = begin + static_cast< dip::sint >( rank );
   if( ourGuy < leftIt ) {
      // our guy is to the left
      std::nth_element( begin, ourGuy, leftIt );
   } else if( ourGuy > leftIt ){
      // our guy is to the right
      std::nth_element( ++leftIt, ourGuy, buffer.end() );
   } // else: ourGuy == leftIt, which is already sorted correctly.
   return *ourGuy;
}

dfloat Mean( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   }
   auto it = featureValues.FirstObject();
   dfloat sum = *it;
   while( ++it ) {
      sum += *it;
   }
   return sum / static_cast< dfloat >( featureValues.NumberOfObjects() );
}

MinMaxAccumulator MaximumAndMinimum( Measurement::IteratorFeature const& featureValues ) {
   MinMaxAccumulator acc;
   auto it = featureValues.FirstObject();
   while( it ) {
      acc.Push( *it );
      ++it;
   }
   return acc;
}

QuartilesResult Quartiles( Measurement::IteratorFeature const& featureValues ) {
   return Quartiles( featureValues.AsScalarImage() );
}

StatisticsAccumulator SampleStatistics( Measurement::IteratorFeature const& featureValues ) {
   StatisticsAccumulator acc;
   auto it = featureValues.FirstObject();
   while( it ) {
      acc.Push( *it );
      ++it;
   }
   return acc;
}

dip::uint ObjectMinimum( Measurement::IteratorFeature const& featureValues ) {
   DIP_THROW_IF( featureValues.NumberOfObjects() == 0, "No objects in IteratorFeature" );
   auto it = featureValues.FirstObject();
   Measurement::ValueType minVal = *it;
   dip::uint minID = it.ObjectID();
   while( ++it ) {
      if( *it < minVal ) {
         minVal = *it;
         minID = it.ObjectID();
      }
   }
   return minID;
}

dip::uint ObjectMaximum( Measurement::IteratorFeature const& featureValues ) {
   DIP_THROW_IF( featureValues.NumberOfObjects() == 0, "No objects in IteratorFeature" );
   auto it = featureValues.FirstObject();
   Measurement::ValueType maxVal = *it;
   dip::uint maxID = it.ObjectID();
   while( ++it ) {
      if( *it > maxVal ) {
         maxVal = *it;
         maxID = it.ObjectID();
      }
   }
   return maxID;
}

namespace {

template< typename ComparisonFunction >
LabelMap ComparisonOperator( Measurement::IteratorFeature const& featureValues, ComparisonFunction comparisonFunction ) {
   LabelMap output( featureValues.Objects() );
   auto it = featureValues.FirstObject();
   do {
      if( !comparisonFunction( *it )) {
         // The `LabelMap` constructor has already tested all the labels, no need to test again through `CastLabelType`.
         output[ static_cast< LabelType >( it.ObjectID() ) ] = 0;
      }
   } while( ++it );
   return output;
}

}

LabelMap operator==( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in == value; } );
}

LabelMap operator!=( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in != value; } );
}

LabelMap operator>( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in > value; } );
}

LabelMap operator>=( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in >= value; } );
}

LabelMap operator<( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in < value; } );
}

LabelMap operator<=( Measurement::IteratorFeature const& featureValues, Measurement::ValueType value ) {
   return ComparisonOperator( featureValues, [ value ]( Measurement::ValueType in ){ return in <= value; } );
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Measurement" ) {

   // Create two objects

   dip::Measurement msr1;
   dip::Measurement msr2;
   {
      dip::Feature::ValueInformationArray values( 2 );
      values[ 0 ].name = "Dim1";
      values[ 0 ].units = dip::Units::Meter();
      values[ 1 ].name = "Dim2";
      values[ 1 ].units = dip::Units::Hertz();
      msr1.AddFeature( "Feature1", values );
      msr2.AddFeature( "Feature1", values );

      values.resize( 1 );
      values[ 0 ].name = "Bla";
      values[ 0 ].units = dip::Units::SquareMeter();
      msr1.AddFeature( "Feature2", values );

      values.resize( 3 );
      values[ 0 ].name = "Foo";
      values[ 0 ].units = dip::Units::SquareMeter();
      values[ 1 ].name = "Bar";
      values[ 1 ].units = dip::Units::CubicMeter();
      values[ 2 ].name = "Ska";
      values[ 2 ].units = dip::Units::Meter();
      msr2.AddFeature( "Feature3", values );

      dip::UnsignedArray ids( 10 );
      for( dip::uint ii = 0; ii < 10; ++ii ) {
         ids[ ii ] = ii + 10;
      }
      msr1.AddObjectIDs( ids );
      for( dip::uint ii = 0; ii < 10; ++ii ) {
         ids[ ii ] = ii + 15;
      }
      msr2.SetObjectIDs( ids );
   }

   // Check

   msr1.Forge();
   DOCTEST_REQUIRE( msr1.IsForged() );
   DOCTEST_REQUIRE( msr1.NumberOfObjects() == 10 );
   DOCTEST_REQUIRE( msr1.NumberOfFeatures() == 2 );
   DOCTEST_REQUIRE( msr1.NumberOfValues() == 3 );
   DOCTEST_REQUIRE( msr1.FeatureExists( "Feature1" ));
   DOCTEST_REQUIRE( msr1.FeatureExists( "Feature2" ));
   DOCTEST_CHECK( msr1.NumberOfValues( "Feature1" ) == 2 );
   DOCTEST_CHECK( msr1.NumberOfValues( "Feature2" ) == 1 );
   DOCTEST_CHECK( msr1.ValueIndex( "Feature1" ) == 0 );
   DOCTEST_CHECK( msr1.ValueIndex( "Feature2" ) == 2 );
   DOCTEST_CHECK_FALSE( msr1.ObjectExists( 1 ));
   DOCTEST_CHECK( msr1.ObjectExists( 10 ));
   DOCTEST_CHECK( msr1.ObjectExists( 19 ));
   DOCTEST_CHECK_FALSE( msr1.ObjectExists( 20 ));
   DOCTEST_CHECK_FALSE( msr1.ObjectExists( 25 ));

   msr2.Forge();
   DOCTEST_REQUIRE( msr2.IsForged() );
   DOCTEST_REQUIRE( msr2.NumberOfObjects() == 10 );
   DOCTEST_REQUIRE( msr2.NumberOfFeatures() == 2 );
   DOCTEST_REQUIRE( msr2.NumberOfValues() == 5 );
   DOCTEST_REQUIRE( msr2.FeatureExists( "Feature1" ));
   DOCTEST_REQUIRE( msr2.FeatureExists( "Feature3" ));
   DOCTEST_CHECK( msr2.NumberOfValues( "Feature1" ) == 2 );
   DOCTEST_CHECK( msr2.NumberOfValues( "Feature3" ) == 3 );
   DOCTEST_CHECK( msr2.ValueIndex( "Feature1" ) == 0 );
   DOCTEST_CHECK( msr2.ValueIndex( "Feature3" ) == 2 );
   DOCTEST_CHECK_FALSE( msr2.ObjectExists( 1 ));
   DOCTEST_CHECK_FALSE( msr2.ObjectExists( 10 ));
   DOCTEST_CHECK( msr2.ObjectExists( 19 ));
   DOCTEST_CHECK( msr2.ObjectExists( 20 ));
   DOCTEST_CHECK_FALSE( msr2.ObjectExists( 25 ));

   // Write some data into the two objects

   dip::Measurement::ValueType* v = msr1.Data();
   dip::uint nValues = msr1.NumberOfValues();
   dip::uint nObjects = msr1.NumberOfObjects();
   for( dip::uint ii = 0; ii < nValues * nObjects; ++ii ) {
      *v = static_cast< dip::Measurement::ValueType >( ii );
      ++v;
   }

   v = msr2.Data();
   nValues = msr2.NumberOfValues();
   nObjects = msr2.NumberOfObjects();
   for( dip::uint ii = 0; ii < nValues * nObjects; ++ii ) {
      *v = static_cast< dip::Measurement::ValueType >( ii + 100 );
      ++v;
   }

   // Check using iterators

   auto objIt = msr1[ 15 ];
   auto itA = objIt[ "Feature1" ];
   DOCTEST_CHECK( itA.FeatureName() == "Feature1" );
   DOCTEST_CHECK( itA.ObjectID() == 15 );
   DOCTEST_CHECK( itA.ObjectIndex() == 15 - 10 );
   DOCTEST_CHECK( itA.size() == 2 );
   DOCTEST_CHECK( *itA == ( 15 - 10 ) * 3 );
   DOCTEST_CHECK( itA[ 1 ] == ( 15 - 10 ) * 3 + 1 );
   ++itA;
   DOCTEST_CHECK( itA.FeatureName() == "Feature2" );
   DOCTEST_CHECK( itA.ObjectID() == 15 );
   DOCTEST_CHECK( itA.ObjectIndex() == 15 - 10 );
   DOCTEST_CHECK( itA.size() == 1 );
   DOCTEST_CHECK( *itA == ( 15 - 10 ) * 3 + 2 );
   ++objIt;
   itA = objIt[ "Feature2" ];
   DOCTEST_CHECK( itA.FeatureName() == "Feature2" );
   DOCTEST_CHECK( itA.ObjectID() == 16 );
   DOCTEST_CHECK( itA.ObjectIndex() == 16 - 10 );
   DOCTEST_CHECK( itA.size() == 1 );
   DOCTEST_CHECK( *itA == ( 16 - 10 ) * 3 + 2 );

   auto featIt = msr1[ "Feature2" ];
   auto itB = featIt[ 13 ];
   DOCTEST_CHECK( itB.FeatureName() == "Feature2" );
   DOCTEST_CHECK( itB.ObjectID() == 13 );
   DOCTEST_CHECK( itB.ObjectIndex() == 13 - 10 );
   DOCTEST_CHECK( itB.size() == 1 );
   DOCTEST_CHECK( *itB == ( 13 - 10 ) * 3 + 2 );
   DOCTEST_CHECK( itB[ 0 ] == ( 13 - 10 ) * 3 + 2 );
   ++itB;
   DOCTEST_CHECK( itB.FeatureName() == "Feature2" );
   DOCTEST_CHECK( itB.ObjectID() == 14 );
   DOCTEST_CHECK( itB.ObjectIndex() == 14 - 10 );
   DOCTEST_CHECK( itB.size() == 1 );
   DOCTEST_CHECK( *itB == ( 14 - 10 ) * 3 + 2 );
   DOCTEST_CHECK( itB[ 0 ] == ( 14 - 10 ) * 3 + 2 );
   ++featIt;
   DOCTEST_CHECK( featIt.IsAtEnd() );

   // Check addition

   auto res = msr1 + msr2;
   DOCTEST_REQUIRE( res.IsForged() );
   DOCTEST_CHECK( res.NumberOfObjects() == 15 );
   DOCTEST_CHECK( res.NumberOfFeatures() == 3 );
   DOCTEST_CHECK( res.NumberOfValues() == 6 );
   DOCTEST_REQUIRE( res.FeatureExists( "Feature1" ));
   DOCTEST_REQUIRE( res.FeatureExists( "Feature2" ));
   DOCTEST_REQUIRE( res.FeatureExists( "Feature3" ));
   DOCTEST_CHECK( res.NumberOfValues( "Feature1" ) == 2 );
   DOCTEST_CHECK( res.NumberOfValues( "Feature2" ) == 1 );
   DOCTEST_CHECK( res.NumberOfValues( "Feature3" ) == 3 );
   DOCTEST_CHECK( res.ValueIndex( "Feature1" ) == 0 );
   DOCTEST_CHECK( res.ValueIndex( "Feature2" ) == 2 );
   DOCTEST_CHECK( res.ValueIndex( "Feature3" ) == 3 );
   DOCTEST_CHECK_FALSE( res.ObjectExists( 1 ));
   DOCTEST_CHECK( res.ObjectExists( 10 ));
   DOCTEST_CHECK( res.ObjectExists( 19 ));
   DOCTEST_CHECK( res.ObjectExists( 20 ));
   DOCTEST_CHECK_FALSE( res.ObjectExists( 25 ));

   // Check the data using iterators

   objIt = res[ 13 ]; // msr1 only
   DOCTEST_CHECK( objIt[ "Feature1" ][ 0 ] == ( 13 - 10 ) * 3 );
   DOCTEST_CHECK( objIt[ "Feature1" ][ 1 ] == ( 13 - 10 ) * 3 + 1 );
   DOCTEST_CHECK( objIt[ "Feature2" ][ 0 ] == ( 13 - 10 ) * 3 + 2 );
   DOCTEST_CHECK( std::isnan( objIt[ "Feature3" ][ 0 ] ));
   DOCTEST_CHECK( std::isnan( objIt[ "Feature3" ][ 1 ] ));
   DOCTEST_CHECK( std::isnan( objIt[ "Feature3" ][ 2 ] ));

   objIt = res[ 18 ]; // both
   DOCTEST_CHECK( objIt[ "Feature1" ][ 0 ] == ( 18 - 10 ) * 3 );
   DOCTEST_CHECK( objIt[ "Feature1" ][ 1 ] == ( 18 - 10 ) * 3 + 1 );
   DOCTEST_CHECK( objIt[ "Feature2" ][ 0 ] == ( 18 - 10 ) * 3 + 2 );
   DOCTEST_CHECK( objIt[ "Feature3" ][ 0 ] == ( 18 - 15 ) * 5 + 102 );
   DOCTEST_CHECK( objIt[ "Feature3" ][ 1 ] == ( 18 - 15 ) * 5 + 103 );
   DOCTEST_CHECK( objIt[ "Feature3" ][ 2 ] == ( 18 - 15 ) * 5 + 104 );

   objIt = res[ 23 ]; // msr2 only
   DOCTEST_CHECK( objIt[ "Feature1" ][ 0 ] == ( 23 - 15 ) * 5 + 100 );
   DOCTEST_CHECK( objIt[ "Feature1" ][ 1 ] == ( 23 - 15 ) * 5 + 101 );
   DOCTEST_CHECK( std::isnan( objIt[ "Feature2" ][ 0 ] ));
   DOCTEST_CHECK( objIt[ "Feature3" ][ 0 ] == ( 23 - 15 ) * 5 + 102 );
   DOCTEST_CHECK( objIt[ "Feature3" ][ 1 ] == ( 23 - 15 ) * 5 + 103 );
   DOCTEST_CHECK( objIt[ "Feature3" ][ 2 ] == ( 23 - 15 ) * 5 + 104 );

   // Check the case of the raw measurement object

   dip::Measurement msr3;
   {
      dip::Feature::ValueInformationArray values( 2 );
      values[ 0 ].name = "Dim1";
      values[ 0 ].units = dip::Units::Meter();
      values[ 1 ].name = "Dim2";
      values[ 1 ].units = dip::Units::Hertz();
      msr3.AddFeature( "Feature1", values );

      values.resize( 1 );
      values[ 0 ].name = "Bla";
      values[ 0 ].units = dip::Units::SquareMeter();
      msr3.AddFeature( "Feature2", values );
   }
   DOCTEST_CHECK_NOTHROW( msr3.Forge() );

   featIt = msr3.FirstFeature();
   DOCTEST_CHECK( static_cast< bool >( featIt ));
   while( featIt ) {
      itB = featIt.FirstObject();
      DOCTEST_CHECK( itB.IsAtEnd() );
      ++featIt;
   }

   featIt = msr3[ "Feature1" ];
   DOCTEST_CHECK( featIt.FeatureName() == "Feature1" );
   DOCTEST_CHECK( featIt.NumberOfValues() == 2 );
   DOCTEST_CHECK( featIt.NumberOfObjects() == 0 );
   ++featIt;
   DOCTEST_CHECK( featIt.FeatureName() == "Feature2" );
   DOCTEST_CHECK( featIt.NumberOfValues() == 1 );
   DOCTEST_CHECK( featIt.NumberOfObjects() == 0 );
   ++featIt;
   DOCTEST_CHECK( featIt.IsAtEnd() );

   // Check feature comparison

   auto map = msr1[ "Feature1" ] > 10;
   DOCTEST_CHECK( map.Size() == 10 );
   DOCTEST_CHECK( map.Count() == 6 );
   DOCTEST_CHECK( !map.Contains( 1 ));
   DOCTEST_CHECK( map.Contains( 10 ));
   DOCTEST_CHECK( !map.Contains( 20 ));
   DOCTEST_CHECK( map[ 10 ] == 0 );
   DOCTEST_CHECK( map[ 13 ] == 0 );
   DOCTEST_CHECK( map[ 14 ] == 14 );
   DOCTEST_CHECK( map[ 19 ] == 19 );

   // Check LabelMap::Apply for measurement object

   auto msr4 = map.Apply( msr1 );
   DOCTEST_CHECK( msr4.NumberOfObjects() == 6 );
   DOCTEST_CHECK( !msr4.ObjectExists( 10 ) );
   DOCTEST_CHECK( !msr4.ObjectExists( 13 ) );
   DOCTEST_CHECK( msr4.ObjectExists( 14 ) );
   DOCTEST_CHECK( msr4.ObjectExists( 19 ) );

   map.Relabel();
   msr4 = map.Apply( msr1 );
   DOCTEST_CHECK( msr4.NumberOfObjects() == 6 );
   DOCTEST_CHECK( !msr4.ObjectExists( 0 ) );
   DOCTEST_CHECK( msr4.ObjectExists( 1 ) );
   DOCTEST_CHECK( msr4.ObjectExists( 6 ) );
   DOCTEST_CHECK( !msr4.ObjectExists( 7 ) );
   DOCTEST_CHECK( !msr4.ObjectExists( 10 ) );
   DOCTEST_CHECK( !msr4.ObjectExists( 19 ) );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
