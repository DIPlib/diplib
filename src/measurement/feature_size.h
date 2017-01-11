/*
 * DIPlib 3.0
 * This file defines the "Size" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureSize : public LineBased {
   public:
      FeatureSize() : LineBased( { "Size", "Number of object pixels", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         data_.clear();
         data_.resize( nObjects, 0 );
         ValueInformationArray out( 1 );
         switch( label.Dimensionality() ) {
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
         }
         PhysicalQuantity unitArea = 1;
         for( dip::uint ii = 0; ii < label.Dimensionality(); ++ii ) {
            PhysicalQuantity pq = label.PixelSize( ii );
            if( pq.IsPhysical() ) {
               unitArea *= pq;
            } else {
               unitArea *= PhysicalQuantity::Pixel();
            }
         }
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         return out;
      }

      virtual void ScanLine(
            LineIterator <uint32> label,
            LineIterator <dfloat>, // unused
            UnsignedArray, // unused
            dip::uint, // unused
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
                     data = &( data_[ it->second ] );
                  }
               }
               if( data ) {
                  ++( *data );
               }
            }
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         *output = data_[ objectIndex ] * scale_;
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
