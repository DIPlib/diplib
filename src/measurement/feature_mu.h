/*
 * DIPlib 3.0
 * This file defines the "Mu" measurement feature
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

#include "diplib.h"
#include "diplib/measurement.h"

namespace dip {
namespace Feature {


class FeatureMu : public LineBased {
   public:
      FeatureMu() : LineBased( { "Mu", "Elements of the inertia tensor", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         nD_ = label.Dimensionality();
         DIP_THROW_IF(( nD_ < 2 ) || ( nD_ > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
         data_.clear();
         dip::uint nOut = nD_ == 2 ? 3 : 6;
         //nValues_ = nD_ == 2 ? 6 : 10;
         nValues_  = nD_ + nOut + 1;
         data_.resize( nObjects * nValues_, 0 );
         scales_.resize( nOut );
         ValueInformationArray out( nOut );
         dip::uint kk = 0;
         constexpr char const* dims = "xyz";
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            for( dip::uint jj = ii; jj < nD_; ++jj ) {
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
               out[ kk ].name = String( "Mu_" ) + dims[ ii ] + dims[ jj ];
               ++kk;
            }
         }
         return out;
      }

      virtual void ScanLine(
            LineIterator <uint32> label,
            LineIterator <dfloat>,
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         dfloat* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it->second * nValues_ ] );
                  }
               }
               if( data ) {
                  for( dip::uint ii = 0; ii < nD_; ++ii ) {
                     data[ ii ] += coordinates[ ii ];
                  }
                  dip::uint kk = nD_;
                  for( dip::uint ii = 0; ii < nD_; ++ii ) {
                     for( dip::uint jj = ii; jj < nD_; ++jj ) {
                        data[ kk ] += coordinates[ ii ] * coordinates[ jj ];
                        ++kk;
                     }
                  }
                  ++( data[ kk ] );
               }
            }
            ++coordinates[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         dfloat* data = &( data_[ objectIndex * nValues_ ] );
         dfloat n = data[ nValues_ - 1 ];
         if( n == 0 ) {
            for( dip::uint ii = 0; ii < scales_.size(); ++ii ) {
               output[ ii ] = 0;
            }
         } else {
            if( nD_ == 2 ) {
               // 2D Mu tensor, as defined in B. Jahne, Practical Handbook on Image Processing
               // for Scientific Applications, section 16.3.5c
               dfloat x = data[ 0 ];
               dfloat y = data[ 1 ];
               output[ 0 ] =  ( data[ 4 ] - ( y * y ) / n ) / n  * scales_[ 0 ];
               output[ 1 ] = -( data[ 3 ] - ( x * y ) / n ) / n  * scales_[ 1 ];
               output[ 2 ] =  ( data[ 2 ] - ( x * x ) / n ) / n  * scales_[ 2 ];
            } else { // nD_ == 3
               // 3D Mu tensor, as defined in G. Lohmann, Volumetric Image Analysis, pp 55
               dfloat x = data[ 0 ];
               dfloat y = data[ 1 ];
               dfloat z = data[ 2 ];
               dfloat xx = ( data[ 3 ] - ( x * x ) / n ) / n;
               dfloat yy = ( data[ 6 ] - ( y * y ) / n ) / n;
               dfloat zz = ( data[ 8 ] - ( z * z ) / n ) / n;
               output[ 0 ] =  ( yy + zz )                        * scales_[ 0 ];
               output[ 1 ] = -( data[ 4 ] - ( x * y ) / n ) / n  * scales_[ 1 ];
               output[ 2 ] = -( data[ 5 ] - ( x * z ) / n ) / n  * scales_[ 2 ];
               output[ 3 ] =  ( xx + zz )                        * scales_[ 3 ];
               output[ 4 ] = -( data[ 7 ] - ( y * z ) / n ) / n  * scales_[ 4 ];
               output[ 5 ] =  ( xx + yy )                        * scales_[ 5 ];
            }
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
         scales_.clear();
      }

   private:
      dip::uint nD_;       // number of dimensions (2 or 3).
      dip::uint nValues_;  // number of values per object in `data_` (6 or 10).     (== nD_ + nOut + 1)
      FloatArray scales_;  // nOut values
      std::vector< dfloat > data_; // size of this array is nObjects * nValues_. Index as data_[ objectIndex * nValues_ ]
      // Format 2D: x y xx xy yy sum
      // Format 3D: x y z xx xy xz yy yz zz sum
};


} // namespace feature
} // namespace dip
