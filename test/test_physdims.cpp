#include <iostream>
#include "diplib/physdims.h"

// Testing

void PrintPhysicalQuantityArray( dip::PhysicalQuantityArray pqa ) {
   std::cout << "Array: ";
   if( pqa.empty() ) {
      std::cout << "(empty)\n";
   } else {
      std::cout << pqa[ 0 ];
      for( dip::uint ii = 1; ii < pqa.size(); ++ii ) {
         std::cout << " x " << pqa[ ii ];
      }
   }
   std::cout << std::endl;
}

void PrintPixelSize( dip::PixelSize ps ) {
   std::cout << "Pixel: ";
   if( !ps.IsDefined() ) {
      std::cout << "undefined";
   } else {
      std::cout << ps.Get( 0 );
      for( dip::uint ii = 1; ii < ps.Size(); ++ii ) {
         std::cout << " x " << ps.Get( ii );
      }
   }
   std::cout << ". Volume = " << ps.Product( ps.Size() );
   if( ps.IsIsotropic() ) {
      std::cout << " (isotropic)";
   }
   std::cout << std::endl;
}

int main() {
   dip::PhysicalQuantity a = 50 * dip::PhysicalQuantity::Nanometer();
   dip::PhysicalQuantity b = .4 * dip::PhysicalQuantity::Micrometer();
   std::cout << "a = " << a << std::endl;
   std::cout << "b = " << b << std::endl;
   std::cout << "a + b = " << a + b << std::endl;
   a.Power( -2 );
   std::cout << "a^-2 = " << a << std::endl;

   dip::PhysicalQuantity c( 100, dip::Units::Second() );
   std::cout << "c = " << c << std::endl;
   c.Power( -1 );
   std::cout << "c^-1 = " << c << std::endl;
   std::cout << "b / c = " << b * c << std::endl;

   dip::PhysicalQuantity d( 2 * std::acos( 0 ), dip::Units::Radian() );
   std::cout << "d = " << d << std::endl;

   try {
      c + d;
   } catch( std::exception& e ) {
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
   std::cout << "10 m^2 = " << f * f * 10 << std::endl;
   std::cout << "100 m^2 = " << f * f * 100 << std::endl;
   std::cout << "1000 m^2 = " << f * f * 1000 << std::endl;
   std::cout << "10000 m^2 = " << f * f * 10000 << std::endl;
   std::cout << "100000 m^2 = " << f * f * 100000 << std::endl;
   std::cout << "1000000 m^2 = " << f * f * 1000000 << std::endl;
   std::cout << "10000000 m^2 = " << f * f * 10000000 << std::endl;
   std::cout << "100000000 m^2 = " << f * f * 100000000 << std::endl;
   std::cout << "1000000000 m^2 = " << f * f * 1000000000 << std::endl;

   std::cout << "1 m = " << f << std::endl;
   std::cout << "1 m^2 = " << f * f << std::endl;
   std::cout << "1 m^3 = " << f * f * f << std::endl;
   std::cout << "1 m^4 = " << f * f * f * f << std::endl;
   std::cout << "1 m^-1 = " << 1 / f << std::endl;
   std::cout << "1 m^-2 = " << 1 / f / f << std::endl;
   std::cout << "1 m^-3 = " << 1 / f / f / f << std::endl;
   std::cout << "1 m^-4 = " << 1 / f / f / f / f << std::endl;

   dip::PhysicalQuantity g = 1.0 * dip::Units::Second();
   std::cout << "1 m/s = " << f / g << std::endl;
   std::cout << "1 m/s^2 = " << f / g / g << std::endl;
   std::cout << "1 m/s^3 = " << f / g / g / g << std::endl;
   std::cout << "1 m/s^4 = " << f / g / g / g / g << std::endl;
   std::cout << "1 s/m = " << g / f << std::endl;
   std::cout << "1 s/m^2 = " << g / f / f << std::endl;
   std::cout << "1 s^2/m = " << g * g / f << std::endl;

   dip::PixelSize sz;
   std::cout << "Default-constructed PixelSize: ";
   PrintPixelSize( sz );

   sz.SetMicrometers( 4, 5.7 );
   std::cout << "Step 1: ";
   PrintPixelSize( sz );

   sz.Set( 2, 3.3 * ( dip::Units::Hertz() * dip::Units::Radian() ) );
   std::cout << "Step 2: ";
   PrintPixelSize( sz );

   sz.Scale( 1.3 );
   std::cout << "Step 3: ";
   PrintPixelSize( sz );

   sz.Scale( 3, 1 / 1.3 );
   std::cout << "Step 4: ";
   PrintPixelSize( sz );

   sz.Scale( 2, 1 / 1.3 );
   std::cout << "Step 5: ";
   PrintPixelSize( sz );

   sz.EraseDimension( 1 );
   sz.InsertDimension( 2 );
   std::cout << "Step 6: ";
   PrintPixelSize( sz );

   sz.SwapDimensions( 0, 1 );
   std::cout << "Step 7: ";
   PrintPixelSize( sz );

   PrintPhysicalQuantityArray( sz.ToPhysical( dip::FloatArray{ 10, 10 } ) );
   PrintPhysicalQuantityArray( sz.ToPhysical( dip::FloatArray{ 10, 10, 10 } ) );
   PrintPhysicalQuantityArray( sz.ToPhysical( dip::FloatArray{ 10, 10, 10, 10, 10 } ) );

   sz.Clear();
   sz.SetMicrometers( 0, 0.5 );
   sz.SetMicrometers( 1, 0.5 ); // doesn't change anything
   sz.SetMicrometers( 2, 0.5 ); // doesn't change anything
   std::cout << "Step 8: ";
   PrintPixelSize( sz );

   sz.SetMicrometers( 2, 0.51 ); // now you should see three dimensions
   sz.SetMicrometers( 1, 0.51 );
   sz.SetMicrometers( 0, 0.51 );
   std::cout << "Step 9: ";
   PrintPixelSize( sz );

   return 0;
}
