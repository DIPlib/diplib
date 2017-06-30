#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/morphology.h"
#include "diplib/testing.h"

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_SFLOAT };
      dip::FillDelta( img );
      img.At( 6, 2 ) = 2;
      //dip::FillRadiusCoordinate( img );

      dip::testing::PrintPixelValues< dip::sfloat >( img );

      auto se = dip::StructuringElement{ { 8, -2 }, "discrete line" };
      dip::Image out = dip::Dilation( img, se, { "zero order" } );
      dip::testing::PrintPixelValues< dip::sfloat >( out );
      se.Mirror();
      out = dip::Erosion( out, se, { "zero order" } );
      dip::testing::PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
