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

#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/binary.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"
#include "diplib/math.h"
#include "diplib/neighborlist.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
struct Qitem {
   TPI value;              // pixel value - used for sorting
   dip::sint offset;       // offset
};
template< typename TPI >
bool QitemComparator_LowFirst( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return a.value > b.value;
}
template< typename TPI >
bool QitemComparator_HighFirst ( Qitem< TPI > const& a, Qitem< TPI > const& b ) {
   return a.value < b.value;
}

// The `flag` binary image is used to store the following flags:
constexpr uint8 PROCESSED_MASK = 1u;   // indicates that the pixel has been propagated from in the 3rd pass
constexpr uint8 BORDER_MASK = 2u;      // indicates that the pixel is on the image border
inline bool isProcessed( uint8 const flag ) {
   return flag & PROCESSED_MASK;
}
inline bool isBorder( uint8 const flag ) {
   return flag & BORDER_MASK;
}
inline void markProcessed( uint8& flag ) {
   flag |= PROCESSED_MASK;
}

template< typename TPI >
void MorphologicalReconstructionInternal(
      Image const& in_img,
      Image& out_img,
      Image& flag_img,
      IntegerArray const& neighborOffsets,
      NeighborList const& neighborList,
      bool dilation
) {
   auto QitemComparator = dilation ? QitemComparator_HighFirst< TPI > : QitemComparator_LowFirst< TPI >;
   std::priority_queue< Qitem< TPI >, std::vector< Qitem< TPI >>, decltype( QitemComparator ) > Q( QitemComparator );

   dip::uint nNeigh = neighborList.Size();
   UnsignedArray const& imsz = in_img.Sizes();
   NeighborList backwardNeighbors = neighborList.SelectBackward();

   TPI* in = static_cast< TPI* >( in_img.Origin() );
   TPI* out = static_cast< TPI* >( out_img.Origin() );
   uint8* flag = static_cast< uint8* >( flag_img.Origin() ); // It's binary, but we use other bit planes too.

   // Step 1: Forward raster pass, propagate values forward (to the right and down)
   {
      IntegerArray backwardOffsets = backwardNeighbors.ComputeOffsets( out_img.Strides() );
      ImageIterator< TPI > it( { out_img } );
      do {
         dip::sint offset = it.Offset();
         TPI val = *it;
         if( isBorder( flag[ offset ] )) {
            if( dilation ) {
               for( dip::uint ii = 0; ii < backwardOffsets.size(); ++ii ) {
                  if( neighborList.IsInImage( ii, it.Coordinates(), imsz )) {
                     val = std::max( val, it.Pointer()[ backwardOffsets[ ii ]] );
                  }
               }
               val = std::min( val, in[ offset ] );
            } else {
               for( dip::uint ii = 0; ii < backwardOffsets.size(); ++ii ) {
                  if( neighborList.IsInImage( ii, it.Coordinates(), imsz )) {
                     val = std::min( val, it.Pointer()[ backwardOffsets[ ii ]] );
                  }
               }
               val = std::max( val, in[ offset ] );
            }
            if( *it != val ) {
               *it = val;
            }
         } else {
            if( dilation ) {
               for( auto n : backwardOffsets ) {
                  val = std::max( val, it.Pointer()[ n ] );
               }
               val = std::min( val, in[ offset ] );
            } else {
               for( auto n : backwardOffsets ) {
                  val = std::min( val, it.Pointer()[ n ] );
               }
               val = std::max( val, in[ offset ] );
            }
            if( *it != val ) {
               *it = val;
            }
         }
      } while( ++it );
   }

   // Step 2: Backward raster pass, propagate values backward (to the left and up), and enqueue pixels where we could
   //         propagate from in Step 3
   {
      Image out_img_mirrored = out_img.QuickCopy();
      out_img_mirrored.Mirror(); // a forward raster scan in a mirrored image is a backward raster scan in the original image
      IntegerArray backwardOffsets = backwardNeighbors.ComputeOffsets( out_img_mirrored.Strides() );
      ImageIterator< TPI > it( { out_img_mirrored } );
      do {
         dip::sint offset = it.Pointer() - out; // The offset in the original image, so we can use it to index into `flag`
         TPI val = *it;
         TPI maxNeighborValue = std::numeric_limits< TPI >::lowest();
         TPI minNeighborValue = std::numeric_limits< TPI >::max();
         if( isBorder( flag[ offset ] )) {
            for( dip::uint ii = 0; ii < backwardOffsets.size(); ++ii ) {
               if( neighborList.IsInImage( ii, it.Coordinates(), imsz )) {
                  TPI v = it.Pointer()[ backwardOffsets[ ii ]];
                  maxNeighborValue = std::max( maxNeighborValue, v );
                  minNeighborValue = std::min( minNeighborValue, v );
               }
            }
         } else {
            for( auto n : backwardOffsets ) {
               TPI v = it.Pointer()[ n ];
               maxNeighborValue = std::max( maxNeighborValue, v );
               minNeighborValue = std::min( minNeighborValue, v );
            }
         }
         if( dilation ) {
            val = std::max( val, maxNeighborValue );
            val = std::min( val, in[ offset ] );
         } else {
            val = std::min( val, minNeighborValue );
            val = std::max( val, in[ offset ] );
         }
         if( *it != val ) {
            *it = val;
            // Enqueue only if pixels in the backward direction might be propagated into (the forward pixels we'll
            // be propagating into later in this raster scan).
            if( dilation ? ( minNeighborValue < val ) : ( maxNeighborValue > val )) {
               Q.push( Qitem< TPI >{ val, offset } );
               // TODO: we are still enqueueing too many elements, we have not checked to see if the neighbor can be incremented
            }
         }
      } while( ++it );
   }

   // Step 3: Priority queue pass, propagate values in every direction from the pixels on the queue.
   auto coordinatesComputer = out_img.OffsetToCoordinatesComputer();
   BooleanArray skipar( nNeigh );
   //dip::uint nDoubleEnqueue = 0;
   while( !Q.empty() ) {
      dip::sint offset = Q.top().offset;
      Q.pop();
      if( isProcessed( flag[ offset ] )) {
         //++nDoubleEnqueue;
         // Note that Step 3 doesn't cause double enqueues, it is Step 2 which enqueues some items it shouldn't.
         continue;
      }
      // Compute coordinates if we're a border pixel
      UnsignedArray coords;
      bool onBorder = isBorder( flag[ offset ] );
      if( onBorder ) {
         coords = coordinatesComputer( offset );
      }
      // Iterate over all neighbors
      for( dip::uint jj = 0; jj < nNeigh; ++jj ) {
         if( !onBorder || neighborList.IsInImage( jj, coords, imsz )) { // test IsInImage only for border pixels
            // Propagate this pixel's value to its unfinished neighbors
            dip::sint nOffset = offset + neighborOffsets[ jj ];
            TPI newval = in[ nOffset ];
            if( dilation ) {
               newval = std::min( newval, out[ offset ] );
               if( out[ nOffset ] < newval ) {
                  out[ nOffset ] = newval;
                  // Add the updated neighbors to the heap
                  Q.push( Qitem< TPI >{ newval, nOffset } );
               }
            } else {
               newval = std::max( newval, out[ offset ] );
               if( out[ nOffset ] > newval ) {
                  out[ nOffset ] = newval;
                  // Add the updated neighbors to the heap
                  Q.push( Qitem< TPI >{ newval, nOffset } );
               }
            }
         }
      }
      // Mark this pixels as processed
      markProcessed( flag[ offset ] );
   }
   //std::cout << "nDoubleEnqueue = " << nDoubleEnqueue << '\n';
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
   UnsignedArray const& inSizes = c_in.Sizes();
   dip::uint nDims = inSizes.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( inSizes != c_marker.Sizes(), E::SIZES_DONT_MATCH );
   DIP_THROW_IF( connectivity > nDims, E::ILLEGAL_CONNECTIVITY );
   bool dilation{};
   DIP_STACK_TRACE_THIS( dilation = BooleanFromString( direction, S::DILATION, S::EROSION ));

   if( c_in.DataType().IsBinary() && c_marker.DataType().IsBinary() ) {
      if( dilation ) {
         DIP_STACK_TRACE_THIS( dip::BinaryPropagation( c_marker, c_in, out, static_cast< dip::sint >( connectivity ), 0, S::BACKGROUND ));
      } else {
         DIP_STACK_TRACE_THIS( dip::BinaryPropagation( ~c_marker, ~c_in, out, static_cast< dip::sint >( connectivity ), 0, S::BACKGROUND ));
         dip::Not( out, out );
      }
      return;
   }

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   Image marker = c_marker.QuickCopy();
   PixelSize pixelSize = c_in.HasPixelSize() ? c_in.PixelSize() : c_marker.PixelSize();

   // Prepare output image
   // We need `out`, `in` and `flag` to have the same strides. This might require an extra copy
   // if we can't do this naturally.
   if( out.Aliases( in )) {
      // We can work in-place if c_marker and out are the same image, but c_in must be separate from out.
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   if( in.HasContiguousData() && ( in.Strides() != out.Strides() )) {
      // Attempt to use same strides from `in` when reforging `out`.
      DIP_STACK_TRACE_THIS( out.Strip() );
      out.SetStrides( in.Strides() );
   }
   if( out.IsForged() && !out.HasContiguousData() ) {
      // If out doesn't have contiguous data, we must reforge it, otherwise we won't be able to allocate another image with the same strides.
      DIP_STACK_TRACE_THIS( out.Strip() );
   }
   out.ReForge( in );
   // If strides still don't match, we need to make a copy of `in` with strides matching `out`.
   if( in.Strides() != out.Strides() ) {
      Image tmp;
      tmp.SetStrides( out.Strides() );
      tmp.SetExternalInterface( out.ExternalInterface() ); // If there's an external interface for out, using it should give us the same strides here.
      tmp.ReForge( in );
      DIP_THROW_IF( tmp.Strides() != out.Strides(), "Couldn't allocate an intermediate image (copy of in) with the same strides as out" );
      tmp.Copy( in );
      in.swap( tmp );
   }

   // Copy `marker` into `out`
   DIP_STACK_TRACE_THIS( Convert( marker, out, out.DataType() ));
   DIP_STACK_TRACE_THIS( dilation ? Infimum( in, out, out ) : Supremum( in, out, out ));

   // Prepare intermediate image. This one must also have matching strides
   Image flag;
   flag.SetStrides( out.Strides() );
   flag.SetExternalInterface( out.ExternalInterface() );
   flag.ReForge( in, DT_UINT8 );
   DIP_THROW_IF( flag.Strides() != out.Strides(), "Couldn't allocate an intermediate image (flag) with the same strides as out" );
   flag.Fill( 0 );
   SetBorder( flag, { BORDER_MASK }, { 1 } );

   // Reorder dimensions to improve iteration -- this has no effect on this algorithm, except to speed it up.
   Image fout = out.QuickCopy(); // we don't want to change dimension order of the output image
   in.StandardizeStrides();
   fout.StandardizeStrides();
   flag.StandardizeStrides(); // These three function calls do the same computations internally. It's not easy to rewrite things to avoid this.
   DIP_ASSERT( in.Strides() == fout.Strides() );
   DIP_ASSERT( in.Strides() == flag.Strides() );

   // Create array with offsets to neighbors
   NeighborList neighborList( { Metric::TypeCode::CONNECTED, connectivity }, nDims );
   IntegerArray neighborOffsets = neighborList.ComputeOffsets( fout.Strides() );

   // Do the data-type-dependent thing
   DIP_OVL_CALL_REAL( MorphologicalReconstructionInternal,
                            ( in, fout, flag, neighborOffsets, neighborList, dilation ),
                            in.DataType() );
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
   bool dilation{};
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
