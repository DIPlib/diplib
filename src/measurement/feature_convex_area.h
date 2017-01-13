/*
 * DIPlib 3.0
 * This file defines the "ConvexArea" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureConvexArea : public ConvexHullBased {
   public:
      FeatureConvexArea() : ConvexHullBased( { "ConvexArea", "Area of the convex hull (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         out[ 0 ].name = "Area";
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude * pq.magnitude;
            out[ 0 ].units = pq.units * pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::SquarePixel();
         }
         return out;
      }

      virtual void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) override {
         output[ 0 ] = convexHull.Area() * scale_;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
