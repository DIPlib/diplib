/*
 * This program demonstrates some of the low-level functionality of DIPviewer:
 *  - Creating and using a dip::viewer::Manager window manager.
 *  - Creating multiple dip::viewer::SliceViewer and dip::viewer::ImageViewer windows.
 *  - Updating the image shown in a dip::viewer::ImageViewer window in a background thread.
 */

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>
#include <diplib/analysis.h>
#include <diplib/linear.h>
#include <diplib/math.h>

#ifdef DIP_CONFIG_HAS_GLFW
#  include <diplib/viewer/glfw.h>
   dip::viewer::GLFWManager manager;
#else
#  include <diplib/viewer/glut.h>
   dip::viewer::GLUTManager manager;
#endif
#include <diplib/viewer/image.h>
#include <diplib/viewer/slice.h>

void run( dip::viewer::ImageViewer::Ptr const& viewer ) {
   double sigma = 1., fact = 1.1;
   dip::Image original;

   // Get original image from viewer.
   {
      dip::viewer::Viewer::Guard guard( *viewer );
      original = viewer->image();
   }

   while( !viewer->destroyed() ) {
      // Do some filtering.
      dip::Image image = dip::Gauss( original, { sigma, sigma } );
      sigma *= fact;
      if( sigma > 20. || sigma < 1. ) {
         fact = 1. / fact;
      }
      image.Convert( dip::DT_UINT8 );

      // Update displayed image.
      {
         dip::viewer::Viewer::Guard guard( *viewer );
         viewer->setImage( image );
      }

      std::this_thread::sleep_for( std::chrono::microseconds( 10000 ));
   }
}

int main() {

   // Read and display primary image
   dip::Image image3 = dip::ImageReadICS( DIP_EXAMPLES_DIR "/chromo3d.ics" );
   image3.PixelSize().Set( 2, 5 );
   manager.createWindow( dip::viewer::SliceViewer::Create( image3, "chromo3d", 500, 400 ));

   // Calculate and display structure tensor
   dip::Image st = dip::StructureTensor( image3 );
   manager.createWindow( dip::viewer::SliceViewer::Create( st, "chromo3d structure tensor", 500, 400 ));

   // Generate 2D RGB image
   dip::Image image2 = dip::Image{ dip::UnsignedArray{ 50, 40 }, 3, dip::DT_UINT8 };
   dip::Image tmp = image2[ 0 ];
   dip::FillXCoordinate( tmp, { "corner" } );
   tmp = image2[ 1 ];
   dip::FillYCoordinate( tmp, { "corner" } );
   tmp = image2[ 2 ];
   dip::FillRadiusCoordinate( tmp );
   image2 *= 5;

   // Display
   auto iv = dip::viewer::ImageViewer::Create( image2 );
   manager.createWindow( iv );

   // Create thread to programmatically alter image
   std::thread thread = std::thread( run, iv );

   // Generate 0D RGB image
   dip::Image image0 = dip::Image{ dip::UnsignedArray{}, 3, dip::DT_UINT8 };
   image0[ 0 ] = 64;
   image0[ 1 ] = 128;
   image0[ 2 ] = 192;
   manager.createWindow( dip::viewer::SliceViewer::Create( image0, "0d", 500, 400 ));

   // Generate 1D RGB image
   dip::Image image1 = dip::Image{ dip::UnsignedArray{ 160 }, 1, dip::DT_UINT8 };
   dip::FillXCoordinate( image1, { "corner" } );
   image1 = dip::Sin( image1 / 10 );
   manager.createWindow( dip::viewer::SliceViewer::Create( image1, "1d", 500, 400 ));

   while( manager.activeWindows() > 0 ) {
      // Only necessary for GLFW
      manager.processEvents();
      std::this_thread::sleep_for( std::chrono::microseconds( 1000 ));
   }

   // Clean up
   thread.join();

   return 0;
}
