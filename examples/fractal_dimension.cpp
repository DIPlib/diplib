#undef DIP__ENABLE_DOCTEST

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/morphology.h"
#include "diplib/file_io.h"
#include "diplib/segmentation.h"

#include "diplib/testing.h"

int main( int argc, char *argv[] ) {

   if( argc < 2 ) {

      dip::Image binary = dip::Infimum( dip::CreateXCoordinate( { 256, 200 } ), dip::CreateYCoordinate( { 256, 200 } )) < 70;

      dip::testing::Timer timer;
      dip::dfloat FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "Solid square: FD = " << FD << "; " << timer << std::endl;

      binary -= dip::Erosion( binary, 3 );

      timer.Reset();
      FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "Square edge: FD = " << FD << "; " << timer << std::endl;

      dip::FillDelta( binary );

      timer.Reset();
      FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "Delta function: FD = " << FD << "; " << timer << std::endl;

      binary.Fill( 0 );
      dip::Random random;
      dip::BinaryNoise( binary, binary, random, 0.1, 0.1 );

      timer.Reset();
      FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "Sparse point process: FD = " << FD << "; " << timer << std::endl;

      binary.Fill( 0 );
      dip::BinaryNoise( binary, binary, random, 0.4, 0.4 );

      timer.Reset();
      FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "Dense point process: FD = " << FD << "; " << timer << std::endl;

   } else {

      dip::Image binary;
      try {
         binary = dip::ImageReadTIFF( argv[ 1 ] );
      } catch( dip::RunTimeError const& e ) {
         std::cout << e.Message() << std::endl;
         return -1;
      }

      binary = !dip::Threshold( binary );

      dip::testing::Timer timer;
      dip::dfloat FD = dip::FractalDimension( binary, 0.5 );
      timer.Stop();
      std::cout << "FD = " << FD << "; " << timer << std::endl;

   }

   return 0;
}
