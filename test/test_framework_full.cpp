#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <numeric>
#include <random>
#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/framework.h"

// Testing the full framework.

template< typename T >
void PrintPixelValues(
      dip::Image img
) {
   DIP_THROW_IF( img.DataType() != dip::DataType( T() ), "Wrong version of PrintPixelValues() called" );
   dip::uint linelength = img.Size( 0 );
   std::cout << "Image of size " << linelength << " x " << img.Sizes().product() / linelength << ":\n";
   dip::ImageIterator< T > it( img, 0 );
   dip::uint line = 0;
   do {
      auto lit = it.GetLineIterator();
      std::cout << line << ": " << *lit;
      while( ++lit ) {
         std::cout << ", " << *lit;
      }
      std::cout << std::endl;
      ++line;
   } while( ++it );
}

class LineFilter : public dip::Framework::FullLineFilter {
   public:
      virtual void Filter( dip::Framework::FullLineFilterParameters const& params ) override {
         dip::ConstSampleIterator< dip::sfloat > in(
               static_cast< dip::sfloat* >(params.inBuffer.buffer),
               params.inBuffer.stride );
         dip::SampleIterator< dip::sfloat > out(
               static_cast< dip::sfloat* >(params.outBuffer.buffer),
               params.outBuffer.stride );
         if( params.pixelTable.HasWeights() ) {
            for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
               dip::sfloat res = 0;
               auto filter = params.pixelTable.begin();
               auto weight = params.pixelTable.Weights().begin();
               do {
                  res += static_cast< dip::sfloat >( in[ *filter ] * *weight );
               } while( ++weight, ++filter );
               *out = res;
               ++in;
               ++out;
            }
         } else {
            for( dip::uint ii = 0; ii < params.bufferLength; ++ii ) {
               dip::sfloat res = 0;
               auto filter = params.pixelTable.begin();
               do {
                  res += in[ *filter ];
               } while( ++filter );
               *out = res / static_cast< dip::sfloat >( params.pixelTable.NumberOfPixels() );
               ++in;
               ++out;
            }
         }
      }
};


int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      {
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         std::mt19937 gen( 0 );
         std::normal_distribution< float > normDist( 9563.0, 500.0 );
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      PrintPixelValues< dip::uint16 >( img );

      LineFilter lineFilter;
      dip::Image out;
      //out.SetDataType( dip::DT_UINT16 );
      //out.Protect();
      dip::Framework::Full(
            img, out, dip::DT_SFLOAT, dip::DT_SFLOAT, dip::DT_SFLOAT, 1,
            dip::BoundaryConditionArray{ dip::BoundaryCondition::SYMMETRIC_MIRROR },
            dip::FloatArray{ 5, 7 }, lineFilter, dip::Framework::Full_AsScalarImage );

      PrintPixelValues< dip::sfloat >( out );
      //PrintPixelValues< dip::uint16 >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
