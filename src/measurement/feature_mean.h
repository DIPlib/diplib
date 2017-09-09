/*
 * DIPlib 3.0
 * This file defines the "Mean" measurement feature
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


namespace dip {
namespace Feature {


class FeatureMean : public LineBased {
   public:
      FeatureMean() : LineBased( { "Mean", "Mean object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 1 );
         out[ 0 ].name = String( "" );
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
         Data* data = nullptr;
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
                  data->sum += *grey;
                  ++( data->number );
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         Data data = data_[ objectIndex ];
         *output = ( data.number != 0 ) ? ( data.sum / static_cast< dfloat >( data.number )) : ( 0.0 );
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      struct Data {
         dfloat sum = 0;
         dip::uint number = 0;
      };
      dip::uint nD_;
      std::vector< Data > data_;
};


} // namespace feature
} // namespace dip
