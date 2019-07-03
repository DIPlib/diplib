/*
 * DIPlib 3.0
 * This file contains the definition of ApplyColorMap and similar functions
 *
 * (c)2019, Cris Luengo.
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
#include "diplib/display.h"
#include "diplib/morphology.h"
#include "diplib/geometry.h"

namespace dip {

void MarkLabelEdges(
      Image const& labels,
      Image& out,
      dip::uint factor
) {
   DIP_THROW_IF( !labels.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !labels.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !labels.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );

   if(( out.DataType() != labels.DataType() ) && ( !out.IsProtected() )) {
      out.Strip(); // Avoid data type conversion unless image is protected
   }
   if( factor > 1 ) {
      DIP_STACK_TRACE_THIS( Resampling( labels, out, { static_cast< dfloat >( factor ) }, { 0.0 }, S::NEAREST ));
   } else {
      out.Copy( labels );
   }

   // Easy solution. This can be sped up by writing a specific algorithm, but it's not worth it.
   StructuringElement se{ 3, S::DIAMOND };
   Image mask = Dilation( out, se ) != Erosion( out, se );
   out.At( mask ) = 0;
}

} // namespace dip
