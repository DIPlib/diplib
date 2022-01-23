/*
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


class FeatureMu : public LineBased {
   public:
      FeatureMu() : LineBased( { "Mu", "Elements of the inertia tensor", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         nD_ = label.Dimensionality();
         data_.clear();
         dip::uint nOut = nD_ * ( nD_ + 1 ) / 2;
         data_.resize( nObjects, MomentAccumulator( nD_ ));
         scales_.resize( nOut );
         ValueInformationArray out( nOut );
         dip::uint kk = 0;
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            PhysicalQuantity pq1 = label.PixelSize( ii );
            if( !pq1.IsPhysical() ) {
               pq1 = PhysicalQuantity::Pixel();
            }
            scales_[ kk ] = pq1.magnitude * pq1.magnitude;
            out[ kk ].units = pq1.units * pq1.units;
            out[ kk ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( ii );
            ++kk;
         }
         for( dip::uint ii = 1; ii < nD_; ++ii ) {
            for( dip::uint jj = 0; jj < ii; ++jj ) {
               PhysicalQuantity pq1 = label.PixelSize( ii );
               if( !pq1.IsPhysical() ) {
                  pq1 = PhysicalQuantity::Pixel();
               }
               PhysicalQuantity pq2 = label.PixelSize( jj );
               if( !pq2.IsPhysical() ) {
                  pq2 = PhysicalQuantity::Pixel();
               }
               scales_[ kk ] = pq1.magnitude * pq2.magnitude;
               out[ kk ].units = pq1.units * pq2.units;
               out[ kk ].name = String( "Mu_" ) + std::to_string( ii ) + '_' + std::to_string( jj );
               ++kk;
            }
         }
         return out;
      }

      virtual void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat >,
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
                  data->Push( pos, 1.0 );
               }
            }
            ++pos[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         MomentAccumulator* data = &( data_[ objectIndex ] );
         FloatArray values = data->SecondOrder();
         for( dip::uint ii = 0; ii < scales_.size(); ++ii ) {
            output[ ii ] = values[ ii ] * scales_[ ii ];
         }
      }

      virtual void Cleanup() override {
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
