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

dip::Random rndGen( 0 );

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

   dip::Image in;
   dip::Image out;
   for( auto sz : sizes ) {
      in.ReForge( { sz, sz }, 1, dip::DT_SCOMPLEX );
      dip::dfloat t = TimeIt( in, out );
      std::cout << "size = " << sz << ", time = " << t * 1e3 << " ms\n";
   }
   std::cout << '\n';

   return 0;
}

/* On my computer:

  size  |                            time (ms)                            |
  (px)  +---------------------+---------------------+---------------------+
        |    FFTW_ESTIMATE    |     FFTW_MEASURE    |       OpenCV        | method
        +----------+----------+----------+----------+----------+----------+
        |     1    |     4    |     1    |     4    |     1    |     4    | threads
--------+----------+----------+----------+----------+----------+----------+
    256 |    1.476 |    0.930 |    1.432 |    0.838 |    1.547 |    0.520 | powers of 2
   1024 |   29.109 |    8.368 |   26.517 |    8.052 |   45.793 |    9.004 |
   2048 |  118.075 |   38.917 |  118.404 |   38.488 |  138.715 |   43.202 |
   4096 |  567.868 |  171.512 |  564.869 |  170.517 |  628.851 |  184.095 |
    243 |    1.332 |    0.486 |    1.315 |    0.499 |    1.441 |    0.438 | powers of 3
    729 |   14.446 |    4.433 |   14.431 |    4.431 |   15.769 |    4.155 |
   2187 |  165.206 |   43.282 |  162.395 |   42.082 |  166.564 |   38.201 |
    125 |    0.317 |    0.197 |    0.312 |    0.260 |    0.340 |    0.172 | powers of 5
    625 |   10.451 |    2.947 |    9.903 |    2.786 |   11.059 |    3.094 |
   3125 |  377.278 |  105.222 |  359.195 |  100.290 |  392.892 |  110.093 |
     49 |    0.344 |    0.193 |    0.069 |    0.115 |    0.079 |    0.164 | powers of 7
    343 |    2.979 |    0.918 |    2.981 |    0.924 |    5.313 |    1.475 |
   2401 |  205.708 |   55.005 |  206.502 |   54.619 |  359.093 |   95.450 |
    121 |    0.379 |    0.221 |    0.376 |    0.218 |    0.579 |    0.261 | powers of 11
   1331 |   66.834 |   17.798 |   64.851 |   17.235 |  105.669 |   27.319 |
    211 |    2.446 |    0.885 |    1.823 |    0.758 |   11.423 |    3.136 | primes
    521 |   13.799 |    3.857 |   10.941 |    3.223 |  167.608 |   43.012 |
   1013 |   83.285 |   22.160 |   78.705 |   20.822 | 1194.8   |  313.467 |
   1531 |  133.017 |   34.380 |  101.332 |   26.642 | 4092.7   | 1072.99  |
--------+----------+----------+----------+----------+----------+----------+

Single-threaded, FFTW is always faster. For prime sizes, the differences are up
to 2 orders of magnitude.

Multi-threaded, the OpenCV code can be slightly faster for small image sizes,
FFTW's setup overhead is probably larger.

*/
