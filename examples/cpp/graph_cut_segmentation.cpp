/*
* An example program that demonstrates using the graph cut segmentation algorithm,
* it displays some images using dip::viewer::Show, and links those displays together.
 */

//#include <cmath>

#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/random.h"
#include "diplib/segmentation.h"
#include "dipviewer.h"
#include "diplib/viewer/slice.h" // To manipulate the dip::viewer::SliceView objects
#include "diplib/viewer/viewer.h" // To manipulate the dip::viewer::SliceView objects


void ComputeAndDisplay( dip::Image const& image, dip::Image const& marker, dip::dfloat lambda, dip::dfloat gamma, dip::viewer::SliceViewer::Ptr& handle ) {
   dip::Image res0 = dip::GraphCut( image, marker, 30.0, lambda, gamma );
   auto h = dip::viewer::Show( res0, "simplified regions, lambda = " + std::to_string( lambda ) + ", gamma = " + std::to_string( gamma ));
   h->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   h->link( *handle );
}

int main() {
   // Create test image
   dip::Image image( { 256, 256 }, 1, dip::DT_UINT8 );
   image.Fill( 0 );
   dip::Image pattern = dip::Sin( dip::CreateXCoordinate( { 128, 128 } ) * ( 2.0 * dip::pi / ( 128.0 / 5.0 ) )) > 0;
   dip::Image patternY = pattern;
   patternY.Rotation90();
   pattern &= patternY;
   image.At( dip::Range( 64, 191 ), dip::Range( 64, 191 )) = pattern * 200;
   dip::Random rng{};
   dip::UniformNoise( image, image, rng, 0, 25 );

   auto handle = dip::viewer::Show( image, "image" );

   // Create initial markers
   dip::Image marker = image.Similar();
   marker.Fill( 0 );
   marker.At( dip::Range( 120, 150 ), dip::Range( 120, 150 ) ) = 1;
   marker.At( dip::Range( 0, 30 ), dip::Range( 0, 30 ) ) = 2;

   auto h = dip::viewer::Show( marker, "marker" );
   h->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Label;
   h->link( *handle );

   // Graph cuts with different values for lambda
   ComputeAndDisplay( image, marker, 0.0, 0.0, handle );
   ComputeAndDisplay( image, marker, 0.01, 0.0, handle );
   ComputeAndDisplay( image, marker, 0.1, 0.0, handle );
   ComputeAndDisplay( image, marker, 1.0, 0.0, handle );

   // Graph cuts with different values for gamma
   ComputeAndDisplay( image, marker, 0.1, 0.0003, handle );
   ComputeAndDisplay( image, marker, 0.1, 0.0004, handle );
   ComputeAndDisplay( image, marker, 0.1, 0.0010, handle );

   dip::viewer::Spin();
}
