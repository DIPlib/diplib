/*
 * DIPlib 3.0
 * This file defines the "Maximum" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureMaximum : public LineBased {
   public:
      FeatureMaximum() : LineBased( { "Maximum", "Maximum coordinates of the object", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         nD_ = label.Dimensionality();
         data_.clear();
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
         data_.resize( nObjects * nD_, 0 );
         return out;
      }

      virtual void ScanLine(
            LineIterator <uint32> label,
            LineIterator <dfloat>, // unused
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't need to fetch the data pointer again
         uint32 objectID = 0;
         dip::uint* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it->second * nD_ ] );
                     for( dip::uint ii = 0; ii < nD_; ii++ ) {
                        data[ ii ] = std::max( data[ ii ], coordinates[ ii ] );
                     }
                  }
               } else {
                  if( data ) {
                     data[ dimension ] = std::max( data[ dimension ], coordinates[ dimension ] );
                  }
               }
            }
            ++coordinates[ dimension ];
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         dip::uint* data = &( data_[ objectIndex * nD_ ] );
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            output[ ii ] = data[ ii ] * scales_[ ii ];
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         scales_.clear();
      }

   private:
      dip::uint nD_;
      FloatArray scales_;
      std::vector< dip::uint > data_; // size of this array is nObjects * nD_. Index as data_[ objectIndex * nD_ ]
};


} // namespace feature
} // namespace dip
