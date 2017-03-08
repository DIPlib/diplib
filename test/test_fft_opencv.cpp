#include <iostream>
#include <typeinfo>
#include <chrono>

#undef DIP__ENABLE_DOCTEST

#include "../src/transform/opencv_dxt.h"

#ifndef M_PIl
#define M_PIl 3.1415926535897932384626433832795029L
#endif

template< typename T >
void dotest( size_t nfft ) {
   std::cout << "type:" << typeid( T ).name() << " nfft:" << nfft << '\t';

   DFTOptions< T > opts( nfft, false );
   std::vector< std::complex< T >> buf( opts.bufferSize() );

   std::vector< std::complex< T>> inbuf( nfft );
   std::vector< std::complex< T>> outbuf( nfft );
   for( size_t k = 0; k < nfft; ++k ) {
      inbuf[ k ] = std::complex< T >( std::rand(), std::rand() ) / ( T )RAND_MAX - ( T )0.5;
   }
   DFT( inbuf.data(), outbuf.data(), buf.data(), opts, T( 1 ) );

   long double totalpower = 0;
   long double difpower = 0;
   for( size_t k0 = 0; k0 < nfft; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = 2 * k0 * M_PIl / nfft;
      for( size_t k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) *
                std::exp( std::complex< long double >( 0, -( k1 * phinc ) ) );
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ) );
   }
   std::cout << " RMSE:" << std::sqrt( difpower / totalpower ) << '\t'; // Root mean square error

   auto t0 = std::chrono::steady_clock::now();
   int nits = int( 20e6 / nfft );
   for( int k = 0; k < nits; ++k ) {
      DFT( inbuf.data(), outbuf.data(), buf.data(), opts, T( 1 ) );
   }
   auto t1 = std::chrono::steady_clock::now();
   double time = ( std::chrono::duration< float >( t1 - t0 ) ).count();
   std::cout << " MSPS:" << ( ( nits * nfft ) * 1e-6 / time ) << std::endl; // Million samples per second
}

int main( int argc, char** argv ) {
   if( argc > 1 ) {
      for( int k = 1; k < argc; ++k ) {
         size_t nfft = std::strtoul( argv[ k ], nullptr, 10 );
         dotest< float >( nfft );
         dotest< double >( nfft );
      }
      /*
      size_t N = 10000;
      std::vector< std::complex< double>> inbuf( N );
      for( size_t k = 0; k < N; ++k ) {
         inbuf[ k ] = std::complex< double >( std::rand(), std::rand() ) / ( double )RAND_MAX - 0.5;
      }

      std::complex< double > c2 = { 0, 0 };
      auto t0 = std::chrono::steady_clock::now();
      for( size_t i = 0; i < 10000; ++i ) {
         for( size_t j = 0; j < N - 1; ++j ) {
            double re = inbuf[ j ].real() * inbuf[ j + 1 ].real() - inbuf[ j ].imag() * inbuf[ j + 1 ].imag();
            double im = inbuf[ j ].real() * inbuf[ j + 1 ].imag() + inbuf[ j ].imag() * inbuf[ j + 1 ].real();
            c2.real( c2.real() + re );
            c2.imag( c2.imag() + im );
         }
      }
      auto t1 = std::chrono::steady_clock::now();
      double time = ( std::chrono::duration< float >( t1 - t0 ) ).count();
      std::cout << c2 << " using manual *:" << time << std::endl;

      c2 = { 0, 0 };
      t0 = std::chrono::steady_clock::now();
      for( size_t i = 0; i < 10000; ++i ) {
         for( size_t j = 0; j < N - 1; ++j ) {
            c2 += inbuf[ j ] * inbuf[ j + 1 ];
         }
      }
      t1 = std::chrono::steady_clock::now();
      time = ( std::chrono::duration< float >( t1 - t0 ) ).count();
      std::cout << c2 << " using stdlib *:" << time << std::endl;
      */
   } else {
      dotest< float >( 32 );
      dotest< double >( 32 );
      dotest< float >( 1024 );
      dotest< double >( 1024 );
      dotest< float >( 1152 );
      dotest< double >( 1152 );
      dotest< float >( 840 );
      dotest< double >( 840 );
      dotest< float >( 1023 );
      dotest< double >( 1023 );
   }
   return 0;
}
