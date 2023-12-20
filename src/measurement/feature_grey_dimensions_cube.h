/*
 * (c)2016-2022, Cris Luengo.
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


class FeatureGreyDimensionsCube : public Composite {
   public:
      FeatureGreyDimensionsCube() : Composite( { "GreyDimensionsCube", "Extent along the principal axes of a cube (grey-weighted, 2D & 3D) ", true } ) {};

      ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint /*nObjects*/ ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         DIP_THROW_IF(( nD_ < 2 ) || ( nD_ > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out = MuSqrtEigenValueInformation( nD_, label.PixelSize() );
         hasIndex_ = false;
         return out;
      }

      StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "GreyMu";
         return out;
      }

      void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            muIndex_ = dependencies.ValueIndex( "GreyMu" );
            hasIndex_ = true;
         }
         dfloat const* data = &it[ muIndex_ ];
         dfloat eig[ 3 ]; // We never have more than 3 eigenvectors.
         SymmetricEigenDecompositionPacked( nD_, data, eig );
         if( nD_ == 2 ) {
            output[ 0 ] = std::sqrt( 12.0 * eig[ 0 ] );
            output[ 1 ] = std::sqrt( 12.0 * eig[ 1 ] );
         } else { // nD_ == 3
            output[ 0 ] = std::sqrt( 6.0 * (   eig[ 0 ] + eig[ 1 ] - eig[ 2 ] ));
            output[ 1 ] = std::sqrt( 6.0 * (   eig[ 0 ] - eig[ 1 ] + eig[ 2 ] ));
            output[ 2 ] = std::sqrt( 6.0 * ( - eig[ 0 ] + eig[ 1 ] + eig[ 2 ] ));
         }
      }

   private:
      dip::uint muIndex_;
      bool hasIndex_;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
