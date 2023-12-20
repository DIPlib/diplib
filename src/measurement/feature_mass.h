/*
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


class FeatureMass : public LineBased {
   public:
      FeatureMass() : LineBased( { "Mass", "Mass of object (sum of object intensity)", true } ) {};

      ValueInformationArray Initialize( Image const& /*label*/, Image const& grey, dip::uint nObjects ) override {
         nTensor_ = grey.TensorElements();
         data_.clear();
         data_.resize( nObjects * nTensor_, 0 );
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

      void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat > grey,
            UnsignedArray /*coordinates*/,
            dip::uint /*dimension*/,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         LabelType objectID = 0;
         dfloat* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it.value() ] );
                  }
               }
               if( data ) {
                  for( dip::uint ii = 0; ii < nTensor_; ++ii ) {
                     data[ ii ] += grey[ ii ];
                  }
               }
            }
            ++grey;
         } while( ++label );
      }

      void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         dfloat* data = &data_[ objectIndex ];
         for( dip::uint ii = 0; ii < nTensor_; ++ii ) {
            output[ ii ] = data[ ii ];
         }
      }

      void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nTensor_;
      std::vector< dfloat > data_;
};


} // namespace feature
} // namespace dip
