/*
 * DIPlib 3.0
 * This file defines the "Inertia" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"

#include <Eigen/Eigenvalues>


namespace dip {
namespace Feature {


class FeatureInertia : public Composite {
   public:
      FeatureInertia() : Composite( { "Inertia", "Moments of inertia of binary object", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint nObjects ) override {
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
         units *= units;
         for( dip::uint ii = 0; ii < nD_; ++ii ) {
            out[ ii ].units = units;
            out[ ii ].name = String( "dim" ) + std::to_string( ii );
         }
         muIndex_ = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "Mu";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( muIndex_ == -1 ) {
            muIndex_ = dependencies.ValueIndex( "Mu" );
         }
         dfloat* data = &it[ muIndex_ ];
         if( nD_ == 2 ) {
            SymmetricEigenValues2DPacked( data, output );
         } else { // nD_ == 3
            SymmetricEigenValues3DPacked( data, output );
         }
      }

   private:
      dip::sint muIndex_;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
