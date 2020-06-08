/*
 * DIPlib 3.0
 * This file contains the various watershed implementations and related functions.
 *
 * (c)2008-2009, 2017-2018, Cris Luengo.
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
#include "diplib/generation.h"
#include "diplib/overload.h"

#include "watershed_support.h"

namespace dip {

namespace {

// Returns `true` if `coords` contains a direction that is close to that of 'direction'.
// If `coords` points at the origin, it always returns `false`. If `coords` and `direction`
// are identical, it also returns `false`.
//
// Of all the non-zero elements in `direction`, at least one must be identical in `coords`,
// the other elements can differ by one. This defines a 90 degree wedge in 2D, a 90 degree
// cone in 3D, and a similar set of angles in higher dimensions.
bool IsValidNeighbor(
      IntegerArray const& direction,
      IntegerArray const& coords
) {
   bool isAllZero = true;
   bool hasUnchanged = false;
   bool isIdentical = true;
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      isAllZero = isAllZero && ( coords[ ii ] == 0 );
      isIdentical = isIdentical && ( coords[ ii ] == direction[ ii ] );
      if( direction[ ii ] != 0 ) {
         dip::sint tmp = std::abs( direction[ ii ] - coords[ ii ] );
         if( tmp > 1 ) {
            return false;
         } else if( tmp == 0 ) {
            hasUnchanged = true;
         }
      }
   }
   return hasUnchanged && !isIdentical && !isAllZero;
}


// Creates a list of upstream and downstream neighbors depending on `direction`,
// which is an array with elements -1, 0 and 1.
void MakeNeighborLists(
      IntegerArray const& direction, // input
      IntegerArray const& stride,    // input
      IntegerArray& offsetUp,        // output
      IntegerArray& offsetDown       // output
) {
   dip::uint ndims = direction.size();
   // We put the neighbor given by "direction" first
   dip::sint ptrup = 0;
   dip::sint dist = 0;
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      ptrup += direction[ ii ] * stride[ ii ];
      dist += std::abs( direction[ ii ] );
   }
   offsetUp.clear();
   offsetDown.clear();
   offsetUp.push_back( ptrup );
   offsetDown.push_back( -ptrup );
   // Now we list all other neighbors
   IntegerArray coords( ndims, -1 );
   for( ;; ) {
      if( IsValidNeighbor( direction, coords )) {
         ptrup = dist = 0;
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            ptrup += coords[ ii ] * stride[ ii ];
            dist += std::abs( coords[ ii ] );
         }
         offsetUp.push_back( ptrup );
         offsetDown.push_back( -ptrup );
      }
      dip::uint ii = 0;
      for( ; ii < ndims; ++ii ) {
         ++( coords[ ii ] );
         if( coords[ ii ] <= 1 ) {
            break;
         }
         coords[ ii ] = -1;
      }
      if( ii == ndims ) {
         break;
      }
   }
}

//
// Recursive updating of upstream (or downstream) neighbors.
//

using PathLenType = uint16;
constexpr auto DT_PATHLEN = DT_UINT16;

constexpr uint8 PO_ACTIVE = 1;
constexpr uint8 PO_QUEUED = 2;
constexpr uint8 PO_CHANGED = 4;

using PixelQueue = std::queue< dip::sint >;

#if defined(__GNUG__) // GCC seems to thing that `uint8 |= uint8` needs a conversion warning just because the computation is performed with an int.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

void ConstrainedPropagateChanges(
      uint8* active,
      PathLenType* straight_length,
      PathLenType* other_length,     // other_length >= straight_length
      IntegerArray const& next,
      IntegerArray const& prev,
      dip::sint index,
      PixelQueue& queue,
      PixelQueue& changed
) {
   // This pixel's length is 0
   straight_length[ index ] = 0;
   other_length[ index ] = 0;
   // Enqueue the neighbors that are still active
   for( dip::uint jj = 0; jj < next.size(); ++jj ) {
      dip::sint ii = index + next[ jj ];
      if( active[ ii ] & PO_ACTIVE ) {
         active[ ii ] |= PO_QUEUED;
         queue.push( ii );
      }
   }
   // Process pixels in queue
   while( !queue.empty() ) {
      // Pop a pixel
      index = queue.front();
      queue.pop();
      uint8* aptr = active + index;
      *aptr &= ~PO_QUEUED; // Clear the queued bit
      // Update this pixel's lengths looking backwards
      dip::sint ii = index + prev[ 0 ];
      PathLenType len_o = other_length[ ii ];
      PathLenType len_s = len_o;
      for( dip::uint jj = 1; jj < prev.size(); ++jj ) {
         ii = index + prev[ jj ];
         if( straight_length[ ii ] > len_o ) {
            len_o = straight_length[ ii ];
         }
      }
      ++len_s;
      ++len_o;
      if( len_s < straight_length[ index ] ) {
         straight_length[ index ] = len_s;
         // Enqueue the neighbors that are still active
         for( dip::uint jj = 0; jj < next.size(); ++jj ) {
            ii = index + next[ jj ];
            if(( active[ ii ] & PO_ACTIVE ) && !( active[ ii ] & PO_QUEUED )) {
               active[ ii ] |= PO_QUEUED;
               queue.push( ii );
            }
         }
         // Put this one on the 'changed' list
         if( !( *aptr & PO_CHANGED )) {
            *aptr |= PO_CHANGED; // this is to make sure they're only pushed once
            changed.push( index );
         }
      }
      if( len_o < other_length[ index ] ) {
         other_length[ index ] = len_o;
         // Enqueue the straight neighbor if still active
         ii = index + next[ 0 ];
         if(( active[ ii ] & PO_ACTIVE ) && !( active[ ii ] & PO_QUEUED )) {
            active[ ii ] |= PO_QUEUED;
            queue.push( ii );
         }
         // Put this one on the 'changed' list
         if( !( *aptr & PO_CHANGED )) {
            *aptr |= PO_CHANGED; // this is to make sure they're only pushed once
            changed.push( index );
         }
      }
   }
}

void PropagateChanges(
      uint8* active,
      PathLenType* length,
      IntegerArray const& next,
      IntegerArray const& prev,
      dip::sint index,
      PixelQueue& queue,
      PixelQueue& changed
) {
   // This pixel's length is 0
   length[ index ] = 0;
   // Enqueue the neighbors that are still active
   for( dip::uint jj = 0; jj < next.size(); ++jj ) {
      dip::sint ii = index + next[ jj ];
      if( active[ ii ] & PO_ACTIVE ) {
         active[ ii ] |= PO_QUEUED; // this is to make sure they're only pushed once
         queue.push( ii );
      }
   }
   // Process pixels in queue
   while( !queue.empty() ) {
      // Pop a pixel
      index = queue.front();
      queue.pop();
      uint8* aptr = active + index;
      *aptr &= ~PO_QUEUED; // Clear the queued bit
      // Update this pixel's length looking backwards
      dip::sint ii = index + prev[ 0 ];
      PathLenType len = length[ ii ];
      for( dip::uint jj = 1; jj < prev.size(); ++jj ) {
         ii = index + prev[ jj ];
         if( length[ ii ] > len ) {
            len = length[ ii ];
         }
      }
      ++len;
      if( len < length[ index ] ) {
         length[ index ] = len;
         // Enqueue the neighbors that are still active
         for( dip::uint jj = 0; jj < next.size(); ++jj ) {
            ii = index + next[ jj ];
            if(( active[ ii ] & PO_ACTIVE ) && !( active[ ii ] & PO_QUEUED )) {
               active[ ii ] |= PO_QUEUED;
               queue.push( ii );
            }
         }
         // Put this one on the 'changed' list
         if( !( *aptr & PO_CHANGED )) {
            *aptr |= PO_CHANGED; // this is to make sure they're only pushed once
            changed.push( index );
         }
      }
   }
}

//
// Data-type dependent portion of the code: Iterate over all pixels of the image, in order of grey-value, and update.
//

template< typename TPI >
void ConstrainedPathOpeningInternal(
      Image& im_grey,                     // grey in & out
      Image& im_active,                   // temp: marks active pixels
      Image& im_slup,                     // temp: upstream length, straight
      Image& im_olup,                     // temp: upstream length, non-straight
      Image& im_sldn,                     // temp: downstream length, straight
      Image& im_oldn,                     // temp: downstream length, non-straight
      std::vector< dip::sint > offsets,   // array with offsets into images
      IntegerArray const& offsetUp,       // offsets to upstream neighbors
      IntegerArray const& offsetDown,     // offsets to upstream neighbors
      dip::uint length                    // param
) {
   TPI* grey = static_cast< TPI* >( im_grey.Origin() );
   uint8* active = static_cast< uint8* >( im_active.Origin() ); // It's `bin`, but uses 3 planes, so we read it as a `uint8` instead
   PathLenType* slup = static_cast< PathLenType* >( im_slup.Origin() );
   PathLenType* olup = static_cast< PathLenType* >( im_olup.Origin() );
   PathLenType* sldn = static_cast< PathLenType* >( im_sldn.Origin() );
   PathLenType* oldn = static_cast< PathLenType* >( im_oldn.Origin() );

   PixelQueue queue;
   PixelQueue changed;

   for( dip::uint jj = 0; jj < offsets.size(); ++jj ) {
      dip::sint offset = offsets[ jj ];
      if( !( active[ offset ] & PO_ACTIVE )) {
         continue;
      }
      // Propagate changes upstream
      ConstrainedPropagateChanges( active, slup, olup, offsetUp, offsetDown, offset, queue, changed );
      // Propagate changes downstream
      ConstrainedPropagateChanges( active, sldn, oldn, offsetDown, offsetUp, offset, queue, changed );
      // Go over changed pixels and update grey, active, etc.
      while( !changed.empty() ) {
         dip::sint index = changed.front(); // we can use `index` because all images have the same strides.
         changed.pop();
         uint8* aptr = active + index;
         *aptr &= ~PO_CHANGED;
         if(( static_cast< dip::uint >( slup[ index ] + oldn[ index ] ) < length + 1 ) &&
            ( static_cast< dip::uint >( olup[ index ] + sldn[ index ] ) < length + 1 )) {
            grey[ index ] = grey[ offset ];
            active[ index ] &= ~PO_ACTIVE;
            slup[ index ] = 0;
            olup[ index ] = 0;
            sldn[ index ] = 0;
            oldn[ index ] = 0;
         }
      }
      active[ offset ] &= ~PO_ACTIVE;
   }
}

template< typename TPI >
void PathOpeningInternal(
      Image& im_grey,                     // grey in & out
      Image& im_active,                   // temp: marks active pixels
      Image& im_lup,                      // temp: upstream length
      Image& im_ldn,                      // temp: downstream length
      std::vector< dip::sint > offsets,   // array with offsets into images
      IntegerArray const& offsetUp,       // offsets to upstream neighbors
      IntegerArray const& offsetDown,     // offsets to upstream neighbors
      dip::uint length                    // param
) {
   TPI* grey = static_cast< TPI* >( im_grey.Origin() );
   uint8* active = static_cast< uint8* >( im_active.Origin() ); // It's `bin`, but uses 3 planes, so we read it as a `uint8` instead
   PathLenType* lup = static_cast< PathLenType* >( im_lup.Origin() );
   PathLenType* ldn = static_cast< PathLenType* >( im_ldn.Origin() );

   PixelQueue queue;
   PixelQueue changed;

   for( dip::uint jj = 0; jj < offsets.size(); ++jj ) {
      dip::sint offset = offsets[ jj ];
      if( !( active[ offset ] & PO_ACTIVE )) {
         continue;
      }
      // Propagate changes upstream
      PropagateChanges( active, lup, offsetUp, offsetDown, offset, queue, changed );
      // Propagate changes downstream
      PropagateChanges( active, ldn, offsetDown, offsetUp, offset, queue, changed );
      // Go over changed pixels and update grey, active, etc.
      while( !changed.empty() ) {
         dip::sint index = changed.front(); // we can use `index` because all images have the same strides.
         changed.pop();
         uint8* aptr = active + index;
         *aptr &= ~PO_CHANGED;
         if( static_cast< dip::uint >( lup[ index ] + ldn[ index ] ) < length + 1 ) {
            grey[ index ] = grey[ offset ];
            active[ index ] &= ~PO_ACTIVE;
            lup[ index ] = 0;
            ldn[ index ] = 0;
         }
      }
      active[ offset ] &= ~PO_ACTIVE;
   }
}

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

void ParsePathMode(
      String const& polarity,
      StringSet const& mode,
      bool& opening,
      bool& constrained,
      bool& robust
) {
   opening = BooleanFromString( polarity, S::OPENING, S::CLOSING );
   constrained = false;
   robust = false;
   for( auto& m : mode ) {
      if( m == S::CONSTRAINED ) {
         constrained = true;
      } else if( m == S::UNCONSTRAINED ) {
         constrained = false;
      } else if( m == S::ROBUST ) {
         robust = true;
      } else {
         DIP_THROW_INVALID_FLAG( m );
      }
   }
}

} // namespace

void PathOpening(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      dip::uint length,
      String const& polarity,
      StringSet const& mode
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( c_in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint ndims = c_in.Dimensionality();
   DIP_THROW_IF( ndims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_THROW_IF( c_in.Size( ii ) < 3, "Input image is too small." );
   }

   bool opening, constrained, robust;
   DIP_STACK_TRACE_THIS( ParsePathMode( polarity, mode, opening, constrained, robust ));

   // Make simplified copy of input image header so we can modify it at will.
   // This also effectively separates input and output images. They still point
   // at the same data, but we can strip the output image without destroying
   // the input pixel data.
   Image in = c_in.QuickCopy();
   PixelSize pixelSize = c_in.PixelSize();
   in.ResetExternalInterface(); // Assure we can do `in = ...` without copying data into our input image.

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
   }

   DIP_THROW_IF(( length < 2 ) || ( length > std::numeric_limits< PathLenType >::max() ), E::PARAMETER_OUT_OF_RANGE );

   // Prepare output image
   if( out.Aliases( in ) || out.Aliases( mask ) || ( !out.IsProtected() && ( out.DataType() != in.DataType() ))) {
      out.Strip();
      // It will be forged later when we do the first copy.
   }

   // First part of robustness
   // Here we create a new pixel buffer for `in`, separate from the true `in`.
   Image orig_in;
   if( robust ) {
      orig_in = in.QuickCopy(); // Save a reference to the input data buffer (note that `c_in` could lose that reference if it is the same object as `out`)
      StructuringElement robustSE{ 2, S::RECTANGULAR };
      if( opening ) {
         in = Dilation( in, robustSE );
      } else {
         in = Erosion( in, robustSE );
      }
   }

   // Prepare temporary images
   Image tmp;
   tmp.Copy( in );
   DIP_ASSERT( tmp.HasContiguousData() );
   DataType ovlType = tmp.DataType();
   if( ovlType.IsBinary() ) {
      ovlType = DT_UINT8; // treat binary image as if it were uint8.
   }

   Image active;
   active.SetStrides( tmp.Strides() );
   active.ReForge( tmp, DT_BIN );
   DIP_ASSERT( active.Strides() == tmp.Strides() );

   Image len1, len2, len3, len4;
   len1.SetStrides( tmp.Strides() );
   len1.ReForge( tmp, DT_PATHLEN );
   DIP_ASSERT( len1.Strides() == tmp.Strides() );
   len2.SetStrides( tmp.Strides() );
   len2.ReForge( tmp, DT_PATHLEN );
   DIP_ASSERT( len2.Strides() == tmp.Strides() );
   if( constrained ) {
      len3.SetStrides( tmp.Strides() );
      len3.ReForge( tmp, DT_PATHLEN );
      DIP_ASSERT( len3.Strides() == tmp.Strides() );
      len4.SetStrides( tmp.Strides() );
      len4.ReForge( tmp, DT_PATHLEN );
      DIP_ASSERT( len4.Strides() == tmp.Strides() );
   }

   // Create sorted offsets array (skipping border)
   std::vector< dip::sint > offsets;
   if( mask.IsForged() ) {
      offsets = CreateOffsetsArray( mask, tmp.Strides() );
   } else {
      offsets = CreateOffsetsArray( tmp.Sizes(), tmp.Strides() );
   }
   if( offsets.empty() ) {
      // This can happen if `mask` is empty.
      out.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
      out.Fill( 0 );
      return;
   }
   SortOffsets( tmp, offsets, opening );

   // Create two arrays with offsets to neighbors
   IntegerArray offsetUp, offsetDown;

   // Loop over all ((3^ndims)-1)/2 directions
   bool firstOne = true;
   IntegerArray direction( ndims, -1 );
   for( ;; ) {

      // Check to see if this direction is "unique":
      // There must be at least one positive value, and the first non-negative value must be positive.
      bool valid = false;
      for( dip::uint ii = 0; ii < ndims; ++ii ) {
         if( direction[ ii ] != 0 ) {
            if( direction[ ii ] > 0 ) {
               valid = true;
            }
            break;
         }
      }
      if( valid ) {

         // Fill arrays with indices to neighbors
         MakeNeighborLists( direction, tmp.Strides(), offsetUp, offsetDown );

         // Initialise temporary images
         if( !firstOne ) {
            tmp.Copy( in );
         }
         if( mask.IsForged() ) {
            active.Copy( mask );
         } else {
            active.Fill( PO_ACTIVE );
         }
         SetBorder( active, Image::Pixel( 0 ) ); // Set border pixels to inactive, we won't process them.
         len1.Fill( length );
         len2.Fill( length );
         if( constrained ) {
            len3.Fill( length );
            len4.Fill( length );
         }

         // Do the data-type-dependent thing
         if( constrained ) {
            DIP_OVL_CALL_REAL( ConstrainedPathOpeningInternal,
                               ( tmp, active, len1, len2, len3, len4, offsets, offsetUp, offsetDown, length ),
                               ovlType );
         } else {
            DIP_OVL_CALL_REAL( PathOpeningInternal,
                               ( tmp, active, len1, len2, offsets, offsetUp, offsetDown, length ),
                               ovlType );
         }

         // Collect in output
         if( firstOne ) {
            out.Copy( tmp );
            firstOne = false;
         } else {
            if( opening ) {
               Supremum( tmp, out, out );
            } else {
               Infimum( tmp, out, out );
            }
         }
      }

      // Next
      dip::uint ii = 0;
      for( ; ii < ndims; ++ii ) {
         ++( direction[ ii ] );
         if( direction[ ii ] <= 1 ) {
            break;
         }
         direction[ ii ] = -1;
      }
      if( ii == ndims ) {
         break;
      }
   }

   // Finalize the robust method
   if( robust ) {
      if( opening ) {
         Infimum( orig_in, out, out );
      } else {
         Supremum( orig_in, out, out );
      }
   }
}

void DirectedPathOpening(
      Image const& c_in,
      Image const& c_mask,
      Image& out,
      IntegerArray filterParam,
      String const& polarity,
      StringSet const& mode
) {
   // Check input
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( c_in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint ndims = c_in.Dimensionality();
   DIP_THROW_IF( ndims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      DIP_THROW_IF( c_in.Size( ii ) < 3, "Input image is too small." );
   }
   DIP_THROW_IF( filterParam.size() != ndims, E::ARRAY_PARAMETER_WRONG_LENGTH );

   bool opening, constrained, robust;
   DIP_STACK_TRACE_THIS( ParsePathMode( polarity, mode, opening, constrained, robust ));

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
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
   }

   dip::uint length = static_cast< dip::uint >( std::abs( filterParam[ 0 ] ));
   for( dip::uint ii = 1; ii < ndims; ++ii ) {
      dip::uint tmp = static_cast< dip::uint >( std::abs( filterParam[ ii ] ));
      if( tmp > length ) {
         length = tmp;
      }
   }
   DIP_THROW_IF(( length < 2 ) || ( length > std::numeric_limits< PathLenType >::max() ), E::PARAMETER_OUT_OF_RANGE );
   IntegerArray direction( ndims, 0 );
   for( dip::uint ii = 0; ii < ndims; ++ii ) {
      direction[ ii ] = div_round( filterParam[ ii ], static_cast< dip::sint >( length ));
   }

   // Prepare output image
   if( out.IsForged() && ( !out.HasContiguousData() || ( out.DataType() != in.DataType() ) || out.Aliases( mask ) )) {
      out.Strip();
   }
   if( robust ) {
      if( out.Aliases( in )) {
         out.Strip(); // We cannot work in-place in this case.
      }
      StructuringElement robustSE{ 2, S::RECTANGULAR };
      if( opening ) {
         Dilation( in, out, robustSE );
      } else {
         Erosion( in, out, robustSE );
      }
   } else {
      out.Copy( in );
   }
   DIP_ASSERT( out.HasContiguousData() );
   DataType ovlType = out.DataType();
   if( ovlType.IsBinary() ) {
      ovlType = DT_UINT8; // treat binary image as if it were uint8.
   }

   // Prepare temporary images
   Image active;
   active.SetStrides( out.Strides() );
   active.ReForge( out, DT_BIN );
   DIP_ASSERT( active.Strides() == out.Strides() );
   if( mask.IsForged() ) {
      active.Copy( mask );
   } else {
      active.Fill( PO_ACTIVE );
   }
   SetBorder( active, Image::Pixel( 0 ) ); // Set border pixels to inactive, we won't process them.

   Image len1, len2, len3, len4;
   len1.SetStrides( out.Strides() );
   len1.ReForge( out, DT_PATHLEN );
   DIP_ASSERT( len1.Strides() == out.Strides() );
   len1.Fill( length );
   len2.SetStrides( out.Strides() );
   len2.ReForge( out, DT_PATHLEN );
   DIP_ASSERT( len2.Strides() == out.Strides() );
   len2.Fill( length );
   if( constrained ) {
      len3.SetStrides( out.Strides() );
      len3.ReForge( out, DT_PATHLEN );
      DIP_ASSERT( len3.Strides() == out.Strides() );
      len3.Fill( length );
      len4.SetStrides( out.Strides() );
      len4.ReForge( out, DT_PATHLEN );
      DIP_ASSERT( len4.Strides() == out.Strides() );
      len4.Fill( length );
   }

   // Create sorted offsets array (skipping border)
   std::vector< dip::sint > offsets;
   if( mask.IsForged() ) {
      offsets = CreateOffsetsArray( mask, out.Strides() );
   } else {
      offsets = CreateOffsetsArray( out.Sizes(), out.Strides() );
   }
   if( offsets.empty() ) {
      // This can happen if `mask` is empty.
      return;
   }
   SortOffsets( out, offsets, opening );

   // Create two arrays with offsets to neighbors
   IntegerArray offsetUp, offsetDown;
   MakeNeighborLists( direction, out.Strides(), offsetUp, offsetDown );

   // Do the data-type-dependent thing
   if( constrained ) {
      DIP_OVL_CALL_REAL( ConstrainedPathOpeningInternal,
                         ( out, active, len1, len2, len3, len4, offsets, offsetUp, offsetDown, length ),
                         ovlType );
   } else {
      DIP_OVL_CALL_REAL( PathOpeningInternal,
                         ( out, active, len1, len2, offsets, offsetUp, offsetDown, length ),
                         ovlType );
   }

   // Finalize the robust method
   if( robust ) {
      if( opening ) {
         Infimum( in, out, out );
      } else {
         Supremum( in, out, out );
      }
   }
}

} // namespace dip
