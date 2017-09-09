/*
 * DIPlib 3.0
 * This file defines the "GreyMajorAxes" measurement feature
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


class FeatureGreyMajorAxes : public Composite {
   public:
      FeatureGreyMajorAxes() : Composite( { "GreyMajorAxes", "Grey-weighted principal axes of the object", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint /*nObjects*/ ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         ValueInformationArray out( nD_ * nD_ );
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            for( dip::uint jj = 0; jj < nD_; ++jj ) {
               out[ ii * nD_ + jj ].name = String( "v" ) + std::to_string( ii ) + "_" + std::to_string( jj );
            }
         }
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "GreyMu";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            muIndex_ = dependencies.ValueIndex( "GreyMu" );
            hasIndex_ = true;
         }
         dfloat const* data = &it[ muIndex_ ];
         FloatArray tmp( nD_ );
         SymmetricEigenDecompositionPacked( nD_, data, tmp.data(), output );
      }

   private:
      dip::uint muIndex_;
      bool hasIndex_;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
