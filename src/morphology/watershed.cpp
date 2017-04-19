/*
 * DIPlib 3.0
 * This file contains the various watershed implementations and related functions.
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

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/overload.h"
#include "offsets.h"

namespace dip {

namespace {

using LabelType = dip::uint32;
constexpr auto DT_LABEL = DT_UINT32;
constexpr dip::uint WATERSHED_LABEL = std::numeric_limits< dip::uint32 >::max();
//constexpr dip::uint PIXEL_ON_STACK = WATERSHED_LABEL - 1;
constexpr dip::uint MAX_LABEL = WATERSHED_LABEL - 2;

template< typename TPI >
class WatershedRegionList {
   public:
      struct WatershedRegion {
         dip::uint size;
         TPI lowest;
         LabelType mapped;
      };
      using WatershedRegionVector = std::vector< WatershedRegion >;
      WatershedRegionList() {
         region.reserve( 1000 );
         region.push_back( { 0, 0, 0 } ); // This element will not be used.
      }
      LabelType CreateRegion( TPI value ) {
         if( region.size() > MAX_LABEL ) {
            // TODO: remap?
            DIP_THROW( "Cannot create more regions!" );
         }
         LabelType index = static_cast< LabelType >( region.size() );
         region.push_back( { 1, value, index } );
         return index;
      }
      void MergeRegions( LabelType label, LabelType other ) {
         region[ label ].lowest = std::min( region[ label ].lowest, region[ other ].lowest );
         region[ label ].size += region[ other ].size;
         for( auto& r : region ) {
            if( r.mapped == other ) {
               r.mapped = label;
            }
         }
         // region[ other ].mapped = label; // this happens automatically in the loop above.
      }
      WatershedRegion& operator[]( LabelType index ) { return region[ index ]; }
      WatershedRegion const& operator[]( LabelType index ) const { return region[ index ]; }
      typename WatershedRegionVector::iterator begin() { return region.begin(); }
      typename WatershedRegionVector::const_iterator begin() const { return region.begin(); }
      typename WatershedRegionVector::iterator end() { return region.end(); }
      typename WatershedRegionVector::const_iterator end() const { return region.end(); }
   private:
      WatershedRegionVector region;
};

template< typename TPI >
TPI AbsDiff( TPI a, TPI b ) {
   return a > b ? a - b : b - a;
}

template< typename TPI >
bool WatershedShouldMerge(
      LabelType lab,
      TPI value,
      WatershedRegionList< TPI > const& regions,
      dfloat maxDepth,
      dfloat maxSize
) {
   return ( AbsDiff( value, regions[ lab ].lowest ) <= maxDepth ) &&
          (( maxSize == 0 ) || ( regions[ lab ].size <= maxSize ));
}

// This class manages a list of neighbour labels.
// There are never more than N neighbours added at a time, N being defined
// by the dimensionality and the connectivity. However, typically there are
// only one or two labels added. Therefore, no effort has been put into making
// this class clever. We could keep a sorted list, but the sorting might costs
// more effort than it would save in checking if a label is present (would it?)
class NeighborLabels{
   public:
      void Reset() { labels.clear(); }
      void Push( LabelType value ) {
         if(( value != 0 ) && !Contains( value )) {
            labels.push_back( value );
         }
      }
      bool Contains( LabelType value ) const {
         for( auto l : labels ) {
            if( l == value ) {
               return true;
            }
         }
         return false;
      }
      dip::uint Size() const {
         return labels.size();
      }
      LabelType Label( dip::uint index ) const {
         return labels[ index ];
      }
   private:
      std::vector< LabelType > labels;
};

template< typename TPI, typename std::enable_if< !std::numeric_limits< TPI >::has_infinity, int >::type = 0 >
bool PixelIsInfinite( TPI /*value*/ ) {
   return false;
}
template< typename TPI, typename std::enable_if< std::numeric_limits< TPI >::has_infinity, int >::type = 0 >
bool PixelIsInfinite( TPI value ) {
   return value == std::numeric_limits< dfloat >::infinity();
}

