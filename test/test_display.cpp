#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/regions.h"
#include "diplib/display.h"
#include "dipviewer.h"

int main() {
   try {

      auto grey = dip::ImageReadICS( "../test/cermet.ics" );
      dip::Image coloredGrey( grey.Sizes(), 3, dip::DT_UINT8 );
      coloredGrey.Copy( grey.QuickCopy().ExpandSingletonTensor( 3 ) );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredGrey )); // 0

      auto bin = grey < 120;
      dip::Image coloredBin( bin.Sizes(), 3, dip::DT_UINT8 );
      coloredBin.Fill( 0 );
      coloredBin.At( bin )[ 0 ] = 255;
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredBin )); // 1

      auto label = dip::Label( bin );
      dip::Image coloredLabel( label.Sizes(), 3, dip::DT_UINT8 );
      coloredLabel.Copy( label.QuickCopy().ExpandSingletonTensor( 3 ) );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredLabel )); // 2

      auto color1 = dip::ApplyColorMap( label, "label" );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color1 )); // 3

      auto color2 = dip::Overlay( grey, bin );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color2 )); // 4

      auto color3 = dip::Overlay( color2, label > 30, { 0, 0, 255 } );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color3 )); // 5

      auto color4 = dip::Overlay( grey, label );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color4 )); // 6

      dip::viewer::Spin();

   } catch( dip::Error const& e ) {
      std::cout << "Caught DIPlib exception:\n " << e.what() << std::endl;
      return -1;
   }

   return 0;
}
