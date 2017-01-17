/*
 * DIPlib 3.0
 * This file defines the "AspectRatioFeret" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureAspectRatioFeret : public Composite {
   public:
      FeatureAspectRatioFeret() : Composite( { "AspectRatioFeret", "Feret-based aspect ratio (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "AspectRatioFeret";
         feretIndex_ = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 1 );
         out[ 0 ] = "Feret";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( feretIndex_ == -1 ) {
            feretIndex_ = dependencies.ValueIndex( "Feret" );
         }
         dfloat minDiameter = it[ feretIndex_ + 1 ];
         *output = minDiameter == 0 ? std::nan( "" ) : it[ feretIndex_ + 2 ] / minDiameter;
      }

   private:
      dip::sint feretIndex_;
};


} // namespace feature
} // namespace dip
