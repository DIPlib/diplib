/*
 * DIPlib 3.0
 * This file defines the "Gravity" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureGravity : public LineBased {
   public:
      FeatureGravity() : LineBased( info_ ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         sums_.clear();
         scales_.resize( nD_ );
         ValueInformationArray out( nD_ );
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            PhysicalQuantity pq = label.PixelSize( ii );
            scales_[ ii ] = pq.magnitude;
            out[ ii ].units = pq.units;
            out[ ii ].name = String( "dim" ) + std::to_string( ii );
         }
         return out;
      }

      virtual void Measure(
            LineIterator< uint32 > label,
            LineIterator< dfloat > grey,
            UnsignedArray coordinates,
            dip::uint dimension
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         FloatArray* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  data = & ( sums_[ objectID ] );
                  if( data->empty() ) {
                     data->resize( nD_ + 1, 0 );
                  }
               }
               for( dip::uint ii = 0; ii < nD_; ii++ ) {
                  (*data)[ ii ] += coordinates[ ii ] * *grey;
               }
               (*data)[ nD_ ] += *grey;
            }
            ++coordinates[ dimension ];
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectID, Measurement::ValueIterator data ) override {
         FloatArray& tmp = sums_[ objectID ];
         if( tmp.empty() || ( tmp[ nD_ ] == 0 )) {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               data[ ii ] = 0;
            }
         } else {
            for( dip::uint ii = 0; ii < nD_; ++ii ) {
               data[ ii ] = tmp[ ii ] / tmp[ nD_ ] * scales_[ ii ];
            }
         }
      }

      virtual void Cleanup() override {
         sums_.clear();
         scales_.clear();
      }

   private:
      static constexpr Information info_ { "Gravity", "coordinates of the center-of-mass of the grey-value object", true };
      dip::uint nD_;
      FloatArray scales_;
      std::map< dip::uint, FloatArray > sums_;
};


} // namespace feature
} // namespace dip
