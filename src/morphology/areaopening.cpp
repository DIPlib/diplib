/*
 * DIPlib 3.0
 * This file contains the area opening and related functions.
 *
 * (c)2008, 2017, Cris Luengo.
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

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/binary.h"
#include "diplib/neighborlist.h"
#include "diplib/iterators.h"
#include "diplib/boundary.h"
#include "diplib/overload.h"
#include "diplib/union_find.h"
#include "watershed_support.h"

namespace dip {

namespace {

/*
The code below is very similar to that for FastWatershed, but instead of recording the value of the local
minimum/maximum in each watershed basin, we record the grey value at which the size criterion was met.
This value can then be used to paint all pixels that are larger/smaller within the basin.
*/

template< typename TPI >
struct AreaOpenRegion {
   dip::uint size = 0;
   TPI lowest = 0;

   AreaOpenRegion() = default;
   explicit AreaOpenRegion( TPI value ) : size( 1 ), lowest( value ) {}

   dip::uint Param() const {
      return size;
   }

   void Saturate( dip::uint filterSize ) {
      size = filterSize;
   }

   void AddRegionSize( AreaOpenRegion const& other ) {
      size += other.size - 1; // Any time we use this operator, the current pixel has already been added to both regions
   }

   void AddPixel( TPI value, dip::uint filterSize ) {
      if( size < filterSize ) {
         ++size;
         lowest = value;
      }
   }
};

/*
For the volume opening, we do the same thing but track the volume of the peak instead of its area.
*/

template< typename TPI >
struct VolumeOpenRegion {
   dip::uint size = 0;
   dfloat volume = 0;
   TPI lowest = 0;

   VolumeOpenRegion() = default;
   explicit VolumeOpenRegion( TPI value ) : size( 1 ), lowest( value ) {}

   dfloat Param() const {
      return volume;
   }

   void Saturate( dfloat filterSize ) {
      volume = filterSize;
   }

   void AddRegionSize( VolumeOpenRegion const& other ) {
      size += other.size - 1; // Any time we use this operator, the current pixel has already been added to both regions, so there's no need to update `lowest`.
      volume += other.volume;
   }

   void AddPixel( TPI value, dfloat filterSize ) {
      if( volume < filterSize ) { // let's compute only if necessary
         dfloat height = std::abs( static_cast< dfloat >( lowest ) - static_cast< dfloat >( value ));
         if(( volume + static_cast< dfloat >( size ) * height ) < filterSize ) {
            // Adding the full height will not make this region larger than the filter size
            volume += static_cast< dfloat >( size ) * height;
            lowest = value;
         } else {
            // We need to add a smaller height to get the exact filter size.
            // But actually, the height is a little less than the exact result, so that our volume remains strictly less than `filterSize`.
            height = ( filterSize - volume ) / static_cast< dfloat >( size ) * ( 1 - 1e-6 );
            if( lowest > value ) {
               height = -height; // we want to subtract from `lowest`, rather than add, to move towards `value`.
            }
            volume = filterSize; // equal to `volume += static_cast< dfloat >( size ) * height` but without rounding errors.
            lowest = static_cast< TPI >( lowest + static_cast< TPI >( height )); // casting to integer will always round towards zero, which is just what we want to do here.
         }
         ++size;
      }
   }
};


template< typename TPI, typename RegionType >
RegionType AddRegions( RegionType region1, RegionType const& region2 ) {
   // When we get here, we've already added the current pixel to both regions, so the `lowest` value should be the same.
   DIP_ASSERT( region1.lowest == region2.lowest );
   region1.AddRegionSize( region2 );
   return region1;
}


template< typename UnionFunction, typename RegionType >
using ParametricOpenRegionList = UnionFind< LabelType, RegionType, UnionFunction >;


