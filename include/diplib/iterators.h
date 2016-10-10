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
#include "diplib/error.h"
#include "diplib/types.h"
#include "diplib/image.h"

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
      using value_type = T;
      using difference_type = dip::sint;
      using reference = T&;
      using pointer = T*;
      /// Default constructor yields an invalid iterator that cannot be dereferenced
      SampleIterator() : stride_( 1 ), ptr_( nullptr ) {}
      /// To construct a useful iterator, provide a pointer and a stride
      SampleIterator( T* ptr, dip::sint stride ) : stride_( stride ), ptr_( ptr ) {}
      /// Swap
      void swap( SampleIterator& other ) {
         using std::swap;
         swap( stride_, other.stride_ );
         swap( ptr_, other.ptr_ );
      }
      /// Convert from non-const iterator to const iterator
      operator SampleIterator< T const >() const {
         return SampleIterator< T const >( ptr_, stride_ );
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      reference operator->() const { return *ptr_; }
      /// Index
      reference operator[]( dip::sint index ) const { return *( ptr_ + index * stride_ ); }
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
      SampleIterator& operator+=( dip::sint index ) {
         ptr_ += index * stride_;
         return *this;
      }
      /// Subtract integer
      SampleIterator& operator-=( dip::sint index ) {
         ptr_ -= index * stride_;
         return *this;
      }
      /// Add integer
      friend SampleIterator operator+( SampleIterator it, dip::sint n ) {
         it += n;
         return it;
      }
      /// Add integer
      friend SampleIterator operator+( dip::sint n, SampleIterator it ) {
         it += n;
         return it;
      }
      /// Subtract integer
      friend SampleIterator operator-( SampleIterator it, dip::sint n ) {
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
/// \see ImageIterator, JointImageIterator, SampleIterator
template< typename T >
class LineIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;
      using difference_type = dip::sint;
      using reference = T&;
      using pointer = T*;
      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      LineIterator() {}
      /// To construct a useful iterator, provide an image, the coordinate of the start pixel, the processing dimension,
      /// and optionally a boundary condition
      LineIterator( Image const& image, UnsignedArray const& coords, dip::uint procDim, BoundaryCondition bc = BoundaryCondition::DEFAULT ) :
            boundaryCondition_( bc ) {
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
         swap( boundaryCondition_, other.boundaryCondition_ );
      }
      /// Convert from non-const iterator to const iterator
      operator LineIterator< T const >() const {
         LineIterator< T const > out;
         out.ptr_ = ptr_ ;
         out.coord_ = coord_ ;
         out.size_ = size_ ;
         out.stride_ = stride_ ;
         out.nTensorElements_ = nTensorElements_ ;
         out.tensorStride_ = tensorStride_ ;
         out.boundaryCondition_ = boundaryCondition_ ;
         return out;
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      reference operator->() const { return *ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( dip::sint index ) const { return *( ptr_ + index * tensorStride_ ); }
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
      SampleIterator< T > begin() const { return SampleIterator< T >( ptr_, tensorStride_ ); } // will yield a const iterator if T is const!
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< T > end() const { return begin() + nTensorElements_; }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< T > cbegin() const { return ConstSampleIterator< T >( ptr_, tensorStride_ ); }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< T > cend() const { return cbegin() + nTensorElements_; }
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
      /// Return the current coordinates
      dip::uint const& Coordinate() const { return coord_; }
      /// Return the current pointer
      pointer Pointer() const { return ptr_; }
      void SetBoundaryCondition( BoundaryCondition bc ) {
         boundaryCondition_ = bc;
      }
   private:
      pointer ptr_ = nullptr;
      dip::uint coord_ = 0;
      dip::uint size_ = 0;
      dip::sint stride_;
      dip::uint nTensorElements_ = 0;
      dip::sint tensorStride_;
      BoundaryCondition boundaryCondition_ = BoundaryCondition::DEFAULT;
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
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// \see JointImageIterator, LineIterator, SampleIterator, GenericImageIterator
template< typename T >
class ImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;
      using difference_type = dip::sint;
      using reference = T&;
      using pointer = T*;
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
         dip_ThrowIf( image_->DataType() != DataType( T(0) ), E::WRONG_DATA_TYPE );
      }
      /// To construct a useful iterator, provide an image, a boundary condition array, and optionally a processing dimension
      ImageIterator( Image const& image, BoundaryConditionArray const& bc, dip::sint procDim = -1 ) :
            image_( &image ),
            ptr_( static_cast< pointer >( image.Origin() )),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            boundaryCondition_( image.Dimensionality(), BoundaryCondition::DEFAULT ) {
         dip_ThrowIf( !image_->IsForged(), E::IMAGE_NOT_FORGED );
         dip_ThrowIf( image_->DataType() != DataType( T(0) ), E::WRONG_DATA_TYPE );
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
      operator ImageIterator< T const >() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         ImageIterator< T const > out( *image_, boundaryCondition_, procDim_ );
         out.SetCoordinates( coords_ );
         return out;
      }
      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      reference operator->() const { return *ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( dip::sint index ) const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return *( ptr_ + index * image_->TensorStride() );
      }
      // TODO: methods to access the pixel's neighborhood.
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
      SampleIterator< T > begin() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return SampleIterator< T >( ptr_, image_->TensorStride() ); // will yield a const iterator if T is const!
      }
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< T > end() const { return begin() + image_->TensorElements(); }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< T > cbegin() const {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
         return ConstSampleIterator< T >( ptr_, image_->TensorStride() );
      }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< T > cend() const { return cbegin() + image_->TensorElements(); }
      /// Get an iterator over the current line
      LineIterator< T > GetLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< T >( *image_, coords_, procDim_, boundaryCondition_[ procDim_ ] );
      }
      /// Get a const iterator over the current line
      ConstLineIterator< T > GetConstLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< T >( *image_, coords_, procDim_, boundaryCondition_[ procDim_ ] );
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
      void SetCoordinates( UnsignedArray const& coords ) {
         dip_ThrowIf( !image_, E::ITERATOR_NOT_VALID );
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
      /// Set the processing dimension, causing the iterator to iterate over all lines along this dimension
      void SetProcessingDimension( dip::sint d ) { procDim_ = d; }
      /// Reset the proccessing dimension, causing the iterator to iterate over all pixels
      void RemoveProcessingDimension() { procDim_ = -1; }
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
/// and write access of the second (output). It behaves similarly to `dip::ImageIterator` with the following
/// differences:
///
/// This iterator is not dereferenceable. The reason is that it points at two pixels at the same time
/// (that is, one pixel in each image). Instead, use the `In` and `Out` methods to obtain references to
/// to the first sample of each pixel. Use the `InElement` and `OutElement` methods instead of the `[]`
/// operator to access the other samples.
///
/// Instead of `GetLineIterator`, use `GetInLineIterator` and `GetOutLineIterator`. Likewise, instead of
/// `Pointer` and `Offset` methods, use `InPointer`, `OutPointer`, `InOffset` and `OutOffset`.
///
/// \see ImageIterator, LineIterator, SampleIterator, GenericJointImageIterator
// TODO: Allow input and output image to differ in size along the processing dimension.
template< typename inT, typename outT >
class JointImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
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
         inImage_->CompareProperties( *outImage_, Option::CmpProps_Sizes + Option::CmpProps_TensorElements );
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
         inImage_->CompareProperties( *outImage_, Option::CmpProps_Sizes + Option::CmpProps_TensorElements );
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
      // TODO: methods to access the pixel's neighborhood.
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
         return ConstLineIterator< inT >( *inImage_, coords_, procDim_, boundaryCondition_[ procDim_ ] );
      }
      /// Get an iterator over the current line of the output image
      LineIterator< inT > GetOutLineIterator() const {
         dip_ThrowIf( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< outT >( *outImage_, coords_, procDim_, boundaryCondition_[ procDim_ ] );
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
      void SetCoordinates( UnsignedArray const& coords ) {
         dip_ThrowIf( !inImage_ || !outImage_, E::ITERATOR_NOT_VALID );
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
      /// Set the processing dimension, causing the iterator to iterate over all lines along this dimension
      void SetProcessingDimension( dip::sint d ) { procDim_ = d; }
      /// Reset the proccessing dimension, causing the iterator to iterate over all pixels
      void RemoveProcessingDimension() { procDim_ = -1; }
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
};

template< typename inT, typename outT >
inline void swap( JointImageIterator< inT, outT >& v1, JointImageIterator< inT, outT >& v2 ) {
   v1.swap( v2 );
}


//
// Two generic image iterators, non-templated
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
         return ( image_ == other.image_ ) && ( offset_ == other.offset_ );
      }
      /// Inequality comparison
      bool operator!=( GenericImageIterator const& other ) const {
         return ( image_ != other.image_ ) || ( offset_ != other.offset_ );
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
      /// Set the processing dimension, causing the iterator to iterate over all lines along this dimension
      void SetProcessingDimension( dip::sint d ) { procDim_ = d; }
      /// Reset the proccessing dimension, causing the iterator to iterate over all pixels
      void RemoveProcessingDimension() { procDim_ = -1; }
      /// Set the boundary condition for accessing pixels outside the image boundary; dimensions not specified will
      /// use the default boundary condition.
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
/// Example usage from `dip::Image:Copy`:
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
         inImage_->CompareProperties( *outImage_, Option::CmpProps_Sizes + Option::CmpProps_TensorElements );
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
         return ( inImage_ == other.inImage_ ) && ( outImage_ == other.outImage_ ) &&
                ( inOffset_ == other.inOffset_ ) && ( outOffset_ == other.outOffset_ );
      }
      /// Inequality comparison
      bool operator!=( GenericJointImageIterator const& other ) const {
         return ( inImage_ != other.inImage_ ) || ( outImage_ != other.outImage_ ) ||
                ( inOffset_ != other.inOffset_ ) || ( outOffset_ != other.outOffset_ );
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
      /// Set the processing dimension, causing the iterator to iterate over all lines along this dimension
      void SetProcessingDimension( dip::sint d ) { procDim_ = d; }
      /// Reset the proccessing dimension, causing the iterator to iterate over all pixels
      void RemoveProcessingDimension() { procDim_ = -1; }
      /// Set the boundary condition for accessing pixels outside the image boundary; dimensions not specified will
      /// use the default boundary condition.
   private:
      Image const* inImage_ = nullptr;
      Image const* outImage_ = nullptr;
      dip::sint inOffset_ = 0;
      dip::sint outOffset_ = 0;
      UnsignedArray coords_ = {};
      dip::sint procDim_ = -1;
};

inline void swap( GenericJointImageIterator& v1, GenericJointImageIterator& v2 ) {
   v1.swap( v2 );
}


// TODO: How to access the pixel's neighborhood, and how to use the boundary condition?

// TODO: Should the line iterator be able to iterate past the image boundaries? (as originally proposed in DIPthoughts.md)

} // namespace dip

#endif // DIP_ITERATORS_H
