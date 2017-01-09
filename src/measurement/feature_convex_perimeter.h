/*
 * DIPlib 3.0
 * This file defines the "ConvexPerimeter" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureConvexPerimeter : public ConvexHullBased {
   public:
      FeatureConvexPerimeter() : ConvexHullBased( { "ConvexPerimeter", "Perimeter of the convex hull (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         out[ 0 ].name = "Perimeter";
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude;
            out[ 0 ].units = pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::Pixel();
         }
         return out;
      }

      virtual void Measure(
            ConvexHull const& convexHull,
            Measurement::ValueIterator data
      ) override {
         data[ 0 ] = convexHull.Perimeter() * scale_;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
