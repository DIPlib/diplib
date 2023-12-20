/*
 * (c)2016-2022, Cris Luengo.
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


class FeatureGreyMu : public LineBased {
   public:
      FeatureGreyMu() : LineBased( { "GreyMu", "Elements of the grey-weighted inertia tensor", true } ) {};

      ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects, MomentAccumulator( nD_ ));
         ValueInformationArray out;
         std::tie( out, scales_ ) = MuInformation( nD_, label.PixelSize() );
         return out;
      }

      void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat > grey,
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         LabelType objectID = 0;
         MomentAccumulator* data = nullptr;
         FloatArray pos{ coordinates };
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
                  data->Push( pos, *grey );
               }
            }
            ++pos[ dimension ];
            ++grey;
         } while( ++label );
      }

      void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         MomentAccumulator* data = &( data_[ objectIndex ] );
         FloatArray values = data->SecondOrder();
         for( dip::uint ii = 0; ii < scales_.size(); ++ii ) {
            output[ ii ] = values[ ii ] * scales_[ ii ];
         }
      }

      void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
         scales_.clear();
      }

   private:
      dip::uint nD_;       // number of dimensions (2 or 3).
      FloatArray scales_;  // nOut values.
      std::vector< MomentAccumulator > data_; // size of this array is nObjects.
};


} // namespace feature
} // namespace dip
