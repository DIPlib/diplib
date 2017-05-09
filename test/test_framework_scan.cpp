#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/math.h"

// Testing the scan framework through the dip_Add and dip_Mul functions.

int main() {
   bool error = false;
   try {

      dip::Tensor example( dip::Tensor::Shape::SYMMETRIC_MATRIX, 2, 2 );

      dip::Image lhs{ dip::UnsignedArray{ 50, 80, 30 }, 3, dip::DT_UINT8 };
      lhs.ReshapeTensor( example );

      dip::Image rhs{ dip::UnsignedArray{ 50, 1, 30 }, 2, dip::DT_SINT16 };
      example.SetShape( dip::Tensor::Shape::DIAGONAL_MATRIX, 2, 2 );
      rhs.ReshapeTensor( example );

      lhs.Fill( 1 );                     // all values
      lhs[ 2 ].At( 3, 4, 5 ) =  9;       // off-diagonal values for this pixel only
      rhs.Fill( 4 );                     // diagonal values for all pixels
      rhs[ 0 ].At( 2, 0, 3 ) = 6;        // first element for this pixel only

      dip::Image out = lhs + rhs;
      std::cout << out;

      if( out.Sizes() != dip::UnsignedArray{ 50, 80, 30 } ) {
         std::cout << "Output image size not as expected\n";
         error |= true;
      }

      if( out.TensorShape() != dip::Tensor::Shape::COL_MAJOR_MATRIX ) {
         std::cout << "Output tensor shape not as expected\n";
         error |= true;
      }
      if( ( out.TensorRows() != 2 ) || ( out.TensorColumns() != 2 ) ) {
         std::cout << "Output tensor size not as expected\n";
         error |= true;
      }

      if( out.At( 0, 0, 0 )[ 0 ] != 1 + 4 ) {
         std::cout << "Output value (0,0,0)[0] not as expected\n";
         error |= true;
      }
      if( out.At( 0, 0, 0 )[ 1 ] != 1 + 0 ) {
         std::cout << "Output value (0,0,0)[1] not as expected\n";
         error |= true;
      }
      if( out.At( 0, 0, 0 )[ 2 ] != 1 + 0 ) {
         std::cout << "Output value (0,0,0)[2] not as expected\n";
         error |= true;
      }
      if( out.At( 0, 0, 0 )[ 3 ] != 1 + 4 ) {
         std::cout << "Output value (0,0,0)[3] not as expected\n";
         error |= true;
      }

      if( out.At( 3, 4, 5 )[ 0 ] != 1 + 4 ) {
         std::cout << "Output value (3,4,5)[0] not as expected\n";
         error |= true;
      }
      if( out.At( 3, 4, 5 )[ 1 ] != 9 + 0 ) {
         std::cout << "Output value (3,4,5)[1] not as expected\n";
         error |= true;
      }
      if( out.At( 3, 4, 5 )[ 2 ] != 9 + 0 ) {
         std::cout << "Output value (3,4,5)[2] not as expected\n";
         error |= true;
      }
      if( out.At( 3, 4, 5 )[ 3 ] != 1 + 4 ) {
         std::cout << "Output value (3,4,5)[3] not as expected\n";
         error |= true;
      }

      if( out.At( 2, 10, 3 )[ 0 ] != 1 + 6 ) {
         std::cout << "Output value (2,10,3)[0] not as expected\n";
         error |= true;
      }
      if( out.At( 2, 10, 3 )[ 1 ] != 1 + 0 ) {
         std::cout << "Output value (2,10,3)[1] not as expected\n";
         error |= true;
      }
      if( out.At( 2, 10, 3 )[ 2 ] != 1 + 0 ) {
         std::cout << "Output value (2,10,3)[2] not as expected\n";
         error |= true;
      }
      if( out.At( 2, 10, 3 )[ 3 ] != 1 + 4 ) {
         std::cout << "Output value (2,10,3)[3] not as expected\n";
         error |= true;
      }

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return error;
}
