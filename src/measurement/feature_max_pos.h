/*
 * DIPlib 3.0
 * This file defines the "MaxPos" measurement feature
 *
 * (c)2018, Cris Luengo.
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


class FeatureMaxPos : public LineBased {
   public:
      FeatureMaxPos() : LineBased( { "MaxPos", "Position of pixel with maximum intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         pos_.clear();
         data_.clear();
         pos_.resize( nObjects * nD_, 0 );
         data_.resize( nObjects, -infinity );
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
            LineIterator <uint32> label,
            LineIterator <dfloat> grey,
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         dip::uint* pos = nullptr;
         dfloat* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     pos = nullptr;
                     data = nullptr;
                  } else {
                     pos = &( pos_[ it->second * nD_ ] );
                     data = &( data_[ it->second ] );
                  }
               }
               if( data && ( *data < *grey )) {
                  *data = *grey;
                  for( dip::uint ii = 0; ii < nD_; ++ii ) {
                     pos[ ii ] = coordinates[ ii ];
                  }
               }
            }
            ++grey;
            ++coordinates[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         dip::uint* pos = &( pos_[ objectIndex * nD_ ] );
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            output[ ii ] = static_cast< dfloat >( pos[ ii ] ) * scales_[ ii ];
         }
      }

      virtual void Cleanup() override {
         pos_.clear();
         pos_.shrink_to_fit();
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nD_;
      FloatArray scales_;
      std::vector< dip::uint > pos_; // size of this array is nObjects. Index as data_[ objectIndex ]
      std::vector< dfloat > data_; // size of this array is nObjects * nD_. Index as data_[ objectIndex * nD_ ]
};


} // namespace feature
} // namespace dip
