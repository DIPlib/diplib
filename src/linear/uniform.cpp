/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the uniform mean filter.
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
#include "diplib/linear.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"

namespace dip {

namespace {


template< typename TPI >
class RectangularUniformLineFilter : public Framework::SeparableLineFilter {
   public:
      RectangularUniformLineFilter( UnsignedArray const& sizes ) :
            sizes_( sizes ) {}
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint filterSize = sizes_[ params.dimension ];
         FloatType< TPI > norm = 1 / static_cast< FloatType< TPI >>( filterSize );
         TPI* left = in - static_cast< dip::sint >( filterSize / 2 ) * inStride; // the leftmost pixel in the filter
         TPI* right = in + static_cast< dip::sint >(( filterSize + 1 ) / 2 ) * inStride; // one past the rightmost pixel in the filter
         TPI sum = 0;
         for( in = left; in != right; in += inStride ) {
            sum += *in;
         }
         *out = sum * norm;
         for( dip::uint ii = 1; ii < length; ++ii ) {
            sum -= *left;
            sum += *right;
            left += inStride;
            right += inStride;
            out += outStride;
            *out = sum * norm;
         }
      }
   private:
      UnsignedArray const& sizes_;
};

void RectangularUniform(
      Image const& in,
      Image& out,
      UnsignedArray const& filterSize,
      BoundaryConditionArray const& bc
) {
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   UnsignedArray sizes( nDims, 1 );
   UnsignedArray border( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( filterSize[ ii ] > 1 ) && ( in.Size( ii ) > 1 )) {
         sizes[ ii ] = filterSize[ ii ];
         process[ ii ] = true;
         border[ ii ] = sizes[ ii ] / 2;
      }
   }
   DIP_START_STACK_TRACE
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, RectangularUniformLineFilter, ( sizes ), dtype );
      Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter, Framework::Separable_AsScalarImage );
   DIP_END_STACK_TRACE
}


template< typename TPI >
class PixelTableUniformLineFilter : public Framework::FullLineFilter {
   public:
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         TPI sum = 0; // Sum of values within the filter
         for( auto offset : pixelTable ) {
            sum += in[ offset ];
         }
         FloatType< TPI > norm = 1 / static_cast< FloatType< TPI >>( pixelTable.NumberOfPixels() );
         *out = sum * norm;
         //in += inStride; // we don't increment `in` here, so that we don't have to subtract one index inside the loop
         //out += outStride; // we don't increment `out` here, we increment it in the loop before the assignment, it saves one addition! :)
         for( dip::uint ii = 1; ii < length; ++ii ) {
            for( auto run : pixelTable.Runs() ) {
               sum -= in[ run.offset ];
               sum += in[ run.offset + static_cast< dip::sint >( run.length ) * inStride ];
            }
            in += inStride;
            out += outStride;
            *out = sum * norm;
         }
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint nRuns ) override {
         return lineLength * nRuns * 4    // number of adds
                + lineLength * nRuns;     // iterating over pixel table runs
      }
};

void PixelTableUniform(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      BoundaryConditionArray const& bc
) {
   DIP_START_STACK_TRACE
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, PixelTableUniformLineFilter, (), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter, Framework::Full_AsScalarImage );
   DIP_END_STACK_TRACE
}


} // namespace

void Uniform(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( kernel.HasWeights(), E::KERNEL_NOT_BINARY );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      if( kernel.IsRectangular() ) {
         RectangularUniform(in, out, kernel.Sizes( in.Sizes() ), bc );
      } else {
         PixelTableUniform( in, out, kernel, bc );
      }
   DIP_END_STACK_TRACE
}


} // namespace dip
