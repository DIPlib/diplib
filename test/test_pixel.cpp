#define DOCTEST_CONFIG_IMPLEMENT

#include "diplib.h"

void ImageFunction( dip::Image const& ) {}
void ValueFunction( dip::dcomplex ) {}

int main() {

   // CONSTRUCTING AND CASTING

   dip::Image::Sample s{ 4.6 };
   dip::Image::Sample c{ dip::dcomplex{ 4.1, 2.1 } };
   std::cout << "s = " << s << std::endl;
   std::cout << (dip::sint)s << std::endl;
   std::cout << (double)s << std::endl;
   std::cout << static_cast< dip::dcomplex >( s ) << std::endl;
   std::cout << "c = " << c << std::endl;
   std::cout << (dip::sint)c << std::endl;
   std::cout << (double)c << std::endl;
   std::cout << static_cast< dip::dcomplex >( c ) << std::endl;

   dip::Image::Pixel p{ 4, 6, 7, 3 };
   std::cout << p << std::endl;

   dip::Image image( { 1 }, 1, dip::DT_UINT16 );
   dip::Image::Pixel pixelRef = image.At( 0 );   // copy constructor
   dip::Image::Sample sampleRef = pixelRef[ 0 ];    // copy constructor
   dip::Image::Sample sampleRef2 = static_cast< dip::Image::Pixel >( image );
   //dip::Image::Sample sampleRef3 = image;        // Illegal: OK.
   dip::Image A{ pixelRef };
   dip::Image B{ sampleRef };
   dip::Image C{ dip::uint32( 0 ) };
   //dip::Image D( { 10.0f, 1.0f, 0.0f } );        // Ambiguous (could be UnsignedArray): how to fix?
   dip::Image E( dip::Image::Pixel{ 10.0f, 1.0f, 0.0f } );

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
   ValueFunction( static_cast< dip::dcomplex >( dip::Image::Sample( image )));
   ValueFunction( static_cast< dip::dcomplex >( image ));
   ValueFunction( static_cast< dip::dcomplex >( image.At( 0 ) ));

   // ASSIGNING

   image = pixelRef;
   image = sampleRef;
   image = dip::uint32( 0 );
   image = pixel;

   pixel = sample;
   pixel = 4;

   pixelRef = 8;
   pixelRef = sample;
   pixelRef = sampleRef;
   pixelRef = image.At( 0 );

   sample = 7;
   sampleRef = 3;
   sampleRef = sample;
   sampleRef = image.At( 0 )[ 0 ];

   image.At( 0 ) = sample;
   image.At( 0 ) = pixel;
   image.At( 0 )[ 0 ] = sample;

   // USING

   dip::Image::Pixel p1 = image.At( 0 ) + 2;
   dip::Image::Pixel p2 = image.At( 0 )[ 0 ] + 2;

   image.At( 0 ) += 2;

   //dip::dfloat f1 = image.At( 0 ) + 2;        // Illegal: how to fix?
   dip::dfloat f2 = image.At( 0 ).As< double >() + 2;
   //dip::dfloat f3 = image.At( 0 )[ 0 ] + 2;   // Illegal: how to fix?
   dip::dfloat f4 = image.At( 0 )[ 0 ].As< double >() + 2;

   if( image.At( 0 )[ 0 ] ) {}
   if( image.At( 0 )[ 0 ] == 0 ) {}
   if( image.At( 0 )[ 0 ].As< int >() == 0 ) {}
   if( image.At( 0 )[ 0 ] == sample ) {}
   if( image.At( 0 ) == pixel ) {}
}
