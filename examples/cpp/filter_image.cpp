/*
 * This program shows how to apply a convolution with a custom kernel to an image.
 * It displays the result using dip::viewer::ShowSimple.
 */

#include <vector>
#include <diplib/math.h>
#include "diplib.h"
#include "dipviewer.h"
#include "diplib/file_io.h"
#include "diplib/linear.h"
#include "diplib/generation.h"
#include "diplib/mapping.h"
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
         filter = dip::CreateGabor( { 10.0, 10.0 }, { 0.1, 0.3 } );
         break;
      case 1:
         // Let's apply a Gaussian filter
         filter = dip::CreateGauss( { 8.0, 8.0 } );
         break;
      case 2:
         // Let's apply a large square filter with random values
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
   dip::Image output;

   // Try to separate the filter
   auto filterArray = dip::SeparateFilter( filter );

   if( filterArray.empty() ) {

      // We failed!
      if( filter.NumberOfPixels() > 7 * 7 ) {
         std::cout << "Not separable, using ConvolveFT\n";
         dip::ConvolveFT( input, filter, output );
      } else {
         std::cout << "Not separable, using GeneralConvolution\n";
         dip::GeneralConvolution( input, filter, output );
      }

   } else {

      // OK, the filter was separable
      std::cout << "filterArray (" << filterArray.size() << " elements)\n";
      for( auto& filter : filterArray ) {
         std::cout << "  - size = " << filter.filter.size() / ( filter.isComplex ? 2 : 1 )
                   << ", isComplex = " << filter.isComplex
                   << ", symmetry = " << filter.symmetry
                   << ", origin = " << filter.origin << '\n';
      }
      dip::SeparableConvolution( input, output, filterArray );

   }

   // Display input and output
   dip::viewer::ShowSimple( input, "input image" );
   if( output.DataType().IsComplex() ) {
      output = dip::Modulus( output );
   }
   dip::ContrastStretch( output, output );
   dip::viewer::ShowSimple( dip::Convert( output, dip::DT_UINT8 ), "output image" );
   dip::viewer::Spin();
}
