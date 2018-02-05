/*
 * DIPlib 3.0
 * This file contains definitions for distance transforms
 *
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

#include <queue>

#include "diplib.h"
#include "diplib/distance.h"
#include "diplib/statistics.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"

namespace dip {

namespace {

constexpr uint8 BORDER = 1;
constexpr uint8 FINISHED = 2;

struct Qitem {
   dip::sint offset;
   sfloat value;
};

bool ShouldBeOutputLater( Qitem const& a, Qitem const& b ) {
   return a.value > b.value;
}

template< typename TPI >
void dip__GreyWeightedDistanceTransform(
      Image const& im_grey,
      Image& im_gdt,
      Image& im_pdt,
      Image& im_flags,
      NeighborList const& neighborhood,
      IntegerArray const& neighborOffsets,
      CoordinatesComputer const& coordComputer
) {
   // Get data pointers
   TPI const* grey = static_cast< TPI const* >( im_grey.Origin() );
   sfloat* gdt = static_cast< sfloat* >( im_gdt.Origin() );
   sfloat* pdt = im_pdt.IsForged() ? static_cast< sfloat* >( im_pdt.Origin() ) : nullptr;
   uint8* flags = static_cast< uint8* >( im_flags.Origin() );
   UnsignedArray const& sizes = im_grey.Sizes();

   // Create priority queue
   std::priority_queue< Qitem, std::vector< Qitem >, decltype( &ShouldBeOutputLater )  > Q( ShouldBeOutputLater );

   // Put all background pixels that have a foreground neighbor in the queue
   ImageIterator< sfloat > it( im_gdt );
   it.Optimize();
   do {
      dip::sint offset = it.Offset();
      if( gdt[ offset ] == 0 ) {
         // This is a background pixel
         flags[ offset ] |= FINISHED;
         if( flags[ offset ] & BORDER ) {
            // We're in a boundary pixel, not all neighbors will be available
            //UnsignedArray const& coords = it.Coordinates();
            UnsignedArray coords = coordComputer( offset ); // Need to compute these because we called `it.Optimize()`
            auto oit = neighborOffsets.begin();
            for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
               if( nit.IsInImage( coords, sizes )) {
                  if( gdt[ offset + *oit ] != 0 ) {
                     Q.push( { offset, 0 } );
                     flags[ offset ] &= static_cast< uint8 >( ~FINISHED ); // reset FINISHED flag, so it'll be processed
                     break;
                  }
               }
            }
         } else {
            // No need to test for out-of-bounds reads
            for( auto o : neighborOffsets ) {
               if( gdt[ offset + o ] != 0 ) {
                  Q.push( { offset, 0 } );
                  flags[ offset ] &= static_cast< uint8 >( ~FINISHED ); // reset FINISHED flag, so it'll be processed
                  break;
               }
            }
         }
      } else {
         gdt[ offset ] = std::numeric_limits< sfloat >::max();
      }
   } while( ++it );

   // Compute distances
   while( !Q.empty() ) {
      // Get next pixel to expand distances from
      dip::sint offset = Q.top().offset;
      Q.pop();
      if( flags[ offset ] & FINISHED ) {
         continue;
      }
      flags[ offset ] |= FINISHED;
      sfloat distance = gdt[ offset ];
      bool isBorder = flags[ offset ] & BORDER;
      UnsignedArray coords;
      if( isBorder ) {
         coords = coordComputer( offset );
      }
      // Check all neighbors
      auto oit = neighborOffsets.begin();
      for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
         if( !isBorder || nit.IsInImage( coords, sizes )) {
            dip::sint neigh = offset + *oit;
            if( !( flags[ neigh ] & FINISHED )) {
               sfloat value = distance + static_cast< sfloat >( *nit ) * static_cast< sfloat >( grey[ neigh ] );
               if( value < gdt[ neigh ] ) {
                  gdt[ neigh ] = value;
                  if( pdt ) {
                     pdt[ neigh ] = pdt[ offset ] + static_cast< sfloat >( *nit );
                  }
                  Q.push( { neigh, value } );
               }
            }
         }
      }
   }
}

} // namespace

void GreyWeightedDistanceTransform(
      Image const& c_grey,
      Image const& bin,
      Image& out,
      Metric metric,
      String const& outputMode
) {
   DIP_THROW_IF( !bin.IsForged() || !c_grey.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !bin.IsScalar() || !c_grey.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !bin.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint dims = bin.Dimensionality();
   DIP_THROW_IF( dims < 1 , E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( bin.Sizes() != c_grey.Sizes(), E::SIZES_DONT_MATCH );

   DIP_THROW_IF( c_grey.HasSingletonDimension(), "Images with singleton dimensions not supported. Use Squeeze." );

   // We can only support non-negative weights --
   DIP_THROW_IF( Minimum( c_grey ).As< dfloat >() < 0.0, "Minimum input value < 0.0" );

   // What will we output?
   bool outputGDT = false;
   bool outputDistance = false;
   if( outputMode == S::GDT ) {
      outputGDT = true;
   } else if( outputMode == S::EUCLIDEAN ) {
      outputDistance = true;
   } else if( outputMode == S::BOTH ) {
      outputGDT = true;
      outputDistance = true;
   } else {
      DIP_THROW( E::INVALID_FLAG );
   }

   // Find pixel size to keep
   PixelSize pixelSize = c_grey.PixelSize();
   if( !pixelSize.IsDefined() ) {
      pixelSize = bin.PixelSize(); // Let's try this one instead...
   }
   if( !metric.HasPixelSize() ) {
      metric.SetPixelSize( pixelSize );
   }

   // We must have contiguous data if we want to create another image with the same strides as `grey`
   Image grey = c_grey.QuickCopy();
   grey.ForceContiguousData(); // this also ensures no singleton-expanded dimensions

   // Create temporary images
   Image gdt;
   gdt.SetStrides( grey.Strides() );
   gdt.SetSizes( grey.Sizes() );
   gdt.SetDataType( DT_SFLOAT );
   gdt.Forge();
   DIP_ASSERT( gdt.Strides() == grey.Strides() );
   gdt.Copy( bin );

   Image distance;
   if( outputDistance ) {
      distance.SetStrides( grey.Strides() );
      distance.SetSizes( grey.Sizes() );
      distance.SetDataType( DT_SFLOAT );
      distance.Forge();
      DIP_ASSERT( distance.Strides() == grey.Strides() );
      distance.Fill( 0 );
   }

   Image flags;
   flags.SetStrides( grey.Strides() );
   flags.SetSizes( grey.Sizes() );
   flags.SetDataType( DT_UINT8 );
   flags.Forge();
   DIP_ASSERT( flags.Strides() == grey.Strides() );
   flags.Fill( 0 );

   // Get neighborhoods and metrics
   NeighborList neighborhood{ metric, dims };
   IntegerArray offsets = neighborhood.ComputeOffsets( grey.Strides() );

   // Initialize `flags` image
   UnsignedArray border = neighborhood.Border();
   SetBorder( flags, { BORDER }, border );

   // Create coordinate computer
   CoordinatesComputer coordComputer = grey.OffsetToCoordinatesComputer();

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__GreyWeightedDistanceTransform, ( grey, gdt, distance, flags, neighborhood, offsets,
                                                               coordComputer ), grey.DataType() );

   // Copy to output image
   if( outputGDT && outputDistance ) {
      out.ReForge( gdt.Sizes(), 2, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
      out[ 0 ].Copy( gdt );
      out[ 1 ].Copy( distance );
   } else if( outputDistance ) {
      out = distance;
   } else {
      out = gdt;
   }
   out.SetPixelSize( pixelSize );
}

} // namespace dip
