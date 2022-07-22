/*
 * This program times the Fourier transform for different image sizes.
 * Use it to compare the timing of the built-in DFT vs FFTW with different planning methods.
 */

#include <iostream>
#include "diplib.h"
#include "diplib/multithreading.h"
#include "diplib/transform.h"
#include "diplib/generation.h"
#include "diplib/testing.h"

dip::dfloat TimeIt( dip::Image const& img, dip::Image& out ) {
   dip::uint N = 5;
   dip::dfloat time = 1e9;
   for( dip::uint ii = 0; ii < 5; ++ii ) {
      dip::testing::Timer timer;
      for( dip::uint jj = 0; jj < N; ++jj ) {
         out.Strip();
         dip::FourierTransform( img, out );
      }
      timer.Stop();
      time = std::min( time, timer.GetWall() / dip::dfloat( N ));
   }
   return time;
}

int main() {
   // A set of sizes: powers of 2,                     3,              5,              7,             11,        primes
   dip::UnsignedArray sizes = { 256, 1024, 2048, 4096, 243, 729, 2187, 125, 625, 3125, 49, 343, 2401, 121, 1331, 211, 521, 1013, 1531 };

   dip::SetNumberOfThreads( 0 );
   std::cout << "Number of threads: " << dip::GetNumberOfThreads() << '\n';

   dip::Random rndGen( 0 );
   dip::Image in;
   dip::Image out;
   for( auto sz : sizes ) {
      if( false ) {
         // For complex to complex transform:
         in.ReForge( { sz, sz }, 1, dip::DT_SCOMPLEX );
         dip::Image tmp = in.SplitComplex();
         tmp.Protect();
         tmp.Fill( 0 );
         dip::UniformNoise( tmp, tmp, rndGen );
      } else {
         in.ReForge( { sz, sz }, 1, dip::DT_SFLOAT );
         in.Fill( 0 );
         dip::UniformNoise( in, in, rndGen );
      }
      dip::dfloat t = TimeIt( in, out );
      std::cout << "size = " << sz << ", time = " << t * 1e3 << " ms\n";
   }
   std::cout << '\n';
}

/* On my computer (2021 M1 iMac):

Complex-to-complex transform:

        |                            time (ms)                            |
        +---------------------+---------------------+---------------------+
  size  |        FFTW         |       OpenCV        |      PocketFFT      | method
  (px)  +----------+----------+----------+----------+----------+----------+
        |     1    |     8    |     1    |     8    |     1    |     8    | threads
--------+----------+----------+----------+----------+----------+----------+
    256 |    1.925 |    1.150 |    2.165 |    1.324 |    2.743 |    1.281 | powers of 2
   1024 |   35.815 |   10.238 |   40.567 |   11.413 |   51.594 |   13.982 |
   2048 |  167.879 |   49.558 |  188.559 |   54.019 |  231.366 |   62.997 |
   4096 |  722.095 |  193.423 |  805.594 |  208.841 |  987.625 |  248.634 |
    243 |    1.494 |    0.730 |    1.837 |    0.836 |    2.204 |    0.912 | powers of 3
    729 |   13.928 |    4.092 |   17.543 |    4.91  |   20.224 |    5.587 |
   2187 |  149.26  |   39.890 |  180.178 |   48.539 |  202.16  |   50.902 |
    125 |    0.378 |    0.350 |    0.456 |    0.459 |    0.584 |    0.507 | powers of 5
    625 |   10.591 |    3.244 |   12.716 |    3.864 |   14.920 |    4.388 |
   3125 |  334.252 |   85.032 |  389.352 |   98.530 |  433.798 |  108.911 |
     49 |    0.068 |    0.340 |    0.094 |    0.346 |    0.101 |    0.341 | powers of 7
    343 |    3.127 |    1.197 |    5.388 |    1.790 |    4.763 |    1.584 |
   2401 |  189.464 |   49.373 |  333.255 |   84.019 |  266.883 |   69.240 |
    121 |    0.402 |    0.454 |    0.624 |    0.524 |    0.578 |    0.519 | powers of 11
   1331 |   62.785 |   17.218 |  101.326 |   26.983 |   83.167 |   22.177 |
    211 |    2.590 |    0.948 |   11.883 |    3.088 |    3.13  |    1.162 | primes
    521 |   19.672 |    5.511 |  179.597 |   37.794 |   18.702 |    5.358 |
   1013 |   82.101 |   20.139 | 1321.51  |  254.447 |   70.472 |   18.966 |
   1531 |  163.703 |   40.256 | 4571.56  |  895.703 |  196.947 |   50.495 |
--------+----------+----------+----------+----------+----------+----------+

Real-to-complex transform:

        |                            time (ms)                            |
        +---------------------+---------------------+---------------------+
  size  |        FFTW         |       OpenCV        |      PocketFFT      | method
  (px)  +----------+----------+----------+----------+----------+----------+
        |     1    |     8    |     1    |     8    |     1    |     8    | threads
--------+----------+----------+----------+----------+----------+----------+
    256 |    0.877 |    0.783 |    N/A   |    N/A   |    0.946 |    0.802 | powers of 2
   1024 |   14.421 |    4.357 |    N/A   |    N/A   |   14.812 |    4.532 |
   2048 |   69.697 |   22.847 |    N/A   |    N/A   |    70.99 |   23.477 |
   4096 |  306.084 |   87.382 |    N/A   |    N/A   |  309.35  |   88.591 |
    243 |    0.417 |    0.393 |    N/A   |    N/A   |    0.332 |    0.383 | powers of 3
    729 |    4.113 |    1.522 |    N/A   |    N/A   |    3.409 |    1.303 |
   2187 |   47.198 |   14.491 |    N/A   |    N/A   |   38.831 |   12.804 |
    125 |    0.104 |    0.197 |    N/A   |    N/A   |    0.091 |    0.182 | powers of 5
    625 |    3.064 |    1.275 |    N/A   |    N/A   |    2.406 |    0.991 |
   3125 |   111.20 |   31.105 |    N/A   |    N/A   |   88.345 |   26.829 |
     49 |    0.024 |    0.231 |    N/A   |    N/A   |    0.02  |    0.269 | powers of 7
    343 |    0.869 |    0.439 |    N/A   |    N/A   |    0.816 |    0.397 |
   2401 |   61.717 |   18.361 |    N/A   |    N/A   |   58.099 |   17.601 |
    121 |    0.124 |    0.275 |    N/A   |    N/A   |    0.101 |    0.269 | powers of 11
   1331 |   19.522 |    5.688 |    N/A   |    N/A   |   16.685 |    4.976 |
    211 |    1.057 |    0.485 |    N/A   |    N/A   |    0.986 |    0.418 | primes
    521 |    8.296 |    2.466 |    N/A   |    N/A   |    5.857 |    1.873 |
   1013 |   35.117 |    9.283 |    N/A   |    N/A   |   22.029 |    6.132 |
   1531 |   67.230 |   17.176 |    N/A   |    N/A   |   66.942 |   17.196 |
--------+----------+----------+----------+----------+----------+----------+

*/
