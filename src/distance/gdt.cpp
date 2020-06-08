/*
 * DIPlib 3.0
 * This file contains definitions for distance transforms
 *
 * (c)2017-2019, Cris Luengo.
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
constexpr uint8 MASKED = 4;
//constexpr uint8 INQUEUE = 8; // We don't really need this, since we don't update stuff that is in the queue, but rather push it again.

inline bool IsSet( uint8 value, uint8 bit ) {
   return ( value & bit ) != 0;
}

inline void Set( uint8& value, uint8 bit ) {
   value |= bit;
}

inline void Reset( uint8& value, uint8 bit ) {
   value &= static_cast< uint8 >( ~bit );
}

struct Qitem {
   dip::sint offset;
   sfloat value;
};

bool ShouldBeOutputLater( Qitem const& a, Qitem const& b ) {
   return a.value > b.value;
}

inline dip::uint FindDim( IntegerArray const& coords ) { // coords contains one non-zero element
   dip::uint ii = 0;
   while(( ii < coords.size() ) && ( coords[ ii ] == 0 )) {
      ++ii;
   }
   return ii;
}

using PriorityQueue = std::priority_queue< Qitem, std::vector< Qitem >, decltype( &ShouldBeOutputLater ) >;

PriorityQueue CreateAndInitializePriorityQueue(
      Image const& im_gdt,
      Image const& im_flags,
      NeighborList const& neighborhood,
      IntegerArray const& neighborOffsets,
      CoordinatesComputer const& coordComputer
) {
   sfloat* gdt = static_cast< sfloat* >( im_gdt.Origin() );
   uint8* flags = static_cast< uint8* >( im_flags.Origin() );
   UnsignedArray const& sizes = im_gdt.Sizes();

   // Create priority queue
   PriorityQueue Q( ShouldBeOutputLater );

   // Put all background pixels that have a foreground neighbor in the queue
   ImageIterator< sfloat > it( im_gdt );
   it.OptimizeAndFlatten(); // We can only do this because we prevented negative strides earlier. With negative strides,
   // this would change the origin of the image, making `offset` below point out of the image.
   do {
      dip::sint offset = it.Offset();
      if(( gdt[ offset ] != 0 ) || IsSet( flags[ offset ], MASKED )) {
         continue;
      }
      // This is a background pixel
      Set( flags[ offset ], FINISHED ); // we'll reset this if necessary
      if( IsSet( flags[ offset ], BORDER )) {
         // We're in a boundary pixel, not all neighbors will be available
         //UnsignedArray const& coords = it.Coordinates();
         UnsignedArray coords = coordComputer( offset ); // Need to compute these because we called `it.Optimize()`
         auto oit = neighborOffsets.begin();
         for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
            if( !nit.IsInImage( coords, sizes )) {
               continue;
            }
            dip::sint neigh = offset + *oit;
            if(( gdt[ neigh ] != 0 ) && !IsSet( flags[ neigh ], MASKED )) {
               Q.push( { offset, 0 } );
               Reset( flags[ offset ], FINISHED ); // reset FINISHED flag, so it'll be processed
               //Set( flags[ offset ], INQUEUE );
               break;
            }
         }
      } else {
         // No need to test for out-of-bounds reads
         for( auto o : neighborOffsets ) {
            dip::sint neigh = offset + o;
            if(( gdt[ neigh ] != 0 ) && !IsSet( flags[ neigh ], MASKED )) {
               Q.push( { offset, 0 } );
               Reset( flags[ offset ], FINISHED ); // reset FINISHED flag, so it'll be processed
               //Set( flags[ offset ], INQUEUE );
               break;
            }
         }
      }
   } while( ++it );

   return Q;
}


template< typename TPI >
void FastMarchingAlgorithm(
      Image const& im_weights,
      Image& im_gdt,
      Image& im_flags,
      NeighborList const& neighborhood,
      IntegerArray const& neighborOffsets,
      CoordinatesComputer const& coordComputer,
      FloatArray& distances   // We re-use this, modify it.
) {
   // Get data pointers
   TPI const* weights = im_weights.IsForged() ? static_cast< TPI const* >( im_weights.Origin() ) : nullptr;
   sfloat* gdt = static_cast< sfloat* >( im_gdt.Origin() );
   uint8* flags = static_cast< uint8* >( im_flags.Origin() );
   UnsignedArray const& sizes = im_gdt.Sizes();

   // Pixel sizes
   for( auto& d : distances ) {
      d = 1.0 / ( d * d );
   }

   // Create priority queue
   PriorityQueue Q = CreateAndInitializePriorityQueue( im_gdt, im_flags, neighborhood, neighborOffsets, coordComputer );

   // Compute distances
   FloatArray nValues( sizes.size() );
   FloatArray dist( sizes.size() );
   while( !Q.empty() ) {
      // Get next pixel to expand distances from
      dip::sint offset = Q.top().offset;
      Q.pop();
      if( IsSet( flags[ offset ], FINISHED )) { // this can happen because we push updated elements anew, rather than update them in the queue
         continue;
      }
      Set( flags[ offset ], FINISHED );
      bool isBorder = IsSet( flags[ offset ], BORDER );
      UnsignedArray coords;
      if( isBorder ) {
         coords = coordComputer( offset );
      }
      // Check all neighbors
      auto oit = neighborOffsets.begin();
      for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
         if( isBorder && !nit.IsInImage( coords, sizes )) {
            continue;
         }
         dip::sint neigh = offset + *oit;
         if( IsSet( flags[ neigh ], FINISHED ) || IsSet( flags[ neigh ], MASKED )) {
            continue;
         }
         bool nisBorder = IsSet( flags[ neigh ], BORDER );
         UnsignedArray ncoords;
         if( nisBorder ) {
            ncoords = coordComputer( neigh );
         }
         // Get neighbor values in each direction
         nValues.fill( infinity );
         auto noit = neighborOffsets.begin();
         for( auto nnit = neighborhood.begin(); nnit != neighborhood.end(); ++nnit, ++noit ) {
            // It would likely be more efficient to implement this for specific dimensionalities
            dip::uint dim = FindDim( nnit.Coordinates() );
            if( nisBorder ) {
               if( nnit.Coordinates()[ dim ] < 0 ) {
                  if( ncoords[ dim ] == 0 ) {
                     continue;
                  }
               } else {
                  if( ncoords[ dim ] + 1 == sizes[ dim ] ) {
                     continue;
                  }
               }
            }
            dip::sint nneigh = neigh + *noit;
            if( IsSet( flags[ nneigh ], MASKED )) {
               continue;
            }
            nValues[ dim ] = std::min< dfloat >( nValues[ dim ], gdt[ nneigh ] );
         }
         dist = distances;
         nValues.sort( dist );
         dfloat W = weights ? static_cast< dfloat >( weights[ neigh ] ) : 1.0;
         // Find: sum{(value - nValues[i])^2 * dist[i]} = W^2, subject to: value >= nValues[i]
         // (note that dist[i] is 1/d[i]^2, the inverse of the square distance between pixels along each dimension)
         dip::uint k = nValues.size(); // The sum is over the first k elements, as long as value >= nValues[i]
         while( std::isinf( nValues[ k - 1 ] )) { // There's always at least one valid value in here.
            --k;
         }
         // So we solve: sum(dist) * value^2 - 2 * value * sum(nValues*dist) + sum(nValues^2*dist) - W^2 = 0
         //           => value = {sum(nValues*dist) + sqrt( X )} / sum(dist)
         //        with: X = sum(nValues*dist)^2 - sum(dist) * {sum(nValues^2*dist) - W^2}, X >= 0
         // (update rule inspired by code here: https://github.com/gpeyre/matlab-toolboxes/tree/master/toolbox_fast_marching/mex)
         dfloat value = 0;
         do {
            if( k == 1 ) {
               value = nValues[ 0 ] + W / std::sqrt( dist[ 0 ] );
               break;
            }
            dfloat sumvd = 0;    // sum(nValues*dist)
            dfloat sumv2d = 0;   // sum(nValues^2*dist)
            dfloat sumd = 0;     // sum(dist)
            for( dip::uint ii = 0; ii < k; ++ii ) {
               sumvd += nValues[ ii ] * dist[ ii ];
               sumv2d += nValues[ ii ] * nValues[ ii ] * dist[ ii ];
               sumd += dist[ ii ];
            }
            dfloat X = sumvd * sumvd - sumd * ( sumv2d - W * W );
            if( X >= 0 ) {
               value = ( sumvd + std::sqrt( X )) / sumd;
            } else {
               value = 0;
            }
            --k;
         } while( value < nValues[ k ] );
         // Update
         // If we could update stuff that's in the queue, we'd check the INQUEUE flag to see if it's in the queue or not.
         if( value < gdt[ neigh ] ) {
            gdt[ neigh ] = static_cast< sfloat >( value );
            Q.push( { neigh, static_cast< sfloat >( value ) } );
            //Set( flags[ offset ], INQUEUE );
         }
      }
   }
}

template< typename TPI >
void ChamferMetricAlgorithm(
      Image const& im_weights,
      Image& im_gdt,
      Image& im_pdt,
      Image& im_flags,
      NeighborList const& neighborhood,
      IntegerArray const& neighborOffsets,
      CoordinatesComputer const& coordComputer
) {
   // Get data pointers
   TPI const* weights = im_weights.IsForged() ? static_cast< TPI const* >( im_weights.Origin() ) : nullptr;
   sfloat* gdt = static_cast< sfloat* >( im_gdt.Origin() );
   sfloat* pdt = im_pdt.IsForged() ? static_cast< sfloat* >( im_pdt.Origin() ) : nullptr;
   uint8* flags = static_cast< uint8* >( im_flags.Origin() );
   UnsignedArray const& sizes = im_gdt.Sizes();

   // Create priority queue
   PriorityQueue Q = CreateAndInitializePriorityQueue( im_gdt, im_flags, neighborhood, neighborOffsets, coordComputer );

   // Compute distances
   while( !Q.empty() ) {
      // Get next pixel to expand distances from
      dip::sint offset = Q.top().offset;
      Q.pop();
      if( IsSet( flags[ offset ], FINISHED )) {
         continue;
      }
      Set( flags[ offset ], FINISHED );
      bool isBorder = IsSet( flags[ offset ], BORDER );
      UnsignedArray coords;
      if( isBorder ) {
         coords = coordComputer( offset );
      }
      sfloat distance = gdt[ offset ];
      // Check all neighbors
      auto oit = neighborOffsets.begin();
      for( auto nit = neighborhood.begin(); nit != neighborhood.end(); ++nit, ++oit ) {
         if( isBorder && !nit.IsInImage( coords, sizes )) {
            continue;
         }
         dip::sint neigh = offset + *oit;
         if( IsSet( flags[ neigh ], FINISHED ) || IsSet( flags[ neigh ], MASKED )) {
            continue;
         }
         sfloat value = weights ? static_cast< sfloat >( weights[ neigh ] ) : 1.0f;
         value = distance + static_cast< sfloat >( *nit ) * value;
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

} // namespace

void GreyWeightedDistanceTransform(
      Image const& c_grey,
      Image const& c_bin,
      Image const& c_mask,
      Image& c_out,
      Metric metric,
      String const& mode
) {
   DIP_THROW_IF( !c_bin.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_bin.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_bin.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   dip::uint dims = c_bin.Dimensionality();
   DIP_THROW_IF( dims < 2, E::DIMENSIONALITY_NOT_SUPPORTED ); // Fast marching makes no sense with less than 2 dimensions.

   // Check grey
   if( c_grey.IsForged()) {
      DIP_THROW_IF( !c_grey.IsScalar(), E::IMAGE_NOT_SCALAR );
      DIP_THROW_IF( !c_grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_THROW_IF( c_bin.Sizes() != c_grey.Sizes(), E::SIZES_DONT_MATCH );

      // We can only support non-negative weights --
      dfloat min;
      DIP_STACK_TRACE_THIS( min = Minimum( c_grey ).As< dfloat >());
      DIP_THROW_IF( min < 0.0, "All input values must be non-negative" );
   }

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   if( c_mask.IsForged()) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( c_bin.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( c_bin.Sizes());
      DIP_END_STACK_TRACE
   }

   // What will we output?
   bool fastMarching; // true if fast marching algorithm, false if chamfer algorithm.
   bool outputDistance = false;
   if( mode == S::FASTMARCHING ) {
      fastMarching = true;
      metric = {}; // use the default city-block neighborhood
   } else {
      fastMarching = false;
      if( mode == S::LENGTH ) {
         outputDistance = true;
      } else if( mode != S::CHAMFER ) {
         DIP_THROW_INVALID_FLAG( mode );
      }
   }

   // Find pixel size to keep
   PixelSize pixelSize;
   if( c_grey.IsForged() ) {
      pixelSize = c_grey.PixelSize();
   }
   if( !pixelSize.IsDefined() ) {
      pixelSize = c_bin.PixelSize(); // Let's try this one instead...
   }
   if( !metric.HasPixelSize() ) {
      metric.SetPixelSize( pixelSize );
   }

   // Is grey OK the way it is, or will it need to be copied?
   Image grey = c_grey.QuickCopy();
   bool greyIsOK = grey.IsForged() && grey.HasContiguousData();

   // Create output image
   if( c_out.IsForged() && greyIsOK && c_out.SharesData( grey )) {
      c_out.Strip();
   }
   if( c_out.IsForged() && !c_out.CheckProperties( c_bin.Sizes(), 1, DT_SFLOAT, Option::ThrowException::DONT_THROW )) {
      c_out.Strip();
   }
   if( c_out.IsForged() && greyIsOK ) {
      // Make sure their strides match
      if( c_out.Strides() != grey.Strides() ) {
         c_out.Strip();
      }
   }
   if( !c_out.IsForged() ) {
      if( greyIsOK ) {
         c_out.SetStrides( grey.Strides() );
      }
      c_out.SetSizes( c_bin.Sizes() );
      c_out.SetDataType( DT_SFLOAT );
      DIP_STACK_TRACE_THIS( c_out.Forge() );
   }
   c_out.SetPixelSize( pixelSize );

   // Initialize `out` image
   c_out.Fill( 0 );
   c_out.At( c_bin ) = infinity;

   // Copy grey if necessary
   greyIsOK = greyIsOK && ( grey.Strides() == c_out.Strides() );
   if( grey.IsForged() && !greyIsOK ){
      Image tmp;
      tmp.SetStrides( c_out.Strides() );
      tmp.SetSizes( c_out.Sizes() );
      tmp.SetDataType( grey.DataType() );
      DIP_STACK_TRACE_THIS( tmp.Forge() );
      DIP_STACK_TRACE_THIS( tmp.Copy( grey ));
      grey.swap( tmp );
   }
   DIP_ASSERT( !grey.IsForged() || ( c_out.Strides() == grey.Strides() ));

   // Create temporary flag image
   Image flags;
   flags.SetStrides( c_out.Strides() );
   flags.SetSizes( c_out.Sizes() );
   flags.SetDataType( DT_UINT8 );
   DIP_STACK_TRACE_THIS( flags.Forge() );
   DIP_ASSERT( flags.Strides() == c_out.Strides() );

   // Create temporary distance image
   Image tmp;
   if( outputDistance ) {
      tmp.SetStrides( c_out.Strides());
      tmp.SetSizes( c_out.Sizes());
      tmp.SetDataType( DT_SFLOAT );
      DIP_STACK_TRACE_THIS( tmp.Forge());
      DIP_ASSERT( tmp.Strides() == c_out.Strides() );
      tmp.Fill( 0 );
   }

   // Remove any singleton dimensions for processing, and prevent negative strides.
   Image out = c_out; // copy also the pixel sizes!
   out.StandardizeStrides();
   flags.StandardizeStrides();
   DIP_ASSERT( flags.Sizes() == out.Sizes() );
   DIP_ASSERT( flags.Strides() == out.Strides() );
   if( grey.IsForged() ) {
      grey.StandardizeStrides();
      DIP_ASSERT( grey.Sizes() == out.Sizes() );
      DIP_ASSERT( grey.Strides() == out.Strides() );
   }
   if( tmp.IsForged() ) {
      tmp.StandardizeStrides();
      DIP_ASSERT( tmp.Sizes() == out.Sizes() );
      DIP_ASSERT( tmp.Strides() == out.Strides() );
   }
   dims = out.Dimensionality();

   // Get neighborhood with offsets
   NeighborList neighborhood{ metric, dims };
   IntegerArray offsets = neighborhood.ComputeOffsets( out.Strides() );

   // Initialize `flags` image
   UnsignedArray border = neighborhood.Border(); // is always 1 here
   flags.Fill( 0 );
   SetBorder( flags, { BORDER }, border );
   if( mask.IsForged() ) {
      JointImageIterator< uint8, dip::bin > it( { flags, mask } );
      it.OptimizeAndFlatten( 1 );
      do {
         if( !it.Sample< 1 >() ) {
            Set( it.Sample< 0 >(), MASKED );
         }
      } while( ++it );
   }

   // Create coordinate computer
   CoordinatesComputer coordComputer = out.OffsetToCoordinatesComputer();

   // Do the data-type-dependent thing
   if( fastMarching ) {
      FloatArray distances( dims );
      for( dip::uint ii = 0; ii < dims; ++ii ) {
         distances[ ii ] = out.PixelSize( ii ).magnitude;
      }
      DIP_OVL_CALL_REAL( FastMarchingAlgorithm, ( grey, out, flags, neighborhood, offsets, coordComputer, distances ), grey.DataType());
   } else {
      if( outputDistance ) {
         out.swap( tmp ); // We need to use these in a different order...
      }
      DIP_OVL_CALL_REAL( ChamferMetricAlgorithm, ( grey, out, tmp, flags, neighborhood, offsets, coordComputer ), grey.DataType() );
   }
}

} // namespace dip
