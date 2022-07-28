/*
 * (c)2017-2022, Cris Luengo, Erik Schuitema
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

/* TODO
    - Make use of R2C and C2R transforms in PocketFFT and FFTW
    - Implement a DCT function
    - Remove FFTW_UNALIGNED by ensuring that the buffers are 16-byte aligned; for this, we need the Separable
        framework to make 16-byte boundary aligned buffers
*/

#include "diplib.h"
#include "diplib/transform.h"
#include "diplib/dft.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/multithreading.h"
#include "diplib/geometry.h"
#include "diplib/math.h"


namespace dip {

namespace {


constexpr BoundaryCondition DFT_PADDING_MODE = BoundaryCondition::ZERO_ORDER_EXTRAPOLATE; // Is this the least damaging boundary condition?
constexpr BoundaryCondition IDFT_PADDING_MODE = BoundaryCondition::ADD_ZEROS;


// FFTW documentation specifies 16-byte alignment required for SIMD implementations:
// http://www.fftw.org/fftw3_doc/SIMD-alignment-and-fftw_005fmalloc.html#SIMD-alignment-and-fftw_005fmalloc
// constexpr dip::uint FFTW_MAX_ALIGN_REQUIRED = 16;


// This function by Alexei: http://stackoverflow.com/a/19752002/7328782
template< typename TPI >
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

// This function by Alexei: http://stackoverflow.com/a/19752002/7328782
template< typename TPI >
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

template< typename TPI >
static void ShiftCornerToCenterHalfLine( TPI* data, dip::uint length ) { // ifftshift, but for a half-line only
   bool isOdd = length & 1u;
   length /= 2;  // the central pixel, the last value in the line that we'll use
   dip::uint jj = ( length + 1 ) / 2;  // the number of swaps
   for( dip::uint ii = 0; ii < jj; ++ii ) {
      std::swap( data[ ii ], data[ length - ii ] );
   }
   for( dip::uint ii = 1; ii < ( length + isOdd ); ++ii ) {
      data[ ii ] = std::conj( data[ ii ] );
   }
}

// TPI is either scomplex or dcomplex.
template< typename TPI >
class MirrorInPlaceLineFilter : public Framework::SeparableLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength;
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         DIP_ASSERT( params.inBuffer.length == params.outBuffer.length );
         DIP_ASSERT( params.inBuffer.stride == params.outBuffer.stride );
         DIP_ASSERT( params.inBuffer.buffer == params.outBuffer.buffer ); // We're reading and writing directly from the image!
         TPI* data = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint length = static_cast< dip::sint >( params.outBuffer.length );
         dip::sint stride = params.outBuffer.stride;
         for( dip::sint ii = 0; ii < length / 2; ++ii ) {
            std::swap( data[ ii * stride ], data[ ( length - ii - 1 ) * stride ] );
         }
      }
};

void MirrorInPlace( Image& img, BooleanArray const& flip ) {
   // We only call this for complex-valued images.
   DIP_ASSERT( img.IsForged() );
   DataType dtype = img.DataType();
   DIP_ASSERT( dtype.IsComplex() );
   DIP_ASSERT( img.Dimensionality() == flip.size() );
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, MirrorInPlaceLineFilter, (), dtype );
      Framework::Separable( img, img, dtype, dtype, flip, { 0 }, {}, *lineFilter,
                            Framework::SeparableOption::AsScalarImage + Framework::SeparableOption::CanWorkInPlace );
   DIP_END_STACK_TRACE
}

