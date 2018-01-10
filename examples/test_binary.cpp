#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/file_io.h"
#include "diplib/iterators.h"
#include "diplib/generation.h"
#include "diplib/testing.h"
#include <iomanip>

// Testing binary morphological functions

// Define some utility functions used in this test
namespace
{

// Int to string with padded zeros
std::string GetPaddedNumberString( size_t number, int numDigits ) {
   std::ostringstream ss;
   ss << std::setfill( '0' ) << std::setw( numDigits ) << number;
   return ss.str();
}

// Write TIFF image with options convenient for this test
void TestWriteTIFF( dip::Image const& in, std::string const& filename ) {
   dip::ImageWriteTIFF( in, filename, "none" );
}

// Write a series of TIFF images with options convenient for this test
void TestWriteTIFFs( std::vector< dip::Image > const& images, std::string const& filenameBase ) {
   for( size_t iImg = 0; iImg < images.size(); ++iImg ) {
      TestWriteTIFF( images[iImg], filenameBase + GetPaddedNumberString( iImg, 2 ) );
   }
}

// Write ICS image with options convenient for this test
void TestWriteICS( dip::Image const& in, std::string const& filename ) {
   dip::ImageWriteICS( in, filename, {}, 0, { "v1", "uncompressed" } );
}

// Write a series of ICS images with options convenient for this test
void TestWriteICSs( std::vector< dip::Image > const& images, std::string const& filenameBase ) {
   for( size_t iImg = 0; iImg < images.size(); ++iImg ) {
      TestWriteICS( images[iImg], filenameBase + GetPaddedNumberString( iImg, 2 ) );
   }
}

}  // anonymous namespace

