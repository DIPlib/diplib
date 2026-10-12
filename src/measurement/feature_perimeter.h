/*
 * (c)2016-2026, Cris Luengo.
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


class FeaturePerimeter : public ChainCodeBased {
   public:
      FeaturePerimeter() : ChainCodeBased( { "Perimeter", "Length of the object perimeter", false } ) {};

      void Configure( String const& parameter, dfloat value ) override {
         if( parameter == "include boundary pixels" ) {
            includeBoundaryPixels = value != 0;
         } else {
            DIP_THROW_INVALID_FLAG( parameter );
         }
      }

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity pq = label.PixelSize().UnitLength();
         scale_ = pq.magnitude;
         out[ 0 ].units = pq.units;
         out[ 0 ].name = "";
         return out;
      }

      void Measure( ChainCode const& chainCode, Measurement::ValueIterator output ) override {
         *output = ( chainCode.Length( includeBoundaryPixels ? S::INCLUDE : S::EXCLUDE ) + pi ) * scale_;
      }

   private:
      dfloat scale_{};
      bool includeBoundaryPixels = false;
};


class FeaturePolygonPerimeter : public PolygonBased {
   public:
      FeaturePolygonPerimeter() : PolygonBased( { "Perimeter", "Length of the object perimeter", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity pq = label.PixelSize().UnitLength();
         scale_ = pq.magnitude;
         out[ 0 ].units = pq.units;
         out[ 0 ].name = "";
         return out;
      }

      void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         *output = polygon.Length() * scale_;
      }

   private:
      dfloat scale_{};
};


} // namespace feature
} // namespace dip
