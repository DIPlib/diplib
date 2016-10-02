#include <iostream>
#include <random>
#include <typeinfo>
#include "diplib.h"
#include "diplib/overload.h"

// Just a test!

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
   try {
      std::cout << "Forging with various strides." << std::endl;
      {
         dip::Image img;
         std::cout << img;
         img.SetSizes( { 50, 80, 30 } );
         img.SetTensorSizes(3);
         img.Forge();
         std::cout << img;
         img.Strip();
         img.SetStrides({-80,-1,4000});
         img.SetTensorStride(120000);
         img.Forge();
         std::cout << img;
      }
      std::cout << std::endl << "Calling a function with overloads." << std::endl;
      {
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
      }
      std::cout << std::endl << "Indexing." << std::endl;
      {
         dip::Image img1;
         img1.SetSizes( { 50, 80, 30 } );
         img1.SetTensorSizes(3);
         img1.SetPixelSize({{dip::PhysicalQuantity::Micrometer(), 3*dip::PhysicalQuantity::Micrometer(), dip::PhysicalQuantity::Radian()}});
         img1.Forge();
         std::cout << img1;
         dip::Image img2(img1);
         std::cout << img2;
         img2 = img1.At(10,10,10);
         std::cout << img2;
         img2 = img1[1];
         std::cout << img2;
         img2 = img1[1].At(10,10,10);
         std::cout << img2;
         img2 = img1.At(10,10,10)[1];
         std::cout << img2;
         img2 = img1.At(dip::Range{},dip::Range{0,-1,4},dip::Range{10});
         std::cout << img2;
         img1.Strip();
         img1.SetDataType( dip::DT_SCOMPLEX );
         img1.Forge();
         std::cout << img1;
         img2 = img1.Imaginary();
         std::cout << img2;
      }
      std::cout << std::endl << "Reshaping." << std::endl;
      {
         dip::Image img1;
         img1.SetSizes( { 50, 80, 30 } );
         img1.SetTensorSizes(3);
         img1.SetPixelSize({{dip::PhysicalQuantity::Micrometer(), 3*dip::PhysicalQuantity::Micrometer(), dip::PhysicalQuantity::Radian()}});
         img1.Forge();
         std::cout << img1;
         img1.PermuteDimensions({2,1,0});
         std::cout << img1;
         img1.SwapDimensions(0,1);
         std::cout << img1;
         img1.Mirror({true,false,false});
         std::cout << img1;
         img1.ExpandDimensionality(5);
         std::cout << img1;
         img1.AddSingleton(0);
         std::cout << img1;
         img1.Squeeze();
         std::cout << img1;
         img1.Strip();
         img1.SetStrides({});
         img1.Forge();
         std::cout << img1;
         img1.Flatten();
         std::cout << img1;
      }
      std::cout << std::endl << "Aliasing (no output is good)." << std::endl;
      {
         dip::Image img1;
         img1.SetSizes( { 50, 80, 30 } );
         img1.SetTensorSizes(3);
         img1.Forge();
         dip::Image img2 = img1[0];
         if( Alias(img1,img2) != true )
            std::cout << "Error: aliasing computation, test #1" << std::endl;
         dip::Image img3 = img1[1];
         if( Alias(img1,img3) != true )
            std::cout << "Error: aliasing computation, test #2" << std::endl;
         if( Alias(img2,img3) != false )
            std::cout << "Error: aliasing computation, test #3" << std::endl;
         dip::Image img4 = img1.At(dip::Range{},dip::Range{},dip::Range{10});
         if( Alias(img1,img4) != true )
            std::cout << "Error: aliasing computation, test #3" << std::endl;
         dip::Image img5 = img1.At(dip::Range{},dip::Range{},dip::Range{11});
         if( Alias(img4,img5) != false )
            std::cout << "Error: aliasing computation, test #4" << std::endl;
         dip::Image img6 = img1.At(dip::Range{0,-1,2},dip::Range{},dip::Range{});
         dip::Image img7 = img1.At(dip::Range{1,-1,2},dip::Range{},dip::Range{});
         if( Alias(img1,img7) != true )
            std::cout << "Error: aliasing computation, test #5" << std::endl;
         if( Alias(img6,img7) != false )
            std::cout << "Error: aliasing computation, test #6" << std::endl;
         img7.Mirror( {true, false, false} );
         if( Alias(img6,img7) != false )
            std::cout << "Error: aliasing computation, test #7" << std::endl;
         img7.SwapDimensions( 0, 1 );
         if( Alias(img6,img7) != false )
            std::cout << "Error: aliasing computation, test #8" << std::endl;
         dip::Image img8;
         img8.SetSizes( { 50, 80, 30 } );
         img8.SetTensorSizes(3);
         img8.Forge();
         if( Alias(img1,img8) != false )
            std::cout << "Error: aliasing computation, test #9" << std::endl;
         img1.Strip();
         img1.SetDataType( dip::DT_SCOMPLEX );
         img1.Forge();
         if( Alias(img1,img1.Imaginary()) != true )
            std::cout << "Error: aliasing computation, test #10" << std::endl;
         if( Alias(img1.Real(),img1.Imaginary()) != false )
            std::cout << "Error: aliasing computation, test #11" << std::endl;
      }
      std::cout << std::endl << "Indices and offsets (no output is good)." << std::endl;
      std::default_random_engine random;
      std::uniform_int_distribution<dip::uint> rand8(1, 8);
      std::uniform_int_distribution<dip::uint> rand30(1, 30);
      std::uniform_real_distribution<double> randF(0, 1);
      for( dip::uint repeat = 0; repeat < 1000; ++repeat ) {
         dip::uint ndims = rand8( random );
         dip::UnsignedArray sz( ndims );
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            sz[ii] = rand30( random );
         }
         dip::Image img;
         img.SetSizes( sz );
         img.Forge();
         std::uniform_int_distribution<int> randD(0, ndims-1);
         for( dip::uint ii = 0; ii < rand8( random ); ++ii) {
            img.SwapDimensions( randD( random ), randD( random ));
         }
         dip::BooleanArray mirror( ndims );
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            mirror[ii] = randF( random ) > 0.7;
         }
         img.Mirror( mirror );
         //std::cout << img;
         const dip::UnsignedArray& dims = img.Sizes();
         auto o2c = img.OffsetToCoordinatesComputer();
         auto i2c = img.IndexToCoordinatesComputer();
         for( dip::uint repeat2 = 0; repeat2 < 100; ++repeat2 ) {
            dip::UnsignedArray coords( ndims );
            for( dip::uint ii = 0; ii < ndims; ++ii ) {
               coords[ii] = (dip::uint)std::floor( randF( random ) * dims[ii] );
            }
            dip::sint offset = img.Offset( coords );
            if( o2c( offset ) != coords ) {
               std::cout << "Error: offset to coordinates computation" << std::endl;
               std::cout << img;
               std::cout << "   coords = ";
               for( dip::uint ii = 0; ii < coords.size(); ++ii ) std::cout << coords[ii] << ", ";
               std::cout << "\n   offset = " << offset << std::endl;
               coords = o2c( offset );
               std::cout << "   coords = ";
               for( dip::uint ii = 0; ii < coords.size(); ++ii ) std::cout << coords[ii] << ", ";
               return 2;
            }
            dip::sint index = img.Index( coords );
            if( i2c( index ) != coords ) {
               std::cout << "Error: index to coordinates computation" << std::endl;
               std::cout << img;
               std::cout << "   coords = ";
               for( dip::uint ii = 0; ii < coords.size(); ++ii ) std::cout << coords[ii] << ", ";
               std::cout << "\n   index = " << index << std::endl;
               coords = i2c( index );
               std::cout << "   coords = ";
               for( dip::uint ii = 0; ii < coords.size(); ++ii ) std::cout << coords[ii] << ", ";
               return 2;
            }
         }
      }

   } catch( dip::Error e ) {
      std::cout << e.what() << std::endl;
   }
   return 1;
}
