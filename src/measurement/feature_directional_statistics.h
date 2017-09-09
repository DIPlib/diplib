/*
 * DIPlib 3.0
 * This file defines the "DirectionalStatistics" measurement feature
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

#include <cmath>

namespace dip {
namespace Feature {


class FeatureDirectionalStatistics : public LineBased {
   public:
      FeatureDirectionalStatistics() : LineBased( { "DirectionalStatistics", "Directional mean and standard deviation of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 2 );
         out[ 0 ].name = String( "Mean" );
         out[ 1 ].name = String( "StdDev" );
         return out;
      }

      virtual void ScanLine(
            LineIterator <uint32> label,
            LineIterator <dfloat> grey,
            UnsignedArray /*coordinates*/,
            dip::uint /*dimension*/,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         DirectionalStatisticsAccumulator* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it->second ] );
                  }
               }
               if( data ) {
                  data->Push( *grey );
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         DirectionalStatisticsAccumulator data = data_[ objectIndex ];
         output[ 0 ] = data.Mean();
         output[ 1 ] = data.StandardDeviation();
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nD_;
      std::vector< DirectionalStatisticsAccumulator > data_;
};


} // namespace feature
} // namespace dip
