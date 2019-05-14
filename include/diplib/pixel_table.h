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
/// \brief A pixel table represents the support of a filter of arbitrary shape and number of dimensions.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


class DIP_NO_EXPORT PixelTable; // forward declaration, it's defined a little lower down.

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
/// But note that the iterator is not as efficient as a manual double-loop over runs and
/// pixels within each run. Even more efficient is to first extract an array with offsets
/// using the `Offsets` method. The difference between these methods is only relevant
/// when iterating over the pixel table for each pixel in an image, as the small difference
/// accumulates.
///
/// ```cpp
///     PixelTableOffsets pt = ...;
///
///     for( auto offset: pt ) { ... }          // easiest, least efficient
///
///     for( auto const& run : pt.Runs() ) {
///        dip::sint offset = run.offset;
///        for( dip::uint ii = 0; ii < run.length; ++ii, offset += pt.Stride() ) {
///           ...
///        }
///     }                                       // more efficient
///
///     auto offsets = pt.Offsets();
///     for( auto offset: offsets ) { ... }     // most efficient (when iterating many times)
/// ```
///
/// \see dip::PixelTable, dip::Framework::Full, dip::ImageIterator
class DIP_NO_EXPORT PixelTableOffsets {
   public:

      /// The pixel table is formed of pixel runs, represented by this structure
      struct DIP_NO_EXPORT PixelRun {
         dip::sint offset;          ///< the offset of the first pixel in a run, w.r.t. the origin
         dip::uint length;          ///< the length of the run
      };

      class DIP_NO_EXPORT iterator;

      /// A default-constructed pixel table is kinda useless
      PixelTableOffsets() = default;

      /// A pixel table with offsets is constructed from a `dip::PixelTable` and a `dip::Image`.
      DIP_EXPORT PixelTableOffsets( PixelTable const& pt, Image const& image );

      /// Returns the vector of runs
      std::vector< PixelRun > const& Runs() const { return runs_; }

      /// Returns the dimensionality of the neighborhood
      dip::uint Dimensionality() const { return sizes_.size(); }

      /// Returns the size of the bounding box of the neighborhood
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Returns the coordinates of the top-left corner of the bounding box w.r.t. the origin of the neighborhood
      IntegerArray const& Origin() const { return origin_; }

      /// Returns the number of pixels in the neighborhood
      dip::uint NumberOfPixels() const { return nPixels_; }

      /// Returns the processing dimension, the dimension along which pixel runs are laid out
      dip::uint ProcessingDimension() const { return procDim_; }

      /// Returns the stride along the processing dimension used by the iterator
      dip::sint Stride() const { return stride_; }

      /// A const iterator to the first pixel in the neighborhood
      iterator begin() const;

      /// A const iterator to one past the last pixel in the neighborhood
      iterator end() const;

      /// Tests if there are weights associated to each pixel in the neighborhood.
      bool HasWeights() const { return !weights_.empty(); }

      /// Returns a const reference to the weights array.
      std::vector< dfloat > const& Weights() const { return weights_; }

      /// Returns an array with the offsets.
      std::vector< dip::sint > Offsets() const;

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
/// It is simple to create a pixel table for unit circles (spheres) in different norms, and for
/// straight lines. And any other shape can be created through a binary image.
///
/// The processing dimension defines the dimension along which the pixel runs are taken.
/// By default it is dimension 0, but it could be beneficial to set it to the dimension
/// in which there would be fewer runs.
///
/// Two ways can be used to walk through the pixel table:
///
/// 1.  `dip::PixelTable::Runs` returns a `std::vector` with all the runs, which are encoded
///     by the coordinates of the first pixel and a run length. Visiting each run is an efficient
///     way to process the whole neighborhood. For example, the filter `dip::Uniform`, which
///     computes the average over all pixels within the neighborhood, only needs to subtract
///     the pixels on the start of each run, shift the neighborhood by one pixel, then add
///     the pixels on the end of each run. See the example in the section \ref iterate_neighborhood.
///
/// 2.  `dip::PixelTable::begin` returns an iterator to the first pixel in the table,
///     incrementing the iterator successively visits each of the pixels in the run. Dereferencing
///     this iterator yields the offset to a neighbor pixel. This makes for a simple way to
///     visit every single pixel within the neighborhood.
///
/// The pixel table can optionally contain a weight for each pixel. These can be accessed
/// only by retrieving the array containing all weights. This array is meant to be used
/// by taking its `begin` iterator, and using that iterator in conjunction with the pixel
/// table's iterator. Taken together, they provide both the location and the weight of each
/// pixel in the neighborhood. For example, modified from from the function `dip::GeneralConvolution`:
///
/// ```cpp
///     sfloat* in = ...  // pointer to the current pixel in the input image
///     sfloat* out = ... // pointer to the current pixel in the output image
///     sfloat sum = 0;
///     auto ito = pixelTable.begin(); // pixelTable is our neighborhood
///     auto itw = pixelTable.Weights().begin();
///     while( !ito.IsAtEnd() ) {
///        sum += in[ *ito ] * static_cast< sfloat >( *itw );
///        ++ito;
///        ++itw;
///     }
///     *out = sum;
/// ```
///
/// \see dip::PixelTableOffsets, dip::Kernel, dip::NeighborList, dip::StructuringElement,
/// dip::Framework::Full, dip::ImageIterator
class DIP_NO_EXPORT PixelTable {
   public:

