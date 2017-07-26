#undef DIP__ENABLE_DOCTEST

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

int main() {
   dip::Image image = dip::ImageReadICS( "../test/chromo3d.ics" );
   std::cout << image << std::endl;

   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );
   std::cout << image << std::endl;

   dip::ImageWriteICS( image, "test1.ics", { "line1", "line2 is good" }, 7, { "v1", "gzip" } );

   image.SwapDimensions( 0, 1 );
   std::cout << image << std::endl;

   dip::ImageWriteICS( image, "test2.ics", { "key\tvalue" }, 7, { "v1", "gzip" } );

   return 0;
}
