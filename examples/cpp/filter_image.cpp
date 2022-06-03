/*
 * This program shows different ways to apply a convolution with a custom kernel to an image.
 * It displays the result using dip::viewer::Show.
 */

#include "diplib.h"
#include "dipviewer.h"
#include "diplib/file_io.h"
#include "diplib/linear.h"
#include "diplib/generation.h"
#include "diplib/statistics.h"

int main( int argc, char *argv[] ) {
   dip::Random random;

   // Parse input argument and create a filter
   int option = 0;
   if( argc > 1 ) {
      option = std::atoi( argv[ 1 ] );
   }
   dip::Image filter;
   switch( option ) {
      case 0:
         // Let's apply a Gabor filter
         filter = dip::CreateGabor( { 10.0, 10.0 }, { 0.1, 0.2 } );
         break;
      case 1:
         // Let's apply a Gaussian filter
         filter = dip::CreateGauss( { 8.0, 8.0 } );
         break;
      case 2:
         // Let's apply a large square filter with random values (not separable)
         filter = dip::Image( { 21, 21 }, 1, dip::DT_DFLOAT );
         filter.Fill( 0 );
         dip::UniformNoise( filter, filter, random, 0.0, 1.0 );
         filter /= dip::Mean( filter );
         break;
      default:
         std::cerr << "Input argument should be an integer between 0 and 2.";
         return 1;
   }

   // Create a test image
   dip::Image input = dip::ImageReadICS( DIP_EXAMPLES_DIR "/trui.ics" );

   // Try to separate the filter
   auto filterArray = dip::SeparateFilter( filter );

   dip::Image output_separable;
   if( !filterArray.empty() ) {
      // OK, the filter was separable
      std::cout << "filterArray (" << filterArray.size() << " elements)\n";
      for( auto const& f: filterArray ) {
         std::cout << "  - size = " << f.filter.size() / ( f.isComplex ? 2 : 1 )
                   << ", isComplex = " << f.isComplex
                   << ", symmetry = \"" << f.symmetry
                   << "\", origin = " << f.origin << '\n';
      }
      dip::SeparableConvolution( input, output_separable, filterArray );
   }

   // Compute the convolution through the other two methods
   dip::Image output_ft = dip::ConvolveFT( input, filter );
   dip::Image output_direct = dip::GeneralConvolution( input, filter );

   // Display input and outputs
   dip::viewer::Show( input, "input image" );
   if( output_separable.IsForged() ) {
      dip::viewer::Show( output_separable, "output image, separable implementation" );
   }
   dip::viewer::Show( output_ft, "output image, Fourier implementation" );
   dip::viewer::Show( output_direct, "output image, direct implementation" );
   dip::viewer::Spin();
}
