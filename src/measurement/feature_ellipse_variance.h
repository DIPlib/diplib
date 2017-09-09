/*
 * DIPlib 3.0
 * This file defines the "EllipseVariance" measurement feature
 *
 * (c)2017, Cris Luengo.
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


class FeatureEllipseVariance : public PolygonBased {
   public:
      FeatureEllipseVariance() : PolygonBased( { "EllipseVariance", "Ellipse variance (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const&, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         return out;
      }

      virtual void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         *output = polygon.EllipseVariance();
      }
};


} // namespace feature
} // namespace dip
