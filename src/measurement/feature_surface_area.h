/*
 * DIPlib 3.0
 * This file defines the "SurfaceArea" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {


std::vector< dfloat > SurfaceArea(
      Image const& label,
      UnsignedArray const& objectIDs
);


namespace Feature {


class FeatureSurfaceArea : public ImageBased {
   public:
      FeatureSurfaceArea() : ImageBased( { "SurfaceArea", "surface area of object (3D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& ) override {
         DIP_THROW_IF( label.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         PhysicalQuantity pq = label.PixelSize( 0 );
         if( label.IsIsotropic() && pq.IsPhysical() ) {
            pq *= pq;
            scale_ = pq.magnitude;
            out[ 0 ].units = pq.units;
         } else {
            scale_ = 1;
            out[ 0 ].units = Units::SquarePixel();
         }
         out[ 0 ].name = "SurfaceArea";
         return out;
      }

      virtual void Measure(
            Image const& label,
            Image const&,
            Measurement::IteratorFeature& data
      ) override {
         std::vector< dfloat > res = SurfaceArea( label, data.Objects() );
         // Note that `res` has objects sorted in the same way as `data`.
         auto dst = data.FirstObject();
         auto src = res.begin();
         do {
            dst[ 0 ] = *src;
         } while( ++src, ++dst );
      }

      virtual void Cleanup() override {}

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
