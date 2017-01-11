/*
 * DIPlib 3.0
 * This file defines the "Gravity" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureGravity : public LineBased {
   public:
      FeatureGravity() : LineBased( { "Gravity", "Coordinates of the center-of-mass of the grey-value object", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects * ( nD_ + 1 ), 0 );
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
         dfloat* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it->second * ( nD_ + 1 ) ] );
                  }
               }
               if( data ) {
                  for( dip::uint ii = 0; ii < nD_; ii++ ) {
                     data[ ii ] += coordinates[ ii ] * *grey;
                  }
                  data[ nD_ ] += *grey;
               }
            }
            ++coordinates[ dimension ];
            ++grey;
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
               output[ ii ] = data[ ii ] / data[ nD_ ] * scales_[ ii ];
            }
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
