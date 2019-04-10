/*
 * DIPlib 3.0
 * This file contains functions for histogram equalization and similar
 *
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

#include "diplib.h"
#include "diplib/mapping.h"
#include "diplib/lookup_table.h"
#include "diplib/histogram.h"

namespace dip {

void HistogramEqualization(
      Image const& in,
      Image& out,
      dip::uint nBins
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   // Get histogram
   Histogram::Configuration configuration( in.DataType() );
   configuration.nBins = nBins;
   configuration.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE;
   Histogram histogram( in, {}, configuration );
   // Get LUT
   auto lut = EqualizationLookupTable( histogram );
   // Find an appropriate output data type
   DataType dt = in.DataType();
   while( !dt.IsInRange( nBins - 1 )) {
      switch( dt ) {
         case DT_UINT8:
            dt = DT_UINT16;
            break;
         case DT_SINT8:
            dt = DT_UINT8;
            break;
         case DT_UINT16:
            dt = DT_UINT32;
            break;
         case DT_SINT16:
            dt = DT_UINT16;
            break;
         case DT_UINT32:
            dt = DT_UINT64;
            break;
         case DT_SINT32:
            dt = DT_UINT32;
            break;
         // case DT_UINT64: will always be in range,
         case DT_SINT64:
            dt = DT_UINT64;
            break;
         default:
            dt = DT_SFLOAT;
            break;
      }
   }
   // Convert the LUT and apply it
   lut.Convert( dt );
   lut.Apply( in, out );
}

void HistogramMatching(
      Image const& in,
      Image& out,
      Histogram const& example
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( example.Dimensionality() != 1, "The Example histogram must be one-dimensional." );
   // Get histogram
   Histogram::Configuration configuration( in.DataType() );
   configuration.nBins = example.Bins();
   configuration.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE;
   Histogram histogram( in, {}, configuration );
   // Get LUT
   auto lut = MatchingLookupTable( histogram, example );
   // Convert the LUT and apply it
   lut.Convert( DT_SFLOAT );
   lut.Apply( in, out );
}

} // namespace dip

