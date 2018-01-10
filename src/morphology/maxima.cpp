/*
 * DIPlib 3.0
 * This file contains the functions Maxima and Minima.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)2008, Cris Luengo.
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
#include "diplib/neighborlist.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

constexpr LabelType PIXEL_NOT_EXTREMUM = std::numeric_limits< LabelType >::max();
constexpr LabelType MAX_LABEL = PIXEL_NOT_EXTREMUM - 1;

struct Qitem {
   UnsignedArray coords;   // coordinates of the pixel // TODO: should we compute these from the offset instead?
   LabelType* pointer;     // pointer into output (labels) image
};

template< typename TPI >
void dip__Extrema(
      Image const& in,
      Image const& mask,
      Image out,
      IntegerArray const& neighborOffsetsIn,
      IntegerArray const& neighborOffsetsMask,
      IntegerArray const& neighborOffsetsOut,
      NeighborList const& neighborList,
      bool maxima
) {
   // Allocate LUT
   std::vector< bool >labmap( 1 ); // The first entry is not used.
   LabelType curlab = 0;

   // Prepare queue
   std::queue< Qitem > Q; // TODO: is this faster with a LIFO queue (std::vector)?

   // Loop over all image pixels
   dip::uint nNeigh = neighborOffsetsIn.size();
   UnsignedArray const& imsz = in.Sizes();

   JointImageIterator< TPI, LabelType, bin > it( { in, out, mask } );
   bool hasMask = mask.IsForged();
   do {
      if( !hasMask || it.template Sample< 2 >() ) {
         LabelType lab = it.Out();
         if( lab == 0 ) {
            // Does it satisfy: all its neighbours are of equal or lower value?
            TPI val = it.In();
            bool doit = true;
            UnsignedArray const& coords = it.Coordinates();
            auto nit = neighborList.begin();
            for( dip::uint jj = 0; jj < nNeigh; ++jj, ++nit ) {
               if( nit.IsInImage( coords, imsz ) ) {
                  if( !hasMask || *( it.template Pointer< 2 >() + neighborOffsetsMask[ jj ] )) {
                     if( maxima ? *( it.InPointer() + neighborOffsetsIn[ jj ] ) > val
                                : *( it.InPointer() + neighborOffsetsIn[ jj ] ) < val ) {
                        doit = false;
                        break;
                     }
                  }
               }
            }
            // If so, we need to visit all equal-value neighbours iteratively and mark them all with the same label
            if( doit ) {
               DIP_THROW_IF( curlab == MAX_LABEL, "Ran out of labels!" );
               curlab = curlab + 1;
               labmap.push_back( true );
               // Mark this pixel with the current label & push it on the stack
               it.Out() = curlab;
               Q.push( Qitem{ it.Coordinates(), it.OutPointer() } );
               // Iteratively process pixels on stack
               while( !Q.empty() ) {
                  UnsignedArray coordsi = Q.front().coords;
                  LabelType* optr = Q.front().pointer;
                  Q.pop();
                  TPI* iptr = static_cast< TPI* >( in.Pointer( coordsi ));
                  bin* mptr = hasMask ? static_cast< bin* >( mask.Pointer( coordsi )) : nullptr;
                  // Visit each neighbour pixel, push equal-valued ones on the stack, mark the label
                  // as bad if one of the neighbours has a higher value
                  nit = neighborList.begin();
                  for( dip::uint jj = 0; jj < nNeigh; ++jj, ++nit ) {
                     if( nit.IsInImage( coordsi, imsz ) ) {
                        if( !hasMask || *( mptr + neighborOffsetsMask[ jj ] )) {
                           LabelType* onptr = optr + neighborOffsetsOut[ jj ];
                           TPI* inptr = iptr + neighborOffsetsIn[ jj ];
                           if( maxima ? *inptr > val : *inptr < val ) { // This is not a local maximum!
                              labmap[ curlab ] = false;
                              // We're not marking this pixel as done, we'll visit it again later
                           } else if( *inptr == val ) {              // This is part of the same connected level set
                              if( *onptr == PIXEL_NOT_EXTREMUM ) {   // This is not a local maximum!
                                 labmap[ curlab ] = false;
                              } else if( *onptr == 0 ) {             // This one we hadn't seen before, push onto stack
                                 *onptr = curlab;
                                 UnsignedArray ncoords = coordsi;
                                 for( dip::uint ii = 0; ii < ncoords.size(); ++ii ) {
                                    ncoords[ ii ] += static_cast< dip::uint >( nit.Coordinates()[ ii ] ); // relying on two's complement arithmetic here!
                                 }
                                 Q.push( Qitem{ ncoords, onptr } );
                              }
                           } else {                                  // No need to visit this pixel again
                              *onptr = PIXEL_NOT_EXTREMUM;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   } while( ++it );
   // Relabel regions so labels are consecutive
   dip::uint nLabs = labmap.size();
   std::vector< LabelType >mapping( nLabs, 0 ); // Matching labmap
   curlab = 0;
   for( dip::uint ii = 0; ii < nLabs; ++ii ) {
      if( labmap[ ii ] ) {
         mapping[ ii ] = ++curlab;
      }
   }
   // Loop over all image pixels again, apply `mapping`.
   ImageIterator< LabelType > oit( out );
   oit.OptimizeAndFlatten();
   do {
      LabelType lab = *oit;
      *oit = lab == PIXEL_NOT_EXTREMUM ? 0 : mapping[ lab ];
   } while( ++oit );
}

void Extrema(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint connectivity,
      String const& output,
      bool maxima
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool binaryOutput;
   DIP_STACK_TRACE_THIS( binaryOutput = BooleanFromString( output, S::BINARY, S::LABELS ));

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

   // TODO: If there's no mask, find the minimum/maximum value in the image, and ignore pixels with that value.

   // Prepare label image
   out.ReForge( in, DT_LABEL );
   out.Fill( 0 );

   // Create array with offsets to neighbours
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsetsIn = neighborList.ComputeOffsets( in.Strides() );
   IntegerArray neighborOffsetsOut = neighborList.ComputeOffsets( out.Strides() );
   IntegerArray neighborOffsetsMask;
   if( mask.IsForged() ) {
      neighborOffsetsMask = neighborList.ComputeOffsets( mask.Strides());
   }

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( dip__Extrema, ( in, mask, out, neighborOffsetsIn, neighborOffsetsMask, neighborOffsetsOut, neighborList, maxima ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into foreground
      NotEqual( out, 0, out );
   }
}

} // namespace

void Minima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, mask, out, connectivity, output, false );
}

void Maxima(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, mask, out, connectivity, output, true );
}

} // namespace dip