      /// The pixel table is formed of pixel runs, represented by this structure
      struct DIP_NO_EXPORT PixelRun {
         IntegerArray coordinates;  ///< The coordinates of the first pixel in a run, w.r.t. the origin.
         dip::uint length;          ///< The length of the run, expected to always be larger than 0.
         PixelRun( IntegerArray coordinates, dip::uint length ) : coordinates( std::move( coordinates )), length( length ) {}
      };

      class DIP_NO_EXPORT iterator;

      /// A default-constructed pixel table is kinda useless
      PixelTable() = default;

      /// \brief Construct a pixel table for default filter shapes.
      ///
      /// The known default `shape`s are `"rectangular"`, `"elliptic"`, and `"diamond"`,
      /// which correspond to a unit circle in the \f$L^\infty\f$, \f$L^2\f$ and \f$L^1\f$ norms; and
      /// `"line"`, which is a single-pixel thick line.
      ///
      /// The `size` array determines the size and dimensionality. For unit circles, it gives the diameter of the
      /// neighborhood (not the radius!); the neighborhood contains all pixels at a distance equal or smaller than
      /// half the diameter from the origin. This means that non-integer sizes can be meaningful.
      /// The exception is for the `"rectangular"` shape, where the sizes are rounded down to the nearest integer,
      /// yielding rectangle sides that are either even or odd in length. For even sizes, one can imagine that the
      /// origin is shifted by half a pixel to accommodate the requested size (though the origin is set to the pixel
      /// that is right of the center). For the `"diamond"` and `"elliptic"` shapes, the bounding box always has odd
      /// sizes, and the origin is always centered on one pixel. To accomplish the same for the "rectangular" shape,
      /// simply round the sizes array to an odd integer:
      ///
      /// ```cpp
      ///     size[ ii ] = std::floor( size[ ii ] / 2 ) * 2 + 1
      /// ```
      ///
      /// For the line, the `size` array gives the size of the bounding box (rounded to the nearest integer), as
      /// well as the direction of the line. A negative value for one dimension means that the line runs from
      /// high to low along that dimension. The line will always run from one corner of the bounding box to the
      /// opposite corner, and run through the origin.
      ///
      /// `procDim` indicates the processing dimension.
      DIP_EXPORT PixelTable( String const& shape, FloatArray size, dip::uint procDim = 0 );

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
      DIP_EXPORT explicit PixelTable( Image const& mask, IntegerArray const& origin = {}, dip::uint procDim = 0 );

      /// Returns the vector of runs
      std::vector< PixelRun > const& Runs() const { return runs_; }

      /// Returns the dimensionality of the neighborhood
      dip::uint Dimensionality() const { return sizes_.size(); }

      /// Returns the size of the bounding box of the neighborhood
      UnsignedArray const& Sizes() const { return sizes_; }

      /// Returns the coordinates of the top-left corner of the bounding box w.r.t. the origin.
      IntegerArray const& Origin() const { return origin_; }

