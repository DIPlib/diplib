/*
 * DIPlib 3.0
 * This file defines the "Perimeter" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeaturePerimeter : public ChainCodeBased {
   public:
      FeaturePerimeter() : ChainCodeBased( { "Perimeter", "length of the object perimeter  (chain-code method, 2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude;
            out[ 0 ].units = pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::Pixel();
         }
         out[ 0 ].name = "Perimeter";
         return out;
      }

      virtual void Measure(
            ChainCode const& chainCode,
            Measurement::ValueIterator data
      ) override {
         *data = ( chainCode.Length() + pi ) * scale_;
      }

      virtual void Cleanup() override {}

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
