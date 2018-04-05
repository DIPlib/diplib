/*
 * DIPlib 3.0
 * This file defines the "StandardDeviation" measurement feature
 *
 * (c)2016-2018, Cris Luengo.
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


namespace dip {
namespace Feature {


class FeatureStandardDeviation : public LineBased {
   public:
      FeatureStandardDeviation() : LineBased( { "StandardDeviation", "Standard deviation of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& /*label*/, Image const& grey, dip::uint nObjects ) override {
         nTensor_ = grey.TensorElements();
         data_.clear();
         data_.resize( nObjects * nTensor_ );
         ValueInformationArray out( nTensor_ );
         if( nTensor_ == 1 ) {
            out[ 0 ].name = "";
         } else {
            for( dip::uint ii = 0; ii < nTensor_; ++ii ) {
               out[ ii ].name = String( "chan" ) + std::to_string( ii );
            }
         }
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
         FastVarianceAccumulator* data = nullptr;
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
                  for( dip::uint ii = 0; ii < nTensor_; ++ii ) {
                     data[ ii ].Push( grey[ ii ] );
                  }
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         FastVarianceAccumulator* data = &data_[ objectIndex ];
         for( dip::uint ii = 0; ii < nTensor_; ++ii ) {
            output[ ii ] = data[ ii ].StandardDeviation();
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nTensor_;
      std::vector< FastVarianceAccumulator > data_;
};


} // namespace feature
} // namespace dip
