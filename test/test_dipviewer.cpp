#undef DIP__ENABLE_DOCTEST

#include <diplib.h>
#include <diplib/file_io.h>
#include <diplib/generation.h>

#include <dipviewer.h>

int main() {
   dip::Image image3 = dip::ImageReadICS( "../test/chromo3d.ics" );
   image3.PixelSize().Set( 2, 5 );
   
   dip::viewer::Show( image3 );
   dip::viewer::Spin( );

   return 0;
}
