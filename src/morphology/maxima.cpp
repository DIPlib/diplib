/*
 * DIPlib 3.0
 * This file contains the functions Maxima and Minima.
 *
 * (c)2017-2018, Cris Luengo.
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
#include "diplib/union_find.h"
#include "diplib/framework.h"
#include "watershed_support.h"

namespace dip {

namespace {

using ExtremalRegionList = SimpleUnionFind< LabelType >;

template< typename TPI >
void ProcessNeighbor(
      TPI const* inPtr,
      LabelType const* outPtr,
      NeighborLabels& neighborLabels,
      ExtremalRegionList& regions,
      dip::sint neighborOffsetIn,
      dip::sint neighborOffsetOut,
      bool isBackwardNeighbor,
      bool maxima,
      bool& isExtremum
) {
   TPI nval = *( inPtr + neighborOffsetIn );
   if( isBackwardNeighbor && ( nval == *inPtr )) {
      LabelType nlab = regions.FindRoot( *( outPtr + neighborOffsetOut ));
      if( nlab == 0 ) {
         // A previously processed neighbor at the same level is not a local maximum, then neither is this one
         isExtremum = false;
      } else {
         neighborLabels.Push( nlab );
      }
   } else if( maxima ? nval > *inPtr : nval < *inPtr ) {
      isExtremum = false;
      // We don't break: let's examine all neighbors so we can cancel any labels that are not extrema
   }
}

void HandleLabels(
      LabelType* outPtr,
      NeighborLabels& neighborLabels,
      ExtremalRegionList& regions,
      bool isExtremum
) {
   if( isExtremum ) {
      // It is an extremum: label
      LabelType lab = 0;
      if( neighborLabels.Size() == 0 ) {
         // No labeled neighbors: create a new label
         lab = regions.Create();      // TODO: can throw if too many regions -- should compact instead, takes time but allows us to continue
      } else {
         // Some labeled neighbors: merge the labels
         auto labit = neighborLabels.begin();
         lab = *labit;
         ++labit;
         for( ; labit != neighborLabels.end(); ++labit ) {
            lab = regions.Union( lab, *labit );
         }
      }
      *outPtr = lab;
   } else {
      // It is not an extremum: if any of the neighbors is at the same level and labeled, cancel that label
      for( auto nlab : neighborLabels ) {
         regions.Union( nlab, 0 );
      }
      *outPtr = 0;
   }
}

template< typename TPI >
void ProcessPixelWithCheck(
      TPI const* inPtr,
      LabelType* outPtr,
      UnsignedArray const& coords,
      NeighborLabels& neighborLabels,
      ExtremalRegionList& regions,
      IntegerArray const& neighborOffsetsIn,
      IntegerArray const& neighborOffsetsOut,
      NeighborList const& neighborList,
      BooleanArray const& isBackwardNeighbor,
      UnsignedArray const& sizes,
      bool maxima
) {
   bool isExtremum = true;
   neighborLabels.Reset();
   auto nlIt = neighborList.begin();
   for( dip::uint ii = 0; ii < neighborList.Size(); ++ii, ++nlIt ) {
      if( nlIt.IsInImage( coords, sizes )) {
         ProcessNeighbor( inPtr, outPtr, neighborLabels, regions, neighborOffsetsIn[ ii ], neighborOffsetsOut[ ii ],
                          isBackwardNeighbor[ ii ], maxima, isExtremum );
      }
   }
   HandleLabels( outPtr, neighborLabels, regions, isExtremum );
}

template< typename TPI >
void ProcessPixel(
      TPI const* inPtr,
      LabelType* outPtr,
      NeighborLabels& neighborLabels,
      ExtremalRegionList& regions,
      IntegerArray const& neighborOffsetsIn,
      IntegerArray const& neighborOffsetsOut,
      std::vector< dip::uint > const& neighbors,
      BooleanArray const& isBackwardNeighbor,
      bool maxima
) {
   bool isExtremum = true;
   neighborLabels.Reset();
   for( dip::uint ii : neighbors ) {
      ProcessNeighbor( inPtr, outPtr, neighborLabels, regions, neighborOffsetsIn[ ii ], neighborOffsetsOut[ ii ],
                       isBackwardNeighbor[ ii ], maxima, isExtremum );
   }
   HandleLabels( outPtr, neighborLabels, regions, isExtremum );
}

template< typename TPI >
void ExtremaInternal(
      Image const& in,
      Image& out,
      IntegerArray const& neighborOffsetsIn,
      IntegerArray const& neighborOffsetsOut,
      NeighborList const& neighborList,
      BooleanArray const& isBackwardNeighbor,
      dip::uint procDim,
      bool maxima
) {
   // Allocate Union-Find data structure
   ExtremalRegionList regions;

   // Loop over all image pixels
   UnsignedArray const& imsz = in.Sizes();
   NeighborLabels neighborLabels;
   dip::sint inStride = in.Stride( procDim );
   dip::sint outStride = out.Stride( procDim );
   dip::uint lastPixel = imsz[ procDim ]  - 1;
   dip::sint endOffset = inStride * static_cast< dip::sint >( lastPixel );
   JointImageIterator< TPI, LabelType > it( { in, out }, procDim );
   do {

      // Find neighbors that are in-image for this image line
      UnsignedArray coords = it.Coordinates();

      // Loop over image line
      TPI* inPtr = it.InPointer();
      TPI* endPtr = inPtr + endOffset;
      LabelType* outPtr = it.OutPointer();

      // First pixel
      ProcessPixelWithCheck( inPtr, outPtr, coords, neighborLabels, regions, neighborOffsetsIn, neighborOffsetsOut,
                             neighborList, isBackwardNeighbor, imsz, maxima );
      inPtr += inStride;
      outPtr += outStride;

      // Body of image line
      coords[ procDim ] = 1;
      std::vector< dip::uint > neighbors;
      //std::cout << "coords = " << coords << ", neighbors = ";
      auto nlIt = neighborList.begin();
      for( dip::uint ii = 0; ii < neighborList.Size(); ++ii, ++nlIt ) {
         if( nlIt.IsInImage( coords, imsz )) {
            neighbors.push_back( ii );
            //std::cout << ii << " (" << isBackwardNeighbor[ii] << ") ";
         }
      }
      //std::cout << '\n';
      do {
         ProcessPixel( inPtr, outPtr, neighborLabels, regions, neighborOffsetsIn, neighborOffsetsOut,
                       neighbors, isBackwardNeighbor, maxima );
         inPtr += inStride;
         outPtr += outStride;
      } while( inPtr != endPtr );

      // Last pixel
      coords[ procDim ] = lastPixel;
      ProcessPixelWithCheck( inPtr, outPtr, coords, neighborLabels, regions, neighborOffsetsIn, neighborOffsetsOut,
                             neighborList, isBackwardNeighbor, imsz, maxima );

   } while( ++it );

   // Relabel regions so labels are consecutive and canceled labels are reset to 0
   regions.Relabel();
   ImageIterator< LabelType > oit( out );
   oit.OptimizeAndFlatten();
   do {
      *oit = regions.Label( *oit );
   } while( ++oit );
}

void Extrema(
      Image const& c_in,
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

   // Prepare label image
   out.ReForge( in, DT_LABEL );
   out.SetPixelSize( pixelSize );
   out.Fill( 0 );

   // Find processing dimension
   dip::uint procDim = Framework::OptimalProcessingDim( out );
   dip::uint length = out.Size( procDim );
   DIP_THROW_IF( length < 2, "Input image is too small" );

   // Create array with offsets to neighbours
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsetsIn = neighborList.ComputeOffsets( in.Strides() );
   IntegerArray neighborOffsetsOut = neighborList.ComputeOffsets( out.Strides() );

   // Find those neighbors that are processed earlier
   BooleanArray isBackwardNeighbor = neighborList.FindBackward( procDim );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( ExtremaInternal, ( in, out, neighborOffsetsIn, neighborOffsetsOut, neighborList, isBackwardNeighbor,
                                      procDim, maxima ), in.DataType() );

   if( binaryOutput ) {
      // Convert the labels into foreground
      NotEqual( out, 0, out );
   }
}

} // namespace

void Minima(
      Image const& in,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, out, connectivity, output, false );
}

void Maxima(
      Image const& in,
      Image& out,
      dip::uint connectivity,
      String const& output
) {
   Extrema( in, out, connectivity, output, true );
}

} // namespace dip
