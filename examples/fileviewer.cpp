#undef DIP__ENABLE_DOCTEST

#include <iostream>

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

#include <dipviewer.h>

int main(int argc, char **argv) {
   if (argc == 1)
   {
      dip::viewer::Show( dip::ImageReadICS( DIP__EXAMPLES_DIR "/chromo3d.ics" ), "chromo3d" );
   }
   else
   {
      for (size_t ii=2; (int)ii < argc; ++ii)
      {
         dip::viewer::Show( dip::ImageReadICS( argv[ii] ), argv[ii] );
      }
   }

   dip::viewer::Spin( );

   return 0;
}
