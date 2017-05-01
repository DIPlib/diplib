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

template< typename TPI >
struct Qitem {
   TPI value;              // pixel value - used for sorting */
   dip::sint offset;       // offset into `done` image
};
template< typename TPI >
struct QitemComparator {
   virtual bool operator()( Qitem< TPI > const&, Qitem< TPI > const& ) { return false; }; // just to silence the linker
};
template< typename TPI >
struct QitemComparator_LowFirst : public QitemComparator< TPI > {
   virtual bool operator()( Qitem< TPI > const& a, Qitem< TPI > const& b ) override {
      return a.value > b.value;
   }
};
template< typename TPI >
struct QitemComparator_HighFirst : public QitemComparator< TPI > {
   virtual bool operator()( Qitem< TPI > const& a, Qitem< TPI > const& b ) override {
      return a.value < b.value;
   }
};

template< typename TPI >
void dip__MorphologicalReconstruction(
      Image const& c_in,
      Image& c_out,
      Image& c_done,
      IntegerArray const& neighborOffsetsIn,
      IntegerArray const& neighborOffsetsOut,
      IntegerArray const& neighborOffsetsDone,
      NeighborList const& neighborList,
      Image const& c_minval,
      bool dilation
) {
   QitemComparator< TPI > comparator;
   if( dilation ) {
      comparator = QitemComparator_HighFirst< TPI >();
   } else {
      comparator = QitemComparator_LowFirst< TPI >();
   }
   std::priority_queue< Qitem< TPI >, std::vector< Qitem< TPI >>, QitemComparator< TPI >> Q( comparator );

   dip::uint nNeigh = neighborList.Size();
   UnsignedArray const& imsz = c_in.Sizes();

   TPI minval = *static_cast< TPI const* >( c_minval.Origin() );

   // Walk over the entire image & put all the pixels larger than minval on the heap.
   // Also set done=DIP_FALSE for all pixels and reduce the value of c_out where c_out > c_in.
   JointImageIterator< TPI, TPI, bin > it( { c_in, c_out, c_done } );
   if( dilation ) {
      do {
         it.Out() = std::min( it.Out(), it.In() );
         if( it.Out() > minval ) {
            Q.push( Qitem< TPI >{ it.Out(), it.template Offset< 2 >() } ); // offset pushed is that of `done`, so we can test it fast.
         }
         it.template Sample< 2 >() = false;
      } while( ++it );
   } else {
      do {
         it.Out() = std::max( it.Out(), it.In() );
         if( it.Out() < minval ) {
            Q.push( Qitem< TPI >{ it.Out(), it.template Offset< 2 >() } ); // offset pushed is that of `done`, so we can test it fast.
         }
         it.template Sample< 2 >() = false;
      } while( ++it );
   }

   // Start processing pixels
   TPI* in = static_cast< TPI* >( c_in.Origin() );
   TPI* out = static_cast< TPI* >( c_out.Origin() );
   bin* done = static_cast< bin* >( c_done.Origin() );
   auto coordinatesComputer = c_done.OffsetToCoordinatesComputer();
   BooleanArray skipar( nNeigh );
   while( !Q.empty() ) {
      dip::sint offsetDone = Q.top().offset;
      Q.pop();
      if( !done[ offsetDone ] ) {
         UnsignedArray coords = coordinatesComputer( offsetDone );
         dip::sint offsetIn = c_in.Offset( coords );
         dip::sint offsetOut = c_out.Offset( coords );
         // Iterate over all neighbors
         auto lit = neighborList.begin();
         for( dip::uint jj = 0; jj < nNeigh; ++jj, ++lit ) {
            if( lit.IsInImage( coords, imsz )) {
               // Propagate this pixel's value to its unfinished neighbours
               if( !done[ offsetDone + neighborOffsetsDone[ jj ]] ) {
                  TPI newval = in[ offsetIn + neighborOffsetsIn[ jj ]];
                  if( dilation ) {
                     newval = std::min( newval, out[ offsetOut ] );
                     if( out[ offsetOut + neighborOffsetsOut[ jj ]] < newval ) {
                        out[ offsetOut + neighborOffsetsOut[ jj ]] = newval;
                        // Add the updated neighbours to the heap
                        Q.push( Qitem< TPI >{ newval, offsetDone + neighborOffsetsDone[ jj ] } );
                     }
                  } else {
                     newval = std::max( newval, out[ offsetOut ] );
                     if( out[ offsetOut + neighborOffsetsOut[ jj ]] > newval ) {
                        out[ offsetOut + neighborOffsetsOut[ jj ]] = newval;
                        // Add the updated neighbours to the heap
                        Q.push( Qitem< TPI >{ newval, offsetDone + neighborOffsetsDone[ jj ] } );
                     }
                  }
               }
            }
         }
         // Mark this pixels done
         done[ offsetDone ] = true;
      }
   }
}

} // namespace

void MorphologicalReconstruction (
      Image const& c_marker,
      Image const& c_in, // grey-value mask
      Image& out,
      dip::uint connectivity,
      String direction // "dilataion" / "erosion"
) {
   // Check input
   DIP_THROW_IF( !c_marker.IsForged() || !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_marker.IsScalar() || !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( inSizes != c_marker.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF(( connectivity < 1 ) || ( connectivity > nDims ), E::ILLEGAL_CONNECTIVITY );
   bool dilation = direction == "dilation";

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();

   // Prepare output image
   Convert( c_marker, out, in.DataType() );
   Image minval = Minimum( out ); // same data type as `out`

   // Intermediate image
   Image done( out.Sizes(), 1, DT_BIN );

   // Create array with offsets to neighbours
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsetsIn = neighborList.ComputeOffsets( in.Strides() );
   IntegerArray neighborOffsetsOut = neighborList.ComputeOffsets( out.Strides() );
   IntegerArray neighborOffsetsDone = neighborList.ComputeOffsets( done.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_NONCOMPLEX( dip__MorphologicalReconstruction, ( in, out, done,
         neighborOffsetsIn, neighborOffsetsOut, neighborOffsetsDone, neighborList,
         minval, dilation ), in.DataType() );
}

} // namespace dip
