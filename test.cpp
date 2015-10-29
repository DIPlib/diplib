#include <iostream>
#include "diplib.h"

// Just a test!

#include "dip_overload.h"
#include <typeinfo>

template< typename TPI >
static void dip__MyFunction( void* vin ) {
   TPI* in = static_cast< TPI* > ( vin );
   std::cout << "Data type = " << typeid( in ).name() << std::endl;
}

void MyFunction( dip::Image image ) {
   dip::DataType dt = image.DataType();
   void* in = nullptr; // image.Data();
   DIP_OVL_CALL_ALL( dip__MyFunction, (in), dt )
}


int main() {
   try {
      std::cout << "Forging with various strides." << std::endl;
      {
         dip::Image img;
         std::cout << img;
         img.SetDimensions({50,80,30});
         img.SetTensorDimensions({3});
         img.Forge();
         std::cout << img;
         dip::uint stride; void* origin;
         img.GetSimpleStrideAndOrigin(stride, origin);
         std::cout << "simple stride: " << stride << std::endl;
         img.Strip();
         img.SetStrides({-80,-1,4000});
         img.SetTensorStride(120000);
         img.Forge();
         std::cout << img;
         img.GetSimpleStrideAndOrigin(stride, origin);
         std::cout << "simple stride: " << stride << std::endl;
      }
      std::cout << std::endl << "Calling a function with overloads." << std::endl;
      {
         dip::Image img;
         img.SetDataType(dip::DT_BIN);
         MyFunction(img);
         img.SetDataType(dip::DT_UINT8);
         MyFunction(img);
         img.SetDataType(dip::DT_SINT32);
         MyFunction(img);
         img.SetDataType(dip::DT_SFLOAT);
         MyFunction(img);
         img.SetDataType(dip::DT_DCOMPLEX);
         MyFunction(img);
      }
      std::cout << std::endl << "Indexing." << std::endl;
      {
         dip::Image img1;
         img1.SetDimensions({50,80,30});
         img1.SetTensorDimensions({3});
         img1.Forge();
         std::cout << img1;
         dip::Image img2(img1);
         std::cout << img2;
         img2.Strip();
         img2 = img1.At(10,10,10);
         std::cout << img2;
         img2.Strip();
         img2 = img1[1];
         std::cout << img2;
         img2.Strip();
         img2 = img1[1].At(10,10,10);
         std::cout << img2;
         img2.Strip();
         img2 = img1.At(10,10,10)[1];
         std::cout << img2;
         img2.Strip();
         img2 = img1.At(dip::Range{},dip::Range{0,-1,4},dip::Range{10});
         std::cout << img2;
      }
      std::cout << std::endl << "Reshaping." << std::endl;
      {
         dip::Image img1;
         img1.SetDimensions({50,80,30});
         img1.SetTensorDimensions({3});
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
      std::cout << e.what() << std::endl;
   }
   return 1;
}
