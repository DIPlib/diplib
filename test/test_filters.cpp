#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/morphology.h"
#include "diplib/geometry.h"
#include "diplib/pixel_table.h"
#include "diplib/testing.h"

int main() {
   try {
      if( false ) {

         dip::Image img{ dip::UnsignedArray{ 64, 41 }, 1, dip::DT_UINT8 };
         dip::FillDelta( img );
         img.At( 7, 5 ) = 2;
         dip::testing::PrintPixelValues< dip::uint8, 1 >( img );

         dip::Image out;
         auto se = dip::StructuringElement{{ 9, 6 }, "line" };
         dip::Dilation( img, out, se, {} );
         dip::testing::PrintPixelValues< dip::uint8, 1 >( out );

         //se.Mirror();
         //dip::Erosion( out, out, se, {} );
         //out = dip::Closing( img, se, { "zero order" } );
         //dip::testing::PrintPixelValues< dip::uint8, 1 >( out );

      } else if( false ) {

         dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_SINT8 };
         //dip::FillRadiusCoordinate( img );
         dip::FillRamp( img, 1 );
         dip::testing::PrintPixelValues< dip::sint8 >( img );

         dip::Image out;
         dip::Skew( img, out, { 0.0, 8.0/9.0 }, 0, 0, "nn", { dip::BoundaryCondition::ZERO_ORDER_EXTRAPOLATE } );
         dip::testing::PrintPixelValues< dip::sint8 >( out );

         //dip::Skew( out, out, { 0.0, -8.0/9.0 }, 0, 0, "nn2", { dip::BoundaryCondition::ZERO_ORDER_EXTRAPOLATE } );
         //dip::testing::PrintPixelValues< dip::sint8 >( out );

      } else {

         dip::PixelTable pixelTable1( "line", { 8.0, 9.0 } );
         dip::testing::PrintPixelValues< dip::bin >( pixelTable1.AsImage() );
         std::cout << pixelTable1.Origin() << std::endl;

         dip::PixelTable pixelTable2( "line", { 8.0, -9.0 } );
         dip::testing::PrintPixelValues< dip::bin >( pixelTable2.AsImage() );
         std::cout << pixelTable2.Origin() << std::endl;

         dip::PixelTable pixelTable3( "line", { -8.0, 9.0 } );
         dip::testing::PrintPixelValues< dip::bin >( pixelTable3.AsImage() );
         std::cout << pixelTable3.Origin() << std::endl;

      }
   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
