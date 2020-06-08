/*
 * This is a simple program that replicates the dip::RadialMean function. It is not very efficient,
 * uses an intermediate image to store the radius, and uses no multithreading. But it shows how to
 * use dip::JointImageIterator to iterate over two images simultaneously.
 */

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"

int main() {

   // Input image
   dip::Image input = dip::ImageReadICS( DIP_EXAMPLES_DIR "/cermet.ics" );
   dip::FourierTransform( input, input );
   dip::Norm( input, input );

   // Step 1: create image with distance to origin
   // (filling the coordinate into an integer typed image causes the distance to be truncated towards zero,
   // equivalent to `floor` because distances are always positive).
   dip::Image coords = input.Similar( dip::DT_UINT16 ); // 16 bits should be enough for any normal image size.
   dip::FillRadiusCoordinate( coords, { "right" } ); // "right" is the default, but we specify it here for completeness.

   // Step 2: prepare arrays to hold the distribution
   dip::uint maxRadius = ( *std::min_element( input.Sizes().begin(), input.Sizes().end() )) / 2;
   std::vector< double > radialMean( maxRadius, 0 );
   std::vector< dip::uint > numel( maxRadius, 0 );

   // Step 3: iterate over images and accumulate distribution
   dip::JointImageIterator< dip::sfloat, dip::uint16 > it( { input, coords } );
   it.Optimize();
   do {
      dip::uint index = it.Sample<1>();
      if( index < maxRadius ) {
         radialMean[ index ] += it.Sample<0>();
         ++numel[ index ];
      }
   } while( ++it );

   // Step 4: divide sum by number to get mean per radius
   for( dip::uint ii = 0; ii < maxRadius; ++ii ) {
      if( numel[ ii ] > 0 ) {
         radialMean[ ii ] /= static_cast< double >( numel[ ii ] );
      }
   }

   // Output to show off result
   std::cout << "The RadialMean of the FT of Cermet:\n";
   for( auto m : radialMean ) {
      std::cout << m << ", ";
   }
   std::cout << '\n';
}
