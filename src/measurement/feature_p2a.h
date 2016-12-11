/*
 * DIPlib 3.0
 * This file defines the "P2A" measurement feature
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureP2A : public Composite {
   public:
      FeatureP2A() : Composite( info_ ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& ) override {
         nD_ = label.Dimensionality();
         DIP_THROW_IF((( nD_ < 2 ) || ( nD_ > 3 )), E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "P2A";
         sizeIndex = -1;
         perimIndex = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "Size";
         out[ 1 ] = nD_ == 2 ? "Perimeter" : "SurfaceArea";
         return out;
      }

      virtual void Measure(
            Measurement::IteratorObject& dependencies,
            Measurement::ValueIterator data
      ) override {
         auto it = dependencies.FirstFeature();
         if( sizeIndex == -1 ) {
            sizeIndex = dependencies.ValueIndex( "Size" );
            if( nD_ == 2 ) {
               perimIndex = dependencies.ValueIndex( "Perimeter" );
            } else  {
               perimIndex = dependencies.ValueIndex( "SurfaceArea" );
            }
         }
         dfloat area = it[ sizeIndex ];
         dfloat perimeter = it[ perimIndex ];
         if( nD_ == 2 ) {
            *data = ( perimeter * perimeter ) / ( 4.0 * pi * area );
         } else {
            *data = std::pow( perimeter, 1.5 ) / ( 6.0 * std::sqrt( pi ) * area );
         }
      }

      virtual void Cleanup() override {}

   private:
      static constexpr Information info_ { "P2A", "circularity of the object (2D & 3D)", false };
      dip::sint sizeIndex;
      dip::sint perimIndex;
      dip::uint nD_;
};


} // namespace feature
} // namespace dip
