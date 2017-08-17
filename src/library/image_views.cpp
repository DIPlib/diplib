/*
 * DIPlib 3.0
 * This file contains definitions for the Image class and related functions.
 *
 * (c)2014-2017, Cris Luengo.
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
#include "diplib/statistics.h"
#include "diplib/generic_iterators.h"

namespace dip {

Image::View::View( Image const& reference, Range range ) : reference_( reference ) {
   DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_STACK_TRACE_THIS( range.Fix( reference_.TensorElements() ));
   reference_.origin_ = reference_.Pointer( range.start * reference_.TensorStride() );
   reference_.tensor_.SetVector( range.Size() );
   reference_.tensorStride_ *= range.Step();
   if( reference_.TensorElements() != reference.TensorElements() ) {
      reference_.ResetColorSpace();
   }
}

Image::View::View( Image const& reference, RangeArray ranges ) : reference_( reference ) {
   DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = reference_.Dimensionality();
   DIP_THROW_IF( nDims != ranges.size(), E::ARRAY_ILLEGAL_SIZE );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_STACK_TRACE_THIS( ranges[ ii ].Fix( reference_.Size(  ii )));
   }
   dip::sint offset = 0;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      offset += static_cast< dip::sint >( ranges[ ii ].Offset() ) * reference_.strides_[ ii ];
      reference_.sizes_[ ii ] = ranges[ ii ].Size();
      reference_.strides_[ ii ] *= ranges[ ii ].Step();
      reference_.pixelSize_.Scale( ii, static_cast< dfloat >( ranges[ ii ].Step() ));
   }
   reference_.origin_ = reference_.Pointer( offset ); // based only on origin and data type sizeof
}

Image::View::View( Image const& reference, Image const& mask ) : reference_( reference ), mask_( mask ) {
   DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask_.IsForged(), E::IMAGE_NOT_FORGED );
   if( mask_.TensorElements() > 1 ) {
      // We're indexing samples
      reference_.TensorToSpatial( 0 ); // will always work, even if it's scalar
      mask_.TensorToSpatial( 0 );
   }
   DIP_STACK_TRACE_THIS( mask_.CheckIsMask( reference_.Sizes(), Option::AllowSingletonExpansion::DONT_ALLOW, Option::ThrowException::DO_THROW ));
}

Image::View::View( Image const& reference, UnsignedArray const& indices ) : reference_( reference ) {
   DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( indices.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   dip::uint maxIndex = reference_.NumberOfPixels();
   for( auto const& ii : indices ) {
      DIP_THROW_IF( ii >= maxIndex, E::INDEX_OUT_OF_RANGE );
   }
   offsets_.resize( indices.size() );
   CoordinatesComputer coordinates = reference_.IndexToCoordinatesComputer();
   auto in = indices.begin();
   auto out = offsets_.begin();
   for( ; in != indices.end(); ++in, ++out ) {
      *out = reference_.Offset( coordinates( static_cast< dip::sint >( *in )));
   }
}

Image::View::View( Image const& reference, CoordinateArray const& coordinates ) : reference_( reference ) {
   DIP_THROW_IF( !reference_.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( coordinates.size() == 0, E::ARRAY_ILLEGAL_SIZE );
   dip::uint nDims = reference_.Dimensionality();
   for( UnsignedArray const& pp : coordinates ) {
      DIP_THROW_IF( pp.size() != nDims, E::COORDINATES_OUT_OF_RANGE );
      DIP_THROW_IF( !( pp < reference_.Sizes() ), E::COORDINATES_OUT_OF_RANGE );
   }
   offsets_.resize( coordinates.size() );
   auto in = coordinates.begin();
   auto out = offsets_.begin();
   for( ; in != coordinates.end(); ++in, ++out ) {
      *out = reference_.Offset( *in );
   }
}

void Image::View::Copy( Image const& source ) {
   DIP_THROW_IF( !source.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( reference_.TensorElements() != source.TensorElements(), E::NTENSORELEM_DONT_MATCH );
   if( mask_.IsForged() ) {
      CopyTo( source, reference_, mask_ );
   } else if( !offsets_.empty() ) {
      CopyTo( source, reference_, offsets_ );
   } else {
      reference_.Copy( source );
   }
}

void Image::View::Fill( Pixel const& pixel ) {
   if( pixel.TensorElements() == 1 ) {
      Fill( pixel[ 0 ] );
      return;
   }
   dip::uint N = reference_.TensorElements();
   DIP_THROW_IF( pixel.TensorElements() != N, E::NTENSORELEM_DONT_MATCH );
   auto backupTensor = reference_.tensor_;
   auto backupOrigin = reference_.origin_;
   reference_.tensor_.SetScalar();
   try {
      for( dip::uint ii = 0; ii < N; ++ii, reference_.origin_ = reference_.Pointer( reference_.tensorStride_ )) {
         // NOTE: The above is equivalent to tmp.reference_.origin += tmp.reference_.tensorStride_ if tmp.origin_
         // were a pointer to the correct data type.
         Fill( pixel[ ii ] );
      }
   } catch( ... ) {
      // The loop above is unlikely to throw, but if it does, it leaves *this in disarray. Catch, fix, rethrow.
      reference_.origin_ = backupOrigin;
      reference_.tensor_ = backupTensor;
      throw;
   }
   reference_.origin_ = backupOrigin;
   reference_.tensor_ = backupTensor;
}

void Image::View::Fill( Sample const& sample ) {
   Image::Sample source( reference_.DataType() );
   source = sample; // cast the value to the same type as that of `reference_`.
   dip::uint telems = reference_.TensorElements();
   dip::uint bytes = reference_.DataType().SizeOf(); // both source and destination have the same types
   if( mask_.IsForged() ) {
      // Iterate over reference_ and mask_
      GenericJointImageIterator< 2 > it( { reference_, mask_ } );
      void* src = source.Origin();
      if( telems == 1 ) {
         do {
            if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
               std::memcpy( it.Pointer< 0 >(), src, bytes );
            }
         } while( ++it );
      } else {
         do {
            if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
               for( dip::uint ii = 0; ii < telems; ++ii ) {
                  std::memcpy( it.Pointer< 0 >( ii ), src, bytes );
               }
            }
         } while( ++it );
      }
   } else if( !offsets_.empty() ) {
      // Iterate over offsets
      void* src = source.Origin();
      if( telems == 1 ) {
         for( auto& offset : offsets_ ) {
            std::memcpy( reference_.Pointer( offset ), src, bytes );
         }
      } else {
         for( auto& offset : offsets_ ) {
            for( dip::uint ii = 0; ii < telems; ++ii ) {
               std::memcpy( reference_.Pointer( offset ), src, bytes );
               offset += reference_.TensorStride();
            }
         }
      }
   } else {
      reference_.Fill( source );
   }
}

Image::Pixel Image::View::At( UnsignedArray const& coords ) const {
   if( mask_.IsForged() || !offsets_.empty() ) {
      DIP_THROW_IF( coords.size() != 1, E::ILLEGAL_DIMENSIONALITY );
      return At( coords[ 0 ] );
   } else {
      DIP_STACK_TRACE_THIS( return reference_.At( coords ));
   }
}

Image::Pixel Image::View::At( dip::uint index ) const {
   if( mask_.IsForged() ) {
      GenericJointImageIterator< 2 > it( { reference_, mask_ } );
      do {
         if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
            if( index == 0 ) {
               return Pixel( it.Pointer< 0 >(), reference_.dataType_, reference_.tensor_, reference_.tensorStride_ );
            }
            --index;
         }
      } while( ++it );
      DIP_THROW( E::INDEX_OUT_OF_RANGE );
   } else if( !offsets_.empty() ) {
      DIP_THROW_IF( index >= offsets_.size(), E::INDEX_OUT_OF_RANGE );
      return Pixel( reference_.Pointer( offsets_[ index ] ), reference_.dataType_, reference_.tensor_, reference_.tensorStride_ );
   } else {
      DIP_STACK_TRACE_THIS( return reference_.At( index ));
   }
}

Image::View Image::View::At( Range x_range ) const {
   if( mask_.IsForged() ) {
      dip::uint N = Count( mask_ );
      DIP_STACK_TRACE_THIS( x_range.Fix( N ));
      View out( reference_ );
      out.offsets_.resize( x_range.Size() );
      dip::uint start = x_range.Offset();
      if( x_range.start > x_range.stop ) {
         start -= ( x_range.Size() - 1 ) * x_range.step;
      }
      GenericJointImageIterator< 2 > it( { reference_, mask_ } );
      // Note that we've counted the number of set pixels in mask_, so the looping below never sees it going bad.
      dip::uint ii = 0;
      while( ii < start ) {
         if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
            ++ii;
         }
         ++it;
      }
      for( auto o : out.offsets_ ) {
         o = it.Offset< 0 >();
         for( ii = 0; ii < x_range.step; ++ii ) {
            ++it;
         }
      }
      if( x_range.start > x_range.stop ) {
         // We've got the list backwards now, reverse it
         std::reverse( out.offsets_.begin(), out.offsets_.end() );
      }
      return out;
   } else if( !offsets_.empty() ) {
      DIP_STACK_TRACE_THIS( x_range.Fix( offsets_.size() ));
      View out( reference_ );
      out.offsets_.resize( x_range.Size() );
      auto outIt = out.offsets_.begin();
      for( auto ii : x_range ) {
         *outIt++ = offsets_[ ii ];
      }
      return out;
   } else {
      DIP_THROW_IF( Dimensionality() != 1, E::ILLEGAL_DIMENSIONALITY );
      return At( RangeArray{ x_range } );
   }
}

Image::View Image::View::At( RangeArray const& ranges ) const {
   if( mask_.IsForged() || !offsets_.empty() ) {
      DIP_THROW_IF( ranges.size() != 1, E::ILLEGAL_DIMENSIONALITY );
      return At( ranges[ 0 ] );
   }
   return View( reference_, ranges );
}


} // namespace dip
