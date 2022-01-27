/*
 * This example program demonstrates how to blend two images together.
 */

#include <iostream>
#include "diplib.h"
#include "diplib/simple_file_io.h"  // for dip::ImageRead()
#include "diplib/generation.h"
#include "dipviewer.h"              // for dip::viewer::Show()

int main() {

   // Load some images to play with
   dip::Image image1 = dip::ImageRead( DIP_EXAMPLES_DIR "/DIP.tif" );
   dip::Image image2 = dip::ImageRead( DIP_EXAMPLES_DIR "/trui.ics" );
   dip::viewer::Show( image1, "image1" );
   dip::viewer::Show( image2, "image2" );

   // Generate a mask image with a smooth transition from foreground to background
   dip::Image mask = image2.Similar( dip::DT_SFLOAT );
   mask.Fill( 0 );
   DIP_STACK_TRACE_THIS( dip::DrawBandlimitedBall( mask, 110, { 126, 91 }, { 1 }, "filled", 10 ));
   dip::viewer::Show( mask, "mask" );

   // Blend image2 into image1 using mask
   dip::Image out1 = image1.Copy();
   DIP_STACK_TRACE_THIS( dip::BlendBandlimitedMask( out1, mask, image2, { 195 - 126, 195 - 91 } ));
   dip::viewer::Show( out1, "out1" );

   // Blend image2 into image1 at 30%
   dip::Image out2 = image1.Copy();
   DIP_STACK_TRACE_THIS( dip::BlendBandlimitedMask( out2, dip::Image{ 0.3 }, image2 ));
   dip::viewer::Show( out2, "out2" );

   // Blend mask into image1 in red
   dip::Image out3 = image1.Copy();
   DIP_STACK_TRACE_THIS( dip::BlendBandlimitedMask( out3, mask, dip::Image{{ 255, 0, 0 }}, { 195 - 126, 195 - 91 } ));
   dip::viewer::Show( out3, "out3" );

   // Allow interaction with the windows and wait until they are closed.
   dip::viewer::Spin();
}
