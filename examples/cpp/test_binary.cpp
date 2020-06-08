/*
 * Testing the binary morphology functions and comparing their timing to the grey-value equivalents.
 */

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/morphology.h"
#include "diplib/distance.h"
#include "diplib/file_io.h"
#include "diplib/testing.h"
#include "diplib/multithreading.h"
#include "diplib/geometry.h"

int main( int argc, char** argv ) {
   dip::testing::Timer timer;
   dip::uint reps = 100;

   // Set default input and output paths.
   // Can be overridden by commandline arguments.
   std::string inputPath = DIP_EXAMPLES_DIR;

   // Arguments:
   // 1: input path (optional)
   if( argc >= 2 ) {
      inputPath = argv[ 1 ];
   }

   std::cout << "Reading " << inputPath << "/erika\n";
   dip::Image image = dip::ImageReadTIFF( inputPath + "/erika" ) > 100;
   dip::Tile( { image, image, image, image, image, image }, image, { 3, 2 } );
   std::cout << image;

   //dip::SetNumberOfThreads( 1 ); // Uncomment this line if you don't want to allow dip::Dilation and dip::EuclideanDistanceTransform to use parallelism
   double binTime, time;

   std::cout << "\nsquare dilations:\n";

   for( dip::uint kk = 1; kk < 7; ++kk ) {

      if( dip::Count( dip::BinaryDilation( image, 2, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "rectangular" } )) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::BinaryDilation( image, 2, kk );
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Dilation( image, { double( 2 * kk + 1 ), "rectangular" } );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\ndiamond dilations:\n";

   for( dip::uint kk = 1; kk < 7; ++kk ) {

      dip::Image diff = dip::BinaryDilation( image, 1, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "diamond" } );
      // Ignore boundaries, we make an error there in dip::Dilation with a diamond SE
      diff = diff.At( dip::Range( dip::sint( kk ), -1 - dip::sint( kk )), dip::Range( dip::sint( kk ), -1 - dip::sint( kk )));
      if( dip::Count( diff ) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::BinaryDilation( image, 1, kk );
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Dilation( image, { double( 2 * kk + 1 ), "diamond" } );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\noctagonal dilations:\n";

   for( dip::uint kk = 2; kk < 9; kk += 2 ) {

      if( dip::Count( dip::BinaryDilation( image, -1, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "octagonal" } )) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::BinaryDilation( image, -1, kk );
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Dilation( image, { double( 2 * kk + 1 ), "octagonal" } );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\nisotropic dilations:\n";

   for( dip::uint kk = 5; kk < 20; kk += 2 ) {

      image.ResetPixelSize(); // We want distances to be in pixels, not physical units.

      if( dip::Count(( dip::EuclideanDistanceTransform( !image, "object", "square" ) < kk * kk ) != dip::Dilation( image, { double( kk ) * 2 - 0.001, "elliptic" } )) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::EuclideanDistanceTransform( !image, "object", "square" ) < kk * kk;
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::Dilation( image, { double( kk ) * 2 - 0.001, "elliptic" } );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), DT method is " << time / binTime << " times faster\n";

   }

   std::cout << "\npropagation:\n";

   for( dip::uint kk = 1; kk < 10; kk += 1 ) {

      dip::Image seeds = dip::Erosion( image, static_cast< dip::dfloat >( kk ));

      if( dip::Count( dip::BinaryPropagation( seeds, image, 1 ) != dip::MorphologicalReconstruction( seeds, image, 1 )) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps/4; ++ii ) {
         dip::Image out = dip::BinaryPropagation( seeds, image, 1 );
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps/4; ++ii ) {
         dip::Image out = dip::MorphologicalReconstruction( seeds, image, 1 );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << time / binTime << " times faster\n";

   }

   std::cout << "\ninverse propagation:\n";

   for( dip::uint kk = 1; kk < 10; kk += 1 ) {

      dip::Image mask = dip::Erosion( image, static_cast< dip::dfloat >( kk ));

      if( dip::Count( ~dip::BinaryPropagation( ~image, ~mask, 1 ) != dip::MorphologicalReconstruction( image, mask, 1, dip::S::EROSION )) != 0 ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps/4; ++ii ) {
         dip::Image out = ~dip::BinaryPropagation( ~image, ~mask, 1 );
      }
      timer.Stop();
      binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps/4; ++ii ) {
         dip::Image out = dip::MorphologicalReconstruction( image, mask, 1, dip::S::EROSION );
      }
      timer.Stop();
      time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << time / binTime << " times faster\n";

   }
}
