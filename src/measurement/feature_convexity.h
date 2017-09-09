/*
 * DIPlib 3.0
 * This file defines the "Convexity" measurement feature
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


class FeatureConvexity : public Composite {
   public:
      FeatureConvexity() : Composite( { "Convexity", "Area fraction of convex hull covered by object (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "Size";
         out[ 1 ] = "ConvexArea";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            sizeIndex_ = dependencies.ValueIndex( "Size" );
            convexIndex_ = dependencies.ValueIndex( "ConvexArea" );
            hasIndex_ = true;
         }
         dfloat convArea = it[ convexIndex_ ];
         *output = convArea == 0 ? nan : it[ sizeIndex_ ] / convArea;
      }

   private:
      dip::uint sizeIndex_;
      dip::uint convexIndex_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
