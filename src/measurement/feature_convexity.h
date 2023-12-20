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


class FeatureConvexity : public Composite {
   public:
      FeatureConvexity() : Composite( { "Convexity", "Ratio of perimeter of convex hull to perimeter of object (2D)", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         hasIndex_ = false;
         return out;
      }

      StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "Perimeter";
         out[ 1 ] = "ConvexPerimeter";
         return out;
      }

      void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            perimeterIndex_ = dependencies.ValueIndex( "Perimeter" );
            convexIndex_ = dependencies.ValueIndex( "ConvexPerimeter" );
            hasIndex_ = true;
         }
         dfloat perimeter = it[ perimeterIndex_ ];
         *output = perimeter == 0 ? nan : clamp( it[ convexIndex_ ] / perimeter, 0.0, 1.0 );
         // Note that convex perimeter for a more or less convex shape can be slightly larger than the
         // perimeter. These two things are estimated very differently, and have different errors.
         // We use `clamp` to prevent the value going over 1.
      }

   private:
      dip::uint perimeterIndex_;
      dip::uint convexIndex_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
