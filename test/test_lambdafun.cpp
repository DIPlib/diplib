#define DOCTEST_CONFIG_IMPLEMENT

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"
#include "diplib/statistics.h"
#include "diplib/testing.h"

// NOTE! As a timing test, remember to compile with
//     cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DOCTEST=OFF ..

template< class TPI, class F >
std::unique_ptr< dip::Framework::ScanLineFilter > NewFilter( F func ) {
   return static_cast< std::unique_ptr< dip::Framework::ScanLineFilter >>( new dip::Framework::VariadicScanLineFilter< 1, TPI, F >( func ));
}

int main( void ) {
   dip::Image in1( { 5000, 4000 }, 3, dip::DT_SFLOAT );
   in1 = 10;
   dip::Image in2( { 5000, 4000 }, 3, dip::DT_SFLOAT );
   in2 = 40;
   dip::DataType dt = dip::DataType::SuggestArithmetic( in1.DataType(), in2.DataType());
   dip::Image out = in1.Similar( dt );
   out = 0; // initialize memory, forcing it to be available

   dip::sfloat offset = 40;

   // Dyadic, timing comparison with `Add`. (Note that this was relevant before we rewrote `Add` to work exactly like below.)

   dip::testing::Timer timer;
   dip::Add( in1, in2, out, dt );
   timer.Stop();
   std::cout << "Add: " << timer << std::endl;

   timer.Reset();
   std::unique_ptr< dip::Framework::ScanLineFilter > dyadicScanLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( dyadicScanLineFilter, dip::Framework::NewDyadicScanLineFilter, (
         []( auto its ) { return dip::saturated_add( *its[ 0 ], *its[ 1 ] ); }
   ), dt );
   dip::Framework::ScanDyadic( in1, in2, out, dt, dt, *dyadicScanLineFilter );
   timer.Stop();
   std::cout << "dyadicScanLineFilter: " << timer << std::endl;

   timer.Reset();
   dip::Add( in1, in2, out, dt );
   timer.Stop();
   std::cout << "Add: " << timer << std::endl;

   // Complex dyadic, following example in documentation to `class VariadicScanLineFilter`

   timer.Reset();
   auto sampleOperator = [ = ]( std::array< dip::sfloat const*, 2 > its ) {
      return ( decltype( *its[ 0 ] ))(( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + decltype( *its[ 0 ] )( offset ));
   };
   dip::Framework::VariadicScanLineFilter< 2, dip::sfloat, decltype( sampleOperator ) > scanLineFilter( sampleOperator );
   dip::Framework::ScanDyadic( in1, in2, out, dip::DT_SFLOAT, dip::DT_SFLOAT, scanLineFilter );
   timer.Stop();
   std::cout << "scanLineFilter: " << timer << std::endl;

   // idem, but with dynamic dispatch

   timer.Reset();
   std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter2;
   DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter2, dip::Framework::NewDyadicScanLineFilter, (
         [ = ]( auto its ) { return ( decltype( *its[ 0 ] ))(( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + decltype( *its[ 0 ] )( offset )); }
   ), dt );
   dip::Framework::ScanDyadic( in1, in2, out, dt, dt, *scanLineFilter2 );
   timer.Stop();
   std::cout << "scanLineFilter2: " << timer << std::endl;

   // Trivial implementation of the same

   {
      dip::Image tmp_in1 = in1; tmp_in1.TensorToSpatial( 0 );
      dip::Image tmp_in2 = in2; tmp_in2.TensorToSpatial( 0 );
      dip::Image tmp_out = out; tmp_out.TensorToSpatial( 0 );
      timer.Reset();
      dip::Image tmp = ( tmp_in1 * 100.0f ) / ( tmp_in2 * 10.0f ) + offset;
      // Note that we use `100.0f` here, not `100`, as that leads to a sint32 image, which turns computation results into doubles
      timer.Stop();
      std::cout << "trivial version: " << timer << std::endl;
      if( dip::Count( tmp_out != tmp ) > 0 ) {
         std::cout << "results are not identical!\n";
      }

      timer.Reset();
      tmp_in1 *= 100; // this modifies in1 and in2 also...
      tmp_in2 *= 10;
      tmp_in1 /= tmp_in2;
      tmp_in1 += offset;
      timer.Stop();
      std::cout << "efficient trivial version: " << timer << std::endl;
      if( dip::Count( tmp_in1 != tmp_out ) > 0 ) {
         std::cout << "   results are not identical!\n"; // Note that the result here is UINT16, which can't represent the result
      }
      if(( tmp_in1.Origin() != in1.Origin()) || ( tmp_in2.Origin() != in2.Origin())) {
         std::cout << "   images were copied!?\n";
      }
   }

   // Monadic, following example in documentation to `class VariadicScanLineFilter`

   timer.Reset();
   std::unique_ptr< dip::Framework::ScanLineFilter > monfilt;
   DIP_OVL_CALL_ASSIGN_REAL( monfilt, NewFilter, (
         [ = ]( auto its ) { return ( decltype( *its[ 0 ] ))(( std::cos( *its[ 0 ] ) * 100 ) + offset ); }
   ), dt );
   dip::Framework::ScanMonadic( in1, out, dt, dt, in1.TensorElements(), *monfilt, dip::Framework::Scan_TensorAsSpatialDim );
   timer.Stop();
   std::cout << "monfilt: " << timer << std::endl;

   return 0;
}
