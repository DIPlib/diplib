/*
 * This program shows how to use dip::FindHoughCircles and dip::RadonTransformCircles.
 * It displays some images using dip::viewer::Show, and sets display properties.
 */

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/detection.h"
#include "diplib/generation.h"
#include "diplib/linear.h"
#include "diplib/mapping.h"
#include "diplib/math.h"

#include "dipviewer.h"
#include "diplib/viewer/slice.h"
#include "diplib/viewer/viewer.h"


int main() {
   // Create a test image
   dip::Image img = dip::Image( { 256, 256 }, 1 );
   std::cout << "Ground-truth:\n";
   dip::FloatArray center = { 117.73, 133.10 };
   double radius = 100.72;
   std::cout << "  - circle at (" << center[ 0 ] << ", " << center[ 1 ] << "), radius " << radius << '\n';
   dip::DrawBandlimitedBall( img, radius * 2, center, { 255 } );
   dip::Random rng;
   dip::GaussianNoise( img, img, rng, 5 ); // A small amount of noise
   dip::viewer::Show( img, "input image" );

   // Compute gradient and mask
   dip::Image gv = dip::Gradient( img );
   dip::viewer::Show( gv, "gradient image" );
   dip::Image gm = dip::Norm( gv );
   dip::Image mask = gm > 40;
   dip::viewer::Show( mask, "mask image" );

   // Compute Hough circles
   auto hough = dip::FindHoughCircles( mask, gv, { 50, 150 } );

   // Compose output
   dip::Image out = img.Similar();
   out.Fill( 0 );
   std::cout << "Detected by Hough:\n";
   for( auto v : hough ) {
      std::cout << "  - circle at (" << v[ 0 ] << ", " << v[ 1 ] << "), radius " << v[ 2 ] << '\n';
      dip::DrawBandlimitedBall( out, v[ 2 ] * 2, { v[ 0 ], v[ 1 ] }, { 255 } );
   }
   dip::viewer::Show( out, "Hough detected circles" );
   auto h = dip::viewer::Show( out - img, "difference Hough - input" );
   h->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Divergent;
   h->options().mapping_range_ = { -100, 100 };

   // Compute Radon circles
   // NOTE! `out` is not used in the call below because we added "no parameter space" to the options.
   auto radon = dip::RadonTransformCircles( gm, out, { 50, 150, 1 }, 1.0, 100.0, "full", { "normalize", "correct", "no parameter space" } );

   // Compose output
   out.Fill( 0 );
   std::cout << "Detected by Radon:\n";
   for( auto v : radon ) {
      std::cout << "  - circle at (" << v.origin[ 0 ] << ", " << v.origin[ 1 ] << "), radius " << v.radius << '\n';
      dip::DrawBandlimitedBall( out, v.radius * 2, v.origin, { 255 } );
   }
   dip::viewer::Show( out, "Radon detected circles" );
   h = dip::viewer::Show( out - img, "difference Radon - input" );
   h->options().lut_ = dip::viewer::ViewingOptions::LookupTable::Divergent;
   h->options().mapping_range_ = { -100, 100 };

   dip::viewer::Spin();
}
