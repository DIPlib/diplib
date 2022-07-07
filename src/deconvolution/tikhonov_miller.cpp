/*
 * DIPlib 3.0
 * This file contains functions for TM and ICTM deconvolution
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

#include "diplib.h"
#include "diplib/deconvolution.h"
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"

#include "common_deconv_utility.h"

namespace dip {

namespace {

std::tuple< bool, bool > ParseTikhonovMillerOptions( StringSet const& options ) {
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

Image ComputeMatrixC( UnsignedArray const& sizes ) {
   // Regularization (an ideal Laplacian) (in the frequency domain)
   dip::uint nD = sizes.size();
   Image C;
   for( dip::uint ii = 0; ii < nD; ++ii ) {
      Image ramp;
      CreateRamp( ramp, sizes, ii, { S::FREQUENCY } );
      ramp.UnexpandSingletonDimensions();
      Power( ramp, 2, ramp );
      if( ii == 0 ) {
         C = ramp;
      } else {
         C += ramp;
      }
   }
   C *= pi * pi;
   return C;
}

Image ComputeMatrixA( Image const& G, Image const& H, double regularization) {
   // Regularization matrix C (an ideal Laplacian) (in the frequency domain)
   Image CtC = ComputeMatrixC( G.Sizes() );
   // A = HtH + regularization CtC (in the frequency domain)
   SquareModulus( CtC, CtC );
   CtC *= regularization;
   Image A = SquareModulus( H );
   A += CtC;
   return A;
}

} // namespace

void TikhonovMiller(
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
   bool isOtf, pad;
   DIP_STACK_TRACE_THIS( std::tie( isOtf, pad ) = ParseTikhonovMillerOptions( options ));

   // Fourier transform of inputs
   Image G, H;
   DIP_STACK_TRACE_THIS( FourierTransformImageAndKernel( in, psf, G, H, isOtf, pad ));

   // A = HtH + regularization CtC (in the frequency domain)
   Image A = ComputeMatrixA( G, H, regularization );
   // H^T g (in the frequency domain)
   Image HtG = MultiplyConjugate( G, H );
   H.Strip();
   SafeDivide( HtG, A, G, G.DataType() );

   // Inverse Fourier transform
   if( pad ) {
      Image tmp;
      DIP_STACK_TRACE_THIS( FourierTransform( G, tmp, { S::INVERSE, S::REAL } ));
      out = tmp.Cropped( in.Sizes() );
   } else {
      DIP_STACK_TRACE_THIS( FourierTransform( G, out, { S::INVERSE, S::REAL } ));
   }
}

void IterativeConstrainedTikhonovMiller(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization,
      dfloat tolerance,
      dip::uint maxIterations,
      dfloat stepSize,
      StringSet const& options
) {
   DIP_THROW_IF( !in.IsForged() || !psf.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !psf.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( regularization <= 0, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( stepSize < 0, E::PARAMETER_OUT_OF_RANGE );
   bool isOtf, pad;
   DIP_STACK_TRACE_THIS( std::tie( isOtf, pad ) = ParseTikhonovMillerOptions( options ));
   bool steepest_descent = stepSize != 0;

   dfloat maxVal = MaximumAbs( in ).As< dfloat >();
   tolerance *= maxVal * maxVal;
   //std::cout << "tolerance = " << tolerance << '\n';

   // Fourier transform of inputs
   Image G, H;
   DIP_STACK_TRACE_THIS( FourierTransformImageAndKernel( in, psf, G, H, isOtf, pad ));

   // A = H^T H + regularization C^T C (in the frequency domain)
   Image A = ComputeMatrixA( G, H, regularization );
   // H^T g (in the frequency domain)
   Image HtG = MultiplyConjugate( G, H );

   // Our first guess for the output is the unconstrained Tikhonov-Miller solution
   //SafeDivide( HtG, A, G, G.DataType() );
   // ... except that that solution can fail miserably, so instead we use the input image
   Image F = G.Copy();
   Image temp_out;
   Image& f = pad ? temp_out : out; // we use `temp_out` if padding, otherwise directly use `out`.
   RangeArray window;
   if( pad ) {
      window = F.CropWindow( in.Sizes() );
   }

   // Initialize the remaining intermediate images used in the iterative process
   Image d;
   Image r;
   Image Tf{ true };
   Image rNorm;
   dfloat thetaPrev = 1e8; // previous step's objective function value
   Image tmp;
   while( true ) {
      // r = A * f - H^T * g
      r = A * F;
      r -= HtG;

      // d
      if( steepest_descent ) {
         d = r;
      } else if( !d.IsForged() ) {
         // First step of conjugate gradients
         d = r;
         SumSquareModulus( r, {}, rNorm );
         //std::cout << "rNorm = " << rNorm.As< double >() << '\n';
      } else {
         // d = r + |r|^2 / |rPrev|^2 * dPrev
         // |r| = sqrt(sum( abs(r(i))^2 )) (in the spatial domain) = sqrt(1/N sum( abs(r(i))^2 )) (in the Fourier domain)
         // using Parseval's theorem. We ignore the sqrt(1/N), since it is present on both sides of the division.
         SafeDivide( d, rNorm, d );
         SumSquareModulus( r, {}, rNorm );
         d *= rNorm;
         d += r;
         //std::cout << "rNorm = " << rNorm.As< double >() << '\n';
      }
      //std::cout << "dNorm = " << SumSquareModulus(d).As< double >() << '\n';

      // beta
      dfloat beta = -stepSize;
      if( !steepest_descent ) {
         // beta = - ( d^T T(f) r ) / ( d^T T(f) A T(f) d )
         // This must be computed in the spatial domain
         // r and d are in the Fourier domain, we need to inverse transform then first.
         Image dSpatial = FourierTransform( d, { S::INVERSE, S::REAL } );
         dSpatial *= Tf;
         Image rSpatial = FourierTransform( r, { S::INVERSE, S::REAL } );
         rSpatial *= Tf;
         // A is a convolution, so we compute it in the Fourier domain
         Image ATfd = FourierTransform( dSpatial );
         ATfd *= A;
         FourierTransform( ATfd, ATfd, { S::INVERSE, S::REAL } );
         // Put them all together. a^T b is the inner product between vectors a and b
         beta = - InProduct( dSpatial, rSpatial ) / InProduct( dSpatial, ATfd );
      }
      //std::cout << "beta = " << beta << '\n';

      // f = P( fPrev + beta * d )
      F += beta * d;

      // To the spatial domain so we can apply the non-negative constraint
      DIP_STACK_TRACE_THIS( FourierTransform( F, f, { S::INVERSE, S::REAL } ));
      Tf = f >= 0; // save this for next iteration
      //std::cout << "Number of negative values corrected: " << ( Tf.NumberOfPixels() - Count( Tf )) << '\n';
      f *= Tf; // P(.) sets negative pixels to 0.

      // Do we stop iterating?
      if( --maxIterations == 0 ) {
         //std::cout << "Terminating because `maxIterations`\n";
         break;
      }

      // Back to the frequency domain
      DIP_STACK_TRACE_THIS( FourierTransform( f, F ));

      // Do we stop iterating? (pt II)
      if( tolerance > 0 ) {
         // Note that we ignore the regularization term of the objective function
         MultiplySampleWise( F, H, tmp );
         tmp -= G;
         dfloat theta = MeanSquareModulus( tmp ).As< dfloat >() / static_cast< dfloat >( tmp.NumberOfPixels() );
         //std::cout << "theta = " << theta << '\n';
         if(( thetaPrev - theta ) < tolerance ) {
            //std::cout << "Terminating because objective function changed less than `tolerance`\n";
            break;
         }
         thetaPrev = theta;
      }
   }

   // When padding, we used `temp_out`; crop and write to `out`.
   if( pad ) {
      out = f.At( window );
   }
}

} // namespace dip
