/*
 * (c)2016-2025, Cris Luengo.
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

#ifndef COPY_NON_NAN_H
#define COPY_NON_NAN_H

#include <thread>

#include "diplib.h"

namespace dip {

template< typename TPI >
void CopyNonNaNValues( dip::Image const& image, dip::Image const& mask, std::vector< TPI >& values ) {
   DIP_ASSERT( image.DataType() == DataType( TPI{} ));
   // Also assumed: mask, if forged, is of the same sizes as image, binary, and scalar. This is tested by the JointImageIterator.
   // Also assumed: image is scalar.
   // Also assumed: TPI is not complex.
   using std::isnan;
   dip::uint N = 0;
   if( mask.IsForged() ) {
      N = Count( mask );
   } else {
      N = image.NumberOfPixels();
   }
   values.resize( N );
   if( N == 0 ) {
      return;
   }
   auto outIt = values.begin();
   if( mask.IsForged() ) {
      JointImageIterator< TPI, bin > it( { image, mask } );
      it.OptimizeAndFlatten();
      do {
         if( it.template Sample< 1 >() && !isnan( it.template Sample< 0 >() )) {
            *( outIt++ ) = it.template Sample< 0 >();
         }
      } while( ++it );
   } else {
      ImageIterator< TPI > it( image );
      it.OptimizeAndFlatten();
      do {
         if( !isnan( *it )) {
            *( outIt++ ) = *it;
         }
      } while( ++it );
   }
   N = static_cast< dip::uint >( outIt - values.begin() );
   values.resize( N );
}

}

#endif // COPY_NON_NAN_H
