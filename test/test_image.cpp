#define DOCTEST_CONFIG_IMPLEMENT

#include <iostream>
#include <typeinfo>
#include "diplib.h"
#include "diplib/overload.h"

// Just a test!

template< typename TPI >
static const char* dip__MyFunction( void* vin ) {
   TPI* in = static_cast< TPI* > ( vin );
   return typeid( in ).name();
}

const char* MyFunction( dip::Image& image ) {
   dip::DataType dt = image.DataType();
   void* in = nullptr; // image.Data();
   const char* out;
   DIP_OVL_CALL_ASSIGN_ALL( out, dip__MyFunction, (in), dt )
   return out;
}


int main() {
   try {
      std::cout << "Forging with various strides." << std::endl;
      {
         dip::Image img;
         std::cout << img;
         img.SetSizes( { 50, 80, 30 } );
         img.SetTensorSizes(3);
         img.Forge();
         std::cout << img;
         img.Strip();
         img.SetStrides({-80,-1,4000});
         img.SetTensorStride(120000);
         img.Forge();
         std::cout << img;
      }
      std::cout << std::endl << "Calling a function with overloads." << std::endl;
      {
         dip::Image img;
         img.SetDataType(dip::DT_BIN);
         std::cout << MyFunction(img) << std::endl;
         img.SetDataType(dip::DT_UINT8);
         std::cout << MyFunction(img) << std::endl;
         img.SetDataType(dip::DT_SINT32);
         std::cout << MyFunction(img) << std::endl;
         img.SetDataType(dip::DT_SFLOAT);
         std::cout << MyFunction(img) << std::endl;
         img.SetDataType(dip::DT_DCOMPLEX);
         std::cout << MyFunction(img) << std::endl;
      }
      std::cout << std::endl << "Indexing." << std::endl;
      {
         dip::Image img1;
         img1.SetSizes( { 50, 80, 30 } );
         img1.SetTensorSizes(3);
         img1.SetPixelSize({{dip::PhysicalQuantity::Micrometer(), 3*dip::PhysicalQuantity::Micrometer(), dip::PhysicalQuantity::Radian()}});
         img1.Forge();
         std::cout << img1;
         dip::Image img2(img1);
         std::cout << img2;
         img2 = img1.At(10,10,10);
         std::cout << img2;
         img2 = img1[1];
         std::cout << img2;
         img2 = img1[1].At(10,10,10);
         std::cout << img2;
         img2 = img1.At(10,10,10)[1];
         std::cout << img2;
         img2 = img1.At(dip::Range{},dip::Range{0,-1,4},dip::Range{10});
         std::cout << img2;
         img1.Strip();
         img1.SetDataType( dip::DT_SCOMPLEX );
         img1.Forge();
         std::cout << img1;
         img2 = img1.Imaginary();
         std::cout << img2;
      }
      std::cout << std::endl << "Reshaping." << std::endl;
      {
         dip::Image img1;
         img1.SetSizes( { 50, 80, 30 } );
         img1.SetTensorSizes(3);
         img1.SetPixelSize({{dip::PhysicalQuantity::Micrometer(), 3*dip::PhysicalQuantity::Micrometer(), dip::PhysicalQuantity::Radian()}});
         img1.Forge();
         std::cout << img1;
         img1.PermuteDimensions({2,1,0});
         std::cout << img1;
         img1.SwapDimensions(0,1);
         std::cout << img1;
         img1.Mirror({true,false,false});
         std::cout << img1;
         img1.ExpandDimensionality(5);
         std::cout << img1;
         img1.AddSingleton(0);
         std::cout << img1;
         img1.Squeeze();
         std::cout << img1;
         img1.Strip();
         img1.SetStrides({});
         img1.Forge();
         std::cout << img1;
         img1.Flatten();
         std::cout << img1;
      }


   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
