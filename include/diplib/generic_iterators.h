/*
 * DIPlib 3.0
 * This file contains support for 1D and nD iterators.
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

#include <utility>

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Defines image iterators that are independent of image data type.
/// \see iterators


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief A data-type--agnostic version of `dip::ImageIterator`. Use this iterator only to write code that
/// does not know at compile-time what the data type of the image is.
///
/// This iterator works similarly to `dip::ImageIterator` except it is not dereferenceable. Use the
/// `Pointer` method to obtain a `void` pointer to the first sample in the pixel. The `[]` operator returns
/// a `void` pointer to any other sample.
///
/// It is not possible to obtain line or sample iterators from this iterator, and it has no support for
/// accessing pixels in the neighborhood of the referenced pixel.
///
/// Example usage from `dip::Image::Fill`:
///
/// ```cpp
///     dip::uint processingDim = Framework::OptimalProcessingDim( dest );
///     auto it = GenericImageIterator( dest, processingDim );
///        do {
///        FillBuffer(
///              it.Pointer(),
///              dest.DataType(),
///              dest.Stride( processingDim ),
///              dest.TensorStride(),
///              dest.Size( processingDim ),
///              dest.TensorElements(),
///              v
///        );
///     } while( ++it );
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, GenericJointImageIterator
class DIP_NO_EXPORT GenericImageIterator {
   public:

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericImageIterator() {}
      /// To construct a useful iterator, provide an image and a processing dimension
      GenericImageIterator( Image const& image, dip::uint procDim ) :
            image_( &image ),
            offset_( 0 ),
            coords_( image.Dimensionality(), 0 ),
            procDim_( static_cast< dip::sint >( procDim )) {
         DIP_THROW_IF( !image_->IsForged(), E::IMAGE_NOT_FORGED );
      }
      /// To construct a useful iterator, provide an image
      GenericImageIterator( Image const& image ) :
            image_( &image ),
            offset_( 0 ),
            coords_( image.Dimensionality(), 0 ) {
         DIP_THROW_IF( !image_->IsForged(), E::IMAGE_NOT_FORGED );
      }

      /// Swap
      void swap( GenericImageIterator& other ) {
         using std::swap;
         swap( image_, other.image_ );
         swap( offset_, other.offset_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
      }

      /// Tensor indexing returns a pointer to the sample
      void* operator[]( dip::sint index ) const {
         return Sample( index );
      }
      /// Pointer to a sample of the current pixel
      void* Sample( dip::sint index ) const {
         DIP_THROW_IF( !image_, E::ITERATOR_NOT_VALID );
         return image_->Pointer( offset_ + index * image_->TensorStride() );
      }

      /// Increment
      GenericImageIterator& operator++() {
         DIP_THROW_IF( !image_, E::ITERATOR_NOT_VALID );
         dip::uint dd;
         for( dd = 0; dd < coords_.size(); ++dd ) {
            if( static_cast< dip::sint >( dd ) != procDim_ ) {
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
            image_ = nullptr;
         }
         return *this;
      }
      /// Increment
      GenericImageIterator operator++( int ) {
         GenericImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Equality comparison
      bool operator==( GenericImageIterator const& other ) const {
         return ( image_ == other.image_ ) && (( image_ == nullptr ) || ( offset_ == other.offset_ ));
      }
      /// Inequality comparison
      bool operator!=( GenericImageIterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return image_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return image_ != nullptr; }

      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray const& coords ) {
         DIP_THROW_IF( !image_, E::ITERATOR_NOT_VALID );
         offset_ = image_->Offset( coords ); // tests for coords to be correct
         coords_ = coords;
      }

      /// Return the current pointer
      void* Pointer() const {
         DIP_THROW_IF( !image_, E::ITERATOR_NOT_VALID );
         return image_->Pointer( offset_ );
      }
      /// Return the current offset
      dip::sint Offset() const { return offset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         DIP_THROW_IF( !image_, E::ITERATOR_NOT_VALID );
         return image_->Index( coords_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( image_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < static_cast< dip::sint >( image_->Dimensionality() ));
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? procDim_ : -1; }

   private:
      Image const* image_ = nullptr;
      dip::sint offset_ = 0;
      UnsignedArray coords_ = {};
      dip::sint procDim_ = -1;
};

inline void swap( GenericImageIterator& v1, GenericImageIterator& v2 ) {
   v1.swap( v2 );
}


/// \brief A data-type--agnostic version of `dip::JointImageIterator`. Use this iterator only to write code that
/// does not know at compile-time what the data type of the image is.
///
/// This iterator works similarly to `dip::JointImageIterator` except it does not have the `In` and `Out`
/// methods to obtain references to samples. Instead, use the `InPointer` and `OutPointer` methods to obtain
/// a `void` pointer to the first sample in the pixels. The `InSample` and `OutSample` methods return
/// a `void` pointer to any other sample. Note that `In` and `Out` could also have been named `First` and
/// `Second` -- there is no need for one image to be input and out to be output.
///
/// It is not possible to obtain line or sample iterators from this iterator, and it has no support for
/// accessing pixels in the neighborhood of the referenced pixel.
///
/// Example usage from `dip::Image::Copy`:
///
/// ```cpp
///     dip::uint processingDim = Framework::OptimalProcessingDim( src );
///     auto it = dip::GenericJointImageIterator( src, *this, processingDim );
///     do {
///        CopyBuffer(
///              it.InPointer(),
///              src.dataType_,
///              src.strides_[ processingDim ],
///              src.tensorStride_,
///              it.OutPointer(),
///              dataType_,
///              strides_[ processingDim ],
///              tensorStride_,
///              sizes_[ processingDim ],
///              tensor_.Elements(),
///              std::vector< dip::sint > {}
///        );
///     } while( ++it );
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see JointImageIterator, GenericImageIterator
class DIP_NO_EXPORT GenericJointImageIterator {
   public:

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericJointImageIterator() {}
      /// To construct a useful iterator, provide two images, and a processing dimension
      GenericJointImageIterator( Image const& input, Image const& output, dip::uint procDim ) :
            inImage_( &input ),
            outImage_( &output ),
            inOffset_( 0 ),
            outOffset_( 0 ),
            coords_( input.Dimensionality(), 0 ),
            procDim_( static_cast< dip::sint >( procDim )) {
         DIP_THROW_IF( !inImage_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !outImage_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !CompareSizes(), E::SIZES_DONT_MATCH );
      }
      /// To construct a useful iterator, provide two images
      GenericJointImageIterator( Image const& input, Image const& output ) :
            inImage_( &input ),
            outImage_( &output ),
            inOffset_( 0 ),
            outOffset_( 0 ),
            coords_( input.Dimensionality(), 0 ) {
         DIP_THROW_IF( !inImage_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !outImage_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !CompareSizes(), E::SIZES_DONT_MATCH );
      }

      /// Swap
      void swap( GenericJointImageIterator& other ) {
         using std::swap;
         swap( inImage_, other.inImage_ );
         swap( outImage_, other.outImage_ );
         swap( inOffset_, other.inOffset_ );
         swap( outOffset_, other.outOffset_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
      }

      /// Pointer to a sample of the current pixel for the in image.
      void* InSample( dip::sint index ) const {
         DIP_THROW_IF( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Pointer( inOffset_ + index * inImage_->TensorStride() );
      }
      /// Pointer to a sample of the current pixel for the out image.
      void* OutSample( dip::sint index ) const {
         DIP_THROW_IF( !outImage_, E::ITERATOR_NOT_VALID );
         return outImage_->Pointer( outOffset_ + index * outImage_->TensorStride() );
      }

      /// Increment
      GenericJointImageIterator& operator++() {
         DIP_THROW_IF( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
         dip::uint dd;
         for( dd = 0; dd < coords_.size(); ++dd ) {
            if( static_cast< dip::sint >( dd ) != procDim_ ) {
               // Increment coordinate and adjust offsets
               ++coords_[ dd ];
               inOffset_ += inImage_->Stride( dd );
               outOffset_ += outImage_->Stride( dd );
               // Check whether we reached the last pixel of the line
               if( coords_[ dd ] < inImage_->Size( dd ) ) {
                  break;
               }
               // Rewind, the next loop iteration will increment the next coordinate
               inOffset_ -= static_cast< dip::sint >( coords_[ dd ] ) * inImage_->Stride( dd );
               outOffset_ -= static_cast< dip::sint >( coords_[ dd ] ) * outImage_->Stride( dd );
               coords_[ dd ] = 0;
            }
         }
         if( dd == coords_.size() ) {
            inImage_ = nullptr;
            outImage_ = nullptr;
         }
         return *this;
      }
      /// Increment
      GenericJointImageIterator operator++( int ) {
         GenericJointImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Equality comparison
      bool operator==( GenericJointImageIterator const& other ) const {
         return ( inImage_ == other.inImage_ ) && (( inImage_ == nullptr ) || ( inOffset_ == other.inOffset_ )) &&
                ( outImage_ == other.outImage_ ) && (( outImage_ == nullptr ) || ( outOffset_ == other.outOffset_ ));
      }
      /// Inequality comparison
      bool operator!=( GenericJointImageIterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return inImage_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return inImage_ != nullptr; }

      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray const& coords ) {
         DIP_THROW_IF( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
         inOffset_ = inImage_->Offset( coords ); // tests for coords to be correct
         outOffset_ = outImage_->Offset( coords );
         coords_ = coords;
      }

      /// Return the current pointer for the input image
      void* InPointer() const {
         DIP_THROW_IF( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Pointer( inOffset_ );
      }
      /// Return the current pointer for the output image
      void* OutPointer() const {
         DIP_THROW_IF( !outImage_, E::ITERATOR_NOT_VALID );
         return outImage_->Pointer( outOffset_ );
      }
      /// Return the current offset for the input image
      dip::sint InOffset() const { return inOffset_; }
      /// Return the current offset for the output image
      dip::sint OutOffset() const { return outOffset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         DIP_THROW_IF( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Index( coords_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( inImage_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < static_cast< dip::sint >( inImage_->Dimensionality() ));
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? procDim_ : -1; }

   private:
      Image const* inImage_ = nullptr;
      Image const* outImage_ = nullptr;
      dip::sint inOffset_ = 0;
      dip::sint outOffset_ = 0;
      UnsignedArray coords_ = {};
      dip::sint procDim_ = -1;
      bool CompareSizes() const {
         if( inImage_->Dimensionality() != outImage_->Dimensionality() ) {
            return false;
         }
         for( dip::uint ii = 0; ii < inImage_->Dimensionality(); ++ii ) {
            if(( static_cast< dip::sint >( ii ) != procDim_ ) && ( inImage_->Size( ii ) != outImage_->Size( ii ) )) {
               return false;
            }
         }
         return true;
      }
};

inline void swap( GenericJointImageIterator& v1, GenericJointImageIterator& v2 ) {
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
/// still be a valid iterator that can be manipulated.
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
         DIP_THROW_IF( !IsValid(), E::ITERATOR_NOT_VALID );
         DIP_THROW_IF( coord >= size_, E::INDEX_OUT_OF_RANGE );
         coord_ = coord;
      }

      /// Return the processing dimension, the direction over which the iterator iterates
      dip::uint ProcessingDimension() const { return procDim_; }

   private:
      Image image_;           // the image whose reference we return when dereferencing
      dip::uint size_ = 0;    // will always be > 0 when not default-constructed
      dip::sint stride_ = 0;  // will always be > 0 when not default-constructed
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
///     dip::ImageTensorIterator it( img );
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

#endif // DIP_GENERIC_ITERATORS_H
