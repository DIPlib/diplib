/*
 * DIPlib 3.0
 * This file contains declarations for the describing local neighborhoods.
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

#ifndef DIP_NEIGHBORHOOD_H
#define DIP_NEIGHBORHOOD_H

#include "diplib.h"
#include "diplib/pixel_table.h"
#include "diplib/math.h"
#include "iterators.h"


/// \file
/// \brief Defines various ways of describing a neighborhood


namespace dip {


/// \addtogroup infrastructure
/// \{

/// \brief Objects of class `%KernelShape` represent the shape and size of a filtering kernel.
///
/// Some image filters allow the specification of arbitrary kernels: the user can specify the shape
/// name and the size of a pre-defined kernel, or the user can pass an image containing the kernel.
///
/// Objects of type `dip::Image`, `dip::FloatArray` and `dip::String` implicitly convert to
/// a `%dip::KernelShape`, so it should be convenient to use these various representations in your
/// code.
///
/// To define a kernel by shape and size, pass a string defining the shape, and a floating-point
/// array with the size along each dimension.
/// These are the valid shape strings:
///
/// - `"elliptic"`: The unit circle in Euclidean (\f$L^2\f$) metric.
///
/// - `"rectangular"`: A box, the unit circle in \f$L^1\f$ metric.
///
/// - `"diamond"`: A box rotated 45 degrees, the unit circle in \f$L^\infty\f$ metric (max-norm).
///
/// In each of these cases, the `size` array indicates the diameter of the circle. The value can
/// be different along each dimension, simply stretching the shape. Note that the sizes are not
/// necessarily odd, and don't even need to be integers. Pixels included in the neighborhood are
/// those covered by the circle, with the origin on a pixel. In the case of the rectangle, however,
/// the box is shifted by half a pixel if `floor(size)` is even. This means that the rectangular
/// kernel is not necessarily symmetric. Set the `size` to odd values to ensure symmetry. Any
/// size that is smaller or equal to 1 causes the kernel to not have an extent in that direction.
///
/// To define a kernel through an image, provide a binary image. The "on" or "true" pixels form
/// the kernel. Note that, for most filters, the image is directly used as neighborhood (i.e. no
/// mirroring is applied). As elsewhere, the origin of the kernel is in the middle of the image,
/// and on the pixel to the right of the center in case of an even-sized image. If the image
/// is a grey-value image, then all pixels with a finite value form the kernel. The kernel then
/// has the given weights associated to each pixel.
///
/// See dip::StructuringElement, dip::NeighborList, dip::PixelTable
class DIP_NO_EXPORT Kernel{
   public:
      enum class ShapeCode {
            RECTANGULAR,
            ELLIPTIC,
            DIAMOND,
            CUSTOM
      };

      /// \brief The default kernel is a disk with a diameter of 7 pixels.
      Kernel() : shape_( ShapeCode::ELLIPTIC ), params_( { 7 } ) {}

      /// \brief A string implicitly converts to a kernel, it is interpreted as a shape.
      Kernel( String const& shape ) : params_( { 7 } ) {
         SetShape( shape );
      }

      /// \brief A `dip::FloatArray` implicitly converts to a kernel, it is interpreted as the
      /// parameter for each dimension. A second argument specifies the shape.
      Kernel( FloatArray const& params, String const& shape = "elliptic" ) : params_( params ) {
         SetShape( shape );
      }

      /// \brief A floating-point value implicitly converts to a kernel, it is interpreted as the
      /// parameter for all dimensions. A second argument specifies the shape.
      Kernel( dfloat param, String const& shape = "elliptic" ) : params_( FloatArray{ param } ) {
         SetShape( shape );
      }

      /// \brief Low-level constructor mostly for internal use.
      Kernel( ShapeCode shape, FloatArray const& params ) : shape_( shape ), params_( params ) {}

      /// \brief An image implicitly converts to a kernel, optionally with weights.
      Kernel( Image const& image ) : shape_( ShapeCode::CUSTOM ), image_( image.QuickCopy() ) {
         DIP_THROW_IF( !image_.IsForged(), E::IMAGE_NOT_FORGED );
         DIP_THROW_IF( !image_.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( image_.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
      }

      /// \brief Mirrors the kernel. This has no effect on elliptic or diamond kernels, which are always symmetric.
      void Mirror() {
         mirror_ = !mirror_;
      }

      /// \brief Creates a `dip::PixelTable` structure representing the shape of the SE
      dip::PixelTable PixelTable( UnsignedArray const& imsz, dip::uint procDim ) const {
         dip::uint nDim = imsz.size();
         DIP_THROW_IF( nDim < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         dip::PixelTable pixelTable;
         if( IsCustom() ) {
            DIP_THROW_IF( image_.Dimensionality() > nDim, E::DIMENSIONALITIES_DONT_MATCH );
            Image kernel = image_.QuickCopy();
            kernel.ExpandDimensionality( nDim );
            if( mirror_ ) {
               kernel.Mirror();
            }
            if( kernel.DataType().IsBinary()) {
               DIP_START_STACK_TRACE
                  pixelTable = { kernel, {}, procDim };
               DIP_END_STACK_TRACE
            } else {
               DIP_START_STACK_TRACE
                  pixelTable = { IsFinite( kernel ), {}, procDim };
                  pixelTable.AddWeights( kernel );
               DIP_END_STACK_TRACE
            }
            if( mirror_ ) {
               pixelTable.MirrorOrigin();
            }
         } else {
            FloatArray sz = params_;
            DIP_START_STACK_TRACE
               ArrayUseParameter( sz, nDim, 1.0 );
               pixelTable = { ShapeString(), sz, procDim };
            DIP_END_STACK_TRACE
            if( mirror_ ) {
               pixelTable.MirrorOrigin();
            }
         }
         return pixelTable;
      }

      /// \brief Retrieves the size of the kernel, adjusted to an image of size `imsz`.
      UnsignedArray Sizes( UnsignedArray const& imsz ) const {
         dip::uint nDim = imsz.size();
         DIP_THROW_IF( nDim < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         UnsignedArray out;
         if( IsCustom() ) {
            DIP_THROW_IF( image_.Dimensionality() > nDim, E::DIMENSIONALITIES_DONT_MATCH );
            out = image_.Sizes();
            out.resize( nDim, 1 ); // expand dimensionality by adding singletons
         } else {
            FloatArray sz = params_;
            DIP_START_STACK_TRACE
               ArrayUseParameter( sz, nDim, 1.0 );
            DIP_END_STACK_TRACE
            out.resize( nDim );
            bool rect = IsRectangular();
            for( dip::uint ii = 0; ii < nDim; ++ii ) {
               out[ ii ] = rect
                           ? static_cast< dip::uint >( sz[ ii ] )
                           : ( static_cast< dip::uint >( sz[ ii ] ) / 2 ) * 2 + 1;
            }
         }
         return out;
      }

      /// \brief Returns the kernel parameters, not adjusted to image dimensionality.
      FloatArray const& Params() const { return params_; }

      /// \brief Returns the kernel shape
      ShapeCode const& Shape() const { return shape_; }

      /// \brief Returns the kernel shape
      String ShapeString() const {
         switch( shape_ ) {
            case ShapeCode::RECTANGULAR:
               return "rectangular";
            case ShapeCode::ELLIPTIC:
               return "elliptic";
            case ShapeCode::DIAMOND:
               return "diamond";
            //case ShapeCode::CUSTOM:
            default:
               return "custom";
         }
      }

      /// \brief Tests to see if the kernel is rectangular
      bool IsRectangular() const { return shape_ == ShapeCode::RECTANGULAR; }

      /// \brief Tests to see if the kernel is a custom shape
      bool IsCustom() const { return shape_ == ShapeCode::CUSTOM; }

      /// \brief Tests to see if the kernel has weights
      bool HasWeights() const {
         return ( shape_ == ShapeCode::CUSTOM ) && !image_.DataType().IsBinary();
      }

   private:
      ShapeCode shape_;
      FloatArray params_;
      Image image_;
      bool mirror_ = false;

      void SetShape( String const& shape ) {
         if( shape == "elliptic" ) {
            shape_ = ShapeCode::ELLIPTIC;
         } else if( shape == "rectangular" ) {
            shape_ = ShapeCode::RECTANGULAR;
         } else if( shape == "diamond" ) {
            shape_ = ShapeCode::DIAMOND;
         } else {
            DIP_THROW( E::INVALID_FLAG );
         }
      }
};


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
               DIP_THROW( E::INVALID_FLAG );
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
class DIP_NO_EXPORT NeighborList {
   private:
      struct Neighbor {
         IntegerArray coords;
         dfloat distance;
      };
      using NeighborListData = std::vector< Neighbor >;
      using NeighborListIterator = std::vector< Neighbor >::const_iterator;

      NeighborListData neighbors_;

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
            IntegerArray Coordinates() const { return it_->coords; }
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

      void ConstructConnectivity( dip::uint dimensionality, dip::uint connectivity, FloatArray pixelSize ) {
         DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( connectivity > dimensionality, E::PARAMETER_OUT_OF_RANGE );
         if( connectivity == 0 ) {
            connectivity = dimensionality;
         }
         for( auto& pxsz : pixelSize ) {
            pxsz *= pxsz;
         }
         IntegerArray coords( dimensionality, -1 );
         for (;;) {
            dip::uint ii, kk = 0;
            dfloat dist2 = 0.0;
            for( ii = 0; ii < dimensionality; ++ii ) {
               if( coords[ ii ] != 0 ) {
                  ++kk;
                  dist2 += pixelSize[ ii ];
               }
            }
            if(( kk <= connectivity ) && ( kk > 0 )) {
               neighbors_.push_back( { coords, std::sqrt( dist2 ) } );
            }
            for( ii = 0; ii < dimensionality; ++ii ) {
               ++coords[ ii ];
               if( coords[ ii ] <= 1 ) {
                  break;
               }
               coords[ ii ] = -1;
            }
            if( ii == dimensionality ) {
               break;
            }
         }
      }

      void ConstructChamfer( dip::uint dimensionality, dip::uint maxDistance, FloatArray pixelSize ) {
         DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( maxDistance < 1, E::PARAMETER_OUT_OF_RANGE );
         dip::sint lim = static_cast< dip::sint >( maxDistance );
         IntegerArray coords( dimensionality, -lim );
         for( ;; ) {
            bool use = false;
            dip::uint ii;
            dfloat dist2 = 0.0;
            for( ii = 0; ii < dimensionality; ++ii ) {
               if( std::abs( coords[ ii ] ) == 1 ) {
                  use = true;
                  break;
               }
            }
            if( use ) {
               for( ii = 0; ii < dimensionality; ++ii ) {
                  dfloat tmp = static_cast< dfloat >( coords[ ii ] ) * pixelSize[ ii ];
                  dist2 += tmp * tmp;
               }
               neighbors_.push_back( { coords, std::sqrt( dist2 ) } );
            }
            for( ii = 0; ii < dimensionality; ++ii ) {
               ++coords[ ii ];
               if( coords[ ii ] <= lim ) {
                  break;
               }
               coords[ ii ] = -lim;
            }
            if( ii == dimensionality ) {
               break;
            }
         }
      }

      void ConstructImage( dip::uint dimensionality, Image const& c_metric ) {
         DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
         DIP_THROW_IF( c_metric.Dimensionality() > dimensionality, E::DIMENSIONALITIES_DONT_MATCH );
         Image metric = c_metric.QuickCopy();
         metric.ExpandDimensionality( dimensionality );
         IntegerArray offset( dimensionality, 0 );
         for( dip::uint ii = 0; ii < dimensionality; ++ii ) {
            DIP_THROW_IF( !( metric.Size( ii ) & 1 ), "Metric image must be odd in size (so I know where the center is)" );
            offset[ ii ] = static_cast< dip::sint >( metric.Size( ii ) / 2 );
         }
         if( metric.DataType() != DT_DFLOAT ) {
            metric.Convert( DT_DFLOAT );
         }
         ImageIterator< dfloat > it( metric );
         do {
            if( *it > 0 ) {
               IntegerArray coords{ it.Coordinates() };
               coords -= offset;
               DIP_THROW_IF( !coords.any(), "Metric image must have a distance of 0 in the middle" );
               neighbors_.push_back( { coords, *it } );
            }
         } while( ++it );
      }
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

#endif // DIP_NEIGHBORHOOD_H
