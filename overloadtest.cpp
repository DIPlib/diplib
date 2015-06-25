#include <iostream>

#define DIP_OVL_CALL(fname, paramlist, dtype) \
   if( dtype == SFLOAT ) { fname <float> paramlist; } \
   if( dtype == DFLOAT ) { fname <double> paramlist; }

enum DataType { SFLOAT, DFLOAT };

template <typename TPI>
static void dip__MyFunction( void* vin ) {
   TPI* in = static_cast <TPI*> ( vin );
   std::cout << "Data type = " << typeid(*in).name() << std::endl;
}

int main () {
   DataType dt = DFLOAT;
   double data;
   void* in = &data;
   DIP_OVL_CALL( dip__MyFunction, (in), dt );
   return 0;
}
