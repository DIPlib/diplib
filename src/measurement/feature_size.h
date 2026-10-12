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


class FeatureSize : public LineBased {
   public:
      FeatureSize() : LineBased( { "Size", "Number of object pixels", { "Area", "Volume" }, false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
         data_.clear();
         data_.resize( nObjects, 0 );
         ValueInformationArray out( 1 );
         /*switch( label.Dimensionality() ) {
            case 1:
               out[ 0 ].name = "Length";
               break;
            case 2:
               out[ 0 ].name = "Area";
               break;
            case 3:
               out[ 0 ].name = "Volume";
               break;
            default:
               out[ 0 ].name = "Size";
         }*/
         out[ 0 ].name = "";
         PhysicalQuantity unitArea = label.PixelSize().UnitSize( label.Dimensionality() );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         return out;
      }

      void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat >, // unused
            UnsignedArray, // unused
            dip::uint, // unused
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't need to fetch the data pointer again
         LabelType objectID = 0;
         dip::uint* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it.value() ] );
                  }
               }
               if( data ) {
                  ++( *data );
               }
            }
         } while( ++label );
      }

      void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         *output = static_cast< dfloat >( data_[ objectIndex ] ) * scale_;
      }

      void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dfloat scale_{};
      std::vector< dip::uint > data_{};
};


class FeaturePolygonArea : public PolygonBased {
   // Just like FeatureSolidArea, but without the +0.5 you need for binary case.
   public:
      FeaturePolygonArea() : PolygonBased( { "Area", "Area of object", { "Size", "SolidArea" }, false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         ValueInformationArray out( 1 );
         PhysicalQuantity unitArea = label.PixelSize().UnitSize( label.Dimensionality() );
         scale_ = unitArea.magnitude;
         out[ 0 ].units = unitArea.units;
         out[ 0 ].name = "";
         return out;
      }

      void Measure( Polygon const& polygon, Measurement::ValueIterator output ) override {
         // The area of an object with a smooth outline is larger than the area of the polygon with
         // vertices along the outline because the polygon cuts off bits of the object. The larger the
         // local curvature, the larger the area cut off. But this evens out, and in the end we have
         // a fixed bias of pi s^2 / 6 for any closed curve with sufficiently low curvature (with
         // very high curvature, the approximation no longer holds as the 1/r^2 part is no longer
         // negligible). s here is the sampling distance.
         dfloat s = polygon.Length() / static_cast< dfloat >( polygon.vertices.size() );
         dfloat bias = pi * s * s / 6;
         *output = ( polygon.Area() + bias ) * scale_;
      }

   private:
      dfloat scale_{};
};


} // namespace feature
} // namespace dip
