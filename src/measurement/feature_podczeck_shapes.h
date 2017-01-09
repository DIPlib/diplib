/*
 * DIPlib 3.0
 * This file defines the "PodczeckShapes" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeaturePodczeckShapes : public Composite {
   public:
      FeaturePodczeckShapes() : Composite( { "PodczeckShapes", "Podczeck shape descriptors (2D)", false } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const&, dip::uint ) override {
         DIP_THROW_IF( label.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         ValueInformationArray out( 5 );
         out[ 0 ].name = "Square";
         out[ 0 ].name = "Circle";
         out[ 0 ].name = "Triangle";
         out[ 0 ].name = "Ellipse";
         out[ 0 ].name = "Elongation";
         sizeIndex = -1;
         feretIndex = -1;
         perimeterIndex = -1;
         return out;
      }

      virtual StringArray Dependencies() override {
         StringArray out( 3 );
         out[ 0 ] = "Size";
         out[ 1 ] = "ConvexArea";
         out[ 2 ] = "Perimeter";
         return out;
      }

      virtual void Compose(
            Measurement::IteratorObject& dependencies,
            Measurement::ValueIterator data
      ) override {
         auto it = dependencies.FirstFeature();
         if( sizeIndex == -1 ) {
            sizeIndex = dependencies.ValueIndex( "Size" );
            feretIndex = dependencies.ValueIndex( "ConvexArea" );
            perimeterIndex = dependencies.ValueIndex( "Perimeter" );
         }
         dfloat area = it[ sizeIndex ];
         if( area == 0 ) {
            data[ 0 ] = data[ 1 ] = data[ 2 ] = data[ 3 ] = data[ 4 ] = std::nan( "" );
         } else {
            dfloat length = it[ feretIndex ];
            dfloat boxWidth = it[ feretIndex + 1 ];
            dfloat boxLength = it[ feretIndex + 2 ];
            dfloat perimeter = it[ perimeterIndex ];
            data[ 0 ] = area / ( boxWidth * boxLength );
            data[ 1 ] = area / ( 0.25 * dip::pi * boxLength * boxLength );
            data[ 2 ] = area / ( 0.5 * boxWidth * boxLength );
            data[ 3 ] = area / ( 0.25 * dip::pi * boxWidth * boxLength );
            data[ 4 ] = perimeter / length;
         }
      }

   private:
      dip::sint sizeIndex;
      dip::sint feretIndex;
      dip::sint perimeterIndex;
};


} // namespace feature
} // namespace dip