int main( int argc, char** argv ) {
   dip::testing::Timer timer;

   // Set default input and output paths.
   // Can be overridden by commandline arguments.
   std::string inputPath = DIP__EXAMPLES_DIR;
   std::string outputPath = "";

   // Arguments:
   // 1: input path (optional)
   // 2: output path (optional)
   if( argc >= 2 ) {
      inputPath = argv[1];
   }
   if( argc >= 3 ) {
      outputPath = argv[2];
   }

   // ==================== 2D ==================== //
   dip::Image org2d; // 2D original image
   std::cout << "Reading " << ( inputPath + "/erika" ) << std::endl;
   dip::ImageReadTIFF( org2d, inputPath + "/erika" );
   dip::Image org2dBin = org2d > 100;
   dip::Image ramp2d = dip::CreateRamp( dip::UnsignedArray{ 256, 256 }, 0 );
   dip::Image ramp2dBin = ramp2d > 100;
   std::vector< dip::Image > dilations2d;
   std::vector< dip::Image > erosions2d;
   std::vector< dip::Image > propagations2d;

   timer.Reset();
   dilations2d.push_back( dip::BinaryDilation( org2dBin, -1, 1, "object" ) );
   dilations2d.push_back( dip::BinaryDilation( org2dBin, -1, 5, "object" ) );
   dilations2d.push_back( dip::BinaryDilation( org2dBin, 2, 2, "object" ) );
   dilations2d.push_back( dip::BinaryDilation( org2dBin, 2, 2, "background" ) );
   erosions2d.push_back( dip::BinaryErosion( org2dBin, -1, 1, "object" ) );
   erosions2d.push_back( dip::BinaryErosion( org2dBin, -1, 5, "object" ) );
   erosions2d.push_back( dip::BinaryErosion( org2dBin, 2, 2, "object" ) );
   erosions2d.push_back( dip::BinaryErosion( org2dBin, 2, 2, "background" ) );

   erosions2d.push_back( dip::BinaryErosion( ramp2dBin, -1, 27, "object" ) );
   erosions2d.push_back( dip::BinaryErosion( ramp2dBin, -1, 26, "object" ) );
   timer.Stop();
   std::cout << "Wall time for 4x 2D-dilation and 6x 2D-erosion: " << timer.GetWall() << std::endl;


   dip::Image propSeed( org2d.Sizes(), 1, dip::DT_BIN );
   propSeed = false;
   dip::Image floodMask( org2d.Sizes(), 1, dip::DT_BIN );
   floodMask = true;
   propSeed.At( 0, 0 ) = 1;
   propagations2d.push_back( dip::BinaryPropagation( propSeed, floodMask, -1, 1, "object" ) );
   propagations2d.push_back( dip::BinaryPropagation( propSeed, floodMask, -1, 1, "background" ) );
   propagations2d.push_back( dip::BinaryPropagation( propSeed, floodMask, -1, 10, "background" ) );
   propagations2d.push_back( dip::BinaryPropagation( propSeed, floodMask, 1, 10, "background" ) );
   propagations2d.push_back( dip::BinaryPropagation( propSeed, floodMask, 2, 10, "background" ) );
   propSeed.At( 60, 10 ) = 1;
   propSeed.At( 10, 60 ) = 1;
   propSeed.At( 100, 100 ) = 1;
   propSeed.At( 190, 190 ) = 1;
   propagations2d.push_back( dip::BinaryPropagation( propSeed, org2dBin, -1, 20, "background" ) );
   propSeed = 0;  // Clear seed image; only use outside-as-object
   propagations2d.push_back( dip::BinaryPropagation( propSeed, org2dBin, -1, 100, "object" ) );
   propagations2d.push_back( dip::BinaryPropagation( propSeed, org2dBin, -1, 0, "object" ) );   // Unlimited iterations / until propagation done
   dip::Image noEdgeObjects = dip::EdgeObjectsRemove( org2dBin, 2 );


   // ==================== 3D ==================== //
   dip::Image org3d; // 3D original image
   std::cout << "Reading " << ( inputPath + "/chromo3d" ) << std::endl;
   dip::ImageReadICS( org3d, inputPath + "/chromo3d" );

   dip::Image org3dBin = org3d > 100;
   std::vector< dip::Image > dilations3d;
   std::vector< dip::Image > erosions3d;
   std::vector< dip::Image > propagations3d;
   dip::Image ramp3d = dip::CreateRamp( dip::UnsignedArray{ 64, 64, 64 }, 0 );
   dip::Image ramp3dBin = ramp3d > 28;

   timer.Reset();
   dilations3d.push_back( dip::BinaryDilation( org3dBin, -1, 1, "object" ) );
   dilations3d.push_back( dip::BinaryDilation( org3dBin, -1, 5, "object" ) );
   dilations3d.push_back( dip::BinaryDilation( org3dBin, 1, 2, "object" ) );
   dilations3d.push_back( dip::BinaryDilation( org3dBin, 2, 2, "background" ) );
   erosions3d.push_back( dip::BinaryErosion( org3dBin, -3, 1, "object" ) );
   erosions3d.push_back( dip::BinaryErosion( org3dBin, -1, 5, "object" ) );
   erosions3d.push_back( dip::BinaryErosion( org3dBin, 3, 2, "object" ) );
   erosions3d.push_back( dip::BinaryErosion( org3dBin, 2, 2, "background" ) );

   erosions3d.push_back( dip::BinaryErosion( ramp3dBin, -1, 3, "object" ) );
   erosions3d.push_back( dip::BinaryErosion( ramp3dBin, -1, 2, "object" ) );
   timer.Stop();
   std::cout << "Wall time for 4x 3D-dilation and 6x 3D-erosion: " << timer.GetWall() << std::endl;

   dip::Image prop3dSeed( dip::UnsignedArray{ 64, 64, 64 }, 1, dip::DT_BIN );
   prop3dSeed = false;
   prop3dSeed.At( 0, 0, 0 ) = 1;
   dip::Image flood3dMask( dip::UnsignedArray{ 64, 64, 64 }, 1, dip::DT_BIN );
   flood3dMask = true;

   propagations3d.push_back( dip::BinaryPropagation( prop3dSeed, flood3dMask, -1, 1, "object" ) ); // one pixel along the border, surface of a cube( from the edge condtion )
   propagations3d.push_back( dip::BinaryPropagation( prop3dSeed, flood3dMask, -1, 1, "background" ) ); // 3 pixels left upper corner on slice 0, and 1 pixel in slice 1
   propagations3d.push_back( dip::BinaryPropagation( prop3dSeed, flood3dMask, -1, 10, "background" ) ); // round something upper left corner, 11 slices deep
   propagations3d.push_back( dip::BinaryPropagation( prop3dSeed, flood3dMask, 1, 10, "background" ) ); // triangle in upper corner, 11 slices deep
   propagations3d.push_back( dip::BinaryPropagation( prop3dSeed, flood3dMask, 2, 10, "background" ) ); // square in upper corner in slice 0, then triangle 11 slices deep

   // Write 2D input images
   TestWriteICS( org2d, outputPath + "test_bin2d_org" );
   TestWriteICS( ramp2d, outputPath + "test_bin2d_ramp" );
   // Write 3D input images
   TestWriteICS( org3d, outputPath + "test_bin3d_org" );
   TestWriteICS( ramp3d, outputPath + "test_bin3d_ramp" );

   // Write 2D output images
   TestWriteICS( org2dBin, outputPath + "test_bin2d_org_bin" );
   TestWriteICS( ramp2dBin, outputPath + "test_bin2d_ramp_bin" );
   TestWriteICSs( dilations2d, outputPath + "test_bin2d_dilation" );
   TestWriteICSs( erosions2d, outputPath + "test_bin2d_erosion" );
   TestWriteICSs( propagations2d, outputPath + "test_bin2d_propagation" );
   TestWriteICS( noEdgeObjects, outputPath + "test_bin2d_edgeobjectsremove" );
   // Write 3D output images
   TestWriteICSs( dilations3d, outputPath + "test_bin3d_dilation" );
   TestWriteICSs( erosions3d, outputPath + "test_bin3d_erosion" );
   TestWriteICSs( propagations3d, outputPath + "test_bin3d_propagation" );
   
   // TODO: verify the results programmatically instead of merely outputting image files
   // TODO: Test skeletons

   return 0;
}
