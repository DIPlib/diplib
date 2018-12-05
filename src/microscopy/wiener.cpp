/*
 * DIPlib 3.0
 * This file contains functions for Wiener deconvolution
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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
#include "diplib/microscopy.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

namespace dip {

namespace {

bool ParseWienerOptions( StringSet const& options ) {
   bool isOtf = false;
   for( auto& opt : options ) {
      if( opt == "OTF" ) {
         isOtf = true;
      } else {
         DIP_THROW_INVALID_FLAG( opt );
      }
   }
   return isOtf;
}

Image GetOTF( Image const& psf, UnsignedArray const& sizes, bool isOtf ) {
   Image H;
   if( isOtf ) {
      H = psf.QuickCopy();
      DIP_THROW_IF( H.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   } else {
      DIP_THROW_IF( !psf.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_STACK_TRACE_THIS( H = psf.Pad( sizes ));
      FourierTransform( H, H );
   }
   return H;
}

}

void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image const& signalPower,
      Image const& noisePower,
      Image& out,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged() || !psf.IsForged() || !noisePower.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !psf.IsScalar() || !noisePower.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal() || !noisePower.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   bool isOtf;
   DIP_STACK_TRACE_THIS( isOtf = ParseWienerOptions( options ));
   // Fourier transforms etc.
   Image H;
   DIP_STACK_TRACE_THIS( H = GetOTF( psf, in.Sizes(), isOtf ));
   Image G = FourierTransform( in );
   Image S;
   if( signalPower.IsForged() ) {
      DIP_THROW_IF( !signalPower.IsScalar(), E::IMAGE_NOT_SCALAR );
      DIP_THROW_IF( !signalPower.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      S = signalPower;
   } else {
      S = SquareModulus( G );
   }
   Image N = noisePower.QuickCopy(); // To make the computations below look more like the equation
   // Compute the Wiener filter in the frequency domain
   MultiplyConjugate( G, H, G, G.DataType() );
   MultiplySampleWise( G, S, G, G.DataType() );
   Image divisor = SquareModulus( H );
   MultiplySampleWise( divisor, S, divisor, divisor.DataType() );
   divisor += noisePower;
   G /= divisor; // Not using dip::SafeDivide() on purpose: zeros indicate a true problem here
   // Inverse Fourier transform
   DIP_STACK_TRACE_THIS( FourierTransform( G, out, { S::INVERSE, S::REAL } ));
}

void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged() || !psf.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !psf.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( regularization <= 0, E::PARAMETER_OUT_OF_RANGE );
   bool isOtf;
   DIP_STACK_TRACE_THIS( isOtf = ParseWienerOptions( options ));
   // Fourier transforms etc.
   Image H;
   DIP_STACK_TRACE_THIS( H = GetOTF( psf, in.Sizes(), isOtf ));
   Image G = FourierTransform( in );
   // Compute the Wiener filter in the frequency domain
   MultiplyConjugate( G, H, G, G.DataType() );
   Image divisor = SquareModulus( H );
   dfloat K = regularization * Maximum( divisor ).As< dfloat >();
   divisor += K;
   G /= divisor; // Not using dip::SafeDivide() on purpose: zeros indicate a true problem here
   // Inverse Fourier transform
   DIP_STACK_TRACE_THIS( FourierTransform( G, out, { S::INVERSE, S::REAL } ));
}

} // namespace dip
