/*
 * DIPlib 3.0
 * This file contains functions for FISTA deconvolution
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

#include <cmath>
#include <tuple>

#include "diplib.h"
#include "diplib/mapping.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

#include "common_deconv_utility.h"

namespace dip {

namespace {

std::tuple< bool, bool >  ParseFISTAOptions( StringSet const& options ) {
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

void FastIterativeShrinkageThresholding(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization,
      dfloat tolerance,
      dip::uint maxIterations,
      dip::uint nScales,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged() || !psf.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !psf.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( regularization <= 0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( nScales < 1, E::PARAMETER_OUT_OF_RANGE );
   bool isOtf{}, pad{};
   DIP_STACK_TRACE_THIS( std::tie( isOtf, pad ) = ParseFISTAOptions( options ));

   dfloat maxVal = MaximumAbs( in ).As< dfloat >();
   tolerance *= maxVal * maxVal;
   //std::cout << "tolerance = " << tolerance << '\n';
   constexpr dfloat stepSize = 0.5; // Do we want to make this a parameter, or doesn't it matter?
   regularization *= stepSize;

   // Fourier transform of inputs etc.
   Image G, H;
   DIP_STACK_TRACE_THIS( FourierTransformImageAndKernel( in, psf, G, H, isOtf, pad, nScales ));
   pad = in.Sizes() != G.Sizes();

   // A = 1 - 2 * s * H^T H (in the frequency domain)
   Image A = SquareModulus( H );
   A *= 2 * stepSize;
   Subtract( 1.0, A, A, A.DataType() );
   // B = 2 * s * H^T g (in the frequency domain)
   Image B = MultiplyConjugate( G, H );
   B *= 2 * stepSize;

   // x is the output
   Image temp_out;
   Image& x = pad ? temp_out : out; // we use `temp_out` if padding, otherwise directly use `out`.
   RangeArray window;
   if( pad ) {
      window = G.CropWindow( in.Sizes() );
   }
   FourierTransform( G, x, { S::INVERSE, S::REAL } );
   Image Y = G.Copy();

   dfloat t = 1;
   Image xPrev, y;
   dfloat thetaPrev = 1e8;
   Image tmp;
   while( true ) {
      Copy( x, xPrev );

      // Compute y (2nd part, we skip the 1st part in the first iteration)
      // Y = X - 2 s (H^T H X + H^T G) = (1 - 2 s H^T H) X + 2 s H^T G = A X + B
      Y *= A;
      Y += B;
      FourierTransform( Y, y, { S::INVERSE, S::REAL } );

      // Shrinkage-threshold of y yields x
      HaarWaveletTransform( y, y, nScales );
      Shrinkage( y, y, regularization );
      HaarWaveletTransform( y, x, nScales, S::INVERSE );
      // Using stationary wavelets we can do the same thing without all the blockiness, but it is much, much slower.
      //dip::Image tmp = StationaryWaveletTransform( y, nScales );
      //Shrinkage( tmp, tmp, regularization );
      //dip::BooleanArray process( tmp.Dimensionality(), false );
      //process.back() = true;
      //Sum( tmp, {}, tmp, process );
      //x = tmp.Squeeze();

      // Do we stop iterating?
      if( --maxIterations == 0 ) {
         //std::cout << "Terminating because maxIterations\n";
         break;
      }
      if( tolerance > 0 ) {
         // Note that we ignore the regularization term of the objective function
         DIP_STACK_TRACE_THIS( FourierTransform( x, tmp ));
         tmp *= H;
         tmp -= G;
         dfloat theta = MeanSquareModulus( tmp ).As< dfloat >() / static_cast< dfloat >( tmp.NumberOfPixels() );
         //std::cout << "theta = " << theta << '\n';
         if( thetaPrev - theta < tolerance ) {
            //std::cout << "Terminating because objective function changed less than `tolerance`\n";
            break;
         }
         thetaPrev = theta;
      }

      // Compute y (1st part)
      dfloat tPrev = t;
      t = 0.5 + std::sqrt( 0.25 + t * t );
      Subtract( x, xPrev, y, x.DataType() );
      y *= ( tPrev - 1 ) / t;
      y += x;
      FourierTransform( y, Y );
   }

   // When padding, we used `temp_out`; crop and write to `out`.
   if( pad ) {
      out = x.At( window );
   }
}

} // namespace dip
