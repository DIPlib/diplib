/*
 * (c)2017, Erik Schuitema.
 * (c)2017-2024, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib/dft.h"

#include <cmath>
#include <complex>
#include <limits>
#include <mutex>
#include <vector>

#include "diplib.h"

#ifdef DIP_CONFIG_HAS_FFTW

#ifdef _WIN32
#define NOMINMAX // windows.h must not define min() and max(), which conflict with std::min() and std::max()
#endif
#include "fftw3api.h"

#else // DIP_CONFIG_HAS_FFTW

#include "diplib/private/robin_map.h"

#if defined(__GNUG__) || defined(__clang__)
// Temporarily turn off -Wconversion, PocketFFT otherwise produces two warnings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#define POCKETFFT_NO_VECTORS // is faster on my M1 Mac!
#define POCKETFFT_NO_MULTITHREADING
#include "pocketfft_hdronly.h"

namespace pocketfft { // we want these in the main namespace
using detail::cmplx;
using detail::pocketfft_c;
using detail::pocketfft_r;
}

#if defined(__GNUG__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // DIP_CONFIG_HAS_FFTW


namespace dip {


#ifdef DIP_CONFIG_HAS_FFTW

dip::uint const maximumDFTSize = static_cast< dip::uint >( std::numeric_limits< int >::max() );
bool const usingFFTW = true;

template< typename T >
using complex = typename fftwapidef< T >::complex;

template< typename T >
using CPlan = typename fftwapidef< T >::plan;

template< typename T >
using RPlan = typename fftwapidef< T >::plan;

#else // DIP_CONFIG_HAS_FFTW

dip::uint const maximumDFTSize = std::numeric_limits< dip::uint >::max();
bool const usingFFTW = false;

template< typename T >
using complex = pocketfft::cmplx< T >;

template< typename T >
using CPlan = pocketfft::pocketfft_c< T >;

template< typename T >
using RPlan = pocketfft::pocketfft_r< T >;

namespace {

std::mutex planCacheMutex;

// This is a container that holds plans, and maintains a use count. Plans in use cannot be
// deleted.
//
// This code is developed based on pocketfft::detail::get_plan<>(), but modified beyond recognition.
//
// PlanType is CPlan< T > or RPlan< T >, T is dip::sfloat or dip::dfloat. There are 4 caches.
template< typename PlanType >
PlanType* PlanCache( dip::uint length, bool free = false ) {
   std::lock_guard< std::mutex > guard( planCacheMutex );

   struct Data {
      dip::uint use_count;
      dip::uint last_access;
      std::unique_ptr< PlanType > plan;
   };
   static tsl::robin_map< dip::uint, Data > cache;
   static dip::uint access_counter = 0;
   constexpr dip::uint maxSize = 30; // If there are more than maxSize plans, delete ones not in use.

   if( free ) {
      auto it = cache.find( length );
      if( it != cache.end()) {
         auto& data = it.value();
         if( data.use_count > 0 ) {
            --( data.use_count );
            //std::cout << "Freeing cache element for length = " << length;
            //std::cout << ", current use count = " << data.use_count << '\n';
         }
      }
      return nullptr;
   }

   if( access_counter == std::numeric_limits< dip::uint >::max()) {
      // This could happen... but really???
      access_counter = 1;
      // Update `last_access` from all plans to be 0, so that things make sense again.
      for( auto& p : cache ) {
         const_cast< Data& >( p.second ).last_access = 0; // Why is the data const-qualified here?
      }
   }
   auto it = cache.find( length );
   if( it != cache.end()) {
      // Update
      //std::cout << "Re-using cache element for length = " << length;
      auto& data = it.value();
      ++( data.use_count );
      data.last_access = ++access_counter;
      //std::cout << ", current use count = " << data.use_count << '\n';
      return data.plan.get();
   }
   // Not found
   if( cache.size() >= maxSize ) {
      //std::cout << "Cache is getting big... ";
      // Delete one cache element that is not in use, if possible
      dip::uint acc = access_counter;
      dip::uint len = 0;
      for( auto& p : cache ) {
         if( p.second.use_count == 0 ) { // not in use
            if( p.second.last_access < acc ) {
               acc = p.second.last_access;
               len = p.second.plan->length();
            }
         }
      }
      // If there is something to delete, we will
      if( len > 0 ) {
         //std::cout << "Deleting cache element for length = " << len << '\n';
         cache.erase( len );
      } else {
         //std::cout << "Nothing found to delete\n";
      }
   }
   //std::cout << "Creating cache element for length = " << length << '\n';
   Data data{ 1, ++access_counter, std::make_unique< PlanType >( length ) };
   cache.insert( { length, std::move( data ) } );
   return cache[ length ].plan.get();
}

} // namespace

#endif // DIP_CONFIG_HAS_FFTW


//--- dip::DFT implementation ---

namespace {

constexpr char const* DFT_NO_PLAN = "No plan defined";

} // namespace

template< typename T >
void DFT< T >::Initialize( dip::uint size, bool inverse, Option::DFTOptions options ) {
   Destroy();
   nfft_ = size;
   inverse_ = inverse;
   options_ = options;
#ifdef DIP_CONFIG_HAS_FFTW
   int sign = inverse ? FFTW_BACKWARD : FFTW_FORWARD;
   unsigned flags = FFTW_MEASURE; // FFTW_MEASURE is almost always faster than FFTW_ESTIMATE, only for very trivial sizes it's not.
   if( !options_.Contains( Option::DFTOption::Aligned )) {
      flags |= FFTW_UNALIGNED;
   }
   AlignedBuffer inBuffer( size * sizeof( complex< T > )); // allocate temporary arrays just for planning...
   complex< T >* in = reinterpret_cast< complex< T >* >( inBuffer.data() );
   if( options_.Contains( Option::DFTOption::InPlace )) {
      plan_ = fftwapidef< T >::plan_dft_1d( static_cast< int >( size ), in, in, sign, flags );
   } else {
      flags |= options_.Contains( Option::DFTOption::TrashInput ) ? FFTW_DESTROY_INPUT : FFTW_PRESERVE_INPUT;
      AlignedBuffer outBuffer( size * sizeof( complex< T > )); // allocate temporary arrays just for planning...
      complex< T >* out = reinterpret_cast< complex< T >* >( outBuffer.data() );
      plan_ = fftwapidef< T >::plan_dft_1d( static_cast< int >( size ), in, out, sign, flags );
   }
#else // DIP_CONFIG_HAS_FFTW
   ( void ) options; // parameter ignored
   plan_ = PlanCache< CPlan< T >>( size );
#endif // DIP_CONFIG_HAS_FFTW
}

template void DFT< dfloat >::Initialize( dip::uint, bool, Option::DFTOptions );
template void DFT< sfloat >::Initialize( dip::uint, bool, Option::DFTOptions );


template< typename T >
void DFT< T >::Apply(
      std::complex< T >* source,
      std::complex< T >* destination,
      T scale
) const {
   DIP_THROW_IF( !plan_, DFT_NO_PLAN );
#ifdef DIP_CONFIG_HAS_FFTW
   DIP_ASSERT(( source == destination) ^ !options_.Contains( Option::DFTOption::InPlace ));
   fftwapidef< T >::execute_dft( static_cast< CPlan< T >>( plan_ ),
                                 reinterpret_cast< complex< T >* >( source ),
                                 reinterpret_cast< complex< T >* >( destination ));
   if( scale != 1.0 ) {
      for( std::complex< T >* ptr = destination; ptr < destination + nfft_; ++ptr ) {
         *ptr *= scale;
      }
   }
#else // DIP_CONFIG_HAS_FFTW
   if( destination != source ) {
      std::copy_n( source, nfft_, destination );
   }
   static_cast< CPlan< T >* >( plan_ )->exec( reinterpret_cast< complex< T >* >( destination ), scale, !inverse_ );
#endif // DIP_CONFIG_HAS_FFTW
}

template void DFT< dfloat >::Apply( std::complex< dfloat >*, std::complex< dfloat >*, dfloat ) const;
template void DFT< sfloat >::Apply( std::complex< sfloat >*, std::complex< sfloat >*, sfloat ) const;


template< typename T >
void DFT< T >::Destroy() {
   if( plan_ ) {
#ifdef DIP_CONFIG_HAS_FFTW
      fftwapidef< T >::destroy_plan( static_cast< CPlan< T >>( plan_ ));
#else // DIP_CONFIG_HAS_FFTW
      PlanCache< CPlan< T >>( nfft_, true );
#endif // DIP_CONFIG_HAS_FFTW
      plan_ = nullptr;
   }
}

template void DFT< dfloat >::Destroy();
template void DFT< sfloat >::Destroy();


// --- dip::RDFT implementation ---


template< typename T >
void RDFT< T >::Initialize( dip::uint size, bool inverse, Option::DFTOptions options ) {
   Destroy();
   nfft_ = size;
   inverse_ = inverse;
   options_ = options;
#ifdef DIP_CONFIG_HAS_FFTW
   unsigned flags = FFTW_MEASURE; // FFTW_MEASURE is almost always faster than FFTW_ESTIMATE, only for very trivial sizes it's not.
   if( !options_.Contains( Option::DFTOption::Aligned )) {
      flags |= FFTW_UNALIGNED;
   }
   AlignedBuffer firstBuffer(( size / 2 + 1 ) * 2 * sizeof( T )); // allocate temporary arrays just for planning...
   if( options_.Contains( Option::DFTOption::InPlace )) {
      T* real = reinterpret_cast< T* >( firstBuffer.data() );
      complex< T >* comp = reinterpret_cast< complex< T >* >( firstBuffer.data() );
      if( inverse_ ) {
         plan_ = fftwapidef< T >::plan_dft_c2r_1d( static_cast< int >( size ), comp, real, flags );
      } else {
         plan_ = fftwapidef< T >::plan_dft_r2c_1d( static_cast< int >( size ), real, comp, flags );
      }
   } else {
      flags |= options_.Contains( Option::DFTOption::TrashInput ) ? FFTW_DESTROY_INPUT : FFTW_PRESERVE_INPUT;
      AlignedBuffer secondBuffer( size * sizeof( T )); // allocate temporary arrays just for planning...
      T* real = reinterpret_cast< T* >( secondBuffer.data() );
      complex< T >* comp = reinterpret_cast< complex< T >* >( firstBuffer.data() );
      if( inverse_ ) {
         plan_ = fftwapidef< T >::plan_dft_c2r_1d( static_cast< int >( size ), comp, real, flags );
      } else {
         plan_ = fftwapidef< T >::plan_dft_r2c_1d( static_cast< int >( size ), real, comp, flags );
      }
   }
#else // DIP_CONFIG_HAS_FFTW
   plan_ = PlanCache< RPlan< T >>( size );
#endif // DIP_CONFIG_HAS_FFTW
}

template void RDFT< dfloat >::Initialize( dip::uint, bool, Option::DFTOptions );
template void RDFT< sfloat >::Initialize( dip::uint, bool, Option::DFTOptions );


template< typename T >
void RDFT< T >::Apply(
      T* source,
      T* destination,
      T scale
) const {
   DIP_THROW_IF( !plan_, DFT_NO_PLAN );
#ifdef DIP_CONFIG_HAS_FFTW
   dip::uint len = nfft_;
   if( inverse_ ) {
      fftwapidef< T >::execute_dft_c2r( static_cast< RPlan< T >>( plan_ ),
                                        reinterpret_cast< complex< T >* >( source ),
                                        destination );
   } else {
      fftwapidef< T >::execute_dft_r2c( static_cast< RPlan< T >>( plan_ ),
                                        source,
                                        reinterpret_cast< complex< T >* >( destination ));
      len = 2 * ( nfft_ / 2 + 1 );
   }
   if( scale != 1.0 ) {
      for( T* ptr = destination; ptr < destination + len; ++ptr ) {
         *ptr *= scale;
      }
   }
#else // DIP_CONFIG_HAS_FFTW
   if( inverse_ ) {
      // Coerce the complex input data into the form expected by PocketFFT: the first (and last if nfft_ is even) elements only have a real value
      destination[ 0 ] = source[ 0 ];
      std::copy_n( source + 2, nfft_ - 1, destination + 1 ); // should work if source==destination
   } else {
      if( source != destination ) {
         // Copy the real input data into the complex output array, which should always have enough space
         std::copy_n( source, nfft_, destination );
      }
   }
   static_cast< RPlan< T >* >( plan_ )->exec( destination, scale, !inverse_ );
   if( !inverse_ ) {
      // Translate the complex output into proper complex numbers.
      dip::uint n = 2 * ( nfft_ / 2 + 1 ); // number of values on output array
      destination[ n - 1 ] = 0;
      if(( nfft_ & 1u ) == 0 ) {
         --n;
      }
      std::copy_backward( destination + 1, destination + nfft_, destination + n );
      destination[ 1 ] = 0;
   }
#endif // DIP_CONFIG_HAS_FFTW
}

template void RDFT< dfloat >::Apply( dfloat*, dfloat*, dfloat ) const;
template void RDFT< sfloat >::Apply( sfloat*, sfloat*, sfloat ) const;


template< typename T >
void RDFT< T >::Destroy() {
   if( plan_ ) {
#ifdef DIP_CONFIG_HAS_FFTW
      fftwapidef< T >::destroy_plan( static_cast< RPlan< T >>( plan_ ));
#else // DIP_CONFIG_HAS_FFTW
      PlanCache< RPlan< T >>( nfft_, true );
#endif // DIP_CONFIG_HAS_FFTW
      plan_ = nullptr;
   }
}

template void RDFT< dfloat >::Destroy();
template void RDFT< sfloat >::Destroy();

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/random.h"

constexpr long double pi = 3.1415926535897932384626433832795029l;

template< typename T >
T test_DFT( dip::uint nfft, bool inverse ) {
   // Initialize
   dip::DFT< T > opts( nfft, inverse );
   // Create test data
   std::vector< std::complex< T >> inbuf( nfft );
   std::vector< std::complex< T >> outbuf( nfft );
   dip::Random random;
   for( dip::uint k = 0; k < nfft; ++k ) {
      inbuf[ k ] = std::complex< T >( static_cast< T >( random() ), static_cast< T >( random() )) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( inbuf.data(), outbuf.data(), T( 1 ));
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( dip::uint k0 = 0; k0 < nfft; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = ( inverse ? 2 : -2 ) * static_cast< long double >( k0 ) * pi / static_cast< long double >( nfft );
      for( dip::uint k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) * std::exp( std::complex< long double >( 0, static_cast< long double >( k1 ) * phinc ));
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ));
   }
   return static_cast< T >( std::sqrt( difpower / totalpower )); // Root mean square error
}

DOCTEST_TEST_CASE("[DIPlib] testing the DFT class") {
   // Test forward (C2C) transform for a few different sizes that have all different radixes.
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 32, false )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 32, false )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 256, false )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 105, false )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 154, false )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 97, false )) == 0 ); // prime
   // Idem for inverse (C2C) transform
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 32, true )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 32, true )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 256, true )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 105, true )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::dfloat >( 154, true )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( test_DFT< dip::sfloat >( 97, true )) == 0 ); // prime
}

template< typename T >
T test_RDFT( dip::uint nfft ) {
   // Initialize
   dip::RDFT< T > opts( nfft, false );
   dip::uint nOut = nfft / 2 + 1;
   // Create test data
   std::vector< T > inbuf( nfft );
   std::vector< std::complex< T >> outbuf( nOut + 1 );
   outbuf.back() = 1e6; // make sure we're not reading past where we're supposed to
   dip::Random random;
   for( dip::uint k = 0; k < nfft; ++k ) {
      inbuf[ k ] = static_cast< T >( random() ) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( inbuf.data(), reinterpret_cast< T* >( outbuf.data() ), T( 1 ));
   DIP_THROW_IF( outbuf.back() != T( 1e6 ), "dip::RDFT< T >::Apply wrote too many values!" );
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( dip::uint k0 = 0; k0 < nOut; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = -2 * static_cast< long double >( k0 ) * pi / static_cast< long double >( nfft );
      for( dip::uint k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) * std::exp( std::complex< long double >( 0, static_cast< long double >( k1 ) * phinc ));
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ));
   }
   return static_cast< T >( std::sqrt( difpower / totalpower )); // Root mean square error
}

template< typename T >
T test_RDFTi( dip::uint nfft ) {
   // Initialize
   dip::RDFT< T > opts( nfft, true );
   dip::uint nIn = nfft / 2 + 1;
   // Create test data
   std::vector< std::complex< T >> inbuf( nIn + 1 );
   std::vector< T > outbuf( nfft );
   dip::Random random;
   inbuf.back() = 1e6; // Make sure we're not reading past where we're supposed to
   inbuf[ 0 ] = static_cast< T >( random() ) / static_cast< T >( random.max() ) - T( 0.5 );
   if(( nfft & 1u ) == 0 ) {
      --nIn;
      inbuf[ nIn ] = static_cast< T >( random() ) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   for( dip::uint k = 1; k < nIn; ++k ) {
      inbuf[ k ] = std::complex< T >( static_cast< T >( random() ), static_cast< T >( random() )) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( reinterpret_cast< T* >( inbuf.data() ), outbuf.data(), T( 1 ));
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( dip::uint k0 = 0; k0 < nfft; ++k0 ) {
      long double phinc = 2 * static_cast< long double >( k0 ) * pi / static_cast< long double >( nfft );
      //long double acc = inbuf[ 0 ].real();
      std::complex< long double > acc = inbuf[ 0 ];
      for( dip::uint k1 = 1; k1 < nIn; ++k1 ) {
         //acc += 2 * ( std::complex< long double >( inbuf[ k1 ] ) * std::exp( std::complex< long double >( 0, static_cast< long double >( k1 ) * phinc )) ).real();
         acc += std::complex< long double >( inbuf[ k1 ] ) * std::exp( std::complex< long double >( 0, static_cast< long double >( k1 ) * phinc ));
         acc += std::conj( std::complex< long double >( inbuf[ k1 ] )) * std::exp( std::complex< long double >( 0, -static_cast< long double >( k1 ) * phinc ));
      }
      if(( nfft & 1u ) == 0 ) {
         acc += ( std::complex< long double >( inbuf[ nIn ] ) * std::exp( std::complex< long double >( 0, static_cast< long double >( nIn ) * phinc )) ).real();
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - static_cast< long double >( outbuf[ k0 ] ));
   }
   return static_cast< T >( std::sqrt( difpower / totalpower )); // Root mean square error
}

DOCTEST_TEST_CASE("[DIPlib] testing the RDFT class") {
   // Test forward (R2C) transform for a few different sizes that have all different radixes.
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::sfloat >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::dfloat >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::dfloat >( 256 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::sfloat >( 105 )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::dfloat >( 154 )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( test_RDFT< dip::sfloat >( 97 )) == 0 ); // prime
   // Idem for inverse (C2R) transform
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::sfloat >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::dfloat >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::dfloat >( 256 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::sfloat >( 105 )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::dfloat >( 154 )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( test_RDFTi< dip::sfloat >( 97 )) == 0 ); // prime
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
