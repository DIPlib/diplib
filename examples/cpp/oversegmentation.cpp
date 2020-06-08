/*
 * This program shows how to use superpixel segmentation and graph manipulation.
 * It displays the result using dip::viewer::Show.
 */

#include "diplib.h"
#include "dipviewer.h"
#include "diplib/file_io.h"
#include "diplib/segmentation.h"
#include "diplib/regions.h"
#include "diplib/measurement.h"

int main() {
   // Read image
   dip::Image input = dip::ImageReadICS( DIP_EXAMPLES_DIR "/trui.ics" );

   // Create superpixels
   dip::Image superpixels = dip::Superpixels( input, 0.01, 1.0, "CW", { "no gaps" } );

   // Convert to graph
   dip::MeasurementTool measurementTool;
   auto msr = measurementTool.Measure( superpixels, input, { "Mean" } );
   dip::Graph graph = dip::RegionAdjacencyGraph( superpixels, msr["Mean"], "touching" );

   // Simplify graph
   graph = graph.MinimumSpanningForest( { 1 } );
   graph.RemoveLargestEdges( 80 - 1 ); // Find 80 regions

   // Convert back to a labeled image
   dip::Image output = dip::Relabel( superpixels, graph );

   // Paint regions with their means
   superpixels = dip::ObjectToMeasurement( superpixels, msr["Mean"] );
   msr = measurementTool.Measure( output, input, { "Mean" } );
   output = dip::ObjectToMeasurement( output, msr["Mean"] );

   // Display
   dip::viewer::Show( input, "input" );
   dip::viewer::Show( superpixels, "superpixels" );
   dip::viewer::Show( output, "output" );
   dip::viewer::Spin();
}
