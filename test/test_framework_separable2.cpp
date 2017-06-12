#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <numeric>
#include <random>
#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/linear.h"
#include "diplib/statistics.h"
#include "diplib/timer.h"

// Timing the separable convolution

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 200, 50, 30 }, 1, dip::DT_UINT16 };
      {
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         std::mt19937 gen;
         std::normal_distribution< float > normDist( 9563.0, 500.0 );
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      // General

      dip::Image out1;
      dip::OneDimensionalFilterArray filterArray( 1 );
      filterArray[ 0 ].filter = {
            1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
            6.0 / 49.0, 5.0 / 49.0, 4.0 / 49.0, 3.0 / 49.0, 2.0 / 49.0, 1.0 / 49.0
      };
      filterArray[ 0 ].origin = 0;
      filterArray[ 0 ].symmetry = "general";

      dip::Timer timer;
      dip::SeparableConvolution( img, out1, filterArray );
      timer.Stop();
      std::cout << "General: " << timer << std::endl;

      dip::Image out2;
      filterArray[ 0 ].filter = {
            1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
      };
      filterArray[ 0 ].symmetry = "even";

      timer.Reset();
      dip::SeparableConvolution( img, out2, filterArray );
      timer.Stop();
      std::cout << "Even: " << timer << std::endl;

      if( dip::Count( out1 != out2 ) > 0 ) {
         std::cout << "Results are not identical.";
      }

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
