/*
 * A simple tool that shows an image file using dip::viewer::Show
 */

#include <iostream>

#include <diplib.h>
#include <diplib/file_io.h>
#ifdef DIP__HAS_JAVAIO
  #include <diplib/javaio.h>
#endif

#include <dipviewer.h>
#include <diplib/viewer/slice.h>

int main( int argc, char** argv ) {
   if( argc < 2 ) {
      std::cerr << "Usage: fileviewer <image> [image ...]" << std::endl;
      return 1;
   } else {
      for( size_t ii = 1; ( int )ii < argc; ++ii ) {
         std::string arg( argv[ ii ] );
         dip::Image img;
         dip::FileInformation info;
         if( dip::FileCompareExtension( arg, "ics" )) {
            info = dip::ImageReadICS( img, arg );
         } else if( dip::FileCompareExtension( arg, "tiff" ) || dip::FileCompareExtension( arg, "tif" )) {
            info = dip::ImageReadTIFF( img, arg );
         } else {
#ifdef DIP__HAS_JAVAIO
            info = dip::ImageReadJavaIO( img, arg );
#else
            std::cerr << "Unrecognized image extension " << dip::FileGetExtension( arg ) << std::endl;
            return -1;
#endif
         }
         std::cout << arg << ": " << img;
         auto wdw = dip::viewer::Show( img, arg );

         dip::viewer::SliceViewer::Guard guard(*wdw);
         wdw->options().offset_ = info.origin;
      }
   }

   dip::viewer::Spin();

   return 0;
}
