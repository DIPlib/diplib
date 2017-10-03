#undef DIP__ENABLE_DOCTEST

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>
#include <diplib/analysis.h>

#ifdef DIP__HAS_GLFW
#include <diplib/viewer/glfw.h>
dip::viewer::GLFWManager manager;
#else
#include <diplib/viewer/glut.h>
dip::viewer::GLUTManager manager;
#endif
#include <diplib/viewer/image.h>
#include <diplib/viewer/slice.h>

int main() {

   dip::Image image3 = dip::ImageReadICS( DIP__EXAMPLES_DIR "/chromo3d.ics" );
   image3.PixelSize().Set( 2, 5 );
   dip::Image st = dip::StructureTensor( image3 );
   
   manager.createWindow( dip::viewer::WindowPtr( new dip::viewer::SliceViewer( image3, "chromo3d", 500, 400 )));
   manager.createWindow( dip::viewer::WindowPtr( new dip::viewer::SliceViewer( st, "chromo3d structure tensor", 500, 400 )));

   dip::Image image2{ dip::UnsignedArray{ 50, 40 }, 3, dip::DT_UINT8 };
   dip::Image tmp = image2[ 0 ];
   dip::FillXCoordinate( tmp, { "corner" } );
   tmp = image2[ 1 ];
   dip::FillYCoordinate( tmp, { "corner" } );
   tmp = image2[ 2 ];
   dip::FillRadiusCoordinate( tmp );
   image2 *= 5;
   manager.createWindow( dip::viewer::WindowPtr( new dip::viewer::ImageViewer( image2 )));

   while( manager.activeWindows() ) {
      // Only necessary for GLFW
      manager.processEvents();
      std::this_thread::sleep_for( std::chrono::microseconds( 1000 ));
   }

   return 0;
}
