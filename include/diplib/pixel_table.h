/*
 * DIPlib 3.0
 * This file contains declarations for the pixel table functionality.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_PIXEL_TABLE_H
#define DIP_PIXEL_TABLE_H

#include "diplib.h"


/// \file
/// A pixel table is a convenient way to simplify neighborhoods of arbitrary
/// dimensionality. Such a neighborhood represents the support of a filter of
/// arbitrary shape and number of dimensions.


namespace dip {

/// The `%PixelTable` represents an arbirarily-shaped neighborhood (filter support)
/// in an arbitrary number of dimensions. It is simple to create a pixel table for
/// unit circles (spheres) in different norms, and any other shape can be created through
/// a binary image.
///
/// The processing dimension defines the dimension along which the pixel runs are taken.
/// By default it is dimension 0, but it could be beneficial to set it to the dimension
/// in which there would be fewer runs.
///
/// Two ways can be used to walk through the pixel table:
/// 1.  `dip::PixelTable::Runs()` returns a vector with all the runs, which are encoded
///     by the coordinates of the first pixel and a run length.
/// 2.  `dip::PixelTable::BeginIterator()` returns an iterator to the first pixel in the table,
///     incrementing the iterator successively visits each of the pixels in the run.
///
/// \see dip::NeighborList, dip::Framework::Full, dip::ImageIterator
class PixelTable {
   public:

      // The table is formed of these runs
      struct PixelRun {
         IntegerArray coordinates;  // the coordinates of the first pixel in a run, w.r.t. the origin
         dip::uint length;          // the length of the run
      };

      /// An iterator that visits each of the neighborhood's pixels in turn. Dereferencing
      /// the iterator returns an offset. It is possible also to query the coordinates of the
      /// pixel. Satisfies the requirements for ForwardIterator.
      class iterator {
         public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = dip::sint;       ///< The value obtained by dereferencing is an offset
            using difference_type = dip::sint;  ///< The type of distances between iterators
            using reference = dip::sint const&; ///< The type of a reference
            using pointer = dip::sint const*;   ///< The type of a pointer
            /// Default constructor yields an invalid iterator that cannot be dereferenced
            iterator() {}
            /// Constructs an iterator
            iterator( PixelTable const& pt, IntegerArray strides ) {
               dip_ThrowIf( pt.sizes_.size() != strides.size(), E::ARRAY_ILLEGAL_SIZE );
               pixelTable_ = &pt;
               strides_ = strides;
               ComputeOffset();
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
               swap( strides_, other.strides_ );
               swap( run_, other.run_ );
               swap( index_, other.index_ );
               swap( offset_, other.offset_ );
            }
            /// Dereference
            reference operator*() const { return offset_; }
            /// Increment
            iterator& operator++() {
               if( pixelTable_ && ( run_ < pixelTable_->runs_.size() )) {
                  ++index_;
                  if( index_ < pixelTable_->runs_[ run_ ].length ) {
                     offset_ += strides_[ pixelTable_->procDim_ ];
                  } else {
                     index_ = 0;
                     ++run_;
                     if( run_ < pixelTable_->runs_.size() ) {
                        ComputeOffset();
                     }
                  }
               }
               return *this;
            }
            /// Increment
            iterator operator++( int ) {
               iterator tmp( *this );
               operator++();
               return tmp;
            }
            /// Equality comparison, is true if the two iterators reference the same pixel in the same pixel table,
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
            operator bool() const { return !IsAtEnd(); }
            /// Return the current coordinate along the line, call only if iterator is valid
            IntegerArray Coordinates() const {
               IntegerArray coord = pixelTable_->runs_[ run_ ].coordinates;
               coord[ pixelTable_->procDim_ ] += index_;
               return coord;
            }
         private:
            PixelTable const* pixelTable_ = nullptr;
            IntegerArray strides_;  // the strides for the image we're operating on
            dip::uint run_ = 0;     // which run we're currently point at
            dip::uint index_ = 0;   // which pixel on the run we're currently pointing at
            dip::sint offset_;      // the offset to the pixel, given the strides

            void ComputeOffset() {
               offset_ = 0;
               for( dip::uint ii = 0; ii < pixelTable_->sizes_.size(); ++ii ) {
                  offset_ += pixelTable_->runs_[ run_ ].coordinates[ ii ] * strides_[ ii ];
               }
            }
      };

      /// A default-constructed pixel table is kinda useless
      PixelTable() {};
      /// A pixel table can be constructed for default filter `shape`s: "rectagular", "elliptic" and "diamond",
      /// which correspond to a unit circle in the L_{infinity} norm, the L_2 norm, and the L_1 norm. The
      /// `size` array determines the size and dimensionality; it gives the diameter of the neighborhood (not
      /// the radius!). For the "rectangular" shape, the diameter is rounded to the nearest integer, yielding
      /// a rectangle that is even or odd in size. For the "diamond" shape, the diameter is rounded to the
      /// nearet odd integer. For the "elliptic" shape, the diameter is not rounded at all, but always yields an
      /// odd-sized bounding box. `procDim` indicates the processing dimension.
      PixelTable( String shape, FloatArray size, dip::uint procDim = 0 );
      /// A pixel table can be constructed for an arbitrary shape defined by a binary image, where set
      /// pixels indicate pixels that belong to the neighborhood. `origin` gives the coordinates of the pixel
      /// in the image that will be placed at the origin (i.e. have coordinates {0,0,0}.
      /// `procDim` indicates the processing dimension.
      PixelTable( Image mask, UnsignedArray origin = {}, dip::uint procDim = 0 );
      /// Returns the vector of runs
      std::vector< PixelRun > const& Runs() const { return runs_; }
      /// Returns the dimensionality of the neighborhood
      dip::uint Dimensionality() const { return sizes_.size(); }
      /// Returns the size of the bounding box of the neighborhood
      UnsignedArray const& Sizes() const { return sizes_; }
      /// Returns the origin of the neighborhood w.r.t. the top-left corner of the bounding box
      UnsignedArray const& Origin() const { return origin_; }
      /// Returns the number of pixels in the neighborhood
      dip::uint NumberOfPixels() const { return nPixels_; }
      /// Returns the processing dimension
      dip::uint ProcessingDimension() const { return procDim_; }
      /// A const iterator to the first pixel in the neighborhood; the image's strides need to be given
      // NOTE: we don't have a `begin()` method because we cannot make an iterator without the strides.
      iterator BeginIterator( IntegerArray strides ) const {
         return iterator( *this, strides );
      }
      /// A const iterator to one past the last pixel in the neighborhood
      iterator EndIterator() const {
         return iterator::end( *this );
      };
      /// Cast to dip::Image yields a binary image representing the neighborhood
      operator dip::Image() const;

   private:
      std::vector< PixelRun > runs_;
      UnsignedArray sizes_;      // the size of the bounding box
      UnsignedArray origin_;     // the coordinates of the origin w.r.t. the top-left corner of the bounding box
      dip::uint nPixels_ = 0;    // the total number of pixels in the pixel table
      dip::uint procDim_ = 0;    // the dimension along which the runs go
};

inline void swap( PixelTable::iterator& v1, PixelTable::iterator& v2 ) {
   v1.swap( v2 );
}

} // namespace dip

#endif // DIP_PIXEL_TABLE_H
