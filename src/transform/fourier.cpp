/*
 * (c)2017, Erik Schuitema.
 * (c)2017-2024, Cris Luengo.
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
    - Implement a DCT function
    - dip::maximumDFTSize can be 2^63-1 for FFTW if we use the guru interface.
*/

#include "diplib/transform.h"

#include <cmath>
#include <memory>
#include <utility>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/dft.h"
#include "diplib/framework.h"
#include "diplib/geometry.h"
#include "diplib/math.h"
#include "diplib/overload.h"


namespace dip {

namespace {


constexpr BoundaryCondition DFT_PADDING_MODE = BoundaryCondition::ZERO_ORDER_EXTRAPOLATE; // Is this the least damaging boundary condition?
constexpr BoundaryCondition IDFT_PADDING_MODE = BoundaryCondition::ADD_ZEROS;


// This function by Alexei: http://stackoverflow.com/a/19752002/7328782
template< typename TPI >
void ShiftCornerToCenter( TPI* data, dip::uint length ) { // fftshift
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
void ShiftCenterToCorner( TPI* data, dip::uint length ) { // ifftshift
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
void ShiftCornerToCenterHalfLine( TPI* data, dip::uint length ) { // fftshift & ifftshift, but for a half-line only
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

template< typename T >
inline void CopyDataToBuffer(
      T const* inBuffer,
      dip::sint inStride,
      T* outBuffer,
      dip::uint pixels
) {
   if( inStride == 0 ) {
      std::fill_n( outBuffer, pixels, *inBuffer );
   } else if( inStride == 1 ) {
      std::copy( inBuffer, inBuffer + pixels, outBuffer );
   } else {
      auto inIt = ConstSampleIterator< T >( inBuffer, inStride );
      std::copy( inIt, inIt + pixels, outBuffer );
   }
}

template< typename TPI >
void CopyForDFT( TPI const* in, dip::uint inLength, dip::sint inStride, TPI* out, dip::uint outLength, bool shift, bool inverse ) {
   dip::uint k = outLength - inLength; // total amount of padding
   if( shift ) {
      // This is the same for both forward and inverse transform
      // Copy right side to left, and left side to right
      dip::uint n = inLength / 2; // position of origin in shifted input array
      CopyDataToBuffer( in, inStride, out + outLength - n, n );
      CopyDataToBuffer( in + static_cast< dip::sint >( n ) * inStride, inStride, out, inLength - n );
      if( k > 0 ) {
         if( inverse ) {
            // Pad the middle part with zeros
            std::fill_n( out + inLength - n, k, TPI( 0 ));
            if(( k & 1u ) == 0 ) {
               // For an even input buffer, we need to split the highest frequency element to maintain symmetry
               out[ outLength - n ] /= 2;
               out[ inLength - n ] = out[ outLength - n ];
            }
         } else {
            // Pad the middle part by repeating last value
            std::fill_n( out + inLength - n, k / 2, out[ inLength - n - 1 ] );
            std::fill_n( out + inLength - n + k / 2, k - k / 2, out[ outLength - n ] );
         }
      }
   } else {
      if( inverse && ( k > 0 )) {
         // Copy left half to left end, and right half to right end, and pad the middle part with zeros
         dip::uint n = ( inLength + 1 ) / 2; // size of the left half
         CopyDataToBuffer( in, inStride, out, n );
         std::fill_n( out + n, k, TPI( 0 ));
         CopyDataToBuffer( in + static_cast< dip::sint >( n ) * inStride, inStride, out + n + k, inLength - n );
         if(( k & 1u ) == 0 ) {
            // For an even input buffer, we need to duplicate the highest frequency element to maintain symmetry
            out[ n + k ] /= 2;
            out[ n ] = out[ n + k ];
         }
      } else {
         // Copy identically
         CopyDataToBuffer( in, inStride, out, inLength );
         if( k > 0 ) {
            // This only happens if !inverse: pad on the right only, keeping the origin on the left pixel
            std::fill_n( out + inLength, k, out[ inLength - 1 ] );
         }
      }
   }
}

// TPI is either scomplex or dcomplex.
template< typename TPI >
class MirrorInPlaceLineFilter : public Framework::SeparableLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /*nTensorElements*/, dip::uint /*border*/, dip::uint /*procDim*/ ) override {
         return lineLength;
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
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
      using TPF = FloatType< TPI >;
   public:
      C2C_DFT_LineFilter(
            UnsignedArray const& outSize,
            BooleanArray const& process,
            bool inverse, bool corner, dfloat scale
      ) : scale_( static_cast< TPF >( scale )), shift_( !corner ) {
         dft_.resize( outSize.size() );
         for( dip::uint ii = 0; ii < outSize.size(); ++ii ) {
            if( process[ ii ] ) {
               dft_[ ii ].Initialize( outSize[ ii ], inverse, Option::DFTOption::InPlace + Option::DFTOption::Aligned );
            }
         }
      }
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /*nTensorElements*/, dip::uint /*border*/, dip::uint /*procDim*/ ) override {
         return 10 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         auto const& dft = dft_[ params.dimension ];
         dip::uint length = dft.TransformSize();
         DIP_ASSERT( params.inBuffer.length <= length );
         DIP_ASSERT( params.outBuffer.length == length );
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint stride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         DIP_ASSERT(  params.outBuffer.stride == 1 );
         TPF scale{ 1.0 };
         if( params.pass == 0 ) {
            scale = scale_;
         }
         CopyForDFT( in, params.inBuffer.length, stride, out, length, shift_, dft.IsInverse() );
         DIP_ASSERT( reinterpret_cast< dip::uint >( out ) % 32 == 0 );
         dft.Apply( out, out, scale );
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }

   private:
      std::vector< DFT< TPF >> dft_; // one for each dimension
      TPF scale_;
      bool shift_;
};

// TPI is either sfloat or dfloat.
// This will always only be called for a single dimension.
template< typename TPI >
class R2C_DFT_LineFilter : public Framework::SeparableLineFilter {
      using TPC = ComplexType< TPI >;
   public:
      R2C_DFT_LineFilter(
            dip::uint outSize, bool corner, dfloat scale
      ) : scale_( static_cast< TPI >( scale )), shift_( !corner ) {
         dft_.Initialize( outSize, false, Option::DFTOption::InPlace + Option::DFTOption::Aligned );
      }
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /*nTensorElements*/, dip::uint /*border*/, dip::uint /*procDim*/ ) override {
         return 5 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = dft_.TransformSize();
         DIP_ASSERT( params.inBuffer.length <= length );
         DIP_ASSERT( params.outBuffer.length == length );
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint stride = params.inBuffer.stride;
         TPI* outR = static_cast< TPI* >( params.outBuffer.buffer ); // view of complex output data as a real array with double the elements
         TPC* out = static_cast< TPC* >( params.outBuffer.buffer );
         DIP_ASSERT( params.outBuffer.stride == 1 );
         CopyForDFT( in, params.inBuffer.length, stride, outR, length, shift_, false );
         DIP_ASSERT( reinterpret_cast< dip::uint >( outR ) % 32 == 0 );
         dft_.Apply( outR, outR, scale_ );
         if( shift_ ) {
            ShiftCornerToCenterHalfLine( out, length );
         }
      }

   private:
      RDFT< TPI > dft_;
      TPI scale_;
      bool shift_;
};

// TPI is either sfloat or dfloat.
// This will always only be called for a single dimension.
template< typename TPI >
class C2R_IDFT_LineFilter : public Framework::SeparableLineFilter {
      using TPC = ComplexType< TPI >;
   public:
      C2R_IDFT_LineFilter(
            dip::uint outSize, dip::uint inSize, bool corner, dfloat scale
      ) : scale_( static_cast< TPI >( scale )), shift_( !corner ), inSize_( inSize ) {
         dft_.Initialize( outSize, true, Option::DFTOption::Aligned + Option::DFTOption::TrashInput );
      }
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /*nTensorElements*/, dip::uint /*border*/, dip::uint /*procDim*/ ) override {
         return 5 * lineLength * static_cast< dip::uint >( std::round( std::log2( lineLength )));
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         dip::uint length = dft_.TransformSize();
         DIP_ASSERT(( inSize_ / 2 + 1 ) == params.inBuffer.length );
         DIP_ASSERT( length >= inSize_ );
         DIP_ASSERT( params.outBuffer.length == length );
         DIP_ASSERT( params.inBuffer.length + 2 * params.inBuffer.border >= length / 2 + 1 );
         TPC* in = static_cast< TPC* >( params.inBuffer.buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         if( params.inBuffer.border > 0 ) {
            std::copy( in, in + params.inBuffer.length, in - params.inBuffer.border );
            in -= params.inBuffer.border;
            std::fill_n( in + params.inBuffer.length, 2 * params.inBuffer.border, TPC( 0 ));
         }
         if( shift_ ) {
            ShiftCornerToCenterHalfLine( in, inSize_ );
         }
         DIP_ASSERT( reinterpret_cast< dip::uint >( in ) % 32 == 0 );
         DIP_ASSERT( reinterpret_cast< dip::uint >( out ) % 32 == 0 );
         dft_.Apply( reinterpret_cast< TPI* >( in ), out, scale_ );
         if( shift_ ) {
            ShiftCornerToCenter( out, length );
         }
      }

   private:
      RDFT< TPI > dft_;
      TPI scale_;
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
   DIP_ASSERT( out.DataType().IsComplex() );
   DataType dtype = out.DataType();
   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_COMPLEX( lineFilter, C2C_DFT_LineFilter, ( out.Sizes(), process, inverse, corner, scale ), dtype );
      Framework::Separable( in, out, dtype, dtype, process, {}, {}, *lineFilter,
                            Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1, buffer is aligned
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
      bool corner,         // where to put the origin
      dfloat scale
) {
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( out.IsForged() );
   DIP_ASSERT( !in.DataType().IsComplex() );
   DIP_ASSERT( out.DataType().IsComplex() );
   DIP_ASSERT( dimension < in.Dimensionality() );
   // Find parameters for separable framework
   DataType outType = out.DataType();
   DataType dtype = outType.Real();

   DIP_START_STACK_TRACE
      // Create a window over `out` that has same dimensions as `in` in the non-processing dimensions
      UnsignedArray sizes = in.Sizes();
      sizes[ dimension ] = out.Size( dimension );
      RangeArray window = out.CropWindow( sizes, corner ? Option::CropLocation::TOP_LEFT : Option::CropLocation::CENTER );
      dip::Image tmp = out.At( window );
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT( lineFilter, R2C_DFT_LineFilter, ( tmp.Size( dimension ), corner, scale ), dtype );
      Framework::OneDimensionalLineFilter( in, tmp, dtype, outType, outType, dimension, 0, DFT_PADDING_MODE, *lineFilter,
                                           Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1, buffer is aligned
                                           Framework::SeparableOption::DontResizeOutput + // output is potentially larger than input, if padding with zeros
                                           Framework::SeparableOption::AsScalarImage      // each tensor element processed separately
      );
      // Extend computed data into output regions outside the window (boundary extension)
      ExtendRegion( out, window, { DFT_PADDING_MODE } );
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
   if( size <= 2 ) {
      return;  // We've got nothing to do here
   }
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

// Computes a 1D complex-to-real IDFT. Uses only the left half of the input.
void IDFT_C2R_1D_compute(
      Image const& in,     // complex-valued (where possibly other dimensions had full DFTs computed)
      Image& out,          // real-valued, already forged and with the right sizes -- all sizes equal to `in` except `dimension`
      dip::uint dimension, // dimension along which to compute
      dip::uint length,    // number of samples of the original input image along `dimension` -- `in.Size(dimension)==length/2+1`
      bool corner,         // where to put the origin
      dfloat scale
) {
   DIP_ASSERT( in.IsForged() );
   DIP_ASSERT( out.IsForged() );
   //DIP_ASSERT( in.DataType().IsComplex() ); // it doesn't need to be, we can compute the inverse transform of the magnitude, for example.
   DIP_ASSERT( !out.DataType().IsComplex() );
   DIP_ASSERT( dimension < in.Dimensionality() );
   DIP_ASSERT( length <= out.Size( dimension ));
#ifdef DIP_CONFIG_ENABLE_ASSERT
   UnsignedArray sz = out.Sizes();
   sz[ dimension ] = length / 2 + 1;
   DIP_ASSERT( in.Sizes() == sz );
#endif
   // Find parameters for separable framework
   DataType inType = DataType::SuggestComplex( in.DataType() );
   DataType dtype = inType.Real();
   dip::uint border = div_ceil( out.Size( dimension ) - length, dip::uint( 2 ));

   // Do the processing
   DIP_START_STACK_TRACE
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT( lineFilter, C2R_IDFT_LineFilter, ( out.Size( dimension ), length, corner, scale ), dtype );
      Framework::OneDimensionalLineFilter( in, out, inType, dtype, dtype, dimension, border, IDFT_PADDING_MODE, *lineFilter,
                                           Framework::SeparableOption::UseInputBuffer +   // input stride is always 1, buffer is aligned
                                           Framework::SeparableOption::UseOutputBuffer +  // output stride is always 1, buffer is aligned
                                           Framework::SeparableOption::DontResizeOutput + // output is larger than input
                                           Framework::SeparableOption::AsScalarImage      // each tensor element processed separately
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
   dip::uint optimalDimension = 0;  // The dimension with the smallest stride is the best to do the R2C or C2R transform on.
                                    // Of course this should probably be the stride of the intermediate (C2R) or output (R2C)
                                    // image, but we haven't allocated those yet... So we look at the input strides?
   UnsignedArray outSize = in.Sizes();
   dfloat scale = 1.0;
   dip::uint maxFactor = fast ? MaxFactor( !realInput && !realOutput ) : 2; // Unused if !fast.
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] ) {
         if( fast ) {
            dip::uint sz = GetOptimalDFTSize( outSize[ ii ], true, maxFactor );
            DIP_THROW_IF( sz < 1u, "Cannot pad image dimension to a larger \"fast\" size" );
            outSize[ ii ] = sz;
         } else {
            DIP_THROW_IF( outSize[ ii ] > maximumDFTSize, "Image size too large for DFT algorithm" );
         }
         if( ii != optimalDimension ) {
            if( !process[ optimalDimension ] || (
                ( outSize[ ii ] > 2 ) && (
                   ( outSize[ optimalDimension ] <= 2 ) ||
                   ( std::abs( in.Stride( ii )) < std::abs( in.Stride( optimalDimension )))
                ))) {
               // Note that we don't want a dimension with size == 2 to be the R2C dimension, as that wouldn't save
               // any work compared to the normal C2C transform. We allow this only if there is no other candidate.
               optimalDimension = ii;
            }
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
   Image const in_copy = in; // Preserve input in case *in == *out. NOLINT(*-unnecessary-copy-initialization)
   if( realInput ) {
      // Real-to-complex transform

      // Create complex-valued output, all processing happens in here
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW ));
      DIP_THROW_IF( !out.DataType().IsComplex(), "Cannot compute Fourier Transform in real-valued output" );
      Image tmp = out.QuickCopy();
      tmp.Protect(); // make sure it won't be reforged by the framework function.
      // One dimension we process with the R2C function
      DIP_STACK_TRACE_THIS( DFT_R2C_1D_compute( in_copy, tmp, optimalDimension, corner, scale ));
      // Make window over half the image
      RangeArray window( nDims );
      window[ optimalDimension ].stop = static_cast< dip::sint >( tmp.Size( optimalDimension ) / 2 );
      Image tmp2;
      DIP_STACK_TRACE_THIS( tmp2 = tmp.At( window ));
      tmp2.Protect();
      // Compute other dimensions in place
      process[ optimalDimension ] = false;
      if( nProcDims > 1 ) {
         DIP_STACK_TRACE_THIS( DFT_C2C_compute( tmp2, tmp2, process, inverse, corner, 1.0 ));
      }
      // Copy data to other half of image
      DIP_STACK_TRACE_THIS( DFT_R2C_1D_finalize( tmp, process, optimalDimension, corner ));
      process[ optimalDimension ] = true; // reset to ensure pixel size is updated along this dimension

   } else if( realOutput ) {
      // Complex-to-real transform

      // Make a window of about half of the input
      dip::uint optimalDimSize = in.Size( optimalDimension );
      RangeArray window( nDims );
      window[ optimalDimension ].stop = static_cast< dip::sint >( optimalDimSize / 2 );
      Image tmpIn;
      DIP_STACK_TRACE_THIS( tmpIn = in.At( window ));
      // Do the complex-to-complex transform in all but one dimension (it is this step where we do the normalization)
      if( nProcDims > 1 ) {
         process[ optimalDimension ] = false;
         UnsignedArray tmpSize = outSize;
         tmpSize[ optimalDimension ] = tmpIn.Size( optimalDimension );
         Image tmpOut( tmpSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() ));
         DIP_STACK_TRACE_THIS( DFT_C2C_compute( tmpIn, tmpOut, process, inverse, corner, 1.0 ));
         tmpIn.swap( tmpOut );
         process[ optimalDimension ] = true; // reset to ensure pixel size is updated along this dimension
      }
      // Create real-valued output image
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), tmpIn.DataType().Real(), Option::AcceptDataTypeChange::DO_ALLOW ));
      // Do the complex-to-real transform in the remaining dimension
      DIP_STACK_TRACE_THIS( IDFT_C2R_1D_compute( tmpIn, out, optimalDimension, optimalDimSize, corner, scale ));

   } else {
      // Plain old complex-to-complex transform

      // Create complex-valued output, all processing happens in there
      DIP_STACK_TRACE_THIS( out.ReForge( outSize, in_copy.TensorElements(), DataType::SuggestComplex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW ));
      Image tmp = out.QuickCopy();
      // Compute transform
      tmp.Protect(); // make sure it won't be reforged by the framework function.
      DIP_STACK_TRACE_THIS( DFT_C2C_compute( in_copy, tmp, process, inverse, corner, scale ));
      tmp.Protect( false );
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


dip::uint OptimalFourierTransformSize( dip::uint size, dip::String const& which, dip::String const& purpose ) {
   bool larger = BooleanFromString( which, S::LARGER, S::SMALLER );
   bool complex = BooleanFromString( purpose, S::COMPLEX, S::REAL );
   DIP_STACK_TRACE_THIS( size = GetOptimalDFTSize( size, larger, MaxFactor( complex )));
   DIP_THROW_IF( size == 0, E::SIZE_EXCEEDS_LIMIT );
   return size;
}


} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"
#include "diplib/statistics.h"

