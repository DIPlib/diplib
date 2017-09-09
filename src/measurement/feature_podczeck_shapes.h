/*
 * DIPlib 3.0
 * This file defines the "PodczeckShapes" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
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


class FeaturePodczeckShapes : public Composite {
   public:
      FeaturePodczeckShapes() : Composite( { "PodczeckShapes", "Podczeck shape descriptors (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 5 );
         out[ 0 ].name = "Square";
         out[ 1 ].name = "Circle";
         out[ 2 ].name = "Triangle";
         out[ 3 ].name = "Ellipse";
         out[ 4 ].name = "Elongation";
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 3 );
         out[ 0 ] = "Size";
         out[ 1 ] = "Feret";
         out[ 2 ] = "Perimeter";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            sizeIndex_ = dependencies.ValueIndex( "Size" );
            feretIndex_ = dependencies.ValueIndex( "Feret" );
            perimeterIndex_ = dependencies.ValueIndex( "Perimeter" );
            hasIndex_ = true;
         }
         dfloat area = it[ sizeIndex_ ];
         if( area == 0 ) {
            output[ 0 ] = output[ 1 ] = output[ 2 ] = output[ 3 ] = output[ 4 ] = nan;
         } else {
            dfloat length = it[ feretIndex_ ];
            dfloat boxWidth = it[ feretIndex_ + 1 ];
            dfloat boxHeight = it[ feretIndex_ + 2 ];
            dfloat perimeter = it[ perimeterIndex_ ];
            output[ 0 ] = area / ( boxWidth * boxHeight );
            output[ 1 ] = area / ( 0.25 * dip::pi * boxHeight * boxHeight );
            output[ 2 ] = area / ( 0.5 * boxWidth * boxHeight );
            output[ 3 ] = area / ( 0.25 * dip::pi * boxWidth * boxHeight );
            output[ 4 ] = perimeter / length;
         }
      }

   private:
      dip::uint sizeIndex_;
      dip::uint feretIndex_;
      dip::uint perimeterIndex_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