template< typename TPI, typename RegionType, typename ParamType >
void ParametricOpeningInternal(
      Image& c_grey,
      Image& c_labels,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      ParamType filterSize,
      bool lowFirst
) {
   TPI* grey = static_cast< TPI* >( c_grey.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );

   auto addRegionsFnc = AddRegions< TPI, RegionType >;
   ParametricOpenRegionList< decltype( addRegionsFnc ), RegionType > regions( addRegionsFnc );
   NeighborLabels neighborLabels;

   // Process first pixel
   labels[ offsets[ 0 ]] = regions.Create( RegionType( grey[ offsets[ 0 ]] ));

   // Process other pixels
   for( dip::uint ii = 1; ii < offsets.size(); ++ii ) {
      dip::sint offset = offsets[ ii ];
      if( lowFirst ? PixelIsInfinity( grey[ offset ] ) : PixelIsMinusInfinity( grey[ offset ] )) {
         break; // we're done
      }
      neighborLabels.Reset();
      for( auto o : neighborOffsets ) {
         neighborLabels.Push( regions.FindRoot( labels[ offset + o ] ));
      }
      if( neighborLabels.Size() == 0 ) {
         // Not touching a label: new label
         labels[ offset ] = regions.Create( RegionType( grey[ offset ] ));
      } else if( neighborLabels.Size() == 1 ) {
         // Touching a single label: grow
         LabelType lab = neighborLabels.Label( 0 );
         labels[ offset ] = lab;
         regions.Value( lab ).AddPixel( grey[ offset ], filterSize );
      } else {
         // Touching two or more labels
         // Grow each of the small regions, and find the label of the smallest region
         LabelType lab = neighborLabels.Label( 0 );
         ParamType size = regions.Value( lab ).Param();
         for( auto lab2 : neighborLabels ) {
            if( regions.Value( lab2 ).Param() < size ) {
               lab = lab2;
               size = regions.Value( lab ).Param();
            }
            regions.Value( lab2 ).AddPixel( grey[ offset ], filterSize );
         }
         // Assign the pixel to the smallest region
         labels[ offset ] = lab;
         if( regions.Value( lab ).Param() < filterSize ) {
            // If the region is still small, combine information from the other regions
            for( LabelType lab2 : neighborLabels ) {
               if( lab != lab2 ) {
                  if(( regions.Value( lab ).Param() + regions.Value( lab2 ).Param() ) < filterSize ) {
                     regions.Union( lab, lab2 );
                  } else {
                     // If we don't merge, both regions should stop growing
                     regions.Value( lab ).Saturate( filterSize );
                     regions.Value( lab2 ).Saturate( filterSize );
                  }
               }
            }
         }
      }

   }
   JointImageIterator< TPI, LabelType > it( { c_grey, c_labels } );
   it.OptimizeAndFlatten();
   if( lowFirst ) {
      do {
         LabelType lab = it.template Sample< 1 >();
         if( lab > 0 ) {
            TPI v = regions.Value( lab ).lowest;
            if( it.template Sample< 0 >() < v ) {
               it.template Sample< 0 >() = v;
            }
         }
      } while( ++it );
   } else {
      do {
         LabelType lab = it.template Sample< 1 >();
         if( lab > 0 ) {
            TPI v = regions.Value( lab ).lowest;
            if( it.template Sample< 0 >() > v ) {
               it.template Sample< 0 >() = v;
            }
         }
      } while( ++it );
   }
}

template< typename TPI >
void AreaOpeningInternal(
      Image& grey,
      Image& labels,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      dip::uint filterSize,
      bool lowFirst
) {
   ParametricOpeningInternal< TPI, AreaOpenRegion< TPI >>( grey, labels, offsets, neighborOffsets, filterSize, lowFirst );
}

template< typename TPI >
void VolumeOpeningInternal(
      Image& grey,
      Image& labels,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      dfloat filterSize,
      bool lowFirst
) {
   ParametricOpeningInternal< TPI, VolumeOpenRegion< TPI >>( grey, labels, offsets, neighborOffsets, filterSize, lowFirst );
}

enum class ParametricOpeningMode {
      AREA_OPENING,
      VOLUME_OPENING
};

