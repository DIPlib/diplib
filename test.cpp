#include <iostream>
#include "diplib.h"

int main() {
   {
   dip::Image img;
   std::cout << img;
   img.SetDimensions({50,80,30});
   img.SetTensorDimensions({3});
   img.Forge();
   std::cout << img;
   img.Strip();
   img.SetStrides({-80,-1,4000});
   img.SetTensorStrides({120000});
   img.Forge();
   std::cout << img;
   }
   return 1;
}
