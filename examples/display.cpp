/*
 * dip::viewer::ShowSimple() shows a 3-channel, UINT8 image as an RGB image on the screen.
 * This example program shows different ways to create such an image.
 *
 * It also shows some simple image manipulations: thresholding, labelling, and indexing using a mask image.
 */

#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/regions.h"
#include "diplib/display.h"
#include "dipviewer.h"

dip::Image GreyToRGB( dip::Image const& in ) { // Assume `in` is a grey-value (scalar) image
   // Create a new image so it has the right data type for ShowSimple
   dip::Image out( in.Sizes(), 3, dip::DT_UINT8 );
   // Copy the data over, repeating the single input sample for each of the three channels.
   // Note that this can be done without initializing out, but then the input data type is used for `out`.
   out.Copy( in.QuickCopy().ExpandSingletonTensor( 3 ));
   return out;
}

int main() {
   try {

      // Read a 2D grey-value image
      auto grey = dip::ImageReadICS( DIP__EXAMPLES_DIR "/cermet.ics" );
      // Convert to RGB, and show it
      dip::Image coloredGrey = GreyToRGB( grey );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredGrey ));

      // Threshold
      auto bin = grey < 120;
      // Create an RGB image, paint the pixels selected by `bin` in red; show it
      dip::Image coloredBin( bin.Sizes(), 3, dip::DT_UINT8 );
      coloredBin.Fill( 0 );
      coloredBin.At( bin )[ 0 ] = 255;
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredBin ));

      // Label the image, convert to RGB, and show it
      auto label = dip::Label( bin );
      dip::Image coloredLabel = GreyToRGB( label );
      DIP_STACK_TRACE_THIS( dip::viewer::ShowSimple( coloredLabel ));

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
