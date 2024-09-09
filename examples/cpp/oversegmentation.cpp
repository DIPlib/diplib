/*
 * This program shows how to use superpixel segmentation and graph manipulation.
 * It displays the result using dip::viewer::Show, and links the displayed windows.
 */

#include "diplib.h"
#include "diplib/simple_file_io.h"
#include "diplib/segmentation.h"
#include "diplib/regions.h"
#include "diplib/measurement.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h" // To manipulate the dip::viewer::SliceView objects

int main() {
   // Read image
   dip::Image input = dip::ImageRead( DIP_EXAMPLES_DIR "/orka.tif" );

   // Create superpixels
   dip::Image superpixels = dip::Superpixels( input, 0.01, 1.0, "CW", { "no gaps" } );

   // Convert to graph
   dip::MeasurementTool measurementTool;
   auto msr = measurementTool.Measure( superpixels, input, { "Mean" } );
   dip::Graph graph = dip::RegionAdjacencyGraph( superpixels, msr["Mean"], "touching" );

   // Simplify graph
   graph = graph.MinimumSpanningForest( { 1 } );
   graph.RemoveLargestEdges( 50 - 1 ); // Find 50 regions

   // Convert back to a labeled image
   dip::Image output = dip::Relabel( superpixels, graph );

   // Display
   auto win1 = dip::viewer::Show( input, "input" );
   auto win2 = dip::viewer::Show( superpixels, "superpixels" );
   auto win3 = dip::viewer::Show( output, "output" );
   win3->link( *win1 );
   win3->link( *win2 );
   win2->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   win3->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   dip::viewer::Spin();

   // Paint regions with their means
   superpixels = dip::ObjectToMeasurement( superpixels, msr["Mean"] );
   msr = measurementTool.Measure( output, input, { "Mean" } );
   output = dip::ObjectToMeasurement( output, msr["Mean"] );

   // Display
   win1 = dip::viewer::Show( input, "input" );
   win2 = dip::viewer::Show( superpixels, "superpixels" );
   win3 = dip::viewer::Show( output, "output" );
   win3->link( *win1 );
   win3->link( *win2 );
   dip::viewer::Spin();
}
