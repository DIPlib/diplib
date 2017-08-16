#undef DIP__ENABLE_DOCTEST

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>
#include <diplib/testing.h>

int main() {

   // Test ICS, 3D grey-value
   std::cout << "\nTEST ICS -- standard strides\n\n";
   dip::Image image = dip::ImageReadICS( "../test/chromo3d.ics" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );

   dip::testing::Timer timer;
   dip::ImageWriteICS( image, "test1.ics", { "line1", "line2 is good" }, 7, { "v1", "gzip" } );
   timer.Stop();
   std::cout << "Writing: " << timer << std::endl;
   timer.Reset();
   dip::Image result = dip::ImageReadICS( "test1", dip::RangeArray{}, {} );
   timer.Stop();
   std::cout << "Reading: " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   dip::ImageWriteICS( image, "test1f.ics", { "line1", "line2 is good" }, 7, { "v1", "gzip", "fast" } );
   timer.Stop();
   std::cout << "Writing (fast): " << timer << std::endl;
   timer.Reset();
   result = dip::ImageReadICS( "test1f", dip::RangeArray{}, {}, "fast" );
   timer.Stop();
   std::cout << "Reading (fast): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   result = dip::ImageReadICS( "test1f", dip::RangeArray{}, {} );
   timer.Stop();
   std::cout << "Reading (fast file): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   result = dip::ImageReadICS( "test1", dip::RangeArray{}, {}, "fast" );
   timer.Stop();
   std::cout << "Reading (fast, regular file): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   // Turn it on its side so the image to write has non-standard strides
   std::cout << "\nTEST ICS -- non-standard strides\n\n";
   image.SwapDimensions( 0, 2 );

   timer.Reset();
   dip::ImageWriteICS( image, "test2.ics", { "key\tvalue" }, 7, { "v1", "gzip" } );
   timer.Stop();
   std::cout << "Writing: " << timer << std::endl;
   timer.Reset();
   result = dip::ImageReadICS( "test2", dip::RangeArray{}, {} );
   timer.Stop();
   std::cout << "Reading: " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   dip::ImageWriteICS( image, "test2f.ics", { "key\tvalue" }, 7, { "v1", "gzip", "fast" } );
   timer.Stop();
   std::cout << "Writing (fast): " << timer << std::endl;
   timer.Reset();
   result = dip::ImageReadICS( "test2f", dip::RangeArray{}, {}, "fast" );
   timer.Stop();
   std::cout << "Reading (fast): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   result = dip::ImageReadICS( "test2f", dip::RangeArray{}, {} );
   timer.Stop();
   std::cout << "Reading (fast file): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   timer.Reset();
   result = dip::ImageReadICS( "test2", dip::RangeArray{}, {}, "fast" );
   timer.Stop();
   std::cout << "Reading (fast, regular file): " << timer << std::endl;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   // Test TIFF, 2D grey-value

   std::cout << "\nTEST TIFF\n\n";
   image = dip::ImageReadICS( "../test/trui.ics" );
   image.SetPixelSize( dip::PhysicalQuantityArray{ 6 * dip::Units::Micrometer(), 300 * dip::Units::Nanometer() } );
   std::cout << "Input image: " << image << std::endl;

   timer.Reset();
   dip::ImageWriteTIFF( image, "test1.tif" );
   timer.Stop();
   std::cout << "Writing: " << timer << std::endl;
   timer.Reset();
   result = dip::ImageReadTIFF( "test1" );
   timer.Stop();
   std::cout << "Reading: " << timer << std::endl;
   std::cout << "Image read back: " << result;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   // Try reading it into an image with non-standard strides
   timer.Reset();
   result.Strip();
   auto strides = result.Strides();
   strides[ 0 ] = static_cast< dip::sint >( result.Size( 1 ));
   strides[ 1 ] = 1;
   result.SetStrides( strides );
   result.Forge();
   dip::ImageReadTIFF( result, "test1" );
   timer.Stop();
   std::cout << "Reading: " << timer << std::endl;
   std::cout << "Image read back into non-standard strides: " << result;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   // Turn it on its side so the image to write has non-standard strides
   image.SwapDimensions( 0, 1 );
   std::cout << "Input image with non-standard strides: " << result;
   timer.Reset();
   dip::ImageWriteTIFF( image, "test2.tif" );
   timer.Stop();
   std::cout << "Writing: " << timer << std::endl;
   timer.Reset();
   result = dip::ImageReadTIFF( "test2" );
   timer.Stop();
   std::cout << "Reading: " << timer << std::endl;
   std::cout << "Image read back: " << result;
   std::cout << ( dip::testing::CompareImages( image, result ) ? "Identical\n\n" : "!!!ERROR!!!\n\n" );

   return 0;
}
