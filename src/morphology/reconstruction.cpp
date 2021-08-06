/*
 * DIPlib 3.0
 * This file contains the morphological reconstruction and related functions.
 *
 * (c)2017-2021, Cris Luengo.
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
#include "diplib/morphology.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/neighborlist.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"
#include "diplib/border.h"

namespace dip {

namespace {

template< typename TPI >
struct Qitem {
   TPI value;              // pixel value - used for sorting
   dip::sint offset;       // offset into `done` image
};
template< typename TPI >
bool QitemComparator_LowFirst( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return a.value > b.value;
}
template< typename TPI >
bool QitemComparator_HighFirst ( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return a.value < b.value;
}

template< typename TPI >
void MorphologicalReconstructionInternal(
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
   auto QitemComparator = dilation ? QitemComparator_HighFirst< TPI > : QitemComparator_LowFirst< TPI >;
   std::priority_queue< Qitem< TPI >, std::vector< Qitem< TPI >>, decltype( QitemComparator ) > Q( QitemComparator );

   dip::uint nNeigh = neighborList.Size();
   UnsignedArray const& imsz = c_in.Sizes();

   TPI minval = *static_cast< TPI const* >( c_minval.Origin() );

   // Walk over the entire image & put all the pixels larger than minval on the heap.
   // TODO: Many of these heap items are not necessary, we will enqueue the same pixel multiple times until it
   //       is actually processed. We cannot update items in the queue, we can only drop them when popping and
   //       seeing that the pixel was already processed. If there was a cheap way to determine here if a pixel
   //       needs to be enqueued at this point or not, it might help speed up things a bit. We'd need to enqueue
   //       only pixels that:
   //         - have a neighbor that has a lower value in `out`, and doesn't have the lower bit of `done` set, and
   //         - don't have a neighbor with a higher value that will propagate into it.
   //       Not an easy test!
   JointImageIterator< bin, TPI  > it( { c_done, c_out } );
   do {
      if( it.Out() != minval ) {
         Q.push( Qitem< TPI >{ it.Out(), it.template Offset< 0 >() } ); // offset pushed is that of `done`, so we can test it fast.
         //std::cout << " - Pushed " << it.Out();
      }
   } while( ++it );

   // Start processing pixels
   TPI* in = static_cast< TPI* >( c_in.Origin() );
   TPI* out = static_cast< TPI* >( c_out.Origin() );
   uint8* done = static_cast< uint8* >( c_done.Origin() ); // It's binary, but we use other bit planes too.
   // To wit: the bottom bit is set if he pixel value will no longer change (set in caller, updated here)
   //         the 2nd bit is set if we already propagated out from this pixel
   //         the 3rd bit is set if this is a border pixel (set in caller)
   auto coordinatesComputer = c_done.OffsetToCoordinatesComputer();
   BooleanArray skipar( nNeigh );
   while( !Q.empty() ) {
      dip::sint offsetDone = Q.top().offset;
      Q.pop();
      //std::cout << " - Popped: " << done[ offsetDone ];
      if( done[ offsetDone ] & 2u ) {
         // 2nd bit is set if we already propagated out from this pixel
         continue;
      }
      UnsignedArray coords = coordinatesComputer( offsetDone );
      dip::sint offsetIn = c_in.Offset( coords );
      dip::sint offsetOut = c_out.Offset( coords );
      //std::cout << " - Popped " << offsetDone << " (" << out[ offsetOut ] << ")";
      // Iterate over all neighbors
      auto lit = neighborList.begin();
      for( dip::uint jj = 0; jj < nNeigh; ++jj, ++lit ) {
         if( !( done[ offsetDone ] & 4u ) || lit.IsInImage( coords, imsz )) { // test IsInImage only for border pixels
            // Propagate this pixel's value to its unfinished neighbours
            if( !( done[ offsetDone + neighborOffsetsDone[ jj ]] & 1u )) { // if the bottom bit is set, we don't need to propagate
               TPI newval = in[ offsetIn + neighborOffsetsIn[ jj ]];
               if( dilation ) {
                  newval = std::min( newval, out[ offsetOut ] );
                  if( out[ offsetOut + neighborOffsetsOut[ jj ]] < newval ) {
                     out[ offsetOut + neighborOffsetsOut[ jj ]] = newval;
                     // Add the updated neighbours to the heap
                     Q.push( Qitem< TPI >{ newval, offsetDone + neighborOffsetsDone[ jj ] } );
                     //std::cout << " - Pushed " << newval;
                  }
               } else {
                  newval = std::max( newval, out[ offsetOut ] );
                  if( out[ offsetOut + neighborOffsetsOut[ jj ]] > newval ) {
                     out[ offsetOut + neighborOffsetsOut[ jj ]] = newval;
                     // Add the updated neighbours to the heap
                     Q.push( Qitem< TPI >{ newval, offsetDone + neighborOffsetsDone[ jj ] } );
                     //std::cout << " - Pushed " << newval;
                  }
               }
            }
         }
      }
      // Mark this pixels done
      done[ offsetDone ] = 3; // bottom two bits both set
   }
}

} // namespace

void MorphologicalReconstruction (
      Image const& c_marker,
      Image const& c_in, // grey-value mask
      Image& out,
      dip::uint connectivity,
      String const& direction
) {
   // Check input
   DIP_THROW_IF( !c_marker.IsForged() || !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_marker.IsScalar() || !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( inSizes != c_marker.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool dilation;
   DIP_STACK_TRACE_THIS( dilation = BooleanFromString( direction, S::DILATION, S::EROSION ));

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   Image marker = c_marker.QuickCopy();
   PixelSize pixelSize = c_in.HasPixelSize() ? c_in.PixelSize() : c_marker.PixelSize();

   // Prepare output image
   if( out.Aliases( in )) {
      out.Strip(); // We can work in-place if c_marker and out are the same image, but c_in must be separate from out.
   }
   DIP_STACK_TRACE_THIS( Convert( marker, out, in.DataType() ));
   DIP_STACK_TRACE_THIS( dilation ? Infimum( in, out, out ) : Supremum( in, out, out ));
   Image minval = dilation ? Minimum( out ) : Maximum( out ); // same data type as `out`

   // Intermediate image
   Image done = in == out; // Sets the bottom bit of the dip::bin byte
   detail::ProcessBorders< bin >( done, []( bin* ptr, dip::sint ){ *reinterpret_cast< uint8* >( ptr ) += 4; } ); // Set the 3rd bit to indicate a border pixel

   // Create array with offsets to neighbours
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsetsIn = neighborList.ComputeOffsets( in.Strides() );
   IntegerArray neighborOffsetsOut = neighborList.ComputeOffsets( out.Strides() );
   IntegerArray neighborOffsetsDone = neighborList.ComputeOffsets( done.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_NONCOMPLEX( MorphologicalReconstructionInternal, ( in, out, done,
         neighborOffsetsIn, neighborOffsetsOut, neighborOffsetsDone, neighborList,
         minval, dilation ), in.DataType() );

   out.SetPixelSize( std::move( pixelSize ));
}

void LimitedMorphologicalReconstruction(
      Image const& marker,
      Image const& in,
      Image& out,
      dfloat maxDistance,
      dip::uint connectivity,
      String const& direction
) {
   DIP_THROW_IF( maxDistance < 1, E::INVALID_PARAMETER );
   bool dilation;
   DIP_STACK_TRACE_THIS( dilation = BooleanFromString( direction, S::DILATION, S::EROSION ));
   Image mask;
   if( dilation ) {
      DIP_STACK_TRACE_THIS( Dilation( marker, mask, { 2 * maxDistance, S::ELLIPTIC } ));
      Infimum( mask, in, mask );
   } else {
      DIP_STACK_TRACE_THIS( Erosion( marker, mask, { 2 * maxDistance, S::ELLIPTIC } ));
      Supremum( mask, in, mask );
   }
   DIP_STACK_TRACE_THIS( MorphologicalReconstruction( marker, mask, out, connectivity, direction ));
}

void ImposeMinima(
      Image const& in,
      Image const& marker,
      Image& out,
      dip::uint connectivity
) {
   DIP_THROW_IF( !in.IsForged() || !marker.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar() || !marker.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !marker.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   // The 'seed' image is the marker image, with the regions set to the min and the background set to the max
   Image seed = in.Similar();
   seed.Fill( Image::Sample::Maximum( seed.DataType() ));
   DIP_STACK_TRACE_THIS( seed.At( marker ) = Image::Sample::Minimum( seed.DataType() ));
   // We need to make sure the 'gray' image doesn't have local minima containing multiple minima in 'seed'. So we
   // add 1 in case of floating point images, and add 1 only to the minimal values for integer images.
   Image gray = in.Copy();
   if( gray.DataType().IsFloat() ) {
      // We can add 1 to 'gray', we're unlikely to overflow (though we're also unlikely to have any pixels at the minimum value, so this might not be necessary at all).
      gray += 1;
   } else {
      // If we add 1 to 'gray', we could overflow. Instead we compute max(gray,min+1).
      auto floor = Image::Sample::Minimum( seed.DataType() );
      floor += 1;
      DIP_STACK_TRACE_THIS( Supremum( gray, Image( floor ), gray ));
   }
   Infimum( gray, seed, gray );
   DIP_STACK_TRACE_THIS( MorphologicalReconstruction( seed, gray, out, connectivity, S::EROSION ));
}

} // namespace dip
