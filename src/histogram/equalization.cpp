/*
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
 *                                  (c)2013, Patrik Malm & Cris Luengo.
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

#include "diplib/histogram.h"

#include <utility>

#include "diplib.h"
#include "diplib/lookup_table.h"

namespace dip {

LookupTable EqualizationLookupTable(
      Histogram const& histogram
) {
   DIP_THROW_IF( histogram.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image lutImage = histogram.Copy().Cumulative().GetImage();
   lutImage.Convert( DT_DFLOAT );
   dip::uint nBins = lutImage.Size( 0 );
   dfloat count = lutImage.At< dfloat >( nBins - 1 );
   lutImage *= static_cast< dfloat >( nBins - 1 ) / count;
   return LookupTable( lutImage, histogram.BinCenters() );
}

LookupTable MatchingLookupTable(
      Histogram const& histogram,
      Histogram const& example
) {
   DIP_THROW_IF( histogram.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( example.Dimensionality() != 1, E::DIMENSIONALITY_NOT_SUPPORTED );

   // First histogram
   Image lutImage1 = histogram.Copy().Cumulative().GetImage();
   lutImage1.Convert( DT_DFLOAT );
   dip::uint nBins1 = lutImage1.Size( 0 );
   dfloat count1 = lutImage1.At< dfloat >( nBins1 - 1 );
   lutImage1 *= static_cast< dfloat >( nBins1 - 1 ) / count1;

   // Second histogram
   Image lutImage2 = example.Copy().Cumulative().GetImage();
   lutImage2.Convert( DT_DFLOAT );
   dip::uint nBins2 = lutImage2.Size( 0 );
   dfloat count2 = lutImage2.At< dfloat >( nBins2 - 1 );
   lutImage2 *= static_cast< dfloat >( nBins1 - 1 ) / count2; // using nBins1 to match it up with lutImage1
   // TODO: remove items with a zero value?

   // Create a LUT with 2nd histogram bin centers as value, and lutImage2 as indices
   FloatArray lutBins2 = example.BinCenters();
   DIP_ASSERT( lutBins2.size() == nBins2 );
   dip::sint stride = lutImage2.Stride( 0 );
   dfloat* ptr = static_cast< dfloat* >( lutImage2.Origin() );
   for( dip::uint ii = 0; ii < nBins2; ++ii ) {
      std::swap( lutBins2[ ii ], *ptr );
      ptr += stride;
   }
   LookupTable lut( lutImage2, lutBins2 );
   lut.Apply( lutImage1, lutImage1 );

   // Create a LUT with the modified 1st histogram
   return LookupTable( lutImage1, histogram.BinCenters() );
}

} // namespace dip
