/*
 * DIPlib 3.0
 * This file contains the definition for function that compute statistics on measurement features
 *
 * (c)2017, Cris Luengo.
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
#include "diplib/measurement.h"

namespace dip {

dfloat Minimum( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   } else {
      auto it = featureValues.FirstObject();
      dfloat minVal = *it;
      while( ++it ) {
         minVal = std::min( minVal, *it );
      }
      return minVal;
   }
}

dfloat Maximum( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   } else {
      auto it = featureValues.FirstObject();
      dfloat maxVal = *it;
      while( ++it ) {
         maxVal = std::max( maxVal, *it );
      }
      return maxVal;
   }
}

dfloat Percentile( Measurement::IteratorFeature const& featureValues, dfloat percentile ) {
   if( percentile == 0.0 ) {
      return Minimum( featureValues );
   } else if( percentile == 100.0 ) {
      return Maximum( featureValues );
   } else {
      dip::uint N = featureValues.NumberOfObjects();
      if( N == 0 ) {
         return 0.0;
      }
      dip::sint rank = static_cast< dip::sint >( std::floor( static_cast< dfloat >( N ) * percentile / 100.0 )); // rank < N, because percentile_ < 100
      std::vector< dfloat > buffer( N );
      auto begin = buffer.begin();
      auto leftIt = begin;
      auto rightIt = buffer.rbegin();
      dfloat pivot{};
      auto it = featureValues.FirstObject();
      pivot = *( it++ );
      do {
         dfloat v = *it;
         if( v < pivot ) {
            *( leftIt++ ) = v;
         } else {
            *( rightIt++ ) = v;
         }
      } while( ++it );
      DIP_ASSERT( &*leftIt == &*rightIt ); // They should both be pointing to the same array element.
      *leftIt = pivot;
      auto ourGuy = begin + rank;
      if( ourGuy < leftIt ) {
         // our guy is to the left
         std::nth_element( begin, ourGuy, leftIt );
      } else if( ourGuy > leftIt ){
         // our guy is to the right
         std::nth_element( ++leftIt, ourGuy, buffer.end() );
      } // else: ourGuy == leftIt, which is already sorted correctly.
      return *ourGuy;
   }
}

dfloat Mean( Measurement::IteratorFeature const& featureValues ) {
   if( featureValues.NumberOfObjects() == 0 ) {
      return 0.0;
   } else {
      auto it = featureValues.FirstObject();
      dfloat sum = *it;
      while( ++it ) {
         sum += *it;
      }
      return sum / static_cast< dfloat >( featureValues.NumberOfObjects() );
   }
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

StatisticsAccumulator SampleStatistics( Measurement::IteratorFeature const& featureValues ) {
   StatisticsAccumulator acc;
   auto it = featureValues.FirstObject();
   while( it ) {
      acc.Push( *it );
      ++it;
   }
   return acc;
}

} // namespace dip