template< typename TPI >
void dip__FastWatershed(
      Image const& c_in,
      Image& c_labels,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      dfloat maxDepth,
      dfloat maxSize,
      bool binaryOutput
) {
   TPI* in = static_cast< TPI* >( c_in.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );

   WatershedRegionList< TPI > regions;
   NeighborLabels neighborLabels;

   // Process first pixel
   labels[ offsets[ 0 ]] = regions.CreateRegion( in[ offsets[ 0 ]] );

   // Process other pixels
   for( dip::uint ii = 1; ii < offsets.size(); ++ii ) {
      dip::sint offset = offsets[ ii ];
      if( PixelIsInfinite( in[ offset ] )) {
         // TODO: in case of reverse ordered watershed, we need to test for minus infinity
         break; // we're done
      }
      neighborLabels.Reset();
      for( auto o : neighborOffsets ) {
         neighborLabels.Push( regions[ labels[ offset + o ]].mapped );
      }
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: new label
            labels[ offset ] = regions.CreateRegion( in[ offset ] );
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offset ] = lab;
            ++( regions[ lab ].size );
            break;
         }
         default: {
            // Touching two or more labels
            dip::uint realRegionCount = 0;
            for( dip::uint jj = 0; jj < neighborLabels.Size(); ++jj ) {
               LabelType lab = neighborLabels.Label( jj );
               if( !WatershedShouldMerge( lab, in[ offset ], regions, maxDepth, maxSize )) {
                  ++realRegionCount;
               }
            }
            LabelType lab = neighborLabels.Label( 0 );
            if( realRegionCount < 2 ) {
               // At most one is a "real" region: merge all
               for( dip::uint jj = 1; jj < neighborLabels.Size(); ++jj ) {
                  regions.MergeRegions( lab, neighborLabels.Label( jj ) );
               }
               labels[ offset ] = lab;
               ++( regions[ lab ].size );
            }
            // Else don't merge, set as watershed label
            //labels[ offset ] = WATERSHED_LABEL;
            break;
         }
      }
   }

   if( !binaryOutput ) {
      // Process label image
      // if binaryOutput it doesn't matter - we're thresholding this label image anyways
      ImageIterator< LabelType > it( c_labels );
      do {
         LabelType lab1 = *it;
         LabelType lab2 = regions[ lab1 ].mapped;
         if( lab1 != lab2 ) {
            *it = lab2;
         }
      } while( ++it );
   }
}


void FastWatershed(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      StringSet const& flags
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   for( auto sz : inSizes ) {
      DIP_THROW_IF( sz < 3, "Input image is too small" );
   }
   DIP_THROW_IF(( connectivity < 1 ) || ( connectivity > nDims ), E::ILLEGAL_CONNECTIVITY );
   if( maxDepth < 0 ) {
      maxDepth = 0;
   }
   bool binaryOutput = flags.count( "labels" ) == 0;
   bool lowFirst = flags.count( "high first" ) == 0;

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

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
   out.ReForge( in, DT_LABEL );
   DIP_THROW_IF( in.Strides() != out.Strides(), "Cannot reforge output image with same strides as input" );
   // TODO: We could create a temporary image here, with the needed strides, and then copy (or move) to out.
   // TODO: Alternatively, we could copy the input if it doesn't have compact strides.
   out.Fill( 0 );

   // Create array with offsets to neighbours
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( in.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__FastWatershed, ( in, out, offsets, neighborOffsets, maxDepth, maxSize, binaryOutput ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into watershed lines
      Equal( out, 0, out );
   }
}

} // namespace

void Watershed(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      StringSet const& flags // "labels" / "binary", "low first" / "high first", "fast" / "correct"
) {
   if( flags.count( "correct" )) {
      Image seeds;
      if( flags.count( "high first" )) {
         seeds = Maxima( in, mask, connectivity, "labels" );
      } else {
         seeds = Minima( in, mask, connectivity, "labels" );
      }
      //SeededWatershed( in,  mask, out, connectivity, maxDepth, maxSize, flags );
   } else {
      FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, flags );
   }
}

} // namespace dip
