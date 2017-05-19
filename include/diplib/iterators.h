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

#ifndef DIP_ITERATORS_H
#define DIP_ITERATORS_H

#include <utility>
#include <tuple>

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Defines image iterators and line iterators.
/// \see iterators


namespace dip {


/// \defgroup iterators Iterators
/// \ingroup infrastructure
/// \brief Objects to iterate over images and image lines in different ways.
///
/// See \ref using_iterators "Using iterators to implement filters"
/// for a mini-tutorial on how to use each of the different iterator types. Next, read the documentation
/// for the iterator you plan to use, to learn about additional options and possibilities.
/// \{


//
// Line iterator, does 1D loop over the pixels in an image line
//


/// \brief An iterator to iterate over all pixels of an image line.
///
/// This iterator is constructed from a `dip::ImageIterator`, but can also be constructed manually
/// if necessary.
///
/// The iterator can be incremented until it reaches the end of the line. At this point, the
/// iterator will become invalid. An invalid iterator will test false. The `IsAtEnd` method
/// can be used instead to test for this condition. It is also possible to compare two iterators
/// for equality (i.e. to compare against an end iterator).
///
/// Dereferencing the iterator yields the first sample of the current pixel (`*it` == `it[ 0 ]`).
/// One can index using the `[]` operator to obtain each of the samples of the tensor
/// (`it[ 0 ]` .. `it[ image.TensorElements() - 1 ]`).
///
/// Alternatively, a `dip::SampleIterator` can be obtained to iterate over the samples of the tensor
/// (`it.begin()` .. `it.end()`).
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, JointImageIterator, SampleIterator
template< typename T >
class DIP_NO_EXPORT LineIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;               ///< The data type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a pixel
      using pointer = T*;                 ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      LineIterator() {}
      /// To construct a useful iterator, provide an image, the coordinate of the start pixel, and the processing dimension
      LineIterator( Image const& image, UnsignedArray const& coords, dip::uint procDim ) {
         DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( image.DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
         DIP_THROW_IF( procDim >= image.Dimensionality(), E::ILLEGAL_DIMENSION );
         ptr_ = static_cast< pointer >( image.Pointer( coords )); // throws if coords are outside of image
         coord_ = coords[ procDim ];
         size_ = image.Size( procDim );
         stride_ = image.Stride( procDim );
         nTensorElements_ = image.TensorElements();
         tensorStride_ = image.TensorStride();
      }
      /// \brief To construct a useful iterator, provide a pointer, the offset within the line, the length of the line,
      /// the stride, the number of tensor elements, and the tensor stride.
      LineIterator(
            pointer ptr,
            dip::uint coord,
            dip::uint size,
            dip::sint stride,
            dip::uint nTensorElements,
            dip::sint tensorStride
      ) :
            ptr_( ptr ),
            coord_( coord ),
            size_( size ),
            stride_( stride ),
            nTensorElements_( nTensorElements ),
            tensorStride_( tensorStride ) {}
      /// \brief To construct a useful iterator, provide a pointer, the length of the line, the stride, the number
      /// of tensor elements, and the tensor stride. The iterator starts at the beginning of the line.
      LineIterator(
            pointer ptr,
            dip::uint size,
            dip::sint stride,
            dip::uint nTensorElements,
            dip::sint tensorStride
      ) :
            ptr_( ptr ),
            coord_( 0 ),
            size_( size ),
            stride_( stride ),
            nTensorElements_( nTensorElements ),
            tensorStride_( tensorStride ) {}
      /// \brief To construct a useful iterator, provide a pointer, the offset within the line, the length of the line,
      /// and the stride. A single tensor element is assumed.
      LineIterator(
            pointer ptr,
            dip::uint coord,
            dip::uint size,
            dip::sint stride
      ) :
            ptr_( ptr ),
            coord_( coord ),
            size_( size ),
            stride_( stride ),
            nTensorElements_( 1 ),
            tensorStride_( 0 ) {}
      /// \brief To construct a useful iterator, provide a pointer, the length of the line, and the stride.
      /// A single tensor element is assumed. The iterator starts at the beginning of the line.
      LineIterator(
            pointer ptr,
            dip::uint size,
            dip::sint stride
      ) :
            ptr_( ptr ),
            coord_( 0 ),
            size_( size ),
            stride_( stride ),
            nTensorElements_( 1 ),
            tensorStride_( 0 ) {}

      /// Swap
      void swap( LineIterator& other ) {
         using std::swap;
         swap( ptr_, other.ptr_ );
         swap( coord_, other.coord_ );
         swap( size_, other.size_ );
         swap( stride_, other.stride_ );
         swap( nTensorElements_, other.nTensorElements_ );
         swap( tensorStride_, other.tensorStride_ );
      }
      /// Convert from non-const iterator to const iterator
      operator LineIterator< value_type const >() const {
         LineIterator< value_type const > out;
         out.ptr_ = ptr_ ;
         out.coord_ = coord_ ;
         out.size_ = size_ ;
         out.stride_ = stride_ ;
         out.nTensorElements_ = nTensorElements_ ;
         out.tensorStride_ = tensorStride_ ;
         return out;
      }

      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( dip::uint index ) const { return *( ptr_ + static_cast< dip::sint >( index ) * tensorStride_ ); }

      /// Increment
      LineIterator& operator++() {
         if( ptr_ ) {
            ++coord_;
            if( coord_ >= size_ ) {
               ptr_ = nullptr;
            } else {
               ptr_ += stride_;
            }
         }
         return *this;
      }
      /// Increment
      LineIterator operator++( int ) {
         LineIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel
      SampleIterator< value_type > begin() const { return SampleIterator< value_type >( ptr_, tensorStride_ ); } // will yield a const iterator if value_type is const!
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< value_type > end() const { return begin() + nTensorElements_; }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cbegin() const { return ConstSampleIterator< value_type >( ptr_, tensorStride_ ); }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cend() const { return cbegin() + nTensorElements_; }

      /// Equality comparison
      bool operator==( LineIterator const& other ) const {
         return ptr_ == other.ptr_;
      }
      /// Inequality comparison
      bool operator!=( LineIterator const& other ) const {
         return ptr_ != other.ptr_;
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return ptr_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return ptr_ != nullptr; }

      /// Return the current coordinate along the line
      dip::uint const& Coordinate() const { return coord_; }
      /// Return the number of pixels along the line
      dip::uint const& Length() const { return size_; }
      /// Return the current pointer
      pointer Pointer() const { return ptr_; }

   private:
      pointer ptr_ = nullptr;
      dip::uint coord_ = 0;
      dip::uint size_ = 0;
      dip::sint stride_;
      dip::uint nTensorElements_ = 0;
      dip::sint tensorStride_;
};

template< typename T >
inline void swap( LineIterator< T >& v1, LineIterator< T >& v2 ) {
   v1.swap( v2 );
}

/// \brief A const iterator to iterate over all pixels of an image line.
///
/// This iterator is identical to `dip::LineIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstLineIterator = LineIterator< T const >;


//
// Image iterator, does nD loops over all pixels in an image
//


/// \brief An iterator to iterate over all pixels of an image, or all lines of an image.
///
/// If it is created with the processing dimension set, then the processing dimension indicates
/// a dimension over which this iterator does not iterate. That is, it will iterate over all pixels
/// that have a coordinate equal to 0 along this dimension. In this case, a `dip::LineIterator`
/// can be obtained to iterate over the line that starts at the current pixel (using the
/// `GetLineIterator` method).
///
/// If the processing dimension is not set, then the iterator visits all pixels in the image.
///
/// The iterator can be incremented until it reaches the end of the image. At this point, the
/// iterator will become invalid. An invalid iterator will test false. The `IsAtEnd` method
/// can be used instead to test for this condition. It is also possible to compare two iterators
/// for equality (i.e. to compare against an end iterator).
///
/// Dereferencing the iterator yields the first sample of the current pixel (`*it` == `it[ 0 ]`).
/// One can index using the `[]` operator to obtain each of the samples of the tensor
/// (`it[ 0 ]` .. `it[ image.TensorElements() - 1 ]`).
///
/// Alternatively, a `dip::SampleIterator` can be obtained to iterate over the samples of the tensor
/// (`it.begin()` .. `it.end()`).
///
/// It is possible to obtain neighboring pixel values using one of two methods. The simpler method is
/// also the most dangerous:
///
/// ```cpp
///     *( it.Pointer() + offset )
/// ```
///
/// accesses the neighbor at a pre-computed offset. Note that this offset can cause a read-out-of-bounds
/// or simply access the wrong pixel if the neighbor is not within the image domain. One would have to test
/// for `it.Coordinates()` to be far enough away from the edge of the image. This method for accessing a
/// neighbor is best used when iterating over a window within a larger image, where one can be sure that
/// neighbors always exist.
///
/// The more complex method is always safe but always slower:
///
/// ```cpp
///     std::array< dip::uint8, image.TensorElements() > pixel;
///     it.PixelAt( coords, pixel.begin() );
/// ```
///
/// copies the pixel values at the current position + `coords` over to a temporary buffer, using the
/// iterator's boundary condition if that location falls outside the image domain. This method is not
/// the most efficient way of accessing neighbor pixels, but can be convenient at times.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see JointImageIterator, LineIterator, SampleIterator, GenericImageIterator
template< typename T >
class DIP_NO_EXPORT ImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;               ///< The data type of the sample, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a sample
      using pointer = T*;                 ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      ImageIterator() {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      ImageIterator( Image const& image, dip::uint procDim = std::numeric_limits< dip::uint >::max() ) :
            image_( &image ),
            ptr_( static_cast< pointer >( image.Origin() )),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( image.Dimensionality(), BoundaryCondition::DEFAULT ) {
         DIP_THROW_IF( !image_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( image_->DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
      }
      /// To construct a useful iterator, provide an image, a boundary condition array, and optionally a processing dimension
      ImageIterator( Image const& image, BoundaryConditionArray const& bc, dip::uint procDim = std::numeric_limits< dip::uint >::max() ) :
            image_( &image ),
            ptr_( static_cast< pointer >( image.Origin() )),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( bc ) {
         DIP_THROW_IF( !image_->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( image_->DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
         BoundaryArrayUseParameter( boundaryCondition_, image_->Dimensionality() );
      }

      /// Swap
      void swap( ImageIterator& other ) {
         using std::swap;
         swap( image_, other.image_ );
         swap( ptr_, other.ptr_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( boundaryCondition_, other.boundaryCondition_ );
      }
      /// Convert from non-const iterator to const iterator
      operator ImageIterator< value_type const >() const {
         DIP_ASSERT( image_ );
         ImageIterator< value_type const > out( *image_, boundaryCondition_, procDim_ );
         // static_cast above: the constructor will cast back to `sint`, yielding the same original value on
         // two's complement machines. On other types of machines, this will presumably also be OK, as it is
         // unlikely to turn into a value that is in the range [0,nDims).
         out.SetCoordinates( coords_ );
         return out;
      }

      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( dip::uint index ) const {
         DIP_ASSERT( image_ );
         return *( ptr_ + static_cast< dip::sint >( index ) * image_->TensorStride() );
      }

      /// \brief Copy the samples of a neighbor with relative coordinates of `coords`, using the
      /// boundary condition if that neighbor is outside of the image domain.
      ///
      /// It is relatively expensive to test for a pixel to be outside the image domain,
      /// if you can be sure that the neighbor exists, use `*( dip::ImageIterator::Pointer() + offset )`
      /// instead.
      ///
      /// \see dip::ReadPixelWithBoundaryCondition
      template< typename OutputIterator >
      void PixelAt( IntegerArray coords, OutputIterator it ) {
         DIP_THROW_IF( coords.size() != coords_.size(), E::ARRAY_ILLEGAL_SIZE );
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            coords[ ii ] += static_cast< dip::sint >( coords_[ ii ] );
         }
         ReadPixelWithBoundaryCondition< value_type >( *image_, it, coords, boundaryCondition_ );
      }

      /// Increment
      ImageIterator& operator++() {
         if( ptr_ ) {
            dip::uint dd;
            for( dd = 0; dd < coords_.size(); ++dd ) {
               if( dd != procDim_ ) {
                  // Increment coordinate and adjust pointer
                  ++coords_[ dd ];
                  ptr_ += image_->Stride( dd );
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < image_->Size( dd ) ) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  ptr_ -= static_cast< dip::sint >( coords_[ dd ] ) * image_->Stride( dd );
                  coords_[ dd ] = 0;
               }
            }
            if( dd == coords_.size() ) {
               ptr_ = nullptr;
            }
         }
         return *this;
      }
      /// Increment
      ImageIterator operator++( int ) {
         ImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel
      SampleIterator< value_type > begin() const {
         DIP_ASSERT( image_ );
         return SampleIterator< value_type >( ptr_, image_->TensorStride() ); // will yield a const iterator if value_type is const!
      }
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< value_type > end() const { return begin() + image_->TensorElements(); }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cbegin() const {
         DIP_ASSERT( image_ );
         return ConstSampleIterator< value_type >( ptr_, image_->TensorStride() );
      }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cend() const { return cbegin() + image_->TensorElements(); }
      /// Get an iterator over the current line
      LineIterator< value_type > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< value_type >( *image_, coords_, procDim_ );
      }
      /// Get a const iterator over the current line
      ConstLineIterator< value_type > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< value_type >( *image_, coords_, procDim_ );
      }

      /// Equality comparison
      bool operator==( ImageIterator const& other ) const {
         return ptr_ == other.ptr_;
      }
      /// Inequality comparison
      bool operator!=( ImageIterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return ptr_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return ptr_ != nullptr; }

      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( image_ );
         if( HasProcessingDimension() && ( coords.size() > procDim_ )) {
            coords[ procDim_ ] = 0;
         }
         ptr_ = image_->Pointer( coords ); // tests for coords to be correct
         coords_ = coords;
      }

      /// Return the current pointer
      pointer Pointer() const { return ptr_; }
      /// Return the current offset
      dip::sint Offset() const {
         DIP_ASSERT( image_ );
         return ptr_ - static_cast< pointer >( image_->Origin() );
      }
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
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? static_cast< dip::sint >( procDim_ ) : -1; }

      /// \brief Set the boundary condition for accessing pixels outside the image boundary. An empty array sets
      /// all dimensions to the default value, and an array with a single element sets all dimensions to
      /// that value.
      void SetBoundaryCondition( BoundaryConditionArray const& bc ) {
         boundaryCondition_ = bc;
         BoundaryArrayUseParameter( boundaryCondition_, image_->Dimensionality() );
      }
      /// Set the boundary condition for accessing pixels outside the image boundary, for the given dimension
      void SetBoundaryCondition( dip::uint d, BoundaryCondition bc ) {
         if( d < boundaryCondition_.size() ) {
            boundaryCondition_[ d ] = bc;
         }
      }

   private:
      Image const* image_ = nullptr;
      pointer ptr_ = nullptr;
      UnsignedArray coords_;
      dip::uint procDim_;
      BoundaryConditionArray boundaryCondition_ = {};
};

template< typename T >
inline void swap( ImageIterator< T >& v1, ImageIterator< T >& v2 ) {
   v1.swap( v2 );
}

/// \brief A const iterator to iterate over all pixels of an image, or all lines of an image.
///
/// This iterator is identical to `dip::ImageIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstImageIterator = ImageIterator< T const >;


//
// Image iterator, does nD loops over all pixels in an image
//

namespace detail {

// Used by JointImageIterator<> constructor, tests to see if all input images have the right data type
template< typename T = void, typename... OtherTs >
void TestDataType( Image const* images[] ) {
   if(( *images )->IsForged() ) {
      DIP_THROW_IF(( *images )->DataType() != DataType( T( 0 )), E::WRONG_DATA_TYPE );
   }
   TestDataType< OtherTs... >( images + 1 );
}
template<>
inline void TestDataType<>( Image const* /*images*/[] ) {} // End of iteration

} // namespace detail

/// \brief An iterator to iterate over all pixels of multiple images.
///
/// The images must have the same sizes except along the processing dimension. It behaves similarly to
/// `dip::ImageIterator` with the following differences:
///
/// - This iterator is not dereferenceable and has no `[]` operator. The reason is that it points at multiple
/// pixels at the same time (that is, one pixel in each image). Instead, use the `Sample<N>` method to obtain
/// a reference to a sample of the pixel in image `N`.
///
/// - The `GetLineIterator`, `Pointer` and `Offset` methods are templated also, requiring a template parameter
/// `N` as in `Offset<N>`.
///
/// - There is no `PixelAt` method, and no boundary condition parameters.
///
/// The first image in the set must be forged. Other images can be empty, but dereferencing them will lead to
/// undefined behavior.
///
/// Note that, within templated code, calling the `Sample<N>` and similar templated methods requires specifying
/// that the method is a template:
/// ```cpp
///     template< typename TPI >
///     void Function( ... ) {
///        JointImageIterator< TPI, bin > it( { in, mask } );
///        do {
///           if( it.template Sample< 1 >() ) {  // Note `template` on this line
///              it.template Sample< 0 >() = 0;  // Note `template` on this line
///           }
///        } while( ++it );
///     }
/// ```
///
/// There exist aliases `InXxx` for `Xxx<0>`, and `OutXxx` for `Xxx<1>`, where `Xxx` is `Sample`, `Pointer` or `Offset`.
/// `In()` is an alias for `Sample<0>()` and `Out()` is an alias for `Sample<1>()`.
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, LineIterator, SampleIterator, GenericJointImageIterator
template< typename... Types >
class DIP_NO_EXPORT JointImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      template< dip::uint I > using value_type = typename std::tuple_element< I, std::tuple< Types ... >>::type;
      ///< The data type of the sample, obtained when dereferencing the iterator
      using difference_type = dip::sint;                ///< The type of distances between iterators
      template< dip::uint I > using reference = value_type< I >&; ///< The type of a reference to a sample
      template< dip::uint I > using pointer = value_type< I >*;   ///< The type of a pointer to a sample

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an
      /// end iterator.
      JointImageIterator() : atEnd_( true ) {
         images_.fill( nullptr );
         offsets_.fill( 0 );
      }
      /// To construct a useful iterator, provide `N` images (`N` equal to the number of template parameters),
      /// and optionally a processing dimension.
      JointImageIterator( ImageConstRefArray const& images, dip::uint procDim = std::numeric_limits< dip::uint >::max() ):
            procDim_( procDim ), atEnd_( false ) {
         DIP_THROW_IF( images.size() != N, E::ARRAY_ILLEGAL_SIZE );
         images_[ 0 ] = &( images[ 0 ].get() );
         DIP_THROW_IF( !images_[ 0 ]->IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( images_[ 0 ]->DataType() != DataType( value_type< 0 >( 0 )), E::WRONG_DATA_TYPE );
         coords_.resize( images_[ 0 ]->Dimensionality(), 0 );
         dummy_.SetStrides( IntegerArray( coords_.size(), 0 ));
         offsets_.fill( 0 );
         for( dip::uint ii = 1; ii < N; ++ii ) {
            images_[ ii ] = &( images[ ii ].get() );
            if( !images_[ ii ]->IsForged() ) {
               images_[ ii ] = &dummy_;
            }
         }
         detail::TestDataType< Types... >( images_.data() );
      }

      /// Swap
      void swap( JointImageIterator& other ) {
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
      reference< I > Sample( dip::uint index ) const {
         DIP_ASSERT( images_[ I ] );
         return *( static_cast< pointer< I >>( images_[ I ]->Origin()) + offsets_[ I ] +
                   static_cast< dip::sint >( index ) * images_[ I ]->TensorStride());
      }
      /// Index into image tensor for image 0.
      reference< 0 > InSample( dip::uint index ) const { return Sample< 0 >( index ); }
      /// Index into image tensor for image 1.
      reference< 1 > OutSample( dip::uint index ) const { return Sample< 1 >( index ); }
      /// Get first tensor element for image `I`.
      template< dip::uint I >
      reference< I > Sample() const {
         DIP_ASSERT( images_[ I ] );
         return *( static_cast< pointer< I >>( images_[ I ]->Origin() ) + offsets_[ I ] );
      }
      /// Get first tensor element for image 0.
      reference< 0 > In() const { return Sample< 0 >(); }
      /// Get first tensor element for image 1.
      reference< 1 > Out() const { return Sample< 1 >(); }

      /// Increment
      JointImageIterator& operator++() {
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
      JointImageIterator operator++( int ) {
         JointImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      SampleIterator <value_type< I >> begin() const {
         DIP_ASSERT( images_[ I ] );
         return SampleIterator< value_type< I >>( Pointer< I >(), images_[ I ]->TensorStride() ); // will yield a const iterator if value_type is const!
      }
      /// Get an end iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      SampleIterator <value_type< I >> end() const { return begin() + images_[ I ]->TensorElements(); }
      /// Get a const iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      ConstSampleIterator <value_type< I >> cbegin() const {
         DIP_ASSERT( images_[ I ] );
         return ConstSampleIterator< value_type< I >>( Pointer< I >(), images_[ I ]->TensorStride() );
      }
      /// Get an end const iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      ConstSampleIterator <value_type< I >> cend() const { return cbegin() + images_[ I ]->TensorElements(); }
      /// Get an iterator over the current line of image `I`
      template< dip::uint I >
      LineIterator< value_type< I >> GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(),
                       "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< value_type< I >>( *images_[ I ], coords_, procDim_ );
      }
      /// Get a const iterator over the current line of image `I`
      template< dip::uint I >
      ConstLineIterator< value_type< I >> GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(),
                       "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< value_type< I >>( *images_[ I ], coords_, procDim_ );
      }

      /// Equality comparison, is equal if the two iterators have the same coordinates. It is possible to compare
      /// JointImageIterators with different images and different types.
      template< typename... OtherTypes >
      bool operator==( JointImageIterator< OtherTypes... > const& other ) const {
         return ( atEnd_ == other.atEnd_ ) && ( coords_ == other.coords_ );
      }
      /// Inequality comparison
      template< typename... OtherTypes >
      bool operator!=( JointImageIterator< OtherTypes... > const& other ) const {
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
         if( HasProcessingDimension() && ( coords.size() > procDim_ )) {
            coords[ procDim_ ] = 0;
         }
         for( dip::uint ii = 0; ii < N; ++ii ) {
            offsets_[ ii ] = images_[ ii ]->Offset( coords );
         }
         coords_ = coords;
      }

      /// Return the current pointer for image `I`
      template< dip::uint I >
      pointer< I > Pointer() const {
         DIP_ASSERT( images_[ I ] );
         return static_cast< pointer< I >>( images_[ I ]->Origin() ) + offsets_[ I ];
      }
      /// Return the current pointer for image 0.
      pointer< 0 > InPointer() const { return Pointer< 0 >(); }
      /// Return the current pointer for image 1.
      pointer< 1 > OutPointer() const { return Pointer< 1 >(); }
      /// Return the current offset for image `I`
      template< dip::uint I >
      dip::sint Offset() const { return offsets_[ I ]; }
      /// Index into image tensor for image 0.
      dip::sint InOffset() const { return offsets_[ 0]; }
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
      dip::sint ProcessingDimension() const {
         return HasProcessingDimension() ? static_cast< dip::sint >( procDim_ ) : -1;
      }

   private:
      constexpr static dip::uint N = sizeof...( Types );
      static_assert( N > 1, "JointImageIterator needs at least one type template argument" );
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

template< typename inT, typename outT >
inline void swap( JointImageIterator< inT, outT >& v1, JointImageIterator< inT, outT >& v2 ) {
   v1.swap( v2 );
}


/// \}

} // namespace dip

#endif // DIP_ITERATORS_H
