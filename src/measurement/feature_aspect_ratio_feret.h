/*
 * DIPlib 3.0
 * This file defines the "AspectRatioFeret" measurement feature
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


class FeatureAspectRatioFeret : public Composite {
   public:
      FeatureAspectRatioFeret() : Composite( { "AspectRatioFeret", "Feret-based aspect ratio (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "Feret";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            feretIndex_ = dependencies.ValueIndex( "Feret" );
            hasIndex_ = true;
         }
         dfloat minDiameter = it[ feretIndex_ + 1 ];
         *output = minDiameter == 0 ? nan : it[ feretIndex_ + 2 ] / minDiameter;
      }

   private:
      dip::uint feretIndex_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