      /// \brief Returns the size of the boundary extension along each dimension that is necessary to accommodate the
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
      void ShiftOrigin( IntegerArray const& shift ) {
         dip::uint nDims = origin_.size();
         DIP_THROW_IF( shift.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
         origin_ -= shift;
         for( auto& run : runs_ ) {
            run.coordinates -= shift;
         }
      }

      /// \brief Shifts the origin of neighborhood by one pixel to the left for even-sized dimensions.
      /// This is useful for neighborhoods with their origin in the default location, that have been mirrored.
      void MirrorOrigin() {
         IntegerArray offset( sizes_.size(), 0 );
         for( dip::uint ii = 0; ii < sizes_.size(); ++ii ) {
            if( !( sizes_[ ii ] & 1 ) ) {
               offset[ ii ] = -1;
            }
         }
         ShiftOrigin( offset );
      }

      /// \brief Mirrors the neighborhood.
      void Mirror() {
         dip::uint nDims = sizes_.size();
         IntegerArray origin( nDims, std::numeric_limits< dip::sint >::max() );
         for( auto& run : runs_ ) {
            run.coordinates[ procDim_ ] += static_cast< dip::sint >( run.length ) - 1; // coordinates now points at end of run
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               run.coordinates[ ii ] = -run.coordinates[ ii ]; // mirror coordinates, it points at beginning of run again
               origin[ ii ] = std::min( origin[ ii ], run.coordinates[ ii ] );
            }
         }
         origin_ = origin;
      }

      /// Returns the number of pixels in the neighborhood
      dip::uint NumberOfPixels() const { return nPixels_; }

      /// Returns the processing dimension, the dimension along which pixel runs are laid out
      dip::uint ProcessingDimension() const { return procDim_; }

      /// A const iterator to the first pixel in the neighborhood
      DIP_NO_EXPORT iterator begin() const;

      /// A const iterator to one past the last pixel in the neighborhood
      DIP_NO_EXPORT iterator end() const;

      /// \brief Creates a binary image representing the neighborhood, or a `dfloat` one if
      /// there are weights associated.
      Image AsImage() const {
         Image out;
         AsImage( out );
         return out;
      }

      /// Same as previous overload, but writing into the given image.
      DIP_EXPORT void AsImage( Image& out ) const;

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
      DIP_EXPORT void AddWeights( Image const& image );

      /// \brief Add weights to each pixel in the neighborhood, using the Euclidean distance to the origin
      /// as the weight. This is useful for algorithms that need to, for example, sort the pixels in the
      /// neighborhood by distance to the origin.
      DIP_EXPORT void AddDistanceToOriginAsWeights();

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
class DIP_NO_EXPORT PixelTable::iterator {
   public:
      using iterator_category = std::forward_iterator_tag; ///< %Iterator category
      using value_type = IntegerArray;       ///< The value obtained by dereferencing are coordinates
      using reference = IntegerArray const&; ///< The type of a reference

      /// Default constructor yields an invalid iterator that cannot be dereferenced
      iterator() = default;

      /// Constructs an iterator to the first pixel in the neighborhood
      explicit iterator( PixelTable const& pt ) {
         DIP_THROW_IF( pt.nPixels_ == 0, "Pixel Table is empty" );
         run_ = pt.runs_.begin();
         end_ = pt.runs_.end();
         procDim_ = pt.procDim_;
         coordinates_ = run_->coordinates;
         // index_ set properly by default
      }

      /// Constructs an end iterator
      static iterator end( PixelTable const& pt ) {
         iterator out;
         out.end_ = out.run_ = pt.runs_.end();
         // index_ set properly by default, procDim_ and coordinates_ we don't care about
         return out;
      }

      /// Swap
      void swap( iterator& other ) {
         using std::swap;
         swap( run_, other.run_ );
         swap( end_, other.end_ );
         swap( index_, other.index_ );
         swap( procDim_, other.procDim_ );
         swap( coordinates_, other.coordinates_ );
      }

      /// Dereference
      reference operator*() const { return coordinates_; }

      /// Pre-increment
      iterator& operator++() {
         ++index_;
         if( index_ < run_->length ) {
            ++coordinates_[ procDim_ ];
         } else {
            index_ = 0;
            ++run_;
            if( run_ != end_ ) {
               coordinates_ = run_->coordinates;
            }
         }
         return *this;
      }

      /// Post-increment
      iterator operator++( int ) {
         iterator tmp( *this );
         operator++();
         return tmp;
      }

      /// \brief Equality comparison, is true if the two iterators reference the same pixel in the same pixel table.
      bool operator==( iterator const& other ) const {
         return ( run_ == other.run_ ) && ( index_ == other.index_ );
      }

      /// Inequality comparison
      bool operator!=( iterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return run_ == end_; }

      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !IsAtEnd(); }

