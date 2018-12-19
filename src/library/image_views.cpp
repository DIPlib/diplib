/*
 * DIPlib 3.0
 * This file contains definitions for support classes for the Image class.
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
   DIP_THROW_IF( nDims != ranges.size(), E::ARRAY_PARAMETER_WRONG_LENGTH );
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
   DIP_THROW_IF( indices.empty(), E::ARRAY_PARAMETER_EMPTY );
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
   DIP_THROW_IF( coordinates.empty(), E::ARRAY_PARAMETER_EMPTY );
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
      DIP_STACK_TRACE_THIS( CopyTo( source, reference_, mask_ ));
   } else if( !offsets_.empty() ) {
      DIP_STACK_TRACE_THIS( CopyTo( source, reference_, offsets_ ));
   } else {
      dip::Image src = source.QuickCopy();
      while( src.Size( src.Dimensionality() - 1 ) == 1 ) { // remove trailing singleton dimensions
         src.Squeeze( src.Dimensionality() - 1 );
      }
      dip::Image dst = reference_.QuickCopy();
      while( dst.Size( dst.Dimensionality() - 1 ) == 1 ) { // remove trailing singleton dimensions
         dst.Squeeze( dst.Dimensionality() - 1 );
      }
      DIP_THROW_IF( dst.Sizes() != src.Sizes(), E::SIZES_DONT_MATCH );
      dst.Protect();
      DIP_STACK_TRACE_THIS( dst.Copy( src )); // This should always work.
   }
}

Image Image::View::Copy() const {
   // This is similar to `Image out = *this`, except in the case of no mask or offsets, we copy the pixels too.
   Image out;
   if( mask_.IsForged() ) {
      CopyFrom( reference_, out, mask_ );
   } else if( !offsets_.empty() ) {
      CopyFrom( reference_, out, offsets_ );
   } else {
      out = reference_.Copy();
   }
   return out;
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
      it.OptimizeAndFlatten();
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
         for( auto offset : offsets_ ) {
            std::memcpy( reference_.Pointer( offset ), src, bytes );
         }
      } else {
         for( auto offset : offsets_ ) {
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
      out.offsets_.resize( x_range.Size(), 0 );
      dip::uint start = x_range.Offset();
      if( x_range.start > x_range.stop ) {
         start -= ( x_range.Size() - 1 ) * x_range.step;
      }
      GenericJointImageIterator< 2 > it( { reference_, mask_ } );
      // Note that we've counted the number of set pixels in mask_, so the looping below never sees it going bad.
      dip::uint ii = 0;
      while( true ) {
         if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
            if( ii == start ) {
               break;
            }
            ++ii;
         }
         ++it;
      }
      for( auto& o : out.offsets_ ) {
         DIP_ASSERT( it );
         o = it.Offset< 0 >();
         ii = 0;
         while( true ) {
            if( !it ) {
               break; // this happens when we just added the offset for the last on pixel in the mask, we shouldn't try to find a next on pixel after that!
               // TODO: we could probably write this loop better so that we don't look for the next set pixel after the last one was found.
            }
            if( *( static_cast< bin* >( it.Pointer< 1 >() ))) {
               if( ii == x_range.step ) {
                  break;
               }
               ++ii;
            }
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


Image::View::Iterator::Iterator() : view_( Image{} ), atEnd_( true ) {}
Image::View::Iterator::Iterator( View const& view ) : view_( view ) { Initialize(); }
Image::View::Iterator::Iterator( View&& view ) : view_( std::move( view )) { Initialize(); }
Image::View::Iterator::Iterator( Iterator&& ) = default;
Image::View::Iterator::~Iterator() = default;

void Image::View::Iterator::Initialize() {
   if( view_.mask_.IsForged() ) {
      // Iterate over the set pixels in the mask
      maskIt_ = std::make_unique< GenericJointImageIterator< 2, dip::dfloat >>( ImageConstRefArray{ view_.reference_, view_.mask_ } );
      // test to see if we're pointing at a set pixel, if not, call operator++.
      if( !*( static_cast< bin* >( maskIt_->Pointer< 1 >() ))) {
         operator++();
      }
   } else if( !view_.offsets_.empty() ) {
      // Iterate over the offset array
      // nothing to do here
   } else {
      // Iterate over the image
      refIt_ = std::make_unique< GenericImageIterator< dip::dfloat >>( view_.reference_ );
   }
}

Image::View::Iterator& Image::View::Iterator::operator++() {
   if( !atEnd_ ) {
      ++position_;
      if( maskIt_ ) {
         ++*maskIt_;
         do {
            if( *( static_cast< bin* >( maskIt_->Pointer< 1 >() ))) {
               break;
            }
         } while( ++*maskIt_ );
         atEnd_ = maskIt_->IsAtEnd();
      } else if( refIt_ ) {
         ++*refIt_;
         atEnd_ = refIt_->IsAtEnd();
      } else {
         atEnd_ = position_ >= view_.offsets_.size();
      }
   }
   return *this;
}

void* Image::View::Iterator::Pointer() const {
   DIP_THROW_IF( atEnd_, "Iterator at end cannot be dereferenced" );
   if( maskIt_ ) {
      return maskIt_->Pointer< 0 >();
   }
   if( refIt_ ) {
      return refIt_->Pointer();
   }
   return view_.reference_.Pointer( view_.offsets_[ position_ ] );
}

void* Image::View::Iterator::Pointer( dip::uint index ) const {
   DIP_THROW_IF( atEnd_, "Iterator at end cannot be dereferenced" );
   if( maskIt_ ) {
      return maskIt_->Pointer< 0 >( index );
   }
   if( refIt_ ) {
      return refIt_->Pointer( index );
   }
   return view_.reference_.Pointer( view_.offsets_[ position_ ] + static_cast< dip::sint >( index ) * view_.reference_.TensorStride() );
}

dip::sint Image::View::Iterator::Offset() const {
   DIP_THROW_IF( atEnd_, "Iterator at end cannot be dereferenced" );
   if( maskIt_ ) {
      return maskIt_->Offset< 0 >();
   }
   if( refIt_ ) {
      return refIt_->Offset();
   }
   return view_.offsets_[ position_ ];
}

void Image::View::Iterator::Reset() {
   if( maskIt_ ) {
      maskIt_->Reset();
   } else if( refIt_ ) {
      refIt_->Reset();
   }
   position_ = 0;
}

Image::View::Iterator Image::View::begin() const {
   return Image::View::Iterator( *this );
}

Image::View::Iterator Image::View::end() const {
   return {};
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Image::Pixel and related classes" ) {

   // Constructing, indexing, comparing
   dip::Image::Sample s{ 4.6 };
   DOCTEST_CHECK( s.DataType() == dip::DT_DFLOAT );
   dip::Image::Sample c{ dip::dcomplex{ 4.1, 2.1 }};
   dip::Image::Pixel p{ dip::dcomplex{ 4.1, 2.1 }, dip::dcomplex{ 4.6 } };
   DOCTEST_CHECK( p.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( p.TensorElements() == 2 );
   DOCTEST_CHECK( p[ 0 ] == c );
   DOCTEST_CHECK( p[ 1 ] == s );

   // Assigning into
   dip::Image image( { 3, 4 }, 3, dip::DT_UINT16 );
   image = 3;
   DOCTEST_CHECK( image.At( 0 )[ 0 ] == 3 ); // fist sample
   DOCTEST_CHECK( image.At( 0 ) == 3 );      // fist pixel
   DOCTEST_CHECK( image.At( 2, 3 )[ 2 ] == 3 ); // last sample
   image.At( 0 ) = 4;
   dip::Image::Pixel expect1{ dip::uint8( 4 ), dip::uint8( 4 ), dip::uint8( 4 ) };
   DOCTEST_CHECK( image.At( 0 ) == 4 );
   DOCTEST_CHECK( image.At( 0 ) == expect1 );
   DOCTEST_CHECK_FALSE( image.At( 0 ) == 0 );
   DOCTEST_CHECK_FALSE( image.At( 1 ) == expect1 );
   DOCTEST_CHECK_FALSE( image.At( 1 )[ 1 ] == 4 );
   DOCTEST_CHECK( image.At( 0 )[ 1 ] == 4 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );
   image.At( 0 )[ 2 ] = 5;
   dip::Image::Pixel expect2 = expect1;
   expect2[ 2 ] = 5;
   DOCTEST_CHECK( image.At( 0 ) == expect2 );
   DOCTEST_CHECK( image.At( 0 )[ 1 ] == 4 );
   DOCTEST_CHECK( image.At( 0 )[ 2 ] == 5 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );
   image.At( 0 ) = { 8, 9, 0 };
   dip::Image::Pixel expect3{ 8, 9, 0 };
   DOCTEST_CHECK( image.At( 0 ) == expect3 );
   DOCTEST_CHECK( image.At( 0 )[ 1 ] == 9 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );
   image.At( 1 ) = expect3;
   DOCTEST_CHECK( image.At( 1 ) == expect3 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );
   image.At( 2, 0 ) = image.At( 0 );
   DOCTEST_CHECK( image.At( 2, 0 ) == expect3 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );
   image.At( 2, 0 ) = image.At( 0 )[ 0 ];
   DOCTEST_CHECK( image.At( 2, 0 ) == 8 );
   DOCTEST_CHECK( image.At( 2, 3 )[ 0 ] == 3 );

   // Reading out
   dip::uint8 v0 = image.At< dip::uint8 >( 2, 0 );
   dip::uint8 v1 = image.At< dip::uint8 >( 2, 0 )[ 0 ];
   dip::sint16 v2 = image.At< dip::sint16 >( 2, 0 )[ 0 ];
   dip::scomplex v3 = image.At< dip::scomplex >( 2, 0 )[ 0 ];
   DOCTEST_CHECK( v0 == 8 );
   DOCTEST_CHECK( v1 == 8 );
   DOCTEST_CHECK( v2 == 8 );
   DOCTEST_CHECK( v3 == 8.0f );

   // Arithmetic
   expect3 = expect3 * 2;
   DOCTEST_CHECK( expect3[ 0 ] == 16 );
   DOCTEST_CHECK( expect3[ 1 ] == 18 );
   DOCTEST_CHECK( expect3[ 2 ] == 0 );
   DOCTEST_CHECK( image.At< dip::uint8 >( 1 ) * 2 == expect3 );
   DOCTEST_CHECK( image.At< dip::sint16 >( 1 ) * 2 == expect3 );
   DOCTEST_CHECK( image.At< dip::scomplex >( 1 ) * 2 == expect3 );
   DOCTEST_CHECK( image.At< dip::uint8 >( 1 )[ 0 ] * 2 == 16 );
   DOCTEST_CHECK( image.At< dip::sint16 >( 1 )[ 0 ] * 2 == 16 );
   DOCTEST_CHECK( image.At< dip::scomplex >( 1 )[ 0 ] * 2 == 16 );

   // Compound assignment
   expect3 += 5;
   DOCTEST_CHECK( expect3[ 0 ] == 21 );
   DOCTEST_CHECK( expect3[ 1 ] == 23 );
   DOCTEST_CHECK( expect3[ 2 ] == 5 );
   expect3 += expect2;
   DOCTEST_CHECK( expect3[ 0 ] == 25 );
   DOCTEST_CHECK( expect3[ 1 ] == 27 );
   DOCTEST_CHECK( expect3[ 2 ] == 10 );
   image.At( 2, 0 ) -= 4;
   DOCTEST_CHECK( image.At( 2, 0 ) == expect1 );
   image.At( 2, 0 )[ 2 ] += 1;
   DOCTEST_CHECK( image.At( 2, 0 ) == expect2 );
   image.At( 2, 0 )[ 2 ] &= 4;
   DOCTEST_CHECK( image.At( 2, 0 ) == expect1 );

   // Iterator
   image.At( 0 ) = expect3;
   auto it1 = image.At( 0 ).begin();
   auto it2 = expect3.begin();
   DOCTEST_CHECK( *it1 == *it2 );
   ++it1; ++it2;
   DOCTEST_CHECK( *it1 == *it2 );
   ++it1; ++it2;
   DOCTEST_CHECK( *it1 == *it2 );
   ++it1; ++it2;
   DOCTEST_CHECK( it2 == expect3.end() );

   // Swapping
   s.swap( c );
   DOCTEST_CHECK( p[ 0 ] == s );
   DOCTEST_CHECK( p[ 1 ] == c );

   expect1.swap( p );
   DOCTEST_CHECK( expect1.DataType() == dip::DT_DCOMPLEX );
   DOCTEST_CHECK( expect1.TensorElements() == 2 );
   DOCTEST_CHECK( expect1[ 0 ] == s );
   DOCTEST_CHECK( expect1[ 1 ] == c );

   DOCTEST_CHECK( p.DataType() == dip::DT_UINT8 );
   DOCTEST_CHECK( p.TensorElements() == 3 );
   DOCTEST_CHECK( p[ 0 ] == 4 );
   DOCTEST_CHECK( p[ 1 ] == 4 );
   DOCTEST_CHECK( p[ 2 ] == 4 );

   // Conversion to/from FloatArray
   dip::FloatArray array{ 10.2, 564535.65, -432.12, 0.004563 };
   dip::Image::Pixel newPixel{ array, dip::DT_DFLOAT };
   auto newArray = static_cast< dip::FloatArray >( newPixel );
   DOCTEST_CHECK( newArray == array );
}

DOCTEST_TEST_CASE( "[DIPlib] testing dip::Image::View" ) {

   // -- Indexing into image (3 types of indexing)

   dip::Image img{ dip::UnsignedArray{ 15, 20, 10 }, 3 };
   img.Fill( 0 );

   // Regular indexing
   auto viewR = img.At( dip::Range{ 3, 9, 3 }, dip::Range{ 3, 9, 3 }, dip::Range{ 3, 9, 3 } ); // 3x3x3 output
   dip::Image ref = viewR;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 3, 3, 3 } );
   DOCTEST_CHECK( ref.TensorElements() == 3 );
   viewR.Fill( 1 );
   DOCTEST_CHECK( dip::Count( ref[ 0 ] ) == 3*3*3 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] ) == 3*3*3 ); // we didn't write into pixels not in the view
   DOCTEST_CHECK( dip::Count( img[ 2 ] ) == 3*3*3 );

   // Indexing using mask image
   dip::Image mask = img[ 0 ] > 0;
   auto viewM = img.At( mask );
   ref = viewM;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 3 * 3 * 3 } );
   DOCTEST_CHECK( ref.TensorElements() == 3 );
   DOCTEST_CHECK( dip::Count( ref[ 0 ] ) == 3*3*3 );

   // Indexing using coordinate array
   dip::CoordinateArray coords;
   coords.push_back( dip::UnsignedArray{ 0, 0, 0 } );
   coords.push_back( dip::UnsignedArray{ 1, 1, 1 } );
   coords.push_back( dip::UnsignedArray{ 0, 1, 1 } );
   coords.push_back( dip::UnsignedArray{ 1, 1, 0 } );
   auto viewC = img.At( coords );
   viewC = 2;
   ref = viewC;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 4 } );
   DOCTEST_CHECK( ref.TensorElements() == 3 );
   DOCTEST_CHECK( dip::Count( ref[ 0 ] == 2 ) == 4 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 2 ) == 4 );
   DOCTEST_CHECK( img.At( 0, 0, 0 )[ 0 ] == 2 );
   DOCTEST_CHECK( img.At( 1, 1, 1 )[ 0 ] == 2 );
   DOCTEST_CHECK( img.At( 0, 1, 1 )[ 0 ] == 2 );
   DOCTEST_CHECK( img.At( 1, 1, 0 )[ 0 ] == 2 );

   // -- Indexing into view

   // Regular view
   viewR.At( 1, 1, 0 ) = 3;
   DOCTEST_CHECK( img.At( 6, 6, 3 ) == 3 );
   auto viewRR = viewR.At( dip::Range{ 0, -1 }, dip::Range{ 0, 1 }, dip::Range{ 0 } );
   ref = viewRR;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 3, 2, 1 } );
   DOCTEST_CHECK( ref.TensorElements() == 3 );
   viewRR = 4;
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 4 ) == 3*2*1 );
   DOCTEST_CHECK( dip::Count( img[ 2 ] == 4 ) == 3*2*1 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 1 ) == 3*3*3 - 3*2*1 ); // we didn't write into pixels not in the view
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 3 ) == 0 );

   // Mask view
   DOCTEST_CHECK( viewM.At( 3 + 1 ) == 4 ); // indexed by viewRR
   DOCTEST_CHECK( viewM.At( 2 * 3 ) == 1 ); // not indexed by viewRR
   auto viewMR = viewM.At( dip::Range{ 1, 12, 2 } );
   viewMR = 5;
   ref = viewMR;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 6 } );
   DOCTEST_CHECK( ref.TensorElements() == 3 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 5 ) == 6 );
   DOCTEST_CHECK( dip::Count( img[ 1 ] == 5 ) == 6 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 4 ) == 3 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 1 ) == 3*3*3 - 6 - 3 ); // we didn't write into pixels not in the view

   // Coordinate array view
   auto viewCR = viewC.At( dip::Range{ 2, 2 } );
   viewCR = 6;
   DOCTEST_CHECK( dip::Count( viewC[ 0 ] == 2 ) == 3 );
   DOCTEST_CHECK( dip::Count( viewC[ 0 ] == 6 ) == 1 );
   DOCTEST_CHECK( viewC.At( 2 ) == 6 );
   DOCTEST_CHECK( viewC.At( 1 ) == 2 );
   DOCTEST_CHECK( dip::Count( img[ 0 ] == 6 ) == 1 );
   DOCTEST_CHECK( img.At( dip::UnsignedArray{ 0, 1, 1 } ) == 6 );
   ref = viewCR;
   DOCTEST_CHECK( ref.Sizes() == dip::UnsignedArray{ 1 } );

   // -- Writing an image into a view

   // Regular indexing
   img.Fill( 0 );
   ref = viewR;
   dip::Image src = ref.Similar();
   dip::ImageIterator< dip::sfloat > it( src );
   for( dip::uint ii = 1; it; ++ii, ++it ) {
      it[ 0 ] = static_cast< dip::sfloat >( ii );
      it[ 1 ] = static_cast< dip::sfloat >( ii + 1000 );
      it[ 2 ] = static_cast< dip::sfloat >( ii + 2000 );
   }
   viewR = src; // copy samples from src to view in img, now ref should match src
   DOCTEST_CHECK( dip::testing::CompareImages( ref, src ));

   // Indexing using mask image
   it.Reset();
   do {
      it[ 0 ] += 500.0f;
      it[ 1 ] += 500.0f;
      it[ 2 ] += 500.0f;
   } while( ++it );
   viewM = src; // copy samples from src to view in img, now ref should match src
   DOCTEST_CHECK( dip::testing::CompareImages( ref, src ));

   // Indexing using coordinate array
   src.ReForge( dip::UnsignedArray{ 4 }, 3 );
   it = dip::ImageIterator< dip::sfloat >( src );
   for( dip::uint ii = 1; it; ++ii, ++it ) {
      it[ 0 ] = static_cast< dip::sfloat >( ii );
      it[ 1 ] = static_cast< dip::sfloat >( ii + 1000 );
      it[ 2 ] = static_cast< dip::sfloat >( ii + 2000 );
   }
   viewC = src;
   DOCTEST_CHECK( img.At( 0, 0, 0 )[ 0 ] == 1 );
   DOCTEST_CHECK( img.At( 1, 1, 1 )[ 0 ] == 2 );
   DOCTEST_CHECK( img.At( 0, 1, 1 )[ 0 ] == 3 );
   DOCTEST_CHECK( img.At( 1, 1, 0 )[ 0 ] == 4 );
   DOCTEST_CHECK( img.At( 0, 0, 0 )[ 1 ] == 1 + 1000 );
   DOCTEST_CHECK( img.At( 1, 1, 1 )[ 1 ] == 2 + 1000 );
   DOCTEST_CHECK( img.At( 0, 1, 1 )[ 1 ] == 3 + 1000 );
   DOCTEST_CHECK( img.At( 1, 1, 0 )[ 1 ] == 4 + 1000 );
   DOCTEST_CHECK( img.At( 0, 0, 0 )[ 2 ] == 1 + 2000 );
   DOCTEST_CHECK( img.At( 1, 1, 1 )[ 2 ] == 2 + 2000 );
   DOCTEST_CHECK( img.At( 0, 1, 1 )[ 2 ] == 3 + 2000 );
   DOCTEST_CHECK( img.At( 1, 1, 0 )[ 2 ] == 4 + 2000 );

   // Writing a 2D image into a slice of a 3D image
   img.Fill( 0 );
   dip::Image slice{ dip::UnsignedArray{ 15, 20 }, 3 };
   it = dip::ImageIterator< dip::sfloat >( slice );
   for( dip::uint ii = 1; it; ++ii, ++it ) {
      it[ 0 ] = static_cast< dip::sfloat >( ii );
      it[ 1 ] = static_cast< dip::sfloat >( ii + 1000 );
      it[ 2 ] = static_cast< dip::sfloat >( ii + 2000 );
   }
   img.At( dip::Range(), dip::Range(), dip::Range( 3 ) ) = slice;
   DOCTEST_CHECK( dip::testing::CompareImages( dip::Image( img.At( dip::Range(), dip::Range(), dip::Range( 3 ))).Squeeze(), slice ));
}

#endif // DIP__ENABLE_DOCTEST
