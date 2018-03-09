/*
 * A simple tool that shows an image file using dip::viewer::Show
 */

#include <iostream>

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

#include <dipviewer.h>

int main( int argc, char** argv ) {
   if( argc < 2 ) {
      std::cerr << "Usage: fileviewer <image> [image ...]" << std::endl;
      return 1;
   } else {
      for( size_t ii = 1; ( int )ii < argc; ++ii ) {
         std::string arg( argv[ ii ] );
         dip::Image img;
         if( dip::FileCompareExtension( arg, "ics" )) {
            img = dip::ImageReadICS( arg );
         } else if( dip::FileCompareExtension( arg, "tiff" ) || dip::FileCompareExtension( arg, "tif" )) {
            img = dip::ImageReadTIFF( arg );
         } else {
            std::cerr << "Unrecognized image extension " << dip::FileGetExtension( arg ) << std::endl;
            return -1;
         }
         std::cout << arg << ": " << img;
         dip::viewer::Show( img, arg );
      }
   }

   dip::viewer::Spin();

   return 0;
}
