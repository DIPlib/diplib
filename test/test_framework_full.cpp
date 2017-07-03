#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include "diplib.h"
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/iterators.h"
#include "diplib/testing.h"

// Testing the full framework.

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
      img.Fill( 50 );
      dip::Random random( 0 );
      dip::GaussianNoise( img, img, random, 20.0 * 20.0 );

      dip::testing::PrintPixelValues< dip::uint16 >( img );

      dip::Image out = img.Similar( dip::DT_UINT16 );
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

      dip::testing::PrintPixelValues< dip::uint16 >( out );

      LineFilter lineFilter;
      dip::Framework::Full(
            img, out, dip::DT_SFLOAT, dip::DT_SFLOAT, dip::DT_SFLOAT, 1,
            dip::BoundaryConditionArray{ dip::BoundaryCondition::SYMMETRIC_MIRROR },
            dip::FloatArray{ 5, 7 }, lineFilter, dip::Framework::Full_AsScalarImage );

      dip::testing::PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
