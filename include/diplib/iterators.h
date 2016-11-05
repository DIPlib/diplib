/*
 * DIPlib 3.0
 * This file contains support for 1D and nD iterators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_ITERATORS_H
#define DIP_ITERATORS_H

#include <utility>

#include "diplib.h"
#include "boundary.h"


/// \file
/// Defines image iterators and pixel iterators


namespace dip {


//
// Sample iterator, does 1D loop over samples with a single stride
//


/// An iterator to iterate over samples in a tensor, or pixels on an image line. This
/// is the simplest iterator available in this library, and is most like working with
/// a pointer to a data segment. The only difference with a pointer is that the data
/// stride is taken into account.
///
/// Satisfies all the requirements for a mutable [RandomAccessIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// This means that you can increment and decrement the iterator, add or subtract an
/// integer from it, dereference it, index it using the `[]` operator, as well as compare
/// two iterators or take the difference between them (as long as they reference samples
/// within the same data segment). It is default constructible and swappable, but the
/// default constructed iterator is invalid and should not be dereferenced.
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, JointImageIterator, LineIterator
template< typename T >
class SampleIterator {
   public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = T;               ///< The data type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a pixel
      using pointer = T*;                 ///< The type of a pointer to a pixel
      /// Default constructor yields an invalid iterator that cannot be dereferenced
      SampleIterator() : stride_( 1 ), ptr_( nullptr ) {}
      /// To construct a useful iterator, provide a pointer and a stride
      SampleIterator( pointer ptr, dip::sint stride ) : stride_( stride ), ptr_( ptr ) {}
      /// Swap
      void swap( SampleIterator& other ) {
         using std::swap;
         swap( stride_, other.stride_ );
         swap( ptr_, other.ptr_ );
      }
      /// Convert from non-const iterator to const iterator
      operator SampleIterator< value_type const >() const {
         return SampleIterator< value_type const >( ptr_, stride_ );
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index
      reference operator[]( difference_type index ) const { return *( ptr_ + index * stride_ ); }
      /// Increment
      SampleIterator& operator++() {
         ptr_ += stride_;
         return *this;
      }
      /// Decrement
      SampleIterator& operator--() {
         ptr_ -= stride_;
         return *this;
      }
      /// Increment
      SampleIterator operator++( int ) {
         SampleIterator tmp( *this );
         ptr_ += stride_;
         return tmp;
      }
      /// Decrement
      SampleIterator operator--( int ) {
         SampleIterator tmp( *this );
         ptr_ -= stride_;
         return tmp;
      }
      /// Add integer
      SampleIterator& operator+=( difference_type index ) {
         ptr_ += index * stride_;
         return *this;
      }
      /// Subtract integer
      SampleIterator& operator-=( difference_type index ) {
         ptr_ -= index * stride_;
         return *this;
      }
      /// Add integer
      friend SampleIterator operator+( SampleIterator it, difference_type n ) {
         it += n;
         return it;
      }
      /// Add integer
      friend SampleIterator operator+( difference_type n, SampleIterator it ) {
         it += n;
         return it;
      }
      /// Subtract integer
      friend SampleIterator operator-( SampleIterator it, difference_type n ) {
         it -= n;
         return it;
      }
      /// Difference between iterators
      friend difference_type operator-( SampleIterator const& it1, SampleIterator const& it2 ) {
         return it1.ptr_ - it2.ptr_;
      }
      /// Equality comparison
      bool operator==( SampleIterator const& other ) const { return ptr_ == other.ptr_; }
      /// Inequality comparison
      bool operator!=( SampleIterator const& other ) const { return ptr_ != other.ptr_; }
      /// Larger than comparison
      bool operator>( SampleIterator const& other ) const { return ptr_ > other.ptr_; }
      /// Smaller than comparison
      bool operator<( SampleIterator const& other ) const { return ptr_ < other.ptr_; }
      /// Not smaller than comparison
      bool operator>=( SampleIterator const& other ) const { return ptr_ >= other.ptr_; }
      /// Not larger than comparison
      bool operator<=( SampleIterator const& other ) const { return ptr_ <= other.ptr_; }
   private:
      dip::sint stride_;
      pointer ptr_;
};

template< typename T >
inline void swap( SampleIterator< T >& v1, SampleIterator< T >& v2 ) {
   v1.swap( v2 );
}

/// A const iterator to iterate over samples in a tensor, or pixels on an image line.
/// This iterator is identical to `dip::SampleIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [RandomAccessIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstSampleIterator = SampleIterator< T const >;


//
// Line iterator, does 1D loop over the pixels in an image line
//


/// An iterator to iterate over all pixels of an image line. This iterator is constructed
/// from a `dip::ImageIterator`, but can also be constructed manually if necessary.
///
/// The iterator can be incremented until it reaches the end of the line. At this point, the
/// iterator will become invalid. An invalid iterator will test false. The `IsAtEnd` method
/// can be used instead to test for this condion. It is also possible to compare two iterators
/// for equality (i.e. to compare against an end iterator).
///
/// Dereferencing the iterator yields the first sample of the current pixel. One can index using the
/// `[]` operator to obtain each of the samples of the tensor:
///
///     *it == it[ 0 ]
///     it[ 0 ] .. it[ image.TensorElements() - 1 ]
///
/// Alternatively, a `dip::SampleIterator` can be obtained to iterate over the samples of the tensor:
///
///     it.begin() .. it.end()
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, JointImageIterator, SampleIterator
template< typename T >
class LineIterator {
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
         dip_ThrowIf( !image.IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( image.DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
         dip_ThrowIf( procDim >= image.Dimensionality(), E::ILLEGAL_DIMENSION );
         ptr_ = image.Pointer( coords ); // throws if coords are outside of image
         coord_ = coords[ procDim ];
         size_ = image.Size( procDim );
         stride_ = image.Stride( procDim );
         nTensorElements_ = image.TensorElements();
         tensorStride_ = image.TensorStride();
      }
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
      reference operator[]( difference_type index ) const { return *( ptr_ + index * tensorStride_ ); }
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
      operator bool() const { return ptr_ != nullptr; }
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

/// A const iterator to iterate over all pixels of an image line.
/// This iterator is identical to `dip::LineIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstLineIterator = LineIterator< T const >;


//
// Image iterator, does nD loops over all pixels in an image
//


/// An iterator to iterate over all pixels of an image, or all lines of an image.
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
/// can be used instead to test for this condion. It is also possible to compare two iterators
/// for equality (i.e. to compare against an end iterator).
///
/// Dereferencing the iterator yields the first sample of the current pixel. One can index using the
/// `[]` operator to obtain each of the samples of the tensor:
///
///     *it == it[ 0 ]
///     it[ 0 ] .. it[ image.TensorElements() - 1 ]
///
/// Alternatively, a `dip::SampleIterator` can be obtained to iterate over the samples of the tensor:
///
///     it.begin() .. it.end()
///
/// It is possible to obtain neighboring pixel values using one of two methods. The simpler method is
/// also the most dangerous:
///
///     *( it.Pointer() + offset )
///
/// accesses the neighbor at a pre-computed offset. Note that this offset can cause a read-out-of-bounds
/// or simply access the wrong pixel if the neighbor is not within the image domain. One would have to test
/// for `it.Coordinates()` to be far enough away from the edge of the image. This method for accessing a
/// neighbor is best used when iterating over a window within a larger image, where one can be sure that
/// neighbors always exist.
///
/// The more complex method is always safe but always slower:
///
///     std::array< dip::uint8, image.TensorElements() > pixel;
///     it.PixelAt( coords, pixel.begin() );
///
/// copies the pixel values at the current position + `coords` over to a temporary buffer, using the
/// iterator's boundary condition if that location falls outside the image domain. This method is not
/// the most efficient way of accessing neighbor pixels, but can be convienient at times.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see JointImageIterator, LineIterator, SampleIterator, GenericImageIterator
template< typename T >
class ImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;               ///< The data type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a pixel
      using pointer = T*;                 ///< The type of a pointer to a pixel
      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      ImageIterator() {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      ImageIterator( Image const& image, dip::sint procDim = -1 ) :
            image_( &image ),
            ptr_( static_cast< pointer >( image.Origin() )),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( image.Dimensionality(), BoundaryCondition::DEFAULT ) {
         dip_ThrowIf( !image_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( image_->DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
      }
      /// To construct a useful iterator, provide an image, a boundary condition array, and optionally a processing dimension
      ImageIterator( Image const& image, BoundaryConditionArray const& bc, dip::sint procDim = -1 ) :
            image_( &image ),
            ptr_( static_cast< pointer >( image.Origin() )),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( image.Dimensionality(), BoundaryCondition::DEFAULT ) {
         dip_ThrowIf( !image_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( image_->DataType() != DataType( value_type(0) ), E::WRONG_DATA_TYPE );
         SetBoundaryCondition( bc );
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
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         ImageIterator< value_type const > out( *image_, boundaryCondition_, procDim_ );
         out.SetCoordinates( coords_ );
         return out;
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( difference_type index ) const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return *( ptr_ + index * image_->TensorStride() );
      }
      /// Copy the samples of a neighbor with relative coordinates of `coords`, using the
      /// boundary condition if that neighbor is outside of the iamge domain.
      ///
      /// It is relatively expensive to test for a pixel to be outside the image domain,
      /// if you can be sure that the neighbor exists, use `*( dip::ImageIterator::Pointer() + offset )`
      /// instead.
      ///
      /// \see dip::ReadPixelWithBoundaryCondition
      template< typename OutputIterator >
      void PixelAt( IntegerArray coords, OutputIterator it ) {
         dip_ThrowIf( coords.size() != coords_.size(), E::ARRAY_ILLEGAL_SIZE );
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            coords[ ii ] += coords_[ ii ];
         }
         ReadPixelWithBoundaryCondition< value_type >( *image_, it, coords, boundaryCondition_ );
      }
      /// Increment
      ImageIterator& operator++() {
         if( ptr_ ) {
            dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
            dip::uint dd;
            for( dd = 0; dd < coords_.size(); ++dd ) {
               if( static_cast< dip::sint >( dd ) != procDim_ ) {
                  // Increment coordinate and adjust pointer
                  ++coords_[ dd ];
                  ptr_ += image_->Stride( dd );
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < image_->Size( dd ) ) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  ptr_ -= coords_[ dd ] * image_->Stride( dd );
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
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return SampleIterator< value_type >( ptr_, image_->TensorStride() ); // will yield a const iterator if value_type is const!
      }
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< value_type > end() const { return begin() + image_->TensorElements(); }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cbegin() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return ConstSampleIterator< value_type >( ptr_, image_->TensorStride() );
      }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cend() const { return cbegin() + image_->TensorElements(); }
      /// Get an iterator over the current line
      LineIterator< value_type > GetLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< value_type >( *image_, coords_, static_cast< dip::uint >( procDim_ ) );
      }
      /// Get a const iterator over the current line
      ConstLineIterator< value_type > GetConstLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< value_type >( *image_, coords_, static_cast< dip::uint >( procDim_ ) );
      }
      /// Equality comparison
      bool operator==( ImageIterator const& other ) const {
         return ptr_ == other.ptr_;
      }
      /// Inequality comparison
      bool operator!=( ImageIterator const& other ) const {
         return ptr_ != other.ptr_;
      }
      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return ptr_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      operator bool() const { return ptr_ != nullptr; }
      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray coords ) {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
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
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return ptr_ - static_cast< pointer >( image_->Origin() );
      }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return image_->Index( coords_ );
      }
      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( image_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < image_->Dimensionality() );
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? procDim_ : -1; }
      /// Set the boundary condition for accessing pixels outside the image boundary; dimensions not specified will
      /// use the default boundary condition.
      void SetBoundaryCondition( BoundaryConditionArray const& bc ) {
         for( dip::uint ii = 0; ii < std::min( bc.size(), boundaryCondition_.size() ); ++ii ) {
            boundaryCondition_[ ii ] = bc[ ii ];
         }
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
      UnsignedArray coords_ = {};
      dip::sint procDim_ = -1;
      BoundaryConditionArray boundaryCondition_ = {};
};

template< typename T >
inline void swap( ImageIterator< T >& v1, ImageIterator< T >& v2 ) {
   v1.swap( v2 );
}

/// A const iterator to iterate over all pixels of an image, or all lines of an image.
/// This iterator is identical to `dip::ImageIterator`, but with a const value type.
///
/// Satisfies all the requirements for a non-mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
template< typename T >
using ConstImageIterator = ImageIterator< T const >;


//
// Image iterator, does nD loops over all pixels in an image
//


/// An iterator to iterate over all pixels of two images, with read-only access of the first image (input)
/// and write access of the second (output). The two images must have the same sizes except along the
/// processing dimension. It behaves similarly to `dip::ImageIterator` with the following differences:
///
/// This iterator is not dereferenceable. The reason is that it points at two pixels at the same time
/// (that is, one pixel in each image). Instead, use the `In` and `Out` methods to obtain references to
/// to the first sample of each pixel. Use the `InElement` and `OutElement` methods instead of the `[]`
/// operator to access the other samples.
///
/// Instead of `GetLineIterator`, use `GetInLineIterator` and `GetOutLineIterator`. Likewise, instead of
/// `Pointer` and `Offset` methods, use `InPointer`, `OutPointer`, `InOffset` and `OutOffset`.
///
/// The `PixelAt` method reads pixels from the input image.
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, LineIterator, SampleIterator, GenericJointImageIterator
template< typename inT, typename outT >
class JointImageIterator {
   public:
      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      JointImageIterator() {}
      /// To construct a useful iterator, provide two images, and optionally a processing dimension
      JointImageIterator( Image const& input, Image const& output, dip::sint procDim = -1 ) :
            inImage_( &input ),
            outImage_( &output ),
            inPtr_( static_cast< inT* >( input.Origin() )),
            outPtr_( static_cast< outT* >( output.Origin() )),
            coords_( input.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( input.Dimensionality(), BoundaryCondition::DEFAULT ) {
         dip_ThrowIf( !inImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( !outImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( inImage_->DataType() != DataType( inT(0) ), E::WRONG_DATA_TYPE );
         dip_ThrowIf( outImage_->DataType() != DataType( outT(0) ), E::WRONG_DATA_TYPE );
         dip_ThrowIf( !CompareSizes(), E::SIZES_DONT_MATCH );
      }
      /// To construct a useful iterator, provide two images, a boundary condition array, and optionally a processing dimension
      JointImageIterator( Image const& input, Image const& output, BoundaryConditionArray const& bc, dip::sint procDim = -1 ) :
            inImage_( &input ),
            outImage_( &output ),
            inPtr_( static_cast< inT* >( input.Origin() )),
            outPtr_( static_cast< outT* >( output.Origin() )),
            coords_( input.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( input.Dimensionality(), BoundaryCondition::DEFAULT ) {
         dip_ThrowIf( !inImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( !outImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( inImage_->DataType() != DataType( inT(0) ), E::WRONG_DATA_TYPE );
         dip_ThrowIf( outImage_->DataType() != DataType( outT(0) ), E::WRONG_DATA_TYPE );
         dip_ThrowIf( !CompareSizes(), E::SIZES_DONT_MATCH );
         SetBoundaryCondition( bc );
      }
      /// Swap
      void swap( JointImageIterator& other ) {
         using std::swap;
         swap( inImage_, other.inImage_ );
         swap( outImage_, other.outImage_ );
         swap( inPtr_, other.inPtr_ );
         swap( outPtr_, other.outPtr_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( boundaryCondition_, other.boundaryCondition_ );
      }
      /// Dereference input image
      inT const& In() const { return *inPtr_; }
      /// Dereference output image
      outT& Out() const { return *outPtr_; }
      /// Index into input tensor.
      inT const& InElement( dip::sint index ) const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return *( inPtr_ + index * inImage_->TensorStride() );
      }
      /// Index into output tensor.
      outT& OutElement( dip::sint index ) const {
         dip_ThrowIf( !outImage_, E::ITERATOR_NOT_VALID );
         return *( outPtr_ + index * outImage_->TensorStride() );
      }
      /// Copy the input samples of a neighbor pixel with relative coordinates of `coords`, using the
      /// boundary condition if that neighbor is outside of the image domain.
      ///
      /// It is relatively expensive to test for a pixel to be outside the image domain,
      /// if you can be sure that the neighbor exists, use `*( dip::JointImageIterator::InPointer() + offset )`
      /// instead.
      ///
      /// \see dip::ReadPixelWithBoundaryCondition
      template< typename OutputIterator >
      void PixelAt( IntegerArray coords, OutputIterator it ) {
         dip_ThrowIf( coords.size() != coords_.size(), E::ARRAY_ILLEGAL_SIZE );
         for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
            coords[ ii ] += coords_[ ii ];
         }
         ReadPixelWithBoundaryCondition< inT >( *inImage_, it, coords, boundaryCondition_ );
      }
      /// Increment
      JointImageIterator& operator++() {
         if( inPtr_ ) {
            dip_ThrowIf( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
            dip::uint dd;
            for( dd = 0; dd < coords_.size(); ++dd ) {
               if( static_cast< dip::sint >( dd ) != procDim_ ) {
                  // Increment coordinate and adjust pointer
                  ++coords_[ dd ];
                  inPtr_ += inImage_->Stride( dd );
                  outPtr_ += outImage_->Stride( dd );
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < inImage_->Size( dd ) ) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  inPtr_ -= coords_[ dd ] * inImage_->Stride( dd );
                  outPtr_ -= coords_[ dd ] * outImage_->Stride( dd );
                  coords_[ dd ] = 0;
               }
            }
            if( dd == coords_.size() ) {
               inPtr_ = nullptr;
               outPtr_ = nullptr;
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
      /// Get an iterator over the current line of the input image
      ConstLineIterator< inT > GetInLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< inT >( *inImage_, coords_, static_cast< dip::uint >( procDim_ ) );
      }
      /// Get an iterator over the current line of the output image
      LineIterator< inT > GetOutLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< outT >( *outImage_, coords_, static_cast< dip::uint >( procDim_ ) );
      }
      /// Equality comparison
      bool operator==( JointImageIterator const& other ) const {
         return ( inPtr_ == other.inPtr_ ) && ( outPtr_ == other.outPtr_ );
      }
      /// Inequality comparison
      bool operator!=( JointImageIterator const& other ) const {
         return ( inPtr_ != other.inPtr_ ) && ( outPtr_ != other.outPtr_ );
      }
      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return inPtr_ == nullptr; }
      /// Test to see if the iterator is still pointing at a pixel
      operator bool() const { return inPtr_ != nullptr; }
      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray coords ) {
         dip_ThrowIf( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
         if( HasProcessingDimension() && ( coords.size() > procDim_ )) {
            coords[ procDim_ ] = 0;
         }
         inPtr_ = inImage_->Pointer( coords ); // tests for coords to be correct
         outPtr_ = outImage_->Pointer( coords );
         coords_ = coords;
      }
      /// Return the current pointer for the input image
      inT const* InPointer() const { return inPtr_; }
      /// Return the current pointer for the output image
      outT* OutPointer() const { return outPtr_; }
      /// Return the current offset for the input image
      dip::sint InOffset() const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return inPtr_ - static_cast< inT* >( inImage_->Origin() );
      }
      /// Return the current offset for the output image
      dip::sint OutOffset() const {
         dip_ThrowIf( !outImage_, E::ITERATOR_NOT_VALID );
         return outPtr_ - static_cast< outT* >( outImage_->Origin() );
      }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Index( coords_ );
      }
      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( inImage_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < inImage_->Dimensionality() );
         } else {
            return false;
         }
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::sint ProcessingDimension() const { return HasProcessingDimension() ? procDim_ : -1; }
      /// Set the boundary condition for accessing pixels outside the image boundary; dimensions not specified will
      /// use the default boundary condition.
      void SetBoundaryCondition( BoundaryConditionArray const& bc ) {
         for( dip::uint ii = 0; ii < std::min( bc.size(), boundaryCondition_.size() ); ++ii ) {
            boundaryCondition_[ ii ] = bc[ ii ];
         }
      }
      /// Set the boundary condition for accessing pixels outside the image boundary, for the given dimension
      void SetBoundaryCondition( dip::uint d, BoundaryCondition bc ) {
         if( d < boundaryCondition_.size() ) {
            boundaryCondition_[ d ] = bc;
         }
      }
   private:
      Image const* inImage_ = nullptr;
      Image const* outImage_ = nullptr;
      inT* inPtr_ = nullptr;
      outT* outPtr_ = nullptr;
      UnsignedArray coords_ = {};
      dip::sint procDim_ = -1;
      BoundaryConditionArray boundaryCondition_ = {};
      bool CompareSizes() const {
         if( inImage_->Dimensionality() != outImage_->Dimensionality() ) {
            return false;
         }
         for( dip::uint ii = 0; ii < inImage_->Dimensionality(); ++ii ) {
            if(( ii != procDim_ ) && ( inImage_->Size( ii ) != outImage_->Size( ii ) )) {
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


//
// Three generic image iterators, non-templated
//

/// A data-type--agnostic version of `dip::ImageIterator`. Use this iterator only to write code that
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
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see ImageIterator, GenericJointImageIterator
class GenericImageIterator {
   public:
      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericImageIterator() {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      GenericImageIterator( Image const& image, dip::sint procDim = -1 ) :
            image_( &image ),
            offset_( 0 ),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ) {
         dip_ThrowIf( !image_->IsForged(), E::IMAGE_NOT_FORGED );
      }
      /// Swap
      void swap( GenericImageIterator& other ) {
         using std::swap;
         swap( image_, other.image_ );
         swap( offset_, other.offset_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
      }
      /// Tensor indexing returns the offset to the sample
      void* operator[]( dip::sint index ) const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return image_->Pointer( offset_ + index * image_->TensorStride() );
      }
      /// Increment
      GenericImageIterator& operator++() {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
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
               offset_ -= coords_[ dd ] * image_->Stride( dd );
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
      operator bool() const { return image_ != nullptr; }
      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray const& coords ) {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         offset_ = image_->Offset( coords ); // tests for coords to be correct
         coords_ = coords;
      }
      /// Return the current pointer
      void* Pointer() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return image_->Pointer( offset_ );
      }
      /// Return the current offset
      dip::sint Offset() const { return offset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return image_->Index( coords_ );
      }
      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( image_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < image_->Dimensionality() );
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


/// A data-type--agnostic version of `dip::JointImageIterator`. Use this iterator only to write code that
/// does not know at compile-time what the data type of the image is.
///
/// This iterator works similarly to `dip::JointImageIterator` except it does not have the `In` and `Out`
/// methods to obtain references to samples. Instead, use the `InPointer` and `OutPointer` methods to obtain
/// a `void` pointer to the first sample in the pixels. The `InElement` and `OutElement` methods return
/// a `void` pointer to any other sample.
///
/// It is not possible to obtain line or sample iterators from this iterator, and it has no support for
/// accessing pixels in the neighborhood of the referenced pixel.
///
/// Example usage from `dip::Image::Copy`:
///
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
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see JointImageIterator, GenericImageIterator
class GenericJointImageIterator {
   public:
      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericJointImageIterator() {}
      /// To construct a useful iterator, provide two images, and optionally a processing dimension
      GenericJointImageIterator( Image const& input, Image const& output, dip::sint procDim = -1 ) :
            inImage_( &input ),
            outImage_( &output ),
            inOffset_( 0 ),
            outOffset_( 0 ),
            coords_( input.Dimensionality(), 0 ),
            procDim_( procDim ) {
         dip_ThrowIf( !inImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( !outImage_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( !CompareSizes(), E::SIZES_DONT_MATCH );
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
      /// Index into input tensor.
      void* InElement( dip::sint index ) const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Pointer( inOffset_ + index * inImage_->TensorStride() );
      }
      /// Index into output tensor.
      void* OutElement( dip::sint index ) const {
         dip_ThrowIf( !outImage_, E::ITERATOR_NOT_VALID );
         return outImage_->Pointer( outOffset_ + index * outImage_->TensorStride() );
      }
      /// Increment
      GenericJointImageIterator& operator++() {
         dip_ThrowIf( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
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
               inOffset_ -= coords_[ dd ] * inImage_->Stride( dd );
               outOffset_ -= coords_[ dd ] * outImage_->Stride( dd );
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
      operator bool() const { return inImage_ != nullptr; }
      /// Return the current coordinates
      UnsignedArray const& Coordinates() const { return coords_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinates( UnsignedArray const& coords ) {
         dip_ThrowIf( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
         inOffset_ = inImage_->Offset( coords ); // tests for coords to be correct
         outOffset_ = outImage_->Offset( coords );
         coords_ = coords;
      }
      /// Return the current pointer for the input image
      void* InPointer() const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Pointer( inOffset_ );
      }
      /// Return the current pointer for the output image
      void* OutPointer() const {
         dip_ThrowIf( !outImage_, E::ITERATOR_NOT_VALID );
         return outImage_->Pointer( outOffset_ );
      }
      /// Return the current offset for the input image
      dip::sint InOffset() const { return inOffset_; }
      /// Return the current offset for the output image
      dip::sint OutOffset() const { return outOffset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         dip_ThrowIf( !inImage_, E::ITERATOR_NOT_VALID );
         return inImage_->Index( coords_ );
      }
      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         if( inImage_ ) {
            return ( procDim_ >= 0 ) && ( procDim_ < inImage_->Dimensionality() );
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
            if(( ii != procDim_ ) && ( inImage_->Size( ii ) != outImage_->Size( ii ) )) {
               return false;
            }
         }
         return true;
      }
};

inline void swap( GenericJointImageIterator& v1, GenericJointImageIterator& v2 ) {
   v1.swap( v2 );
}


/// An iterator for plane-by-plane processing of an image. Use it to process a multi-dimensional
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
///     dip::ImageSliceIterator it( img, 2 );
///     do {
///        // do something with the image *it here.
///     } while( ++it );
///
/// The function `dip::ImageSliceEndIterator` creates an iterator that points at a slice one past
/// the last, and so is a end iterator. Because it is not possible to decrement below 0, a loop that
/// iterates in reverse order must test the `dip::ImageSliceIterator::Coordinate()` for equality to
/// zero:
///
///     dip::ImageSliceEndIterator it( img, 2 );
///     do {
///        --it;
///        // do something with the image *it here.
///     } while( it.Coordinate() != 0 );
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
/// Additionally, it behaves like a RandomAccessIterator except for the indexing operator `[]`,
/// which would be less efficient in use and therefore it's better to not offer it.
///
/// \see ImageIterator, JointImageIterator, GenericImageIterator, GenericJointImageIterator
class ImageSliceIterator {
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
         dip_ThrowIf( !image.IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( procDim_ >= image.Dimensionality(), E::ILLEGAL_DIMENSION );
         // copy image with shared data
         image_ = image;
         // remove the processing dimension
         size_ = image_.sizes_[ procDim_ ];
         stride_ = image_.strides_[ procDim_ ];
         image_.sizes_.erase( procDim_ );
         image_.strides_.erase( procDim_ );
         image_.pixelSize_.EraseDimension( procDim_ );
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
         dip_ThrowIf( !IsValid(), E::ITERATOR_NOT_VALID );
         ++coord_;
         image_.origin_ = image_.Pointer( stride_ );
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
         dip_ThrowIf( !IsValid(), E::ITERATOR_NOT_VALID );
         if( coord_ != 0 ) {
            --coord_;
            image_.origin_ = image_.Pointer( -stride_ );
         }
         return *this;
      }
      /// Decrement, but never past the first slide
      ImageSliceIterator operator--( int ) {
         ImageSliceIterator tmp( *this );
         operator--();
         return tmp;
      }
      /// Add integer
      ImageSliceIterator& operator+=( difference_type index ) {
         dip_ThrowIf( !IsValid(), E::ITERATOR_NOT_VALID );
         coord_ += index;
         image_.origin_ = image_.Pointer( index * stride_ );
         return *this;
      }
      /// Subtract integer, but never moves the iterator to before the first slide
      ImageSliceIterator& operator-=( difference_type index ) {
         dip_ThrowIf( !IsValid(), E::ITERATOR_NOT_VALID );
         if( index > coord_ ) {
            index = coord_;
         }
         coord_ -= index;
         image_.origin_ = image_.Pointer( -index * stride_ );
         return *this;
      }
      /// Add integer
      friend ImageSliceIterator operator+( ImageSliceIterator it, difference_type n ) {
         it += n;
         return it;
      }
      /// Add integer
      friend ImageSliceIterator operator+( difference_type n, ImageSliceIterator it ) {
         it += n;
         return it;
      }
      /// Subtract integer, but never moves the iterator to before the first slide
      friend ImageSliceIterator operator-( ImageSliceIterator it, difference_type n ) {
         it -= n;
         return it;
      }
      /// Difference between iterators
      friend difference_type operator-( ImageSliceIterator const& it1, ImageSliceIterator const& it2 ) {
         dip_ThrowIf( !it1.IsValid() || !it2.IsValid(), E::ITERATOR_NOT_VALID );
         dip_ThrowIf( ( it1.image_.dataBlock_ != it2.image_.dataBlock_ ) ||
                      ( it1.image_.sizes_ != it2.image_.sizes_ ) ||
                      ( it1.stride_ != it2.stride_ ) ||
                      ( it1.procDim_ != it2.procDim_ ), "Iterators index in different images or along different dimensions" );
         return it1.coord_ - it2.coord_;
      }
      /// Equality comparison
      bool operator==( ImageSliceIterator const& other ) const {
         return image_.origin_ == other.image_.origin_;
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
      /// Test to see if the iterator is valid (i.e. not default-constructed); it can still be at end, and thus not dereferenceble
      bool IsValid() const { return size_ > 0; }
      /// Test to see if the iterator reached past the last plane
      bool IsAtEnd() const { return coord_ >= size_; }
      /// Test to see if the iterator is valid and can be dereferenced
      operator bool() const { return !IsAtEnd(); }
      /// Return the current position
      dip::uint Coordinate() const { return coord_; }
      /// Set the iterator to point at a different location in the image
      void SetCoordinate( dip::uint coord ) {
         dip_ThrowIf( !IsValid(), E::ITERATOR_NOT_VALID );
         dip_ThrowIf( coord >= size_, E::INDEX_OUT_OF_RANGE );
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

inline void swap( ImageSliceIterator& v1, ImageSliceIterator& v2 ) {
   v1.swap( v2 );
}

/// Constructs an end iterator corresponding to a `dip::ImageSliceIterator`
inline ImageSliceIterator ImageSliceEndIterator( Image const& image, dip::uint procDim ) {
   ImageSliceIterator out { image, procDim }; // Tests for `procDim` to be Ok
   out += image.Size( procDim );
   return out;
}

} // namespace dip

#endif // DIP_ITERATORS_H
