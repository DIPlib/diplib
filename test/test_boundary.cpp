#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <numeric>
#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/iterators.h"
#include "diplib/testing.h"

// Testing the BoundaryArray options and the ExtendImage function.

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      {
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip::ImageIterator< dip::uint16 > it( img );
         dip::uint16 counter = 0;
         do {
            *it = counter++;
         } while( ++it );
      }

      std::cout << "img = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( img );
      std::cout << "\n\n";

      dip::Image out = dip::ExtendImage( img, { 4, 4 }, { "first order" } );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( out );
      std::cout << "\n\n";

      out.Strip();
      out.SetDataType( dip::DT_SFLOAT );
      out.Protect();

      dip::ExtendImage( img, out, { 4, 4 }, { "third order" } );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::sfloat >( out );
      std::cout << "\n\n";

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
