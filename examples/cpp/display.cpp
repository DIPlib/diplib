/*
 * dip::viewer::ShowSimple() shows a UINT8 image with 1 or 3 channels as an RGB image on the screen.
 * This example program shows different ways to create such an image.
 *
 * It also shows some simple image manipulations: thresholding, labeling, and indexing using a mask image.
 */

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/regions.h"
#include "diplib/display.h"
#include "dipviewer.h"

int main() {
   try {

      // Read a 2D grey-value image
      auto grey = dip::ImageReadICS( DIP_EXAMPLES_DIR "/cermet.ics" );
      // Show it
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( grey ));

      // Threshold
      auto bin = grey < 120;
      // Create an RGB image, paint the pixels selected by `bin` in red; show it
      dip::Image coloredBin( bin.Sizes(), 3, dip::DT_UINT8 );
      coloredBin.Fill( 0 );
      coloredBin.At( bin )[ 0 ] = 255; // assigning 255 to the first channel, the other channels remain 0
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredBin ));

      // Label the image, and show it
      auto label = dip::Label( bin );
      dip::Image labelUInt8 = Convert( label, dip::DT_UINT8 );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( labelUInt8 ));

      // Apply the "label" color map, and show it again
      auto color1 = dip::ApplyColorMap( label, "label" );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color1 ));

      // Overlay the binary image over the original grey-value image, and show it (objects are red)
      auto color2 = dip::Overlay( grey, bin );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color2 ));

      // Overlay objects 31 and higher in blue, and show it
      auto color3 = dip::Overlay( color2, label > 30, { 0, 0, 255 } );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color3 ));

      // Overlay the labeled image over the original grey-value image, and show it
      auto color4 = dip::Overlay( grey, label );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( color4 ));

      // This is required to release resources
      dip::viewer::Spin();

   } catch( dip::Error const& e ) {
      std::cout << "Caught DIPlib exception:\n " << e.what() << std::endl;
      return -1;
   }

   return 0;
}
