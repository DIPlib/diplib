/*
 * Testing the binary morphology functions and comparing their timing to the grey-value equivalents.
 */

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/morphology.h"
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

   std::cout << "Reading " << inputPath << "/cameraman\n";
   dip::Image image = dip::ImageReadTIFF( inputPath + "/cameraman" ) > 100;
   dip::Tile( { image, image, image, image, image, image, image, image, image, image, image, image }, image, { 4, 3 } );
   std::cout << image;

   std::cout << "\nTimes below are for grayscale vs binary version of operator.\n";
   std::cout << "Each operator is applied " << reps << " times.\n";

   //dip::SetNumberOfThreads( 1 ); // Uncomment this line if you don't want to allow dip::Dilation and dip::EuclideanDistanceTransform to use parallelism

   std::cout << "\nsquare dilations:\n";

   for( dip::uint kk = 1; kk < 7; ++kk ) {

      dip::Image diff = dip::BinaryDilation( image, 2, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "rectangular" } );
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::BinaryDilation( image, 2, kk );
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::Dilation( image, { double( 2 * kk + 1 ), "rectangular" } );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\ndiamond dilations:\n";

   for( dip::uint kk = 1; kk < 7; ++kk ) {

      dip::Image diff = dip::BinaryDilation( image, 1, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "diamond" } );
      // Ignore boundaries, we make an error there in dip::Dilation with a diamond SE
      diff = diff.At( dip::Range( dip::sint( kk ), -1 - dip::sint( kk )), dip::Range( dip::sint( kk ), -1 - dip::sint( kk )));
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::BinaryDilation( image, 1, kk );
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::Dilation( image, { double( 2 * kk + 1 ), "diamond" } );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\noctagonal dilations:\n";

   for( dip::uint kk = 2; kk < 9; kk += 2 ) {

      dip::Image diff = dip::BinaryDilation( image, -1, kk ) != dip::Dilation( image, { double( 2 * kk + 1 ), "octagonal" } );
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::BinaryDilation( image, -1, kk );
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::Dilation( image, { double( 2 * kk + 1 ), "octagonal" } );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time << " times slower\n";

   }

   std::cout << "\nisotropic dilations:\n";

   for( dip::uint kk = 5; kk < 20; kk += 2 ) {

      dip::Image diff = dip::IsotropicDilation( image, double( kk )) != dip::Dilation( image, { double( kk ) * 2 - 0.001, "elliptic" } );
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::IsotropicDilation( image, double( kk ));
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::Dilation( image, { double( kk ) * 2 - 0.001, "elliptic" } );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << time / binTime << " times faster\n";

   }

   std::cout << "\npropagation (with a large number of iterations, not using 0 because then both use the same code):\n";
   // Note that MorphologicalReconstruction calls BinaryPropagation with iterations=0.

   for( dip::uint kk = 1; kk < 10; kk += 1 ) {

      dip::Image seeds = dip::Erosion( image, static_cast< dip::dfloat >( kk ));

      dip::Image diff = dip::BinaryPropagation( seeds, image, 1, 100000 ) != dip::MorphologicalReconstruction( seeds, image, 1 );
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::BinaryPropagation( seeds, image, 1, 100000 );
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::MorphologicalReconstruction( seeds, image, 1 );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time  << " times slower\n";

   }

   std::cout << "\ninverse propagation (with a large number of iterations, not using 0 because then both use the same code):\n";

   for( dip::uint kk = 1; kk < 10; kk += 1 ) {

      dip::Image mask = dip::Erosion( image, static_cast< dip::dfloat >( kk ));

      dip::Image diff = ~dip::BinaryPropagation( ~image, ~mask, 1, 100000 ) != dip::MorphologicalReconstruction( image, mask, 1, dip::S::EROSION );
      if( dip::Any( diff ).As< bool >() ) {
         std::cout << "!!!Error for kk = " << kk << '\n';
      }

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = ~dip::BinaryPropagation( ~image, ~mask, 1, 100000 );
      }
      timer.Stop();
      double binTime = timer.GetWall();

      timer.Reset();
      for( dip::uint ii = 0; ii < reps; ++ii ) {
         dip::Image out = dip::MorphologicalReconstruction( image, mask, 1, dip::S::EROSION );
      }
      timer.Stop();
      double time = timer.GetWall();
      std::cout << kk << ": " << time << " vs " << binTime << " (s), binary is " << binTime / time  << " times slower\n";

   }
}
