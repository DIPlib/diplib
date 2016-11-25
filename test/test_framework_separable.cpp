#include <iostream>
#include <numeric>
#include <random>
#include "diplib.h"
#include "diplib/iterators.h"
#include "diplib/framework.h"

// Testing the separable framework and the line-by-line iterators.

template< typename T >
void PrintPixelValues(
      dip::Image img
) {
   dip_ThrowIf( img.DataType() != dip::DataType( T() ), "Wrong version of PrintPixelValues() called" );
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

void lineFilter(
      dip::Framework::SeparableBuffer const& inBuffer,
      dip::Framework::SeparableBuffer& outBuffer,
      dip::uint dimension, // unused
      dip::UnsignedArray const& position, // unused
      void const* functionParameters, // unused
      void* functionVariables // unused
) {
   // struct SeparableBuffer {
   //    void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   //    dip::uint length;       ///< Length of the buffer, not counting the expanded boundary
   //    dip::uint border;       ///< Length of the expanded boundary at each side of the buffer.
   //    dip::sint stride;       ///< Stride to walk along pixels.
   //    dip::sint tensorStride; ///< Stride to walk along tensor elements.
   //    dip::uint tensorLength; ///< Number of tensor elements.
   // };
   constexpr dip::sint N = 2;
   constexpr std::array< float, 2 * N + 1 > filter_{ { 1.0f / 9.0f, 2.0f / 9.0f, 3.0f / 9.0f, 2.0f / 9.0f, 1.0f / 9.0f } };
   float const* filter = &( filter_[ N ] );
   dip::ConstSampleIterator< dip::sfloat > in ( static_cast< dip::sfloat* >(inBuffer.buffer), inBuffer.stride );
   dip::SampleIterator< dip::sfloat > out ( static_cast< dip::sfloat* >(outBuffer.buffer), outBuffer.stride );
   for( dip::uint ii = 0; ii < inBuffer.length; ++ii ) {
      float res = 0;
      for( dip::sint jj = -N; jj <= N; ++jj ) {
         res += in[ jj ] * filter[ jj ];
      }
      *out = res;
      ++in;
      ++out;
   }
}

int main() {
   try {
      dip::Image img{ dip::UnsignedArray{ 20, 15 }, 1, dip::DT_UINT16 };
      {
         dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         std::random_device rd;
         std::mt19937 gen(rd());
         std::normal_distribution<float> normDist(9563.0, 500.0);
         dip::ImageIterator< dip::uint16 > it( img );
         do {
            *it = dip::clamp_cast< dip::uint16 >( normDist( gen ));
         } while( ++it );
      }

      PrintPixelValues< dip::uint16 >( img );

      {
         // Copied from src/documentation/iterators.md
         dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip::ImageIterator< dip::uint16 > it( img, 0 );
         do {
            auto lit = it.GetLineIterator();
            dip::uint sum = 0;
            do {
               sum += *lit;
            } while( ++lit );
            dip::uint mean = sum / lit.Length();
            lit = it.GetLineIterator();
            do {
               dip::uint res = mean == 0 ? 0 : ( *lit * 1000 ) / mean;
               *lit = dip::clamp_cast< dip::uint16 >( res );
            } while( ++lit );
         } while( ++it );
      }

      PrintPixelValues< dip::uint16 >( img );

      dip::Image out = img.Similar( dip::DT_SFLOAT );
      {
         // Copied from src/documentation/iterators.md
         dip_ThrowIf( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
         dip_ThrowIf( out.DataType() != dip::DT_SFLOAT, "Expecting single-precision float image" );
         constexpr dip::uint N = 2;
         std::array< double, 2 * N + 1 > filter{ { 1.0 / 9.0, 2.0 / 9.0, 3.0 / 9.0, 2.0 / 9.0, 1.0 / 9.0 } };
         dip::JointImageIterator< dip::uint16, dip::sfloat > it( img, out, 0 );
         do {
            auto iit = it.GetInLineIterator();
            auto oit = it.GetOutLineIterator();
            // At the beginning of the line the filter has only partial support within the image
            for( dip::uint ii = N; ii > 0; --ii, ++oit ) {
               *oit = std::inner_product( filter.begin() + ii, filter.end(), iit, 0.0f );
            }
            // In the middle of the line the filter has full support
            for( dip::uint ii = N; ii < oit.Length() - N; ++ii, ++iit, ++oit ) {
               *oit = std::inner_product( filter.begin(), filter.end(), iit, 0.0f );
            }
            // At the end of the line the filter has only partial support
            for( dip::uint ii = 1; ii <= N; ++ii, ++iit, ++oit ) {
               *oit = std::inner_product( filter.begin(), filter.end() - ii, iit, 0.0f );
            }
         } while( ++it );
      }

      PrintPixelValues< dip::sfloat >( out );

      dip::Framework::Separable( img, out, dip::DT_SFLOAT, dip::DT_SFLOAT,
            dip::BooleanArray{ true, false }, { 2 },
            dip::BoundaryConditionArray{ dip::BoundaryCondition::ADD_ZEROS },
            lineFilter, nullptr, std::vector< void* > {}, dip::Framework::Separable_AsScalarImage);

      PrintPixelValues< dip::sfloat >( out );

   } catch( dip::Error e ) {
      std::cout << "DIPlib error: " << e.what() << std::endl;
      return 1;
   }
   return 0;
}