// TPI is either scomplex or dcomplex.
template< typename TPI >
class C2C_DFT_LineFilter : public Framework::SeparableLineFilter {
   public:
      C2C_DFT_LineFilter(
            UnsignedArray const& outSize,
            BooleanArray const& process,
            bool inverse, bool corner, dfloat scale
      ) : scale_( static_cast< FloatType< TPI >>( scale )), shift_( !corner ) {
         dft_.resize( outSize.size() );
         for( dip::uint ii = 0; ii < outSize.size(); ++ii ) {
            if( process[ ii ] ) {
               dft_[ ii ].Initialize( outSize[ ii ], inverse );
            }
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         auto const& dft = dft_[ params.dimension ];
         dip::uint length = dft.TransformSize();
         dip::uint border = params.inBuffer.border;
         DIP_ASSERT( params.inBuffer.length + 2 * border >= length );
         DIP_ASSERT( params.outBuffer.length == length );
         if( !( params.inBuffer.length & 1u ) && ( length & 1u )) {
            // When padding from an even-sized array to an odd-sized array, we pad one fewer element to the left
            // In other cases we pad evenly or we pad one fewer element to the right.
            --border;
         }
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer ) - border;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         FloatType< TPI > scale{ 1.0 };
         if( params.pass == params.nPasses - 1 ) {
            scale = scale_;
         }
         if( shift_ ) {
            ShiftCenterToCorner( in, length );
         } else if( border > 0 ) {
            // we only get here with "corner"+"fast", we cannot combine those with "inverse".
            DIP_ASSERT( !dft.IsInverse() );
            // If not shifting, we need the padding to be on the right, thus we move data over a bit
            std::copy( in + border, in + length, in );
            // We copy the padded border as well, extending it appropriately
         }
         dft.Apply( in, out, scale );
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }

   private:
      std::vector< DFT< FloatType< TPI >>> dft_; // one for each dimension
      FloatType< TPI > scale_;
      bool shift_;
};

// TPI is either scomplex or dcomplex.
// This will always only be called for a single dimension.
// Input buffer is actually complex-valued too. TODO: make it real-valued, and call FFTW's R2C function.
template< typename TPI >
class R2C_DFT_LineFilter : public Framework::SeparableLineFilter {
   public:
      R2C_DFT_LineFilter( dip::uint outSize, bool corner ) : shift_( !corner ) {
         dft_.Initialize( outSize, false );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = dft_.TransformSize();
         dip::uint border = params.inBuffer.border;
         DIP_ASSERT( params.inBuffer.length + 2 * border >= length );
         DIP_ASSERT( params.outBuffer.length == length );
         if( !( params.inBuffer.length & 1u ) && ( length & 1u )) {
            // When padding from an even-sized array to an odd-sized array, we pad one fewer element to the left
            // In other cases we pad evenly or we pad one fewer element to the right.
            --border;
         }
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer ) - border;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         if( shift_ ) {
            ShiftCenterToCorner( in, length );
         } else if( border > 0 ) {
            // If not shifting, we need the padding to be on the right, thus we move data over a bit
            std::copy( in + border, in + length, in );
            // We copy the padded border as well, extending it appropriately
         }
         dft_.Apply( in, out, 1 );
         if( shift_ ) {
            ShiftCornerToCenterHalfLine( out, length );
         }
      }

   private:
      DFT< FloatType< TPI >> dft_;
      bool shift_;
};

