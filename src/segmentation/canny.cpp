/*
 * DIPlib 3.0
 * This file contains the definition of the dip::Canny function.
 *
 * (c)2017-2018, Cris Luengo.
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
#include "diplib/segmentation.h"
#include "diplib/linear.h"
#include "diplib/nonlinear.h"
#include "diplib/statistics.h"
#include "diplib/binary.h"

namespace dip {

void Canny(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      dfloat lower,
      dfloat upper,
      String const& selection
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_START_STACK_TRACE
      Image gradient = Gradient( in, sigmas, S::BEST );
      NonMaximumSuppression( {}, gradient, {}, out, S::INTERPOLATE ); // use interpolation in 2D, for higher dims it's always "round"
      dfloat t1 = upper;
      dfloat t2 = lower;
      if( selection == S::ALL ) {
         t1 = Percentile( out, {}, upper * 100 ).As< dfloat >();
         if( t1 == 0 ) {
            t1 = 1e-6; // Some very small value. Cannot use `std::numeric_limits< dfloat >::min()`, because can't multiply that with `lower`.
         }
         t2 = lower * t1;
      } else if( selection == "nonzero" ) {
         Image tmp = out > 0;
         t1 = Percentile( out, tmp, upper * 100 ).As< dfloat >();
         t2 = lower * t1;
      } else if( selection != "absolute" ) {
         DIP_THROW_INVALID_FLAG( selection );
      }
      HysteresisThreshold( out, out, t2, t1 );
      if(( out.Dimensionality() == 2 ) || ( out.Dimensionality() == 3 )) {
         EuclideanSkeleton( out, out );
      }
   DIP_END_STACK_TRACE
}

} // namespace dip
