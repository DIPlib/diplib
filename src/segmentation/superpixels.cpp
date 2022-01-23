/*
 * (c)2019-2021, Cris Luengo.
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
#include "diplib/segmentation.h"
#include "diplib/generation.h"
#include "diplib/nonlinear.h"
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/morphology.h"

namespace dip {

void Superpixels(
      Image const& in,
      Image& out,
      Random& random,
      dfloat density,
      dfloat compactness,
      String const& method,
      StringSet const& flags
) {
   // Parse input
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   bool rectangular = true;
   bool noGaps = false;
   for( auto const& f : flags ) {
      if( f == S::RECTANGULAR ) {
         rectangular = true;
      } else if( f == S::HEXAGONAL ) {
         rectangular = false;
      } else if( f == S::NOGAPS ) {
         noGaps = true;
      } else {
         DIP_THROW_INVALID_FLAG( f );
      }
   }
   if(( nDims > 3 ) || ( nDims < 2 )) {
      rectangular = true;
   }
   String shape = rectangular ? S::RECTANGULAR : ( nDims == 2 ? S::HEXAGONAL : S::FCC );
   // Compute gradient magnitude
   Image gradmag;
   DIP_STACK_TRACE_THIS( GradientMagnitude( in, gradmag ));
   if( !gradmag.IsScalar() ) {
      DIP_STACK_TRACE_THIS( Norm( gradmag, gradmag ));
   }
   // Place seeds
   Image seeds;
   DIP_STACK_TRACE_THIS( CreateRandomGrid( seeds, in.Sizes(), random, density, shape, S::TRANSLATION ));
   MoveToLocalMinimum( seeds, gradmag, seeds );
   // Do the thing
   if( method == S::CW ) {
      StringSet cwFlags;
      cwFlags.emplace( S::LABELS );
      if( noGaps ) {
         cwFlags.emplace( S::NOGAPS );
      }
      DIP_STACK_TRACE_THIS( CompactWatershed( gradmag, seeds, {}, out, 1, compactness, cwFlags ));
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
}

} // namespace dip
