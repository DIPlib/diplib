#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/multithreading.h"
#include "diplib/generation.h"
#include "diplib/testing.h"

#include "diplib/morphology.h"

dip::Random rndGen( 0 );

dip::dfloat TimeIt( dip::Image const& img, dip::Image& out, dip::StructuringElement const& se, dip::uint N ) {
   dip::dfloat time = 1e9;
   for( dip::uint ii = 0; ii < 10; ++ii ) {
      dip::testing::Timer timer;
      for( dip::uint jj = 0; jj < N; ++jj ) {
         out.Strip();
         dip::Dilation( img, out, se /*, { "add zeros" }*/ );
         //dip::Opening( img, out, se /*, { "add zeros" }*/ );
      }
      timer.Stop();
      time = std::min( time, timer.GetCpu() / dip::dfloat( N ));
   }
   return time;
}

int main() {
   dip::Image img( { 1800, 2100 }, 1, dip::DT_SFLOAT );
   img.Fill( 50 );
   dip::GaussianNoise( img, img, rndGen, 400.0 );

   dip::UnsignedArray sizes = { 2, 3, 4, 5, 7, 10, 20, 30, 50, 100 };

   dip::SetNumberOfThreads( 1 );

   dip::Image out;
   for( auto sz : sizes ) {
      try {
         dip::dfloat timeV = TimeIt( img, out, {{ 1.0, dip::dfloat( sz ) }, "rectangular" }, 4 );
         dip::dfloat timeH = TimeIt( img, out, {{ dip::dfloat( sz ), 1.0 }, "rectangular" }, 4 );
         std::cout << "size = " << sz << ", time vertical = " << timeV * 1e3 << " ms, time horizontal = " << timeH * 1e3 << " ms\n";
      } catch ( dip::Error& e ) {
         std::cout << e.what() << std::endl;
      }
   }
   std::cout << std::endl;
   for( auto sz : sizes ) {
      try {
         dip::dfloat timeP = TimeIt( img, out, {{ dip::dfloat( sz ) * 2, dip::dfloat( sz ) }, "periodic line" }, 1 );
         dip::dfloat timeD = TimeIt( img, out, {{ dip::dfloat( sz ), dip::dfloat( sz ) }, "periodic line" }, 1 );
         std::cout << "size = " << sz << ", time periodic = " << timeP * 1e3 << " ms, time diagonal = " << timeD * 1e3 << " ms\n";
      } catch ( dip::Error& e ) {
         std::cout << e.what() << std::endl;
      }
   }
   std::cout << std::endl;
   for( auto sz : sizes ) {
      try {
         dip::dfloat timeE = TimeIt( img, out, { dip::dfloat( sz ), "elliptic" }, 1 );
         dip::dfloat timeD = TimeIt( img, out, { dip::dfloat( sz ), "diamond" }, 1 );
         std::cout << "size = " << sz << ", time elliptic = " << timeE * 1e3 << " ms, time diamond = " << timeD * 1e3 << " ms\n";
      } catch ( dip::Error& e ) {
         std::cout << e.what() << std::endl;
      }
   }

   return 0;
}
