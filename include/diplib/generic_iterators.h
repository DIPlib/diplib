/*
 * DIPlib 3.0
 * This file contains support for nD iterators that are independent of image data type.
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


/// \brief An iterator to iterate over pixels along a straight line.
///
/// The iterator is created giving two points: a start and and end point.
/// The iterator can be incremented until it reaches past the end point. When it does, the
/// iterator will become invalid. An invalid iterator will test false. The `IsAtEnd` method
/// can be used instead to test for this condition. It is also possible to compare two iterators
/// for equality (i.e. to compare against an end iterator).
///
/// Dereferencing the iterator yields the offset to the current pixel.
///
/// Satisfies all the requirements for a mutable [ForwardIterator](http://en.cppreference.com/w/cpp/iterator).
///
/// \see LineIterator, SampleIterator
class DIP_NO_EXPORT BresenhamLineIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = dip::sint;       ///< The type of an offset
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = value_type const&;///< The type of a reference to an offset
      using pointer = value_type const*;  ///< The type of a pointer an offset

      constexpr static dfloat epsilon = 1e-5;
      constexpr static dfloat delta = 1.0 - epsilon;

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      BresenhamLineIterator() {}
      /// To construct a useful iterator, provide image strides, and coordinates of the start and end pixels
      BresenhamLineIterator( IntegerArray strides, UnsignedArray start, UnsignedArray const& end ) :
            coord_( std::move( start )), strides_( std::move( strides )) {
         dip::uint nDims = strides_.size();
         DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( coord_.size() != nDims, E::ARRAY_SIZES_DONT_MATCH );
         DIP_THROW_IF( end.size() != nDims, E::ARRAY_SIZES_DONT_MATCH );
         stepSize_.resize( nDims, 1.0 );
         length_ = 1; // to prevent dividing by 0 later on.
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            dip::uint size;
            if( coord_[ ii ] < end[ ii ] ) {
               size = end[ ii ] - coord_[ ii ] + 1;
               stepSize_[ ii ] = static_cast< dfloat >( size );
            } else {
               size = coord_[ ii ] - end[ ii ] + 1;
               stepSize_[ ii ] = -static_cast< dfloat >( size );
            }
            length_ = std::max( length_, size );
            if( size == 1 ) {
               stepSize_[ ii ] = 0.0; // don't step anywhere in this direction
            }
         }
         pos_ = FloatArray( coord_ );
         offset_ = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            stepSize_[ ii ] /= static_cast< dfloat >( length_ );
            // Here, we presume that we won't chain more than 100,000 pixels in a row...
            if( stepSize_[ ii ] < 0 ) {
               pos_[ ii ] += delta; // start at the opposite edge of the pixel, such that `floor` still gives 0.
            } else {
               pos_[ ii ] += epsilon; // add a small value to prevent rounding errors.
            }
            offset_ += static_cast< dip::sint >( coord_[ ii ] ) * strides_[ ii ];
         }
         --length_; // we've got one fewer pixels after the current one
      }
      /// To construct a useful iterator, provide image strides, a step size, a start position, and a length
      BresenhamLineIterator( IntegerArray strides, FloatArray stepSize, UnsignedArray start, dip::uint length ) :
            coord_( std::move( start )), stepSize_( std::move( stepSize )), length_( length - 1 ), strides_( std::move( strides )) {
         dip::uint nDims = strides_.size();
         DIP_THROW_IF( nDims < 2, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( stepSize_.size() != nDims, E::ARRAY_SIZES_DONT_MATCH );
         DIP_THROW_IF( coord_.size() != nDims, E::ARRAY_SIZES_DONT_MATCH );
         dfloat maxStepSize = 0;
         for( auto l: stepSize_ ) {
            maxStepSize = std::max( maxStepSize, std::abs( l ));
         }
         DIP_THROW_IF( maxStepSize == 0.0, "Step size is 0" );
         pos_ = FloatArray( coord_ );
         offset_ = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            stepSize_[ ii ] /= static_cast< dfloat >( maxStepSize );
            // Here, we presume that we won't chain more than 100,000 pixels in a row...
            if( stepSize_[ ii ] < 0 ) {
               pos_[ ii ] += delta; // start at the opposite edge of the pixel, such that `floor` still gives 0.
            } else {
               pos_[ ii ] += epsilon; // add a small value to prevent rounding errors.
            }
            offset_ += static_cast< dip::sint >( coord_[ ii ] ) * strides_[ ii ];
         }
      }

      /// Swap
      void swap( BresenhamLineIterator& other ) {
         using std::swap;
         swap( offset_, other.offset_ );
         swap( coord_, other.coord_ );
         swap( pos_, other.pos_ );
         swap( stepSize_, other.stepSize_ );
         swap( length_, other.length_ );
         swap( strides_, other.strides_ );
      }

      /// Dereference
      dip::sint operator*() const { return offset_; }
      // Dereference (makes no sense)
      //pointer operator->() const { return &offset_; }

      /// Increment
      BresenhamLineIterator& operator++() {
         if( length_ == 0 ) {
            coord_.clear(); // mark we're done
            return *this;
         }
         pos_ += stepSize_;
         for( dip::uint ii = 0; ii < pos_.size(); ++ii ) {
            dip::sint newcoord = floor_cast( pos_[ ii ] );
            dip::sint diff = newcoord - static_cast< dip::sint >( coord_[ ii ] );
            if( diff != 0 ) {
               offset_ += diff * strides_[ ii ];
               coord_[ ii ] = static_cast< dip::uint >( newcoord );
            }
         }
         --length_;
         return *this;
      }
      /// Increment
      BresenhamLineIterator operator++( int ) {
         BresenhamLineIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Equality comparison (is equal if coordinates are identical)
      bool operator==( BresenhamLineIterator const& other ) const {
         return coord_ == other.coord_;
      }
      /// Inequality comparison (is unequal if coordinates are not identical)
      bool operator!=( BresenhamLineIterator const& other ) const {
         return coord_ != other.coord_;
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return coord_.empty(); }
      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !coord_.empty(); }

      /// Return the current coordinates in the image
      UnsignedArray const& Coordinates() const { return coord_; }
      /// Return the current offset
      dip::sint Offset() const { return offset_; }
      /// Return the number of pixels left on the line after the current one
      dip::uint Length() const { return length_; }

   private:
      dip::sint offset_ = 0;  // keeps the offset to the current coordinates
      UnsignedArray coord_;   // keeps the current integer coordinates
      FloatArray pos_;        // keeps the current sub-pixel position (floor(pos_) == coord_)
      FloatArray stepSize_;   // sub-pixel increment along each dimension
      dip::uint length_ = 0;  // counts down to 0.
      IntegerArray strides_;  // image strides, to compute offset.
};

inline void swap( BresenhamLineIterator& v1, BresenhamLineIterator& v2 ) {
   v1.swap( v2 );
}


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
/// Example usage from `dip::CopyTo`:
///
/// ```cpp
///     auto arrIt = offsets.begin();
///     GenericImageIterator<> srcIt( src );
///     do {
///        Image::Pixel d( dest.Pointer( *arrIt ), dest.DataType(), dest.Tensor(), dest.TensorStride() );
///        d = *srcIt;
///     } while( ++arrIt, ++srcIt ); // these two must end at the same time, we test the image iterator, as indIt should be compared with the end iterator.
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see \ref using_iterators, ImageIterator, GenericJointImageIterator
template< typename T = dfloat >
class DIP_NO_EXPORT GenericImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image::CastPixel< T >; ///< The type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = value_type;       ///< The type of a reference to a pixel (note dip::Image::CastPixel references a value in the image)
      using pointer = value_type*;        ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericImageIterator() : procDim_( std::numeric_limits< dip::uint >::max() ), sizeOf_( 0 ), atEnd_( true ) {}
      /// To construct a useful iterator, provide an image and optionally a processing dimension
      explicit GenericImageIterator( Image const& image, dip::uint procDim = std::numeric_limits< dip::uint >::max() ) :
            origin_( image.Origin() ),
            sizes_( image.Sizes() ),
            strides_( image.Strides() ),
            tensorElements_( image.TensorElements() ),
            tensorStride_( image.TensorStride() ),
            offset_( 0 ),
            coords_( image.Dimensionality(), 0 ),
            procDim_( procDim ),
            dataType_( image.DataType() ),
            sizeOf_( static_cast< sint8 >( dataType_.SizeOf() )),
            atEnd_( false ) {
         DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
      }

      /// Swap
      template< typename S >
      void swap( GenericImageIterator< S >& other ) {
         using std::swap;
         swap( origin_, other.origin_ );
         swap( sizes_, other.sizes_ );
         swap( strides_, other.strides_ );
         swap( tensorElements_, other.tensorElements_ );
         swap( tensorStride_, other.tensorStride_ );
         swap( offset_, other.offset_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( dataType_, other.dataType_ );
         swap( sizeOf_, other.sizeOf_ );
         swap( atEnd_, other.atEnd_ );
      }

      /// Dereference
      value_type operator*() const {
         return value_type( Pointer(), dataType_, Tensor( tensorElements_ ), tensorStride_ );
      }
      /// Dereference
      value_type operator->() const {
         return operator*();
      }
      /// Index into tensor, `it[index]` is equal to `(*it)[index]`.
      Image::CastSample< T > operator[]( dip::uint index ) const {
         return Image::CastSample< T >( Pointer( index ), dataType_ );
      }

      /// Increment
      GenericImageIterator& operator++() {
         DIP_ASSERT( origin_ );
         dip::uint dd;
         for( dd = 0; dd < coords_.size(); ++dd ) {
            if( dd != procDim_ ) {
               // Increment coordinate and adjust offset
               ++coords_[ dd ];
               offset_ += strides_[ dd ];
               // Check whether we reached the last pixel of the line ...
               if( coords_[ dd ] < sizes_[ dd ] ) {
                  break;
               }
               // Rewind, the next loop iteration will increment the next coordinate
               offset_ -= static_cast< dip::sint >( coords_[ dd ] ) * strides_[ dd ];
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
         return typename value_type::Iterator( Pointer(), dataType_, tensorStride_ );
      }
      /// Get an end iterator over the tensor for the current pixel
      typename value_type::Iterator end() const {
         return typename value_type::Iterator( Pointer(), dataType_, tensorStride_, tensorElements_ );
      }
      /// Get an iterator over the current line
      template< typename S = T >
      LineIterator< S > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< S >( Pointer(), sizes_[ procDim_ ], strides_[ procDim_ ], tensorElements_, tensorStride_ );
      }
      /// Get a const iterator over the current line
      template< typename S = T >
      ConstLineIterator< S > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< S >( Pointer(), sizes_[ procDim_ ], strides_[ procDim_ ], tensorElements_, tensorStride_ );
      }

      /// \brief Equality comparison, is equal if the two iterators have the same coordinates. It is possible to compare
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
      GenericImageIterator& SetCoordinates( UnsignedArray coords ) {
         DIP_ASSERT( origin_ );
         DIP_ASSERT( coords.size() == sizes_.size() );
         if( HasProcessingDimension() ) {
            coords[ procDim_ ] = 0;
         }
         offset_ = Image::Offset( coords, strides_, sizes_ ); // tests for coords to be correct
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
      void* Pointer() const {
         DIP_ASSERT( origin_ );
         return static_cast< uint8* >( origin_ ) + offset_ * sizeOf_;
      }
      /// Return a pointer to the tensor element `index`
      void* Pointer( dip::uint index ) const {
         DIP_ASSERT( origin_ );
         return static_cast< uint8* >( origin_ ) + ( offset_ + static_cast< dip::sint >( index ) * tensorStride_ ) * sizeOf_;
      }
      /// Return the current offset
      dip::sint Offset() const { return offset_; }
      /// Return the current index, which is computed: this function is not trivial
      dip::uint Index() const {
         return Image::Index( coords_, sizes_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         return origin_ ? ( procDim_ < sizes_.size() ) : false;
      }
      /// Return the processing dimension, the direction of the lines over which the iterator iterates
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      GenericImageIterator& Reset() {
         offset_ = 0;
         coords_.fill( 0 );
         atEnd_ = false;
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
      GenericImageIterator& Optimize() {
         // Standardize strides
         UnsignedArray order;
         dip::sint offset;
         std::tie( order, offset ) = Image::StandardizeStrides( strides_, sizes_ );
         origin_ = static_cast< uint8* >( origin_ ) + offset * sizeOf_;
         sizes_ = sizes_.permute( order );
         strides_ = strides_.permute( order );
         procDim_ = order.find( procDim_ );
         coords_.resize( sizes_.size() );
         // Reset iterator
         return Reset();
      }

      /// \brief Like `Optimize`, but additionally folds dimensions together where possible (flattens the image,
      /// so that the iterator has fewer dimensions to work with). The processing dimension is not affected.
      GenericImageIterator& OptimizeAndFlatten() {
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
      void* origin_ = nullptr;
      UnsignedArray sizes_;
      IntegerArray strides_;
      dip::uint tensorElements_;
      dip::sint tensorStride_ = 0;
      dip::sint offset_ = 0;
      UnsignedArray coords_;
      dip::uint procDim_;
      DataType dataType_;
      sint8 sizeOf_;
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
/// Example usage slightly modified from `dip::Image::Copy`:
///
/// ```cpp
///     dip::uint processingDim = Framework::OptimalProcessingDim( src );
///     auto it = dip::GenericJointImageIterator< 2 >( { src, dest }, processingDim );
///     do {
///        detail::CopyBuffer(
///              it.InPointer(),
///              src.DataType(),
///              src.Stride( processingDim ),
///              src.TensorStride(),
///              it.OutPointer(),
///              dest.DataType(),
///              dest.Stride( processingDim ),
///              dest.TensorStride(),
///              dest.Size( processingDim ),
///              dest.tensor_.Elements()
///        );
///     } while( ++it );
/// ```
///
/// Note that when an image is stripped or reforged, all its iterators are invalidated.
///
/// \see \ref using_iterators, JointImageIterator, GenericImageIterator
template< dip::uint N, typename T = dfloat >
class DIP_NO_EXPORT GenericJointImageIterator {
   public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = Image::CastPixel< T >; ///< The type of the pixel, obtained when dereferencing the iterator
      using difference_type = dip::sint;  ///< The type of distances between iterators
      using reference = value_type;       ///< The type of a reference to a pixel (note dip::Image::CastPixel references a value in the image)
      using pointer = value_type*;        ///< The type of a pointer to a pixel

      /// Default constructor yields an invalid iterator that cannot be dereferenced, and is equivalent to an end iterator
      GenericJointImageIterator() : procDim_( std::numeric_limits< dip::uint >::max() ), atEnd_( true ) {
         origins_.fill( nullptr );
         offsets_.fill( 0 );
      }
      /// To construct a useful iterator, provide two images, and optionally a processing dimension
      explicit GenericJointImageIterator( ImageConstRefArray const& images, dip::uint procDim = std::numeric_limits< dip::uint >::max() ):
            procDim_( procDim ), atEnd_( false ) {
         DIP_THROW_IF( images.size() != N, E::ARRAY_ILLEGAL_SIZE );
         Image const& img0 = images[ 0 ].get();
         DIP_THROW_IF( !img0.IsForged(), E::IMAGE_NOT_FORGED );
         coords_.resize( img0.Dimensionality(), 0 );
         sizes_ = img0.Sizes();
         origins_[ 0 ] = img0.Origin();
         dataTypes_[ 0 ] = img0.DataType();
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
               dataTypes_[ ii ] = imgI.DataType();
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
      }

      /// Swap
      template< typename S >
      void swap( GenericJointImageIterator< N, S >& other ) {
         using std::swap;
         swap( origins_, other.origins_ );
         swap( sizes_, other.sizes_ );
         swap( stridess_, other.stridess_ );
         swap( tensorElementss_, other.tensorElementss_ );
         swap( tensorStrides_, other.tensorStrides_ );
         swap( offsets_, other.offsets_ );
         swap( coords_, other.coords_ );
         swap( procDim_, other.procDim_ );
         swap( dataTypes_, other.dataTypes_ );
         swap( sizeOf_, other.sizeOf_ );
         swap( atEnd_, other.atEnd_ );
      }

      /// Index into image tensor for image `I`
      template< dip::uint I >
      Image::CastSample< T > Sample( dip::uint index ) const {
         return Image::CastSample< T >( Pointer< I >( index ), dataTypes_[ I ] );
      }
      /// Index into image tensor for image 0.
      Image::CastSample< T > InSample( dip::uint index ) const { return Sample< 0 >( index ); }
      /// Index into image tensor for image 1.
      Image::CastSample< T > OutSample( dip::uint index ) const { return Sample< 1 >( index ); }
      /// Get first tensor element for image `I`.
      template< dip::uint I >
      Image::CastSample< T > Sample() const {
         return Image::CastSample< T >( Pointer< I >(), dataTypes_[ I ] );
      }
      /// Get pixel for image 0.
      value_type In() const { return Pixel< 0 >(); }
      /// Get pixel for image 1.
      value_type Out() const { return Pixel< 1 >(); }
      /// Get pixel for image `I`.
      template< dip::uint I >
      value_type Pixel() const {
         return value_type( Pointer< I >(), dataTypes_[ I ], Tensor( tensorElementss_[ I ] ), tensorStrides_[ I ] );
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
      GenericJointImageIterator operator++( int ) {
         GenericJointImageIterator tmp( *this );
         operator++();
         return tmp;
      }

      /// Get an iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      typename value_type::Iterator begin() const {
         return typename value_type::Iterator( Pointer< I >(), dataTypes_[ I ], tensorStrides_[ I ] );
      }
      /// Get an end iterator over the tensor for the current pixel of image `I`
      template< dip::uint I >
      typename value_type::Iterator end() const {
         return typename value_type::Iterator( Pointer< I >(), dataTypes_[ I ], tensorStrides_[ I ], tensorElementss_[ I ] );
      }
      /// Get an iterator over the current line of image `I`
      template< dip::uint I, typename S = T >
      LineIterator< S > GetLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return LineIterator< S >( Pointer< I >(), sizes_[ procDim_ ], stridess_[ I ][ procDim_ ],
                                   tensorElementss_[ I ], tensorStrides_[ I ] );
      }
      /// Get a const iterator over the current line of image `I`
      template< dip::uint I, typename S = T >
      ConstLineIterator< S > GetConstLineIterator() const {
         DIP_THROW_IF( !HasProcessingDimension(), "Cannot get a line iterator if there's no valid processing dimension" );
         return ConstLineIterator< S >( Pointer< I >(), sizes_[ procDim_ ], stridess_[ I ][ procDim_ ],
                                        tensorElementss_[ I ], tensorStrides_[ I ] );
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
      GenericJointImageIterator& SetCoordinates( UnsignedArray coords ) {
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

      /// Index into image tensor for image `I`
      template< dip::uint I >
      void* Pointer( dip::uint index ) const {
         DIP_ASSERT( origins_[ I ] );
         DIP_ASSERT( !atEnd_ );
         return static_cast< uint8* >( origins_[ I ] ) + ( offsets_[ I ] + static_cast< dip::sint >( index ) * tensorStrides_[ I ] ) * sizeOf_[ I ];
      }
      /// Index into image tensor for image 0.
      void* InPointer( dip::uint index ) const { return Pointer< 0 >( index ); }
      /// Index into image tensor for image 1.
      void* OutPointer( dip::uint index ) const { return Pointer< 1 >( index ); }
      /// Return the current pointer for image `I`
      template< dip::uint I >
      void* Pointer() const {
         DIP_ASSERT( origins_[ I ] );
         DIP_ASSERT( !atEnd_ );
         return static_cast< uint8* >( origins_[ I ] ) + offsets_[ I ] * sizeOf_[ I ];
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
         return Image::Index( coords_, sizes_ );
      }

      /// True if the processing dimension is set
      bool HasProcessingDimension() const {
         return origins_[ 0 ] ? ( procDim_ < sizes_.size() ) : false;
      }
      /// \brief Return the processing dimension, the direction of the lines over which the iterator iterates.
      ///
      /// If the return value is larger or equal to the dimensionality (i.e. not one of the image dimensions), then
      /// there is no processing dimension.
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first pixel in the image (as it was when first created)
      GenericJointImageIterator& Reset() {
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
      GenericJointImageIterator& Optimize( dip::uint n = 0 ) {
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
      GenericJointImageIterator& OptimizeAndFlatten( dip::uint n = 0 ) {
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
         coords_.resize( sizes_.size());
         return *this;
      }

   private:
      static_assert( N > 1, "GenericJointImageIterator needs at least two type template arguments" );
      std::array< void*, N > origins_;
      UnsignedArray sizes_;
      std::array< IntegerArray, N > stridess_;
      std::array< dip::uint, N > tensorElementss_;
      std::array< dip::sint, N > tensorStrides_;
      std::array< dip::sint, N > offsets_;
      UnsignedArray coords_;
      dip::uint procDim_;
      std::array< DataType, N > dataTypes_;
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
/// \see \ref using_iterators, ImageTensorIterator, ImageIterator, JointImageIterator, GenericImageIterator, GenericJointImageIterator
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
      ImageSliceIterator& SetCoordinate( dip::uint coord ) {
         DIP_THROW_IF( coord >= size_, E::INDEX_OUT_OF_RANGE );
         coord_ = coord;
         return *this;
      }

      /// Return the processing dimension, the direction over which the iterator iterates
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Reset the iterator to the first image plane (as it was when the iterator first was created)
      ImageSliceIterator& Reset() {
         coord_ = 0;
         return *this;
      }

      /// \brief Set the iterator to index `plane`. If `plane` is outside the image domain, the iterator is still valid,
      /// but should not be dereferenced.
      ImageSliceIterator& Set( dip::uint plane ) {
         coord_ = plane;
         return *this;
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
/// \see \ref using_iterators, ImageSliceIterator, ImageIterator, JointImageIterator, GenericImageIterator, GenericJointImageIterator
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
   DOCTEST_REQUIRE( img.DataType() == dip::DT_UINT16 );
   {
      dip::ImageIterator< dip::uint16 > it( img );
      dip::uint16 counter = 0;
      do {
         *it = static_cast< dip::uint16 >( counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension());
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( !it.HasProcessingDimension());
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   img.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::ImageIterator< dip::uint16 > it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::ImageIterator< dip::uint16 > it( img, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3*2 } );
   }

   img.StandardizeStrides(); // returns strides to normal.
   {
      dip::GenericImageIterator< dip::sint32 > it( img );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( *it == counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1 } );
   }
   {
      dip::GenericImageIterator<> it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2 * 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   img.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::GenericImageIterator<> it( img, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::GenericImageIterator<> it( img, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::GenericImageIterator<> it( img, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides() == dip::IntegerArray{ 1, 3*2 } );
   }

   dip::Image img2{ dip::UnsignedArray{ 3, 4 }, 3, dip::DT_SINT32 };
   DOCTEST_REQUIRE( img2.DataType() == dip::DT_SINT32 );
   {
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

DOCTEST_TEST_CASE("[DIPlib] testing JointImageIterator and GenericJointImageIterator") {
   dip::Image imgA{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_UINT16 };
   dip::Image imgB{ dip::UnsignedArray{ 3, 2, 4 }, 1, dip::DT_SINT8 };
   DOCTEST_REQUIRE( imgA.DataType() == dip::DT_UINT16 );
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB } );
      dip::uint16 counter = 0;
      do {
         it.Sample< 0 >() = static_cast< dip::uint16 >( counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension());
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( !it.HasProcessingDimension());
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   imgA.Rotation90( 1 ); // rotates over dims 0 and 1.
   imgB.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::JointImageIterator< dip::uint16, dip::sint8 > it( { imgA, imgB }, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3*2 } );
   }

   imgA.StandardizeStrides(); // returns strides to normal.
   imgB.StandardizeStrides(); // returns strides to normal.
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB } );
      dip::sint32 counter = 0;
      do {
         DOCTEST_CHECK( (dip::sint32)(it.Sample< 0 >()) == counter++ );
      } while( ++it );
      DOCTEST_CHECK( !it.HasProcessingDimension() );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3 * 2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension());
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2 * 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   imgA.Rotation90( 1 ); // rotates over dims 0 and 1.
   imgB.Rotation90( 1 ); // rotates over dims 0 and 1.
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 0 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3, 3*2 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 1 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 0 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3, 2*4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3 } );
   }
   {
      dip::GenericJointImageIterator< 2 > it( { imgA, imgB }, 2 );
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 2 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 2, 3, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ -3, 1, 3*2 } );
      it.OptimizeAndFlatten();
      DOCTEST_CHECK( it.HasProcessingDimension() );
      DOCTEST_CHECK( it.ProcessingDimension() == 1 );
      DOCTEST_CHECK( it.Sizes() == dip::UnsignedArray{ 3*2, 4 } );
      DOCTEST_CHECK( it.Strides< 0 >() == dip::IntegerArray{ 1, 3*2 } );
      DOCTEST_CHECK( it.Strides< 1 >() == dip::IntegerArray{ 1, 3*2 } );
   }
}

#endif // DIP__ENABLE_DOCTEST

#endif // DIP_GENERIC_ITERATORS_H
