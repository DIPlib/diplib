/*
 * DIPlib 3.0
 * This file contains the definition for the GetObjectLabels function.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <set>

#include "diplib.h"
#include "diplib/regions.h"
#include "diplib/framework.h"
#include "diplib/overload.h"


namespace dip {

using LabelSet = std::set< dip::uint >;

template< typename TPI >
static void dip__GetLabels(
      std::vector< Framework::ScanBuffer > const& inBuffer,
      std::vector< Framework::ScanBuffer >&,
      dip::uint bufferLength,
      dip::uint,
      UnsignedArray const&,
      void const*,
      void* functionVariables
) {
   LabelSet* objectIDs = static_cast< LabelSet* >( functionVariables );
   TPI* data = static_cast< TPI* >( inBuffer[ 0 ].buffer );
   dip::sint stride = inBuffer[ 0 ].stride;
   if( inBuffer.size() > 1 ) {
      bin* mask = static_cast< bin* >( inBuffer[ 1 ].buffer );
      dip::sint mask_stride = inBuffer[ 1 ].stride;
      dip::uint prevID = 0;
      bool setPrevID = false;
      for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
         if( *mask ) {
            if( !setPrevID || ( *data != prevID )) {
               prevID = *data;
               setPrevID = true;
               objectIDs->insert( prevID );
            }
         }
         data += stride;
         mask += mask_stride;
      }
   } else {
      dip::uint prevID = *data + 1; // something that's different from the first pixel value
      for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
         if( *data != prevID ) {
            prevID = *data;
            objectIDs->insert( prevID );
         }
      }
      data += stride;
   }
}

UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      bool nullIsObject
) {
   // Check input
   DIP_THROW_IF( label.TensorElements() != 1, E::NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   if( mask.IsForged() ) {
      DIP_THROW_IF( mask.TensorElements() != 1, E::NOT_SCALAR );
      DIP_THROW_IF( !mask.DataType().IsBinary(), E::MASK_NOT_BINARY );
      DIP_TRY
         mask.CompareProperties( label, Option::CmpProps_Sizes );
      DIP_CATCH
   }

   // Create arrays for Scan framework
   ImageConstRefArray inar{ label };
   DataTypeArray inBufT{ label.DataType() }; // All but guarantees that data won't be copied.
   if( mask.IsForged() ) {
      inar.emplace_back( mask );
      inBufT.emplace_back( mask.DataType() ); // All but guarantees that data won't be copied.
   }
   ImageRefArray outar{};
   DataTypeArray outBufT{};
   DataTypeArray outImT{};
   UnsignedArray nElem{};
   std::vector< LabelSet > functionVariables( 1 ); // we only use one, as this is not multi-threaded.

   // Get pointer to overloaded scan function
   Framework::ScanFilter lineFilter;
   DIP_OVL_ASSIGN_UINT( lineFilter, dip__GetLabels, label.DataType() );

   // Do the scan
   Framework::Scan(
         inar, outar, inBufT, outBufT, outImT, nElem,
         lineFilter,
         nullptr,
         Framework::CastToVoidpVector( functionVariables ),
         Framework::Scan_NoMultiThreading
   );

   // Copy the labels to output array
   LabelSet& objectIDs = functionVariables[ 0 ];
   UnsignedArray out;
   for( auto const& id : objectIDs ) {
      out.push_back( id );
   }
   return out;
}

} // namespace dip
