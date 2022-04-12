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


class FeatureCenter : public LineBased {
   public:
      FeatureCenter() : LineBased( { "Center", "Coordinates of the geometric mean of the object", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects * ( nD_ + 1 ), 0 );
         scales_.resize( nD_ );
         ValueInformationArray out( nD_ );
         PixelSize ps = label.PixelSize();
         ps.ForcePhysical();
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            scales_[ ii ] = ps[ ii ].magnitude;
            out[ ii ].units = ps[ ii ].units;
            out[ ii ].name = String( "dim" ) + std::to_string( ii );
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
         dfloat* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it.value() * ( nD_ + 1 ) ] );
                  }
               }
               if( data ) {
                  for( dip::uint ii = 0; ii < nD_; ii++ ) {
                     data[ ii ] += static_cast< dfloat >( coordinates[ ii ] );
                  }
                  ++( data[ nD_ ] );
               }
            }
            ++coordinates[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         dfloat* data = &( data_[ objectIndex * ( nD_ + 1 ) ] );
         if( data[ nD_ ] == 0 ) {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               output[ ii ] = 0;
            }
         } else {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               output[ ii ] = data[ ii ] / data[ nD_ ];
            }
         }
      }

      virtual void Scale( Measurement::ValueIterator output ) override {
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            output[ ii ] *= scales_[ ii ];
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
         scales_.clear();
      }

   private:
      dip::uint nD_;
      FloatArray scales_;
      std::vector< dfloat > data_; // size of this array is nObjects * ( nD_ + 1 ). Index as data_[ objectIndex * ( nD_ + 1 ) ]
};


} // namespace feature
} // namespace dip
