/*
 * DIPlib 3.0
 * This file contains the definition for the GetObjectLabels function.
 *
 * (c)2016-2017, Cris Luengo.
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

#include <set>

#include "diplib.h"
#include "diplib/regions.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

using LabelSet = std::set< dip::uint >;

template< typename TPI >
class dip__GetLabels : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI* data = static_cast< TPI* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         if( params.inBuffer.size() > 1 ) {
            bin* mask = static_cast< bin* >( params.inBuffer[ 1 ].buffer );
            dip::sint mask_stride = params.inBuffer[ 1 ].stride;
            dip::uint prevID = 0;
            bool setPrevID = false;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( * mask ) {
                  if( !setPrevID || ( * data != prevID ) ) {
                     prevID = * data;
                     setPrevID = true;
                     objectIDs.insert( prevID );
                  }
               }
               data += stride;
               mask += mask_stride;
            }
         } else {
            dip::uint prevID = * data + 1; // something that's different from the first pixel value
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( * data != prevID ) {
                  prevID = * data;
                  objectIDs.insert( prevID );
               }
               data += stride;
            }
         }
      }
      dip__GetLabels( LabelSet& objectIDs ) : objectIDs( objectIDs ) {}
   private:
      LabelSet& objectIDs;
};

UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      bool nullIsObject
) {
   // Check input
   DIP_THROW_IF( label.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   if( mask.IsForged() ) {
      DIP_THROW_IF( mask.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
      DIP_THROW_IF( !mask.DataType().IsBinary(), E::MASK_NOT_BINARY );
      DIP_START_STACK_TRACE
         mask.CompareProperties( label, Option::CmpProps_Sizes );
      DIP_END_STACK_TRACE
   }

   // Create arrays for Scan framework
   ImageConstRefArray inar{ label };
   DataTypeArray inBufT{ label.DataType() }; // All but guarantees that data won't be copied.
   if( mask.IsForged() ) {
      inar.emplace_back( mask );
      inBufT.emplace_back( mask.DataType() ); // All but guarantees that data won't be copied.
   }
   ImageRefArray outar{};

   LabelSet objectIDs; // output

   // Get pointer to overloaded scan function
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_UINT( scanLineFilter, dip__GetLabels, ( objectIDs ), label.DataType() );

   // Do the scan
   Framework::Scan( inar, outar, inBufT, {}, {}, {}, *scanLineFilter, Framework::Scan_NoMultiThreading );

   // Copy the labels to output array
   UnsignedArray out;
   for( auto const& id : objectIDs ) {
      if(( id != 0 ) || nullIsObject ) {
         out.push_back( id );
      }
   }
   return out;
}

} // namespace dip