// TPI is either scomplex or dcomplex.
// This will always only be called for a single dimension.
template< typename TPI >
class C2R_IDFT_LineFilter : public Framework::SeparableLineFilter {
   public:
      C2R_IDFT_LineFilter( dip::uint outSize, dip::uint inSize, bool corner ) : shift_( !corner ), inSize_( inSize ) {
         dft_.Initialize( outSize, true, true );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = dft_.TransformSize();
         DIP_ASSERT(( inSize_ / 2 + 1 ) == params.inBuffer.length );
         DIP_ASSERT( length >= inSize_ );
         DIP_ASSERT( params.outBuffer.length == length );
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::uint firstSample = 0;
         dip::uint start = params.inBuffer.length;
         dip::uint end = 0;
         if( shift_ ) {
            // Rightmost input sample is central output sample: `params.inBuffer.length-1` ==> `length/2`
            firstSample = length / 2 - params.inBuffer.length + 1;
            start -= 1;                         // the last value is the 0 frequency, don't copy it
            end = ( inSize_ & 1u ) ? 0 : 1;     // for even-sized buffer, the first value is not to be copied.
         } else {
            // Leftmost input sample is leftmost output sample, but we never pad in this case
            DIP_ASSERT( length == inSize_ );
            //firstSample = 0;
            start -= ( inSize_ & 1u ) ? 0 : 1;  // for even-sized buffer, the last value is not to be copied.
            end = 1;                            // the first value is the 0 frequency, don't copy it
         }
         TPI* outPtr = out;
         // Pad the output array by adding zeros (IDFT_PADDING_MODE = BoundaryCondition::ADD_ZEROS)
         for( dip::uint ii = 0; ii < firstSample; ++ii ) {
            *outPtr = 0;
            ++outPtr;
         }
         // Copy the data from in to the left half of out
         for( dip::uint ii = 0; ii < params.inBuffer.length; ++ii ) {
            *outPtr = in[ ii ];
            ++outPtr;
         }
         // Copy the complex conjugated data from in to the right half of out
         for( dip::uint ii = start; ii > end; ) {      // loop excludes start, includes end!
            --ii;
            *outPtr = std::conj( in[ ii ] );
            ++outPtr;
         }
         // Pad the output array by adding zeros (IDFT_PADDING_MODE = BoundaryCondition::ADD_ZEROS)
         while( outPtr < out + length ) {
            *outPtr = 0;
            ++outPtr;
         }
         if( shift_ ) {
            ShiftCenterToCorner( out, length );
         }
         dft_.Apply( out, out, 1 );
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }

   private:
      DFT< FloatType< TPI >> dft_;
      bool shift_;
      dip::uint inSize_;
};

// Computes the complex-to-complex Fourier transform
void DFT_C2C_compute(
      Image const& in,     // real- or complex-valued
      Image& out,          // complex-valued, already forged and of the expected sizes
      BooleanArray const& process,
      bool inverse,
      bool corner,
      dfloat scale

) {
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( out.IsForged() );
   //DIP_ASSERT( in.DataType().IsComplex() ); // NOTE! Input could be real-valued: if 1D we don't do R2C transform.
   DIP_ASSERT( out.DataType().IsComplex() );
   // Find parameters for separable framework
   DataType dtype = out.DataType();
   dip::uint nDims = in.Dimensionality();
   UnsignedArray border( nDims, 0 );
   BoundaryConditionArray bc{ inverse ? IDFT_PADDING_MODE : DFT_PADDING_MODE };
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( out.Size( ii ) > in.Size( ii )) {
         border[ ii ] = div_ceil< dip::uint >( out.Size( ii ) - in.Size( ii ), 2 );
      }
   }
   // Do the processing
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, C2C_DFT_LineFilter, ( out.Sizes(), process, inverse, corner, scale ), dtype );
      Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter,
                            Framework::SeparableOption::UseInputBuffer +   // input stride is always 1
                            Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1
                            Framework::SeparableOption::DontResizeOutput + // output is potentially larger than input, if padding with zeros
                            Framework::SeparableOption::AsScalarImage      // each tensor element processed separately
      );
   DIP_END_STACK_TRACE
}

