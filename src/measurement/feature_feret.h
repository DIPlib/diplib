/*
 * DIPlib 3.0
 * This file defines the "Feret" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)2011, Cris Luengo.
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


namespace dip {
namespace Feature {


class FeatureFeret : public ConvexHullBased {
   public:
      FeatureFeret() : ConvexHullBased( { "Feret", "Maximum and minimum object diameters (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
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
         out[ 0 ].name = "Max";
         out[ 1 ].name = "Min";
         out[ 2 ].name = "PerpMin";
         out[ 3 ].name = "MaxAng";
         out[ 4 ].name = "MinAng";
         return out;
      }

      virtual void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) override {
         FeretValues feret = convexHull.Feret();
         output[ 0 ] = feret.maxDiameter * scale_;
         output[ 1 ] = feret.minDiameter * scale_;
         output[ 2 ] = feret.maxPerpendicular * scale_;
         output[ 3 ] = feret.maxAngle;
         output[ 4 ] = feret.minAngle;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
