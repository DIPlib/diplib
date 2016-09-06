#include <iostream>
#include "dip_physdims.h"

// Testing

int main() {
   dip::PhysicalQuantity a = 50 * dip::PhysicalQuantity::Nanometer();
   dip::PhysicalQuantity b = .4 * dip::PhysicalQuantity::Micrometer();
   std::cout << "a = " << a << std::endl;
   std::cout << "b = " << b << std::endl;
   std::cout << "a + b = " << a + b << std::endl;
   a.Power( -2 );
   std::cout << "a^-2 = " << a << std::endl;

   dip::PhysicalQuantity c( 100, dip::Units::BaseUnits::TIME );
   std::cout << "c = " << c << std::endl;
   c.Power( -1 );
   std::cout << "c^-1 = " << c << std::endl;
   std::cout << "b / c = " << b * c << std::endl;

   dip::PhysicalQuantity d( 2 * std::acos( 0 ), dip::Units::BaseUnits::ANGLE );
   std::cout << "d = " << d << std::endl;

   try {
      c + d;
   } catch ( std::exception& e ) {
      std::cout << "Caught exception: " << e.what() << std::endl;
   }

   dip::PhysicalQuantity f = dip::PhysicalQuantity::Meter();
   std::cout << "1 m = " << f << std::endl;
   std::cout << "0.1 m = " << f * 0.1 << std::endl;
   std::cout << "0.01 m = " << f * 0.01 << std::endl;
   std::cout << "0.001 m = " << f * 0.001 << std::endl;
   std::cout << "0.0001 m = " << f * 0.0001 << std::endl;
   std::cout << "0.00001 m = " << f * 0.00001 << std::endl;
   std::cout << "0.000001 m = " << f * 0.000001 << std::endl;
   std::cout << "0.0000001 m = " << f * 0.0000001 << std::endl;
   std::cout << "0.00000001 m = " << f * 0.00000001 << std::endl;
   std::cout << "0.000000001 m = " << f * 0.000000001 << std::endl;
   std::cout << "0.0000000001 m = " << f * 0.0000000001 << std::endl;
   std::cout << "10 m = " << f * 10 << std::endl;
   std::cout << "100 m = " << f * 100 << std::endl;
   std::cout << "1000 m = " << f * 1000 << std::endl;
   std::cout << "10000 m = " << f * 10000 << std::endl;
   std::cout << "100000 m = " << f * 100000 << std::endl;
   std::cout << "1000000 m = " << f * 1000000 << std::endl;
   std::cout << "10000000 m = " << f * 10000000 << std::endl;
   std::cout << "100000000 m = " << f * 100000000 << std::endl;
   std::cout << "1000000000 m = " << f * 1000000000 << std::endl;

   std::cout << "1 m = " << f << std::endl;
   std::cout << "1 m^2 = " << f*f << std::endl;
   std::cout << "1 m^3 = " << f*f*f << std::endl;
   std::cout << "1 m^4 = " << f*f*f*f << std::endl;
   std::cout << "1 m^-1 = " << 1/f << std::endl;
   std::cout << "1 m^-2 = " << 1/f/f << std::endl;
   std::cout << "1 m^-3 = " << 1/f/f/f << std::endl;
   std::cout << "1 m^-4 = " << 1/f/f/f/f << std::endl;

   dip::PhysicalQuantity g = 1.0 * dip::Units( dip::Units::BaseUnits::TIME );
   std::cout << "1 m/s = " << f/g << std::endl;
   std::cout << "1 m/s^2 = " << f/g/g << std::endl;
   std::cout << "1 m/s^3 = " << f/g/g/g << std::endl;
   std::cout << "1 m/s^4 = " << f/g/g/g/g << std::endl;
   std::cout << "1 s/m = " << g/f << std::endl;
   std::cout << "1 s/m^2 = " << g/f/f << std::endl;
   std::cout << "1 s^2/m = " << g*g/f << std::endl;

   return 0;
}
