/*
 * DIPlib 3.0
 * This file contains definitions of the Fourier Transform function.
 *
 * (c)2017, Cris Luengo, Erik Schuitema
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
#include "diplib/iterators.h"
#include "diplib/geometry.h"

#ifdef DIP__HAS_FFTW
   #ifdef _WIN32
      #define NOMINMAX // windows.h must not define min() and max(), which are conflicting with std::min() and std::max()
   #endif
   #include "fftw3api.h"
#endif

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

#ifdef DIP__HAS_FFTW

namespace {

#define FFTW_MAX_ALIGN_REQUIRED 64  // TODO: how to determine this number?

// Singleton class that performs FFTW threading initialization and cleanup just once.
// Before calling *any* FFTW function, initialize in a similar way to:
// ```cpp
//     int fftwThreadingInitResult = FFTWThreading< FloatType >::GetInstance()->GetInitResult();
//     // verify that fftwThreadingInitResult != 0
// ```
template< typename FloatType >
class FFTWThreading
{
private:
   FFTWThreading() {
      // Initialization must be done for the specific float type
      // Initialization is called from the user of the shared library: the executable
      initResult_ = fftwapidef< FloatType >::init_threads();
   }

   int initResult_;
public:
   ~FFTWThreading() {
      // Destruction of the singleton is called by the shared library
      // and causes a lockup :( .. disabled for now.
      //fftwapidef< FloatType >::cleanup_threads();
   }

   // Singleton interface
   static FFTWThreading* GetInstance() {
      static FFTWThreading singleton;
      return &singleton;
   }

   // Returns a non-zero value upon successful init and zero if there was some error
   int GetInitResult() const {
      return initResult_;
   }

   int GetOptimalNumThreads( UnsignedArray const& outSize ) const {
      // TODO: find heuristic. The FFTW 3 manual says:
      // "You will have to experiment with your system to see what level of parallelization is best
      // for your problem size. Typically, the problem will have to involve at least a few thousand
      // data points before threads become beneficial. If you plan with FFTW_PATIENT, it will
      // automatically disable threads for sizes that don't benefit from parallelization."
      if( outSize.product() < 2000 )
         return 1;
      else
         return omp_get_max_threads();
   }
};

// Force FFTW threading initialization at shared library creation
//static FFTWThreading<float>* p1 = FFTWThreading<float>::GetInstance();
//static FFTWThreading<double>* p2 = FFTWThreading<double>::GetInstance();

// FFTW helper class.
// See derived types for different transform types for more details.
//
// All transform variants (R2R, R2C, C2C, C2R) operate in-place, i.e.,
// the input data is overwritten by the transformed data.
// There are two reasons:
// 1) Planning with FFTW_MEASURE destroys the input/output data while planning (so does C2R while transforming)
// 2) The input is often modified before doing the transform
// Operations that require input processing are:
// - Integral to float conversion
// - Shifting the origin
// - Normalization (done while copying the input in order to save a post processing step)
//
// First, the output is forged. Next, FFTW plans with the FFTW_MEASURE flag on uninitialized data in the
// output image. Finally, the output image is filled with the (processed) input data and the
// transform is done in-place. This eliminates all needs for an intermediate image.
template< class fftwapi >
class FFTWHelper
{
public:
   FFTWHelper( Image const& in, Image& out )
      : floatType_( static_cast< typename fftwapi::real >(0) )
      , complexType_( DataType::SuggestComplex( floatType_ ) )
      , in_( in )
      , out_( out )
      , largestProcessedDim_( 0 )
      , inputTensorStride_( 0 )
      , transformScale_( 1.0 )
   {}

   // Prepare input/output dimension descriptors
   // Requires calling HandleProcessingDims() and ForgeOutput() first.
   // Input strides are taken from `inputStrides_` and `inputTensorStride_`, output strides are taken from `out_`.
   // Sizes are taken from `out_`.
   virtual void PrepareIODims() {
      DIP_THROW_IF( inputStrides_.empty(), "FFTWHelper did not set inputStrides_" );
      DIP_THROW_IF( inputTensorStride_ == 0, "FFTWHelper did not set inputTensorStride_" );

      // Prepare iodim structs for in-place operation
      sizeDims_.resize( out_.Dimensionality() );
      for( int iSizeDim = 0; iSizeDim < sizeDims_.size(); ++iSizeDim ) {
         sizeDims_[ iSizeDim ].n = static_cast<int>(out_.Size( iSizeDim ));    // Number of elements
         sizeDims_[ iSizeDim ].is = static_cast<int>(inputStrides_[ iSizeDim ]);  // Input stride
         sizeDims_[ iSizeDim ].os = static_cast<int>(out_.Stride( iSizeDim ));  // Output stride
      }

      // Repeat along the tensor dimension
      // If the tensor has just 1 element, this also works fine.
      // We use the tensor entry to create a non-empty repeatDims array for easy parameter passing when all dims are processed.
      if( dimsNotProcessed_.empty() || in_.TensorElements() > 1 ) {
         repeatDims_.resize( 1 );
         for( int iTensorEl = 0; iTensorEl < repeatDims_.size(); ++iTensorEl ) {
            repeatDims_[ iTensorEl ].n = static_cast<int>(out_.TensorElements());   // Number of tensor elements
            repeatDims_[ iTensorEl ].is = static_cast<int>(inputTensorStride_);  // Input tensor stride
            repeatDims_[ iTensorEl ].os = static_cast<int>(out_.TensorStride()); // Output tensor stride
         }
      }
      else {
         repeatDims_.clear();
      }

      //  Repeat along dimensions not processed
      if( !dimsNotProcessed_.empty() ) {
         // Do reverse iteration for easy erasing of vector entries
         for( UnsignedArray::const_reverse_iterator itDimNot = dimsNotProcessed_.rbegin(); itDimNot != dimsNotProcessed_.rend(); ++itDimNot ) {
            repeatDims_.push_back( sizeDims_[*itDimNot] ); // Move size dim to repeat dim
            sizeDims_.erase( sizeDims_.begin() + *itDimNot ); // Erase size dim
         }
      }
   }

   // Do the handling related to the selection of dimensions that are processed
   virtual void HandleProcessingDims( BooleanArray const& process ) {
      // Handle processing dims.
      // If a dimension is excluded from the transform,
      // it is added as a 'repeat' dimension. For example, in 2D, if Y is excluded,
      // the transform is performed in the X-direction only and repeated along the Y-dimension.
      // 
      // Keep track of the largest dimension that is processed.
      // It is used in completing the R2C image and in C2R padding.
      largestProcessedDim_ = 0;
      // Keep track of the scaling that the transform will cause
      transformScale_ = 1.0;
      dimsNotProcessed_.clear();
      dimsProcessed_.clear();
      for( dip::uint iDim = 0; iDim < in_.Dimensionality(); ++iDim ) {
         if( process[iDim] ) {
            dimsProcessed_.push_back( iDim );   // Keep track of processed dims
            largestProcessedDim_ = iDim;  // Keep track of largest processed dim
            transformScale_ *= static_cast< typename fftwapi::real >(in_.Size(iDim)); // TODO: change this to outSize when supporting padding
         }
         else {
            dimsNotProcessed_.push_back( iDim ); // Keep track of dims not processed
         }
      }
   }

   // Forge the output image
   virtual void ForgeOutput( UnsignedArray const& outSize ) = 0;

   // Prepare the input data.
   // This involves copying, scaling and shifting into the output buffer for in-place transformation.
   // Note that this must be done after creating an FFTW plan, because planning can destroy the input/output buffer.
   virtual void PrepareInput( bool inverse, bool symmetricNormalization, bool shiftOriginToCenter ) = 0;

   // Finalize the output, e.g., complete the R2C output image with only N/2+1 sub-images
   // Scaling is done while preparing the input, so before the transform
   virtual void FinalizeOutput( bool shiftOriginToCenter ) {} // Not always necessary

   // Create the FFTW plan
   // The FFTW_MEASURE flag is advised. Initialization takes longer, but is done only once by FFTW.
   // No re-measuring is done for subsequent calls with the same sizes.
   virtual typename fftwapi::plan CreatePlan( bool inverse ) = 0;

protected:
   // Define dip's float type and complex type
   DataType floatType_;
   DataType complexType_;

   Image const& in_;
   Image& out_;

   std::vector< typename fftwapi::iodim > sizeDims_;
   std::vector< typename fftwapi::iodim > repeatDims_;
   UnsignedArray dimsNotProcessed_; // Indices of dimension not processed
   UnsignedArray dimsProcessed_; // Indices of processed dimensions

   dip::uint largestProcessedDim_;

   IntegerArray inputStrides_;   // Strides of the in-place input (note: data is located in out_, due to in-place processing)
   dip::sint inputTensorStride_;

   typename fftwapi::real transformScale_;   // Scale caused by the transform

   ExternalInterface* GetOutputExternalInterface() {
      // If out_ already contains an external interface, use it.
      // Otherwise, use aligned allocator for better FFTW performance.
      if( out_.HasExternalInterface() )
         return out_.ExternalInterface();
      else
         return AlignedAllocInterface::GetInstance<FFTW_MAX_ALIGN_REQUIRED>();
   }

   // Shift center to corner. Done before the transform. Also see Matlab's ifftshift().
   // One dimension, `fixedDim`, can be left untouched while shifting.
   // All dimensions are shifted by default or by passing -1.
   void ShiftCenterToCorner( Image& img, dip::uint fixedDim = -1 ) {
      // Wrap 'backward'
      IntegerArray wrapShifts( img.Dimensionality() );
      for( int iDim = 0; iDim < wrapShifts.size(); ++iDim ) {
         wrapShifts[iDim] = -static_cast<dip::sint>(img.Size( iDim )) / 2;   // Note the minus sign
      }
      // Set fixed dim shift to zero
      if( fixedDim != -1 ) {
         wrapShifts[fixedDim] = 0;
      }

      // Do in-place Wrap
      Wrap( img, img, wrapShifts );
   }

   // Shift corner to center. Done after the transform. Also see Matlab's fftshift().
   static void ShiftCornerToCenter( Image& img ) {
      // Wrap 'forward'
      IntegerArray wrapShifts( img.Dimensionality() );
      for( int iDim = 0; iDim < wrapShifts.size(); ++iDim ) {
         wrapShifts[iDim] = static_cast<dip::sint>(img.Size( iDim )) / 2;
      }

      // Do in-place Wrap
      Wrap( img, img, wrapShifts );
   }
};

// FFTW helper class for real to real transforms.
// Not yet implemented.
template< class fftwapi >
class FFTWHelperR2R : public FFTWHelper< fftwapi >
{
      using FFTWHelper< fftwapi >::sizeDims_;
      using FFTWHelper< fftwapi >::repeatDims_;
      using FFTWHelper< fftwapi >::out_;
public:
   FFTWHelperR2R( Image const& in, Image& out ) : FFTWHelper< fftwapi >( in, out ) {
      DIP_THROW( "FFTW R2R not yet supported" );
   }

   virtual void ForgeOutput( UnsignedArray const& outSize ) override {
      DIP_THROW( "FFTW R2R not yet supported" );
   }

   virtual void PrepareInput( bool inverse, bool symmetricNormalization, bool shiftOriginToCenter ) override {
      DIP_THROW( "FFTW R2R not yet supported" );
   }

   virtual typename fftwapi::plan CreatePlan( bool inverse ) override {
      std::vector< typename fftwapi::r2r_kind > r2rKinds( sizeDims_.size(), FFTW_REDFT10 );  // TODO: what kind of R2R transform is requested?
      return fftwapi::plan_guru_r2r( static_cast<int>( sizeDims_.size() ), &sizeDims_[0], static_cast<int>( repeatDims_.size() ), &repeatDims_[0],
         (typename fftwapi::real*)out_.Origin(), (typename fftwapi::real*)out_.Origin(), &r2rKinds[0], FFTW_MEASURE );
   }
};

// FFTW helper class for real to complex transforms
//
// On interpreting FFTW real-to-complex results:
// http://www.fftw.org/fftw3_doc/Multi_002dDimensional-DFTs-of-Real-Data.html
template< class fftwapi >
class FFTWHelperR2C : public FFTWHelper< fftwapi >
{
      using FFTWHelper< fftwapi >::sizeDims_;
      using FFTWHelper< fftwapi >::repeatDims_;
      using FFTWHelper< fftwapi >::in_;
      using FFTWHelper< fftwapi >::out_;
      using FFTWHelper< fftwapi >::complexType_;
      using FFTWHelper< fftwapi >::inputStrides_;
      using FFTWHelper< fftwapi >::inputTensorStride_;
      using FFTWHelper< fftwapi >::largestProcessedDim_;
      using FFTWHelper< fftwapi >::dimsProcessed_;
      using FFTWHelper< fftwapi >::transformScale_;
      using FFTWHelper< fftwapi >::GetOutputExternalInterface;
      using FFTWHelper< fftwapi >::ShiftCenterToCorner;
      using FFTWHelper< fftwapi >::ShiftCornerToCenter;
public:
   FFTWHelperR2C( Image const& in, Image& out ) : FFTWHelper< fftwapi >( in, out ) {}

   virtual void ForgeOutput( UnsignedArray const& outSize ) override {
      out_.Strip();
      out_.SetExternalInterface( GetOutputExternalInterface() );
      out_.ReForge( outSize, in_.TensorElements(), complexType_, Option::AcceptDataTypeChange::DO_ALLOW );

      // Set inputStrides_
      // PrepareInput() for R2C in-place copies the real data to the complex output image,
      // interleaving it with the (empty/unused) imaginary data,
      // which results in an input float stride that is twice the complex stride
      // Manual says: "for an in-place transform, each individual dimension should be able to operate in place".
      // Not sure what this implies exactly.
      inputStrides_ = out_.Strides();
      for( int iDim = 0; iDim < inputStrides_.size(); ++iDim ) {
         inputStrides_[iDim] *= 2;
      }
      inputTensorStride_ = 2 * out_.TensorStride();
   }

   virtual void PrepareInput( bool inverse, bool symmetricNormalization, bool shiftOriginToCenter ) override {
      // This must be a forward transform
      DIP_THROW_IF( inverse, "FFTW real-to-complex cannot be an inverse transform" );
      // Copy pixel data to out_ without modifying its properties.
      Image tmp = out_;
      if( symmetricNormalization ) {
         // Use the Multiply operation to scale and copy the input to the output buffer
         typename fftwapi::real normalizationScale = std::sqrt( static_cast<typename fftwapi::real>(1.0) / transformScale_ );
         Multiply( in_, normalizationScale, tmp, tmp.DataType() );
      }
      else {
         // Just copy; no normalization in the forward transform
         tmp.Copy( in_ );
      }
      if( shiftOriginToCenter ) {
         // TODO: only the real values need wrapping -> create Image with same data but different type and strides?
         ShiftCenterToCorner( tmp );
      }
   }

   virtual void FinalizeOutput( bool shiftOriginToCenter ) override {
      // TODO: rewrite; this approach is likely to be very inefficient!
      // The least we can do is iterate over the missing data instead of over existing data

      // Due to Hermitian symmetry, only half the transform is computed and we have to fill the other half, for which holds:
      // Y [k_0, k_1, ..., k_(d-1)] = Y [n_0-k_0, n_1-k_1, ..., n_(d-1)-k_(d-1)]*
      // Note that 0 maps to 0.
      const dip::uint completionDim = largestProcessedDim_;  // This must be the dimension *represented* by the last element of fftwSizeDims
      typedef ComplexType< typename fftwapi::real > dip_complex;
      dip::ImageIterator< dip_complex > itOut( out_ );
      dip::uint completionDimIndex = 0;
      do {
         // Only process the part of the image containing input values
         completionDimIndex = itOut.Coordinates()[completionDim];
         if( completionDimIndex <= out_.Size( completionDim ) / 2 ) {
            dip::UnsignedArray outCoords = itOut.Coordinates();
            // Transform the coords of the processed dimensions from k to N-k
            for( UnsignedArray::const_iterator itDim = dimsProcessed_.begin(); itDim != dimsProcessed_.end(); ++itDim ) {
               dip::uint dimSize = out_.Size( *itDim );
               outCoords[*itDim] = (dimSize - itOut.Coordinates()[*itDim]) % dimSize;
            }
            if( outCoords[completionDim] >= out_.Size( completionDim ) / 2 + 1 ) {
               dip_complex* pIn = static_cast<dip_complex*>(itOut.Pointer());
               dip_complex* pOut = static_cast<dip_complex*>(out_.At( outCoords ).Origin());
               // Write the complex conjugate of input to output for all tensor elements
               for( int iT = 0; iT < out_.TensorElements(); ++iT, pIn += out_.TensorStride(), pOut += out_.TensorStride() ) {
                  pOut->real( pIn->real() );
                  pOut->imag( -pIn->imag() );
               }
            }
         }
         ++itOut;
      } while( itOut );

      if( shiftOriginToCenter ) {
         ShiftCornerToCenter( out_ );
      }
   }

   // Create r2c plan
   virtual typename fftwapi::plan CreatePlan( bool inverse ) override {
      return fftwapi::plan_guru_dft_r2c( static_cast<int>( sizeDims_.size() ), &sizeDims_[0], static_cast<int>( repeatDims_.size() ), &repeatDims_[0],
         (typename fftwapi::real*)out_.Origin(), (typename fftwapi::complex*)out_.Origin(), FFTW_MEASURE );
   }
};

// FFTW helper class for complex to real transforms
template< class fftwapi >
class FFTWHelperC2R : public FFTWHelper< fftwapi >
{
      using FFTWHelper< fftwapi >::sizeDims_;
      using FFTWHelper< fftwapi >::repeatDims_;
      using FFTWHelper< fftwapi >::in_;
      using FFTWHelper< fftwapi >::out_;
      using FFTWHelper< fftwapi >::complexType_;
      using FFTWHelper< fftwapi >::floatType_;
      using FFTWHelper< fftwapi >::inputStrides_;
      using FFTWHelper< fftwapi >::inputTensorStride_;
      using FFTWHelper< fftwapi >::largestProcessedDim_;
      using FFTWHelper< fftwapi >::dimsProcessed_;
      using FFTWHelper< fftwapi >::transformScale_;
      using FFTWHelper< fftwapi >::GetOutputExternalInterface;
      using FFTWHelper< fftwapi >::ShiftCenterToCorner;
      using FFTWHelper< fftwapi >::ShiftCornerToCenter;
public:
   FFTWHelperC2R( Image const& in, Image& out ) : FFTWHelper< fftwapi >( in, out ) {}

   virtual void ForgeOutput( UnsignedArray const& outSize ) override {
      // Backup outSize
      floatOutSize_ = in_.Sizes(); // TODO: support padding via outSize (currently ignored)
      // Create real-typed image around pre-allocated data, which is large enough to contain (a little over half of) the complex input image
      void* origin;
      Tensor outTensor( in_.Tensor() );
      // Prepare outSize with the special treatment of the last processed dimension
      complexOutSize_ = in_.Sizes();
      complexOutSize_[largestProcessedDim_] = outSize[largestProcessedDim_] / 2 + 1;
      // Allocate data (also fills inputStrides_)
      dataLargeEnoughForComplex_ = GetOutputExternalInterface()->AllocateData( origin, complexType_, complexOutSize_, inputStrides_, outTensor, inputTensorStride_ );

      IntegerArray floatOutStrides = inputStrides_;
      // Correct the stride of the N/2+1 padded ("peculiar") dimension in the float output image 
      // Assumption: this stride is stored in strides[ peculiar_dim + 1 ]
      if( largestProcessedDim_ < in_.Dimensionality() - 1 ) {
         floatOutStrides[largestProcessedDim_ + 1] *= 2;
      }
      out_ = Image( dataLargeEnoughForComplex_, origin, floatType_, floatOutSize_, floatOutStrides, outTensor, inputTensorStride_, GetOutputExternalInterface() );
   }

   virtual void PrepareInput( bool inverse, bool symmetricNormalization, bool shiftOriginToCenter ) override {
      // This must be an inverse transform
      DIP_THROW_IF( !inverse, "FFTW complex-to-real must be an inverse transform" );

      // Determine scale
      typename fftwapi::real normalizationScale = static_cast<typename fftwapi::real>(1.0) / transformScale_;
      if( symmetricNormalization ) {
         normalizationScale = std::sqrt( normalizationScale );
      }

      // Define a wrapper around the out_ image with size N/2+1 and complex type
      Image complexOut( NonOwnedRefToDataSegment( out_.Data() ), out_.Origin(), in_.DataType(), complexOutSize_, inputStrides_, in_.Tensor(), inputTensorStride_ );

      // Copy one half of the complex input to the output
      if( shiftOriginToCenter ) {

         // Do the shifting in the largestProcessedDim_ dimension here, because it is more difficult in the N/2+1 sized output image
         bool dimIsOdd = in_.Size( largestProcessedDim_ ) & 1;
         UnsignedArray shiftedInCoords( in_.Dimensionality(), 0 );
         shiftedInCoords[largestProcessedDim_] = in_.Size( largestProcessedDim_ ) / 2;  // During ifft, the shift is negative. But this means the shift in in_ during a copy is positive.
         UnsignedArray inSize = complexOutSize_;
         UnsignedArray outSize = complexOutSize_;
         // If the dimension in question is even sized, copy N/2 sub-images from in_[..., N/2, ...] to out_[..., 0, ...] and copy in_[..., 0, ...] to out_[..., N/2, ...]
         // If the dimension in question is odd sized, copy N/2+1 sub-images from in_[..., N/2, ...] to out_[..., 0, ...]
         if( !dimIsOdd ) {
            inSize[largestProcessedDim_]--;
            outSize[largestProcessedDim_]--;
         }

         // Copy N/2 (or N/2+1) sub-images
         void* shiftedInOrigin = in_.Pointer( shiftedInCoords );
         Image halfIn( NonOwnedRefToDataSegment( in_.Data() ), shiftedInOrigin, in_.DataType(), inSize, in_.Strides(), in_.Tensor(), in_.TensorStride() );
         Image partialComplexOut( NonOwnedRefToDataSegment( out_.Data() ), out_.Origin(), in_.DataType(), outSize, inputStrides_, in_.Tensor(), inputTensorStride_ );
         // Multiply() performs the copy and the scaling in one operation
         Multiply( halfIn, normalizationScale, partialComplexOut, partialComplexOut.DataType() );

         if( !dimIsOdd ) {
            // Copy one last sub-image
            inSize[largestProcessedDim_] = 1;
            outSize[largestProcessedDim_] = 1;
            void* shiftedInOrigin = in_.Origin();
            UnsignedArray shiftedOutCoords( out_.Dimensionality(), 0 );
            shiftedOutCoords[largestProcessedDim_] = in_.Size( largestProcessedDim_ ) / 2;
            void* shiftedOutOrigin = complexOut.Pointer( shiftedOutCoords );
            Image subImgIn( NonOwnedRefToDataSegment( in_.Data() ), shiftedInOrigin, in_.DataType(), inSize, in_.Strides(), in_.Tensor(), in_.TensorStride() );
            Image subImgComplexOut( NonOwnedRefToDataSegment( out_.Data() ), shiftedOutOrigin, in_.DataType(), outSize, inputStrides_, in_.Tensor(), inputTensorStride_ );
            // Multiply() performs the copy and the scaling in one operation
            Multiply( subImgIn, normalizationScale, subImgComplexOut, subImgComplexOut.DataType() );
         }

         // Wrap the remaining dimensions (largestProcessedDim_ is skipped, we just did that one)
         ShiftCenterToCorner( complexOut, largestProcessedDim_ );
      }
      else {
         // Create wrappers around in_ such that a single Copy() call can do the job
         Image halfIn( NonOwnedRefToDataSegment(in_.Data()), in_.Origin(), in_.DataType(), complexOutSize_, in_.Strides(), in_.Tensor(), in_.TensorStride() );
         // Multiply() performs the copy and the scaling in one operation
         Multiply( halfIn, normalizationScale, complexOut, complexOut.DataType() );
         // operation to just copy without scaling: complexOut.Copy( halfIn );
      }
   }

   virtual void FinalizeOutput( bool shiftOriginToCenter ) override {
      if( shiftOriginToCenter ) {
         ShiftCornerToCenter( out_ );
      }
   }

   // Create c2r plan
   virtual typename fftwapi::plan CreatePlan( bool inverse ) override {
      return fftwapi::plan_guru_dft_c2r( static_cast<int>( sizeDims_.size() ), &sizeDims_[0], static_cast<int>( repeatDims_.size() ), &repeatDims_[0],
         (typename fftwapi::complex*)out_.Origin(), (typename fftwapi::real*)out_.Origin(), FFTW_MEASURE );
   }

protected:
   UnsignedArray complexOutSize_;
   UnsignedArray floatOutSize_;  // filled by ForgeOutput()
   DataSegment dataLargeEnoughForComplex_;
};

// FFTW helper class for complex to complex transforms
template< class fftwapi >
class FFTWHelperC2C : public FFTWHelper< fftwapi >
{
      using FFTWHelper< fftwapi >::sizeDims_;
      using FFTWHelper< fftwapi >::repeatDims_;
      using FFTWHelper< fftwapi >::in_;
      using FFTWHelper< fftwapi >::out_;
      using FFTWHelper< fftwapi >::complexType_;
      using FFTWHelper< fftwapi >::floatType_;
      using FFTWHelper< fftwapi >::inputStrides_;
      using FFTWHelper< fftwapi >::inputTensorStride_;
      using FFTWHelper< fftwapi >::largestProcessedDim_;
      using FFTWHelper< fftwapi >::dimsProcessed_;
      using FFTWHelper< fftwapi >::transformScale_;
      using FFTWHelper< fftwapi >::GetOutputExternalInterface;
      using FFTWHelper< fftwapi >::ShiftCenterToCorner;
      using FFTWHelper< fftwapi >::ShiftCornerToCenter;
public:
   FFTWHelperC2C( Image const& in, Image& out ) : FFTWHelper< fftwapi >( in, out ) {}

   virtual void ForgeOutput( UnsignedArray const& outSize ) override {
      out_.Strip();
      out_.SetExternalInterface( GetOutputExternalInterface() );
      out_.ReForge( outSize, in_.TensorElements(), complexType_, Option::AcceptDataTypeChange::DO_ALLOW );

      // Fill inputStrides_: equal to out_'s strides
      // Manual says: "for in-place transforms the input/output strides should be the same."
      inputStrides_ = out_.Strides();
      inputTensorStride_ = out_.TensorStride();
   }

   virtual void PrepareInput( bool inverse, bool symmetricNormalization, bool shiftOriginToCenter ) override {
      if( inverse || symmetricNormalization ) {
         // Use the Multiply operation to scale and copy the input to the output buffer
         typename fftwapi::real normalizationScale = static_cast<typename fftwapi::real>( 1.0 ) / transformScale_;
         if( symmetricNormalization ) {
            normalizationScale = std::sqrt( normalizationScale );
         }
         Multiply( in_, normalizationScale, out_, out_.DataType() );
      }
      else {
         // Just copy without normalization
         out_.Copy( in_ );
      }

      if( shiftOriginToCenter ) {
         ShiftCenterToCorner( out_ );
      }
   }

   virtual void FinalizeOutput( bool shiftOriginToCenter ) override {
      if( shiftOriginToCenter ) {
         ShiftCornerToCenter( out_ );
      }
   }

   // Create regular dft (c2c) plan
   virtual typename fftwapi::plan CreatePlan( bool inverse ) override {
      int sign = inverse ? FFTW_BACKWARD : FFTW_FORWARD;
      return fftwapi::plan_guru_dft( static_cast<int>( sizeDims_.size() ), &sizeDims_[0], static_cast<int>( repeatDims_.size() ), &repeatDims_[0],
         (typename fftwapi::complex*)out_.Origin(), (typename fftwapi::complex*)out_.Origin(), sign, FFTW_MEASURE );
   }
};

// \brief Function that performs the FFTW transform, templated in the floating point type
//
// The actual work is delegated to a helper class, depending on the transform type. See `FFTWHelper`.
// `outSize` can be the result of padding for optimal DFT sizes, but is not yet supported. It must be the same as in's size.
template< typename FloatType >
void PerformFFTW( Image const& in, Image& out, UnsignedArray const& outSize, BooleanArray const& process, bool inverse, bool realOutput, bool shiftOriginToCenter, bool symmetric ) {
   // Use the correct FFTW API depending on the floating point type
   using fftwapi = fftwapidef< FloatType >;

   // Perform FFTW threading initialization. This is done once; subsequent calls will not re-initialize.
   DIP_THROW_IF( FFTWThreading< FloatType >::GetInstance()->GetInitResult() == 0, "Error initializing FFTW with threading" );

   // In-place processing is not supported. We need a separate output image
   // to perform the necessary preparations like shifting, scaling and (possibly) data conversion
   DIP_THROW_IF( &in == &out, "FFTW for in == out not supported" );

   // Determine transform type and reate data helper for it
   std::shared_ptr< FFTWHelper< fftwapi > > helper;
   if (in.DataType().IsReal() && realOutput) // Real-to-real
      helper.reset( new FFTWHelperR2R< fftwapi >( in, out ) );
   else if (in.DataType().IsReal()) // Real-to-complex
      helper.reset( new FFTWHelperR2C< fftwapi >( in, out ) );
   else if (in.DataType().IsComplex() && realOutput ) // Complex-to-real
      helper.reset( new FFTWHelperC2R< fftwapi >( in, out ) );
   else // Complex-to-complex
      helper.reset( new FFTWHelperC2C< fftwapi >( in, out ) );

   // Handle processing dims
   helper->HandleProcessingDims( process );

   // Forge the output image
   helper->ForgeOutput( outSize );

   // Prepare iodim structs
   helper->PrepareIODims();

   // Create FFTW plan
   fftwapi::plan_with_nthreads( FFTWThreading< FloatType >::GetInstance()->GetOptimalNumThreads( outSize ) );
   typename fftwapi::plan plan = helper->CreatePlan( inverse );
   DIP_THROW_IF( plan == NULL, "FFTW planner failed, requested data formats/strides not supported" );

   // Fill output for in-place operation
   // NOTE!! This must be done after creating the plan, because FFTW_MEASURE overwrites the in/out arrays.
   helper->PrepareInput( inverse, symmetric, shiftOriginToCenter );

   // The actual work: execute the plan
   fftwapi::execute( plan );

   // Destroy the plan
   fftwapi::destroy_plan( plan );

   // Finalize the output image
   helper->FinalizeOutput( shiftOriginToCenter );
}

} // end anonymous namespace for FFTW functionality

#endif // DIP__HAS_FFTW

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
      if( option == S::INVERSE ) {
         inverse = true;
      } else if( option == S::REAL ) {
         // TODO: We should probably write code to do real -> 1/2 plane complex, and 1/2 plane complex -> real DFTs.
         // TODO: If so, we'll need to write our own loop code here, we won't be able to depend on Framework::Separable (unless we add some options there...)
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

   Image const in_copy = in; // Make a copy of the header to preserve image in case in == out

#ifdef DIP__HAS_FFTW

   // Determine floating point size and call appropriate work horse
   // NOTE: There is no support for padded output yet, so outSize is not yet passed
   DataType floatOutType = DataType::SuggestFloat( in.DataType() );
   switch( floatOutType ) {
   case DT_SFLOAT:
      PerformFFTW< float >( in, out, in.Sizes(), process, inverse, real, !corner, symmetric );
      break;
   case DT_DFLOAT:
      PerformFFTW< double >( in, out, in.Sizes(), process, inverse, real, !corner, symmetric );
      break;
   default:
      DIP_THROW( "Unknown float type for FFTW" );
      break;
   }

#else // DIP__HAS_FFTW

   // Determine output data type
   DataType dtype = DataType::SuggestComplex( in.DataType() );
   // Allocate output image, so that it has the right (padded) size. If we don't do padding, then we're just doing the framework's work here
   Image tmp;
   if( real ) {
      tmp.ReForge( outSize, in_copy.TensorElements(), dtype );
   } else {
      out.ReForge( outSize, in_copy.TensorElements(), dtype );
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

#endif // DIP__HAS_FFTW

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
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 256 )) == 0 );
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 105 )) == 0 ); // 3*5*7
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 154 )) == 0 ); // 2*7*11
   DOCTEST_CHECK( doctest::Approx( dotest< float >( 97 )) == 0 ); // prime
   DOCTEST_CHECK( doctest::Approx( dotest< double >( 105, true )) == 0 );
}

#endif // DIP__ENABLE_DOCTEST
