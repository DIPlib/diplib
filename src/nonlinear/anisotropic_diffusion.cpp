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
#include "diplib/linear.h"
#include "diplib/framework.h"

namespace dip {

namespace {

template< typename TPI, bool forward_ >
class FiniteDifferenceLineFilter : public Framework::SeparableLineFilter {
   public:
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         dip::sint step = forward_ ? inStride : -inStride;
         dip::uint length = params.inBuffer.length;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         for( dip::uint ii = 0; ii < length; ++ii ) {
            *out = in[ step ] - in[ 0 ];
            in += inStride;
            out += outStride;
         }
      }
};

} // namespace

void PeronaMalik(
      Image const& in,
      Image& out,
      dip::uint iterations,
      dfloat K,
      dfloat lambda,
      String g_s
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( iterations < 1, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( K <= 0.0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF(( lambda <= 0.0 ) || ( lambda > 1.0 ), E::PARAMETER_OUT_OF_RANGE );

   // Create a line filter for the scan framework that applies `g`.
   std::unique_ptr< Framework::ScanLineFilter > g;
   sfloat fK = static_cast< sfloat >( K );
   if( g_s == "Gauss" ) {
      g = Framework::NewMonadicScanLineFilter< sfloat >(
            [ fK ]( auto its ) { sfloat v = *its[ 0 ] / fK; return std::exp( -v * v ); },
            20 );
   } else if( g_s == "quadratic") {
      g = Framework::NewMonadicScanLineFilter< sfloat >(
            [ fK ]( auto its ) { sfloat v = *its[ 0 ] / fK; return 1.0f / ( 1.0f + ( v * v )); },
            5 );
   } else if( g_s == "exponential") {
      g = Framework::NewMonadicScanLineFilter< sfloat >(
            [ fK ]( auto its ) { sfloat v = *its[ 0 ] / fK; return std::exp( -std::abs( v )); },
            20 );
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }
   FiniteDifferenceLineFilter< sfloat, true > forwardFiniteDifference;
   FiniteDifferenceLineFilter< sfloat, false > backwardFiniteDifference;

   // Each iteration is applied to `out`.
   Convert( in, out, DT_SFLOAT );
   dip::uint nDims = in.Dimensionality();
   BooleanArray process( nDims, false );
   UnsignedArray border( nDims, 1 );
   BoundaryConditionArray bc( nDims, BoundaryCondition::ADD_ZEROS );
   for( dip::uint ii = 0; ii < iterations; ++ii ) {
      // TODO: each iteration could be computed in a single step of a Framework::Full filter.
      Image fd;
      Image c;
      process[ 0 ] = true;
      Framework::Separable( out, fd, DT_SFLOAT, DT_SFLOAT, process, border, bc, forwardFiniteDifference );
      Framework::ScanMonadic( fd, c, DT_SFLOAT, DT_SFLOAT, 1, *g );
      Image delta = fd * c;
      Framework::Separable( out, fd, DT_SFLOAT, DT_SFLOAT, process, border, bc, backwardFiniteDifference );
      Framework::ScanMonadic( fd, c, DT_SFLOAT, DT_SFLOAT, 1, *g );
      c *= fd; delta += c; // delta += fd * c;
      process[ 0 ] = false;
      for( dip::uint jj = 1; jj < nDims; ++jj ) {
         process[ jj ] = true;
         Framework::Separable( out, fd, DT_SFLOAT, DT_SFLOAT, process, border, bc, forwardFiniteDifference );
         Framework::ScanMonadic( fd, c, DT_SFLOAT, DT_SFLOAT, 1, *g );
         c *= fd; delta += c; // delta += fd * c;
         Framework::Separable( out, fd, DT_SFLOAT, DT_SFLOAT, process, border, bc, backwardFiniteDifference );
         Framework::ScanMonadic( fd, c, DT_SFLOAT, DT_SFLOAT, 1, *g );
         c *= fd; delta += c; // delta += fd * c;
         process[ jj ] = false;
      }
      delta *= lambda;
      out += delta;
   }
}

} // namespace dip
