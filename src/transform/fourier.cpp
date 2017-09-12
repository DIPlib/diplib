/*
 * DIPlib 3.0
 * This file contains definitions of the Fourier Transform function.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diplib.h"
#include "diplib/transform.h"
#include "diplib/dft.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

// TPI is either scomplex or dcomplex.
template< typename TPI >
class DFTLineFilter : public Framework::SeparableLineFilter {
   public:
      DFTLineFilter(
            UnsignedArray const& outSize,
            BooleanArray const& process,
            bool inverse, bool corner, bool symmetric
      ) : shift_( !corner ) {
         dft_.resize( outSize.size() );
         scale_ = 1.0;
         for( dip::uint ii = 0; ii < outSize.size(); ++ii ) {
            if( process[ ii ] ) {
               bool found = false;
               for( dip::uint jj = 0; jj < ii; ++jj ) {
                  if( process[ jj ] && ( outSize[ jj ] == outSize[ ii ] )) {
                     dft_[ ii ] = dft_[ jj ];
                     found = true;
                     break;
                  }
               }
               if( !found ) {
                  dft_[ ii ].Initialize( outSize[ ii ], inverse );
               }
               if( inverse || symmetric ) {
                  scale_ /= static_cast< FloatType< TPI >>( outSize[ ii ] );
               }
            }
         }
         if( symmetric ) {
            scale_ = std::sqrt( scale_ );
         }
      }
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         DFT< FloatType< TPI >> const& dft = dft_[ params.dimension ];
         if( buffers_[ params.thread ].size() != dft.BufferSize() ) {
            buffers_[ params.thread ].resize( dft.BufferSize() );
         }
         dip::uint length = dft.TransformSize();
         dip::uint border = params.inBuffer.border;
         DIP_ASSERT( params.inBuffer.length + 2 * border >= length );
         DIP_ASSERT( params.outBuffer.length >= length );
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer ) - border;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         FloatType< TPI > scale{ 1.0 };
         if( params.pass == params.nPasses - 1 ) {
            scale = scale_;
         }
         if( shift_ ) {
            ShiftCenterToCorner( in, length );
         }
         dft.Apply( in, out, buffers_[ params.thread ].data(), scale );
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }
      // The two functions below by Alexei: http://stackoverflow.com/a/19752002/7328782
      static void ShiftCornerToCenter( TPI* data, dip::uint length ) { // fftshift
         dip::uint jj = length / 2;
         if( length & 1 ) { // Odd-sized transform
            TPI tmp = data[ 0 ];
            for( dip::uint ii = 0; ii < jj; ++ii ) {
               data[ ii ] = data[ jj + ii + 1 ];
               data[ jj + ii + 1 ] = data[ ii + 1 ];
            }
            data[ jj ] = tmp;
         } else { // Even-sized transform
            for( dip::uint ii = 0; ii < jj; ++ii ) {
               std::swap( data[ ii ], data[ ii + jj ] );
            }
         }
      }
      static void ShiftCenterToCorner( TPI* data, dip::uint length ) { // ifftshift
         dip::uint jj = length / 2;
         if( length & 1 ) { // Odd-sized transform
            TPI tmp = data[ length - 1 ];
            for( dip::uint ii = jj; ii > 0; ) {
               --ii;
               data[ jj + ii + 1 ] = data[ ii ];
               data[ ii ] = data[ jj + ii ];
            }
            data[ jj ] = tmp;
         } else { // Even-sized transform
            for( dip::uint ii = 0; ii < jj; ++ii ) {
               std::swap( data[ ii ], data[ ii + jj ] );
            }
         }
      }

   private:
      std::vector< DFT< FloatType< TPI >>> dft_; // one for each dimension
      std::vector< std::vector< TPI >> buffers_; // one for each thread
      FloatType< TPI > scale_;
      bool shift_;
};

} // namespace


void FourierTransform(
      Image const& in,
      Image& out,
      StringSet const& options,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   // Read `options` set
   bool inverse = false; // forward or inverse transform?
   bool real = false; // real-valued output?
   bool fast = false; // pad the image to a "nice" size?
   bool corner = false;
   bool symmetric = false;
   for( auto& option : options ) {
      if( option == "inverse" ) {
         inverse = true;
      } else if( option == "real" ) {
         // TODO: We should probably write code to do real -> 1/2 plane complex, and 1/2 plane complex -> real DFTs.
         // TODO: If so, we'll need to write our own loop code here, we won't be able to depend on Framework::Separable (unless we add some options there...)
         real = true;
      } else if( option == "fast" ) {
         fast = true;
      } else if( option == "corner" ) {
         corner = true;
      } else if( option == "symmetric" ) {
         symmetric = true;
      } else {
         DIP_THROW_INVALID_FLAG( option );
      }
   }
   // Handle `process` array
   if( process.empty() ) {
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }
   //std::cout << "process = " << process << std::endl;
   // Determine output size and create `border` array
   UnsignedArray outSize = in.Sizes();
   UnsignedArray border( nDims, 0 );
   BoundaryConditionArray bc{ BoundaryCondition::ZERO_ORDER_EXTRAPOLATE }; // Is this the least damaging boundary condition?
   if( fast ) {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( process[ ii ] ) {
            dip::uint sz;
            sz = GetOptimalDFTSize( outSize[ ii ] ); // Awkward: OpenCV uses int a lot. We cannot handle image sizes larger than can fit in an int (2^31-1 on most platforms)
            DIP_THROW_IF( sz < 1u, "Cannot pad image dimension to a larger \"fast\" size." );
            border[ ii ] = div_ceil< dip::uint >( sz - outSize[ ii ], 2 );
            outSize[ ii ] = sz;
         }
      }
   } else {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         DIP_THROW_IF( outSize[ ii ] > maximumDFTSize, "Image size too large for DFT algorithm." );
      }
   }
   //std::cout << "outSize = " << outSize << std::endl;
   //std::cout << "border = " << border << std::endl;
   // Determine output data type
   DataType dtype = DataType::SuggestComplex( in.DataType() );
   // Allocate output image, so that it has the right (padded) size. If we don't do padding, then we're just doing the framework's work here
   Image const in_copy = in; // Make a copy of the header to preserve image in case in == out
   Image tmp;
   if( real ) {
      tmp.ReForge( outSize, 1, dtype );
   } else {
      out.ReForge( outSize, 1, dtype );
      tmp = out.QuickCopy();
   }
   // Do the processing
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, DFTLineFilter, ( outSize, process, inverse, corner, symmetric ), dtype );
      Framework::Separable( in_copy, tmp, dtype, dtype, process, border, bc, *lineFilter,
            Framework::Separable_UseInputBuffer +   // input stride is always 1
            Framework::Separable_UseOutputBuffer +  // output stride is always 1
            Framework::Separable_DontResizeOutput + // output is potentially larger than input, if padding with zeros
            Framework::Separable_AsScalarImage      // each tensor element processed separately
      );
   DIP_END_STACK_TRACE
   // Produce real-valued output
   // TODO: OpenCV has code for a DFT that takes complex data but reads only half the array, assumes symmetry, and produces a real ouput. We should use that here.
   // TODO: We should also use the code that takes real data in.
   if( real ) {
      tmp = tmp.Real();
      if(( out.DataType() != tmp.DataType() ) && ( !out.IsProtected() )) {
         out.Strip(); // Avoid accidental data conversion.
      }
      out.Copy( tmp );
   }
   // Set output pixel sizes
   PixelSize pixelSize = in_copy.PixelSize();
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         pixelSize.Scale( ii, static_cast< dfloat >( out.Size( ii )));
         pixelSize.Invert( ii );
      }
   }
   pixelSize.Resize( nDims );
   out.SetPixelSize( pixelSize );
}


dip::uint OptimalFourierTransformSize( dip::uint size ) {
   size = GetOptimalDFTSize( size );
   DIP_THROW_IF( size == 0, E::SIZE_EXCEEDS_LIMIT );
   return size;
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/random.h"

#ifndef M_PIl
#define M_PIl 3.1415926535897932384626433832795029L
#endif

template< typename T >
T dotest( size_t nfft, bool inverse = false ) {
   // Initialize
   dip::DFT< T > opts( nfft, inverse );
   std::vector< std::complex< T >> buf( opts.BufferSize() );
   // Create test data
   std::vector< std::complex< T >> inbuf( nfft );
   std::vector< std::complex< T >> outbuf( nfft );
   dip::Random random;
   for( size_t k = 0; k < nfft; ++k ) {
      inbuf[ k ] = std::complex< T >( static_cast< T >( random() ), static_cast< T >( random() ) ) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( inbuf.data(), outbuf.data(), buf.data(), T( 1 ) );
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( size_t k0 = 0; k0 < nfft; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = ( inverse ? 2.0l : -2.0l ) * ( long double )k0 * M_PIl / ( long double )nfft;
      for( size_t k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) *
                std::exp( std::complex< long double >( 0, ( k1 * phinc )));
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ));
   }
   return ( T )std::sqrt( difpower / totalpower ); // Root mean square error
}

DOCTEST_TEST_CASE("[DIPlib] testing the DFT function") {
   // Test a few different sizes that have all different radixes.
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 1024 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 1152 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 840 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 1023 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 840, true )) == 0 );
}

#endif // DIP__ENABLE_DOCTEST
