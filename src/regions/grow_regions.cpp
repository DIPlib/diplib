/*
 * DIPlib 3.0
 * This file contains region growing algorithms.
 *
 * (c)2018, Cris Luengo.
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

#include <queue>

#include "diplib.h"
#include "diplib/regions.h"
#include "diplib/distance.h"
#include "diplib/morphology.h"
#include "diplib/border.h"
#include "diplib/overload.h"
#include "../binary/binary_support.h"

namespace dip {

namespace {

constexpr uint8 MASK = 1; // must be 1
constexpr uint8 BORDER = 2;

using FifoQueue = std::queue< dip::sint >;

template< typename TPI >
void GrowRegionsInternal(
      Image& im_label,
      Image& im_flags,
      dip::uint iterations,
      NeighborList const& neighborhood0,
      IntegerArray const& offsets0,
      NeighborList const& neighborhood1,
      IntegerArray const& offsets1,
      CoordinatesComputer const& coordComputer
) {
   TPI* label = static_cast< TPI* >( im_label.Origin() );
   uint8* flags = static_cast< uint8* >( im_flags.Origin() );
   UnsignedArray const& sizes = im_label.Sizes();

   // The queue
   FifoQueue Q;

   // Put all background pixels that have a foreground neighbor in the queue
   DIP_START_STACK_TRACE
   ImageIterator< TPI > it( im_label );
   it.OptimizeAndFlatten();
   do {
      dip::sint offset = it.Offset();
      if(( flags[ offset ] & MASK ) && label[ offset ] ) {
         // This is a foreground pixel within the mask
         if( flags[ offset ] & BORDER ) {
            // We're in a boundary pixel, not all neighbors will be available
            //UnsignedArray const& coords = it.Coordinates();
            UnsignedArray coords = coordComputer( offset ); // Need to compute these because we called `it.Optimize()`
            auto oit = offsets0.begin();
            for( auto nit = neighborhood0.begin(); nit != neighborhood0.end(); ++nit, ++oit ) {
               if( nit.IsInImage( coords, sizes )) {
                  if( label[ offset + *oit ] == 0 ) {
                     Q.push( offset );
                     break;
                  }
               }
            }
         } else {
            // No need to test for out-of-bounds reads
            for( auto o : offsets0 ) {
               if( label[ offset + o ] == 0 ) {
                  Q.push( offset );
                  break;
               }
            }
         }

      }
   } while( ++it );
   DIP_END_STACK_TRACE

   // Do `iterations` loops
   for( dip::uint ii = 0; ii < iterations; ++ii ) {

      // Number of elements to process
      dip::uint count = Q.size();
      if( count == 0 ) {
         break; // We're done propagating
      }

      // For alternating connectivity: pick the right neighborhood for this iteration
      NeighborList const& neighborhood = ( ii & 1 ) == 1 ? neighborhood1 : neighborhood0;
      IntegerArray const& offsets = ( ii & 1 ) == 1 ? offsets1 : offsets0;

      // Iterate over all elements currently in the queue
      for( dip::uint jj = 0; jj < count; ++jj ) {

         // Get front pixel from the queue
         dip::sint offset = Q.front();
         Q.pop();
         TPI ll = label[ offset ];
         bool isBorder = flags[ offset ] & BORDER;
         UnsignedArray coords;
         if( isBorder ) {
            coords = coordComputer( offset );
         }

         // Propagate label to all neighbours which are not yet processed
         auto oit = offsets.begin();
         for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
            if( !isBorder || nit.IsInImage( coords, sizes )) {
               dip::sint neigh = offset + *oit;
               if(( flags[ neigh ] & MASK ) && label[ neigh ] == 0 ) {
                  label[ neigh ] = ll;
                  Q.push( neigh );
               }
            }
         }
      }
   }
}

} // namespace

void GrowRegions(
      Image const& c_label,
      Image const& c_mask,
      Image& out,
      dip::sint connectivity,
      dip::uint iterations
) {
   DIP_THROW_IF( !c_label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !c_label.IsScalar(), E::IMAGE_NOT_SCALAR );
   dip::uint nDims = c_label.Dimensionality();
   DIP_THROW_IF( connectivity > static_cast< dip::sint >( nDims ), E::ILLEGAL_CONNECTIVITY );

   // Zero iterations means: continue until propagation is done
   if( iterations == 0 ) {
      iterations = std::numeric_limits< dip::uint >::max();
   }

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( c_label.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( c_label.Sizes() );
      DIP_END_STACK_TRACE
   }

   // Initialize out with label
   if( out.Aliases( mask )) {
      DIP_THROW_IF( &out == &c_label, "The output and label images are the same, but alias the mask image. This is unexpected and weird, and I cannot handle it." );
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   DIP_STACK_TRACE_THIS( out.Copy( c_label ));

   // Create flags image
   Image flags;
   flags.SetStrides( out.Strides() );
   flags.SetSizes( out.Sizes() );
   flags.SetDataType( DT_UINT8 );
   flags.Forge();
   DIP_ASSERT( flags.Strides() == out.Strides() );

   // Initialize flags image with mask, if it's given
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( flags.Copy( mask )); // Sets the MASK bit
   } else {
      DIP_STACK_TRACE_THIS( flags.Fill( MASK )); // Sets the MASK bit everywhere
   }

   // Set the BORDER flag
   DIP_STACK_TRACE_THIS(
   detail::ProcessBorders< uint8 >( flags, []( uint8* ptr, dip::sint ) { static_cast< uint8& >( *ptr ) |= BORDER; } );
   );

   // Create arrays with offsets to neighbours for even iterations
   dip::uint iterConnectivity0;
   DIP_STACK_TRACE_THIS( iterConnectivity0 = GetAbsBinaryConnectivity( nDims, connectivity, 0 ));
   NeighborList neighborhood0( { Metric::TypeCode::CONNECTED, iterConnectivity0 }, nDims );
   IntegerArray offsets0 = neighborhood0.ComputeOffsets( out.Strides() );

   // Create arrays with offsets to neighbours for odd iterations
   dip::uint iterConnectivity1 = GetAbsBinaryConnectivity( nDims, connectivity, 1 ); // won't throw
   NeighborList neighborhood1( { Metric::TypeCode::CONNECTED, iterConnectivity1 }, nDims );
   IntegerArray offsets1 = neighborhood1.ComputeOffsets( out.Strides() );

   // Create coordinate computer
   CoordinatesComputer coordComputer = out.OffsetToCoordinatesComputer();

   // Do the data-type dependent part
   DIP_OVL_CALL_UINT( GrowRegionsInternal, ( out, flags, iterations,
                                             neighborhood0, offsets0, neighborhood1, offsets1,
                                             coordComputer ), out.DataType() );
}


void GrowRegionsWeighted(
      Image const& label,
      Image const& grey,
      Image const& mask,
      Image& out,
      Metric const& metric
) {
   // Compute grey-weighted distance transform
   Image binary = label == 0;
   Image distance;
   DIP_STACK_TRACE_THIS( GreyWeightedDistanceTransform( grey, binary, mask, distance, metric, S::CHAMFER )); // TODO: Use FASTMATCHING here!
   binary.Strip();

   // Grow regions
   DIP_STACK_TRACE_THIS( SeededWatershed( distance, label, mask, out, 1, -1, 0, { S::NOGAPS } )); // maxDepth = -1: disables region merging
}

} // namespace dip
