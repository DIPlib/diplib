/*
 * This program shows how to quantize colors in an RGB image.
 * It displays the result using dip::viewer::ShowSimple.
 */

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
   constexpr dip::uint subsample = 4;

   // Compute the color histogram.
   dip::Histogram hist( input, {}, { dip::Histogram::Configuration( 0.0, 255.0, 256 / subsample ) } );

   // Cluster the histogram, the output histogram has a label assigned to each bin.
   // Each label corresponds to one of the clusters.
   dip::Image histImage = hist.GetImage(); // Copy with shared data
   dip::Image tmp;
   dip::CoordinateArray centers = dip::MinimumVariancePartitioning( histImage, tmp, nClusters );
   std::cout << nClusters << " clusters requested, " << centers.size() << " clusters found.\n";
   histImage.Copy( tmp ); // Copy 32-bit label image into 64-bit histogram image.

   // Find the cluster label for each pixel in the input image.
   dip::Image labels = hist.ReverseLookup( input );
   dip::viewer::ShowSimple( dip::ApplyColorMap( labels, "label" ), "clusters" );

   // The `centers` array contains histogram coordinates for each of the centers.
   // We need to convert these coordinates to RGB values by multiplying by 8 (=256/32).
   // `centers[ii]` corresponds to label `ii+1`.
   dip::Image lutImage( { centers.size() + 1 }, 3, dip::DT_UINT8 );
   lutImage.At( 0 ) = 0; // label 0 doesn't exist
   for( dip::uint ii = 0; ii < centers.size(); ++ii ) {
      lutImage.At( ii + 1 ) = { centers[ ii ][ 0 ] * subsample, centers[ ii ][ 1 ] * subsample, centers[ ii ][ 2 ] * subsample };
   }

   // Finally, we apply our look-up table mapping, painting each label in the
   // image with its corresponding RGB color.
   dip::LookupTable lut( lutImage );
   dip::Image output = lut.Apply( labels );
   output.SetColorSpace( "sRGB" );
   dip::viewer::ShowSimple( output, "quantized colors" );

   // Draw windows on screen and wait for them to be closed.
   dip::viewer::Spin();
}
