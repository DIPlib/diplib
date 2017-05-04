#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <numeric>
#include <random>
#include "diplib.h"
#include "diplib/morphology.h"

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      {
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         std::mt19937 gen( 0 );
         std::normal_distribution< float > normDist( 9563.0, 500.0 );
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      dip::Image minima = dip::Minima( img, {}, 1, "labels" );
      dip::Image segmented = dip::SeededWatershed( img, minima );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
