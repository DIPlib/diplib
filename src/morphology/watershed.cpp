/*
 * (c)2017-2022, Cris Luengo.
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

#include "diplib/morphology.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/border.h"
#include "diplib/iterators.h"
#include "diplib/neighborlist.h"
#include "diplib/overload.h"
#include "diplib/regions.h"
#include "diplib/statistics.h"
#include "diplib/union_find.h"

#include "watershed_support.h"

namespace dip {

namespace {

constexpr char const* STRIDES_STILL_DONOT_MATCH = "Couldn't get input and output strides to match";
constexpr char const* TOO_MANY_SEEDS = "The seed image has too many seeds";

// --- COMMON TO BOTH WATERSHED ALGORITHMS ---

template< typename TPI >
struct WatershedRegion {
   dip::uint size = 0;
   TPI lowest = 0;
   WatershedRegion() = default;
   WatershedRegion( TPI value ): size( 1 ), lowest( value ) {}
   WatershedRegion( dip::uint sz, TPI value ): size( sz ), lowest( value ) {}
};
template< typename TPI >
WatershedRegion< TPI > AddRegionsLowFist( WatershedRegion< TPI > const& region1, WatershedRegion< TPI > const& region2 ) {
   return { region1.size + region2.size, std::min( region1.lowest, region2.lowest ) };
}
template< typename TPI >
WatershedRegion< TPI > AddRegionsHighFist( WatershedRegion< TPI > const& region1, WatershedRegion< TPI > const& region2 ) {
   return { region1.size + region2.size, std::max( region1.lowest, region2.lowest ) };
}

template< typename TPI, typename UnionFunction >
using WatershedRegionList = UnionFind< LabelType, WatershedRegion< TPI >, UnionFunction >;

template< typename TPI, typename UnionFunction >
void AddPixel( WatershedRegionList< TPI, UnionFunction >& list, LabelType index, TPI value, bool lowFirst ) {
   auto& region = list.Value( index );
   ++( region.size );
   if( lowFirst ? ( region.lowest > value )
                : ( region.lowest < value )) {
      region.lowest = value;
   }
}
template< typename TPI, typename UnionFunction >
void AddPixel( WatershedRegionList< TPI, UnionFunction >& list, LabelType index ) {
   auto& region = list.Value( index );
   ++( region.size );
}

template< typename TPI >
dfloat AbsDiff( TPI a, TPI b ) {
   return static_cast< dfloat >( a > b ? a - b : b - a );
}

template< typename TPI >
inline bool WatershedShouldMerge(
      TPI value,
      WatershedRegion< TPI > const& region,
      dfloat maxDepth,
      dip::uint maxSize
) {
   return ( AbsDiff( value, region.lowest ) <= maxDepth ) &&
          (( maxSize == 0 ) || ( region.size <= maxSize ));
}

// --- FAST WATERSHED ---

enum class FastWatershedOperation : uint8 {
      WATERSHED,
      EXTREMA
};

template< typename TPI >
void FastWatershedInternal(
      Image const& c_in,
      Image& c_labels,
      Image& c_binary,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      dfloat maxDepth,
      dip::uint maxSize,
      bool lowFirst,
      bool binaryOutput,
      FastWatershedOperation operation
) {
   TPI* in = static_cast< TPI* >( c_in.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );

   auto AddRegions = lowFirst ? AddRegionsLowFist< TPI > : AddRegionsHighFist< TPI >;
   WatershedRegionList< TPI, decltype( AddRegions ) > regions( AddRegions );
   NeighborLabels neighborLabels;

   // Process first pixel
   labels[ offsets[ 0 ]] = regions.Create( in[ offsets[ 0 ]] );

   // Process other pixels
   for( dip::uint ii = 1; ii < offsets.size(); ++ii ) {
      dip::sint offset = offsets[ ii ];
      if( lowFirst ? PixelIsInfinity( in[ offset ] ) : PixelIsMinusInfinity( in[ offset ] )) {
         break; // we're done
      }
      neighborLabels.Reset();
      for( auto o : neighborOffsets ) {
         neighborLabels.Push( regions.FindRoot( labels[ offset + o ] ));
      }
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: new label
            labels[ offset ] = regions.Create( in[ offset ] );
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offset ] = lab;
            AddPixel( regions, lab );
            break;
         }
         default: {
            // Touching two or more labels
            dip::uint realRegionCount = 0;
            for( LabelType lab : neighborLabels ) {
               if( !WatershedShouldMerge( in[ offset ], regions.Value( lab ), maxDepth, maxSize )) {
                  ++realRegionCount;
               }
            }
            LabelType lab = neighborLabels.Label( 0 );
            if( realRegionCount <= 1 ) {
               // At most one is a "real" region: merge all
               for( dip::uint jj = 1; jj < neighborLabels.Size(); ++jj ) {
                  regions.Union( lab, neighborLabels.Label( jj ));
               }
               labels[ offset ] = lab;
               AddPixel( regions, lab );
            }
            // Else don't merge, leave at 0 to indicate watershed label
            break;
         }
      }
   }

   if( operation == FastWatershedOperation::WATERSHED ) {
      if( binaryOutput ) {
         // Process binary output image
         JointImageIterator< LabelType, bin > it( { c_labels, c_binary } );
         it.OptimizeAndFlatten();
         do {
            if( it.template Sample< 0 >() == 0 ) {
               it.template Sample< 1 >() = true;
            }
         } while( ++it );
      } else {
         // Process labels output image
         regions.Relabel();
         ImageIterator< LabelType > it( c_labels );
         it.OptimizeAndFlatten();
         do {
            LabelType lab = *it;
            if( lab > 0 ) {
               *it = regions.Label( lab );
            }
         } while( ++it );
      }
   } else { // operation == FastWatershedOperation::EXTREMA
      if( binaryOutput ) {
         // Process binary output image
         JointImageIterator< LabelType, TPI, bin > it( { c_labels, c_in, c_binary } );
         it.OptimizeAndFlatten();
         do {
            LabelType lab = it.template Sample< 0 >();
            if( lab > 0 ) {
               if( it.template Sample< 1 >() == regions.Value( lab ).lowest ) {
                  it.template Sample< 2 >() = true;
               }
            }
         } while( ++it );
      } else {
         // Process labels output image
         regions.Relabel();
         JointImageIterator< TPI, LabelType > it( { c_in, c_labels } );
         it.OptimizeAndFlatten();
         do {
            LabelType lab = it.Out();
            if( lab > 0 ) {
               it.Out() = ( it.In() == regions.LabelValue( lab ).lowest ) ? regions.Label( lab ) : LabelType( 0 );
            }
         } while( ++it );
      }
   }
}

void FastWatershed(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      StringSet const& flags,
      FastWatershedOperation operation
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   for( auto sz : inSizes ) {
      DIP_THROW_IF( sz < 3, "Input image is too small" );
   }
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   if( maxDepth < 0 ) {
      // We always must allow merging within a plateau
      maxDepth = 0;
   }
   bool binaryOutput = true;
   bool lowFirst = true;
   for( auto& flag : flags ) {
      if( flag == S::LABELS ) {
         binaryOutput = false;
      } else if( flag == S::BINARY ) {
         binaryOutput = true;
      } else if( flag == S::LOWFIRST ) {
         lowFirst = true;
      } else if( flag == S::HIGHFIRST ) {
         lowFirst = false;
      } else {
         DIP_THROW_INVALID_FLAG( flag );
      }
   }

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // We need the input image to have contiguous data, so that we can allocate other images with the
   // same strides. This call will copy data if `in` is an ROI in another image, or for some other
   // reason has non-contiguous data.
   in.ForceContiguousData();

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( inSizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( inSizes );
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // Create sorted offsets array (skipping border)
   std::vector< dip::sint > offsets;
   if( hasMask ) {
      offsets = CreateOffsetsArray( mask, in.Strides() );
   } else {
      offsets = CreateOffsetsArray( inSizes, in.Strides() );
   }
   SortOffsets( in, offsets, lowFirst );

   // Prepare output image
   if( in.Strides() != out.Strides() ) {
      out.Strip();
      out.SetStrides( in.Strides() );
   }
   Image binary;
   Image labels;
   if( binaryOutput ) {
      out.ReForge( in, DT_BIN );
      DIP_THROW_IF( in.Strides() != out.Strides(), STRIDES_STILL_DONOT_MATCH );
      binary = out.QuickCopy();
      binary.Fill( false );
      labels.SetStrides( in.Strides() );
      labels.ReForge( in, DT_LABEL );
      DIP_ASSERT( in.Strides() == labels.Strides() );
   } else {
      out.ReForge( in, DT_LABEL );
      DIP_THROW_IF( in.Strides() != out.Strides(), STRIDES_STILL_DONOT_MATCH );
      labels = out.QuickCopy();
      // binary remains unforged.
   }
   labels.Fill( 0 );
   out.SetPixelSize( std::move( pixelSize ));

   if( offsets.empty() ) {
      // This can happen if `mask` is empty. We test here because the output image is now forged and initialized to zeros.
      return;
   }

   // Create array with offsets to neighbors
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( in.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( FastWatershedInternal, ( in, labels, binary, offsets, neighborOffsets,
         maxDepth, maxSize, lowFirst, binaryOutput, operation ), in.DataType() );
}

// --- SEEDED WATERSHED ---

constexpr LabelType WATERSHED_LABEL = std::numeric_limits< LabelType >::max();
constexpr LabelType IMAGE_BORDER = WATERSHED_LABEL - 1;
constexpr LabelType PIXEL_ON_STACK = WATERSHED_LABEL - 2;
constexpr LabelType MAX_LABEL = WATERSHED_LABEL - 3;

// Returns true if the value is a label, not a watershed pixel or other marker value
inline bool IsValidLabel( LabelType label ) {
   return ( label > 0 ) && ( label <= MAX_LABEL );
}

// Returns true if a pixel in the neighbor list is foreground and not WATERSHED_LABEL
inline bool PixelHasForegroundNeighbor(
      LabelType const* label,
      NeighborList const& neighbors,
      IntegerArray const& neighborsLabels,
      UnsignedArray const& coords,
      UnsignedArray const& imsz,
      bool onEdge
) {
   auto it = neighbors.begin();
   for( dip::uint jj = 0; jj < neighborsLabels.size(); ++jj, ++it ) {
      if( !onEdge || it.IsInImage( coords, imsz )) {
         LabelType lab = *( label + neighborsLabels[ jj ] );
         if( IsValidLabel( lab )) {
            return true;
         }
      }
   }
   return false;
}

// Returns true if a pixel in the neighbor list is foreground and not WATERSHED_LABEL, and is larger/smaller in grey value
// COMPARATOR must be std::greater<> (for uphill) or std::less<> (for downhill).
template< typename COMPARATOR, typename TPI >
inline bool PixelHasUpOrDownhillForegroundNeighbor(
      LabelType const* label,
      TPI const* grey,
      NeighborList const& neighbors,
      IntegerArray const& neighborsOffsets,
      UnsignedArray const& coords,
      UnsignedArray const& imsz,
      bool onEdge
) {
   COMPARATOR comparator;
   auto it = neighbors.begin();
   for( dip::uint jj = 0; jj < neighborsOffsets.size(); ++jj, ++it ) {
      if( !onEdge || it.IsInImage( coords, imsz )) {
         LabelType lab = *( label + neighborsOffsets[ jj ] );
         if( IsValidLabel( lab ) && ( comparator( *( grey + neighborsOffsets[ jj ] ), *grey ))) {
            return true;
         }
      }
   }
   return false;
}

template< typename TPI >
struct Qitem {
   TPI value;              // pixel value - used for sorting
   dip::uint insertOrder;  // order of insertion - used for sorting
   dip::sint offset;       // offset into labels image
   bool isOnEdge;
};
template< typename TPI >
bool QitemComparator_LowFirst( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return ( a.value > b.value ) || (( a.value == b.value ) && ( a.insertOrder > b.insertOrder )); // NOTE comparison on insertOrder! It's always "low first"
}
template< typename TPI >
bool QitemComparator_HighFirst ( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return ( a.value < b.value ) || (( a.value == b.value ) && ( a.insertOrder > b.insertOrder )); // NOTE comparison on insertOrder! It's always "low first"
}

template< typename TPI, typename QType >
inline void EnqueueNeighbors(
      TPI* grey, LabelType* labels, BooleanArray const& useNeighbor,
      dip::sint offset, IntegerArray const& neighborOffsets,
      QType& Q, dip::uint& order, bool lowFirst, bool uphillOnly
) {
   for( dip::uint jj = 0; jj < useNeighbor.size(); ++jj ) {
      if( useNeighbor[ jj ] ) {
         dip::sint neighOffset = offset + neighborOffsets[ jj ];
         LabelType lab = labels[ neighOffset ];
         if(( lab == 0 ) || ( lab == IMAGE_BORDER )) {
            TPI nVal = grey[ neighOffset ];
            if( !uphillOnly || ( lowFirst ? grey[ offset ] < nVal : grey[ offset ] > nVal )) {
               Q.push( Qitem< TPI >{ nVal, order++, neighOffset, lab == IMAGE_BORDER } );
               labels[ neighOffset ] = PIXEL_ON_STACK;
            }
         }
      }
   }
}

template< typename TPI >
void SeededWatershedInternal(
      Image const& c_grey,
      Image& c_labels,
      IntegerArray const& neighborOffsets,
      NeighborList const& neighborList,
      dip::uint numlabs,
      dfloat maxDepth,
      dip::uint maxSize,
      bool lowFirst,
      bool binaryOutput,
      bool noGaps,
      bool uphillOnly
) {
   auto AddRegions = lowFirst ? AddRegionsLowFist< TPI > : AddRegionsHighFist< TPI >;
   WatershedRegion< TPI > defaultRegion( 0, lowFirst
                                            ? std::numeric_limits< TPI >::max()
                                            : std::numeric_limits< TPI >::lowest() );
   WatershedRegionList< TPI, decltype( AddRegions ) > regions( numlabs, defaultRegion, AddRegions );

   auto QitemComparator = lowFirst ? QitemComparator_LowFirst< TPI > : QitemComparator_HighFirst< TPI >;
   std::priority_queue< Qitem< TPI >, std::vector< Qitem< TPI >>, decltype( QitemComparator ) > Q( QitemComparator );

   dip::uint nNeigh = neighborOffsets.size();
   UnsignedArray const& imsz = c_grey.Sizes();

   // Walk over the entire image & put all the background border pixels on the heap
   JointImageIterator< TPI, LabelType > it( { c_grey, c_labels } );
   dip::uint order = 0;
   do {
      LabelType lab = it.template Sample< 1 >();
      bool onEdge = lab == IMAGE_BORDER;
      if(( lab == 0 ) || onEdge ) {
         // A non-labeled, non-watershed pixel
         if( uphillOnly
             ? ( lowFirst
                 ? PixelHasUpOrDownhillForegroundNeighbor< std::less<> >(
                           it.template Pointer< 1 >(), it.template Pointer< 0 >(),
                                 neighborList, neighborOffsets, it.Coordinates(), imsz, onEdge )
                 : PixelHasUpOrDownhillForegroundNeighbor< std::greater<> >(
                           it.template Pointer< 1 >(), it.template Pointer< 0 >(),
                                 neighborList, neighborOffsets, it.Coordinates(), imsz, onEdge ))
             : PixelHasForegroundNeighbor(
                   it.template Pointer< 1 >(), neighborList, neighborOffsets, it.Coordinates(), imsz, onEdge )) {
            Q.push( Qitem< TPI >{ it.template Sample< 0 >(), order, it.template Offset< 1 >(), onEdge } );
            it.template Sample< 1 >() = PIXEL_ON_STACK;
         }
      } else if( lab <= numlabs ) {
         // A labeled pixel
         AddPixel( regions, lab, it.template Sample< 0 >(), lowFirst );
      }
   } while( ++it );

   // Start processing pixels
   TPI* grey = static_cast< TPI* >( c_grey.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );
   auto coordinatesComputer = c_labels.OffsetToCoordinatesComputer();
   NeighborLabels neighborLabels;
   BooleanArray useNeighbor( nNeigh );
   while( !Q.empty() ) {
      dip::sint offset = Q.top().offset;
      bool onEdge = Q.top().isOnEdge;
      Q.pop();
      //DIP_ASSERT( labels[ offset ] == PIXEL_ON_STACK );
      if( lowFirst ? PixelIsInfinity( grey[ offset ] ) : PixelIsMinusInfinity( grey[ offset ] )) {
         break; // we're done
      }
      UnsignedArray coords;
      if( onEdge ) {
         coords = coordinatesComputer( offset );
      }
      neighborLabels.Reset();
      auto lit = neighborList.begin();
      for( dip::uint jj = 0; jj < nNeigh; ++jj, ++lit ) {
         useNeighbor[ jj ] = ( !onEdge || lit.IsInImage( coords, imsz )) &&
                             ( labels[ offset + neighborOffsets[ jj ]] != WATERSHED_LABEL );
         if( useNeighbor[ jj ] ) {
            LabelType lab = labels[ offset + neighborOffsets[ jj ]];
            if( IsValidLabel( lab )) {
               neighborLabels.Push( regions.FindRoot( lab ));
            }
         }
      }
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: what?
            //DIP_THROW( "This should not have happened: there's a pixel on the stack with all background neighbors!" );
            labels[ offset ] = 0;
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offset ] = lab;
            AddPixel( regions, lab, grey[ offset ], lowFirst );
            // Add all unprocessed neighbors to heap
            EnqueueNeighbors( grey, labels, useNeighbor, offset, neighborOffsets, Q, order, lowFirst, uphillOnly );
            break;
         }
         default: {
            // Touching two or more labels
            dip::uint realRegionCount = 0;
            for( LabelType lab : neighborLabels ) {
               if( !WatershedShouldMerge( grey[ offset ], regions.Value( lab ), maxDepth, maxSize )) {
                  ++realRegionCount;
               }
            }
            if( realRegionCount < 2 ) {
               // At most one is a "real" region: merge all
               LabelType lab = neighborLabels.Label( 0 );
               for( dip::uint jj = 1; jj < neighborLabels.Size(); ++jj ) {
                  lab = regions.Union( lab, neighborLabels.Label( jj ));
               }
               labels[ offset ] = lab;
               AddPixel( regions, lab, grey[ offset ], lowFirst );
               // Add all unprocessed neighbors to heap
               EnqueueNeighbors( grey, labels, useNeighbor, offset, neighborOffsets, Q, order, lowFirst, uphillOnly );
            } else {
               // Else don't merge
               if( noGaps ) {
                  // Grow one of the regions (whichever has the lowest value, which we need to find out here because
                  // we don't store the origin label in the queue)
                  TPI bestVal = 0;
                  LabelType bestLab = 0;
                  for( dip::uint jj = 0; jj < nNeigh; ++jj ) {
                     if( useNeighbor[ jj ] ) {
                        LabelType lab = labels[ offset + neighborOffsets[ jj ]];
                        if( IsValidLabel( lab )) {
                           TPI nVal = grey[ offset + neighborOffsets[ jj ]];
                           if(( bestLab == 0 ) || ( lowFirst ? nVal < bestVal : nVal > bestVal )) {
                              bestVal = nVal;
                              bestLab = lab;
                           }
                        }
                     }
                  }
                  if( bestLab == 0 ) {
                     // This should not really happen. Set as watershed label.
                     labels[ offset ] = WATERSHED_LABEL;
                  } else {
                     labels[ offset ] = bestLab;
                     AddPixel( regions, bestLab, grey[ offset ], lowFirst );
                     // Add all unprocessed neighbors to heap
                     EnqueueNeighbors( grey, labels, useNeighbor, offset, neighborOffsets, Q, order, lowFirst, uphillOnly );
                  }
               } else {
                  // Set as watershed label (so it won't be considered again)
                  labels[ offset ] = WATERSHED_LABEL;
               }
            }
            break;
         }
      }
   }

   if( !binaryOutput ) {
      // Process label image if we want to use it as such
      ImageIterator< LabelType > lit( c_labels );
      lit.OptimizeAndFlatten();
      do {
         LabelType lab = *lit;
         if( lab > MAX_LABEL ) {
            *lit = 0;
         } else if( IsValidLabel( lab )) {
            *lit = regions.FindRoot( lab );
         }
      } while( ++lit );
   }
}

} // namespace

void SeededWatershed(
      Image const& c_in,
      Image const& c_seeds,
      Image const& c_mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      StringSet const& flags
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged() || !c_seeds.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar() || !c_seeds.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !c_seeds.DataType().IsUInt() && !c_seeds.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( c_in.Sizes() != c_seeds.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool binaryOutput = true;
   bool lowFirst = true;
   bool noGaps = false;
   bool uphillOnly = false;
   for( auto& flag : flags ) {
      if( flag == S::LABELS ) {
         binaryOutput = false;
      } else if( flag == S::BINARY ) {
         binaryOutput = true;
      } else if( flag == S::LOWFIRST ) {
         lowFirst = true;
      } else if( flag == S::HIGHFIRST ) {
         lowFirst = false;
      } else if( flag == S::NOGAPS ) {
         noGaps = true;
      } else if( flag == S::UPHILLONLY ) {
         uphillOnly = true;
      } else {
         DIP_THROW_INVALID_FLAG( flag );
      }
   }
   if( noGaps ) {
      binaryOutput = false; // "no gaps" implies "labels"
   }

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // We need the input image to have contiguous data, so that we can allocate other images with the
   // same strides. This call will copy data if `in` is an ROI in another image, or for some other
   // reason has non-contiguous data.
   in.ForceContiguousData();

   // Also separate the seeds image in case it's the same as out.
   Image seeds = c_seeds.QuickCopy();

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
   }

   // Prepare output and labels image
   Image labels;
   if( binaryOutput ) {
      // The output image can be whatever, `labels` is an intermediate image
      labels.SetStrides( in.Strides() );
      labels.ReForge( in, DT_LABEL );
      DIP_ASSERT( in.Strides() == labels.Strides() );
   } else {
      // The `labels` image is the output image
      if( in.Strides() != out.Strides() ) {
         out.Strip();
         out.SetStrides( in.Strides() );
      }
      out.ReForge( in, DT_LABEL );
      DIP_THROW_IF( in.Strides() != out.Strides(), STRIDES_STILL_DONOT_MATCH );
      labels = out.QuickCopy();
   }
   dip::uint numlabs{};
   DIP_START_STACK_TRACE
      if( seeds.DataType().IsBinary() ) {
         numlabs = Label( seeds, labels, connectivity );
      } else {
         auto m = MaximumAndMinimum( seeds, mask );
         numlabs = static_cast< dip::uint >( m.Maximum() );
         labels.Copy( seeds );
      }
   DIP_END_STACK_TRACE
   DIP_THROW_IF( numlabs > MAX_LABEL, TOO_MANY_SEEDS );
   // Set pixels outside the mask region to the watershed label
   if( mask.IsForged() ) {
      labels.At( !mask ) = WATERSHED_LABEL;
   }
   // Flag border pixels without a label so we know it's on the border
   detail::ProcessBorders< LabelType >( labels, []( LabelType* ptr, dip::sint ){
      if( *ptr == 0 ) { *ptr = IMAGE_BORDER; }
   } );

   // Create array with offsets to neighbors
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( in.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( SeededWatershedInternal, ( in, labels, neighborOffsets, neighbors,
         numlabs, maxDepth, maxSize, lowFirst, binaryOutput, noGaps, uphillOnly ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into watershed lines
      DIP_STACK_TRACE_THIS( Equal( labels, WATERSHED_LABEL, out ));
   }
   out.SetPixelSize( std::move( pixelSize ));
}

// --- COMPACT SEEDED WATERSHED ---
// This is modified from SeededWatershed().

namespace {

template< typename TPI >
struct CQitem {
   dfloat value;           // pixel value + `compactness` * `seedDistance`
   dip::uint seedDistance; // distance to origin
   dip::sint offset;       // offset into labels image
   bool isOnEdge;
};
template< typename TPI >
bool CQitemComparator_LowFirst( CQitem< TPI > const& a, CQitem< TPI > const& b ) {
   return ( a.value > b.value ) || (( a.value == b.value ) && ( a.seedDistance > b.seedDistance )); // NOTE comparison on seedDistance! It's always "low first"
}
template< typename TPI >
bool CQitemComparator_HighFirst ( CQitem< TPI > const& a, CQitem< TPI > const& b ) {
   return ( a.value < b.value ) || (( a.value == b.value ) && ( a.seedDistance > b.seedDistance )); // NOTE comparison on seedDistance! It's always "low first"
}

template< typename TPI, typename QType >
inline void EnqueueNeighbors(
      TPI* grey, LabelType* labels, BooleanArray const& useNeighbor,
      dip::sint offset, IntegerArray const& neighborOffsets,
      QType& Q, dip::uint distance, dfloat compactness
) {
   for( dip::uint jj = 0; jj < useNeighbor.size(); ++jj ) {
      if( useNeighbor[ jj ] ) {
         dip::sint neighOffset = offset + neighborOffsets[ jj ];
         LabelType lab = labels[ neighOffset ];
         if(( lab == 0 ) || ( lab == IMAGE_BORDER )) {
            dfloat nVal = static_cast< dfloat >( grey[ neighOffset ] ) + compactness * static_cast< dfloat >( distance );
            Q.push( CQitem< TPI >{ nVal, distance, neighOffset, lab == IMAGE_BORDER } );
            labels[ neighOffset ] = PIXEL_ON_STACK;
         }
      }
   }
}

template< typename TPI >
void CompactWatershedInternal(
      Image const& c_grey,
      Image& c_labels,
      IntegerArray const& neighborOffsets,
      NeighborList const& neighborList,
      dfloat compactness,
      bool lowFirst,
      bool binaryOutput,
      bool noGaps
) {
   auto CQitemComparator = lowFirst ? CQitemComparator_LowFirst< TPI > : CQitemComparator_HighFirst< TPI >;
   std::priority_queue< CQitem< TPI >, std::vector< CQitem< TPI >>, decltype( CQitemComparator ) > Q( CQitemComparator );

   dip::uint nNeigh = neighborOffsets.size();
   UnsignedArray const& imsz = c_grey.Sizes();

   // Walk over the entire image & put all the background border pixels on the heap
   JointImageIterator< TPI, LabelType > it( { c_grey, c_labels } );
   do {
      LabelType lab = it.template Sample< 1 >();
      bool onEdge = lab == IMAGE_BORDER;
      if(( lab == 0 ) || onEdge ) {
         // A non-labeled, non-watershed pixel
         if( PixelHasForegroundNeighbor( it.template Pointer< 1 >(),
                                         neighborList, neighborOffsets,
                                         it.Coordinates(), imsz, onEdge )) {
            Q.push( CQitem< TPI >{ static_cast< dfloat >( it.template Sample< 0 >() ), 0, it.template Offset< 1 >(), onEdge } );
            it.template Sample< 1 >() = PIXEL_ON_STACK;
         }
      }
   } while( ++it );

   // Start processing pixels
   TPI* grey = static_cast< TPI* >( c_grey.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );
   auto coordinatesComputer = c_labels.OffsetToCoordinatesComputer();
   NeighborLabels neighborLabels;
   BooleanArray useNeighbor( nNeigh );
   while( !Q.empty() ) {
      dip::sint offset = Q.top().offset;
      bool onEdge = Q.top().isOnEdge;
      dip::uint distance = Q.top().seedDistance + 1;
      Q.pop();
      //DIP_ASSERT( labels[ offset ] == PIXEL_ON_STACK );
      if( lowFirst ? PixelIsInfinity( grey[ offset ] ) : PixelIsMinusInfinity( grey[ offset ] )) {
         break; // we're done
      }
      UnsignedArray coords;
      if( onEdge ) {
         coords = coordinatesComputer( offset );
      }
      neighborLabels.Reset();
      auto lit = neighborList.begin();
      for( dip::uint jj = 0; jj < nNeigh; ++jj, ++lit ) {
         useNeighbor[ jj ] = ( !onEdge || lit.IsInImage( coords, imsz )) &&
                             ( labels[ offset + neighborOffsets[ jj ]] != WATERSHED_LABEL );
         if( useNeighbor[ jj ] ){
            LabelType lab = labels[ offset + neighborOffsets[ jj ]];
            if( IsValidLabel( lab )) {
               neighborLabels.Push( lab );
            }
         }
      }
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: what?
            //DIP_THROW( "This should not have happened: there's a pixel on the stack with all background neighbors!" );
            labels[ offset ] = 0;
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offset ] = lab;
            // Add all unprocessed neighbors to heap
            EnqueueNeighbors( grey, labels, useNeighbor, offset, neighborOffsets, Q, distance, compactness );
            break;
         }
         default: {
            // Touching two or more labels
            if( noGaps ) {
               // Grow one of the regions (whichever has the lowest value, which we need to find out here because
               // we don't store the origin label in the queue)
               TPI bestVal = 0;
               LabelType bestLab = 0;
               for( dip::uint jj = 0; jj < nNeigh; ++jj ) {
                  if( useNeighbor[ jj ] ) {
                     LabelType lab = labels[ offset + neighborOffsets[ jj ]];
                     if( IsValidLabel( lab )) {
                        TPI nVal = grey[ offset + neighborOffsets[ jj ]];
                        if(( bestLab == 0 ) || ( lowFirst ? nVal < bestVal : nVal > bestVal )) {
                           bestVal = nVal;
                           bestLab = lab;
                        }
                     }
                  }
               }
               if( bestLab == 0 ) {
                  // This should not really happen. Set as watershed label.
                  labels[ offset ] = WATERSHED_LABEL;
               } else {
                  labels[ offset ] = bestLab;
                  // Add all unprocessed neighbors to heap
                  EnqueueNeighbors( grey, labels, useNeighbor, offset, neighborOffsets, Q, distance, compactness );
               }
            } else {
               // Set as watershed label (so it won't be considered again)
               labels[ offset ] = WATERSHED_LABEL;
            }
            break;
         }
      }
   }

   if( !binaryOutput ) {
      // Process label image if we want to use it as such
      ImageIterator< LabelType > lit( c_labels );
      lit.OptimizeAndFlatten();
      do {
         if( *lit == WATERSHED_LABEL ) {
            *lit = 0;
         }
      } while( ++lit );
   }
}

} // namespace

void CompactWatershed(
      Image const& c_in,
      Image const& c_seeds,
      Image const& c_mask,
      Image& out,
      dip::uint connectivity,
      dfloat compactness,
      StringSet const& flags
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged() || !c_seeds.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar() || !c_seeds.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !c_seeds.DataType().IsUInt() && !c_seeds.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( c_in.Sizes() != c_seeds.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool binaryOutput = true;
   bool lowFirst = true;
   bool noGaps = false;
   for( auto& flag : flags ) {
      if( flag == S::LABELS ) {
         binaryOutput = false;
      } else if( flag == S::BINARY ) {
         binaryOutput = true;
      } else if( flag == S::LOWFIRST ) {
         lowFirst = true;
      } else if( flag == S::HIGHFIRST ) {
         lowFirst = false;
      } else if( flag == S::NOGAPS ) {
         noGaps = true;
      } else {
         DIP_THROW_INVALID_FLAG( flag );
      }
   }
   if( noGaps ) {
      binaryOutput = false; // "no gaps" implies "labels"
   }
   compactness = std::abs( compactness );
   if( !lowFirst ) {
      compactness = -compactness; // This subtracts the distance from the grey values, when doing upside-down watershed.
   }

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // We need the input image to have contiguous data, so that we can allocate other images with the
   // same strides. This call will copy data if `in` is an ROI in another image, or for some other
   // reason has non-contiguous data.
   in.ForceContiguousData();

   // Also separate the seeds image in case it's the same as out.
   Image seeds = c_seeds.QuickCopy();

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
   }

   // Prepare output image
   // Prepare output and labels image
   Image labels;
   if( binaryOutput ) {
      // The output image can be whatever, `labels` is an intermediate image
      labels.SetStrides( in.Strides() );
      labels.ReForge( in, DT_LABEL );
      DIP_ASSERT( in.Strides() == labels.Strides() );
   } else {
      // The `labels` image is the output image
      if( in.Strides() != out.Strides() ) {
         out.Strip();
         out.SetStrides( in.Strides() );
      }
      out.ReForge( in, DT_LABEL );
      DIP_THROW_IF( in.Strides() != out.Strides(), STRIDES_STILL_DONOT_MATCH );
      labels = out.QuickCopy();
   }
   dip::uint numlabs{};
   DIP_START_STACK_TRACE
   if( seeds.DataType().IsBinary() ) {
      numlabs = Label( seeds, labels, connectivity );
   } else {
      auto m = MaximumAndMinimum( seeds, mask );
      numlabs = static_cast< dip::uint >( m.Maximum() );
      labels.Copy( seeds );
   }
   DIP_END_STACK_TRACE
   DIP_THROW_IF( numlabs > MAX_LABEL, TOO_MANY_SEEDS );
   // Set pixels outside the mask region to the watershed label
   if( mask.IsForged() ) {
      labels.At( !mask ) = WATERSHED_LABEL;
   }
   // Flag border pixels without a label so we know it's on the border
   detail::ProcessBorders< LabelType >( labels, []( LabelType* ptr, dip::sint ){
      if( *ptr == 0 ) { *ptr = IMAGE_BORDER; }
   } );

   // Create array with offsets to neighbors
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( in.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( CompactWatershedInternal, ( in, labels, neighborOffsets, neighbors,
         compactness, lowFirst, binaryOutput, noGaps ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into watershed lines
      DIP_STACK_TRACE_THIS( Equal( labels, WATERSHED_LABEL, out ));
   }
   out.SetPixelSize( std::move( pixelSize ));
}

// --- DISPATCH FUNCTIONS ---

void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      StringSet flags // by copy so we can modify it
) {
   bool correct = flags.count( S::CORRECT ) != 0;
   // we remove these two elements if there, so we don't throw an error later when we see them.
   flags.erase( S::CORRECT );
   flags.erase( S::FAST );
   DIP_START_STACK_TRACE
      if( correct ) {
         Image seeds;
         if( flags.count( S::HIGHFIRST )) {
            seeds = Maxima( in, connectivity, S::LABELS );
         } else {
            seeds = Minima( in, connectivity, S::LABELS );
         }
         SeededWatershed( in, seeds, mask, out, connectivity, maxDepth, maxSize, flags );
      } else {
         FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, flags, FastWatershedOperation::WATERSHED );
      }
   DIP_END_STACK_TRACE
}

void WatershedMinima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      String const& output
) {
   DIP_STACK_TRACE_THIS( FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, { output, S::LOWFIRST }, FastWatershedOperation::EXTREMA ));
}

void WatershedMaxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      String const& output
) {
   DIP_STACK_TRACE_THIS( FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, { output, S::HIGHFIRST }, FastWatershedOperation::EXTREMA ));
}

} // namespace dip
