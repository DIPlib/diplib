/*
 * DIPlib 3.0
 * This file defines the "P2A" measurement feature
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


class FeatureP2A : public Composite {
   public:
      FeatureP2A() : Composite( { "P2A", "Circularity of the object (2D & 3D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         nD_ = label.Dimensionality();
         DIP_THROW_IF(( nD_ < 2 ) || ( nD_ > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "";
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "Size";
         out[ 1 ] = nD_ == 2 ? "Perimeter" : "SurfaceArea";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            sizeIndex_ = dependencies.ValueIndex( "Size" );
            if( nD_ == 2 ) {
               perimIndex_ = dependencies.ValueIndex( "Perimeter" );
            } else  {
               perimIndex_ = dependencies.ValueIndex( "SurfaceArea" );
            }
            hasIndex_ = true;
         }
         dfloat area = it[ sizeIndex_ ];
         if( area == 0 ) {
            *output = nan;
         } else {
            dfloat perimeter = it[ perimIndex_ ];
            if( nD_ == 2 ) {
               *output = ( perimeter * perimeter ) / ( 4.0 * pi * area );
            } else {
               *output = std::pow( perimeter, 1.5 ) / ( 6.0 * std::sqrt( pi ) * area );
            }
         }
      }

   private:
      dip::uint sizeIndex_;
      dip::uint perimIndex_;
      dip::uint nD_;
      bool hasIndex_;
};


} // namespace feature
} // namespace dip
