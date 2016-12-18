/*
 * DIPlib 3.0
 * This file defines the "Size" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureSize : public LineBased {
   public:
      FeatureSize() : LineBased( { "Size", "number of object pixels", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey ) override {
         sizes_.clear();
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

      virtual void Measure(
            LineIterator< uint32 > label,
            LineIterator< dfloat >, // unused
            UnsignedArray, // unused
            dip::uint // unused
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         dip::uint* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  data = &( sizes_[ objectID ] ); // when creating a new entry in the table, the value is initialized to 0.
               }
               ++ *data;
            }
         } while( ++label );
      }

      virtual void Finish( dip::uint objectID, Measurement::ValueIterator data ) override {
         *data = sizes_[ objectID ] * scale_;
      }

      virtual void Cleanup() override {
         sizes_.clear();
      }

   private:
      dfloat scale_;
      std::map< dip::uint, dip::uint > sizes_;
};


} // namespace feature
} // namespace dip
