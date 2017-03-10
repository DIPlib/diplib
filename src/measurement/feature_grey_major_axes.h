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

#include "diplib.h"
#include "diplib/measurement.h"

#include <Eigen/Eigenvalues>

namespace dip {
namespace Feature {


class FeatureGreyMajorAxes : public Composite {
   public:
      FeatureGreyMajorAxes() : Composite( { "GreyMajorAxes", "Grey-weighted principal axes of the object", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::IMAGE_NOT_SCALAR );
         nD_ = label.Dimensionality();
         DIP_THROW_IF(( nD_ < 2 ) || ( nD_ > 3 ), E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( nD_ * nD_ );
         constexpr char const* dim = "xyz";
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            for( dip::uint jj = 0; jj < nD_; ++jj ) {
               out[ ii * nD_ + jj ].name = String( "v" ) + std::to_string( ii ) + "_" + dim[ jj ];
            }
         }
         muIndex_ = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "GreyMu";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( muIndex_ == -1 ) {
            muIndex_ = dependencies.ValueIndex( "GreyMu" );
         }
         dfloat const* data = &it[ muIndex_ ];
         dfloat tmp[ 3 ];
         if( nD_ == 2 ) {
            SymmetricEigenSystem2DPacked( data, tmp, output );
         } else { // nD_ == 3
            SymmetricEigenSystem3DPacked( data, tmp, output );
         }
      }

   private:
      dip::sint muIndex_;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
