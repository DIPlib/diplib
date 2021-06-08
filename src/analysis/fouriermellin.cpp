/*
 * DIPlib 3.0
 * This file contains definitions for the Fourier-Mellin transform
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
#include "diplib/analysis.h"
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/generation.h"
#include "diplib/geometry.h"

namespace dip {

// TODO: Is high-pass filtering necessary?
// In my experiments, it is the windowing that makes all the difference.
//
// At https://github.com/sthoduka/imreg_fmt/ they apply a high-pass filter:
//    a = 1-cos(rr([256,256],'radfreq')/2)^2;
// At https://www.lfd.uci.edu/~gohlke/code/imreg.py.html they apply a high-pass filter:
//    b = cos(xx1([256,256],'radfreq')/2) * cos(yy1([256,256],'radfreq')/2); b = (1.0 - b) * (2.0 - b);

FloatArray FourierMellinMatch2D(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& interpolationMethod,
      String const& correlationMethod
) {
   DIP_THROW_IF( !in1.IsForged() || !in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( in1.CheckProperties( 2, 1, DataType::Class_Real ));
   DIP_STACK_TRACE_THIS( in2.CheckProperties( in1.Sizes(), 1, DataType::Class_Real ));

   // Apply Fourier-Mellin transform
   Image fIn1 = ApplyWindow( in1, "GaussianTukey", 10 );
   FourierTransform( fIn1, fIn1 );
   Image fmIn1 = Abs( fIn1 );
   Ln( fmIn1, fmIn1 );
   LogPolarTransform2D( fmIn1, fmIn1, S::LINEAR ); // Don't use S::CUBIC_ORDER_3 here, it does not work.

   Image fmIn2 = ApplyWindow( in2, "GaussianTukey", 10 );
   Abs( FourierTransform( fmIn2 ), fmIn2 );
   Ln( fmIn2, fmIn2 );
   LogPolarTransform2D( fmIn2, fmIn2, S::LINEAR );

   DIP_ASSERT( fmIn2.Size( 0 ) == fmIn2.Size( 1 )); // The LogPolarTransform2D function returns a square image
   dfloat size = static_cast< dfloat >( fmIn2.Size( 0 ));

   // Compute cross-correlation
   String findShiftMethod;
   if( correlationMethod == S::DONT_NORMALIZE ) {
      findShiftMethod = S::CC;
   } else if( correlationMethod == S::NORMALIZE  ) {
      findShiftMethod = S::NCC;
   } else if( correlationMethod == S::PHASE  ) {
      findShiftMethod = S::PC;
   } else {
      DIP_THROW_INVALID_FLAG( correlationMethod );
   }
   FloatArray shift;
   DIP_STACK_TRACE_THIS( shift = FindShift( fmIn1, fmIn2, findShiftMethod ));
   //std::cout << "shift = " << shift << '\n';
   fmIn1.Strip();
   fmIn2.Strip();

   // Compute the scale and rotation (compare to computations in LogPolarTransform2D)
   FloatArray center = in1.GetCenter();
   dfloat maxr = center.minimum_value();
   dfloat zoom = std::pow( maxr, shift[ 0 ] / ( size - 1 ));
   dfloat theta = shift[ 1 ] * 2 * pi / size;
   //std::cout << "zoom = " << zoom << ", theta = " << theta << '\n';

   // Compute a transformed in2 for scale and rotation
   FloatArray matrix( 6, 0 );
   matrix[ 0 ] = zoom * cos( theta );
   matrix[ 1 ] = -zoom * sin( theta );
   matrix[ 2 ] = -matrix[ 1 ];
   matrix[ 3 ] = matrix[ 0 ];
   Image in2a = AffineTransform( in2, matrix, interpolationMethod );

   // Compute a 2nd transformed in2 rotated 180 degrees w.r.t. the first one
   Image in2b = in2a.QuickCopy();
   in2b.Rotation90( 2 ); // shares memory!

   // Compute cross-correlation, pick the rotation with the best match
   // (code modified from FindShift_CC, FindShift doesn't return the cross correlation value)
   Image cross;
   DIP_STACK_TRACE_THIS( CrossCorrelationFT( fIn1, in2a, cross, S::FREQUENCY, S::SPATIAL, S::SPATIAL, correlationMethod ));
   DIP_ASSERT( cross.DataType().IsReal());
   SubpixelLocationResult locA;
   DIP_STACK_TRACE_THIS( locA = SubpixelLocation( cross, MaximumPixel( cross, {} )));
   locA.coordinates -= cross.GetCenter();
   //std::cout << "A: coordinates = " << locA.coordinates << ", value = " << locA.value << '\n';

   DIP_STACK_TRACE_THIS( CrossCorrelationFT( fIn1, in2b, cross, S::FREQUENCY, S::SPATIAL, S::SPATIAL, correlationMethod ));
   DIP_ASSERT( cross.DataType().IsReal());
   SubpixelLocationResult locB;
   DIP_STACK_TRACE_THIS( locB = SubpixelLocation( cross, MaximumPixel( cross, {} )));
   locB.coordinates -= cross.GetCenter();
   //std::cout << "B: coordinates = " << locB.coordinates << ", value = " << locB.value << '\n';
   fIn1.Strip();
   cross.Strip();

   if( locA.value >= locB.value ) {
      // Pick in2a
      in2b.Strip();
      matrix[ 4 ] = locA.coordinates[ 0 ];
      matrix[ 5 ] = locA.coordinates[ 1 ];
      DIP_STACK_TRACE_THIS( Shift( in2a, out, locA.coordinates, interpolationMethod, { S::ADD_ZEROS } ));
   } else {
      // Pick in2b
      in2a.Strip();
      matrix[ 4 ] = locB.coordinates[ 0 ];
      matrix[ 5 ] = locB.coordinates[ 1 ];
      DIP_STACK_TRACE_THIS( Shift( in2b, out, locB.coordinates, interpolationMethod, { S::ADD_ZEROS } ));
      matrix[ 0 ] = -matrix[ 0 ]; // our rotation is theta + pi, which leads to all matrix elements being inverted
      matrix[ 1 ] = -matrix[ 1 ];
      matrix[ 2 ] = -matrix[ 2 ];
      matrix[ 3 ] = -matrix[ 3 ];
   }
   return matrix;
}

} // namespace dip
