/*
 * DIPlib 3.0
 * This file contains declarations for the pixel table functionality.
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

#ifndef DIP_PIXEL_TABLE_H
#define DIP_PIXEL_TABLE_H

#include "diplib.h"


/// \file
/// \brief A pixel table is a convenient way to simplify neighborhoods of arbitrary
/// dimensionality. Such a neighborhood represents the support of a filter of
/// arbitrary shape and number of dimensions.


namespace dip {


/// \addtogroup infrastructure
/// \{


class PixelTable; // forward declaration, it's defined a little lower down.

/// \brief Represents an arbitrarily-shaped neighborhood (filter support)
/// in an arbitrary number of dimensions.
///
/// A `%PixelTableOffsets` object is created from a `dip::PixelTable` through
/// its `dip::PixelTable::Prepare` method. The object is identical to its parent, but
/// instead of coordinates it contains offsets. It is ready to be applied to a specific
/// image. It can only be used on other images that have the exact same strides as the
/// image it was prepared for.
///
/// Offsets cannot be used to test for the neighbor to be within the image domain, so this
/// object is meant to be used with images in which the boundary has been extended through
/// `dip::ExtendImage`, or where the pixels being processed are away from the image edges.
///
/// Its iterator dereferences to an offset rather than coordinates.
///
/// \see dip::PixelTable, dip::NeighborList, dip::Framework::Full, dip::ImageIterator
class PixelTableOffsets {
   public:

      /// The pixel table is formed of pixel runs, represented by this strucuture
      struct PixelRun {
         dip::sint offset;          ///< the offset of the first pixel in a run, w.r.t. the origin
         dip::uint length;          ///< the length of the run
      };

      class iterator;

      /// A default-constructed pixel table is kinda useless
      PixelTableOffsets() {};

      /// A pixel table with offsets is constructed from a `dip::PixelTable` and a `dip::Image`.
      PixelTableOffsets( PixelTable const& pt, Image const& image );

      /// Returns the vector of runs
      std::vector< PixelRun > const& Runs() const { return runs_; }

      /// Returns the dimensionality of the neighborhood
      dip::uint Dimensionality() const { return sizes_.size(); }

      /// Returns the size of the bounding box of the neighborhood
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Returns the origin of the neighborhood w.r.t. the top-left corner of the bounding box
      IntegerArray const& Origin() const { return origin_; }

      /// Returns the number of pixels in the neighborhood
      dip::uint NumberOfPixels() const { return nPixels_; }

      /// Returns the processing dimension, the dimension along which pixel runs are laid out
      dip::uint ProcessingDimension() const { return procDim_; }

      /// A const iterator to the first pixel in the neighborhood
      iterator begin() const;

      /// A const iterator to one past the last pixel in the neighborhood
      iterator end() const;

      /// Tests if there are weights associated to each pixel in the neighborhood.
      bool HasWeights() const { return !weights_.empty(); }

      /// Returns a const reference to the weights array.
      std::vector< dfloat > const& Weights() const { return weights_; }

   private:
      std::vector< PixelRun > runs_;
      std::vector< dfloat > weights_;
      UnsignedArray sizes_;      // the size of the bounding box
      IntegerArray origin_;      // the coordinates of the origin w.r.t. the top-left corner of the bounding box
      dip::uint nPixels_ = 0;    // the total number of pixels in the pixel table
      dip::uint procDim_ = 0;    // the dimension along which the runs go
      dip::sint stride_ = 0;     // the stride of the image along the processing dimension
};

/// \brief Represents an arbitrarily-shaped neighborhood (filter support)
/// in an arbitrary number of dimensions.
///
/// The `%PixelTable` is an array of pixel runs, where each run is encoded by start coordinates
/// and a length (number of pixels). The runs all go along the same dimension, given by
/// `dip::PixelTable::ProcessingDimension`.
///
/// It is simple to create a pixel table for
/// unit circles (spheres) in different norms, and any other shape can be created through
/// a binary image.
///
/// The processing dimension defines the dimension along which the pixel runs are taken.
/// By default it is dimension 0, but it could be beneficial to set it to the dimension
/// in which there would be fewer runs.
///
/// Two ways can be used to walk through the pixel table:
/// 1.  `dip::PixelTable::Runs` returns a vector with all the runs, which are encoded
///     by the coordinates of the first pixel and a run length.
/// 2.  `dip::PixelTable::begin` returns an iterator to the first pixel in the table,
///     incrementing the iterator successively visits each of the pixels in the run.
///
/// The pixel table can optionally contain a weight for each pixel. These can be accessed
/// only by retrieving the array containing all weights. This array is meant to be used
/// by taking its `begin` iterator, and using that iterator in conjunction with the pixel
/// table's iterator. Taken together, they provide both the location and the weight of each
/// pixel in the neighborhood.
///
/// TODO: Add an example somewhere.
///
/// \see dip::PixelTableOffsets, dip::NeighborList, dip::Framework::Full, dip::ImageIterator
class PixelTable {
   public:

      /// The pixel table is formed of pixel runs, represented by this strucuture
      struct PixelRun {
         IntegerArray coordinates;  ///< the coordinates of the first pixel in a run, w.r.t. the origin
         dip::uint length;          ///< the length of the run
         PixelRun( IntegerArray const& coordinates, dip::uint length ) : coordinates( coordinates ), length( length ) {}
      };

      class iterator;

      /// A default-constructed pixel table is kinda useless
      PixelTable() {};

      /// \brief Construct a pixel table for default filter shapes.
      ///
      /// The known default `shape`s are "rectangular", "elliptic", and "diamond",
      /// which correspond to a unit circle in the L<sub>infinity</sub> norm, the L<sub>2</sub> norm,
      /// and the L<sub>1</sub> norm.
      ///
      /// The `size` array determines the size and dimensionality. It gives the diameter of the neighborhood (not
      /// the radius!); the neighborhood contains all pixels at a distance equal or smaller than half the diameter
      /// from the origin. This means that not only integer sizes are meaningful.
      /// The exception is for the "rectangular" shape, where the sizes are rounded down to the nearest integer,
      /// yielding rectangle sides that are either even or odd in length. For even sizes, one can imagine that the
      /// origin is shifted by half a pixel to accommodate the requested size (thought the origin is set to the pixel
      /// that is right of the center). For the "diamond" and "elliptic" shapes, the bounding box always has odd
      /// sizes, and the origin is always centered on one pixel. To accomplish the same for the "rectangular" shape,
      /// simply round the sizes array to an odd integer: `size[ ii ] = std::floor( size[ ii ] / 2 ) * 2 + 1`.
      ///
      /// `procDim` indicates the processing dimension.
      PixelTable( String const& shape, FloatArray size, dip::uint procDim = 0 );

      /// \brief Construct a pixel table for an arbitrary shape defined by a binary image.
      ///
      /// Set pixels in `mask` indicate pixels that belong to the neighborhood.
      ///
      /// `origin` gives the coordinates of the pixel in the image that will be placed at the origin
      /// (i.e. have coordinates {0,0,0}. If `origin` is an empty array, the origin is set to the middle
      /// pixel, as given by `mask.Sizes() / 2`. That is, for odd-sized dimensions, the origin is the
      /// exact middle pixel, and for even-sized dimensions the origin is the pixel to the right of the
      /// exact middle.
      ///
      /// `procDim` indicates the processing dimension.
      PixelTable( Image const& mask, IntegerArray const& origin = {}, dip::uint procDim = 0 );

      /// Returns the vector of runs
      std::vector< PixelRun > const& Runs() const { return runs_; }

      /// Returns the dimensionality of the neighborhood
      dip::uint Dimensionality() const { return sizes_.size(); }

      /// Returns the size of the bounding box of the neighborhood
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Returns the origin of the neighborhood w.r.t. the top-left corner of the bounding box
      IntegerArray const& Origin() const { return origin_; }

      /// Returns the size of the boundary extension along each dimension that is necessary to accommodate the
      /// neighborhood on the edge pixels of the image
      UnsignedArray Boundary() const {
         dip::uint nDims = sizes_.size();
         UnsignedArray boundary( nDims );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            boundary[ ii ] = dip::uint( std::max( std::abs( origin_[ ii ] ), std::abs( origin_[ ii ] + dip::sint( sizes_[ ii ] ) - 1 )));
         }
         return boundary;
      }

      /// Shifts the origin of the neighborhood by the given amount
      void ShiftOrigin( IntegerArray const& shift );

      /// Returns the number of pixels in the neighborhood
      dip::uint NumberOfPixels() const { return nPixels_; }

      /// Returns the processing dimension, the dimension along which pixel runs are laid out
      dip::uint ProcessingDimension() const { return procDim_; }

      /// A const iterator to the first pixel in the neighborhood
      iterator begin() const;

      /// A const iterator to one past the last pixel in the neighborhood
      iterator end() const;

      /// \brief Creates a binary image representing the neighborhood, or a `dfloat` one if
      /// there are weights associated.
      Image AsImage() const {
         Image out;
         AsImage( out );
         return out;
      }

      /// Same as previous overload, but writing into the given image.
      void AsImage( Image& out ) const;

      /// \brief Prepare the pixel table to be applied to a specific image.
      ///
      /// The resulting object is identical to `this`,
      /// but has knowledge of the image's strides and thus directly gives offsets rather than coordinates to
      /// the neighbors.
      PixelTableOffsets Prepare( Image const& image ) const {
         return PixelTableOffsets( *this, image );
      }

      /// \brief Add weights to each pixel in the neighborhood, taken from an image. The image must be of the same
      /// sizes as the `%PixelTable`'s bounding box (i.e. the image used to construct the pixel table), and of a
      /// real type (i.e. integer or float).
      // TODO: Do we need to support complex weights? Tensor weights?
      void AddWeights( Image const& image );

      /// \brief Add weights to each pixel in the neighborhood, using the Euclidean distance to the origin
      /// as the weight. This is useful for algorithms that need to, for example, sort the pixels in the
      /// neighborhood by distance to the origin.
      void AddDistanceToOriginAsWeights();

      /// Tests if there are weights associated to each pixel in the neighborhood.
      bool HasWeights() const { return !weights_.empty(); }

      /// Returns a const reference to the weights array.
      std::vector< dfloat > const& Weights() const { return weights_; }

   private:
      std::vector< PixelRun > runs_;
      std::vector< dfloat > weights_;
      UnsignedArray sizes_;      // the size of the bounding box
      IntegerArray origin_;      // the coordinates of the top-left corner of the bounding box
      dip::uint nPixels_ = 0;    // the total number of pixels in the pixel table
      dip::uint procDim_ = 0;    // the dimension along which the runs go
};

/// \brief An iterator that visits each of the neighborhood's pixels in turn.
///
/// Dereferencing the iterator returns the coordinates of the pixel.
/// Satisfies the requirements for ForwardIterator.
class PixelTable::iterator {
   public:

      using iterator_category = std::forward_iterator_tag;
      using value_type = IntegerArray;       ///< The value obtained by dereferencing are coordinates
      using reference = IntegerArray const&; ///< The type of a reference

      /// Default constructor yields an invalid iterator that cannot be dereferenced
      iterator() {}

      /// Constructs an iterator to the first pixel in the neighborhood
      iterator( PixelTable const& pt ) {
         DIP_THROW_IF( pt.nPixels_ == 0, "Pixel Table is empty" );
         pixelTable_ = &pt;
         coordinates_ = pt.runs_[ 0 ].coordinates;
         // run_ and index_ are set properly by default
      }

      /// Constructs an end iterator
      static iterator end( PixelTable const& pt ) {
         iterator out;
         out.pixelTable_ = &pt;
         out.run_ = pt.runs_.size();
         return out;
      }

      /// Swap
      void swap( iterator& other ) {
         using std::swap;
         swap( pixelTable_, other.pixelTable_ );
         swap( run_, other.run_ );
         swap( index_, other.index_ );
         swap( coordinates_, other.coordinates_ );
      }

      /// Dereference
      reference operator*() const { return coordinates_; }

      /// Increment
      iterator& operator++() {
         if( pixelTable_ && ( run_ < pixelTable_->runs_.size() ) ) {
            ++index_;
            if( index_ < pixelTable_->runs_[ run_ ].length ) {
               ++coordinates_[ pixelTable_->procDim_ ];
            } else {
               index_ = 0;
               ++run_;
               if( run_ < pixelTable_->runs_.size() ) {
                  coordinates_ = pixelTable_->runs_[ run_ ].coordinates;
               }
            }
         }
         return * this;
      }

      /// Increment
      iterator operator++( int ) {
         iterator tmp( *this );
         operator++();
         return tmp;
      }

      /// \brief Equality comparison, is true if the two iterators reference the same pixel in the same pixel table.
      bool operator==( iterator const& other ) const {
         return ( pixelTable_ == other.pixelTable_ ) && ( run_ == other.run_ ) && ( index_ == other.index_ );
      }

      /// Inequality comparison
      bool operator!=( iterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return run_ == pixelTable_->runs_.size(); }

      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !IsAtEnd(); }

   private:
      PixelTable const* pixelTable_ = nullptr;
      dip::uint run_ = 0;        // which run we're currently point at
      dip::uint index_ = 0;      // which pixel on the run we're currently pointing at
      value_type coordinates_;   // the coordinates of the pixel
};

inline void swap( PixelTable::iterator& v1, PixelTable::iterator& v2 ) {
   v1.swap( v2 );
}

inline PixelTable::iterator PixelTable::begin() const {
   return iterator( *this );
}

inline PixelTable::iterator PixelTable::end() const {
   return iterator::end( *this );
}

/// \brief An iterator that visits each of the neighborhood's pixels in turn.
///
/// Dereferencing the iterator returns an offset.
/// Satisfies the requirements for ForwardIterator.
class PixelTableOffsets::iterator {
   public:

      using iterator_category = std::forward_iterator_tag;
      using value_type = dip::sint;       ///< The value obtained by dereferencing is an offset
      using reference = dip::sint;        ///< The type of a reference, but we don't return by reference, it's just as easy to copy

      /// Default constructor yields an invalid iterator that cannot be dereferenced
      iterator() {}

      /// Constructs an iterator to the first pixel in the neighborhood
      iterator( PixelTableOffsets const& pt ) {
         DIP_THROW_IF( pt.nPixels_ == 0, "Pixel Table is empty" );
         pixelTable_ = &pt;
         offset_ = pt.runs_[ 0 ].offset;
         // run_ and index_ are set properly by default
      }

      /// Constructs an end iterator
      static iterator end( PixelTableOffsets const& pt ) {
         iterator out;
         out.pixelTable_ = &pt;
         out.run_ = pt.runs_.size();
         return out;
      }

      /// Swap
      void swap( iterator& other ) {
         using std::swap;
         swap( pixelTable_, other.pixelTable_ );
         swap( run_, other.run_ );
         swap( index_, other.index_ );
         swap( offset_, other.offset_ );
      }

      /// Dereference
      reference operator*() const { return offset_; }

      /// Get offset, identical to dereferencing
      reference Offset() const { return offset_; }

      /// Get index within run
      dip::uint Index() const { return index_; }

      /// Increment
      iterator& operator++() {
         if( pixelTable_ && ( run_ < pixelTable_->runs_.size() ) ) {
            ++index_;
            if( index_ < pixelTable_->runs_[ run_ ].length ) {
               offset_ += pixelTable_->stride_;
            } else {
               index_ = 0;
               ++run_;
               if( run_ < pixelTable_->runs_.size() ) {
                  offset_ = pixelTable_->runs_[ run_ ].offset;
               }
            }
         }
         return * this;
      }

      /// Increment
      iterator operator++( int ) {
         iterator tmp( *this );
         operator++();
         return tmp;
      }

      /// \brief Equality comparison, is true if the two iterators reference the same pixel in the same pixel table,
      /// even if they use the strides of different images.
      bool operator==( iterator const& other ) const {
         return ( pixelTable_ == other.pixelTable_ ) && ( run_ == other.run_ ) && ( index_ == other.index_ );
      }

      /// Inequality comparison
      bool operator!=( iterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return run_ == pixelTable_->runs_.size(); }

      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !IsAtEnd(); }

   private:
      PixelTableOffsets const* pixelTable_ = nullptr;
      dip::uint run_ = 0;        // which run we're currently point at
      dip::uint index_ = 0;      // which pixel on the run we're currently pointing at
      value_type offset_;        // the offset of the pixel
};

inline void swap( PixelTableOffsets::iterator& v1, PixelTableOffsets::iterator& v2 ) {
   v1.swap( v2 );
}

inline PixelTableOffsets::iterator PixelTableOffsets::begin() const {
   return iterator( *this );
}

inline PixelTableOffsets::iterator PixelTableOffsets::end() const {
   return iterator::end( *this );
}

/// \}

} // namespace dip

#endif // DIP_PIXEL_TABLE_H
