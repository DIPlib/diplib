/*
 * This program shows how to quantize colors in an RGB image.
 * It displays the result using dip::viewer::ShowSimple.
 */

#include <iostream>

#include "diplib.h"
#include "dipviewer.h"
#include "diplib/display.h"
#include "diplib/histogram.h"
#include "diplib/lookup_table.h"
#include "diplib/segmentation.h"
#include "diplib/simple_file_io.h"


int main() {
   dip::Image input = dip::ImageRead( DIP_EXAMPLES_DIR "/DIP.tif" );
   dip::viewer::ShowSimple( input, "input image" );

   constexpr dip::uint nClusters = 3;

   // Compute the color histogram.
   dip::Histogram hist( input, {}, { dip::Histogram::Configuration( 0.0, 255.0, 64 ) } );

   // Cluster the histogram, the output histogram has a label assigned to each bin.
   // Each label corresponds to one of the clusters.
   dip::FloatCoordinateArray centers = dip::MinimumVariancePartitioning( hist, hist, nClusters );

   // Find the cluster label for each pixel in the input image.
   dip::Image labels = hist.ReverseLookup( input );
   dip::viewer::ShowSimple( dip::ApplyColorMap( labels, "label" ), "clusters" );

   std::cout << nClusters << " clusters requested, " << centers.size() << " clusters found:\n";
   for( dip::uint ii = 0; ii < centers.size(); ++ii ) {
      std::cout << "   cluster " << ii << ": " << centers[ ii ] << "\n";
   }

   // Create a lookup table with the colors and apply it to create an image with reduced number of colors.
   // `centers[ii]` corresponds to label `ii+1`.
   dip::Image lutImage( { centers.size() + 1 }, 3, dip::DT_UINT8 );
   lutImage.At( 0 ) = 0; // label 0 doesn't exist
   for( dip::uint ii = 0; ii < centers.size(); ++ii ) {
      lutImage.At( ii + 1 ) = { centers[ ii ][ 0 ], centers[ ii ][ 1 ], centers[ ii ][ 2 ] };
   }
   dip::LookupTable lut( lutImage );
   dip::Image output = lut.Apply( labels );
   output.SetColorSpace( "sRGB" );
   dip::viewer::ShowSimple( output, "quantized colors" );

   // Draw windows on screen and wait for them to be closed.
   dip::viewer::Spin();
}
