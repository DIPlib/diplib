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
   dip::Image::Pixel pixelRef = image.At( 0 );     // copy constructor
   dip::Image::Sample sampleRef = pixelRef[ 0 ];   // copy constructor
   dip::Image::Sample sampleRef2 = static_cast< dip::Image::Pixel >( image );
   //dip::Image::Sample sampleRef3 = image;        // Illegal: OK.
   dip::Image A{ pixelRef };
   dip::Image B{ sampleRef };
   dip::Image C{ dip::uint32( 0 ) };
   dip::Image D{ 10.0f, 1.0f, 0.0f };
   dip::Image E( { 10, 1, 0 } ); // if interpreted as UnsignedArray, it will throw because size==0 in one dimension

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
   ValueFunction( image.At< dip::dcomplex >( 0 )[ 0 ] );
   ValueFunction( image.At( 0 )[ 0 ].As< dip::dcomplex>() );
   ValueFunction( static_cast< dip::dcomplex >( dip::Image::Sample( image )));
   ValueFunction( image.As< dip::dcomplex>() );
   ValueFunction( image.At< dip::dcomplex >( 0 ) );
   ValueFunction( image.At( 0 ).As< dip::dcomplex >() );

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
   image.At( 0 ) = 1;
   image.At( 0 ) = { 2 };
   image.At( 0 )[ 0 ] = 3;

   // USING

   dip::Image::Pixel p1 = image.At( 0 ) + 2;
   //dip::Image::Pixel p2 = image.At( 0 )[ 0 ] + 2;

   image.At( 0 ) += 2;

   dip::dfloat f1 = static_cast< double >( image.At( 0 ) + 2 );
   dip::dfloat f2 = image.At( 0 ).As< double >() + f1;
   dip::dfloat f3 = image.At< double >( 0 )[ 0 ] + f2;
   dip::dfloat f4 = image.At( 0 )[ 0 ].As< double >() + f3;

   dip::sfloat f5 = static_cast< float >( image.At< float >( 0 ) + f4 );
   dip::sfloat f6 = image.At( 0 ).As< float >() + f5;
   dip::sfloat f7 = image.At< float >( 0 )[ 0 ] + f6;
   f1 = image.At< float >( 0 )[ 0 ].As< float >() + f7;

   if( image.At( 0 )[ 0 ] ) {}
   if( image.At< bool >( 0 )[ 0 ] ) {}
   if( image.At( 0 )[ 0 ] == 0 ) {}
   if( image.At( 0 )[ 0 ].As< int >() == 0 ) {}
   if( image.At( 0 )[ 0 ] == sample ) {}
   if( image.At( 0 ) == pixel ) {}

   dip::Image img( { 256, 256 }, 3 );
   img.Fill( 0 );
   img.At( 10, 20 ) = { 4, 5, 6 };
   img.At( 9, 19 )[ 0 ] = 3;
   std::cout << img.At( 10, 20 ) << std::endl;
   std::cout << img.At( 9, 19 ) << std::endl;


   dip::Image bla( {}, 1, dip::DT_SCOMPLEX );
   bla.Fill( dip::dcomplex( 4.4, 2.3 ));
   std::cout << "bla.At() = " << bla.At< dip::dcomplex >( 0 ) << std::endl;
   std::cout << "bla.At()[0] = " << bla.At< dip::dcomplex >( 0 )[ 0 ] << std::endl;
   std::cout << "bla.As() = " << bla.As< dip::dcomplex >() << std::endl;
}
