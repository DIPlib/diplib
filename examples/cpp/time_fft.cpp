/*
 * This program times the Fourier transform for different image sizes.
 * Use it to compare the timing of PocketFFT vs FFTW.
 */

#include <iostream>
#include "diplib.h"
#include "diplib/multithreading.h"
#include "diplib/transform.h"
#include "diplib/dft.h"
#include "diplib/generation.h"
#include "diplib/testing.h"

dip::dfloat TimeIt( dip::Image const& img, dip::Image& out, dip::StringSet const& opts ) {
   dip::dfloat time = 1e9;
   for( dip::uint ii = 0; ii < 15; ++ii ) {
      out.Strip();
      dip::testing::Timer timer;
      dip::FourierTransform( img, out, opts );
      timer.Stop();
      time = std::min( time, timer.GetWall() );
   }
   return time;
}

int main() {
   // A set of sizes: powers of 2,                     3,              5,              7,             11,        primes
   dip::UnsignedArray sizes = { 256, 1024, 2048, 4096, 243, 729, 2187, 125, 625, 3125, 49, 343, 2401, 121, 1331, 211, 521, 1013, 1531 };

   dip::SetNumberOfThreads( 1 );
   std::cout << ( dip::usingFFTW ? "FFTW" : "PocketFFT" ) << ", "
             << dip::GetNumberOfThreads() << " threads\n";

   dip::Random rndGen( 0 );
   dip::Image in;
   dip::Image out;

   std::cout << std::setw(6) << std::right << "size"
              << std::setw(10) << std::right << "C2C (ms)"
              << std::setw(10) << std::right << "R2C (ms)"
              << std::setw(10) << std::right << "C2R (ms)" << '\n';
   std::cout << std::setw(6) << std::right << "-----"
             << std::setw(10) << std::right << "---------"
             << std::setw(10) << std::right << "---------"
             << std::setw(10) << std::right << "---------" << '\n';

   for( auto sz : sizes ) {
      // C2C
      in.ReForge( { sz, sz }, 1, dip::DT_SCOMPLEX );
      dip::Image tmp = in.QuickCopy();
      tmp.SplitComplex();
      tmp.Protect();
      tmp.Fill( 0 );
      dip::UniformNoise( tmp, tmp, rndGen );
      dip::dfloat t_c2c = TimeIt( in, out, {} );

      // C2R
      dip::dfloat t_c2r = TimeIt( in, out, { "inverse", "real" } );

      // R2C
      in.ReForge( { sz, sz }, 1, dip::DT_SFLOAT );
      in.Fill( 0 );
      dip::UniformNoise( in, in, rndGen );
      dip::dfloat t_r2c = TimeIt( in, out, {} );

      std::cout << std::setw(6) << std::right << sz
                << std::setw(10) << std::fixed << std::setprecision(2) << std::right << t_c2c * 1e3
                << std::setw(10) << std::fixed << std::setprecision(2) << std::right << t_r2c * 1e3
                << std::setw(10) << std::fixed << std::setprecision(2) << std::right << t_c2r * 1e3 << '\n';
   }
}

/* Timings of `dip::FourierTransform()` for a square image with side `size` on Cris' M1 iMac.

=== DIPlib 3.3 ===

                FFTW, 1 threads               OpenCV, 1 threads
        -----------------------------  -----------------------------
  size   C2C (ms)  R2C (ms)  C2R (ms)   C2C (ms)  R2C (ms)  C2R (ms)
 -----  --------- --------- ---------  --------- --------- ---------
   256       0.90      0.72      0.63       1.09      0.82      0.74  powers of 2
  1024      15.14     14.31      7.38      17.01     15.80      8.77
  2048      78.51     69.36     34.36      86.34     75.02     39.93
  4096     358.98    303.45    188.91     391.51    328.29    213.73
   243       0.50      0.42      0.49       0.66      0.53      0.56  powers of 3
   729       5.00      4.06      4.26       6.72      5.34      5.42
  2187      59.98     46.21     50.69      74.77     57.56     57.89
   125       0.12      0.11      0.12       0.15      0.13      0.13  powers of 5
   625       3.70      3.04      3.05       4.65      3.77      3.79
  3125     148.52    109.52    104.17     174.64    128.72    123.26
    49       0.03      0.03      0.03       0.04      0.03      0.03  powers of 7
   343       1.06      0.87      1.00       2.42      1.88      1.93
  2401      79.38     60.74     65.60     167.80    127.21    128.56
   121       0.15      0.12      0.14       0.27      0.22      0.22  powers of 11
  1331      24.25     19.33     24.66      47.69     36.96     37.40
   211       1.36      1.06      1.07       7.48      5.65      5.66  primes
   521      10.81      8.30      8.36     116.68     87.63     87.66
  1013      46.06     34.98     35.05     868.79    654.88    659.31
  1531      87.69     67.03     65.88    3018.81   2272.91   2272.69

=== New code ===

                FFTW, 1 threads             PocketFFT, 1 threads
        -----------------------------  -----------------------------
  size   C2C (ms)  R2C (ms)  C2R (ms)   C2C (ms)  R2C (ms)  C2R (ms)
 -----  --------- --------- ---------  --------- --------- ---------
   256       0.90      0.61      0.36       0.71      0.61      0.37  powers of 2
  1024      14.88     12.71      5.07      14.99     12.78      5.59
  2048      77.45     61.32     26.15      77.99     61.87     28.17
  4096     354.52    269.27    147.96     359.06    268.56    154.13
   243       0.56      0.41      0.34       0.35      0.27      0.28  powers of 3
   729       4.95      3.53      3.02       3.62      2.69      2.81
  2187      64.58     40.15     42.11      44.49     30.98     38.04
   125       0.13      0.12      0.10       0.10      0.07      0.07  powers of 5
   625       3.44      2.45      2.06       2.49      1.90      1.78
  3125     140.96     86.75     94.29     109.23     72.13     82.51
    49       0.03      0.03      0.03       0.02      0.02      0.02  powers of 7
   343       1.19      0.72      0.68       0.92      0.70      0.68
  2401      82.36     46.95     55.91      69.35     45.84     55.83
   121       0.15      0.12      0.11       0.11      0.10      0.10  powers of 11
  1331      32.90     17.56     19.00      18.94     12.15     12.76
   211       1.33      1.01      0.99       1.23      0.97      0.96  primes
   521      10.66      7.08      6.98       7.30      5.69      5.58
  1013      45.35     30.54     30.48      27.49     21.43     21.16
  1531      86.77     67.25     66.62      85.34     64.88     63.96

*/
