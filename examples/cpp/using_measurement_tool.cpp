/*
 * This example program demonstrates how to use dip::MeasurementTool and work with its output dip::Measurement data.
 * It loads and segments an image, then measures some features. Next it displays the measured features as a table,
 * and generates and displays several versions of the image marked up with feature data.
 */

#include <iostream>
#include "diplib.h"
#include "diplib/simple_file_io.h"  // for dip::ImageRead()
#include "diplib/measurement.h"
#include "diplib/regions.h"         // for dip::Label()
#include "diplib/binary.h"          // for dip::EdgeObjectsRemove()
#include "diplib/morphology.h"      // and dip::Erosion()
#include "diplib/generation.h"      // for dip::DrawText() and dip::CreateXCoordinate()
#include "diplib/display.h"         // for dip::ApplyColorMap() and dip::Overlay()
#include "dipviewer.h"              // for dip::viewer::Show()
#include "diplib/viewer/slice.h"    // for manipulating the dip::viewer::SliceViewer objects

dip::Image ColorMap() {
   // Generate a color map we can use to give the text some color
   dip::Image out = dip::CreateXCoordinate( { 256 }, { "corner" } );
   return dip::ApplyColorMap( out, "linear" );
}

void WriteFeatureData(
      dip::Image& out,
      dip::Measurement::IteratorFeature center,
      dip::Measurement::IteratorFeature feature
) {
   // A color map and how to scale features to index into it
   dip::Image colorMap = ColorMap();
   dip::dfloat offset = dip::Minimum( feature );
   dip::dfloat scale = 255 / ( dip::Maximum( feature ) - offset );
   // A buffer we'll use to convert numbers to strings
   constexpr dip::uint buflen = 8;
   char buf[ buflen ];
   // Physical units to give meaning to the centroid feature
   auto centerValues = center.Values();
   dip::PhysicalQuantityArray centroid = { centerValues[ 0 ].units, centerValues[ 1 ].units };
   // Because the two IteratorFeature objects point at data in the same dip::Measurement object,
   // we can iterate over both and get data for the same object each time.
   auto centerIt = center.FirstObject();
   auto featureIt = feature.FirstObject();
   do {
      DIP_ASSERT( centerIt.ObjectID() == featureIt.ObjectID() ); // This is not necessary, as explained above.
      // Find the centroid of the object, which is in micrometer. We need to find the corresponding value in pixels.
      centroid[ 0 ].magnitude = centerIt[ 0 ];
      centroid[ 1 ].magnitude = centerIt[ 1 ];
      auto origin = out.PhysicalToPixels( centroid );
      // Get the value to draw
      dip::dfloat value = *featureIt;
      auto color = colorMap.At( static_cast< dip::uint >( std::round(( value - offset ) * scale )));
      std::snprintf( buf, buflen, "%.1f", value );
      dip::DrawText( out, buf, origin, color, 0, "center" );
   } while( ++centerIt, ++featureIt );
}

int main() {

   // The input grayscale image.
   dip::Image input = dip::ImageRead( DIP_EXAMPLES_DIR "/cermet.ics" );

   // Normally images coming off a microscope have metadata indicating the pixel size, dip::ImageRead
   // will extract that information and add it to the dip::Image object.
   // Our test image does not have such metadata. In fact, I have no idea what the pixel size in this image is.
   // So we add some random pixel size to the image here. The measurement tool will use this pixel size
   // to derive real-world measurements, rather than measurements in pixel units.
   input.SetPixelSize( 0.32 * dip::Units::Micrometer() );

   // Threshold and label the image.
   // Note that removing edge objects like this leads to biased statistics about the objects in the image;
   // for serious work, use a counting frame or similar unbiased sampling of objects.
   dip::Image binary = input < 120;
   dip::EdgeObjectsRemove( binary, binary, 2 );
   dip::Image labels = dip::Label( binary, 2 );

   // Measure some features.
   dip::MeasurementTool measurementTool;
   dip::Measurement msr = measurementTool.Measure( labels, input, { "Size", "Center", "EllipseVariance", "StandardDeviation" } );

   // Display our measurements as a table to the console.
   std::cout << msr;

   // Display the "EllipseVariance" feature by painting each object with the feature as the grayscale,
   // and displaying this image using a color map.
   dip::Image ellipseVariance = dip::ObjectToMeasurement( labels, msr[ "EllipseVariance" ] );
   auto win1 = dip::viewer::Show( ellipseVariance, "The EllipseVariance feature" );
   win1->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Sequential;

   // Write the "StandardDeviation" value on the original gray-scale image, and display.
   dip::Image outline = binary - dip::Erosion( binary, { 3, "diamond" } );
   dip::Image stdev = dip::Overlay( input, outline, { 0, 210, 0 } );
   WriteFeatureData( stdev, msr[ "Center" ], msr[ "StandardDeviation" ] );
   auto win2 = dip::viewer::Show( stdev, "The StandardDeviation feature" );
   win2->link( *win1 );

   // Allow interaction with the windows and wait until they are closed.
   dip::viewer::Spin();
}