// Computes a 1D real-to-complex DFT. Potentially computes only the redundant part (if using FFTW).
void DFT_R2C_1D_compute(
      Image const& in,     // real-valued
      Image& out,          // the first half of the image is filled in, pixels 0 through size/2+1
      dip::uint dimension, // dimension along which to compute
      bool corner          // where to put the origin
) {
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( out.IsForged() );
   DIP_ASSERT( !in.DataType().IsComplex() );
   DIP_ASSERT( out.DataType().IsComplex() );
   dip::uint nDims = in.Dimensionality();
   DIP_ASSERT( dimension < nDims );
   // Find parameters for separable framework
   DataType dtype = out.DataType();
   BooleanArray process( nDims, false );
   process[ dimension ] = true;
   UnsignedArray border( nDims, 0 );
   border[ dimension ] = div_ceil< dip::uint >( out.Size( dimension ) - in.Size( dimension ), 2 );
   BoundaryConditionArray bc{ DFT_PADDING_MODE };

   DIP_START_STACK_TRACE
      // Create a window over `out` that has same dimensions as `in` in the non-processing dimensions
      UnsignedArray sizes = in.Sizes();
      sizes[ dimension ] = out.Size( dimension );
      RangeArray window = out.CropWindow( sizes, corner ? Option::CropLocation::TOP_LEFT : Option::CropLocation::CENTER );
      dip::Image tmp = out.At( window );
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, R2C_DFT_LineFilter, ( tmp.Size( dimension ), corner ), dtype );
      Framework::Separable( in, tmp, dtype, dtype, process, border, bc, *lineFilter,
                            Framework::SeparableOption::UseInputBuffer +   // input stride is always 1
                            Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1
                            Framework::SeparableOption::DontResizeOutput + // output is potentially larger than input, if padding with zeros
                            Framework::SeparableOption::AsScalarImage      // each tensor element processed separately
      );
      // Extend computed data into output regions outside the window (boundary extension)
      ExtendRegion( out, window, std::move( bc ));
   DIP_END_STACK_TRACE
}

// Copies data from one half of the DFT to the other half, applying conjugate symmetry.
void DFT_R2C_1D_finalize(
      Image& img,          // complex-valued, result of DFT_R2C_1D_compute() (where possibly other dimensions had full DFTs computed)
      BooleanArray const& process, // the C2C dimensions
      dip::uint dimension, // dimension along which to compute -- same value passed to DFT_R2C_1D_compute()!
      bool corner          // where to put the origin -- same value passed to DFT_R2C_1D_compute()!
) {
   DIP_ASSERT( img.IsForged() );
   DIP_ASSERT( img.DataType().IsComplex() );
   dip::uint nDims = img.Dimensionality();
   DIP_ASSERT( dimension < nDims );
   DIP_ASSERT( !process[ dimension ] ); // the R2C process dimension should not be a C2C process dimension
   // Input: pixels 0 to size/2 are set
   dip::uint size = img.Size( dimension );
   RangeArray leftWindow( nDims );
   if( !( size & 1u )) {
      // even size: pixels 0 and size/2 stay where they are
      leftWindow[ dimension ] = { 1, static_cast< dip::sint >( size / 2 - 1 ) };
   } else {
      if( corner ) {
         // odd size + corner: only pixel 0 stays where it is
         leftWindow[ dimension ] = { 1, static_cast< dip::sint >( size / 2 ) };
      } else {
         // odd size + !corner: only pixel size/2 stays where it is
         leftWindow[ dimension ] = { 0, static_cast< dip::sint >( size / 2 - 1 ) };
      }
   }
   RangeArray rightWindow( nDims );
   rightWindow[ dimension ] = { static_cast< dip::sint >( size / 2 + 1 ), -1 };
   dip::Image left = img.At( leftWindow );
   dip::Image right = img.At( rightWindow );
   // We need to mirror the copy along all transform dimensions
   // In the R2C dimension, just mirror
   left.Mirror( dimension );
   // In the other processing dimensions, mirror in a way that preserves the origin.
   BooleanArray flip( nDims, false );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         if(( img.Size( ii ) & 1u ) && !corner ) { // odd size, origin in middle
            // Plain old mirroring
            left.Mirror( ii );
         } else {
            // Mirror all but index 0, which is copied in place.
            flip[ ii ] = true;
            rightWindow[ ii ].start = 1;
         }
      }
   }
   DIP_STACK_TRACE_THIS( right.Copy( left ));
   Conjugate( right, right );
   right = img.At( rightWindow );
   DIP_STACK_TRACE_THIS( MirrorInPlace( right, flip ));
}

