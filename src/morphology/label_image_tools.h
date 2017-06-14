/*
 * DIPlib 3.0
 * This file declares functionality to work with labeled images.
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

#ifndef DIP_LABEL_IMAGE_TOOLS_H
#define DIP_LABEL_IMAGE_TOOLS_H

#include "diplib.h"

namespace dip {

using LabelType = uint32;
constexpr auto DT_LABEL = DT_UINT32;

// This class manages a list of neighbor labels.
// There are never more than N neighbors added at a time, N being defined
// by the dimensionality and the connectivity. However, typically there are
// only one or two labels added. Therefore, no effort has been put into making
// this class clever. We could keep a sorted list, but the sorting might costs
// more effort than it would save in checking if a label is present (would it?)
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

#endif // DIP_LABEL_IMAGE_TOOLS_H
