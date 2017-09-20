#undef DIP__ENABLE_DOCTEST

#include <iostream>

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

#include <dipviewer.h>

int main(int argc, char **argv) {
   if (argc != 2)
   {
     std::cout << "Missing filename" << std::endl;
     return 1;
   }

   dip::Image image = dip::ImageReadICS( argv[1] );
   
   dip::viewer::Show( image );
   dip::viewer::Spin( );

   return 0;
}
