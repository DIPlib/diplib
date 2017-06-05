#include <iostream>
#include <random>
#include <chrono>

#include "diplib/private/pcg_random.h"
#include "prng_engine.hpp"

#undef DIP__ENABLE_DOCTEST
#include "diplib/library/numeric.h"

// #define TEST_DISCARD

// g++ -std=c++11 -O3 ../test/test_random.cpp -o test_random -I../include -I../dependencies/pcg-cpp/include -I.

void test( std::vector< double > const& array ) {
   dip::MinMaxAccumulator macc;
   dip::StatisticsAccumulator sacc;
   for( double a: array ) {
      macc.Push( a );
      sacc.Push( a );
   }
   if( std::abs( macc.Minimum() - 0.0 ) > 1e-6 ) {
      std::cout << "Minimum value = " << macc.Minimum() << " (expect 0.000)\n";
   }
   if( std::abs( macc.Maximum() - 1.0 ) > 1e-6 ) {
      std::cout << "Maximum value = " << macc.Maximum() << " (expect 1.000)\n";
   }
   if( std::abs( sacc.Mean() - 0.5 ) > 1e-3 ) {
      std::cout << "Mean = " << sacc.Mean() << " (expect 0.500)\n";
   }
   if( std::abs( sacc.StandardDeviation() - std::sqrt( 1.0 / 12.0 )) > 1e-3 ) {
      std::cout << "Standard deviation = " << sacc.StandardDeviation() << " (expect 0.2887)\n";
   }
   if( std::abs( sacc.Skewness() - 0.0 ) > 1e-3 ) {
      std::cout << "Skewness = " << sacc.Skewness() << " (expect 0.000)\n";
   }
   if( std::abs( sacc.ExcessKurtosis() + 1.2 ) > 1e-2 ) {
      std::cout << "Excess kurtosis = " << sacc.ExcessKurtosis() << " (expect -1.200)\n";
   }
}

int main() {

   std::vector< double > array( 100000000 );
   std::uniform_real_distribution<> dis( 0.0, 1.0 );

   // Touch all memory
   for( double& a: array ) {
      a = 0.0;
   }

   // PGC XSH RR 64/32
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_xsh_rr_64_32 /* == pcg32 */ rng( seed_source );
      rng.set_stream( 0 );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPCG XSH RR 64/32 time: " << time << " ms\n";
      test( array );
   }

   // PGC XSH RR 128/64
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_xsh_rr_128_64 rng( seed_source );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPCG XSH RR 128/64 time: " << time << " ms\n";
      test( array );
   }

   // PGC XSL RR 128/64
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_xsl_rr_128_64 /* == pcg64 */ rng( seed_source );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPCG XSL RR 128/64 time: " << time << " ms\n";
      test( array );
   }

   // PGC RXS M XS 64/64
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_rxs_m_xs_64_64 /* == pcg64_once_insecure */ rng( seed_source );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPGC RXS M XS 64/64 time: " << time << " ms\n";
      test( array );
   }

   // PGC RXS M XS 128/128
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_rxs_m_xs_128_128 rng( seed_source );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPGC RXS M XS 128/128 time: " << time << " ms\n";
      test( array );
   }

   // PGC XLS RR RR 128/128
   {
      pcg_extras::seed_seq_from< std::random_device > seed_source;
      pcg_engines::setseq_xsl_rr_rr_128_128 /* == pcg128_once_insecure */ rng( seed_source );
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nPGC XLS RR RR 128/128 time: " << time << " ms\n";
      test( array );
   }

   // Sitmo PRNG
   {
      sitmo::prng_engine rng;
      auto t0 = std::chrono::steady_clock::now();
      for( double& a: array ) {
         a = dis( rng );
         #ifdef TEST_DISCARD
         rng.discard( 10 );
         #endif
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< double >( t1 - t0 )).count();
      std::cout << "\nSitmo PRNG time: " << time << " ms\n";
      test( array );
   }


   return 0;
}
