/*
 * (c)2019, Cris Luengo
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

#include "diplib/transform.h"

#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/linear.h"
#include "diplib/generic_iterators.h"

namespace dip {

void StationaryWaveletTransform(
      Image const& in,
      Image& out,
      dip::uint nLevels,
      StringArray const& boundaryCondition,
      BooleanArray const& process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   if( nLevels == 0 ) {
      out = in;
      return;
   }
   dip::uint nDims = in.Dimensionality();
   UnsignedArray sizes = in.Sizes();
   sizes.push_back( nLevels + 1 );
   Image input = in.Copy();
   DataType dt = DataType::SuggestSigned( in.DataType() );
   out.ReForge( sizes, in.TensorElements(), dt ); // Don't use `in` any more, it could have been reforged here.
   out.CopyNonDataProperties( input );
   out.SetPixelSize( nDims, {} );
   // Lowest scale filter
   OneDimensionalFilterArray filter = {{{ 1.0 / 16.0, 1.0 / 4.0, 3.0 / 8.0 }, -1, S::EVEN }};
   // Iterate over scales, starting with lowest scale
   ImageSliceIterator slice( out, nDims );
   Image smoothed; // temporary storage
   for( dip::uint ii = 0; ii < nLevels; ++ii, ++slice ) {
      SeparableConvolution( input, smoothed, filter, boundaryCondition, process );
      Subtract( input, smoothed, *slice, slice->DataType() );
      input.swap( smoothed );
      // Increase scale by inserting zeros into the filter
      std::vector< dfloat > newFilter( filter[ 0 ].filter.size() * 2 - 1, 0.0 );
      for( dip::uint jj = 0; jj < filter[ 0 ].filter.size(); ++jj ) {
         newFilter[ 2 * jj ] = filter[ 0 ].filter[ jj ];
      }
      filter[ 0 ].filter = std::move( newFilter );
   }
   slice->Copy( input );
}

} // namespace dip
