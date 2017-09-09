/*
 * DIPlib 3.0
 * This file defines the "DimensionsEllipsoid" measurement feature
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


class FeatureDimensionsEllipsoid : public Composite {
   public:
      FeatureDimensionsEllipsoid() : Composite( { "DimensionsEllipsoid", "Extent along the principal axes of an ellipsoid", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint /*nObjects*/ ) override {
         nD_ = label.Dimensionality();
         DIP_THROW_IF(( nD_ < 2 ) || ( nD_ > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( nD_ );
         PhysicalQuantity pq = label.PixelSize( 0 );
         bool sameUnits = pq.IsPhysical();
         if( sameUnits ) {
            for( dip::uint ii = 1; ii < nD_; ++ii ) {
               if( label.PixelSize( ii ).units != pq.units ) {
                  // This tests false if the SI prefix differs. This is intentional, as the Mu values will be given
                  // with different SI prefixes and we'd need complex logic here to fix it.
                  sameUnits = false;
                  break;
               }
            }
         }
         Units units = sameUnits ? pq.units : Units::Pixel();
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            out[ ii ].units = units;
            out[ ii ].name = String( "axis" ) + std::to_string( ii );
         }
         hasIndex_ = false;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "Inertia";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( !hasIndex_ ) {
            inertiaIndex_ = dependencies.ValueIndex( "Inertia" );
            hasIndex_ = true;
         }
         dfloat const* data = &it[ inertiaIndex_ ];
         if( nD_ == 2 ) {
            output[ 0 ] = std::sqrt( 16 * data[ 0 ] );
            output[ 1 ] = std::sqrt( 16 * data[ 1 ] );
         } else { // nD_ == 3
            output[ 0 ] = std::sqrt( 10 * (   data[ 0 ] + data[ 1 ] - data[ 2 ] ));
            output[ 1 ] = std::sqrt( 10 * (   data[ 0 ] - data[ 1 ] + data[ 2 ] ));
            output[ 2 ] = std::sqrt( 10 * ( - data[ 0 ] + data[ 1 ] + data[ 2 ] ));
         }
      }

   private:
      dip::uint inertiaIndex_;
      bool hasIndex_;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
