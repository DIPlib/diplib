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

      dip::Image out = dip::ExtendImage( img, { 4, 5 }, { "first order" } );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( out );
      std::cout << "\n\n";

      dip::ExtendImageLowLevel( {}, out, { 4, 5 }, { dip::BoundaryCondition::SYMMETRIC_MIRROR },
                                dip::Option::ExtendImage_FillBoundaryOnly );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( out );
      std::cout << "\n\n";

      out.Strip();
      out.SetDataType( dip::DT_SFLOAT );
      out.Protect();
      dip::ExtendImage( img, out, { 4, 6 }, { "third order" } );
      out.Protect( false );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::sfloat >( out );
      std::cout << "\n\n";

      dip::ExtendImageLowLevel( img, out, { 4, 5 }, { dip::BoundaryCondition::SYMMETRIC_MIRROR },
                                dip::Option::ExtendImage_Masked );
      dip::ExtendImageLowLevel( {}, out, { 4, 5 }, { dip::BoundaryCondition::ADD_ZEROS },
                                dip::Option::ExtendImage_Masked + dip::Option::ExtendImage_FillBoundaryOnly );

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( out );
      std::cout << "\n\n";

      {
         dip::uint8* ptr = static_cast< dip::uint8* >( out.Origin())
                           - 4 * 2 * out.Stride( 0 ) - 5 * 2 * out.Stride( 1 );
         out.dip__SetOrigin( ptr );
         dip::UnsignedArray fullSizes = out.Sizes();
         fullSizes[ 0 ] += 4 * 2;
         fullSizes[ 1 ] += 5 * 2;
         out.dip__SetSizes( fullSizes );
      }

      std::cout << "out data type = " << out.DataType().Name() << std::endl;
      std::cout << "out = \n";
      dip::testing::PrintPixelValues< dip::uint16 >( out );
      std::cout << "\n\n";

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
