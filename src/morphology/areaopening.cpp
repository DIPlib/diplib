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
#include "diplib/iterators.h"
#include "diplib/overload.h"
#include "offsets.h"
#include "label_image_tools.h"

namespace dip {

namespace {

/*
The code below is very similar to that for FastWatershed, but instead of recording the value of the local
minimum/maximum in each watershed basin, we record the grey value at which the size criterion was met.
This value can then be used to paint all pixels that are larger/smaller within the basin.
*/

template< typename TPI >
class AreaOpenRegionList {
   public:
      AreaOpenRegionList() {
         region.reserve( 1000 );
         region.push_back( { 0, 0, 0 } ); // This element will not be used.
      }
      LabelType RootLabel( LabelType index ) const {
         if( region[ index ].parent == 0 ) {
            return index;
         } else {
            // Note we update the parent node here to point directly to the root.
            return region[ index ].parent = RootLabel( region[ index ].parent );
         }
      }
      LabelType Create( TPI value ) {
         if( region.size() > std::numeric_limits< LabelType >::max() ) {
            // TODO: remap?
            DIP_THROW( "Cannot create more regions!" );
         }
         LabelType index = static_cast< LabelType >( region.size() );
         region.push_back( { 1, value, 0 } );
         return index;
      }
      void Merge( LabelType label, LabelType other, bool lowFirst ) {
         label = RootLabel( label );
         other = RootLabel( other );
         region[ label ].lowest = lowFirst ? std::min( region[ label ].lowest, region[ other ].lowest )
                                                 : std::max( region[ label ].lowest, region[ other ].lowest );
         region[ label ].size += region[ other ].size;
         region[ other ].parent = label;
      }
      dip::uint Size( LabelType index ) const { return region[ RootLabel( index ) ].size; }
      TPI Lowest( LabelType index ) const { return region[ RootLabel( index ) ].lowest; }
      void AddPixel( LabelType index, TPI value, dip::uint filterSize ) {
         index = RootLabel( index );
         if( region[ index ].size < filterSize ) {
            ++( region[ index ].size );
            region[ index ].lowest = value;
         }
      }
      void AddSize( LabelType label, LabelType other ) {
         label = RootLabel( label );
         other = RootLabel( other );
         region[ label ].size += region[ other ].size;
      }
   private:
      struct Region {
         dip::uint size;
         TPI lowest;
         mutable LabelType parent; // index to the parent, which should be the label of this region. A value of 0 indicates the root.
      };
      std::vector< Region > region;
};

template< typename TPI >
void dip__AreaOpening(
      Image& c_grey,
      Image& c_labels,
      std::vector< dip::sint > const& offsets,
      IntegerArray const& neighborOffsets,
      NeighborList const& neighborList,
      dip::uint filterSize,
      bool lowFirst
) {
   TPI* grey = static_cast< TPI* >( c_grey.Origin() );
   LabelType* labels = static_cast< LabelType* >( c_labels.Origin() );

   AreaOpenRegionList< TPI > regions;
   NeighborLabels neighborLabels;

   //dip::uint nNeigh = neighborOffsets.size();
   //UnsignedArray const& imsz = c_grey.Sizes();
   //auto coordComp = c_grey.OffsetToCoordinatesComputer();

   // Process first pixel
   labels[ offsets[ 0 ]] = regions.Create( grey[ offsets[ 0 ]] );

   // Process other pixels
   for( dip::uint ii = 1; ii < offsets.size(); ++ii ) {
      dip::sint offset = offsets[ ii ];
      if( lowFirst ? PixelIsInfinity( grey[ offset ] ) : PixelIsMinusInfinity( grey[ offset ] )) {
         break; // we're done
      }
      neighborLabels.Reset();
      for( auto o : neighborOffsets ) {
         neighborLabels.Push( regions.RootLabel( labels[ offset + o ] ));
      }
      //auto nit = neighborList.begin();
      //for( dip::uint jj = 0; jj < nNeigh; ++jj, ++nit ) {
      //   dip::sint oo = offset + neighborOffsets[ jj ];
      //   if( nit.IsInImage( coordComp( oo ), imsz ) ) {
      //      neighborLabels.Push( regions.RootLabel( labels[ oo ] ));
      //   }
      //}
      switch( neighborLabels.Size() ) {
         case 0:
            // Not touching a label: new label
            labels[ offset ] = regions.Create( grey[ offset ] );
            break;
         case 1: {
            // Touching a single label: grow
            LabelType lab = neighborLabels.Label( 0 );
            labels[ offset ] = lab;
            regions.AddPixel( lab, grey[ offset ], filterSize );
            break;
         }
         default: {
            // Touching two or more labels
            // Find a small region, if it exists
            LabelType lab;
            for( dip::uint jj = 0; jj < neighborLabels.Size(); ++jj ) {
               lab = neighborLabels.Label( jj );
               if( regions.Size( lab ) < filterSize ) {
                  break;
               }
            }
            // If there was a small region, assign this pixel to it, then combine information from the other regions
            if( regions.Size( lab ) < filterSize ) {
               labels[ offset ] = lab;
               regions.AddPixel( lab, grey[ offset ], filterSize );
               // This region is small, let's merge all other small regions into it, and increase the size
               // with that of the large regions as well.
               for( dip::uint jj = 0; jj < neighborLabels.Size(); ++jj ) {
                  LabelType lab2 = neighborLabels.Label( jj );
                  if( lab != lab2 ) {
                     if( regions.Size( lab2 ) < filterSize ) {
                        // A small neighboring region should be merged in
                        regions.Merge( lab, lab2, lowFirst );
                     } else {
                        // A large neighboring region should lend its size to the small regions
                        regions.AddSize( lab, lab2 );
                     }
                  }
               }
            } else {
               // There were no small regions, we just need to assign this pixel to any one region and we're done
               // (increasing the size of the large regions is futile)
               labels[ offset ] = lab;
            }
            break;
         }
      }
   }
   JointImageIterator< TPI, LabelType > it( { c_grey, c_labels } );
   if( lowFirst ) {
      do {
         LabelType lab = it.template Sample< 1 >();
         if( lab > 0 ) {
            TPI v = regions.Lowest( lab );
            if( it.template Sample< 0 >() < v ) {
               it.template Sample< 0 >() = v;
            }
         }
      } while( ++it );
   } else {
      do {
         LabelType lab = it.template Sample< 1 >();
         if( lab > 0 ) {
            TPI v = regions.Lowest( lab );
            if( it.template Sample< 0 >() > v ) {
               it.template Sample< 0 >() = v;
            }
         }
      } while( ++it );
   }
}

} // namespace

void AreaOpening(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint filterSize,
      dip::uint connectivity,
      String const& polarity
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = c_in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF(( connectivity < 1 ) || ( connectivity > nDims ), E::ILLEGAL_CONNECTIVITY );
   bool lowFirst;
   DIP_START_STACK_TRACE
      lowFirst = BooleanFromString( polarity, "closing", "opening" );
   DIP_END_STACK_TRACE

   // Add a 1-pixel boundary around the input image
   Image grey;
   ExtendImageLowLevel( c_in, grey, { 1 }, { lowFirst ? BoundaryCondition::ADD_MAX_VALUE : BoundaryCondition::ADD_MIN_VALUE }, {} );

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
      UnsignedArray inSizes = c_in.Sizes();
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
   SortOffsets( grey, offsets, lowFirst );

   // Create array with offsets to neighbors
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighbors.ComputeOffsets( grey.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__AreaOpening, ( grey, labels, offsets, neighborOffsets, neighbors, filterSize, lowFirst ), grey.DataType() );

   // Copy result to output
   grey = grey.Crop( c_in.Sizes() );
   PixelSize pixelSize = c_in.PixelSize();
   out.Copy( grey );
   out.SetPixelSize( pixelSize );
}

} // namespace dip