// Copies the left half of the input image, and applies boundary extension for C2C dimensions
void IDFT_C2R_1D_prepare(
      Image const& in,     // real- or complex-valued, only half of it is used
      Image& out,          // half-sized copy of data, can be modified -- Note that the Sizes array of this image are expected to be set to the expected output size of the FT
      dip::uint dimension, // dimension along which to compute
      bool corner
) {
   DIP_ASSERT( in.IsForged() );
   dip::uint nDims = in.Dimensionality();
   DIP_ASSERT( dimension < nDims );
   UnsignedArray outSize = out.Sizes(); // NOTE!!! we're reading  the sizes from `out`, but then modifying them
   outSize[ dimension ] = in.Size( dimension ) / 2 + 1;
   DIP_STACK_TRACE_THIS( out.ReForge( outSize, in.TensorElements(), DataType::SuggestComplex( in.DataType() )));
   RangeArray inWindow( nDims );
   inWindow[ dimension ] = { 0, static_cast< dip::sint >( outSize[ dimension ] - 1 ) };
   outSize = in.Sizes();
   outSize[ dimension ] = out.Size( dimension );
   RangeArray outWindow = out.CropWindow( outSize, corner ? Option::CropLocation::TOP_LEFT : Option::CropLocation::CENTER );
   DIP_START_STACK_TRACE
      out.At( outWindow ).Copy( in.At( inWindow ) );
      ExtendRegion( out, outWindow, { IDFT_PADDING_MODE } );
   DIP_END_STACK_TRACE
}

