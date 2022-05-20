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


class FeatureRoundness : public Composite {
   public:
      FeatureRoundness() : Composite( { "Roundness", "Roundness of the object (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         hasIndex_ = false;
         scale_ = ReverseSizeScale( 2, label.PixelSize() );
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "SolidArea";
         out[ 1 ] = "Perimeter";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            sizeIndex_ = dependencies.ValueIndex( "SolidArea" );
            perimIndex_ = dependencies.ValueIndex( "Perimeter" );
            hasIndex_ = true;
         }
         dfloat perimeter = it[ perimIndex_ ];
         if( perimeter == 0 ) {
            *output = nan;
         } else {
            dfloat area = it[ sizeIndex_ ] * scale_;
            *output = clamp(( 4.0 * pi * area ) / ( perimeter * perimeter ), 0.0, 1.0);
            // Note that perimeter estimate is not perfect, and the ratio could potentially go over 1.
            // We use `clamp` to prevent this.
         }
      }

   private:
      dip::uint sizeIndex_;
      dip::uint perimIndex_;
      dfloat scale_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