DOCTEST_TEST_CASE("[DIPlib] testing the FourierTransform function (2D image, 2D transform)") {
   dip::UnsignedArray sz{ 128, 105 }; // 105 = 3*5*7
   dip::dfloat sigma = 7.0;
   dip::FloatArray shift{ -5.432, -2.345 };
   dip::Image input{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   dip::Image expectedOutput{ sz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   dip::FloatArray outSigma{ static_cast< dip::dfloat >( sz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                             static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );

   // Complex-to-complex transform
   // Note: we test everything in place, if it works in place, it certainly will work with separate input and output images.
   dip::Image output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   double maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   dip::FourierTransform( output, output, { "inverse" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // Real-to-complex transform (even-sized axis)
   output = input.Copy();
   dip::FourierTransform( output, output );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   // Complex-to-real inverse transform  (even-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // Real-to-real inverse transform  (even-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );

   // === Repeat with different R2C and C2R dimension ===
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
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-4 ); // Much larger error because of smaller image

   // Complex-to-real inverse transform (odd-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // Real-to-real inverse transform  (odd-sized axis)
   dip::FourierTransform( output, output, { "inverse", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
}

DOCTEST_TEST_CASE("[DIPlib] testing the FourierTransform function (fast option)") {
   dip::dfloat sigma = 7.0;
   dip::FloatArray shift{ -5.432, -2.345 };
   dip::UnsignedArray sz{ 97, 107 }; // prime sizes
   dip::Image input( sz, 1, dip::DT_SFLOAT );
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );

   // = Expected output with C2C transform
   dip::UnsignedArray expectedOutSz{ 98, 108 }; // Same size for both FFT implementations
   dip::Image expectedOutput( expectedOutSz, 1, dip::DT_SFLOAT );
   expectedOutput.Fill( 0 );
   dip::FloatArray outSigma{ static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                             static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );
   expectedOutput.Crop( sz );

   // Complex-to-complex transform (fast)
   dip::Image output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   dip::dfloat maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   dip::FourierTransform( output, output, { "inverse", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz ); // This should really be a scaling, but it doesn't matter because the scaling factor is so small
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // = Expected output with R2C/C2R transform
   if( dip::usingFFTW ) {
      expectedOutSz = { 98, 108 };
   } else {
      expectedOutSz = { 100, 108 };
   }
   expectedOutput = dip::Image{ expectedOutSz, 1, dip::DT_SFLOAT };
   expectedOutput.Fill( 0 );
   outSigma = { static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, shift );
   expectedOutput.Crop( sz );

   // Real-to-complex transform (fast)
   output = input.Copy();
   dip::FourierTransform( output, output, { "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   // Complex-to-real inverse transform (fast)
   dip::FourierTransform( output, output, { "inverse", "fast", "real" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   output.Crop( sz ); // This should really be a scaling, but it doesn't matter because the scaling factor is so small
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );
}

DOCTEST_TEST_CASE("[DIPlib] testing the FourierTransform function (corner and symmetric options)") {
   // Note that we test these at the same time because they're orthogonal features
   dip::dfloat sigma = 7.0;
   dip::FloatArray shift{ -5.432, -2.345 };
   dip::UnsignedArray sz{ 64, 105 };
   dip::Image input( sz, 1, dip::DT_SFLOAT );
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );
   dip::Image expectedOutput( sz, 1, dip::DT_SFLOAT );
   expectedOutput.Fill( 0 );
   dip::FloatArray outSigma{ static_cast< dip::dfloat >( sz[ 0 ] ) / ( 2.0 * dip::pi * sigma ),
                             static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ) };
   dip::DrawBandlimitedPoint( expectedOutput, expectedOutput.GetCenter(), { 2.0 * dip::pi * outSigma.product() / std::sqrt( sz.product()) }, outSigma, 7.0 );
   dip::FloatArray newShift = shift;
   newShift += expectedOutput.GetCenter();
   dip::ShiftFT( expectedOutput, expectedOutput, newShift );
   dip::Wrap( expectedOutput, expectedOutput, { -static_cast< dip::sint >( sz[ 0 ] ) / 2,
                                                -static_cast< dip::sint >( sz[ 1 ] ) / 2 } );

   // Complex-to-complex transform (corner)
   dip::Image output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   dip::dfloat maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-6 ); // Much larger error because of smaller image, but smaller error also because of normalization

   dip::FourierTransform( output, output, { "inverse", "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // Real-to-complex transform (corner)
   output = input.Copy();
   dip::FourierTransform( output, output, { "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-6 ); // Much larger error because of smaller image, but smaller error also because of normalization

   // Complex-to-real inverse transform (corner)
   dip::FourierTransform( output, output, { "inverse", "real", "corner", "symmetric" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-9 );

   // === Test "corner" + "fast" option
   sz = { 97, 107 }; // prime sizes
   input = dip::Image{ sz, 1, dip::DT_SFLOAT };
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ] }, { 1 }, { sigma }, 7.0 );

   // = Expected output with C2C transform
   dip::UnsignedArray expectedOutSz{ 98, 108 };
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
   dip::FloatArray zoom{ static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / static_cast< dip::dfloat >( sz[ 0 ] ),
                         static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / static_cast< dip::dfloat >( sz[ 1 ] )};
   dip::Image expectedInverseOutput = dip::Resampling( input, zoom );
   expectedInverseOutput *= static_cast< dip::dfloat >( sz.product() ) / static_cast< dip::dfloat >( expectedOutSz.product() );

   // Complex-to-complex transform (fast + corner)
   output = dip::Convert( input, dip::DT_SCOMPLEX );
   dip::FourierTransform( output, output, { "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   dip::FourierTransform( input, output, { "corner" } );
   dip::FourierTransform( output, output, { "inverse", "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxabs = dip::MaximumAbs( output - expectedInverseOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-6 ); // interpolation error

   // = Expected output with R2C/C2R transform
   if( dip::usingFFTW ) {
      expectedOutSz = { 98, 108 };
   } else {
      expectedOutSz = { 100, 108 };
   }
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
   zoom = { static_cast< dip::dfloat >( expectedOutSz[ 0 ] ) / static_cast< dip::dfloat >( sz[ 0 ] ),
            static_cast< dip::dfloat >( expectedOutSz[ 1 ] ) / static_cast< dip::dfloat >( sz[ 1 ] )};
   expectedInverseOutput = dip::Resampling( input, zoom );
   expectedInverseOutput *= static_cast< dip::dfloat >( sz.product() ) / static_cast< dip::dfloat >( expectedOutSz.product() );

   // Real-to-complex transform (fast + corner)
   output = input.Copy();
   dip::FourierTransform( output, output, { "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-7 );

   // Complex-to-real inverse transform (corner)
   dip::FourierTransform( input, output, { "corner" } );
   dip::FourierTransform( output, output, { "inverse", "real", "corner", "fast" } );
   DOCTEST_CHECK( output.DataType() == dip::DT_SFLOAT );
   DOCTEST_CHECK( output.Sizes() == expectedOutSz );
   maxabs = dip::MaximumAbs( output - expectedInverseOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-6 ); // interpolation error
}

DOCTEST_TEST_CASE("[DIPlib] testing the FourierTransform function (3D image, 1D transform)") {
   dip::UnsignedArray sz{ 3, 32, 2 };
   dip::BooleanArray process{ false, true, false };
   dip::FloatArray shift{ 2.0, -1.765, 1.0 };
   dip::dfloat sigma = 3.0;
   dip::Image input( sz, 1, dip::DT_DFLOAT );
   input.Fill( 0 );
   dip::DrawBandlimitedPoint( input, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                       static_cast< dip::dfloat >( sz[ 1 ] / 2 ) + shift[ 1 ],
                                       static_cast< dip::dfloat >( sz[ 2 ] / 2 ) + shift[ 2 ] }, { 1 }, { sigma }, 7.0 );
   dip::Image expectedOutput( sz, 1, dip::DT_DFLOAT );
   expectedOutput.Fill( 0 );
   dip::FloatArray outSigma{ sigma, static_cast< dip::dfloat >( sz[ 1 ] ) / ( 2.0 * dip::pi * sigma ), sigma };
   dip::DrawBandlimitedPoint( expectedOutput, { static_cast< dip::dfloat >( sz[ 0 ] / 2 ) + shift[ 0 ],
                                                static_cast< dip::dfloat >( sz[ 1 ] / 2 ),
                                                static_cast< dip::dfloat >( sz[ 2 ] / 2 ) + shift[ 2 ] },
                              { std::sqrt( 2.0 * dip::pi ) * outSigma[ 1 ] }, outSigma, 7.0 );
   dip::ShiftFT( expectedOutput, expectedOutput, { 0.0, shift[ 1 ], 0.0 } );

    // Complex-to-complex transform (corner)
   dip::Image output = dip::Convert( input, dip::DT_DCOMPLEX );
   dip::FourierTransform( output, output, {}, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   dip::dfloat maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-8 ); // Much larger error because of smaller image

   dip::FourierTransform( output, output, { "inverse" }, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-18 );

   // Real-to-complex transform
   output = input.Copy();
   dip::FourierTransform( output, output, {}, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - expectedOutput ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 1e-8 ); // Much larger error because of smaller image

   // Complex-to-real inverse transform
   dip::FourierTransform( output, output, { "inverse", "real" }, process );
   DOCTEST_CHECK( output.DataType() == dip::DT_DFLOAT );
   DOCTEST_CHECK( output.Sizes() == sz );
   maxabs = dip::MaximumAbs( output - input ).As< double >();
   //std::cout << "max = " << maxabs << '\n';
   DOCTEST_CHECK( maxabs < 2e-18 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
