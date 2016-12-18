/*
 * DIPlib 3.0
 * This file defines the "Feret" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureFeret : public ConvexHullBased {
   public:
      FeatureFeret() : ConvexHullBased( { "Feret", "maximum and minimum object diameters (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& ) override {
         ValueInformationArray out( 5 );
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude;
            out[ 0 ].units = pq.units;
            out[ 1 ].units = pq.units;
            out[ 2 ].units = pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::Pixel();
            out[ 1 ].units = Units::Pixel();
            out[ 2 ].units = Units::Pixel();
         }
         out[ 3 ].units = Units::Radian();
         out[ 4 ].units = Units::Radian();
         out[ 0 ].name = "FeretMax";
         out[ 1 ].name = "FeretMin";
         out[ 2 ].name = "FeretPerpMin";
         out[ 3 ].name = "FeretMaxAng";
         out[ 4 ].name = "FeretMinAng";
         return out;
      }

      virtual void Measure(
            ConvexHull const& convexHull,
            Measurement::ValueIterator data
      ) override {
         FeretValues feret = convexHull.Feret();
         data[ 0 ] = feret.maxDiameter * scale_;
         data[ 1 ] = feret.minDiameter * scale_;
         data[ 2 ] = feret.maxPerpendicular * scale_;
         data[ 3 ] = feret.maxAngle;
         data[ 4 ] = feret.minAngle;
      }

      virtual void Cleanup() override {}

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
