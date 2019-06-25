/*
 * A simple tool that shows an image file using dip::viewer::Show
 */

#include <iostream>

#include <diplib.h>
#include <diplib/simple_file_io.h>

#include <dipviewer.h>
#include <diplib/viewer/slice.h>

int main( int argc, char** argv ) {
   if( argc < 2 ) {
      std::cerr << "Usage: fileviewer [-b] <image> [<image> ...]\n";
      std::cerr << "   The -b option forces the use of Bio-Formats for all file types.\n";
      return 1;
   }

   int ii = 1;
   dip::String format;
   if( std::strcmp( argv[ ii ], "-b" ) == 0 ) {
      ++ii;
      format = "bioformats";
   }

   for( ; ii < argc; ++ii ) {
      std::string arg( argv[ ii ] );
      dip::Image img;
      dip::FileInformation info = dip::ImageRead( img, arg, format );
      std::cout << info.name << ":\n";
      std::cout << "   - fileType:        " << info.fileType        << '\n';
      std::cout << "   - dataType:        " << info.dataType        << '\n';
      std::cout << "   - significantBits: " << info.significantBits << '\n';
      std::cout << "   - sizes:           " << info.sizes           << '\n';
      std::cout << "   - tensorElements:  " << info.tensorElements  << '\n';
      std::cout << "   - colorSpace:      " << info.colorSpace      << '\n';
      std::cout << "   - pixelSize:       " << info.pixelSize       << '\n';
      std::cout << "   - origin:          " << info.origin          << '\n';
      std::cout << "   - numberOfImages:  " << info.numberOfImages  << '\n';
      if( !info.history.empty() ) {
         std::cout << "   - history:\n";
         for( auto& line : info.history ) {
            std::cout << "        " << line << '\n';
         }
      }

      auto wdw = dip::viewer::Show( img, arg );
      dip::viewer::SliceViewer::Guard guard( *wdw );
      wdw->options().offset_ = info.origin;
   }

   dip::viewer::Spin();

   return 0;
}
