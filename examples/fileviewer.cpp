#include <iostream>

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

#include <dipviewer.h>

int main(int argc, char **argv) {
   if (argc < 2)
   {
      std::cerr << "Usage: fileviewer <image> [image ...]" << std::endl;
      return 1;
   }
   else
   {
      for (size_t ii=1; (int)ii < argc; ++ii)
      {
         std::string arg(argv[ii]);
      
         if ( dip::FileCompareExtension(arg, "ics") )
         {
            dip::viewer::Show( dip::ImageReadICS( arg ), arg );
         }
         else if ( dip::FileCompareExtension(arg, "tiff") || dip::FileCompareExtension(arg, "tif") )
         {
            dip::viewer::Show( dip::ImageReadTIFF( arg ), arg );
         }
         else
         {
            std::cerr << "Unrecognized image extension " << dip::FileGetExtension(arg) << std::endl;
            return -1;
         }
      }
   }

   dip::viewer::Spin( );

   return 0;
}
