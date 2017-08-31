/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the variance filter.
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
#include "diplib/nonlinear.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class VarianceLineFilter : public Framework::FullLineFilter {
   public:
      virtual void SetNumberOfThreads( dip::uint threads ) override {
         accumulators_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint nKernelPixels, dip::uint nRuns ) {
         return 5 * nKernelPixels + lineLength * (
               nRuns * 10     // number of multiply-adds
               + nRuns );     // iterating over pixel table runs
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         VarianceAccumulator& acc = accumulators_[ params.thread ];
         acc.Reset();
         for( auto offset : pixelTable ) {
            acc.Push( in[ offset ] );
         }
         *out = static_cast< TPI >( acc.Variance() );
         //in += inStride; // we don't increment `in` here, so that we don't have to subtract one index inside the loop
         //out += outStride; // we don't increment `out` here, we increment it in the loop before the assignment, it saves one addition! :)
         for( dip::uint ii = 1; ii < length; ++ii ) {
            for( auto run : pixelTable.Runs() ) {
               acc.Pop( in[ run.offset ] );
               acc.Push( in[ run.offset + static_cast< dip::sint >( run.length ) * inStride ] );
            }
            in += inStride;
            out += outStride;
            *out = static_cast< TPI >( acc.Variance() );
         }
      }
   private:
      std::vector< VarianceAccumulator > accumulators_;
};

} // namespace

void VarianceFilter(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( kernel.HasWeights(), E::KERNEL_NOT_BINARY );
   DIP_START_STACK_TRACE
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT( lineFilter, VarianceLineFilter, (), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter, Framework::Full_AsScalarImage );
   DIP_END_STACK_TRACE
}

} // namespace dip
