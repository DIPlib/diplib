/*
 * (c)2016-2026, Cris Luengo.
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


class FeatureConvexArea : public ConvexHullBased {
   public:
      FeatureConvexArea() : ConvexHullBased( { "ConvexArea", "Area of the convex hull", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity unitArea = label.PixelSize().UnitSize( label.Dimensionality() );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         out[ 0 ].name = "";
         return out;
      }

      void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) override {
         output[ 0 ] = ( convexHull.Area() + 0.5 ) * scale_;
      }

   private:
      dfloat scale_{};
};


class FeaturePolygonConvexArea : public ConvexHullBased {
   public:
      FeaturePolygonConvexArea() : ConvexHullBased( { "ConvexArea", "Area of the convex hull", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity unitArea = label.PixelSize().UnitSize( label.Dimensionality() );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         out[ 0 ].name = "";
         return out;
      }

      void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) override {
         // See explanation for bias in FeaturePolygonArea
         dfloat s = convexHull.Length() / static_cast< dfloat >( convexHull.vertices.size() );
         dfloat bias = pi * s * s / 6;
         output[ 0 ] = ( convexHull.Area() + bias ) * scale_;
      }

   private:
      dfloat scale_{};
};


} // namespace feature
} // namespace dip
