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

#include <functional>

#include "diplib.h"
#include "diplib/morphology.h"
#include "diplib/overload.h"
#include "offsets.h"

namespace dip {

namespace {

enum class FastWatershedOperation {
      WATERSHED,
      EXTREMA
};

using LabelType = dip::uint32;
constexpr auto DT_LABEL = DT_UINT32;
constexpr LabelType WATERSHED_LABEL = std::numeric_limits< LabelType >::max();
constexpr LabelType PIXEL_ON_STACK = WATERSHED_LABEL - 1;
constexpr LabelType MAX_LABEL = WATERSHED_LABEL - 2;

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
      explicit WatershedRegionList( dip::uint n ) {
         region.resize( n + 1, { 0, 0, 0 } );
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
      void MergeRegions( LabelType label, LabelType other, bool lowFirst ) {
         region[ label ].lowest = lowFirst ? std::min( region[ label ].lowest, region[ other ].lowest )
                                           : std::max( region[ label ].lowest, region[ other ].lowest );
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
   return a > b ? TPI( a - b ) : TPI( b - a ); // casting back to TPI, because of inane default casts.
}

template< typename TPI >
bool WatershedShouldMerge(
      LabelType lab,
      TPI value,
      WatershedRegionList< TPI > const& regions,
      dfloat maxDepth,
      dip::uint maxSize
) {
   return ( AbsDiff( value, regions[ lab ].lowest ) <= maxDepth ) &&
          (( maxSize == 0 ) || ( regions[ lab ].size <= maxSize ));
}

// Returns true if a pixel in the neighbor list is foreground and has the mask set
bool PixelHasForegroundNeighbour(
      LabelType* label,
      bin* mask,
      NeighborList neighbors,
      IntegerArray const& neighborsLabels,
      IntegerArray const& neighborsMask,
      UnsignedArray const& coords,
      UnsignedArray const& imsz
) {
   auto it = neighbors.begin();
   for( dip::uint jj = 0; jj < neighborsLabels.size(); ++jj, ++it ) {
      if( it.IsInImage( coords, imsz ) ) {
         if(( *( label + neighborsLabels[ jj ] ) > 0 ) &&
            ( !mask || *( mask + neighborsMask[ jj ] ) != 0 )) {
            return true;
         }
      }
   }
   return false;
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
            if( realRegionCount <= 1 ) {
               // At most one is a "real" region: merge all
               for( dip::uint jj = 1; jj < neighborLabels.Size(); ++jj ) {
                  regions.MergeRegions( lab, neighborLabels.Label( jj ), lowFirst );
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

   if( operation == FastWatershedOperation::WATERSHED ) {
      if( binaryOutput ) {
         // Process binary output image
         JointImageIterator< LabelType, bin > it( { c_labels, c_binary } );
         do {
            if( it.template Sample< 0 >() > 0 ) {
               it.template Sample< 1 >() = true;
            }
         } while( ++it );
      } else {
         // Process labels output image
         ImageIterator< LabelType > it( c_labels );
         do {
            LabelType lab1 = *it;
            if( lab1 > 0 ) {
               LabelType lab2 = regions[ lab1 ].mapped;
               if( lab1 != lab2 ) {
                  *it = lab2;
               }
            }
         } while( ++it );
      }
   } else { // operation == FastWatershedOperation::EXTREMA
      if( binaryOutput ) {
         // Process binary output image
         JointImageIterator< LabelType, TPI, bin > it( { c_labels, c_in, c_binary } );
         do {
            LabelType lab = it.template Sample< 0 >();
            if( lab > 0 ) {
               lab = regions[ lab ].mapped;
               if( it.template Sample< 1 >() == regions[ lab ].lowest ) {
                  it.template Sample< 2 >() = true;
               }
            }
         } while( ++it );
      } else {
         // Process labels output image
         JointImageIterator< TPI, LabelType > it( { c_in, c_labels } );
         do {
            LabelType lab = it.Out();
            if( lab > 0 ) {
               lab = regions[ lab ].mapped;
               it.Out() = it.In() == regions[ lab ].lowest ? lab : LabelType( 0 );
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
   Image binary;
   Image labels;
   if( binaryOutput ) {
      out.ReForge( in, DT_BIN );
      binary = out.QuickCopy();
      binary.Fill( false );
      labels.SetStrides( in.Strides() );
      labels.ReForge( in, DT_LABEL );
   } else {
      out.ReForge( in, DT_LABEL );
      labels = out.QuickCopy();
      // binary remains unforged.
   }
   DIP_THROW_IF( in.Strides() != labels.Strides(), "Cannot reforge labels image with same strides as input" );
   DIP_THROW_IF( binaryOutput && ( in.Strides() != binary.Strides() ), "Cannot reforge output image with same strides as input" );
   // TODO: We could create a temporary image here, with the needed strides, and then copy (or move) to out.
   // TODO: Alternatively, we could copy the input if it doesn't have compact strides.
   labels.Fill( 0 );

   // Create array with offsets to neighbours
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( in.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__FastWatershed, ( in, labels, binary, offsets, neighborOffsets,
         maxDepth, maxSize, lowFirst, binaryOutput, operation ), in.DataType() );
}

template< typename TPI >
struct Qitem {
   TPI value;           // pixel value - used for sorting */
   dip::uint insertOrder;  // order of insertion - used for sorting */
   dip::sint offset;       // offset into labels image
};
template< typename TPI >
struct QitemComparator {
   virtual bool operator()( Qitem< TPI > const&, Qitem< TPI > const& ) { return false; }; // just to silence the linker
};
template< typename TPI >
struct QitemComparator_LowFirst : public QitemComparator< TPI > {
   virtual bool operator()( Qitem< TPI > const& a, Qitem< TPI > const& b ) override {
      return ( a.value > b.value ) || (( a.value == b.value ) && ( a.insertOrder < b.insertOrder )); // NOTE comparison on insertOrder!
   }
};
template< typename TPI >
struct QitemComparator_HighFirst : public QitemComparator< TPI > {
   virtual bool operator()( Qitem< TPI > const& a, Qitem< TPI > const& b ) override {
      return ( a.value < b.value ) || (( a.value == b.value ) && ( a.insertOrder < b.insertOrder ));
   }
};

template< typename TPI >
void dip__SeededWatershed(
      Image const& c_grey,
      Image const& c_mask,
      Image& c_labels,
      IntegerArray const& neighborOffsetsGrey,
      IntegerArray const& neighborOffsetsMask,
      IntegerArray const& neighborOffsetsLabels,
      NeighborList const& neighborList,
      dip::uint numlabs,
      dfloat maxDepth,
      dip::uint maxSize,
      bool lowFirst,
      bool binaryOutput
) {
   WatershedRegionList< TPI > regions( numlabs );
   QitemComparator< TPI > comparator;
   if( lowFirst ) {
      comparator = QitemComparator_LowFirst< TPI >();
   } else {
      comparator = QitemComparator_HighFirst< TPI >();
   }
   std::priority_queue< Qitem< TPI >, std::vector< Qitem< TPI >>, QitemComparator< TPI >> Q( comparator );

   dip::uint nNeigh = neighborOffsetsLabels.size();
   UnsignedArray const& imsz = c_grey.Sizes();

   // Walk over the entire image & put all the background border pixels on the heap
   JointImageIterator< TPI, LabelType, bin > it( { c_grey, c_labels, c_mask } );
   bool hasMask = c_mask.IsForged();
   dip::uint order = 0;
   do {
      if( !hasMask || it.template Sample< 2 >() ) {
         LabelType lab = it.template Sample< 1 >();
         if( lab == 0 ) {
            if( PixelHasForegroundNeighbour( it.template Pointer< 1 >(), it.template Pointer< 2 >(), neighborList,
                                             neighborOffsetsLabels, neighborOffsetsMask,
                                             it.Coordinates(), imsz )) {
               Q.push( Qitem< TPI >{ it.template Sample< 0 >(), order++, it.template Offset< 1 >() } );
               it.template Sample< 1 >() = PIXEL_ON_STACK;
            }
         } else { /* lab > 0 */
            DIP_ASSERT( lab <= numlabs ); // Not really necessary, is it?
            ++( regions[ lab ].size );
            TPI value = it.template Sample< 0 >();
            if( regions[ lab ].mapped == 0 ) {
               regions[ lab ].mapped = lab;
               regions[ lab ].lowest = value;
            } else {
               if( lowFirst ? ( regions[ lab ].lowest > value )
                            : ( regions[ lab ].lowest < value )) {
                  regions[ lab ].lowest = value;
               }
            }
         }
      }
   } while( ++it );

   // Start processing pixels
   TPI* grey = static_cast< TPI* >( c_grey.Origin() );
   bin* mask = nullptr;
   if( c_mask.IsForged() ) {
      mask = static_cast< bin* >( c_mask.Origin() );
   }
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );
   auto coordinatesComputer = c_labels.OffsetToCoordinatesComputer();
   NeighborLabels neighborLabels;
   BooleanArray skipar( nNeigh );
   while( !Q.empty() ) {
      dip::sint offsetLabels = Q.top().offset;
      Q.pop();
      UnsignedArray coords = coordinatesComputer( offsetLabels );
      dip::sint offsetGrey = c_grey.Offset( coords );
      if( PixelIsInfinite( grey[ offsetGrey ] )) {
         // TODO: in case of reverse ordered watershed, we need to test for minus infinity
         break; // we're done
      }
      neighborLabels.Reset();
      auto lit = neighborList.begin();
      for( dip::uint jj = 0; jj < nNeigh; ++jj, ++lit ) {
         skipar[ jj ] = lit.IsInImage( coords, imsz );
         if( !skipar[ jj ] ) {
            if( !mask || *( mask + neighborOffsetsMask[ jj ] )) {
               LabelType lab = labels[ offsetLabels + neighborOffsetsLabels[ jj ]];
               if( lab > 0 ) {
                  neighborLabels.Push( regions[ lab ].mapped );
               }
            }
         }
      }
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: what?
            DIP_THROW( "This should not have happened: there's a pixel on the stack with all background neighbours!" );
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offsetLabels ] = lab;
            ++( regions[ lab ].size );
            if( lowFirst ? ( regions[ lab ].lowest > grey[ offsetGrey ] )
                         : ( regions[ lab ].lowest < grey[ offsetGrey ] )) {
               regions[ lab ].lowest = grey[ offsetGrey ];
            }
            // Add all unprocessed neighbours to heap
            for( dip::uint jj = 0; jj < nNeigh; ++jj ) {
               if( !skipar[ jj ] ) {
                  if( !mask || *( mask + neighborOffsetsMask[ jj ] )) {
                     dip::sint neighOffset = offsetLabels + neighborOffsetsLabels[ jj ];
                     if( labels[ neighOffset ] == 0 ) {
                        Q.push( Qitem< TPI >{ grey[ offsetGrey + neighborOffsetsGrey[ jj ]], order++, neighOffset } );
                        labels[ neighOffset ] = PIXEL_ON_STACK;
                     }
                  }
               }
            }
            break;
         }
         default: {
            // Touching two or more labels
            dip::uint realRegionCount = 0;
            for( dip::uint jj = 0; jj < neighborLabels.Size(); ++jj ) {
               LabelType lab = neighborLabels.Label( jj );
               if( !WatershedShouldMerge( lab, grey[ offsetGrey + neighborOffsetsGrey[ jj ]], regions, maxDepth, maxSize )) {
                  ++realRegionCount;
               }
            }
            LabelType lab = neighborLabels.Label( 0 );
            if( realRegionCount < 2 ) {
               // At most one is a "real" region: merge all
               for( dip::uint jj = 1; jj < neighborLabels.Size(); ++jj ) {
                  regions.MergeRegions( lab, neighborLabels.Label( jj ), lowFirst );
               }
               labels[ offsetLabels ] = lab;
               ++( regions[ lab ].size );
            } else {
               // Else don't merge, set as watershed label
               labels[ offsetLabels ] = WATERSHED_LABEL;
            }
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
         if( lab1 == WATERSHED_LABEL ) {
            *it = 0;
         } else if( lab1 > 0 ) {
            LabelType lab2 = regions[ lab1 ].mapped;
            if( lab1 != lab2 ) {
               *it = lab2;
            }
         }
      } while( ++it );
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
   DIP_THROW_IF( !c_seeds.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );    // TODO: If seeds is binary, label it.
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( inSizes != c_seeds.Sizes(), E::SIZES_DONT_MATCH );
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
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( inSizes, Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( inSizes );
      DIP_END_STACK_TRACE
   }

   // What is the largest label?
   // TODO: If c_seeds was binary and we labeled it, we already have this value!
   auto m = GetMaximumAndMinimum( c_seeds, c_mask );
   dip::uint numlabs = static_cast< dip::uint >( m.Maximum() );

   // Prepare output image
   Convert( c_seeds, out, DT_LABEL );

   // Create array with offsets to neighbours
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsetsIn = neighborList.ComputeOffsets( in.Strides() );
   IntegerArray neighborOffsetsOut = neighborList.ComputeOffsets( out.Strides() );
   IntegerArray neighborOffsetsMask;
   if( mask.IsForged() ) {
      neighborOffsetsMask = neighborList.ComputeOffsets( mask.Strides());
   }

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__SeededWatershed, ( in, mask, out, neighborOffsetsIn, neighborOffsetsMask, neighborOffsetsOut, neighborList, numlabs, maxDepth, maxSize, lowFirst, binaryOutput ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into watershed lines
      Equal( out, WATERSHED_LABEL, out );
   }
}

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
      SeededWatershed( in, seeds, mask, out, connectivity, maxDepth, maxSize, flags );
   } else {
      FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, flags, FastWatershedOperation::WATERSHED );
   }
}

void LocalMinima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      String const& output
) {
   FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, { output, "low first" }, FastWatershedOperation::EXTREMA );
}

void LocalMaxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      dfloat maxDepth,
      dip::uint maxSize,
      String const& output
) {
   FastWatershed( in, mask, out, connectivity, maxDepth, maxSize, { output, "high first" }, FastWatershedOperation::EXTREMA );
}

} // namespace dip
