/*
 * DIPlib 3.0
 * This file contains definitions of the Fourier Transform function.
 *
 * (c)2017-2020, Cris Luengo, Erik Schuitema
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
#include "diplib/multithreading.h"
#include "diplib/geometry.h"

#ifdef DIP__HAS_FFTW
   #ifdef _WIN32
      #define NOMINMAX // windows.h must not define min() and max(), which conflict with std::min() and std::max()
   #endif
   #include "fftw3api.h"
#endif

namespace dip {

namespace {


#ifdef DIP__HAS_FFTW

// FFTW documentation specifies 16-byte alignment required for SIMD implementations:
// http://www.fftw.org/fftw3_doc/SIMD-alignment-and-fftw_005fmalloc.html#SIMD-alignment-and-fftw_005fmalloc
constexpr dip::uint FFTW_MAX_ALIGN_REQUIRED = 16;

// This is the equivalent of dip::DFT, but encapsulating FFTW functionality
template< typename T >
class FFTW {
   public:
      using complex = typename fftwapidef< T >::complex;

      /// \brief A default-initialized `%FFTW` object is useless. Call `Initialize` to make it useful.
      FFTW() = default;

      /// \brief Equivalent to calling `Initialize()` on a default-initialized object.
      FFTW( dip::uint size, bool inverse ) { this->Initialize( size, inverse ); }

      ~FFTW() {
         if( plan_ ) {
            fftwapidef< T >::destroy_plan( plan_ );
         }
      }

      /// \brief Re-configure a `%FFTW` object to the given transform size and direction.
      /// Note that this is not a trivial operation.
      /// This function is not thread safe, as it calls the FFTW planner.
      void Initialize( dip::uint size, bool inverse ) {
         nfft_ = size;
         int sign = inverse ? FFTW_BACKWARD : FFTW_FORWARD;
         std::vector< std::complex< T >> in( size );
         std::vector< std::complex< T >> out( size ); // allocate temporary arrays just for planning...
         plan_ = fftwapidef< T >::plan_dft_1d(
               static_cast< int >( size ),
               reinterpret_cast< complex* >( in.data() ),
               reinterpret_cast< complex* >( out.data() ),
               sign, FFTW_MEASURE | FFTW_UNALIGNED );
         // FFTW_MEASURE is almost always faster than FFTW_ESTIMATE, only for very trivial sizes it's not.
         // TODO: Remove FFTW_UNALIGNED by ensuring that the buffers are 16-byte aligned.
         //       This requires aligning buffers created by the Separable framework.
         // TODO: Separable framework needs an option for 16-byte boundary aligned buffers.
      }

      /// \brief Apply the transform that the `%FFTW` object is configured for.
      ///
      /// `source` and `destination` are pointers to contiguous buffers with the appropriate number of
      /// elements for a transform of size `TransformSize`. This is the value of the `size` parameter of
      /// the constructor or `Initialize`.
      ///
      /// `scale` is a real scalar that the output values are multiplied by. It is typically set to `1/size` for
      /// the inverse transform, and 1 for the forward transform.
      ///
      /// This function is thread-safe, as opposed to `Initialse()`.
      void Apply(
            std::complex< T >* source,
            std::complex< T >* destination,
            T scale
      ) const {
         fftwapidef< T >::execute_dft( plan_, reinterpret_cast< complex* >( source ), reinterpret_cast< complex* >( destination ));
         for( std::complex< T >* ptr = destination; ptr < destination + nfft_; ++ptr ) {
            *ptr *= scale;
         }
      }

      /// \brief Returns the size that the transform is configured for.
      dip::uint TransformSize() const { return nfft_; }

   protected:
      dip::uint nfft_ = 0;
      typename fftwapidef< T >::plan plan_ = nullptr;
};

#endif


// TPI is either scomplex or dcomplex.
template< typename TPI >
class DFTLineFilter : public Framework::SeparableLineFilter {
   public:
      DFTLineFilter(
            UnsignedArray const& outSize,
            BooleanArray const& process,
            bool inverse, bool corner, bool symmetric
      ) : shift_( !corner ) {
#ifdef DIP__HAS_FFTW
         fftw_.resize( outSize.size() );
#else
         dft_.resize( outSize.size() );
#endif
         scale_ = 1.0;
         for( dip::uint ii = 0; ii < outSize.size(); ++ii ) {
            if( process[ ii ] ) {
#ifdef DIP__HAS_FFTW
               // FFTW re-uses plans internally
               fftw_[ ii ].Initialize( outSize[ ii ], inverse );
#else
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
#endif
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
#ifdef DIP__HAS_FFTW
         (void) threads;
#else
         buffers_.resize( threads );
#endif
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
#ifdef DIP__HAS_FFTW
         auto const& fftw = fftw_[ params.dimension ];
         dip::uint length = fftw.TransformSize();
#else
         auto const& dft = dft_[ params.dimension ];
         if( buffers_[ params.thread ].size() != dft.BufferSize() ) {
            buffers_[ params.thread ].resize( dft.BufferSize() );
         }
         dip::uint length = dft.TransformSize();
#endif
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
#ifdef DIP__HAS_FFTW
         fftw.Apply( in, out, scale );
#else
         dft.Apply( in, out, buffers_[ params.thread ].data(), scale );
#endif
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }
      // The two functions below by Alexei: http://stackoverflow.com/a/19752002/7328782
      static void ShiftCornerToCenter( TPI* data, dip::uint length ) { // fftshift
         dip::uint jj = length / 2;
         if( length & 1u ) { // Odd-sized transform
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
         if( length & 1u ) { // Odd-sized transform
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
#ifdef DIP__HAS_FFTW
      std::vector< FFTW< FloatType< TPI >>> fftw_; // one for each dimension
#else
      std::vector< DFT< FloatType< TPI >>> dft_; // one for each dimension
      std::vector< std::vector< TPI >> buffers_; // one for each thread
#endif
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
   bool real = out.IsProtected() && !out.DataType().IsComplex(); // real-valued output?
   bool fast = false; // pad the image to a "nice" size?
   bool corner = false;
   bool symmetric = false;
   for( auto const& option : options ) {
      if( option == S::INVERSE ) {
         inverse = true;
      } else if( option == S::REAL ) {
         real = true;
      } else if( option == S::FAST ) {
         fast = true;
      } else if( option == S::CORNER ) {
         corner = true;
      } else if( option == S::SYMMETRIC ) {
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
            dip::uint sz = GetOptimalDFTSize( outSize[ ii ] ); // Awkward: OpenCV uses int a lot. We cannot handle image sizes larger than can fit in an int (2^31-1 on most platforms)
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

   Image const in_copy = in; // Make a copy of the header to preserve image in case in == out

   // Determine output data type
   DataType dtype = DataType::SuggestComplex( in.DataType() );
   // Allocate output image, so that it has the right (padded) size. If we don't do padding, then we're just doing the framework's work here
   Image tmp;
   if( real ) {
      tmp.ReForge( outSize, in_copy.TensorElements(), dtype );
   } else {
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), dtype, Option::AcceptDataTypeChange::DO_ALLOW ));
      tmp = out.QuickCopy();
   }
   tmp.Protect(); // make sure it won't be reforged by the framework function.
   // Do the processing
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, DFTLineFilter, ( outSize, process, inverse, corner, symmetric ), dtype );
      Framework::Separable( in_copy, tmp, dtype, dtype, process, border, bc, *lineFilter,
            Framework::SeparableOption::UseInputBuffer +   // input stride is always 1
            Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1
            Framework::SeparableOption::DontResizeOutput + // output is potentially larger than input, if padding with zeros
            Framework::SeparableOption::AsScalarImage      // each tensor element processed separately
      );
   DIP_END_STACK_TRACE
   tmp.Protect( false );
   // Produce real-valued output
   // TODO: OpenCV has code for a Real->Complex DFT and for a Complex->Real DFT. We should use that here.
   //       FFTW has equivalent functions that we can call.
   if( real ) {
      tmp = tmp.Real();
      if(( out.DataType() != tmp.DataType() ) && ( !out.IsProtected() )) {
         out.Strip(); // Avoid accidental data conversion.
      }
      out.Copy( tmp );
   }

   // Set output tensor shape
   out.ReshapeTensor( in_copy.Tensor() );

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

   // Set output color space
   if( in_copy.IsColor() ) {
      out.SetColorSpace( in_copy.ColorSpace() );
   }
}


dip::uint OptimalFourierTransformSize( dip::uint size ) {
   // OpenCV's optimal size can be factorized into small primes: 2, 3, and 5.
   // FFTW performs best with sizes that can be factorized into 2, 3, 5, and 7.
   // For FFTW, we'll stick with the OpenCV implementation.
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
T dotest( std::size_t nfft, bool inverse = false ) {
   // Initialize
   dip::DFT< T > opts( nfft, inverse );
   std::vector< std::complex< T >> buf( opts.BufferSize() );
   // Create test data
   std::vector< std::complex< T >> inbuf( nfft );
   std::vector< std::complex< T >> outbuf( nfft );
   dip::Random random;
   for( std::size_t k = 0; k < nfft; ++k ) {
      inbuf[ k ] = std::complex< T >( static_cast< T >( random() ), static_cast< T >( random() )) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( inbuf.data(), outbuf.data(), buf.data(), T( 1 ));
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( std::size_t k0 = 0; k0 < nfft; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = ( inverse ? 2.0l : -2.0l ) * static_cast< long double >( k0 ) * M_PIl / static_cast< long double >( nfft );
      for( std::size_t k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) *
                std::exp( std::complex< long double >( 0, k1 * phinc ));
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ));
   }
   return static_cast< T >( std::sqrt( difpower / totalpower )); // Root mean square error
}

DOCTEST_TEST_CASE("[DIPlib] testing the DFT function") {
   // Test a few different sizes that have all different radixes.
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 256 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 105 )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 154 )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 97 )) == 0 ); // prime
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 105, true )) == 0 );
}

#ifdef DIP__HAS_FFTW

template< typename T >
T dotest_FFTW( std::size_t nfft, bool inverse = false ) {
   // Initialize
   dip::FFTW< T > opts( nfft, inverse );
   // Create test data
   std::vector< std::complex< T >> inbuf( nfft );
   std::vector< std::complex< T >> outbuf( nfft );
   dip::Random random;
   for( std::size_t k = 0; k < nfft; ++k ) {
      inbuf[ k ] = std::complex< T >( static_cast< T >( random() ), static_cast< T >( random() )) / static_cast< T >( random.max() ) - T( 0.5 );
   }
   // Do the thing
   opts.Apply( inbuf.data(), outbuf.data(), T( 1 ));
   // Check
   long double totalpower = 0;
   long double difpower = 0;
   for( std::size_t k0 = 0; k0 < nfft; ++k0 ) {
      std::complex< long double > acc{ 0, 0 };
      long double phinc = ( inverse ? 2.0l : -2.0l ) * static_cast< long double >( k0 ) * M_PIl / static_cast< long double >( nfft );
      for( std::size_t k1 = 0; k1 < nfft; ++k1 ) {
         acc += std::complex< long double >( inbuf[ k1 ] ) *
                std::exp( std::complex< long double >( 0, k1 * phinc ));
      }
      totalpower += std::norm( acc );
      difpower += std::norm( acc - std::complex< long double >( outbuf[ k0 ] ));
   }
   return static_cast< T >( std::sqrt( difpower / totalpower )); // Root mean square error
}

DOCTEST_TEST_CASE("[DIPlib] testing the FFTW integration") {
   // Test a few different sizes that have all different radixes.
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< float >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< double >( 32 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< double >( 256 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< float >( 105 )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< double >( 154 )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< float >( 97 )) == 0 ); // prime
   DOCTEST_CHECK( doctest::Approx( dotest_FFTW< double >( 105, true )) == 0 );
}

#endif // DIP__HAS_FFTW

#endif // DIP__ENABLE_DOCTEST
