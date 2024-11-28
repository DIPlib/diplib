/*
 * This program shows how to use superpixel segmentation and graph manipulation.
 * It displays the result using dip::viewer::Show, and links the displayed windows.
 */

#include <cmath>

#include "diplib.h"
#include "diplib/graph.h"
#include "diplib/measurement.h"
#include "diplib/regions.h"
#include "diplib/segmentation.h"
#include "diplib/simple_file_io.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h" // To manipulate the dip::viewer::SliceView objects
#include "diplib/viewer/viewer.h" // To manipulate the dip::viewer::SliceView objects


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
   dip::Graph segmented_graph = dip::MinimumSpanningForest( graph, { 1 } );
   segmented_graph.RemoveLargestEdges( 50 - 1 ); // Find 50 regions

   // Convert back to a labeled image
   dip::Image output = dip::Relabel( superpixels, segmented_graph );

   // Display
   auto win1 = dip::viewer::Show( input, "input" );
   auto win2 = dip::viewer::Show( superpixels, "superpixels" );
   auto win3 = dip::viewer::Show( output, "simplified regions" );
   win3->link( *win1 );
   win3->link( *win2 );
   win2->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   win3->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   dip::viewer::Spin();

   // Paint regions with their means
   dip::Image painted = dip::ObjectToMeasurement( superpixels, msr["Mean"] );
   msr = measurementTool.Measure( output, input, { "Mean" } );
   output = dip::ObjectToMeasurement( output, msr["Mean"] );

   // Display
   win1 = dip::viewer::Show( input, "input" );
   win2 = dip::viewer::Show( painted, "superpixels painted with their mean" );
   win3 = dip::viewer::Show( output, "simplified regions, painted with their mean" );
   win3->link( *win1 );
   win3->link( *win2 );
   dip::viewer::Spin();

   // Apply a graph cut based on some marker pixels
   // Compute weights using a Gaussian function
   dip::dfloat sigma = 10;
   graph.UpdateEdgeWeights( [ = ]( dip::dfloat val1, dip::dfloat val2 ) { return std::exp((( val1 - val2 ) * ( val1 - val2 )) / ( -2 * sigma * sigma )); } );
   // Foreground points:
   dip::uint foreground = superpixels.At( 139, 97 ).As< dip::uint >();
   graph.AddEdge( foreground, superpixels.At( 212, 61 ).As< dip::uint >(), 1e12 ); // Adding edges between the foreground node and other marker pixels, with a very large capacity
   graph.AddEdge( foreground, superpixels.At( 214, 76 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( foreground, superpixels.At( 199, 140 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( foreground, superpixels.At( 171, 124 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( foreground, superpixels.At( 114, 149 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( foreground, superpixels.At( 60, 182 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( foreground, superpixels.At( 186, 112 ).As< dip::uint >(), 1e12 );
   // Background pixels:
   dip::uint background = superpixels.At( 100, 65 ).As< dip::uint >();
   graph.AddEdge( background, superpixels.At( 84, 204 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 105, 176 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 229, 117 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 184, 38 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 71, 116 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 34, 139 ).As< dip::uint >(), 1e12 );
   graph.AddEdge( background, superpixels.At( 32, 177 ).As< dip::uint >(), 1e12 );

   // Copy graph to directed graph and apply graph cut algorithm
   dip::DirectedGraph dgraph( graph );
   dip::GraphCut( dgraph, background, foreground );

   // Convert back to a labeled image
   output = dip::Relabel( superpixels, dgraph );

   // Display
   win1 = dip::viewer::Show( input, "input" );
   win2 = dip::viewer::Show( superpixels, "superpixels" );
   win3 = dip::viewer::Show( output, "graph cut of superpixels" );
   win3->link( *win1 );
   win3->link( *win2 );
   win2->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   win3->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   dip::viewer::Spin();
}
