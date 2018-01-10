/*
 * DIPlib 3.0
 * This file contains support for 1D and nD iterators.
 *
 * (c)2016-2018, Cris Luengo.
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

#include <tuple>

#include "diplib.h"


/// \file
/// \brief Defines image iterators and line iterators.
/// \see iterators


namespace dip {


/// \defgroup iterators Iterators
/// \ingroup infrastructure
/// \brief Objects to iterate over images and image lines in different ways.
///
/// See \ref using_iterators
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
/// \see \ref using_iterators, ImageIterator, JointImageIterator, BresenhamLineIterator, SampleIterator
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
         DIP_THROW_IF( image.DataType() != DataType( value_type( 0 )), E::WRONG_DATA_TYPE );
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
         out.ptr_ = ptr_;
         out.coord_ = coord_;
         out.size_ = size_;
         out.stride_ = stride_;
         out.nTensorElements_ = nTensorElements_;
         out.tensorStride_ = tensorStride_;
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


// TODO: Image iterators could have 4 modes of operation:
//  1- Fast: The image is flattened as much as possible, to minimize the loop cost. Pixels are accessed in storage
//     order. No Coordinates(), no Index(). This is what you get with the current iterator after calling OptimizeAndFlatten().
//  2- With coordinates: Strides are sorted, the image is iterated not in index order but in storage order. There's
//     access to Coordinates(). Index() exists but doesn't increase monotonically. Difficultly: do we re-sort coordinates
//     when outputting them, or do we index coordinates for incrementing them using an index array (coord[index[ii]])?
//  3- In index order: Strides are not sorted. This is the current iterator without Optimize().
//  4- With neighbor tests: As either #2 or #3, but it also keeps track of whether it's on the edge of the image.
//     IsOnEdge() becomes a trivial function. Every time an index is reset to the beginning, the onEdge flag is set,
//     and the next time operator++() is called, there's a check to see if we're now still on the edge of the image.
//     Every time operator++() is called when onEdge is false, we check to see if we reached the end of the line. When
//     IsOnEdge() is called for every pixel, this would save a massive amount of testing.
//
// We'd use mode #1 in functions that modify each pixel, without regard for where the pixel is or what its
// neighbors are. Mode #2 is used when it's efficient to do stuff along image lines. Mode #3 is used when the index
// ordering is important, e.g. when reading pixel data from file. Mode #4 is used when neighbors need to be checked
// (i.e. dip::Label).
//
// There should be an option to automatically pick the optimal processing dimension.
//
// Option #4 adds some code to the operator++. This one could be a derived class (an additional boolean template
// parameter would not play well with the `Joint` iterators).


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
/// It is possible to obtain neighboring pixel values by adding the neighbor's offset to the current
/// pointer:
///
/// ```cpp
///     *( it.Pointer() + offset )
/// ```
///
/// Note that this offset can cause a read-out-of-bounds or simply access the wrong pixel if the neighbor
/// is not within the image domain. One would have to test for `it.Coordinates()` to be far enough away
/// from the edge of the image. The most optimal way to access neighbors is to iterate over a window
/// within a larger image, such that one can be sure that neighbors always exist. See `dip::ExtendImage`.
///
/// By default, the iterator loops over pixels (or image lines) in linear index order. That is, coordinates
/// change as: {0,0}, {1,0}, {2,0}, ... {0,1}, {1,1}, ... If the image does not have normal strides, this
/// is not the most efficient way of looping over all pixels. The method `Optimize` changes the order in
/// which pixels are accessed to be the order in which they are stored in memory. After calling `Optimize`,
/// the output of `Coordinates` and `Index` no longer match the input image.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see \ref using_iterators, JointImageIterator, LineIterator, SampleIterator, GenericImageIterator
template< typename T >
class DIP_NO_EXPORT ImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;               ///< The data type of the sample, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = T&;               ///< The type of a reference to a sample
      using pointer = T*;                 ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      ImageIterator() : procDim_( std::numeric_limits< dip::uint >::max() ) {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      explicit ImageIterator( Image const& image, dip::uint procDim = std::numeric_limits< dip::uint >::max() ) :
            origin_( static_cast< pointer >( image.Origin() )),
            sizes_( image.Sizes() ),
            strides_( image.Strides() ),
            tensorElements_( image.TensorElements() ),
            tensorStride_( image.TensorStride() ),
            ptr_( origin_ ),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ) {
         DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( image.DataType() != DataType( value_type( 0 )), E::WRONG_DATA_TYPE );
      }

      /// Swap
      void swap( ImageIterator& other ) {
         using std::swap;
         swap( origin_, other.origin_ );
         swap( sizes_, other.sizes_ );
         swap( strides_, other.strides_ );
         swap( tensorElements_, other.tensorElements_ );
         swap( tensorStride_, other.tensorStride_ );
         swap( ptr_, other.ptr_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
      }

      /// Dereference
      reference operator*() const { return *ptr_; }
      /// Dereference
      pointer operator->() const { return ptr_; }
      /// Index into tensor, `it[0]` is equal to `*it`, but `it[1]` is not equal to `*(++it)`.
      reference operator[]( dip::uint index ) const {
         DIP_ASSERT( origin_ );
         return *( ptr_ + static_cast< dip::sint >( index ) * tensorStride_ );
      }

      /// Increment
      ImageIterator& operator++() {
         if( ptr_ ) {
            dip::uint dd;
            for( dd = 0; dd < coords_.size(); ++dd ) {
               if( dd != procDim_ ) {
                  // Increment coordinate and adjust pointer
                  ++coords_[ dd ];
                  ptr_ += strides_[ dd ];
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < sizes_[ dd ] ) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  ptr_ -= static_cast< dip::sint >( coords_[ dd ] ) * strides_[ dd ];
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
         DIP_ASSERT( origin_ );
         return SampleIterator< value_type >( ptr_, tensorStride_ ); // will yield a const iterator if value_type is const!
      }
      /// Get an end iterator over the tensor for the current pixel
      SampleIterator< value_type > end() const { return begin() + tensorElements_; }
      /// Get a const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cbegin() const {
         DIP_ASSERT( origin_ );
         return ConstSampleIterator< value_type >( ptr_, tensorStride_ );
      }
      /// Get an end const iterator over the tensor for the current pixel
      ConstSampleIterator< value_type > cend() const { return cbegin() + tensorElements_; }
      /// Get an iterator over the current line
      LineIterator< value_type > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< value_type >( ptr_, sizes_[ procDim_ ], strides_[ procDim_ ], tensorElements_, tensorStride_ );
      }
      /// Get a const iterator over the current line
      ConstLineIterator< value_type > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< value_type >( ptr_, sizes_[ procDim_ ], strides_[ procDim_ ], tensorElements_, tensorStride_ );
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
      ImageIterator& SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( origin_ );
         DIP_ASSERT( coords.size() == sizes_.size() );
         if( HasProcessingDimension() ) {
            coords[ procDim_ ] = 0;
         }
         ptr_ = origin_ + Image::Offset( coords, strides_, sizes_ ); // tests for coords to be correct
         coords_ = coords;
         return *this;
      }

      /// Return the sizes of the image we're iterating over.
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Return the size along the processing dimension
      dip::uint ProcessingDimensionSize() const {
         DIP_ASSERT( HasProcessingDimension() );
         return sizes_[ procDim_ ];
      }

      /// Return the strides used to iterate over the image.
      IntegerArray const& Strides() const { return strides_; }

      /// Return the stride along the processing dimension
      dip::sint ProcessingDimensionStride() const {
         DIP_ASSERT( HasProcessingDimension() );
         return strides_[ procDim_ ];
      }

      /// \brief Return true if the iterator points at a pixel on the edge of the image.
      ///
      /// If there is a processing dimension, then the iterator always points at an edge pixel; in this case
      /// only returns true if all pixels on the line are edge pixels (i.e. the first and last pixel of the
      /// line are not counted).
      bool IsOnEdge() const {
         for( dip::uint dd = 0; dd < coords_.size(); ++dd ) {
            if( dd != procDim_ ) {
               if(( coords_[ dd ] == 0 ) || ( coords_[ dd ] == sizes_[ dd ] - 1 )) {
                  return true;
               }
            }
         }
         return false;
      }

      /// Return the current pointer
      pointer Pointer() const { return ptr_; }
      /// Return the current offset
      dip::sint Offset() const {
         return ptr_ - origin_;
      }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         return Image::Index( coords_, sizes_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         return origin_ ? ( procDim_ < sizes_.size() ) : false;
      }
      /// \brief Return the processing dimension, the direction of the lines over which the iterator iterates.
      ///
      /// If the return value is larger or equal to the dimensionality (i.e. not one of the image dimensions), then
      /// there is no processing dimension.
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      ImageIterator& Reset() {
         ptr_ = origin_;
         coords_.fill( 0 );
         return *this;
      }

      /// \brief Optimizes the order in which the iterator visits the image pixels.
      ///
      /// The iterator internally reorders and flips image dimensions to change the linear index to match
      /// the storage order (see `dip::Image::StandardizeStrides`).
      /// If the image's strides were not normal, this will significantly increase the speed of reading or
      /// writing to the image. Expanded singleton dimensions are eliminated, meaning that each pixel is
      /// always only accessed once. Additionally, singleton dimensions are ignored.
      ///
      /// After calling this function, `Coordinates` and `Index` no longer match the input image. Do not
      /// use this method if the order of accessing pixels is relevant, or if `Coordinates` are needed.
      ///
      /// Note that the processing dimension stride could change sign. Use `ProcessingDimensionStride`.
      /// If the processing dimension was a singleton dimension, or singleton-expanded, the iterator will
      /// no longer have a singleton dimension. In this case, `HasProcessingDimension` will return false.
      ///
      /// The iterator is reset to the first pixel.
      ImageIterator& Optimize() {
         // Standardize strides
         UnsignedArray order;
         dip::sint offset;
         std::tie( order, offset ) = Image::StandardizeStrides( strides_, sizes_ );
         origin_ = origin_ + offset;
         sizes_ = sizes_.permute( order );
         strides_ = strides_.permute( order );
         procDim_ = order.find( procDim_ );
         coords_.resize( sizes_.size() );
         // Reset iterator
         return Reset();
      }

      /// \brief Like `Optimize`, but additionally folds dimensions together where possible (flattens the image,
      /// so that the iterator has fewer dimensions to work with). The processing dimension is not affected.
      ImageIterator& OptimizeAndFlatten() {
         Optimize();
         // Merge dimensions that can be merged, but not procDim.
         for( dip::uint ii = sizes_.size() - 1; ii > 0; --ii ) {
            if(( ii != procDim_ ) && ( ii - 1 != procDim_ )) {
               if( strides_[ ii - 1 ] * static_cast< dip::sint >( sizes_[ ii - 1 ] ) == strides_[ ii ] ) {
                  // Yes, we can merge these dimensions
                  sizes_[ ii - 1 ] *= sizes_[ ii ];
                  sizes_.erase( ii );
                  strides_.erase( ii );
                  if( ii < procDim_ ) {
                     --procDim_;
                  }
               }
            }
         }
         coords_.resize( sizes_.size() );
         return *this;
      }

   private:
      pointer origin_ = nullptr;
      UnsignedArray sizes_;
      IntegerArray strides_;
      dip::uint tensorElements_ = 1;
      dip::sint tensorStride_ = 0;
      pointer ptr_ = nullptr;
      UnsignedArray coords_;
      dip::uint procDim_;
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
void TestDataType( ImageConstRefArray::const_pointer images ) {
   if( images->get().IsForged() ) {
      DIP_THROW_IF( images->get().DataType() != DataType( T( 0 )), E::WRONG_DATA_TYPE );
   }
   TestDataType< OtherTs... >( images + 1 );
}
template<>
inline void TestDataType<>( const ImageConstRefArray::const_pointer ) {} // End of iteration

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
/// - The `Optimize` method sorts image dimensions to optimize looping over the first image. The iterator will
///   be most efficient if all images share dimension ordering.
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
/// \see \ref using_iterators, ImageIterator, LineIterator, SampleIterator, GenericJointImageIterator
template< typename... Types >
class DIP_NO_EXPORT JointImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      template< dip::uint I > using value_type = typename std::tuple_element< I, std::tuple< Types ... >>::type;
      ///< The data type of the sample, obtained when dereferencing the iterator
      using difference_type = dip::sint;                ///< The type of distances between iterators
      template< dip::uint I > using reference = value_type< I >&; ///< The type of a reference to a sample
      template< dip::uint I > using pointer = value_type< I >*;   ///< The type of a pointer to a sample

      /// \brief Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an
      /// end iterator.
      JointImageIterator() : procDim_( std::numeric_limits< dip::uint >::max() ), atEnd_( true ) {
         origins_.fill( nullptr );
         offsets_.fill( 0 );
      }
      /// \brief To construct a useful iterator, provide `N` images (`N` equal to the number of template parameters),
      /// and optionally a processing dimension.
      explicit JointImageIterator( ImageConstRefArray const& images, dip::uint procDim = std::numeric_limits< dip::uint >::max() ):
            procDim_( procDim ), atEnd_( false ) {
         DIP_THROW_IF( images.size() != N, E::ARRAY_ILLEGAL_SIZE );
         Image const& img0 = images[ 0 ].get();
         DIP_THROW_IF( !img0.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( img0.DataType() != DataType( value_type< 0 >( 0 )), E::WRONG_DATA_TYPE );
         coords_.resize( img0.Dimensionality(), 0 );
         sizes_ = img0.Sizes();
         origins_[ 0 ] = img0.Origin();
         sizeOf_[ 0 ] = static_cast< sint8 >( img0.DataType().SizeOf() ); // will always fit in an 8-bit signed integer (sizeof(dcomplex)==16).
         stridess_[ 0 ] = img0.Strides();
         tensorElementss_[ 0 ] = img0.TensorElements();
         tensorStrides_[ 0 ] = img0.TensorStride();
         offsets_.fill( 0 );
         for( dip::uint ii = 1; ii < N; ++ii ) {
            Image const& imgI = images[ ii ].get();
            if( imgI.IsForged() ) {
               DIP_THROW_IF( !CompareSizes( imgI ), E::SIZES_DONT_MATCH );
               origins_[ ii ] = imgI.Origin();
               sizeOf_[ ii ] = static_cast< sint8 >( imgI.DataType().SizeOf() ); // will always fit in an 8-bit signed integer (sizeof(dcomplex)==16).
               stridess_[ ii ] = imgI.Strides();
               tensorElementss_[ ii ] = imgI.TensorElements();
               tensorStrides_[ ii ] = imgI.TensorStride();
            } else {
               origins_[ ii ] = nullptr;
               sizeOf_[ ii ] = 0;
               stridess_[ ii ] = IntegerArray( sizes_.size(), 0 );
               tensorElementss_[ ii ] = 0;
               tensorStrides_[ ii ] = 0;
            }
         }
         detail::TestDataType< Types... >( images.data() );
      }

      /// Swap
      void swap( JointImageIterator& other ) {
         using std::swap;
         swap( origins_, other.origins_ );
         swap( sizes_, other.sizes_ );
         swap( stridess_, other.stridess_ );
         swap( tensorElementss_, other.tensorElementss_ );
         swap( tensorStrides_, other.tensorStrides_ );
         swap( offsets_, other.offsets_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( sizeOf_, other.sizeOf_ );
         swap( atEnd_, other.atEnd_ );
      }

      /// Index into image tensor for image `I`
      template< dip::uint I >
      reference< I > Sample( dip::uint index ) const {
         return *( Pointer< I >() + static_cast< dip::sint >( index ) * tensorStrides_[ I ] );
      }
      /// Index into image tensor for image 0.
      reference< 0 > InSample( dip::uint index ) const { return Sample< 0 >( index ); }
      /// Index into image tensor for image 1.
      reference< 1 > OutSample( dip::uint index ) const { return Sample< 1 >( index ); }
      /// Get first tensor element for image `I`.
      template< dip::uint I >
      reference< I > Sample() const {
         return *Pointer< I >();
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
                     offsets_[ ii ] += stridess_[ ii ][ dd ];
                  }
                  // Check whether we reached the last pixel of the line
                  if( coords_[ dd ] < sizes_[ dd ] ) {
                     break;
                  }
                  // Rewind, the next loop iteration will increment the next coordinate
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     offsets_[ ii ] -= static_cast< dip::sint >( coords_[ dd ] ) * stridess_[ ii ][ dd ];
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
      SampleIterator< value_type< I >> begin() const {
         return SampleIterator< value_type< I >>( Pointer< I >(), tensorStrides_[ I ] ); // will yield a const iterator if value_type is const!
      }
      /// Get an end iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      SampleIterator< value_type< I >> end() const { return begin() + tensorElementss_[ I ]; }
      /// Get a const iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      ConstSampleIterator< value_type< I >> cbegin() const {
         return ConstSampleIterator< value_type< I >>( Pointer< I >(), tensorStrides_[ I ] );
      }
      /// Get an end const iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      ConstSampleIterator< value_type< I >> cend() const { return cbegin() + tensorElementss_[ I ]; }
      /// Get an iterator over the current line of image `I`
      template< dip::uint I >
      LineIterator< value_type< I >> GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(),
                       "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< value_type< I >>( Pointer< I >(), sizes_[ procDim_ ], stridess_[ I ][ procDim_ ],
                                                tensorElementss_[ I ], tensorStrides_[ I ] );
      }
      /// Get a const iterator over the current line of image `I`
      template< dip::uint I >
      ConstLineIterator< value_type< I >> GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(),
                       "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< value_type< I >>( Pointer< I >(), sizes_[ procDim_ ], stridess_[ I ][ procDim_ ],
                                                     tensorElementss_[ I ], tensorStrides_[ I ] );
      }

      /// \brief Equality comparison, is equal if the two iterators have the same coordinates. It is possible to compare
      /// `JointImageIterators` with different images and different types.
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
      JointImageIterator& SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( coords.size() == sizes_.size() );
         if( HasProcessingDimension() ) {
            coords[ procDim_ ] = 0;
         }
         for( dip::uint ii = 0; ii < N; ++ii ) {
            offsets_[ ii ] = Image::Offset( coords, stridess_[ ii ], sizes_ );
         }
         coords_ = coords;
         return *this;
      }

      /// Return the sizes of the images we're iterating over.
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Return the size along the processing dimension.
      dip::uint ProcessingDimensionSize() const {
         DIP_ASSERT( HasProcessingDimension() );
         return sizes_[ procDim_ ];
      }

      /// Return the strides used to iterate over the image `I`.
      template< dip::uint I >
      IntegerArray const& Strides() const { return stridess_[ I ]; }

      /// Return the stride along the processing dimension.
      template< dip::uint I >
      dip::sint ProcessingDimensionStride() const {
         DIP_ASSERT( HasProcessingDimension() );
         return stridess_[ I ][ procDim_ ];
      }

      /// \brief Return true if the iterator points at a pixel on the edge of the image.
      ///
      /// If there is a processing dimension, then the iterator always points at an edge pixel; in this case
      /// only returns true if all pixels on the line are edge pixels (i.e. the first and last pixel of the
      /// line are not counted).
      bool IsOnEdge() const {
         for( dip::uint dd = 0; dd < coords_.size(); ++dd ) {
            if( dd != procDim_ ) {
               if(( coords_[ dd ] == 0 ) || ( coords_[ dd ] == sizes_[ dd ] - 1 )) {
                  return true;
               }
            }
         }
         return false;
      }

      /// Return the current pointer for image `I`
      template< dip::uint I >
      pointer< I > Pointer() const {
         DIP_ASSERT( origins_[ I ] );
         DIP_ASSERT( !atEnd_ );
         return static_cast< pointer< I >>( origins_[ I ] ) + offsets_[ I ];
      }
      /// Return the current pointer for image 0.
      pointer< 0 > InPointer() const { return Pointer< 0 >(); }
      /// Return the current pointer for image 1.
      pointer< 1 > OutPointer() const { return Pointer< 1 >(); }
      /// Return the current offset for image `I`
      template< dip::uint I >
      dip::sint Offset() const { return offsets_[ I ]; }
      /// Index into image tensor for image 0.
      dip::sint InOffset() const { return offsets_[ 0 ]; }
      /// Index into image tensor for image 1.
      dip::sint OutOffset() const { return offsets_[ 1 ]; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         return Image::Index( coords_, sizes_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         return origins_[ 0 ] ? procDim_ < sizes_.size() : false;
      }
      /// \brief Return the processing dimension, the direction of the lines over which the iterator iterates.
      ///
      /// If the return value is larger or equal to the dimensionality (i.e. not one of the image dimensions), then
      /// there is no processing dimension.
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      JointImageIterator& Reset() {
         offsets_.fill( 0 );
         coords_.fill( 0 );
         atEnd_ = false;
         return *this;
      }

      /// \brief Optimizes the order in which the iterator visits the image pixels.
      ///
      /// The iterator internally reorders and flips image dimensions to change the linear index to match
      /// the storage order of the first image (see `dip::Image::StandardizeStrides`), or image `n`
      /// if a parameter is given.
      /// If the image's strides were not normal, this will significantly increase the speed of reading or
      /// writing to the image. Expanded singleton dimensions are eliminated only if the dimension is expanded
      /// in all images. Additionally, singleton dimensions are ignored.
      ///
      /// After calling this function, `Coordinates` and `Index` no longer match the input images. Do not
      /// use this method if the order of accessing pixels is relevant, or if `Coordinates` are needed.
      ///
      /// Note that the processing dimension stride could change sign. Use `ProcessingDimensionStride`.
      /// If the processing dimension was a singleton dimension, or singleton-expanded, the iterator will
      /// no longer have a singleton dimension. In this case, `HasProcessingDimension` will return false.
      ///
      /// The iterator is reset to the first pixel.
      JointImageIterator& Optimize( dip::uint n = 0 ) {
         DIP_ASSERT( origins_[ n ] );
         //std::tie( order, offset ) = Image::StandardizeStrides( stridess_[ n ], sizes_ );
         dip::uint nd = sizes_.size();
         DIP_ASSERT( stridess_[ n ].size() == nd );
         // Un-mirror and un-expand
         offsets_.fill( 0 );
         for( dip::uint jj = 0; jj < nd; ++jj ) {
            if( stridess_[ n ][ jj ] < 0 ) {
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  offsets_[ ii ] += static_cast< dip::sint >( sizes_[ jj ] - 1 ) * stridess_[ ii ][ jj ];
                  stridess_[ ii ][ jj ] = -stridess_[ ii ][ jj ];
               }
            } else if( stridess_[ n ][ jj ] == 0 ) {
               bool all = true;
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  if( stridess_[ ii ][ jj ] != 0 ) {
                     all = false;
                     break;
                  }
               }
               if( all ) {
                  sizes_[ jj ] = 1;
               }
               // TODO: What if not `all`? This dimension will be sorted first, should it???
            }
         }
         // Sort strides
         UnsignedArray order = stridess_[ n ].sorted_indices();
         // Remove singleton dimensions
         dip::uint jj = 0;
         for( dip::uint ii = 0; ii < nd; ++ii ) {
            if( sizes_[ order[ ii ]] > 1 ) {
               order[ jj ] = order[ ii ];
               ++jj;
            }
         }
         order.resize( jj );
         sizes_ = sizes_.permute( order );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            origins_[ ii ] = static_cast< uint8* >( origins_[ ii ] ) + offsets_[ ii ] * sizeOf_[ ii ];
            stridess_[ ii ] = stridess_[ ii ].permute( order );
         }
         procDim_ = order.find( procDim_ );
         coords_.resize( sizes_.size() );
         // Reset iterator
         return Reset();
      }

      /// \brief Like `Optimize`, but additionally folds dimensions together where possible (flattens the image,
      /// so that the iterator has fewer dimensions to work with). The processing dimension is not affected.
      JointImageIterator& OptimizeAndFlatten( dip::uint n = 0 ) {
         Optimize( n );
         // Merge dimensions that can be merged, but not procDim.
         for( dip::uint jj = sizes_.size() - 1; jj > 0; --jj ) {
            if(( jj != procDim_ ) && ( jj - 1 != procDim_ )) {
               bool all = true;
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  if( stridess_[ ii ][ jj - 1 ] * static_cast< dip::sint >( sizes_[ jj - 1 ] ) != stridess_[ ii ][ jj ] ) {
                     all = false;
                     break;
                  }
               }
               if( all ) {
                  // Yes, we can merge these dimensions
                  sizes_[ jj - 1 ] *= sizes_[ jj ];
                  sizes_.erase( jj );
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     stridess_[ ii ].erase( jj );
                  }
                  if( jj < procDim_ ) {
                     --procDim_;
                  }
               }
            }
         }
         coords_.resize( sizes_.size() );
         return *this;
      }

   private:
      constexpr static dip::uint N = sizeof...( Types );
      static_assert( N > 1, "JointImageIterator needs at least two type template arguments" );
      std::array< void*, N > origins_;
      UnsignedArray sizes_;
      std::array< IntegerArray, N > stridess_;
      std::array< dip::uint, N > tensorElementss_;
      std::array< dip::sint, N > tensorStrides_;
      std::array< dip::sint, N > offsets_;
      UnsignedArray coords_;
      dip::uint procDim_;
      std::array< sint8, N > sizeOf_;
      bool atEnd_;

      // Compares size of image to sizes_
      bool CompareSizes( Image const& image ) const {
         if( sizes_.size() != image.Dimensionality() ) {
            return false;
         }
         for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
            if(( ii != procDim_ ) && ( sizes_[ ii ] != image.Size( ii ))) {
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
