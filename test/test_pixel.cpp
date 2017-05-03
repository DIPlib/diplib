#define DOCTEST_CONFIG_IMPLEMENT

#include "diplib.h"

void ImageFunction( dip::Image const& ) {}
void ValueFunction( dip::dcomplex v ) {}

int main() {

   // CASTING

   dip::Image::Sample s{ 4.6 };
   dip::Image::Sample c{ dip::dcomplex{ 4.1, 2.1 } };
   std::cout << "s = " << s << std::endl;
   std::cout << (dip::sint32)s << std::endl;
   std::cout << (double)s << std::endl;
   std::cout << static_cast< dip::dcomplex >( s ) << std::endl;
   std::cout << "c = " << c << std::endl;
   std::cout << (dip::sint32)c << std::endl;
   std::cout << (double)c << std::endl;
   std::cout << static_cast< dip::dcomplex >( c ) << std::endl;

   dip::Image::Pixel p{ 4, 6, 7, 3 };
   std::cout << p << std::endl;

   dip::Image image( { 1 }, 1, dip::DT_UINT16 );
   dip::Image::PixelRef pixelRef = image.At( 0 );   // copy constructor
   dip::Image::SampleRef sampleRef = pixelRef[ 0 ]; // copy constructor
   dip::Image::SampleRef sampleRef2 = static_cast< dip::Image::PixelRef >( image );
   dip::Image A{ pixelRef };
   dip::Image B{ sampleRef };
   dip::Image C{ dip::uint32( 0 ) };
   //dip::Image D( { 10.0f, 1.0f, 0.0f } );        // Ambiguous: how to fix?
   dip::Image D( dip::Image::Pixel{ 10.0f, 1.0f, 0.0f } );

   dip::Image::Pixel pixel = pixelRef;             // copy constructor
   dip::Image::Sample sample = dip::uint32( 0 );   // cast & copy constructor
   dip::Image::Pixel pixel2 = sample;              // cast & copy constructor
   //dip::Image::Pixel pixel3 = dip::uint32( 0 );  // Illegal: how to fix?

   ImageFunction( image );
   //ImageFunction( pixelRef );   // illegal: OK
   //ImageFunction( sampleRef2 ); // illegal: OK
   //ImageFunction( 0 );          // illegal: OK

   ValueFunction( 0 );
   ValueFunction( static_cast< dip::dcomplex >( sampleRef2 ));
   ValueFunction( static_cast< dip::dcomplex >( pixelRef[ 0 ] ));
   ValueFunction( static_cast< dip::dcomplex >( image.At( 0 )[ 0 ] ));
   ValueFunction( static_cast< dip::dcomplex >( dip::Image::SampleRef( image )));
   //ValueFunction( static_cast< dip::dcomplex >( image ));            // Illegal: how to fix?
   //ValueFunction( static_cast< dip::dcomplex >( image.At( 0 ) ));    // Illegal: how to fix?

   // ASSIGNING

   image = pixelRef;
   image = sampleRef;
   image = dip::uint32( 0 );
   image = pixel;

   pixel = sample;

   pixelRef = sample;
   pixelRef = sampleRef;
   pixelRef = image.At( 0 );

   sampleRef = sample;
   sampleRef = image.At( 0 )[ 0 ];

   image.At( 0 ) = pixel;
   image.At( 0 )[ 0 ] = sample;
}
