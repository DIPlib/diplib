/*
 * DIPlib 3.0
 * This file contains support for nD iterators that are independent of image data type.
 *
 * (c)2016-2017, Cris Luengo.
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

#ifndef DIP_GENERIC_ITERATORS_H
#define DIP_GENERIC_ITERATORS_H

#include "diplib.h"
#include "diplib/iterators.h"


/// \file
/// \brief Defines image iterators that are independent of image data type.
/// \see iterators


namespace dip {


/// \addtogroup iterators
/// \{


/// \brief A data-type--agnostic version of `dip::ImageIterator`. Use this iterator only to write code that
/// does not know at compile-time what the data type of the image is.
///
/// This iterator works similarly to `dip::ImageIterator`. The `Pointer` method returns a `void` pointer to
/// the first sample in the pixel. This is the more efficient way of using the iterator.
///
/// Dereferencing the iterator returns a `dip::Image::Pixel` object (actually a `dip::Image::CastPixel`),
/// and the `[]` operator return a `dip::Image::Sample` object (actually a `dip::Image::CastSample`).
/// These objects reference the pixel or sample, assigning to them changes the
/// pixel's values in the image. They are convenient in use, but not very efficient. The optional template
/// argument to `%GenericImageIterator` sets the template argument to the `dip::Image::CastPixel` object
/// that is actually returned by dereferencing the iterator. Choose a type in which you wish to work, but
/// know that this choice will not affect the results of reading from and assigning to the dereferenced
/// iterator. The only difference is the type to which the dereferenced iterator can implicitly be cast to.
///
/// Example usage from `dip::Image::CopyAt`:
///
/// ```cpp
///     auto indIt = indices.begin();
///     GenericImageIterator<> srcIt( source );
///     do {
///        detail::CopyBuffer(
///              srcIt.Pointer(),
///              source.DataType(),
///              1, source.TensorStride(),
///              Pointer( coordinates( static_cast< dip::sint >( *indIt ))),
///              DataType(),
///              1, TensorStride(),
///              1, TensorElements()   // copy one pixel
///        );
///     } while( ++indIt, ++srcIt ); // srcIt is at the end, and determines the value of the `while` expression.
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, GenericJointImageIterator
template< typename T = dfloat >
class DIP_NO_EXPORT GenericImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image::CastPixel< T >; ///< The type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = value_type;       ///< The type of a reference to a pixel (note dip::Image::CastPixel references a value in the image)
      using pointer = value_type*;        ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericImageIterator() : atEnd_( true ) {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      GenericImageIterator( Image const& image, dip::uint procDim = std::numeric_limits< dip::uint >::max() ) :
            image_( &image ),
            offset_( 0 ),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            atEnd_( false ) {
         DIP_THROW_IF( !image_->IsForged(), E::IMAGE_NOT_FORGED );
      }

      /// Swap
      template< typename S >
      void swap( GenericImageIterator< S >& other ) {
         using std::swap;
         swap( image_, other.image_ );
         swap( offset_, other.offset_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( atEnd_, other.atEnd_ );
      }

      /// Dereference
      value_type operator*() const {
         return value_type( image_->Pointer( offset_ ), image_->DataType(), image_->Tensor(), image_->TensorStride() );
      }
      /// Dereference
      value_type operator->() const {
         return operator*();
      }
      /// Index into tensor, `it[index]` is equal to `(*it)[index]`.
      Image::CastSample< T > operator[]( dip::uint index ) const {
         DIP_ASSERT( image_ );
         return Image::CastSample< T >( image_->Pointer( offset_ + static_cast< dip::sint >( index ) * image_->TensorStride() ),
                                        image_->DataType() );
      }

      /// Increment
      GenericImageIterator& operator++() {
         DIP_ASSERT( image_ );
         dip::uint dd;
         for( dd = 0; dd < coords_.size(); ++dd ) {
            if( dd != procDim_ ) {
               // Increment coordinate and adjust offset
               ++coords_[ dd ];
               offset_ += image_->Stride( dd );
               // Check whether we reached the last pixel of the line ...
               if( coords_[ dd ] < image_->Size( dd ) ) {
                  break;
               }
               // Rewind, the next loop iteration will increment the next coordinate
               offset_ -= static_cast< dip::sint >( coords_[ dd ] ) * image_->Stride( dd );
               coords_[ dd ] = 0;
            }
         }
         if( dd == coords_.size() ) {
            atEnd_ = true;
         }
         return *this;
      }
      /// Increment
      GenericImageIterator operator++( int ) {
         GenericImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel, `it.begin()` is equal to `(*it).begin()`.
      typename value_type::Iterator begin() const {
         return typename value_type::Iterator( Pointer(), image_->DataType(), image_->TensorStride() );
      }
      /// Get an end iterator over the tensor for the current pixel
      typename value_type::Iterator end() const {
         return typename value_type::Iterator( Pointer(), image_->DataType(), image_->TensorStride(), image_->TensorElements() );
      }
      /// Get an iterator over the current line
      template< typename S = T >
      LineIterator< S > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< S >( *image_, coords_, procDim_ );
      }
      /// Get a const iterator over the current line
      template< typename S = T >
      ConstLineIterator< S > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< S >( *image_, coords_, procDim_ );
      }

      /// Equality comparison, is equal if the two iterators have the same coordinates. It is possible to compare
      /// GenericImageIterator with different images.
      template< typename S >
      bool operator==( GenericImageIterator< S > const& other ) const {
         return ( atEnd_ == other.atEnd_ ) && ( coords_ == other.coords_ );
      }
      /// Inequality comparison
      template< typename S >
      bool operator!=( GenericImageIterator< S > const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return atEnd_; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !atEnd_; }

      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( image_ );
         DIP_ASSERT( coords.size() == image_->Dimensionality() );
         if( HasProcessingDimension() ) {
            coords[ procDim_ ] = 0;
         }
         offset_ = image_->Offset( coords ); // tests for coords to be correct
         coords_ = coords;
      }

      /// Return the current pointer
      void* Pointer() const {
         DIP_ASSERT( image_ );
         return image_->Pointer( offset_ );
      }
      /// Return a pointer to the tensor element `index`
      void* Pointer( dip::uint index ) const {
         DIP_ASSERT( image_ );
         return image_->Pointer( offset_ + static_cast< dip::sint >( index ) * image_->TensorStride() );
      }
      /// Return the current offset
      dip::sint Offset() const { return offset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         DIP_ASSERT( image_ );
         return image_->Index( coords_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( image_ ) {
            return procDim_ < image_->Dimensionality();
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const {
         return HasProcessingDimension() ? static_cast< dip::sint >( procDim_ ) : -1;
      }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      void Reset() {
         offset_ = 0;
         coords_.fill( 0 );
      }

   private:
      Image const* image_ = nullptr;
      dip::sint offset_ = 0;
      UnsignedArray coords_;
      dip::uint procDim_;
      bool atEnd_;
};

template< typename T, typename S >
inline void swap( GenericImageIterator< T >& v1, GenericImageIterator< S >& v2 ) {
   v1.swap( v2 );
}

inline GenericImageIterator< dip::dfloat > Image::begin() {
   return GenericImageIterator< dip::dfloat >( *this );
}

inline GenericImageIterator< dip::dfloat > Image::end() {
   return GenericImageIterator< dip::dfloat >();
}



/// \brief A data-type--agnostic version of `dip::JointImageIterator`. Use this iterator only to write code that
/// does not know at compile-time what the data type of the image is.
///
/// This iterator works similarly to `dip::JointImageIterator`. The `Pointer<N>` method returns a `void`
/// pointer to the first sample in the pixel for image `N`. This is the more efficient way of using the
/// iterator.
///
/// The `Sample<N>` method returns a `dip::Image::Sample` object. This object references the sample, so that
/// assigning to it changes the samples's value in the image. It is convenient in use, but not very efficient.
/// The optional template argument to `%GenericJointImageIterator` sets the template argument to the
/// `dip::Image::CastSample` object that is actually returned by the method. Choose a type in which you wish
/// to work, but know that this choice will not affect the results of reading from and assigning to the
/// samples. The only difference is the type to which the output can implicitly be cast to.
///
/// Example usage from `dip::Image::Copy`:
///
/// ```cpp
///     dip::uint processingDim = Framework::OptimalProcessingDim( src );
///     auto it = dip::GenericJointImageIterator< 2 >( { src, *this }, processingDim );
///     do {
///        detail::CopyBuffer(
///              it.InPointer(),
///              src.dataType_,
///              src.strides_[ processingDim ],
///              src.tensorStride_,
///              it.OutPointer(),
///              dataType_,
///              strides_[ processingDim ],
///              tensorStride_,
///              sizes_[ processingDim ],
///              tensor_.Elements()
///        );
///     } while( ++it );
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see JointImageIterator, GenericImageIterator
template< dip::uint N, typename T = dfloat >
class DIP_NO_EXPORT GenericJointImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image::CastPixel< T >; ///< The type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = value_type;       ///< The type of a reference to a pixel (note dip::Image::CastPixel references a value in the image)
      using pointer = value_type*;        ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericJointImageIterator() : atEnd_( true ) {
         images_.fill( nullptr );
         offsets_.fill( 0 );
      }
      /// To construct a useful iterator, provide two images, and optionally a processing dimension
      GenericJointImageIterator( ImageConstRefArray const& images, dip::uint procDim = std::numeric_limits< dip::uint >::max() ):
            procDim_( procDim ), atEnd_( false ) {
         DIP_THROW_IF( images.size() != N, E::ARRAY_ILLEGAL_SIZE );
         images_[ 0 ] = &( images[ 0 ].get() );
         DIP_THROW_IF( !images_[ 0 ]->IsForged(), E::IMAGE_NOT_FORGED );
         coords_.resize( images_[ 0 ]->Dimensionality(), 0 );
         dummy_.SetStrides( IntegerArray( coords_.size(), 0 ));
         offsets_.fill( 0 );
         for( dip::uint ii = 1; ii < N; ++ii ) {
            images_[ ii ] = &( images[ ii ].get() );
            if( !images_[ ii ]->IsForged() ) {
               images_[ ii ] = &dummy_;
            }
         }
      }

      /// Swap
      template< typename S >
      void swap( GenericJointImageIterator< N, S >& other ) {
         using std::swap;
         swap( images_, other.images_ );
         swap( offsets_, other.offsets_ );
         swap( dummy_, other.dummy_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( atEnd_, other.atEnd_ );
      }

      /// Index into image tensor for image `I`
      template< dip::uint I >
      Image::CastSample< T > Sample( dip::uint index ) const {
         return Image::CastSample< T >( Pointer< I >( index ), images_[ I ]->DataType() );
      }
      /// Index into image tensor for image 0.
      Image::CastSample< T > InSample( dip::uint index ) const { return Sample< 0 >( index ); }
      /// Index into image tensor for image 1.
      Image::CastSample< T > OutSample( dip::uint index ) const { return Sample< 1 >( index ); }
      /// Get first tensor element for image `I`.
      template< dip::uint I >
      Image::CastSample< T > Sample() const {
         return Image::CastSample< T >( Pointer< I >(), images_[ I ]->DataType() );
      }
      /// Get pixel for image 0.
      value_type In() const { return Pixel< 0 >(); }
      /// Get pixel for image 1.
      value_type Out() const { return Pixel< 1 >(); }
      /// Get pixel for image `I`.
      template< dip::uint I >
      value_type Pixel() const {
         return value_type( Pointer< I >(), images_[ I ]->DataType(), images_[ I ]->Tensor(), images_[ I ]->TensorStride() );
      }

      /// Increment
      GenericJointImageIterator& operator++() {
         if( *this ) {
            dip::uint dd;
            for( dd = 0; dd < coords_.size(); ++dd ) {
               if( dd != procDim_ ) {
                  // Increment coordinate and adjust pointer
                  ++coords_[ dd ];
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     offsets_[ ii ] += images_[ ii ]->Stride( dd );
                  }
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < images_[ 0 ]->Size( dd )) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     offsets_[ ii ] -= static_cast< dip::sint >( coords_[ dd ] ) * images_[ ii ]->Stride( dd );
                  }
                  coords_[ dd ] = 0;
               }
            }
            if( dd == coords_.size() ) {
               atEnd_ = true;
            }
         }
         return *this;
      }
      /// Increment
      GenericJointImageIterator operator++( int ) {
         GenericJointImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      typename value_type::Iterator begin() const {
         DIP_ASSERT( images_[ I ] );
         return typename value_type::Iterator( Pointer< I >(), images_[ I ]->DataType(), images_[ I ]->TensorStride() );
      }
      /// Get an end iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      typename value_type::Iterator end() const {
         DIP_ASSERT( images_[ I ] );
         return typename value_type::Iterator( Pointer< I >(), images_[ I ]->DataType(), images_[ I ]->TensorStride(), images_[ I ]->TensorElements() );
      }
      /// Get an iterator over the current line of image `I`
      template< dip::uint I, typename S = T >
      LineIterator< S > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< S >( *images_[ I ], coords_, procDim_ );
      }
      /// Get a const iterator over the current line of image `I`
      template< dip::uint I, typename S = T >
      ConstLineIterator< S > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< S >( *images_[ I ], coords_, procDim_ );
      }

      /// Equality comparison, is equal if the two iterators have the same coordinates.
      template< typename S >
      bool operator==( GenericJointImageIterator< N, S > const& other ) const {
         return ( atEnd_ == other.atEnd_ ) && ( coords_ == other.coords_ );
      }
      /// Inequality comparison
      template< typename S >
      bool operator!=( GenericJointImageIterator< N, S > const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return atEnd_; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !atEnd_; }

      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( images_[ 0 ] );
         DIP_ASSERT( coords.size() == images_[ 0 ]->Dimensionality() );
         if( HasProcessingDimension() ) {
            coords[ procDim_ ] = 0;
         }
         for( dip::uint ii = 0; ii < N; ++ii ) {
            offsets_[ ii ] = images_[ ii ]->Offset( coords );
         }
         coords_ = coords;
      }

      /// Index into image tensor for image `I`
      template< dip::uint I >
      void* Pointer( dip::uint index ) const {
         DIP_ASSERT( images_[ I ] );
         return images_[ I ]->Pointer( offsets_[ I ] + static_cast< dip::sint >( index ) * images_[ I ]->TensorStride() );
      }
      /// Index into image tensor for image 0.
      void* InPointer( dip::uint index ) const { return Pointer< 0 >( index ); }
      /// Index into image tensor for image 1.
      void* OutPointer( dip::uint index ) const { return Pointer< 1 >( index ); }
      /// Return the current pointer for image `I`
      template< dip::uint I >
      void* Pointer() const {
         DIP_ASSERT( images_[ I ] );
         return images_[ I ]->Pointer( offsets_[ I ] );
      }
      /// Return the current pointer for image 0.
      void* InPointer() const { return Pointer< 0 >(); }
      /// Return the current pointer for image 1.
      void* OutPointer() const { return Pointer< 1 >(); }
      /// Return the current offset for image `I`
      template< dip::uint I >
      dip::sint Offset() const { return offsets_[ I ]; }
      /// Index into image tensor for image 0.
      dip::sint InOffset() const { return offsets_[ 0 ]; }
      /// Index into image tensor for image 1.
      dip::sint OutOffset() const { return offsets_[ 1 ]; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         DIP_ASSERT( images_[ 0 ] );
         return images_[ 0 ]->Index( coords_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( images_[ 0 ] ) {
            return procDim_ < images_[ 0 ]->Dimensionality();
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? static_cast< dip::sint >( procDim_ ) : -1; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      void Reset() {
         offsets_.fill( 0 );
         coords_.fill( 0 );
      }

   private:
      static_assert( N > 1, "GenericJointImageIterator needs at least one type template argument" );
      std::array< Image const*, N > images_;
      std::array< dip::sint, N > offsets_;
      Image dummy_;
      UnsignedArray coords_;
      dip::uint procDim_;
      bool atEnd_;

      // Compares size of image n to first
      bool CompareSizes( dip::uint n ) const {
         if( images_[ 0 ]->Dimensionality() != images_[ n ]->Dimensionality() ) {
            return false;
         }
         for( dip::uint ii = 0; ii < images_[ 0 ]->Dimensionality(); ++ii ) {
            if(( ii != procDim_ ) && ( images_[ 0 ]->Size( ii ) != images_[ n ]->Size( ii ))) {
               return false;
            }
         }
         return true;
      }
};

template< dip::uint N, typename T, typename S >
inline void swap( GenericJointImageIterator< N, T >& v1, GenericJointImageIterator< N, S >& v2 ) {
   v1.swap( v2 );
}


/// \brief An iterator for slice-by-slice processing of an image. Use it to process a multi-dimensional
/// image as a series of lower-dimensional images.
///
/// Dereferencing the iterator yields a reference to an image that encapsulates a plane in the
/// original image. This image has the protected flag set so that it cannot be stripped or reforged.
///
/// The iterator can be moved to any arbitrary slice with a non-negative index (so you cannot decrement
/// it below 0, the first slice; if you try nothing will happen), even slices past the last one.
/// If the iterator points at a slice that does not exist, the iterator will test false, but it will
/// still be a valid iterator that can be manipulated. Do not dereference such an iterator!
///
/// ```cpp
///     dip::ImageSliceIterator it( img, 2 );
///     do {
///        // do something with the image *it here.
///     } while( ++it );
/// ```
///
/// The function `dip::ImageSliceEndIterator` creates an iterator that points at a slice one past
/// the last, and so is a end iterator. Because it is not possible to decrement below 0, a loop that
/// iterates in reverse order must test the `dip::ImageSliceIterator::Coordinate()` for equality to
/// zero:
///
/// ```cpp
///     dip::ImageSliceEndIterator it( img, 2 );
///     do {
///        --it;
///        // do something with the image *it here.
///     } while( it.Coordinate() != 0 );
/// ```
///
/// Note that when the original image is stripped or reforged, the iterator is still valid and
/// holds on to the original data segment.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
/// Additionally, it behaves like a RandomAccessIterator except for the indexing operator `[]`,
/// which would be less efficient in use and therefore it's better to not offer it.
///
/// \see ImageTensorIterator, ImageIterator, JointImageIterator, GenericImageIterator, GenericJointImageIterator
class DIP_NO_EXPORT ImageSliceIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image;           ///< The type obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = Image&;           ///< The type of a reference to `value_type`
      using pointer = Image*;             ///< The type of a pointer to `value_type`

      /// Default constructor yields an invalid iterator that cannot be dereferenced or used in any way
      ImageSliceIterator() {}
      /// To construct a useful iterator, provide an image and a processing dimension
      ImageSliceIterator( Image const& image, dip::uint procDim ) :
            procDim_( procDim ) {
         DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( procDim_ >= image.Dimensionality(), E::ILLEGAL_DIMENSION );
         size_ = image.Size( procDim_ );
         stride_ = image.Stride( procDim_ );
         // copy image with shared data
         image_ = image;
         // remove the processing dimension
         UnsignedArray sizes = image_.Sizes();
         sizes[ procDim_ ] = 1;
         image_.dip__SetSizes( sizes );
         image_.Squeeze( procDim_ );
         // protect the image to avoid modifications
         image_.Protect();
      }

      /// Swap
      void swap( ImageSliceIterator& other ) {
         using std::swap;
         swap( image_, other.image_ );
         swap( size_, other.size_ );
         swap( stride_, other.stride_ );
         swap( coord_, other.coord_ );
         swap( procDim_, other.procDim_ );
      }

      /// Dereference
      Image& operator*() { return image_; }
      /// Dereference
      Image* operator->() { return &image_; }

      /// Increment
      ImageSliceIterator& operator++() {
         DIP_THROW_IF( !IsValid(), E::ITERATOR_NOT_VALID );
         ++coord_;
         image_.dip__ShiftOrigin( stride_ );
         return *this;
      }
      /// Increment
      ImageSliceIterator operator++( int ) {
         ImageSliceIterator tmp( *this );
         operator++();
         return tmp;
      }
      /// Decrement, but never past the first slide
      ImageSliceIterator& operator--() {
         DIP_THROW_IF( !IsValid(), E::ITERATOR_NOT_VALID );
         if( coord_ != 0 ) {
            --coord_;
            image_.dip__ShiftOrigin( -stride_ );
         }
         return *this;
      }
      /// Decrement, but never past the first slide
      ImageSliceIterator operator--( int ) {
         ImageSliceIterator tmp( *this );
         operator--();
         return tmp;
      }
      /// Increment by `n`
      ImageSliceIterator& operator+=( difference_type n ) {
         DIP_THROW_IF( !IsValid(), E::ITERATOR_NOT_VALID );
         if( n < 0 ) {
            dip::uint nn = std::min( coord_, static_cast< dip::uint >( -n ));
            coord_ -= nn;
            image_.dip__ShiftOrigin( -static_cast< dip::sint >( nn ) * stride_ );
         } else {
            coord_ += static_cast< dip::uint >( n );
            image_.dip__ShiftOrigin( n * stride_ );
         }
         return * this;
      }
      /// Increment by `n`
      ImageSliceIterator& operator+=( dip::uint n ) {
         return operator+=( static_cast< difference_type >( n ));
      }
      /// Decrement by `n`, but never moves the iterator to before the first slide
      ImageSliceIterator& operator-=( difference_type n ) {
         return operator+=( -n );
      }
      /// Decrement by `n`, but never moves the iterator to before the first slide
      ImageSliceIterator& operator-=( dip::uint n ) {
         return operator+=( -static_cast< difference_type >( n ));
      }

      /// Difference between iterators
      difference_type operator-( ImageSliceIterator const& it2 ) const {
         DIP_THROW_IF( !IsValid() || !it2.IsValid(), E::ITERATOR_NOT_VALID );
         DIP_THROW_IF(( image_.Data() != it2.image_.Data() ) ||
                      ( image_.Sizes() != it2.image_.Sizes() ) ||
                      ( stride_ != it2.stride_ ) ||
                      ( procDim_ != it2.procDim_ ), "Iterators index in different images or along different dimensions" );
         return static_cast< difference_type >( coord_ ) - static_cast< difference_type >( it2.coord_ );
      }

      /// Equality comparison
      bool operator==( ImageSliceIterator const& other ) const {
         return image_.Origin() == other.image_.Origin();
      }
      /// Inequality comparison
      bool operator!=( ImageSliceIterator const& other ) const {
         return !operator==( other );
      }

      // Comparison operators below implemented in terms of `operator-` because it tests to make sure the iterators are comparable
      /// Larger than comparison
      bool operator>( ImageSliceIterator const& other ) const { return ( *this - other ) > 0; }
      /// Smaller than comparison
      bool operator<( ImageSliceIterator const& other ) const { return ( *this - other ) < 0; }
      /// Not smaller than comparison
      bool operator>=( ImageSliceIterator const& other ) const { return ( *this - other ) >= 0; }
      /// Not larger than comparison
      bool operator<=( ImageSliceIterator const& other ) const { return ( *this - other ) <= 0; }

      /// Test to see if the iterator is valid (i.e. not default-constructed); it can still be at end, and thus not dereferenceable
      bool IsValid() const { return size_ > 0; }
      /// Test to see if the iterator reached past the last plane
      bool IsAtEnd() const { return coord_ >= size_; }
      /// Test to see if the iterator is valid and can be dereferenced
      explicit operator bool() const { return !IsAtEnd(); }

      /// Return the current position
      dip::uint Coordinate() const { return coord_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinate( dip::uint coord ) {
         DIP_THROW_IF( coord >= size_, E::INDEX_OUT_OF_RANGE );
         coord_ = coord;
      }

      /// Return the processing dimension, the direction over which the iterator iterates
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first image plane (as it was when the iterator first was created)
      void Reset() {
         coord_ = 0;
      }

      /// Set the iterator to index `plane`. If `plane` is outside the image domain, the iterator is still valid,
      /// but should not be dereferenced.
      void Set( dip::uint plane ) {
         coord_ = plane;
      }

   private:
      Image image_;           // the image whose reference we return when dereferencing
      dip::uint size_ = 0;    // will always be > 0 when not default-constructed
      dip::sint stride_ = 0;
      dip::uint coord_ = 0;   // the plane currently pointing to
      dip::uint procDim_ = 0; // the dimension along which we iterate, the image contains all other dimensions
};

/// \brief Increment an image slice iterator by `n`
inline ImageSliceIterator operator+( ImageSliceIterator it, dip::sint n ) {
   it += n;
   return it;
}
/// \brief Increment an image slice iterator by `n`
inline ImageSliceIterator operator+( ImageSliceIterator it, dip::uint n ) {
   return operator+( it, static_cast< dip::sint >( n ));
}
/// \brief Decrement an image slice iterator by `n`, but never moves the iterator to before the first slide
inline ImageSliceIterator operator-( ImageSliceIterator it, dip::sint n ) {
   it -= n;
   return it;
}
/// \brief Decrement an image slice iterator by `n`, but never moves the iterator to before the first slide
inline ImageSliceIterator operator-( ImageSliceIterator it, dip::uint n ) {
   return operator-( it, static_cast< dip::sint >( n ));
}

inline void swap( ImageSliceIterator& v1, ImageSliceIterator& v2 ) {
   v1.swap( v2 );
}

/// Constructs an end iterator corresponding to a `dip::ImageSliceIterator`
inline ImageSliceIterator ImageSliceEndIterator( Image const& image, dip::uint procDim ) {
   ImageSliceIterator out { image, procDim }; // Tests for `procDim` to be OK
   out += static_cast< dip::sint >( image.Size( procDim ));
   return out;
}


/// \brief An iterator for element-by-element processing of a tensor image. Use it to process a tensor
/// image as a series of scalar images.
///
/// This iterator is implemented as a `dip::ImageSliceIterator`, see that iterator's documentation for further
/// information. When the iterator is dereferenced it yields a scalar image of the same size as the input image.
/// Each of the tensor elements is visited in the order in which it is stored. For the case of symmetric and
/// triangular tensors, this means that fewer tensor elements will be visited. See `dip::Tensor` for information
/// on storage order.
///
/// ```cpp
///     auto it = dip::ImageTensorIterator( img );
///     do {
///        // do something with the image *it here.
///     } while( ++it );
/// ```
///
/// Note that when the original image is stripped or reforged, the iterator is still valid and
/// holds on to the original data segment.
///
/// \see ImageSliceIterator, ImageIterator, JointImageIterator, GenericImageIterator, GenericJointImageIterator
inline ImageSliceIterator ImageTensorIterator( Image const& image ) {
   Image tmp = image;
   dip::uint dim = tmp.Dimensionality();
   tmp.TensorToSpatial( dim ); // adds the tensor dimension as the last dimension, the same as -1.
   return ImageSliceIterator( tmp, dim );
}


/// \}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing ImageIterator and GenericImageIterator") {
   dip::Image img{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_UINT16 };
   {
      DOCTEST_REQUIRE( img.DataType() == dip::DT_UINT16 );
      dip::ImageIterator< dip::uint16 > it( img );
      dip::uint16 counter = 0;
      do {
         *it = static_cast< dip::uint16 >( counter++ );
      } while( ++it );
   }
   {
      dip::GenericImageIterator< dip::sint32 > it( img );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( *it == counter++ );
      } while( ++it );
   }
   dip::Image img2{ dip::UnsignedArray{ 3, 4 }, 3, dip::DT_SINT32 };
   {
      DOCTEST_REQUIRE( img2.DataType() == dip::DT_SINT32 );
      dip::ImageIterator< dip::sint32 > it( img2 );
      dip::sint32 counter = 0;
      do {
         it[ 0 ] = static_cast< dip::sint32 >( counter );
         it[ 1 ] = static_cast< dip::sint32 >( counter * 1000 );
         it[ 2 ] = static_cast< dip::sint32 >( counter * -10000 );
         ++counter;
      } while( ++it );
   }
   {
      dip::GenericImageIterator< dip::sint32 > it( img2 );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( it[ 0 ] == counter );
         DOCTEST_CHECK( it[ 1 ] == counter * 1000 );
         DOCTEST_CHECK( it[ 2 ] == counter * -10000 );
         ++counter;
      } while( ++it );
   }
   {
      dip::GenericImageIterator< dip::sint32 > it( img2 );
      ++it;
      auto iit = it.begin();
      DOCTEST_CHECK( *iit == 1 );
      ++iit;
      DOCTEST_CHECK( *iit == 1000 );
      ++iit;
      DOCTEST_CHECK( *iit == -10000 );
      ++iit;
      DOCTEST_CHECK( iit == it.end() );
   }
}

#endif // DIP__ENABLE_DOCTEST

#endif // DIP_GENERIC_ITERATORS_H
