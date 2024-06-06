/*
 * DIPlib 3.0
 * This file contains functions for RL deconvolution
 *
 * (c)2022, Cris Luengo.
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

#include "diplib/deconvolution.h"

#include <tuple>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/transform.h"

#include "common_deconv_utility.h"

namespace dip {

namespace {

std::tuple< bool, bool >  ParseRichardsonLucyOptions( StringSet const& options ) {
   bool isOtf = false;
   bool pad = false;
   for( auto const& opt : options ) {
      if( opt == S::OTF ) {
         isOtf = true;
      } else if( opt == S::PAD ) {
         pad = true;
      } else {
         DIP_THROW_INVALID_FLAG( opt );
      }
   }
   return { isOtf, pad };
}

} // namespace

void RichardsonLucy(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization,
      dip::uint nIterations,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged() || !psf.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !psf.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( regularization < 0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( nIterations < 1, E::INVALID_PARAMETER );
   bool isOtf{}, pad{};
   DIP_STACK_TRACE_THIS( std::tie( isOtf, pad ) = ParseRichardsonLucyOptions( options ));
   DIP_THROW_IF( pad && isOtf, E::ILLEGAL_FLAG_COMBINATION );

   // Fourier transform of inputs

   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( psf.Dimensionality() != nDims, E::DIMENSIONALITIES_DONT_MATCH );
   Image g, H;
   if( pad ) {
      dip::UnsignedArray sizes = in.Sizes();
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         sizes[ ii ] += OptimalFourierTransformSize( sizes[ ii ] + psf.Size( ii ) - 1, S::LARGER, S::REAL );
      }
      g = ExtendImageToSize( in, sizes, S::CENTER );
   } else {
      g = in;
   }
   DIP_STACK_TRACE_THIS( H = GetOTF( psf, g.Sizes(), isOtf ));


   // Our first guess for the output is the input
   Image temp_out;
   Image& f = pad ? temp_out : out; // we use `temp_out` if padding, otherwise directly use `out`.
   f.Copy( g );
   RangeArray window;
   if( pad ) {
      window = f.CropWindow( in.Sizes() );
   }

   Image Tmp, tmp, grad, F;
   while( true ) {
      // f_{k+1} = { [ g / ( f_k * h ) ] * h^c } f_k
      // f_{k+1} = { [ g / ( f_k * h ) ] * h^c } f_k / { 1 - regularization div( grad(f_k) / |grad(f_k)| ) }
      DIP_START_STACK_TRACE
         if( regularization != 0 ) {
            Gradient( f, grad, { 0 }, S::FINITEDIFF );
            Norm( grad, tmp );
            SafeDivide( grad, tmp, grad, grad.DataType());
            Divergence( grad, tmp, { 0 }, S::FINITEDIFF );
            tmp *= -regularization;
            tmp += 1;
            SafeDivide( f, tmp, f, f.DataType() );
         }
         FourierTransform( f, F );
         Multiply( F, H, Tmp );
         FourierTransform( Tmp, tmp, { S::INVERSE, S::REAL } );
         SafeDivide( g, tmp, tmp, tmp.DataType() );
         FourierTransform( tmp, Tmp );
         MultiplyConjugate( Tmp, H, Tmp, Tmp.DataType() );
         FourierTransform( Tmp, tmp, { S::INVERSE, S::REAL } );
         f *= tmp;
      DIP_END_STACK_TRACE

      // Do we stop iterating?
      if( --nIterations == 0 ) {
         break;
      }
   }

   // When padding, we used `temp_out`; crop and write to `out`.
   if( pad ) {
      out = f.At( window );
   }
}

} // namespace dip
