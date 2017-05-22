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
         std::mt19937 gen;
         std::normal_distribution< float > normDist( 9563.0, 500.0 );
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      PrintPixelValues< dip::uint16 >( img );

      dip::Image out = img.Similar( dip::DT_UINT16 );
      {
         // Copied from src/documentation/iterators.md
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         DIP_THROW_IF( out.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip::PixelTable kernel( "elliptic", { 5, 5 } );
         dip::ImageIterator< dip::uint16 > it( img );
         dip::ImageIterator< dip::uint16 > ot( out );
         do {
            dip::uint value = 0;
            for( auto kit = kernel.begin(); kit != kernel.end(); ++kit ) {
               dip::uint16 pix; // If the image is not scalar, we need to provide an array here.
               it.PixelAt( *kit, &pix );
               value += pix;
            }
            *ot = static_cast< dip::uint16 >( value / kernel.NumberOfPixels() );
         } while( ++ot, ++it );
      }

      PrintPixelValues< dip::uint16 >( out );

      {
         // Copied from src/documentation/iterators.md
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         DIP_THROW_IF( out.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip::Image in = dip::ExtendImage( img, { 2, 2 }, {}, { "masked" } ); // a copy of the input image with data ouside of its domain
         dip::PixelTable kernel( "elliptic", { 5, 5 }, 0 );
         dip::PixelTableOffsets offsets = kernel.Prepare( in );
         dip::JointImageIterator< dip::uint16, dip::uint16 > it( { in, out }, 0 );
         dip::sint inStride = in.Stride( 0 );
         do {
            auto iit = it.GetLineIterator< 0 >();
            auto oit = it.GetLineIterator< 1 >();
            // Compute the sum across all pixels in the kernels for the first point on the line only
            dip::uint value = 0;
            for( auto offset : offsets ) {
               value += *( iit.Pointer() + offset );
            }
            *oit = static_cast< dip::uint16 >( value / kernel.NumberOfPixels() );
            ++oit;
            do {
               // Subtract the pixels that will exit the kernel when it moves
               // Add the pixels that will enter the kernel when it moves
               for( auto run : offsets.Runs() ) {
                  value -= *( iit.Pointer() + run.offset );
                  value += *( iit.Pointer() + run.offset + static_cast< dip::sint >( run.length ) * inStride );
               }
               *oit = static_cast< dip::uint16 >( value / kernel.NumberOfPixels() );
            } while( ++iit, ++oit ); // the two images are of the same size, the line iterators reach the end at the same time
         } while( ++it );
      }

      PrintPixelValues< dip::uint16 >( out );

      LineFilter lineFilter;
      dip::Framework::Full(
            img, out, dip::DT_SFLOAT, dip::DT_SFLOAT, dip::DT_SFLOAT, 1,
            dip::BoundaryConditionArray{ dip::BoundaryCondition::SYMMETRIC_MIRROR },
            dip::FloatArray{ 5, 7 }, lineFilter, dip::Framework::Full_AsScalarImage );

      PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
