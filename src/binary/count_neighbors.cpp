/*
 * DIPlib 3.0
 * This file contains `dip::CountNeighbors` and related functions.
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
#include "diplib/binary.h"
#include "diplib/neighborlist.h"
#include "diplib/framework.h"

namespace dip {

namespace {

template< bool MAJORITY = false >
class CountNeighborsLineFilter : public Framework::ScanLineFilter {
   public:
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return 2 * neighbors_.Size(); // number of neighbors we test. We don't count the cost of testing for image boundaries.
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         auto bufferLength = params.bufferLength;
         bin const* in = static_cast< bin const* >( params.inBuffer[ 0 ].buffer );
         auto inStride = params.inBuffer[ 0 ].stride;
         uint8* out = static_cast< uint8* >( params.outBuffer[ 0 ].buffer ); // We treat this as bin if MAJORITY==true
         auto outStride = params.outBuffer[ 0 ].stride;
         dip::uint threshold = neighbors_.Size() / 2;
         // Determine if the processing line is on an edge of the image or not
         bool isOnEdge = false;
         for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
            if( ii != params.dimension ) {
               if(( params.position[ ii ] == 0 ) || ( params.position[ ii ] == sizes_[ ii ] - 1 )) {
                  isOnEdge = true;
                  break;
               }
            }
         }
         if( isOnEdge ) {
            // If so, tread carefully!
            UnsignedArray pos = params.position;
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               if( all_ || *in ) {
                  // count neighbors
                  auto it = neighbors_.begin();
                  auto off = offsets_.begin();
                  uint8 count = *in;
                  while( it != neighbors_.end() ) {
                     if( it.IsInImage( pos, sizes_ )) {
                        if( in[ *off ] ) {
                           ++count;
                        }
                     } else {
                        if( edgeCondition_ ) {
                           ++count;
                        }
                     }
                     ++it;
                     ++off;
                  }
                  if( MAJORITY ) {
                     *out = count > threshold ? 1 : 0;
                  } else {
                     *out = count;
                  }
               } else {
                  *out = 0;
               }
               in += inStride;
               out += outStride;
               ++pos[ params.dimension ];
            }
         } else {
            // Otherwise, just plow ahead. Only the first and last pixel can access outside of image domain
            if( all_ || *in ) {
               // count neighbors
               auto it = neighbors_.begin();
               auto off = offsets_.begin();
               uint8 count = *in;
               while( it != neighbors_.end() ) {
                  if( it.IsInImage( params.position, sizes_ )) {
                     if( in[ *off ] ) {
                        ++count;
                     }
                  } else {
                     if( edgeCondition_ ) {
                        ++count;
                     }
                  }
                  ++it;
                  ++off;
               }
               if( MAJORITY ) {
                  *out = count > threshold ? 1 : 0;
               } else {
                  *out = count;
               }
            } else {
               *out = 0;
            }
            in += inStride;
            out += outStride;
            for( dip::uint ii = 1; ii < bufferLength - 1; ++ii ) {
               if( all_ || *in ) {
                  // count neighbors
                  uint8 count = *in;
                  for( auto off : offsets_ ) {
                     if( in[ off ] ) {
                        ++count;
                     }
                  }
                  if( MAJORITY ) {
                     *out = count > threshold ? 1 : 0;
                  } else {
                     *out = count;
                  }
               } else {
                  *out = 0;
               }
               in += inStride;
               out += outStride;
            }
            if( all_ || *in ) {
               // count neighbors
               UnsignedArray pos = params.position;
               pos[ params.dimension ] += bufferLength - 1;
               auto it = neighbors_.begin();
               auto off = offsets_.begin();
               uint8 count = *in;
               while( it != neighbors_.end() ) {
                  if( it.IsInImage( pos, sizes_ )) {
                     if( in[ *off ] ) {
                        ++count;
                     }
                  } else {
                     if( edgeCondition_ ) {
                        ++count;
                     }
                  }
                  ++it;
                  ++off;
               }
               if( MAJORITY ) {
                  *out = count > threshold ? 1 : 0;
               } else {
                  *out = count;
               }
            } else {
               *out = 0;
            }
         }
      }
      CountNeighborsLineFilter( NeighborList const& neighbors, IntegerArray const& offsets, bool all, bool edgeCondition, UnsignedArray const& sizes ) :
            neighbors_( neighbors ), offsets_( offsets ), all_( all ), edgeCondition_( edgeCondition ), sizes_( sizes ) {}
   private:
      NeighborList const& neighbors_;
      IntegerArray const& offsets_;
      bool all_;
      bool edgeCondition_;
      UnsignedArray const& sizes_;
};

} // namespace

void CountNeighbors(
      Image const& in,
      Image& out,
      dip::uint connectivity,
      dip::String const& s_mode,
      dip::String const& s_edgeCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( connectivity > in.Dimensionality(), E::ILLEGAL_CONNECTIVITY );
   DIP_START_STACK_TRACE
      NeighborList neighbors( Metric( Metric::TypeCode::CONNECTED, connectivity ), in.Dimensionality() );
      IntegerArray offsets = neighbors.ComputeOffsets( in.Strides() );
      bool all = BooleanFromString( s_mode, S::ALL, S::FOREGROUND );
      bool edgeCondition = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND );
      CountNeighborsLineFilter< false > scanLineFilter( neighbors, offsets, all, edgeCondition, in.Sizes() );
      // We're guaranteed here that the framework will not use a temporary input buffer, because:
      //  - The input image is DT_BIN, and we request a DT_BIN buffer, and
      //  - We did not give the ScanOption::ExpandTensorInBuffer option.
      // Thus we can access pixels outside of the scan line in our scanLineFilter. Be careful when doing this!
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { DT_BIN }, { DT_UINT8 }, { DT_UINT8 }, { 1 }, scanLineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

void MajorityVote(
      Image const& in,
      Image& out,
      dip::uint connectivity,
      dip::String const& s_edgeCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsBinary(), E::IMAGE_NOT_BINARY );
   DIP_THROW_IF( connectivity > in.Dimensionality(), E::ILLEGAL_CONNECTIVITY );
   DIP_START_STACK_TRACE
      NeighborList neighbors( Metric( Metric::TypeCode::CONNECTED, connectivity ), in.Dimensionality() );
      IntegerArray offsets = neighbors.ComputeOffsets( in.Strides() );
      bool edgeCondition = BooleanFromString( s_edgeCondition, S::OBJECT, S::BACKGROUND );
      CountNeighborsLineFilter< true > scanLineFilter( neighbors, offsets, true, edgeCondition, in.Sizes() );
      // We're guaranteed here that the framework will not use a temporary input buffer, because:
      //  - The input image is DT_BIN, and we request a DT_BIN buffer, and
      //  - We did not give the ScanOption::ExpandTensorInBuffer option.
      // Thus we can access pixels outside of the scan line in our scanLineFilter. Be careful when doing this!
      ImageRefArray outar{ out };
      Framework::Scan( { in }, outar, { DT_BIN }, { DT_BIN }, { DT_BIN }, { 1 }, scanLineFilter, Framework::ScanOption::NeedCoordinates );
      // Note that we're requesting a binary output image and binary output buffers. The scan line filter uses uint8
      // buffers. These are the same size by definition, so this works fine.
   DIP_END_STACK_TRACE
}

} // namespace dip
