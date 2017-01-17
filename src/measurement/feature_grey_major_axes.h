/*
 * DIPlib 3.0
 * This file defines the "GreyMajorAxes" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
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
