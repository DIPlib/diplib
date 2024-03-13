/*
 * (c)2017-2018, Cris Luengo.
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

#include <algorithm>

#include "diplib.h"
#include "diplib/file_io.h"
#include "file_io_support.h"

namespace dip {

RangeArray ConvertRoiSpec(
      UnsignedArray const& origin,
      UnsignedArray const& sizes,
      UnsignedArray const& spacing
) {
   dip::uint n = origin.size();
   n = std::max( n, sizes.size() );
   n = std::max( n, spacing.size() );
   if( n > 1 ) {
      DIP_THROW_IF(( origin.size() > 1 ) && ( origin.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( sizes.size() > 1 ) && ( sizes.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
      DIP_THROW_IF(( spacing.size() > 1 ) && ( spacing.size() != n ), E::ARRAY_SIZES_DONT_MATCH );
   }
   RangeArray roi( n );
   if( origin.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ 0 ] );
      }
   } else if( origin.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].start = static_cast< dip::sint >( origin[ ii ] );
      }
   }
   if( sizes.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ 0 ] ) - 1;
      }
   } else if( sizes.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].stop = roi[ ii ].start + static_cast< dip::sint >( sizes[ ii ] ) - 1;
      }
   }
   if( spacing.size() == 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ 0 ];
      }
   } else if( spacing.size() > 1 ) {
      for( dip::uint ii = 0; ii < n; ++ii ) {
         roi[ ii ].step = spacing[ ii ];
      }
   }
   return roi;
}

RoiSpec CheckAndConvertRoi(
      RangeArray const& roi,
      Range const& channels,
      FileInformation const& fileInformation,
      dip::uint nDims
) {
   RoiSpec roiSpec;
   roiSpec.sizes.resize( nDims, 0 );
   roiSpec.mirror.resize( nDims, false );
   roiSpec.roi = roi;
   roiSpec.channels = channels;
   ArrayUseParameter( roiSpec.roi, nDims, Range{} );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      roiSpec.roi[ ii ].Fix( fileInformation.sizes[ ii ] );
      if( roiSpec.roi[ ii ].start > roiSpec.roi[ ii ].stop ) {
         std::swap( roiSpec.roi[ ii ].start, roiSpec.roi[ ii ].stop );
         roiSpec.mirror[ ii ] = true;
      }
      roiSpec.sizes[ ii ] = roiSpec.roi[ ii ].Size();
      if( roiSpec.sizes[ ii ] != fileInformation.sizes[ ii ] ) {
         roiSpec.isFullImage = false;
      }
   }
   roiSpec.channels.Fix( fileInformation.tensorElements );
   if( roiSpec.channels.start > roiSpec.channels.stop ) {
      std::swap( roiSpec.channels.start, roiSpec.channels.stop );
      // We don't read the tensor dimension in reverse order
   }
   roiSpec.tensorElements = roiSpec.channels.Size();
   if( roiSpec.tensorElements != fileInformation.tensorElements ) {
      roiSpec.isAllChannels = false;
   }
   return roiSpec;
}

} // namespace
