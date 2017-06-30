#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/morphology.h"
//#include "diplib/geometry.h"
#include "diplib/testing.h"

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_SFLOAT };
      dip::FillDelta( img );
      img.At( 6, 2 ) = 2;
      //dip::FillRadiusCoordinate( img );

      dip::testing::PrintPixelValues< dip::sfloat >( img );

      //dip::Image out = dip::Uniform( img, { { 10, 7 }, "line" } );
      dip::Image out = dip::Dilation( img, { { 17, 5 }, "interpolated line" }, { "zero order" } );
      //dip::Image out = dip::Skew( img, dip::pi/12.0, 0, 1, "", "" );

      dip::testing::PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