   private:
      std::vector< PixelTable::PixelRun >::const_iterator run_;
      std::vector< PixelTable::PixelRun >::const_iterator end_;
      dip::uint index_ = 0;      // which pixel on the run we're currently pointing at
      dip::uint procDim_ = 0;    // the direction of the runs
      value_type coordinates_;   // the coordinates of the pixel
};

inline void swap( PixelTable::iterator& v1, PixelTable::iterator& v2 ) {
   v1.swap( v2 );
}

/// \cond

inline PixelTable::iterator PixelTable::begin() const {
   return iterator( *this );
}

inline PixelTable::iterator PixelTable::end() const {
   return iterator::end( *this );
}

/// \endcond


/// \brief An iterator that visits each of the neighborhood's pixels in turn.
///
/// Dereferencing the iterator returns an offset.
/// Satisfies the requirements for ForwardIterator.
class DIP_NO_EXPORT PixelTableOffsets::iterator {
   public:
      using iterator_category = std::forward_iterator_tag; ///< %Iterator category
      using value_type = dip::sint;       ///< The value obtained by dereferencing is an offset
      using reference = dip::sint;        ///< The type of a reference, but we don't return by reference, it's just as easy to copy

      /// Default constructor yields an invalid iterator that cannot be dereferenced
      iterator() = default;

      /// Constructs an iterator to the first pixel in the neighborhood
      explicit iterator( PixelTableOffsets const& pt ) {
         DIP_THROW_IF( pt.nPixels_ == 0, "Pixel Table is empty" );
         run_ = pt.runs_.begin();
         end_ = pt.runs_.end();
         stride_ = pt.stride_;
         // index_ set properly by default
      }

      /// Constructs an end iterator
      static iterator end( PixelTableOffsets const& pt ) {
         iterator out;
         out.end_ = out.run_ = pt.runs_.end();
         // index_ set properly by default, stride_ we don't care about
         return out;
      }

      /// Swap
      void swap( iterator& other ) {
         using std::swap;
         swap( run_, other.run_ );
         swap( end_, other.end_ );
         swap( index_, other.index_ );
         swap( stride_, other.stride_ );
      }

      /// Dereference
      reference operator*() const { return Offset(); }

      /// Get offset, identical to dereferencing
      reference Offset() const { return run_->offset + static_cast< dip::sint >( index_ ) * stride_; }

      /// Get index within run
      dip::uint Index() const { return index_; }

      /// Pre-increment
      iterator& operator++() {
         ++index_;
         if( index_ == run_->length ) {
            index_ = 0;
            ++run_;
         }
         return *this;
      }

      /// Post-increment
      iterator operator++( int ) {
         iterator tmp( *this );
         operator++();
         return tmp;
      }

      /// \brief Equality comparison, is true if the two iterators reference the same pixel in the same pixel table,
      /// even if they use the strides of different images.
      bool operator==( iterator const& other ) const {
         return ( run_ == other.run_ ) && ( index_ == other.index_ );
      }

      /// Inequality comparison
      bool operator!=( iterator const& other ) const {
         return !operator==( other );
      }

      /// Test to see if the iterator reached past the last pixel
      bool IsAtEnd() const { return run_ == end_; }

      /// Test to see if the iterator is still pointing at a pixel
      explicit operator bool() const { return !IsAtEnd(); }

   private:
      std::vector< PixelTableOffsets::PixelRun >::const_iterator run_;
      std::vector< PixelTableOffsets::PixelRun >::const_iterator end_;
      dip::uint index_ = 0;      // which pixel on the run we're currently pointing at
      dip::sint stride_ = 1;     // image stride along the run direction
};

inline void swap( PixelTableOffsets::iterator& v1, PixelTableOffsets::iterator& v2 ) {
   v1.swap( v2 );
}

/// \cond

inline PixelTableOffsets::iterator PixelTableOffsets::begin() const {
   return iterator( *this );
}

inline PixelTableOffsets::iterator PixelTableOffsets::end() const {
   return iterator::end( *this );
}

inline std::vector< dip::sint > PixelTableOffsets::Offsets() const {
   std::vector< dip::sint > out( nPixels_ );
   auto dit = out.begin();
   for( auto iit = begin(); !iit.IsAtEnd(); ++iit, ++dit ) {
      *dit = *iit;
   }
   return out;
}

/// \endcond

/// \}

} // namespace dip

#endif // DIP_PIXEL_TABLE_H
