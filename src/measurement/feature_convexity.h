/*
 * DIPlib 3.0
 * This file defines the "Convexity" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureConvexity : public Composite {
   public:
      FeatureConvexity() : Composite( { "Convexity", "Area fraction of convex hull covered by object (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 1 );
         out[ 0 ].name = "Convexity";
         sizeIndex_ = -1;
         convexIndex_ = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 2 );
         out[ 0 ] = "Size";
         out[ 1 ] = "ConvexArea";
         return out;
      }

      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) override {
         auto it = dependencies.FirstFeature();
         if( sizeIndex_ == -1 ) {
            sizeIndex_ = dependencies.ValueIndex( "Size" );
            convexIndex_ = dependencies.ValueIndex( "ConvexArea" );
         }
         dfloat convArea = it[ convexIndex_ ];
         *output = convArea == 0 ? std::nan( "" ) : it[ sizeIndex_ ] / convArea;
      }

   private:
      dip::sint sizeIndex_;
      dip::sint convexIndex_;
};


} // namespace feature
} // namespace dip
