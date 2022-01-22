/*
 * (c)2018, Cris Luengo
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
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/generation.h"

namespace dip {

void RieszTransform(
      Image const& in,
      Image& out,
      String const& inRepresentation,
      String const& outRepresentation,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   // Which dimensions to process?
   DIP_STACK_TRACE_THIS( ArrayUseParameter( process, in.Dimensionality(), true ));
   bool inIsSpatial;
   DIP_STACK_TRACE_THIS( inIsSpatial = BooleanFromString( inRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool outIsSpatial;
   DIP_STACK_TRACE_THIS( outIsSpatial = BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY ));
   dip::uint tElems = process.count();
   // Compute Fourier transform of `in`, before reforging `out` in case `&in==&out`.
   bool isReal = false;
   Image FourierIn;
   if( inIsSpatial ) {
      isReal = !in.DataType().IsComplex();
      FourierIn = FourierTransform( in );
   } else {
      FourierIn = in.QuickCopy();
      if( in.Aliases( out )) {
         out.Strip(); // We cannot work in-place.
      }
   }
   // Reforge filtered to have `tElems` tensor elements and complex values.
   Image tmp;
   Image& filtered = outIsSpatial ? tmp : out; // Write directly in out if we won't do inverse Fourier transform
   UnsignedArray sizes = in.Sizes();
   DIP_STACK_TRACE_THIS( filtered.ReForge( sizes, tElems, DataType::SuggestComplex( in.DataType() ), Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( filtered.Real().Fill( 0 )); // Will throw if the reforge above failed to produce a complex type
   // Fill imaginary part of `out` with coordinates (this is x_j in the equation).
   Image coord = filtered.Imaginary();
   dip::uint jj = 0;
   for( dip::uint ii = 0; ii < process.size(); ++ii ) {
      if( process[ ii ] ) {
         Image x = coord[ jj ];
         FillRamp( x, ii );
         ++jj;
      }
   }
   // Compute -i x_j / |x|
   Image norm = Norm( coord );
   norm *= -1;
   sizes /= 2;
   norm.At( sizes ) = 1; // Avoid division by zero by setting the center pixel to 1 (the origin has norm 0)
   filtered /= norm;
   // Compute -i x_j / |x| F(f)
   filtered *= FourierIn;
   // Compute inverse Fourier transform
   if( outIsSpatial ) {
      StringSet options = { S::INVERSE };
      if( isReal ) {
         options.insert( S::REAL );
      }
      DIP_STACK_TRACE_THIS( FourierTransform( filtered, out, options ));
   }
}

} // namespace dip
