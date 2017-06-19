/*
 * DIPlib 3.0
 * This file contains the declaration for dip::NeighborList.
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_NEIGHBORLIST_H
#define DIP_NEIGHBORLIST_H

#include "diplib.h"


/// \file
/// \brief Defines a dimensionality-independent way of iterating over neighbor pixels


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief Represents a metric to be used to create a `dip::NeighborList`
///
/// A metric describes the distance between a pixel and its neighbors. It also describes implicitly the
/// size of a neighborhood, as the minimum neighborhood size required to propagate distances in the given
/// metric.
///
/// \see dip::NeighborList
class DIP_NO_EXPORT Metric {
   public:
      enum class TypeCode {
            CONNECTED,
            CHAMFER,
            IMAGE
      };

      /// \brief The default metric is the city distance (\f$L^1\f$ metric).
      Metric( TypeCode type = TypeCode::CONNECTED, dip::uint param = 1 ) : type_( type ), param_( param ) {}

      /// \brief A string implicitly converts to a metric.
      ///
      /// Valid metrics are:
      ///
      /// - `"city"`: \f$L^1\f$ metric, a neighborhood with connectivity = 1.
      ///
      /// - `"chess"`: \f$L^\infty\f$ metric, a neighborhood with connectivity = dimensionality.
      ///
      /// - `"chamfer"`: a chamfer metric. `param` indicates the neighborhood size: A value of 1
      /// gives a full 3x3 neighborhood (in 2D, or 3x3x3 in 3D, etc). A value of 2 gives the 5x5
      /// chamfer neighborhood (i.e. the 3x3 neighborhood plus the pixels that are night's move
      /// away from the origin).
      ///
      /// - `"connected"`: here, `param` is the connectivity, see \ref connectivity for information
      /// on the connectivity parameter. A value of 1 corresponds to the `"city"` metric; a value of
      /// 0 indicates a connectivity equal to the image dimensionality is requested, and corresponds
      /// to the `"chess"` metric.
      ///
      /// - `"4-connected"` is equivalent to `"connected"` with `param=1`.
      ///
      /// - `"8-connected"` is equivalent to `"connected"` with `param=2`.
      ///
      /// - `"6-connected"` is equivalent to `"connected"` with `param=1`.
      ///
      /// - `"18-connected"` is equivalent to `"connected"` with `param=2`.
      ///
      /// - `"28-connected"` is equivalent to `"connected"` with `param=3`.
      ///
      /// The `pixelSize` parameter, if given, causes the neighbor's distances to be scaled by the
      /// pixel size. The units must be identical in all dimensions, and only the magnitude is used.
      Metric( String const& type, dip::uint param = 1, dip::PixelSize const& pixelSize = {} ) {
         if( type == "chamfer" ) {
            DIP_THROW_IF( param < 1, E::PARAMETER_OUT_OF_RANGE );
            type_ = TypeCode::CHAMFER;
            param_ = param;
         } else {
            type_ = TypeCode::CONNECTED;
            if( type == "connected" ) {
               param_ = param;
            } else if( type == "city" ) {
               param_ = 1;
            } else if( type == "chess" ) {
               param_ = 0;
            } else if( type == "4-connected" ) {
               param_ = 1;
            } else if( type == "8-connected" ) {
               param_ = 2;
            } else if( type == "6-connected" ) {
               param_ = 1;
            } else if( type == "18-connected" ) {
               param_ = 2;
            } else if( type == "28-connected" ) {
               param_ = 3;
            } else {
               DIP_THROW_INVALID_FLAG( type );
            }
         }
         if( pixelSize.IsDefined() ) {
            pixelSize_.resize( pixelSize.Size() );
            auto pxsz = pixelSize[ 0 ];
            Units units = pxsz.units;
            pixelSize_[ 0 ] = pxsz.magnitude;
            for( dip::uint ii = 1; ii < pixelSize.Size(); ++ii ) {
               pxsz = pixelSize[ ii ];
               DIP_THROW_IF( pxsz.units != units, "The pixel size has different units along different dimensions" );
               pixelSize_[ ii ] = pxsz.magnitude;
            }
         }
      }

      /// \brief An image implicitly converts to a metric.
      Metric( dip::Image const& image ) : type_( TypeCode::IMAGE ), image_( image.QuickCopy() ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( image_.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
      }

      /// \brief Retrieve the type string.
      TypeCode Type() const { return type_; }

      /// \brief Retrieve the parameter.
      dip::uint Param() const { return param_; }

      /// \brief Retrieve the image.
      dip::Image const& Image() const { return image_; }

      /// \brief Retrieve the pixel size array.
      FloatArray const& PixelSize() const { return pixelSize_; }

   private:
      TypeCode type_;
      dip::uint param_;
      dip::Image image_;
      FloatArray pixelSize_;
};


/// \brief Defines the neighborhood of a pixel as a set of coordinates, with optionally their distance.
///
/// An object of this class lists all neighbors in some neighborhood of a pixel, and is useful in
/// dimensionality-agnostic algorithms that need direct access to each neighbor. The neighborhood
/// can be specified as a connectivity (as in 4-connected, 26-connected, etc., except that we use
/// an integer in a way that the concept extends readily to any dimensionality, see \ref connectivity).
/// Alternatively, the neighborhood can be created as used in chamfer distance computation, where
/// a larger neighborhood increases the accuracy of the computed distances.
///
/// If given a pixel size, the neighborhood list contains the magnitude of the physical distances
/// to each neighbor. Otherwise, a default unit distance is substituted.
///
/// See dip::Kernel, dip::PixelTable, dip::Metric
class DIP_NO_EXPORT NeighborList {
   private:
      struct Neighbor {
         IntegerArray coords;
         dfloat distance;
      };
      using NeighborListData = std::vector< Neighbor >;
      using NeighborListIterator = std::vector< Neighbor >::const_iterator;

      NeighborListData neighbors_;

      NeighborList() {} // Creating a default-initialized object only allowd by class methods.

   public:

      /// \brief Iterates over the neighbors in the `%NeighborList`.
      class Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = dfloat;             ///< The type that the iterator points at
            using reference = value_type const&;   ///< The type you get when you dereference
         public:
            Iterator() {}
            Iterator( NeighborListIterator const& it ) : it_( it ) {}
            /// Swap
            void swap( Iterator& other ) {
               using std::swap;
               swap( it_, other.it_ );
            }
            /// Dereference, yields the distance to the neighbor
            reference operator*() const { return it_->distance; }
            /// Get the coordinates for the current neighbor
            IntegerArray const& Coordinates() const { return it_->coords; }
            /// Increment
            Iterator& operator++() {
               ++it_;
               return *this;
            }
            /// Increment
            Iterator operator++( int ) {
               Iterator tmp( *this );
               ++it_;
               return tmp;
            }
            /// Equality comparison
            bool operator==( Iterator const& other ) const { return it_ == other.it_; }
            /// Inequality comparison
            bool operator!=( Iterator const& other ) const { return it_ != other.it_; }
            /// Returns true if the neighbor pointed to is within the image
            bool IsInImage( UnsignedArray const& coords, UnsignedArray const& imsz ) const {
               for( dip::uint ii = 0; ii < it_->coords.size(); ii++ ) {
                  // We're assuming it_->coords is very small w.r.t. the image size.
                  // Unsigned addition of uint + {..,-1,0,1,..}, works just fine: 0+(-1), yields a very large value.
                  dip::uint pos = coords[ ii ] + static_cast< dip::uint >( it_->coords[ ii ] );
                  if( pos >= imsz[ ii ] ) {
                     return false;
                  }
               }
               return true;
            }
         private:
            NeighborListIterator it_;
      };

      /// \brief Creates a `%NeighborList` given the image dimensionality and a `dip::Metric`.
      NeighborList( Metric const& metric, dip::uint dimensionality ) {
         if( metric.Type() == Metric::TypeCode::IMAGE ) {
            ConstructImage( dimensionality, metric.Image() );
         } else if( metric.Type() == Metric::TypeCode::CHAMFER ) {
            ConstructChamfer( dimensionality, metric.Param(), metric.PixelSize() );
         } else { // metric.Type() == Metric::TypeCode::CONNECTED
            ConstructConnectivity( dimensionality, metric.Param(), metric.PixelSize() );
         }
      }

      /// \brief Returns an array with offsets corresponding to each of the neighbors in the list, given an
      /// image's strides array.
      IntegerArray ComputeOffsets( IntegerArray const& strides ) const {
         dip::uint ndims = strides.size();
         DIP_THROW_IF( ndims != neighbors_[ 0 ].coords.size(), E::ARRAY_SIZES_DONT_MATCH );
         IntegerArray out( neighbors_.size() );
         for( dip::uint jj = 0; jj < neighbors_.size(); ++jj ) {
            auto const& coords = neighbors_[ jj ].coords;
            dip::sint offset = 0;
            for( dip::uint ii = 0; ii < ndims; ++ii ) {
               offset += coords[ ii ] * strides[ ii ];
            }
            out[ jj ] = offset;
         }
         return out;
      }

      /// \brief Returns a new `%NeighborList` object containing only those neighbors that would be processed earlier
      /// if processing as `ImageIterator` and the like would. `procDim` must be the iterator's processing dimension.
      DIP_EXPORT NeighborList SelectBackward( dip::uint procDim = 0 ) const;

      /// \brief Returns a new `%NeighborList` object containing only those neighbors that would be processed later
      /// if processing as `ImageIterator` and the like would. `procDim` must be the iterator's processing dimension.
      DIP_EXPORT NeighborList SelectForward( dip::uint procDim = 0 ) const;

      /// \brief Returns the number of neighbors.
      dip::uint Size() const {
         return neighbors_.size();
      }

      /// \brief A forward iterator to the first neighbor
      Iterator begin() const {
         return Iterator{ neighbors_.begin() };
      }

      /// \brief A forward iterator to one past the last neighbor
      Iterator end() const {
         return Iterator{ neighbors_.end() };
      }

   private:
      DIP_EXPORT void ConstructConnectivity( dip::uint dimensionality, dip::uint connectivity, FloatArray pixelSize );
      DIP_EXPORT void ConstructChamfer( dip::uint dimensionality, dip::uint maxDistance, FloatArray pixelSize );
      DIP_EXPORT void ConstructImage( dip::uint dimensionality, Image const& c_metric );
};

inline void swap( NeighborList::Iterator& v1, NeighborList::Iterator& v2 ) {
   v1.swap( v2 );
}

/// \}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the NeighborList class") {
   dip::dfloat x = 1.2;
   dip::dfloat y = 1.6;
   dip::dfloat diag = std::hypot( x, y );
   dip::dfloat diag_v = std::hypot( x, 2*y );
   dip::dfloat diag_h = std::hypot( 2*x, y );
   dip::PixelSize pxsz{ dip::PhysicalQuantityArray{ 1.2 * dip::Units::Meter(), 1.6 * dip::Units::Meter() }};
   dip::NeighborList list( dip::Metric( "connected", 2, pxsz ), 2 );
   DOCTEST_REQUIRE( list.Size() == 8 );
   auto it = list.begin();
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( y ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( x ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( x ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( y ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   dip::IntegerArray strides{ 1, 10 };
   dip::IntegerArray offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 8 );
   auto ot = offsets.begin();
   DOCTEST_CHECK( *(ot++) == -1 -10 );
   DOCTEST_CHECK( *(ot++) == +0 -10 );
   DOCTEST_CHECK( *(ot++) == +1 -10 );
   DOCTEST_CHECK( *(ot++) == -1 + 0 );
   DOCTEST_CHECK( *(ot++) == +1 + 0 );
   DOCTEST_CHECK( *(ot++) == -1 +10 );
   DOCTEST_CHECK( *(ot++) == +0 +10 );
   DOCTEST_CHECK( *(ot++) == +1 +10 );

   list = dip::NeighborList( dip::Metric( "chamfer", 2, pxsz ), 2 );
   DOCTEST_REQUIRE( list.Size() == 16 );
   it = list.begin();
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_v ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_v ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_h ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( y ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_h ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( x ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( x ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_h ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( y ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_h ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_v ));
   DOCTEST_CHECK( *(it++) == doctest::Approx( diag_v ));
   offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 16 );
   ot = offsets.begin();
   DOCTEST_CHECK( *(ot++) == -1 -20 );
   DOCTEST_CHECK( *(ot++) == +1 -20 );
   DOCTEST_CHECK( *(ot++) == -2 -10 );
   DOCTEST_CHECK( *(ot++) == -1 -10 );
   DOCTEST_CHECK( *(ot++) == +0 -10 );
   DOCTEST_CHECK( *(ot++) == +1 -10 );
   DOCTEST_CHECK( *(ot++) == +2 -10 );
   DOCTEST_CHECK( *(ot++) == -1 + 0 );
   DOCTEST_CHECK( *(ot++) == +1 + 0 );
   DOCTEST_CHECK( *(ot++) == -2 +10 );
   DOCTEST_CHECK( *(ot++) == -1 +10 );
   DOCTEST_CHECK( *(ot++) == +0 +10 );
   DOCTEST_CHECK( *(ot++) == +1 +10 );
   DOCTEST_CHECK( *(ot++) == +2 +10 );
   DOCTEST_CHECK( *(ot++) == -1 +20 );
   DOCTEST_CHECK( *(ot++) == +1 +20 );

   dip::Image m( { 3, 3 }, 1, dip::DT_UINT8 );
   dip::uint8* ptr = ( dip::uint8* )m.Origin();
   for( dip::uint ii = 1; ii <= 9; ++ii ) {
      *( ptr++ ) = ( dip::uint8 )ii;
   }
   ptr[ -5 ] = 0;
   list = dip::NeighborList( m, 2 );
   DOCTEST_REQUIRE( list.Size() == 8 );
   it = list.begin();
   DOCTEST_CHECK( *(it++) == 1 );
   DOCTEST_CHECK( *(it++) == 2 );
   DOCTEST_CHECK( *(it++) == 3 );
   DOCTEST_CHECK( *(it++) == 4 );
   DOCTEST_CHECK( *(it++) == 6 );
   DOCTEST_CHECK( *(it++) == 7 );
   DOCTEST_CHECK( *(it++) == 8 );
   DOCTEST_CHECK( *(it++) == 9 );
   offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 8 );
   ot = offsets.begin();
   DOCTEST_CHECK( *(ot++) == -1 -10 );
   DOCTEST_CHECK( *(ot++) == +0 -10 );
   DOCTEST_CHECK( *(ot++) == +1 -10 );
   DOCTEST_CHECK( *(ot++) == -1 + 0 );
   DOCTEST_CHECK( *(ot++) == +1 + 0 );
   DOCTEST_CHECK( *(ot++) == -1 +10 );
   DOCTEST_CHECK( *(ot++) == +0 +10 );
   DOCTEST_CHECK( *(ot++) == +1 +10 );
}

#endif // DIP__ENABLE_DOCTEST

#endif // DIP_NEIGHBORLIST_H
