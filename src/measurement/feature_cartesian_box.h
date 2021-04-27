/*
 * DIPlib 3.0
 * This file defines the "CartesianBox" measurement feature
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


class FeatureCartesianBox : public LineBased {
   public:
      FeatureCartesianBox() : LineBased( { "CartesianBox", "Cartesian box size of the object in all dimensions", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects * nD_ );
         scales_.resize( nD_ );
         ValueInformationArray out( nD_ );
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            PhysicalQuantity pq = label.PixelSize( ii );
            if( pq.IsPhysical() ) {
               scales_[ ii ] = pq.magnitude;
               out[ ii ].units = pq.units;
            } else {
               scales_[ ii ] = 1;
               out[ ii ].units = Units::Pixel();
            }
            out[ ii ].name = String( "dim" ) + std::to_string( ii );
         }
         return out;
      }

      virtual void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat >, // unused
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't need to fetch the data pointer again
         LabelType objectID = 0;
         MinMaxCoord* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it.value() * nD_ ] );
                     for( dip::uint ii = 0; ii < nD_; ii++ ) {
                        data[ ii ].min = std::min( data[ ii ].min, coordinates[ ii ] );
                        data[ ii ].max = std::max( data[ ii ].max, coordinates[ ii ] );
                     }
                  }
               } else {
                  if( data ) {
                     data[ dimension ].max = std::max( data[ dimension ].max, coordinates[ dimension ] );
                  }
               }
            }
            ++coordinates[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         MinMaxCoord* data = &( data_[ objectIndex * nD_ ] );
         if( data[ 0 ].min > data[ 0 ].max ) {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               output[ ii ] = 0;
            }
         } else {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               output[ ii ] = static_cast< dfloat >( data[ ii ].max - data[ ii ].min + 1 ) * scales_[ ii ];
            }
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
         scales_.clear();
      }

   private:
      struct MinMaxCoord {
         dip::uint min = std::numeric_limits< dip::uint >::max();
         dip::uint max = 0;
      };
      dip::uint nD_;
      FloatArray scales_;
      std::vector< MinMaxCoord > data_; // size of this array is nObjects * nD_. Index as data_[ objectIndex * nD_ ]
};


} // namespace feature
} // namespace dip
