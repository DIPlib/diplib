/*
 * DIPlib 3.0
 * This file contains functions for anisotropic diffusion.
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

namespace dip {

namespace {

template< typename F >
class PeronaMalikLineFilter : public Framework::FullLineFilter {
   public:
      PeronaMalikLineFilter( F const& func, dip::uint cost, sfloat lambda ) : func_( func ), cost_( cost ), lambda_( lambda ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return cost_ * lineLength;
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         sfloat* in = static_cast< sfloat* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         sfloat* out = static_cast< sfloat* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         dip::uint nDims = params.pixelTable.Dimensionality();
         std::vector< PixelTableOffsets::PixelRun > pixelTableRuns = params.pixelTable.Runs(); // copy!
         // The pixel table has `(nDims-1)*2+1` runs of length 1, and 1 run of length 3:
         DIP_ASSERT( pixelTableRuns.size() == ( nDims - 1 ) * 2 + 1 );
         for( dip::uint ii = 0; ii < ( nDims - 1 ) * 2 + 1; ++ii ) {
            if( pixelTableRuns[ ii ].length == 3 ) {
               //DIP_ASSERT( pixelTableRuns[ ii ].offset == -inStride );
               pixelTableRuns[ ii ].length = 1; // the run should have a length of 1
               pixelTableRuns.push_back( { -pixelTableRuns[ ii ].offset, 1 } ); // add another run of length one to the other side
            //} else {
               //DIP_ASSERT( pixelTableRuns[ ii ].length == 1 );
            }
         }
         //DIP_ASSERT( pixelTableRuns.size() == nDims * 2 );
         // Now the pixel table has `2*nDims` runs, one for each neighbor. The `offset` is the address of the neighbor.
         for( dip::uint ii = 0; ii < length; ++ii ) {
            sfloat delta = 0;
            for( auto run : pixelTableRuns ) {
               sfloat diff = in[ run.offset ] - in[ 0 ];
               delta += func_( diff ) * diff;
            }
            *out = *in + lambda_ * delta;
            in += inStride;
            out += outStride;
         }
      }
   private:
      F func_; // save a copy of the lambda, in case we want to use it with a temporary-constructed lambda that captures a variable.
      dip::uint cost_;
      sfloat lambda_;
};

template< typename F >
inline std::unique_ptr< Framework::FullLineFilter > NewPeronaMalikLineFilter( F const& func, dip::uint cost, sfloat lambda ) {
   return static_cast< std::unique_ptr< Framework::FullLineFilter >>( new PeronaMalikLineFilter< F >( func, cost, lambda ));
}

} // namespace

void PeronaMalik(
      Image const& in,
      Image& out,
      dip::uint iterations,
      dfloat K,
      dfloat lambda,
      String g
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( iterations < 1, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( K <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF(( lambda <= 0.0 ) || ( lambda > 1.0 ), E::PARAMETER_OUT_OF_RANGE );

   // Create a line filter for the scan framework that applies `g`.
   std::unique_ptr< Framework::FullLineFilter > lineFilter;
   sfloat fK = static_cast< sfloat >( K );
   if( g == "Gauss" ) {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return std::exp( -v * v ); },
            20, static_cast< sfloat >( lambda ));
   } else if( g == "quadratic") {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return 1.0f / ( 1.0f + ( v * v )); },
            5, static_cast< sfloat >( lambda ));
   } else if( g == "exponential") {
      lineFilter = NewPeronaMalikLineFilter(
            [ fK ]( sfloat v ) { v /= fK; return std::exp( -std::abs( v )); },
            20, static_cast< sfloat >( lambda ));
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }

   // Each iteration is applied to `out`.
   BoundaryConditionArray bc( in.Dimensionality(), BoundaryCondition::ADD_ZEROS );
   Kernel kernel( Kernel::ShapeCode::DIAMOND, { 3 } );
   for( dip::uint ii = 0; ii < iterations; ++ii ) {
      Framework::Full( ii == 0 ? in : out, out, DT_SFLOAT, DT_SFLOAT, DT_SFLOAT, 1, bc, kernel, *lineFilter, Framework::FullOption::AsScalarImage );
   }
}

} // namespace dip
