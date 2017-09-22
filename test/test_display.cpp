#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/regions.h"
#include "diplib/display.h"
#include "dipviewer.h"

int main() {
   auto grey = dip::ImageReadICS( "../test/cermet.ics" );
   dip::viewer::Show( grey ); // 0

   auto bin = grey < 120;
   dip::viewer::Show( bin ); // 1

   auto label = dip::Label( bin );
   dip::viewer::Show( label ); // 2

   auto color1 = dip::ApplyColorMap( label, "label" );
   dip::viewer::Show( color1 ); // 3

   auto color2 = dip::Overlay( grey, bin );
   dip::viewer::Show( color2 ); // 4

   auto color3 = dip::Overlay( color2, label > 30, { 0, 0, 255 } );
   dip::viewer::Show( color3 ); // 5

   auto color4 = dip::Overlay( grey, label );
   dip::viewer::Show( color4 ); // 6

   dip::viewer::Spin( );
   return 0;
}
