/*
 * (c)2022, Cris Luengo
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

#include <memory>
#include <cmath>
#include "diplib/transform.h"

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class HaarWaveletLineFilter : public Framework::SeparableLineFilter {
      using TPF = FloatType< TPI >;
   public:
      HaarWaveletLineFilter( bool isForward ) : isForward( isForward ) {}
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /*nTensorElements*/, dip::uint /*border*/, dip::uint /*procDim*/ ) override {
         return 2 * lineLength;
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         const TPF scale = static_cast< TPF >( 1 / std::sqrt( 2 ));
         DIP_ASSERT( params.inBuffer.length == params.outBuffer.length );
         DIP_ASSERT(( params.inBuffer.length & 1u ) == 0 );
         dip::uint N = params.inBuffer.length / 2;
         dip::sint inStride = params.inBuffer.stride;
         dip::sint outStride = params.outBuffer.stride;
         dip::sint inPair = static_cast< dip::sint >( isForward ? 1u : N ) * inStride;
         dip::sint outPair = static_cast< dip::sint >( isForward ? N : 1u ) * outStride;
         if( isForward ) {
            inStride *= 2;
         } else {
            outStride *= 2;
         }
         TPI* inData = static_cast< TPI* >( params.inBuffer.buffer );
         TPI* outData = static_cast< TPI* >( params.outBuffer.buffer );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            outData[ 0 ] = ( inData[ 0 ] + inData[ inPair ] ) * scale;
            outData[ outPair ] = ( inData[ 0 ] - inData[ inPair ] ) * scale;
            inData += inStride;
            outData += outStride;
         }
      }
   private:
      bool isForward;
};

void HaarWaveletTransformStep(
      Image& img,
      bool isForward,
      BooleanArray const& process
) {
   DIP_START_STACK_TRACE
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, HaarWaveletLineFilter, ( isForward ), img.DataType() );
      Framework::Separable( img, img, img.DataType(), img.DataType(), process, { 0 }, {}, *lineFilter,
                            Framework::SeparableOption::AsScalarImage );
   DIP_END_STACK_TRACE
}

} // namespace

void HaarWaveletTransform(
      Image const& in,
      Image& out,
      dip::uint nLevels,
      String const& direction,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( nLevels == 0 ) {
      out = in;
      return;
   }
   bool isForward{};
   DIP_STACK_TRACE_THIS( isForward = BooleanFromString( direction, S::FORWARD, S::INVERSE ));

   // Figure out what sizes out must have
   dip::uint multiple = static_cast< dip::uint >( std::pow( 2, nLevels ));
   DIP_STACK_TRACE_THIS( ArrayUseParameter( process, in.Dimensionality(), true ));
   UnsignedArray sizes = in.Sizes();
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      if( process[ ii ] ) {
         DIP_THROW_IF( static_cast< dfloat>( nLevels ) > std::floor( std::log2( sizes[ ii ] )), E::PARAMETER_OUT_OF_RANGE );
         sizes[ ii ] = div_ceil( sizes[ ii ], multiple ) * multiple;
      }
   }
   if( !isForward ) {
      DIP_THROW_IF( in.Sizes() != sizes, "Unexpected image sizes for an inverse Haar wavelet transform" );
   }

   // Does out have the right sizes and data type?
   Image inCopy = in; // in case *in == *out, and we need to strip out. NOLINT(*-unnecessary-copy-initialization)
   if( out.IsForged() && ( out.Sizes() != sizes )) {
      // We'll have to resize it, let's strip now, make things easier later on.
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   DataType dt = DataType::SuggestFlex( inCopy.DataType() );
   if( out.IsForged() && ( out.DataType() != dt )) {
      // OK, maybe it's still OK to use this data type...
      if( !out.DataType().IsFlex() || ( out.DataType().IsComplex() != dt.IsComplex() )) {
         // We'll have to resize it, let's strip now, make things easier later on.
         DIP_STACK_TRACE_THIS( out.Strip() ); // Will throw if the image is protected. We cannot force the output to be of an integral type, sorry.
      }
   }

   // Set the data type and protect so that, when we copy data into it, we'll have the right data type
   if( !out.IsForged() ) {
      out.SetDataType( dt );
   }
   bool outIsProtected = out.Protect();

   // Copy in to out, possibly padding with zeros so each process dimension is a multiple of 2^nLevels
   DIP_STACK_TRACE_THIS( ExtendImageToSize( inCopy, out, sizes, Option::CropLocation::TOP_LEFT, { BoundaryCondition::ADD_ZEROS } ));
   Image tmp = out.QuickCopy(); // This is the image we'll iteratively shrink as we compute the levels

   // The inverse transform starts small
   if( !isForward ) {
      multiple /= 2;
      for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
         if( process[ ii ] ) {
            sizes[ ii ] /= multiple;
         }
      }
      tmp.SetSizesUnsafe( sizes );
   }

   // Do nLevels iterations of the single step
   do {
      // Apply one level of the Haar wavelet transform
      DIP_STACK_TRACE_THIS( HaarWaveletTransformStep( tmp, isForward, process ));
      // Shrink or expand the image to half its size along each processing dimension, to prepare for the next iteration
      for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
         if( process[ ii ] ) {
            if( isForward ) {
               sizes[ ii ] /= 2;
            } else {
               sizes[ ii ] *= 2;
            }
         }
      }
      tmp.SetSizesUnsafe( sizes );
   } while( --nLevels > 0 );

   out.Protect( outIsProtected );
}

} // namespace dip
