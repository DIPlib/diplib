// An example of catastrophic cancellation in dip::FastVarianceAccumulator
// The timing measurements are not precise, accumulating 1M values is really fast either way.
// Nonetheless, the large speed difference between the two accumulators is clear.

#include "diplib/accumulators.h"
#include "diplib/testing.h"

int main() {

   constexpr dip::uint N = 1000000;
   constexpr dip::dfloat value1 = 1.0e9;
   constexpr dip::dfloat value2 = 1.0001e9;

   dip::VarianceAccumulator acc1;
   dip::testing::Timer timer;
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc1.Push( value1 );
   }
   acc1.Push( value2 );
   timer.Stop();
   std::cout << "acc1 mean = " << acc1.Mean() << ", var = " << acc1.Variance() << "; " << timer << std::endl;

   dip::FastVarianceAccumulator acc2;
   timer.Reset();
   for( dip::uint ii = 0; ii < N; ++ii ) {
      acc2.Push( value1 );
   }
   acc2.Push( value2 );
   timer.Stop();
   std::cout << "acc2 mean = " << acc2.Mean() << ", var = " << acc2.Variance() << "; " << timer << std::endl;

   return 0;
}