// Computes a 1D complex-to-real IDFT. Uses only the left half of the input.
void IDFT_C2R_1D_compute(
      Image const& in,     // complex-valued, result of IDFT_C2R_1D_prepare() (where possibly other dimensions had full DFTs computed)
      Image& out,          // real-valued, already forged and with the right sizes
      dip::uint dimension, // dimension along which to compute -- same value passed to IDFT_C2R_1D_prepare()!
      dip::uint length,    // number of samples of the original input image along `dimension`. `in.Size(dimension)==length/2+1`
      bool corner          // where to put the origin -- same value passed to IDFT_C2R_1D_prepare()!
) {
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( out.IsForged() );
   DIP_ASSERT( in.DataType().IsComplex() );
   DIP_ASSERT( !out.DataType().IsComplex() );
   dip::uint nDims = in.Dimensionality();
   DIP_ASSERT( dimension < nDims );
#ifdef DIP_CONFIG_ENABLE_ASSERT
   UnsignedArray outSizes = out.Sizes();
   DIP_ASSERT( length <= outSizes[ dimension ] );
   outSizes[ dimension ] = length / 2 + 1;
   DIP_ASSERT( in.Sizes() == outSizes );
#endif
   // Find parameters for separable framework
   DataType outType = out.DataType();
   DataType dtype = DataType::SuggestComplex( outType );
   BooleanArray process( nDims, false );
   process[ dimension ] = true;
   UnsignedArray border( nDims, 0 );
   BoundaryConditionArray bc{ IDFT_PADDING_MODE };
   // Do the processing
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, C2R_IDFT_LineFilter, ( out.Size( dimension ), length, corner ), dtype );
      Framework::Separable( in, out, dtype, outType, process, border, bc, *lineFilter,
                            Framework::SeparableOption::UseInputBuffer +   // input stride is always 1
                            Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1
                            Framework::SeparableOption::DontResizeOutput + // output is larger than input
                            Framework::SeparableOption::AsScalarImage +    // each tensor element processed separately
                            Framework::SeparableOption::UseRealComponentOfOutput // the buffers are complex-valued, don't cast to real using abs(), but using real().
      );
   DIP_END_STACK_TRACE
}

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
   bool realOutput = false; // real-valued output?
   bool fast = false; // pad the image to a "nice" size?
   bool corner = false;
   bool symmetric = false;
   for( auto const& option : options ) {
      if( option == S::INVERSE ) {
         inverse = true;
      } else if( option == S::REAL ) {
         realOutput = true;
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
   if( inverse ) {
      // If the output is protected and real-valued, compute a real-valued inverse transform
      realOutput |= out.IsProtected() && !out.DataType().IsComplex();
      DIP_THROW_IF( fast && corner, "Cannot use 'corner', 'fast' and 'inverse' together" ); // TODO: figure out how to properly pad "fast"+"inverse"+"corner"...
   } else {
      DIP_THROW_IF( realOutput, "Cannot use 'real' without 'inverse' option" );
   }
   bool realInput = !inverse && !in.DataType().IsComplex(); // forward transform starting with real-valued data?
   DIP_ASSERT( !( realOutput && realInput )); // can't do real-to-real DFT.
   // Handle `process` array
   if( process.empty() ) {
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }
   dip::uint nProcDims = process.count();
   DIP_THROW_IF( nProcDims == 0, "Zero dimensions selected for processing" );

   // Determine output size and scaling
   dip::uint longestDimension = 0;
   UnsignedArray outSize = in.Sizes();
   dfloat scale = 1.0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         if( fast ) {
            dip::uint sz = GetOptimalDFTSize( outSize[ ii ] );
            DIP_THROW_IF( sz < 1u, "Cannot pad image dimension to a larger \"fast\" size." );
            outSize[ ii ] = sz;
         } else {
            DIP_THROW_IF( outSize[ ii ] > maximumDFTSize, "Image size too large for DFT algorithm." );
         }
         if( !process[ longestDimension ] || ( outSize[ ii ] > outSize[ longestDimension ] )) {
            longestDimension = ii;
         }
         scale /= static_cast< dfloat >( outSize[ ii ] );
      }
   }
   if( symmetric ) {
      scale = std::sqrt( scale );
   } else if( !inverse ) {
      scale = 1.0; // forward transform doesn't have a scaling when "symmetric" is not given
   }

   // Do the processing
   Image const in_copy = in; // Preserve input in case *in == *out
   if( realInput && nProcDims > 1 ) {
      // Real-to-complex transform, but only if more than one dimension is processed

      // Create complex-valued output, all processing happens in here
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW ));
      DIP_THROW_IF( !out.DataType().IsComplex(), "Cannot compute Fourier Transform in real-valued output" );
      Image tmp = out.QuickCopy();
      tmp.Protect(); // make sure it won't be reforged by the framework function.
      // One dimension we process with the R2C function
      DIP_STACK_TRACE_THIS( DFT_R2C_1D_compute( in_copy, tmp, longestDimension, corner ));
      // Make window over half the image
      RangeArray window( nDims );
      window[ longestDimension ].stop = static_cast< dip::sint >( tmp.Size( longestDimension ) / 2 );
      dip::Image tmp2;
      DIP_STACK_TRACE_THIS( tmp2 = tmp.At( window ));
      tmp2.Protect();
      // Compute other dimensions in place (it is this step where we do the normalization)
      process[ longestDimension ] = false;
      DIP_STACK_TRACE_THIS( DFT_C2C_compute( tmp2, tmp2, process, inverse, corner, scale ));
      // Copy data to other half of image
      DIP_STACK_TRACE_THIS( DFT_R2C_1D_finalize( tmp, process, longestDimension, corner ));

   } else if( realOutput && nProcDims > 1 ) {
      // Complex-to-real transform, but only if more than one dimension is processed

      // Make a complex-valued copy of about half of the input
      Image tmp;
      tmp.SetSizes( outSize );
      dip::uint longestDimSize = in_copy.Size( longestDimension );
      DIP_STACK_TRACE_THIS( IDFT_C2R_1D_prepare( in_copy, tmp, longestDimension, corner )); // Note that tmp now has `longestDimSize/2+1` pixels along `longestDimension`.
      // Do the complex-to-complex transform in all but one dimension, in-place (it is this step where we do the normalization)
      process[ longestDimension ] = false;
      DIP_STACK_TRACE_THIS( DFT_C2C_compute( tmp, tmp, process, inverse, corner, scale ));
      // Create real-valued output image
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, tmp.TensorElements(), tmp.DataType().Real(), Option::AcceptDataTypeChange::DO_ALLOW ));
      // Do the complex-to-real transform in the remaining dimension
      DIP_STACK_TRACE_THIS( IDFT_C2R_1D_compute( tmp, out, longestDimension, longestDimSize, corner ));

   } else {
      // Plain old complex-to-complex transform, or 1D transform
      // In the 1D case, using the C2R or R2C machinery above yields little or no benefit, so we don't use it.

      // Create complex-valued output, all processing happens in there
      Image tmp;
      if( realOutput ) {
         DIP_STACK_TRACE_THIS( tmp.ReForge( outSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() )));
      } else {
         DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW ));
         tmp = out.QuickCopy();
      }
      // Compute transform
      tmp.Protect(); // make sure it won't be reforged by the framework function.
      DIP_STACK_TRACE_THIS( DFT_C2C_compute( in_copy, tmp, process, inverse, corner, scale ));
      tmp.Protect( false );
      // Create real-valued output if necessary
      if( realOutput ) { // Could happen if nProcDims==1
         tmp = tmp.Real();
         if(( out.DataType() != tmp.DataType() ) && ( !out.IsProtected() )) {
            out.Strip(); // Avoid accidental data conversion.
         }
         DIP_STACK_TRACE_THIS( out.Copy( tmp ));
      }

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
   out.SetPixelSize( std::move( pixelSize ));

   // Set output color space
   if( in_copy.IsColor() ) {
      out.SetColorSpace( in_copy.ColorSpace() );
   }
}


