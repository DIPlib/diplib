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


class FeatureSize : public LineBased {
   public:
      FeatureSize() : LineBased( { "Size", "Number of object pixels", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         data_.clear();
         data_.resize( nObjects, 0 );
         ValueInformationArray out( 1 );
         /*switch( label.Dimensionality() ) {
            case 1:
               out[ 0 ].name = "Length";
               break;
            case 2:
               out[ 0 ].name = "Area";
               break;
            case 3:
               out[ 0 ].name = "Volume";
               break;
            default:
               out[ 0 ].name = "Size";
         }*/
         out[ 0 ].name = "";
         PhysicalQuantity unitArea = label.PixelSize().UnitSize( label.Dimensionality() );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         return out;
      }

      virtual void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat >, // unused
            UnsignedArray, // unused
            dip::uint, // unused
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't need to fetch the data pointer again
         LabelType objectID = 0;
         dip::uint* data = nullptr;
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
                  ++( *data );
               }
            }
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         *output = static_cast< dfloat >( data_[ objectIndex ] );
      }

      virtual void Scale( Measurement::ValueIterator output ) override {
         *output *= scale_;
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dfloat scale_;
      std::vector< dip::uint > data_;
};


} // namespace feature
} // namespace dip
