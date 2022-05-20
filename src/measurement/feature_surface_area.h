/*
 * (c)2016-2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIP_FEATURE_SURFACE_AREA_H
#define DIP_FEATURE_SURFACE_AREA_H

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

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         PhysicalQuantity unitArea = label.PixelSize().UnitLength().Power( 2 );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         out[ 0 ].name = "SurfaceArea";
         return out;
      }

      virtual void Measure( Image const& label, Image const&, Measurement::IteratorFeature& output ) override {
         std::vector< dfloat > res = SurfaceArea( label, output.Objects() );
         // Note that `res` has objects sorted in the same way as `output`.
         auto dst = output.FirstObject();
         auto src = res.begin();
         do {
            dst[ 0 ] = *src * scale_;
         } while( ++src, ++dst );
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip

#endif // DIP_FEATURE_SURFACE_AREA_H
