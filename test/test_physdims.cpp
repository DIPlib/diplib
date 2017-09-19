#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib/library/physical_dimensions.h"

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
   std::cout << ". Aspect ratio = " << ps.AspectRatio( ps.Size() );
   std::cout << std::endl;
}

int main() {

   std::cout << "m = " << dip::Units( "m" ) << std::endl;
   std::cout << "m^2 = " << dip::Units( "m^2" ) << std::endl;
   std::cout << "mm = " << dip::Units( "mm" ) << std::endl;
   std::cout << "mm^2 = " << dip::Units( "mm^2" ) << std::endl;
   std::cout << "mm^-2 = " << dip::Units( "mm^-2" ) << std::endl;
   std::cout << "10^6.mm^2 = " << dip::Units( "10^6.mm^2" ) << std::endl; // writes m^2
   std::cout << "km = " << dip::Units( "km" ) << std::endl;
   std::cout << "km/s = " << dip::Units( "km/s" ) << std::endl;
   std::cout << "km.cd.rad.px = " << dip::Units( "km.cd.rad.px" ) << std::endl;
   std::cout << "km.cd/rad.px = " << dip::Units( "km.cd/rad.px" ) << std::endl; // writes km.cd.px/rad
   std::cout << "10^3.km^-1.cd^-2/K = " << dip::Units( "10^3.km^-1.cd^-2/K" ) << std::endl; // writes m^-1/K/cd^2

   std::cout << "s/m = " << dip::Units( "s/m" ) << std::endl;
   std::cout << "s/m^2 = " << dip::Units( "s/m^2" ) << std::endl;
   std::cout << "s^2/m = " << dip::Units( "s^2/m" ) << std::endl;

   dip::PhysicalQuantity a = 50 * dip::Units::Nanometer();
   dip::PhysicalQuantity b = .4 * dip::Units::Micrometer();
   std::cout << "a = " << a << std::endl;
   std::cout << "b = " << b << std::endl;
   std::cout << "a + b = " << a + b << std::endl;
   std::cout << "b + a = " << b + a << std::endl;
   std::cout << "a + a = " << a + a << std::endl;
   std::cout << "2 * a = " << 2 * a << std::endl;
   std::cout << "a^-2 = " << a.Power( -2 ) << std::endl;

   dip::PhysicalQuantity c( 100, dip::Units::Second() );
   std::cout << "c = " << c << std::endl;
   c = c.Power( -1 );
   std::cout << "c^-1 = " << c << std::endl;
   std::cout << "b / c = " << b * c << std::endl;

   dip::PhysicalQuantity d = 180 * dip::PhysicalQuantity::Degree();
   std::cout << "d = " << d << std::endl;

   try {
      c + d;
   } catch( std::exception& e ) {
      std::cout << "Caught exception: " << e.what() << std::endl;
   }

   dip::PhysicalQuantity f = dip::PhysicalQuantity::Meter();
   std::cout << "1 m = " << f << std::endl;
   std::cout << "0.1 m = " << (f * 0.1).Normalize() << std::endl;
   std::cout << "0.01 m = " << (f * 0.01).Normalize() << std::endl;
   std::cout << "0.001 m = " << (f * 0.001).Normalize() << std::endl;
   std::cout << "0.0001 m = " << (f * 0.0001).Normalize() << std::endl;
   std::cout << "0.00001 m = " << (f * 0.00001).Normalize() << std::endl;
   std::cout << "0.000001 m = " << (f * 0.000001).Normalize() << std::endl;
   std::cout << "0.0000001 m = " << (f * 0.0000001).Normalize() << std::endl;
   std::cout << "0.00000001 m = " << (f * 0.00000001).Normalize() << std::endl;
   std::cout << "0.000000001 m = " << (f * 0.000000001).Normalize() << std::endl;
   std::cout << "0.0000000001 m = " << (f * 0.0000000001).Normalize() << std::endl;
   std::cout << "10 m = " << (f * 10).Normalize() << std::endl;
   std::cout << "100 m = " << (f * 100).Normalize() << std::endl;
   std::cout << "1000 m = " << (f * 1000).Normalize() << std::endl;
   std::cout << "10000 m = " << (f * 10000).Normalize() << std::endl;
   std::cout << "100000 m = " << (f * 100000).Normalize() << std::endl;
   std::cout << "1000000 m = " << (f * 1000000).Normalize() << std::endl;
   std::cout << "10000000 m = " << (f * 10000000).Normalize() << std::endl;
   std::cout << "100000000 m = " << (f * 100000000).Normalize() << std::endl;
   std::cout << "1000000000 m = " << (f * 1000000000).Normalize() << std::endl;
   std::cout << "10 m^2 = " << (f * f * 10).Normalize() << std::endl;
   std::cout << "100 m^2 = " << (f * f * 100).Normalize() << std::endl;
   std::cout << "1000 m^2 = " << (f * f * 1000).Normalize() << std::endl;
   std::cout << "10000 m^2 = " << (f * f * 10000).Normalize() << std::endl;
   std::cout << "100000 m^2 = " << (f * f * 100000).Normalize() << std::endl;
   std::cout << "1000000 m^2 = " << (f * f * 1000000).Normalize() << std::endl;
   std::cout << "10000000 m^2 = " << (f * f * 10000000).Normalize() << std::endl;
   std::cout << "100000000 m^2 = " << (f * f * 100000000).Normalize() << std::endl;
   std::cout << "1000000000 m^2 = " << (f * f * 1000000000).Normalize() << std::endl;

   std::cout << "1 m = " << (f).Normalize() << std::endl;
   std::cout << "1 m^2 = " << (f * f).Normalize() << std::endl;
   std::cout << "1 m^3 = " << (f * f * f).Normalize() << std::endl;
   std::cout << "1 m^4 = " << (f * f * f * f).Normalize() << std::endl;
   std::cout << "1 m^-1 = " << (1 / f).Normalize() << std::endl;
   std::cout << "1 m^-2 = " << (1 / f / f).Normalize() << std::endl;
   std::cout << "1 m^-3 = " << (1 / f / f / f).Normalize() << std::endl;
   std::cout << "1 m^-4 = " << (1 / f / f / f / f).Normalize() << std::endl;

   dip::PhysicalQuantity g = 1.0 * dip::Units::Second();
   std::cout << "1 m/s = " << (f / g).Normalize() << std::endl;
   std::cout << "1 m/s^2 = " << (f / g / g).Normalize() << std::endl;
   std::cout << "1 m/s^3 = " << (f / g / g / g).Normalize() << std::endl;
   std::cout << "1 m/s^4 = " << (f / g / g / g / g).Normalize() << std::endl;
   std::cout << "1 s/m = " << (g / f).Normalize() << std::endl;
   std::cout << "1 s/m^2 = " << (g / f / f).Normalize() << std::endl;
   std::cout << "1 s^2/m = " << (g * g / f).Normalize() << std::endl;

   std::cout << "10^6.m^2 = " << dip::Units( "10^6.mm^2" ) << std::endl;
   std::cout << "km/s = " << dip::Units( "km/s" ) << std::endl;
   std::cout << "km.cd.rad.px = " << dip::Units( "km.cd.rad.px" ) << std::endl;
   std::cout << "km.cd/rad.px = " << dip::Units( "km.cd/rad.px" ) << std::endl;
   std::cout << "10^3.km^-1.cd^-2/K = " << dip::Units( "10^3.km^-1.cd^-2/K" ) << std::endl;

#ifdef DIP__ENABLE_UNICODE
   std::cout << "10\u2076\u00B7mm\u00B2 = " << dip::Units( "10\u2076\u00B7mm\u00B2" ) << std::endl;
   std::cout << "km/s = " << dip::Units( "km/s" ) << std::endl;
   std::cout << "km\u00B7cd\u00B7rad\u00B7px = " << dip::Units( "km\u00B7cd\u00B7rad\u00B7px" ) << std::endl;
   std::cout << "km\u00B7cd/rad\u00B7px = " << dip::Units( "km\u00B7cd/rad\u00B7px" ) << std::endl;
   std::cout << "10\u00B3\u00B7km\u207B\u00B9\u00B7cd\u207B\u00B2/K = " << dip::Units( "10\u00B3\u00B7km\u207B\u00B9\u00B7cd\u207B\u00B2/K" ) << std::endl;
#endif

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