void ParametricOpening(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      void* filterSizePtr, // Points to either a dip::uint or a dip::dfloat, depending on `mode`. Sorry, don't know how else to do this nicely without a `constexpr if`. TODO: Upgrade to C++17?
      dip::uint connectivity,
      String const& polarity,
      ParametricOpeningMode mode
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool lowFirst;
   DIP_STACK_TRACE_THIS( lowFirst = BooleanFromString( polarity, S::CLOSING, S::OPENING ));

   // Add a 1-pixel boundary around the input image
   Image grey;
   ExtendImage( c_in, grey, { 1 }, { lowFirst ? BoundaryCondition::ADD_MAX_VALUE : BoundaryCondition::ADD_MIN_VALUE } );

   // Prepare labels image
   Image labels;
   labels.SetStrides( grey.Strides() );
   labels.SetSizes( grey.Sizes() );
   labels.SetDataType( DT_LABEL );
   labels.Forge();
   DIP_ASSERT( labels.Strides() == grey.Strides() );
   labels.Fill( 0 );

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      UnsignedArray const& inSizes = c_in.Sizes();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( inSizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( inSizes );
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // Create sorted offsets array (skipping border)
   std::vector< dip::sint > offsets;
   if( hasMask ) {
      offsets = CreateOffsetsArray( mask, grey.Strides() );
      // The mask is shifted by one pixel, because we added a pixel to `grey`. Add the offset to the offsets!
      UnsignedArray pos( nDims, 1 );
      dip::sint firstPixelOffset = grey.Offset( pos );
      for( auto& o : offsets ) {
         o += firstPixelOffset;
      }
   } else {
      offsets = CreateOffsetsArray( grey.Sizes(), grey.Strides() );
   }
   if( offsets.empty() ) {
      // This can happen if `mask` is empty.
      return;
   }
   SortOffsets( grey, offsets, lowFirst );

   // Create array with offsets to neighbors
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( grey.Strides() );

   // Do the data-type-dependent thing
   DIP_START_STACK_TRACE
      if( mode == ParametricOpeningMode::AREA_OPENING ) {
         DIP_OVL_CALL_REAL( AreaOpeningInternal, ( grey, labels, offsets, neighborOffsets, *static_cast< dip::uint* >( filterSizePtr ), lowFirst ), grey.DataType());
      } else {
         DIP_OVL_CALL_REAL( VolumeOpeningInternal, ( grey, labels, offsets, neighborOffsets, *static_cast< dfloat* >( filterSizePtr ), lowFirst ), grey.DataType());
      }
   DIP_END_STACK_TRACE

   // Copy result to output
   grey.Crop( c_in.Sizes() );
   PixelSize pixelSize = c_in.PixelSize();
   out.Copy( grey );
   out.SetPixelSize( std::move( pixelSize ));
}

} // namespace

void AreaOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity,
      String const& polarity
) {
   bool lowFirst;
   DIP_STACK_TRACE_THIS( lowFirst = BooleanFromString( polarity, S::CLOSING, S::OPENING ));
   if( in.DataType().IsBinary() ) {
      DIP_START_STACK_TRACE
         if( lowFirst ) {
            BinaryAreaClosing( in, out, filterSize, connectivity );
         } else {
            BinaryAreaOpening( in, out, filterSize, connectivity );
         }
      DIP_END_STACK_TRACE
      return;
   }
   DIP_STACK_TRACE_THIS( ParametricOpening( in, mask, out, &filterSize, connectivity, polarity, ParametricOpeningMode::AREA_OPENING ));
}

void VolumeOpening(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat filterSize,
      dip::uint connectivity,
      String const& polarity
) {
   DIP_THROW_IF( filterSize <= 0, E::INVALID_PARAMETER );
   DIP_STACK_TRACE_THIS( ParametricOpening( in, mask, out, &filterSize, connectivity, polarity, ParametricOpeningMode::VOLUME_OPENING ));
}

} // namespace dip
