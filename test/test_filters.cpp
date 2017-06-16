#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/morphology.h"
#include "diplib/iterators.h"

template< typename T >
void PrintPixelValues(
      dip::Image img
) {
   DIP_THROW_IF( img.DataType() != dip::DataType( T() ), "Wrong version of PrintPixelValues() called" );
   dip::uint linelength = img.Size( 0 );
   std::cout << "Image of size " << linelength << " x " << img.Sizes().product() / linelength << ":\n";
   dip::ImageIterator< T > it( img, 0 );
   dip::uint line = 0;
   do {
      auto lit = it.GetLineIterator();
      std::cout << it.Coordinates() << ": " << std::setw(4) << *lit;
      while( ++lit ) {
         std::cout << ", " << std::setw(4) << *lit;
      }
      std::cout << std::endl;
      ++line;
   } while( ++it );
}

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      dip::FillDelta( img );

      PrintPixelValues< dip::uint16 >( img );

      //dip::Image out = dip::Uniform( img, { { 10, 7 }, "line" } );
      dip::Image out = dip::Dilation( img, { { 17, 5 }, "diamond" } );

      PrintPixelValues< dip::uint16 >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
