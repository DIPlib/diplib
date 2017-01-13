/*
 * DIPlib 3.0
 * This file defines the "Radius" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureRadius : public ChainCodeBased {
   public:
      FeatureRadius() : ChainCodeBased( { "Radius", "Statistics on radius of object (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 4 );
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude;
            out[ 0 ].units = pq.units;
            out[ 1 ].units = pq.units;
            out[ 2 ].units = pq.units;
            out[ 3 ].units = pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::Pixel();
            out[ 1 ].units = Units::Pixel();
            out[ 2 ].units = Units::Pixel();
            out[ 3 ].units = Units::Pixel();
         }
         out[ 0 ].name = "RadiusMax";
         out[ 1 ].name = "RadiusMean";
         out[ 2 ].name = "RadiusMin";
         out[ 3 ].name = "RadiusStD";
         return out;
      }

      virtual void Measure( ChainCode const& chaincode, Measurement::ValueIterator output ) override {
         ChainCode::RadiusValues radius = chaincode.Radius();
         output[ 0 ] = radius.max * scale_;
         output[ 1 ] = radius.mean * scale_;
         output[ 2 ] = radius.min * scale_;
         output[ 3 ] = std::sqrt( radius.var ) * scale_;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
