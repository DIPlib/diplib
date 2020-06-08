/*
 * DIPlib 3.0
 * This file defines support functionality used by dip::Watershed and similar functions.
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

#include "watershed_support.h"
#include "diplib/overload.h"

namespace dip {

std::vector< dip::sint > CreateOffsetsArray( UnsignedArray const& sizes, IntegerArray const& strides ) {
   dip::uint ndims = sizes.size();
   dip::uint noInd = sizes[ 0 ] - 2;
   for( dip::uint ii = 1; ii < ndims; ++ii ) {
      noInd *= sizes[ ii ] - 2;
   }
   std::vector< dip::sint > offsets;
   offsets.reserve( noInd );
   UnsignedArray coords( ndims, 1 );
   for( ;; ) {
      dip::sint offset = 0;
      dip::uint ii;
      for( ii = 0; ii < ndims; ++ii ) {
         offset += static_cast< dip::sint >( coords[ ii ] ) * strides[ ii ];
      }
      for( ii = 1; ii < sizes[ 0 ] - 1; ++ii ) {
         offsets.push_back( offset );
         offset += strides[ 0 ];
      }
      for( ii = 1; ii < ndims; ++ii ) {
         ++( coords[ ii ] );
         if( coords[ ii ] < sizes[ ii ] - 1 ) {
            break;
         }
         coords[ ii ] = 1;
      }
      if( ii == ndims ) {
         break;
      }
   }
   return offsets;
}

std::vector< dip::sint > CreateOffsetsArray( Image const& maskim, IntegerArray const& strides ) {
   DIP_ASSERT( maskim.DataType() == DT_BIN );
   UnsignedArray const& sizes = maskim.Sizes();
   dip::uint ndims = sizes.size();
   DIP_ASSERT( strides.size() == ndims );
   std::vector< dip::sint > offsets;
   IntegerArray const& maskstrides = maskim.Strides();
   bin* mask = ( bin* )maskim.Origin();
   UnsignedArray coords( ndims, 1 );
   for( ;; ) {
      dip::sint offset = 0;
      dip::sint moffset = 0;
      dip::uint ii;
      for( ii = 0; ii < ndims; ++ii ) {
         offset += static_cast< dip::sint >( coords[ ii ] ) * strides[ ii ];
         moffset += static_cast< dip::sint >( coords[ ii ] ) * maskstrides[ ii ];
      }
      for( ii = 1; ii < sizes[ 0 ] - 1; ++ii ) {
         if( mask[ moffset ] ) {
            offsets.push_back( offset );
         }
         offset += strides[ 0 ];
         moffset += maskstrides[ 0 ];
      }
      for( ii = 1; ii < ndims; ++ii ) {
         ++( coords[ ii ] );
         if( coords[ ii ] < sizes[ ii ] - 1 ) {
            break;
         }
         coords[ ii ] = 1;
      }
      if( ii == ndims ) {
         break;
      }
   }
   return offsets;
}

namespace {

template< typename TPI >
void SortOffsetsInternal( void const* ptr, std::vector< dip::sint >& offsets, bool lowFirst ) {
   TPI const* data = static_cast< TPI const* >( ptr );
   if( lowFirst ) {
      std::sort( offsets.begin(), offsets.end(), [ & ]( dip::sint const& a, dip::sint const& b ) {
         return data[ a ] < data[ b ];
      } );
   } else {
      std::sort( offsets.begin(), offsets.end(), [ & ]( dip::sint const& a, dip::sint const& b ) {
         return data[ a ] > data[ b ];
      } );
   }
}

} // namespace

void SortOffsets( Image const& img, std::vector< dip::sint >& offsets, bool lowFirst ) {
   DataType ovlType = img.DataType();
   if( ovlType.IsBinary() ) {
      ovlType = DT_UINT8;
   }
   DIP_OVL_CALL_REAL( SortOffsetsInternal, ( img.Origin(), offsets, lowFirst ), ovlType );
}

} // namespace dip
