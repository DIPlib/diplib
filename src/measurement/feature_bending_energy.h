/*
 * DIPlib 3.0
 * This file defines the "BendingEnergy" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureBendingEnergy : public ChainCodeBased {
   public:
      FeatureBendingEnergy() : ChainCodeBased( { "BendingEnergy", "Bending energy of object perimeter (chain-code method, 2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            scale_ = pq.magnitude * pq.magnitude;
            out[ 0 ].units = pq.units * pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::SquarePixel();
         }
         out[ 0 ].name = "BendingEnergy";
         return out;
      }

      virtual void Measure( ChainCode const& chainCode, Measurement::ValueIterator output ) override {
         *output = chainCode.BendingEnergy() * scale_;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
