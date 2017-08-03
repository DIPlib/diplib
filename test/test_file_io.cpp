#undef DIP__ENABLE_DOCTEST

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>
#include <diplib/statistics.h>

int main() {
   dip::Image image = dip::ImageReadICS( "../test/chromo3d.ics" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );
   std::cout << image << std::endl;

   dip::ImageWriteICS( image, "test1.ics", { "line1", "line2 is good" }, 7, { "v1", "gzip" } );
   dip::Image result = dip::ImageReadICS( "test1" );
   std::cout << result;
   if( dip::All( image == result ).As< bool >() ) {
      std::cout << "Identical\n\n";
   } else {
      std::cout << "!!!ERROR!!!\n\n";
   }

   image.SwapDimensions( 0, 1 );
   std::cout << image << std::endl;
   dip::ImageWriteICS( image, "test2.ics", { "key\tvalue" }, 7, { "v1", "gzip" } );
   result = dip::ImageReadICS( "test2" );
   std::cout << result;
   if( dip::All( image == result ).As< bool >() ) {
      std::cout << "Identical\n\n";
   } else {
      std::cout << "!!!ERROR!!!\n\n";
   }

   image = dip::ImageReadICS( "../test/trui.ics" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );
   std::cout << image << std::endl;

   dip::ImageWriteTIFF( image, "test1.tif" );
   result = dip::ImageReadTIFF( "test1" );
   std::cout << result;
   if( dip::All( image == result ).As< bool >() ) {
      std::cout << "Identical\n\n";
   } else {
      std::cout << "!!!ERROR!!!\n\n";
   }

   image.SwapDimensions( 0, 1 );
   std::cout << image << std::endl;
   dip::ImageWriteTIFF( image, "test2.tif" );
   result = dip::ImageReadTIFF( "test2" );
   std::cout << result;
   if( dip::All( image == result ).As< bool >() ) {
      std::cout << "Identical\n\n";
   } else {
      std::cout << "!!!ERROR!!!\n\n";
   }

   return 0;
}
