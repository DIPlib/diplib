/*
 * DIPlib 3.0
 * This file defines the "EllipseVariance" measurement feature
 *
 * (c)2017, Cris Luengo.
 */


#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureEllipseVariance : public PolygonBased {
   public:
      FeatureEllipseVariance() : PolygonBased( { "EllipseVariance", "Ellipse variance (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const&, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         out[ 0 ].name = "EllipseVariance";
         return out;
      }

      virtual void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         *output = polygon.EllipseVariance();
      }
};


} // namespace feature
} // namespace dip