dip::uint OptimalFourierTransformSize( dip::uint size, dip::String const& which ) {
   DIP_STACK_TRACE_THIS( size = GetOptimalDFTSize( size, BooleanFromString( which, "larger", "smaller" )));
   DIP_THROW_IF( size == 0, E::SIZE_EXCEEDS_LIMIT );
   return size;
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the OptimalFourierTransformSize function") {
   DOCTEST_CHECK_THROWS( dip::OptimalFourierTransformSize( 0 ));
   DOCTEST_CHECK_THROWS( dip::OptimalFourierTransformSize( 2125764001 ));
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 1 ) == 1 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 2 ) == 2 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 2125764000 ) == 2125764000 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 1, "smaller" ) == 1 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 2, "smaller" ) == 2 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 2125764000, "smaller" ) == 2125764000 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 500 ) == 500 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 490 ) == 500 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 500, "smaller" ) == 500 );
   DOCTEST_CHECK( dip::OptimalFourierTransformSize( 510, "smaller" ) == 500 );
}

#include "diplib/generation.h"
#include "diplib/statistics.h"

DOCTEST_TEST_CASE("[DIPlib] testing the FourierTransform function") {
   // === 2D image, 2D transform ===
   dip::UnsignedArray sz{ 128, 105 }; // 105 = 3*5*7
   dip::dfloat sigma = 7.0;
   dip::FloatArray shift = { -5.432, -2.345 };
   dip::Image input{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   dip::Image expectedOutput{ sz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   dip::FloatArray outSigma = { static_cast< dip::dfloat >( sz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                                static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );

   // Complex-to-complex transform
   // Note: we test everything in place, if it works in place, it certainly will work with separate input and output images.
   dip::Image output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   auto maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   dip::FourierTransform( output, output, { "inverse" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // Real-to-complex transform (even-sized axis)
   output = input.Copy();
   dip::FourierTransform( output, output );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   // Complex-to-real inverse transform  (even-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // === 2D image, 2D transform -- repeat with different R2C and C2R dimension ===
   sz = { 64, 105 };
   input = dip::Image{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   expectedOutput = dip::Image{ sz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { static_cast< dip::dfloat >( sz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );

   // Real-to-complex transform (odd-sized axis)
   output = input.Copy();
   dip::FourierTransform( output, output );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-4 ); // Much larger error because of smaller image

   // Complex-to-real inverse transform (odd-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // === Test "fast" option
   sz = { 97, 107 }; // prime sizes
   dip::UnsignedArray expectedOutSz{ 100, 108 };
   input = dip::Image{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   expectedOutput = dip::Image{ expectedOutSz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );
   expectedOutput.Crop( sz );

   // Complex-to-complex transform (fast)
   output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   dip::FourierTransform( output, output, { "inverse", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // Real-to-complex transform (fast)
   output = input.Copy();
   dip::FourierTransform( output, output, { "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   // Complex-to-real inverse transform (fast)
   dip::FourierTransform( output, output, { "inverse", "fast", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // === Test "corner" and "symmetric" option (we test these at the same time because they're orthogonal features)
   sz = { 64, 105 };
   input = dip::Image{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   expectedOutput = dip::Image{ sz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { static_cast< dip::dfloat >( sz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() / std::sqrt( sz.product()) }, outSigma, 7.0 );
   dip::FloatArray newShift = shift;
   newShift += expectedOutput.GetCenter();
   dip::ShiftFT( expectedOutput, expectedOutput, newShift );
   dip::Wrap( expectedOutput, expectedOutput, { -static_cast< dip::sint >( sz[ 0 ] ) / 2,
                                                -static_cast< dip::sint >( sz[ 1 ] ) / 2 } );

   // Complex-to-complex transform (corner)
   output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-6 ); // Much larger error because of smaller image, but smaller error also because of normalization

   dip::FourierTransform( output, output, { "inverse", "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // Real-to-complex transform (corner)
   output = input.Copy();
   dip::FourierTransform( output, output, { "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-6 ); // Much larger error because of smaller image, but smaller error also because of normalization

   // Complex-to-real inverse transform (corner)
   dip::FourierTransform( output, output, { "inverse", "real", "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-9 );

   // === Test "corner" + "fast" option (only forward, the combination is not allowed with inverse transform)
   sz = { 97, 107 }; // prime sizes
   expectedOutSz = { 100, 108 };
   input = dip::Image{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   expectedOutput = dip::Image{ expectedOutSz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   newShift = shift;
   newShift += input.GetCenter();
   dip::ShiftFT( expectedOutput, expectedOutput, newShift );
   dip::Wrap( expectedOutput, expectedOutput, { -static_cast< dip::sint >( expectedOutSz[ 0 ] ) / 2,
                                                -static_cast< dip::sint >( expectedOutSz[ 1 ] ) / 2 } );

   // Complex-to-complex transform (fast + corner)
   output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   // Real-to-complex transform (fast + corner)
   output = input.Copy();
   dip::FourierTransform( output, output, { "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-7 );

   // === 3D image, 1D transform ===
   sz = { 3, 32, 2 };
   dip::BooleanArray process{ false, true, false };
   shift = { 2.0, -1.765, 1.0 };
   sigma = 3.0;
   input = dip::Image{ sz, 1, dip::DT_DFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ],
                                       static_cast< dip::dfloat >( sz[ 2 ] / 2 ) + shift[ 2 ] }, { 1 }, { sigma }, 7.0 );
   expectedOutput = dip::Image{ sz, 1, dip::DT_DFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { sigma, static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ), sigma };
   dip::DrawBandlimitedPoint( expectedOutput, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                                static_cast< dip::dfloat >( sz[ 1 ] / 2 ),
                                                static_cast< dip::dfloat >( sz[ 2 ] / 2 ) + shift[ 2 ] },
                              { std::sqrt( 2.0 * dip::pi ) * outSigma[ 1 ] }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, { 0.0, shift[ 1 ], 0.0 } );

    // Complex-to-complex transform (corner)
   output = dip::Convert( input, dip::DT_DCOMPLEX );
   dip::FourierTransform( output, output, {}, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-8 ); // Much larger error because of smaller image

   dip::FourierTransform( output, output, { "inverse" }, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-18 );

   // Real-to-complex transform
   output = input.Copy();
   dip::FourierTransform( output, output, {}, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 1e-8 ); // Much larger error because of smaller image

   // Complex-to-real inverse transform
   dip::FourierTransform( output, output, { "inverse", "real" }, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxmin = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxmin << '\n';
   DOCTEST_CHECK( maxmin < 2e-18 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
