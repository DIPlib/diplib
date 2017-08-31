/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the percentile filter.
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
#include "diplib/morphology.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"

namespace dip {

namespace {

// TODO: an 8 or 16-bit specialization could use the moving histogram technique
template< typename TPI >
class RankLineFilter : public Framework::FullLineFilter {
   public:
      RankLineFilter( dip::uint rank ) : rank_( static_cast< dip::sint >( rank )) {}
      void SetNumberOfThreads( dip::uint threads ) override {
         buffers_.resize( threads );
      }
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint nKernelPixels, dip::uint nRuns ) override {
         return lineLength * (
               nKernelPixels // copying
               + 3 * nKernelPixels * static_cast< dip::uint >( std::round( std::log( nKernelPixels )))  // sorting
               + 2 * nKernelPixels + nRuns );   // iterating over pixel table
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         dip::uint N = pixelTable.NumberOfPixels();
         buffers_[ params.thread ].resize( N );
         for( dip::uint ii = 0; ii < length; ++ii ) {
            TPI* buffer = buffers_[ params.thread ].data();
            for( auto offset : pixelTable ) {
               *buffer = in[ offset ];
               ++buffer;
            }
            auto ourGuy = buffers_[ params.thread ].begin() + rank_;
            std::nth_element( buffers_[ params.thread ].begin(), ourGuy, buffers_[ params.thread ].end() );
            *out = *ourGuy;
            in += inStride;
            out += outStride;
         }
      }
   private:
      dip::sint rank_;
      std::vector< std::vector< TPI >> buffers_;
};

void ComputeRankFilter(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      dip::uint rank,
      BoundaryConditionArray const& bc
) {
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, RankLineFilter, ( rank ), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter, Framework::Full_AsScalarImage );
   DIP_END_STACK_TRACE
}

} // namespace

void RankFilter(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      dip::uint rank,
      String const& order,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !se.IsFlat(), E::KERNEL_NOT_BINARY );
   Kernel kernel;
   dip::uint nPixels;
   DIP_START_STACK_TRACE
      kernel = se.Kernel();
      nPixels = kernel.NumberOfPixels( in.Dimensionality());
   DIP_END_STACK_TRACE
   DIP_THROW_IF(( rank < 1 ) || ( rank > nPixels ), E::PARAMETER_OUT_OF_RANGE );
   DIP_START_STACK_TRACE
      if( !BooleanFromString( order, "increasing", "decreasing" )) {
         rank = nPixels - rank + 1;
      }
   DIP_END_STACK_TRACE
   if( rank == 1 ) {
      DIP_STACK_TRACE_THIS( Erosion( in, out, se, boundaryCondition ));
   } else if( rank == nPixels ) {
      DIP_STACK_TRACE_THIS( Dilation( in, out, se, boundaryCondition ));
   } else {
      DIP_START_STACK_TRACE
         BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
         if( bc.empty() ) {
            if( rank <= nPixels / 2 ) {
               bc.push_back( BoundaryCondition::ADD_MAX_VALUE );
            } else {
               bc.push_back( BoundaryCondition::ADD_MIN_VALUE );
            }
         }
         ComputeRankFilter( in, out, kernel, rank - 1, bc );
      DIP_END_STACK_TRACE
   }
}

void PercentileFilter(
      Image const& in,
      Image& out,
      dfloat percentile,
      Kernel const& kernel,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( kernel.HasWeights(), E::KERNEL_NOT_BINARY );
   DIP_THROW_IF(( percentile < 0.0 ) || ( percentile > 100.0 ), E::PARAMETER_OUT_OF_RANGE );
   DIP_START_STACK_TRACE
      dip::uint nPixels = kernel.NumberOfPixels( in.Dimensionality());
      dip::uint rank = static_cast< dip::uint >( std::round( static_cast< dfloat >( nPixels ) * percentile / 100.0 ));
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      ComputeRankFilter( in, out, kernel, rank, bc );
   DIP_END_STACK_TRACE
}

} // namespace dip
