/*
 * This program shows how to delete small objects from a grey-value image.
 * It displays the result using dip::viewer::ShowSimple.
 */

#include "diplib.h"
#include "dipviewer.h"
#include "diplib/file_io.h"
#include "diplib/regions.h"
#include "diplib/measurement.h"

int main() {
   // Create a test image
   dip::Image input = -dip::ImageReadICS( DIP_EXAMPLES_DIR "/cermet.ics" );
   input.At( input < 120 ) = 0;
   // Display
   dip::viewer::ShowSimple( input, "input image" );
   // Threshold and label
   dip::Image label = dip::Label( input > 0 );
   // Obtain sum of pixels per label
   dip::MeasurementTool measurementTool;
   auto msr = measurementTool.Measure( label, input, { "Mass" } );
   // Paint each label with the measured value
   dip::Image feature = dip::ObjectToMeasurement( label, msr[ "Mass" ] );
   // Create output as a copy of the input, with low feature values set to 0
   dip::Image output1 = input.Copy();
   output1.At( feature < 100000 ) = 0;
   // Display
   dip::viewer::ShowSimple( output1, "output image" );
   dip::viewer::Spin();
}
