/*
 * (c)2018, Cris Luengo.
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


namespace dip {
namespace Feature {


class FeatureCircularity : public PolygonBased {
   public:
      FeatureCircularity() : PolygonBased( { "Circularity", "Circularity of the object (2D)", false } ) {};

      ValueInformationArray Initialize( Image const&, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         return out;
      }

      void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         output[ 0 ] = polygon.RadiusStatistics().Circularity();
      }
};


} // namespace feature
} // namespace dip
