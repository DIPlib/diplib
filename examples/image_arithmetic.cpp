/*
 * This example program shows different ways to do image arithmetic, and times them to show the efficiency
 * differences. The "line filter" approaches have the advantage of only traversing the image(s) once, performing
 * more complicated operations. The "trivial versions" traverse the image(s) multiple times, each time
 * performing a single operation. The "iterator" approach is a compromise between the two. The image is traversed
 * only once, but without multi-threading, and with pre-determined input and output data types.
 *
 * Pick the version that offers the best compromise between ease of implementation and run-time performance for
 * your specific application.
 *
 * NOTE! As a timing test, remember to compile with
 *     cmake -DCMAKE_BUILD_TYPE=Release -DDIP_ENABLE_ASSERT=Off ..
 */

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"
#include "diplib/saturated_arithmetic.h"
#include "diplib/math.h"
#include "diplib/testing.h"

template< class TPI, class F >
std::unique_ptr< dip::Framework::ScanLineFilter > NewFilter( F func ) {
   return static_cast< std::unique_ptr< dip::Framework::ScanLineFilter >>( new dip::Framework::VariadicScanLineFilter< 1, TPI, F >( func ));
}

int main( void ) {

   // Two large input images to do the computations on

   dip::Image in1( { 5000, 4000 }, 1, dip::DT_SFLOAT );
   in1 = 10;
   dip::Image in2( { 5000, 4000 }, 1, dip::DT_SFLOAT );
   in2 = 40;
   dip::DataType dt = dip::DataType::SuggestArithmetic( in1.DataType(), in2.DataType());
   dip::Image out = in1.Similar( dt );
   out = 0; // initialize memory, forcing it to be available
   dip::Image tmp = in1.Similar( dt );
   tmp = 0; // initialize memory, forcing it to be available

   dip::sfloat offset = 40;

   // Complex dyadic, following example in documentation to `class VariadicScanLineFilter`. Does the computation
   // using SFLOAT type, input images are converted to that type for computation.

   dip::testing::Timer timer;
   auto sampleOperator = [ = ]( std::array< dip::sfloat const*, 2 > its ) {
      return ( decltype( *its[ 0 ] ))(( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + decltype( *its[ 0 ] )( offset ));
   };
   dip::Framework::VariadicScanLineFilter< 2, dip::sfloat, decltype( sampleOperator ) > diadicLineFilter( sampleOperator );
   dip::Framework::ScanDyadic( in1, in2, out, dip::DT_SFLOAT, dip::DT_SFLOAT, diadicLineFilter );
   timer.Stop();
   std::cout << "diadicLineFilter: " << timer << std::endl;

   // Idem, but with dynamic dispatch (i.e. does the computation in data type `dt`)

   timer.Reset();
   std::unique_ptr< dip::Framework::ScanLineFilter > diadicLineFilter2;
   DIP_OVL_CALL_ASSIGN_REAL( diadicLineFilter2, dip::Framework::NewDyadicScanLineFilter, (
         [ = ]( auto its ) { return ( decltype( *its[ 0 ] ))(( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + decltype( *its[ 0 ] )( offset )); }
   ), dt );
   dip::Framework::ScanDyadic( in1, in2, tmp, dt, dt, *diadicLineFilter2 );
   timer.Stop();
   std::cout << "diadicLineFilter2: " << timer << std::endl;
   dip::testing::CompareImages( out, tmp );

   // Implementation using an iterator. Does the computation in SFLOAT type.

   tmp = 0; // reset to show we're really doing the computation
   timer.Reset();
   dip::JointImageIterator< dip::sfloat, dip::sfloat, dip::sfloat > it( { in1, in2, tmp } );
   it.OptimizeAndFlatten();
   do {
      it.Sample< 2 >() = ( it.Sample< 0 >() * 100.0f ) / ( it.Sample< 1 >() * 10.0f ) + offset;
      // Note that for tensor images, it is necessary to iterate over tensor elements here.
      // The ScanDyadic function takes care of that, applying the same operation to each of the tensor elements.
   } while( ++it );
   timer.Stop();
   std::cout << "JointImageIterator: " << timer << std::endl;
   dip::testing::CompareImages( out, tmp );

   // Trivial implementation of the same, data-type agnostic.

   tmp = 0; // reset to show we're really doing the computation
   timer.Reset();
   tmp = ( in1 * 100.0f ) / ( in2 * 10.0f ) + offset;
   // Note that we use `100.0f` here, not `100`, as that leads to a sint32 image, which turns computation results into doubles
   timer.Stop();
   std::cout << "trivial version: " << timer << std::endl;
   dip::testing::CompareImages( out, tmp );

   dip::Image tmp_in1; tmp_in1.Copy( in1 ); // Copy in1 and in2 so we can modify them below
   dip::Image tmp_in2; tmp_in2.Copy( in2 );

   timer.Reset();
   tmp_in1 *= 100;
   tmp_in2 *= 10;
   tmp_in1 /= tmp_in2;
   tmp_in1 += offset;
   timer.Stop();
   std::cout << "efficient trivial version: " << timer << std::endl;
   dip::testing::CompareImages( out, tmp_in1 );

   // Monadic, following example in documentation to `class VariadicScanLineFilter`

   std::cout << std::endl;

   timer.Reset();
   std::unique_ptr< dip::Framework::ScanLineFilter > monadicLineFilter;
   DIP_OVL_CALL_ASSIGN_REAL( monadicLineFilter, NewFilter, (
         [ = ]( auto its ) { return ( decltype( *its[ 0 ] ))(( std::cos( *its[ 0 ] ) * 100 ) + offset ); }
   ), dt );
   dip::Framework::ScanMonadic( in1, out, dt, dt, in1.TensorElements(), *monadicLineFilter, dip::Framework::ScanOption::TensorAsSpatialDim );
   timer.Stop();
   std::cout << "monadicLineFilter: " << timer << std::endl;

   timer.Reset();
   tmp = ( dip::Cos( in1 ) * 100.0f ) + offset;
   // Note that we use `100.0f` here, not `100`, as that leads to a sint32 image, which turns computation results into doubles
   timer.Stop();
   std::cout << "trivial version: " << timer << std::endl;
   dip::testing::CompareImages( out, tmp );

   return 0;
}
