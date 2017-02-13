#define DOCTEST_CONFIG_IMPLEMENT

#include <chrono>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"
#include "diplib/math.h"

// NOTE! As a timing test, remember to compile with
//     cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DOCTEST=OFF ..

template< class TPI, class F >
std::unique_ptr< dip::Framework::ScanLineFilter > NewFilter( F func ) {
   return static_cast< std::unique_ptr< dip::Framework::ScanLineFilter >>( new dip::Framework::NadicScanLineFilter< 1, TPI, F >( func ));
}

int main( void ) {
   dip::Image in1( { 5000, 4000 }, 3, dip::DT_UINT16 );
   in1 = 10;
   dip::Image in2( { 5000, 4000 }, 3, dip::DT_UINT8 );
   in2 = 40;
   dip::DataType dt = dip::DataType::SuggestArithmetic( in1.DataType(), in2.DataType() );
   dip::Image out = in1.Similar( dt );
   out = 0; // initialize memory, forcing it to be available

   dip::dfloat offset = 40;

   // Dyadic, timing comparison with `Add`. (Note that this was relevant before we rewrote `Add` to work exactly like below.)

   auto start = std::chrono::steady_clock::now();
   dip::Add( in1, in2, out, dt );
   std::cout << "Add: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   start = std::chrono::steady_clock::now();
   std::unique_ptr< dip::Framework::ScanLineFilter > dyadicScanLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( dyadicScanLineFilter, dip::Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_add( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   dip::Framework::ScanDyadic( in1, in2, out, dt, dt, *dyadicScanLineFilter );
   std::cout << "dyadicScanLineFilter: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   start = std::chrono::steady_clock::now();
   dip::Add( in1, in2, out, dt );
   std::cout << "Add: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   // Complex dyadic, following example in documentation to `class NadicScanLineFilter`

   start = std::chrono::steady_clock::now();
   auto sampleOperator = [ = ]( std::array< dip::sfloat const*, 2 > its ) { return ( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + offset; };
   dip::Framework::NadicScanLineFilter< 2, dip::sfloat, decltype( sampleOperator ) > scanLineFilter( sampleOperator );
   dip::Framework::ScanDyadic( in1, in2, out, dip::DT_SFLOAT, dip::DT_SFLOAT, scanLineFilter );
   std::cout << "scanLineFilter: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   // idem, but with dynamic dispatch

   start = std::chrono::steady_clock::now();
   std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter2;
   DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter2, dip::Framework::NewDyadicScanLineFilter, (
         [ = ]( auto its ) { return ( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + offset; }
   ), dt );
   dip::Framework::ScanDyadic( in1, in2, out, dt, dt, *scanLineFilter2 );
   std::cout << "scanLineFilter2: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   // Trivial implementation of the same

   {
      dip::Image tmp_in1 = in1; tmp_in1.TensorToSpatial( 0 );
      dip::Image tmp_in2 = in2; tmp_in2.TensorToSpatial( 0 );
      dip::Image tmp_out = out; tmp_out.TensorToSpatial( 0 );
      start = std::chrono::steady_clock::now();
      dip::Image tmp = ( tmp_in1 * 100 ) / ( tmp_in2 * 10 ) + offset;
      std::cout << "trivial version: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;
      if( dip::Count( tmp_out == tmp ) > 0 ) {
         std::cout << "results are not identical!\n";
      }

      start = std::chrono::steady_clock::now();
      tmp_in1 *= 100; // this modifies in1 and in2 also...
      tmp_in2 *= 10;
      tmp_in1 /= tmp_in2;
      tmp_in1 += offset;
      std::cout << "efficient trivial version: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;
      if( dip::Count( tmp_in1 == tmp ) > 0 ) {
         std::cout << "results are not identical!\n";
      }
      if(( tmp_in1.Origin() != in1.Origin() ) || ( tmp_in2.Origin() != in2.Origin() )) {
         std::cout << "images were copied!?\n";
      }
   }

   // Monadic, following example in documentation to `class NadicScanLineFilter`

   start = std::chrono::steady_clock::now();
   std::unique_ptr< dip::Framework::ScanLineFilter > monfilt;
   DIP_OVL_CALL_ASSIGN_REAL( monfilt, NewFilter, (
         [ = ]( auto its ) { return ( std::cos( *its[ 0 ] ) * 100 ) + offset; }
   ), dt );
   dip::Framework::ScanMonadic( in1, out, dt, dt, in1.TensorElements(), *monfilt, dip::Framework::Scan_TensorAsSpatialDim );
   std::cout << "monfilt: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

   return 0;
}
