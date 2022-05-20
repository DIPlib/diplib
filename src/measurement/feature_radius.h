/*
 * (c)2016-2022, Cris Luengo.
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


class FeatureRadius : public PolygonBased {
   public:
      FeatureRadius() : PolygonBased( { "Radius", "Statistics on radius of object (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 4 );
         PhysicalQuantity pq = label.PixelSize().UnitLength();
         scale_ = pq.magnitude;
         out[ 0 ].units = pq.units;
         out[ 1 ].units = pq.units;
         out[ 2 ].units = pq.units;
         out[ 3 ].units = pq.units;
         out[ 0 ].name = "Max";
         out[ 1 ].name = "Mean";
         out[ 2 ].name = "Min";
         out[ 3 ].name = "StdDev";
         return out;
      }

      virtual void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         RadiusValues radius = polygon.RadiusStatistics();
         output[ 0 ] = radius.Maximum() * scale_;
         output[ 1 ] = radius.Mean() * scale_;
         output[ 2 ] = radius.Minimum() * scale_;
         output[ 3 ] = radius.StandardDeviation() * scale_;
      }

   private:
      dfloat scale_;
};


} // namespace feature
} // namespace dip
