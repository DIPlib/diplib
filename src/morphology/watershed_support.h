/*
 * DIPlib 3.0
 * This file declares support functionality used by dip::Watershed and similar functions.
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_WATERSHED_SUPPORT_H
#define DIP_WATERSHED_SUPPORT_H

#include "diplib.h"

namespace dip {

// Creates a list of offsets into an image with the given sizes and strides. Pixels at the image
// boundary are excluded.
DIP_NO_EXPORT std::vector< dip::sint > CreateOffsetsArray( UnsignedArray const& sizes, IntegerArray const& strides );

// Creates a list of offsets into an image with the size of `mask` and the given strides. Only those
// pixels set in `mask` are indexed. Pixels at the image boundary are excluded.
DIP_NO_EXPORT std::vector< dip::sint > CreateOffsetsArray( Image const& mask, IntegerArray const& strides );

// Sorts the list of offsets by the grey value they index.
DIP_NO_EXPORT void SortOffsets( Image const& img, std::vector< dip::sint >& offsets, bool lowFirst );

// This class manages a list of neighbor labels.
// There are never more than N neighbors added at a time, N being defined
// by the dimensionality and the connectivity. However, typically there are
// only one or two labels added. Therefore, no effort has been put into making
// this class clever. We could keep a sorted list, but the sorting might cost
// more effort than it would save in checking if a label is present (TODO: would it?)
class DIP_NO_EXPORT NeighborLabels{
   public:
      void Reset() { labels.clear(); }
      void Push( LabelType value ) {
         if(( value != 0 ) && !Contains( value )) {
            labels.push_back( value );
         }
      }
      bool Contains( LabelType value ) const {
         for( auto l : labels ) {
            if( l == value ) {
               return true;
            }
         }
         return false;
      }
      dip::uint Size() const {
         return labels.size();
      }
      LabelType Label( dip::uint index ) const {
         return labels[ index ];
      }
   private:
      std::vector< LabelType > labels;
};

} // namespace dip

#endif // DIP_WATERSHED_SUPPORT_H
