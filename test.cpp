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
   // Forging with various strides
   {
   dip::Image img;
   std::cout << img;
   img.SetDimensions({50,80,30});
   img.SetTensorDimensions({3});
   img.Forge();
   std::cout << img;
   img.Strip();
   img.SetStrides({-80,-1,4000});
   img.SetTensorStride(120000);
   img.Forge();
   std::cout << img;
   }
   // Calling a function with overloads
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
   //

   return 1;
}
