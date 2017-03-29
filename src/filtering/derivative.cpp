/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the Gaussian filter.
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
#include <diplib/framework.h>
#include <diplib/overload.h>
#include <diplib/transform.h>

namespace dip {

namespace {

void GaussDispatch(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   // If any( sigmas < 0.8 ) || any( derivativeOrder > 3 )  ==>  FT
   // Else if any( sigmas > 10 )  ==>  IIR
   // Else ==>  FIR
   for( dip::uint ii = 0; ii < derivativeOrder.size(); ++ii ) { // We can't fold this loop in with the next one, the two arrays might be of different size
      if( derivativeOrder[ ii ] > 3 ) {
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if( sigmas[ ii ] < 0.8 ) {
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if( sigmas[ ii ] > 10 ) {
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
         return;
      }
   }
   GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
}

} // namespace

void Gauss(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      String method,
      dfloat truncation
) {
   if( method == "best" ) {
      DIP_START_STACK_TRACE
         GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "FIR" ) || ( method == "fir" ) ) {
      DIP_START_STACK_TRACE
         GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "FT" ) || ( method == "ft" ) ) {
      DIP_START_STACK_TRACE
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
      DIP_END_STACK_TRACE
   } else if( ( method == "IIR" ) || ( method == "iir" ) ) {
      DIP_START_STACK_TRACE
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
      DIP_END_STACK_TRACE
   } else {
      DIP_THROW( "Unknown Gauss filter method" );
   }
}

void Derivative(
      Image const& in,
      Image& out,
      UnsignedArray const& derivativeOrder,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   if( method == "finitediff" ) {
      // Set process to false where sigma <= 0 and derivativeOrder == 0;
      BooleanArray ps = process;
      FloatArray ss = sigmas;
      UnsignedArray order = derivativeOrder;
      dip::uint nDims = in.Dimensionality();
      DIP_START_STACK_TRACE
         ArrayUseParameter( ps, nDims, true );
         ArrayUseParameter( ss, nDims, 1.0 );
         ArrayUseParameter( order, nDims, dip::uint( 0 ));
      DIP_END_STACK_TRACE
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if((( ss[ ii ] <= 0.0 ) && ( order[ ii ] == 0 )) || ( in.Size( ii ) == 1 )) {
            ps[ ii ] = 0;
         }
      }
      DIP_START_STACK_TRACE
         FiniteDifference( in, out, order, "smooth", boundaryCondition, ps );
      DIP_END_STACK_TRACE
   } else if( ( method == "best" ) || ( method == "gauss" ) ) {
      DIP_START_STACK_TRACE
         GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussFIR" ) || ( method == "gaussfir" ) ) {
      DIP_START_STACK_TRACE
         GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, truncation );
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussFT" ) || ( method == "gaussft" ) ) {
      DIP_START_STACK_TRACE
         GaussFT( in, out, sigmas, derivativeOrder, process, truncation ); // ignores boundaryCondition
      DIP_END_STACK_TRACE
   } else if( ( method == "gaussIIR" ) || ( method == "gaussiir" ) ) {
      DIP_START_STACK_TRACE
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, process, {}, "", truncation );
      DIP_END_STACK_TRACE
   } else {
      DIP_THROW( "Unknown derivative method" );
   }
}

void Gradient(
      Image const& c_in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   Image in = c_in.QuickCopy();
   PixelSize ps = c_in.PixelSize();
   dip::uint nDims = in.Dimensionality();
   if( out.IsForged() && in.Aliases( out )) {
      out.Strip();
   }
   out.ReForge( in.Sizes(), nDims, DataType::SuggestFlex( in.DataType() ));
   // TODO: Create dip::TensorIterator and use it here.
   dip::UnsignedArray order( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      order[ ii ] = 1;
      Image tmp = out[ ii ];
      Derivative( in, tmp, order, sigmas, method, boundaryCondition, process, truncation );
      order[ ii ] = 0;
   }
   out.SetPixelSize( ps );
}

void Hessian (
      Image const& c_in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   Image in = c_in.QuickCopy();
   PixelSize ps = c_in.PixelSize();
   dip::uint nDims = in.Dimensionality();
   if( out.IsForged() && in.Aliases( out )) {
      out.Strip();
   }
   Tensor tensor( Tensor::Shape::SYMMETRIC_MATRIX, nDims, nDims );
   out.ReForge( in.Sizes(), tensor.Elements(), DataType::SuggestFlex( in.DataType() ));
   out.ReshapeTensor( tensor );
   dip::UnsignedArray order( nDims, 0 );
   for( dip::uint jj = 0; jj < nDims; ++jj ) {
      for( dip::uint ii = 0; ii <= jj; ++ii ) {
         ++order[ ii ];
         ++order[ jj ];
         Image tmp = out[ { ii, jj } ];
         Derivative( in, tmp, order, sigmas, method, boundaryCondition, process, truncation );
         order[ ii ] = 0;
         order[ jj ] = 0;
      }
   }
   out.SetPixelSize( ps );
}

} // namespace dip
