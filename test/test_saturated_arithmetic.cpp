#include <iostream>
#include "diplib/saturated_arithmetic.h"

// Testing

int main() {
   std::cout << "10u - 20u =" << 10u - 20u << std::endl;
   std::cout << "saturated_sub(10u, 20u) =" << dip::saturated_sub(10u, 20u) << std::endl;
   return 0;
}
