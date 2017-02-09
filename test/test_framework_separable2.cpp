#define DOCTEST_CONFIG_IMPLEMENT
#include <iostream>
#include <numeric>
#include <random>
#include <chrono>
#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/framework.h"
#include "diplib/math.h"

// Timing the separable framework.

class LineFilter : public dip::Framework::SeparableLineFilter {
   public:
      virtual void Filter( dip::Framework::SeparableLineFilterParameters const& params ) override {
         float const* filter = &( filter_[ N ] );
         dip::ConstSampleIterator< dip::sfloat > in(
               static_cast< dip::sfloat* >(params.inBuffer.buffer),
               params.inBuffer.stride );
         dip::SampleIterator< dip::sfloat > out(
               static_cast< dip::sfloat* >(params.outBuffer.buffer),
               params.outBuffer.stride );
         for( dip::uint ii = 0; ii < params.inBuffer.length; ++ii ) {
            float res = 0;
            for( dip::sint jj = -N; jj <= N; ++jj ) {
               res += in[ jj ] * filter[ jj ];
            }
            *out = res;
            ++in;
            ++out;
         }
      }
   private:
      static constexpr dip::sint N = 2;
      std::array< float, 2 * N + 1 > filter_{
            {
                  1.0f / 9.0f, 2.0f / 9.0f, 3.0f / 9.0f, 2.0f / 9.0f, 1.0f / 9.0f
            }
      };
};

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 400, 500, 300 }, 1, dip::DT_UINT16 };
      {
         DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         std::random_device rd;
         std::mt19937 gen( rd() );
         std::normal_distribution< float > normDist( 9563.0, 500.0 );
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      LineFilter lineFilter;

      // Straight-on

      dip::Image out1;
      auto start = std::chrono::steady_clock::now();
      dip::Framework::Separable(
            img, out1, dip::DT_SFLOAT, dip::DT_SFLOAT, /* process = */ {}, /* border = */ { 2 },
            { dip::BoundaryCondition::ADD_ZEROS }, &lineFilter );
      std::cout << "Straight-on: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

      // Separable_SaveMemory

      dip::Image out2;
      start = std::chrono::steady_clock::now();
      dip::Framework::Separable(
            img, out2, dip::DT_SFLOAT, dip::DT_SFLOAT, /* process = */ {}, /* border = */ { 2 },
            { dip::BoundaryCondition::ADD_ZEROS }, &lineFilter /*, dip::Framework::Separable_SaveMemory*/ );
      std::cout << "Separable_SaveMemory: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

      // in == out

      dip::Image out3;
      dip::Convert( img, out3, dip::DT_SFLOAT );
      start = std::chrono::steady_clock::now();
      dip::Framework::Separable(
            out3, out3, dip::DT_SFLOAT, dip::DT_SFLOAT, /* process = */ {}, /* border = */ { 2 },
            { dip::BoundaryCondition::ADD_ZEROS }, &lineFilter );
      std::cout << "in == out: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

      // in == out + Separable_SaveMemory

      dip::Image out4;
      dip::Convert( img, out4, dip::DT_SFLOAT );
      start = std::chrono::steady_clock::now();
      dip::Framework::Separable(
            out4, out4, dip::DT_SFLOAT, dip::DT_SFLOAT, /* process = */ {}, /* border = */ { 2 },
            { dip::BoundaryCondition::ADD_ZEROS }, &lineFilter /*, dip::Framework::Separable_SaveMemory*/ );
      std::cout << "in == out + Separable_SaveMemory: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms" << std::endl;

      if( dip::Count( out1 != out2 ) > 0 ) {
         std::cout << "out1 and out2 differ\n";
      }
      if( dip::Count( out1 != out3 ) > 0 ) {
         std::cout << "out1 and out3 differ\n";
      }
      if( dip::Count( out1 != out4 ) > 0 ) {
         std::cout << "out1 and out4 differ\n";
      }

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
