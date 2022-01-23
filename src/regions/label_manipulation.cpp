/*
 * (c)2016-2021, Cris Luengo.
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
#include "diplib/private/robin_map.h"
#include "diplib/private/robin_set.h"
#include "diplib/regions.h"
#include "diplib/measurement.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

using LabelSet = tsl::robin_set< dip::uint >;

namespace {

template< typename TPI >
class GetLabelsLineFilter: public Framework::ScanLineFilter {
   public:
      // not defining GetNumberOfOperations(), always called in a single thread
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* data = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         if( params.inBuffer.size() > 1 ) {
            bin* mask = static_cast< bin* >( params.inBuffer[ 1 ].buffer );
            dip::sint mask_stride = params.inBuffer[ 1 ].stride;
            dip::uint prevID = 0;
            bool setPrevID = false;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *mask ) {
                  if( !setPrevID || ( *data != prevID ) ) {
                     prevID = static_cast< dip::uint >( *data );
                     setPrevID = true;
                     objectIDs_.insert( prevID );
                  }
               }
               data += stride;
               mask += mask_stride;
            }
         } else {
            dip::uint prevID = static_cast< dip::uint >( *data ) + 1; // something that's different from the first pixel value
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( *data != prevID ) {
                  prevID = static_cast< dip::uint >( *data );
                  objectIDs_.insert( prevID );
               }
               data += stride;
            }
         }
      }
      GetLabelsLineFilter( LabelSet& objectIDs ): objectIDs_( objectIDs ) {}
   private:
      LabelSet& objectIDs_;
};

} // namespace

UnsignedArray GetObjectLabels(
      Image const& label,
      Image const& mask,
      String const& background
) {
   // Check input
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   if( mask.IsForged() ) {
      DIP_STACK_TRACE_THIS( mask.CheckIsMask( label.Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW ));
   }
   bool nullIsObject;
   DIP_STACK_TRACE_THIS( nullIsObject = BooleanFromString( background, S::INCLUDE, S::EXCLUDE ));

   LabelSet objectIDs; // output

   // Get pointer to overloaded scan function
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_UINT( scanLineFilter, GetLabelsLineFilter, ( objectIDs ), label.DataType() );

   // Do the scan
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( label, mask, label.DataType(), *scanLineFilter, Framework::ScanOption::NoMultiThreading ));

   // Should we ignore the 0 label?
   if( !nullIsObject ) {
      objectIDs.erase( 0 );
   }

   // Copy the labels to output array
   UnsignedArray out( objectIDs.size() );
   dip::uint count = 0;
   for( auto id : objectIDs ) {
      out[ count ] = id;
      ++count;
   }

   // Our set is unordered, we now need to sort the list of objects
   std::sort( out.begin(), out.end() );

   return out;
}

namespace {

template< typename TPI >
class RelabelLineFilter: public Framework::ScanLineFilter {
   public:
      // not defining GetNumberOfOperations(), always called in a single thread
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         TPI inLabel = 0;       // last label seen, initialize to background label
         TPI outLabel = 0;      // new label assigned to prevID
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if( *in == 0 ) {
               // The background label is processed differently
               *out = 0;
            } else if( *in == inLabel ) {
               *out = outLabel;
            } else {
               inLabel = *in;
               auto it = objectIDs_.find( inLabel );
               if( it == objectIDs_.end() ) {
                  // It's a new label
                  outLabel = ++lastLabel_;
                  objectIDs_.emplace( inLabel, outLabel );
               } else {
                  outLabel = it.value();
               }
               *out = outLabel;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      tsl::robin_map< TPI, TPI > objectIDs_;
      TPI lastLabel_ = 0;
};

} // namespace

void Relabel( Image const& label, Image& out ) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );

   // Get pointer to overloaded scan function
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_UINT( scanLineFilter, RelabelLineFilter, (), label.DataType() );

   // Do the scan
   DIP_STACK_TRACE_THIS( Framework::ScanMonadic( label, out, label.DataType(), label.DataType(), 1, *scanLineFilter, Framework::ScanOption::NoMultiThreading ));
}

void SmallObjectsRemove(
      Image const& in,
      Image& out,
      dip::uint threshold,
      dip::uint connectivity
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   if( in.DataType().IsBinary() ) {
      Image tmp = Label( in, connectivity, threshold, 0 );
      NotEqual( tmp, Image( 0, tmp.DataType() ), out );
   } else if( in.DataType().IsUnsigned() ) {
      MeasurementTool msr;
      Measurement sizes = msr.Measure( in, {}, { "Size" }, {}, 1 );
      // TODO: It would be more efficient to copy the ObjectToMeasurement function, and make a LUT that does all of the following in a single pass through the image.
      Image tmp = ObjectToMeasurement( in, sizes[ "Size" ] );
      tmp = tmp >= threshold;
      MultiplySampleWise( in, tmp, out, in.DataType() );
   } else {
      DIP_THROW( E::DATA_TYPE_NOT_SUPPORTED );
   }
}


namespace {

template< typename TPI >
void ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck(
      TPI* ptr,
      NeighborList const& neighbors,
      IntegerArray const& offsets,
      UnsignedArray const& coords,
      UnsignedArray const& sizes
) {
   if( *ptr ) {
      // Examine all neighbors to see if any has a different label
      auto nit = neighbors.begin();
      for( auto o : offsets ) {
         if( nit.IsInImage( coords, sizes )) {
            TPI n = ptr[ o ];
            if( n > *ptr ) {
               *ptr = 0;
               break;
            }
         }
         ++nit;
      }
   }
}

template< typename TPI >
void ZeroPixelIfHasDifferentNeighbor(
      TPI* ptr,
      IntegerArray const& offsets
) {
   if( *ptr ) {
      // Examine all neighbors to see if any has a different label
      for( auto o : offsets ) {
         TPI n = ptr[ o ];
         if( n > *ptr ) {
            *ptr = 0;
            break;
         }
      }
   }
}

template< typename TPI >
void SplitRegions(
   Image& img,
   NeighborList const& neighbors,
   IntegerArray const& offsets
) {
   dip::uint procDim = Framework::OptimalProcessingDim( img );
   // Iterate over image lines
   ImageIterator< TPI > it( img, procDim );
   do {
      auto coords = it.Coordinates();
      auto lit = it.GetLineIterator();
      if( it.IsOnEdge()) {
         // We test all the pixels along the line
         do {
            ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
            ++coords[ procDim ];
         } while( ++lit );
      } else {
         // We test the first pixel along the line
         ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
         ++lit;
         // The bulk of the pixels we don't need to worry about testing
         for( dip::uint ii = 1; ii < lit.Length() - 1; ++ii, ++lit ) {
            ZeroPixelIfHasDifferentNeighbor( lit.Pointer(), offsets );
         }
         // The last pixel we test again
         coords[ procDim ] = lit.Coordinate();
         ZeroPixelIfHasDifferentNeighbor_WithBoundaryCheck( lit.Pointer(), neighbors, offsets, coords, img.Sizes() );
      }
   } while( ++it );
}

} // namespace

void SplitRegions(
      Image const& label,
      Image& out,
      dip::uint connectivity
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );

   // We work in-place in the output image. Copy input data there.
   if( &out != &label ) {
      out.Copy( label );
      DIP_THROW_IF( !out.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   }

   // Create a copy with optimally sorted strides and no singleton dimensions
   Image img = out.QuickCopy();
   img.StandardizeStrides();
   dip::uint nDims = img.Dimensionality();
   NeighborList neighbors(Metric( Metric::TypeCode::CONNECTED, connectivity ), nDims);
   IntegerArray offsets = neighbors.ComputeOffsets( img.Strides() );

   // Process
   DIP_OVL_CALL_UINT( SplitRegions, ( img, neighbors, offsets ), img.DataType() );

}

namespace {

template< typename TPI >
RangeArray GetLabelBoundingBoxInernal(
      Image const& label,
      dip::uint objectID
) {
   DIP_ASSERT( label.DataType() == DataType( TPI( 0 )));
   dip::uint nDims = label.Dimensionality();
   RangeArray bb;
   bool needInit = true; // bb needs initialization
   ImageIterator< TPI > it( label );
   do {
      if( *it == objectID ) {
         if( needInit ) {
            // The first pixel with this value: initialize the output RangeArray
            bb.resize( nDims );
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               bb[ ii ] = Range{ static_cast< dip::sint >( it.Coordinates()[ ii ] ) };
            }
            needInit = false;
         } else {
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               bb[ ii ].start = std::min( bb[ ii ].start, static_cast< dip::sint >( it.Coordinates()[ ii ] ));
               bb[ ii ].stop = std::max( bb[ ii ].stop, static_cast< dip::sint >( it.Coordinates()[ ii ] ));
            }
         }
      }
   } while( ++it );
   return bb;
}

} // namespace

RangeArray GetLabelBoundingBox(
      Image const& label,
      dip::uint objectID
) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( label.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   RangeArray bb;
   DIP_OVL_CALL_ASSIGN_UNSIGNED( bb, GetLabelBoundingBoxInernal, ( label, objectID ), label.DataType() );
   return bb;
}

} // namespace dip
