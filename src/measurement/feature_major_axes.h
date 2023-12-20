/*
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


class FeatureMajorAxes : public Composite {
   public:
      FeatureMajorAxes() : Composite( { "MajorAxes", "Principal axes of the binary object", false } ) {};

      ValueInformationArray Initialize( Image const& label, Image const&, dip::uint /*nObjects*/ ) override {
         nD_ = label.Dimensionality();
         ValueInformationArray out = MuEigenVectorInformation( nD_, label.PixelSize() );
         hasIndex_ = false;
         return out;
      }

      StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "Mu";
         return out;
      }

      void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            muIndex_ = dependencies.ValueIndex( "Mu" );
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
