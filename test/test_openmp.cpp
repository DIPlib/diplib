#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/multithreading.h"
#include "diplib/generation.h"
//#include "diplib/linear.h"
//#include "diplib/nonlinear.h"
//#include "diplib/morphology.h"
//#include "diplib/transform.h"
//#include "diplib/statistics.h"
//#include "diplib/geometry.h"
#include "diplib/testing.h"

#include "diplib/histogram.h"

dip::Random rndGen( 0 );

dip::dfloat TimeIt( dip::Image const& img, dip::Image& out ) {
   dip::dfloat time = 1e9;
   for( dip::uint ii = 0; ii < 10; ++ii ) {
      dip::testing::Timer timer;
      for( dip::uint jj = 0; jj < 50; ++jj ) {
         out.Strip();
         //-- Framework::Full --
         //dip::GeneralConvolution( img, filter, out );
         //dip::Uniform( img, out );
         //dip::MedianFilter( img, out );
         //-- Framework::Separable --
         //dip::Gauss( img, out, { 1.0, 0.0 } );
         //dip::Gauss( img, out, { 5.0 }, { 0 }, "IIR" );
         //dip::Uniform( img, out, "rectangular" );
         //dip::Dilation( img, out, "parabolic" );
         //dip::FourierTransform( img, out );
         //dip::CumulativeSum( img, out );
         //dip::Resampling( img, out, { 1.1 }, { 0.3 }, "3-cubic" );
         //-- Framework::Scan --
         //dip::GaussianNoise( img, out, rndGen, 1.0 );
         //dip::PoissonNoise( img, out, rndGen, 1.0 );
         //dip::Norm( img, out );
         //dip::Angle( img, out );
         //dip::SingularValues( img, out );
         //dip::Image U, V; dip::SingularValueDecomposition( img, out, U, V );
         //dip::PseudoInverse( img, out );
         //dip::Square( img, out );
         //dip::BesselY1( img, out );
         //dip::BesselYN( img, out, 8 ); // 200
         //dip::Erf( img, out ); // 60
         //dip::Sinc( img, out ); // 65
         //dip::CreateRadiusCoordinate( img, out );
         //dip::CreatePhiCoordinate( img, out );
         //dip::CreateCoordinates( img, out, {}, "cartesian" );
         auto h = dip::Histogram( img ); // TODO: Rewrite how parallelism is done? This does not work!
      }
      timer.Stop();
      time = std::min( time, timer.GetWall());
   }
   return time;
}

int main() {
   dip::Random random( 0 );

   dip::Image filter( { 7, 7 }, 1, dip::DT_SFLOAT );
   filter.Fill( 50 );
   dip::GaussianNoise( filter, filter, random, 20.0 * 20.0 );

   dip::UnsignedArray sizes =
         //{ 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70 };
         //{ 5, 7, 10, 13, 15, 20, 25 };
         { 10, 15, 25, 50, 75, 100, 125, 150, 200, 300, 400 };

   std::cout << dip::GetNumberOfThreads() << std::endl;

   dip::uint maxThreads = dip::GetNumberOfThreads();

   dip::Image img;
   dip::Image out;
   for( auto sz : sizes ) {

      img = dip::Image{ dip::UnsignedArray{ sz, sz }, 2, dip::DT_SFLOAT };
      //img.ReshapeTensor( dip::Tensor{ "symmetric matrix", 2, 2 } );
      //img.ReshapeTensor( 2, 2 );
      img.Fill( 50 );
      dip::GaussianNoise( img, img, random, 20.0 * 20.0 );

      dip::SetNumberOfThreads( 1 );
      dip::dfloat time1 = TimeIt( img, out );
      dip::SetNumberOfThreads( maxThreads );
      dip::dfloat timeN = TimeIt( img, out );
      std::cout << "size = " << sz << ", time1 = " << time1 * 1e3 << " ms, timeN = " << timeN * 1e3 << " ms\n";

   }

   return 0;
}
