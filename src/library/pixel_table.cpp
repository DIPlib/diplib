/*
 * DIPlib 3.0
 * This file contains definitions of functions declared in pixel_table.h.
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

#include "diplib/pixel_table.h"
#include "diplib/iterators.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"

namespace dip {

// Construct a pixel table with offsets from a pixel table
PixelTableOffsets::PixelTableOffsets(
      PixelTable const& pt,
      Image const& image
) {
   sizes_ = pt.Sizes();
   origin_ = pt.Origin();
   nPixels_ = pt.NumberOfPixels();
   procDim_ = pt.ProcessingDimension();
   stride_ = image.Stride( procDim_ );
   auto const& inRuns = pt.Runs();
   runs_.resize( inRuns.size() );
   for( dip::uint ii = 0; ii < runs_.size(); ++ii ) {
      runs_[ ii ].offset = image.Offset( inRuns[ ii ].coordinates );
      runs_[ ii ].length = inRuns[ ii ].length;
   }
   weights_ = pt.Weights();
}

// Construct a pixel table from a given shape and size
PixelTable::PixelTable(
      String const& shape,
      FloatArray size, // by copy
      dip::uint procDim
) {
   dip::uint nDims = size.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( procDim >= nDims, E::INVALID_PARAMETER );
   procDim_ = procDim;

   if( shape == S::LINE ) {

      //
      // Construct a pixel table from a Bresenham line
      //
      // Ideally runs go along the longest dimension of the line.
      // Worst case is when they go along the shortest, then all runs have length 1.

      // Initialize sizes and origin, and find the start and end position
      sizes_.resize( nDims, 0 );
      origin_.resize( nDims, 0 );
      UnsignedArray startPos( nDims, 0 );
      UnsignedArray endPos( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( size[ ii ] < 0 ) {
            size[ ii ] = -std::max( std::round( -size[ ii ] ), 1.0 );
            sizes_[ ii ] = static_cast< dip::uint >( -size[ ii ] );
            startPos[ ii ] = sizes_[ ii ] - 1;
         } else {
            size[ ii ] = std::max( std::round( size[ ii ] ), 1.0 );
            sizes_[ ii ] = static_cast< dip::uint >( size[ ii ] );
            endPos[ ii ] = sizes_[ ii ] - 1;
         }
      }

      //std::cout << "\n[PixelTable] size = " << size << ", sizes_ = " << sizes_ << ", origin_ = " << origin_ << std::endl;
      //std::cout << "[PixelTable] startPos = " << startPos << ", endPos = " << endPos << std::endl;

      // Create some false strides for the iterator. ProcDim gets stride==0, others get stride==1.
      IntegerArray falseStrides( nDims, 1 );
      falseStrides[ procDim ] = 0;
      // Create iterator to walk from startPos to endPos
      BresenhamLineIterator iterator( falseStrides, startPos, endPos );
      dip::uint length = iterator.Length() + 1;
      //std::cout << "[PixelTable] length = " << length << std::endl;

      if( length >= 2 ) {
         // We need the line to go through the origin, which can be done by setting `origin_` properly,
         // but predicting what it needs to be is a little complex, depending on even/odd lengths in combinations
         // of dimensions. For now we just record the coordinates when we reach the origin along the processing
         // dimension, and shift the origin later on.
         IntegerArray shift;
         // Walk the line, extract runs
         IntegerArray coords{ iterator.Coordinates() }; // cast dip::uint to dip::sint.
         dip::uint runLength = 1;
         //std::cout << "[PixelTable] pos = " << pos << ", coords = " << coords << std::endl;
         for( dip::uint step = 1; step < length; ++step ) {
            ++iterator; // we increment at the start of the loop: the first pixel is already "processed" before the loop starts.
            // Are all integer coordinates the same except for the one along procDim_?
            bool same = true;
            IntegerArray newcoords{ iterator.Coordinates() };
            //std::cout << "[PixelTable] newcoords = " << newcoords << ", coords = " << coords << std::endl;
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               if( ii != procDim_ ) {
                  if( newcoords[ ii ] != coords[ ii ] ) {
                     same = false;
                     break;
                  }
               }
            }
            if( !same ) {
               // Save run
               runs_.emplace_back( coords, runLength );
               //std::cout << "[PixelTable] added run: " << coords << ", runLength = " << runLength << std::endl;
               nPixels_ += runLength;
               // Start new run
               coords = std::move( newcoords );
               runLength = 1;
            } else {
               ++runLength;
            }
            // Are we at the origin?
            // Note: If length/2==0, this will never test true. But in that case, we don't need to shift.
            if( step == length / 2 ) {
               shift = IntegerArray( iterator.Coordinates() ); // cannot use newcoords because we might have moved from it.
               //std::cout << "[PixelTable] shift = " << shift << std::endl;
            }
         }
         runs_.emplace_back( coords, runLength );
         nPixels_ += runLength;
         if( !shift.empty() ) {
            ShiftOrigin( shift );
         }
      } else {
         // A single point!
         runs_.emplace_back( origin_, 1 );
         nPixels_ = 1;
      }

   } else {

      //
      // Construct a pixel table from a unit circle in different metrics
      //

      // Make sure filter is at least 1px in each dimension
      for( auto& s : size ) {
         s = std::max( 1.0, s );
      }

      if( shape == S::RECTANGULAR ) {
         // A rectangle has all runs of the same length, easy!

         // Initialize sizes and origin
         sizes_.resize( nDims, 0 );
         origin_.resize( nDims, 0 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            sizes_[ ii ] = static_cast< dip::uint >( size[ ii ] );
            origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
         }

         // Determine number of pixel table runs
         dip::uint nRuns = 1;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != procDim ) {
               nRuns *= sizes_[ ii ];
            }
         }
         runs_.reserve( nRuns );
         dip::uint length = sizes_[ procDim ];
         nPixels_ = nRuns * length;

         // Fill the pixel table runs
         IntegerArray cor = origin_;
         for( ;; ) {

            // Fill next pixel table run
            runs_.emplace_back( cor, length );

            // Some nD looping bookkeeping stuff
            dip::uint ii = 0;
            for( ; ii < nDims; ++ii ) {
               if( ii == procDim ) {
                  continue;
               }
               ++cor[ ii ];
               if( cor[ ii ] >= origin_[ ii ] + static_cast< dip::sint >( sizes_[ ii ] )) {
                  cor[ ii ] = origin_[ ii ];
                  continue;
               }
               break;
            }
            if( ii >= nDims ) {
               break;
            }
         }

      } else if( shape == S::ELLIPTIC ) {
         // A unit circle in Euclidean space, normalized by the sizes.

         // Initialize sizes and origin
         sizes_.resize( nDims, 0 );
         origin_.resize( nDims, 0 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            sizes_[ ii ] = ( static_cast< dip::uint >( size[ ii ] ) / 2 ) * 2 + 1;
            origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
            size[ ii ] /= 2;
         }

         // Fill the pixel table runs
         dfloat sz = size[ procDim ];
         IntegerArray cor = origin_;
         for( ;; ) { // Loop over image lines

            // Find the square distance from the origin for the pixel in the middle of this line
            dfloat distance2 = 0.0;
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               if( ii != procDim ) {
                  dfloat tmp = static_cast< dfloat >( cor[ ii ] ) / size[ ii ];
                  distance2 += tmp * tmp;
               }
            }
            // If we're still within the radius, this line intersects the ellipsoid
            if( distance2 <= 1.0 ) {
               // Find the distance from the origin, along this line, that we can go and still stay within the ellipsoid
               dip::sint length = floor_cast( sz * std::sqrt( 1.0 - distance2 ));
               // Determine and fill the run for this line
               IntegerArray coordinate = cor;
               coordinate[ procDim ] = -length;
               dip::uint len = static_cast< dip::uint >( 2 * length + 1 );
               runs_.emplace_back( coordinate, len );
               nPixels_ += len;
            }

            // Some nD looping bookkeeping stuff
            dip::uint ii = 0;
            for( ; ii < nDims; ++ii ) {
               if( ii == procDim ) {
                  continue;
               }
               ++cor[ ii ];
               if( cor[ ii ] >= origin_[ ii ] + static_cast< dip::sint >( sizes_[ ii ] )) {
                  cor[ ii ] = origin_[ ii ];
                  continue;
               }
               break;
            }
            if( ii >= nDims ) {
               break;
            }
         }

      } else if( shape == S::DIAMOND ) {
         // Same as "elliptic" but with L1 norm.

         // Initialize sizes and origin
         sizes_.resize( nDims, 0 );
         origin_.resize( nDims, 0 );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            size[ ii ] /= 2;
            sizes_[ ii ] = static_cast< dip::uint >( size[ ii ] ) * 2 + 1;
            origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
         }

         // Fill the pixel table runs
         dfloat sz = size[ procDim ];
         IntegerArray cor = origin_;
         for( ;; ) { // Loop over image lines

            // Find the L1 distance from the origin for the pixel in the middle of this line
            dfloat distance = 0.0;
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               if( ii != procDim ) {
                  distance += static_cast< dfloat >( std::abs( cor[ ii ] )) / size[ ii ];
               }
            }
            // If we're still within the radius, this line intersects the diamond-oid
            if( distance <= 1.0 ) {
               // Find the distance from the origin, along this line, that we can go and still stay within the ellipsoid
               dip::sint length = floor_cast( sz * ( 1.0 - distance ));
               // Determine and fill the run for this line
               IntegerArray coordinate = cor;
               coordinate[ procDim ] = -length;
               dip::uint len = static_cast< dip::uint >( 2 * length + 1 );
               runs_.emplace_back( coordinate, len );
               nPixels_ += len;
            }

            // some nD looping bookkeeping stuff
            dip::uint ii = 0;
            for( ; ii < nDims; ++ii ) {
               if( ii == procDim ) {
                  continue;
               }
               ++cor[ ii ];
               if( cor[ ii ] >= origin_[ ii ] + static_cast< dip::sint >( sizes_[ ii ] )) {
                  cor[ ii ] = origin_[ ii ];
                  continue;
               }
               break;
            }
            if( ii >= nDims ) {
               break;
            }
         }

      } else {
         DIP_THROW( "Neighborhood shape name not recognized: " + shape );
      }
   }
}

// Construct a pixel table from a binary image
PixelTable::PixelTable(
      Image const& mask,
      IntegerArray const& origin,
      dip::uint procDim
) {
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( mask.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( mask.DataType() != DT_BIN, E::IMAGE_NOT_BINARY );
   dip::uint nDims = mask.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( procDim >= nDims, E::INVALID_PARAMETER );
   procDim_ = procDim;
   sizes_ = mask.Sizes();
   if( origin.empty() ) {
      origin_.resize( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
      }
   } else {
      DIP_THROW_IF( origin.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
      origin_.resize( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin_[ ii ] = -origin[ ii ];
      }
   }
   ImageIterator< dip::bin > it( mask, procDim );
   do {
      IntegerArray position = origin_;
      position += it.Coordinates();
      dip::sint start = position[ procDim ];
      dip::uint length = 0;
      auto data = it.GetLineIterator();
      do {
         if( *data ) {
            ++length;
         } else {
            if( length ) {
               position[ procDim ] = start + static_cast< dip::sint >( data.Coordinate() ) - static_cast< dip::sint >( length );
               runs_.emplace_back( position, length );
               nPixels_ += length;
            }
            length = 0;
         }
      } while( ++data );
      if( length ) {
         position[ procDim ] = start + static_cast< dip::sint >( data.Coordinate() ) - static_cast< dip::sint >( length );
         runs_.emplace_back( position, length );
         nPixels_ += length;
      }
   } while( ++it );
}

// Create a binary or grey-value image from a pixel table
void PixelTable::AsImage( Image& out ) const {
   if( HasWeights() ) {
      out.ReForge( sizes_, 1, DT_DFLOAT );
      out.Fill( 0.0 );
      dip::sint stride = out.Stride( procDim_ );
      auto wIt = weights_.begin();
      for( auto& run : runs_ ) {
         IntegerArray position = run.coordinates;
         position -= origin_;
         dfloat* data = static_cast< dfloat* >( out.Pointer( out.Offset( position )));
         for( dip::uint ii = 0; ii < run.length; ++ii ) {
            *data = *wIt;
            ++wIt;
            data += stride;
         }
      }
   } else {
      out.ReForge( sizes_, 1, DT_BIN );
      out.Fill( false );
      dip::sint stride = out.Stride( procDim_ );
      for( auto& run : runs_ ) {
         IntegerArray position = run.coordinates;
         position -= origin_;
         dip::bin* data = static_cast< dip::bin* >( out.Pointer( out.Offset( position )));
         for( dip::uint ii = 0; ii < run.length; ++ii ) {
            *data = true;
            data += stride;
         }
      }
   }
}

namespace {

template< typename TPI >
void AddWeightsInternal(
      Image const& image,
      dip::sint stride,
      std::vector< PixelTable::PixelRun > const& runs,
      std::vector< dfloat >& weights,
      IntegerArray const& origin
) {
   for( auto& run : runs ) {
      IntegerArray position = run.coordinates;
      position -= origin;
      TPI* data = static_cast< TPI* >( image.Pointer( position ));
      for( dip::uint ii = 0; ii < run.length; ++ii ) {
         weights.push_back( static_cast< dfloat >( *data ));
         data += stride;
      }
   }
}

} // namespace

// Add weights from an image
void PixelTable::AddWeights( Image const& image ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( image.TensorElements() != 1, E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( image.Sizes() != sizes_, E::SIZES_DONT_MATCH );
   DIP_THROW_IF( !image.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   weights_.reserve( nPixels_ );
   dip::sint stride = image.Stride( procDim_ );
   DIP_OVL_CALL_REAL( AddWeightsInternal, ( image, stride, runs_, weights_, origin_ ), image.DataType() );
   DIP_ASSERT( weights_.size() == nPixels_ );
}

// Add weights from distances
void PixelTable::AddDistanceToOriginAsWeights() {
   weights_.reserve( nPixels_ );
   for( auto& run : runs_ ) {
      IntegerArray position = run.coordinates;
      dfloat sum2 = position.norm_square();
      weights_.push_back( std::sqrt( sum2 ));
      if( run.length > 1 ) {
         dfloat d = static_cast< dfloat >( position[ procDim_ ] );
         for( dip::uint ii = 1; ii < run.length; ++ii ) {
            dfloat x = static_cast< dfloat >( ii );
            // new sum2 = sum2 - d^2 + (d+x)^2 = sum2 + x^2 - 2*x*d
            weights_.push_back( std::sqrt( sum2 + x * x + 2 * x * d ));
         }
      }
   }
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing the PixelTable class") {
   dip::PixelTable pt( "elliptic", dip::FloatArray{ 10.1, 12.7, 5.3 }, 1 );
   DOCTEST_REQUIRE( pt.Sizes().size() == 3 );
   DOCTEST_CHECK( pt.Sizes()[ 0 ] == 11 );
   DOCTEST_CHECK( pt.Sizes()[ 1 ] == 13 );
   DOCTEST_CHECK( pt.Sizes()[ 2 ] == 5 );
   DOCTEST_REQUIRE( pt.Origin().size() == 3 );
   DOCTEST_CHECK( pt.Origin()[ 0 ] == -5 );
   DOCTEST_CHECK( pt.Origin()[ 1 ] == -6 );
   DOCTEST_CHECK( pt.Origin()[ 2 ] == -2 );
   DOCTEST_CHECK( pt.Runs().size() == 43 );
   DOCTEST_CHECK( pt.NumberOfPixels() == 359 );
   DOCTEST_CHECK( pt.ProcessingDimension() == 1 );
   DOCTEST_CHECK_FALSE( pt.HasWeights() );

   dip::Image img = pt.AsImage(); // convert to image
   dip::PixelTable pt2( img, {}, 1 ); // convert back to pixel table, should be exactly the same table.
   DOCTEST_REQUIRE( pt2.Sizes().size() == 3 );
   DOCTEST_CHECK( pt2.Sizes()[ 0 ] == 11 );
   DOCTEST_CHECK( pt2.Sizes()[ 1 ] == 13 );
   DOCTEST_CHECK( pt2.Sizes()[ 2 ] == 5 );
   DOCTEST_REQUIRE( pt2.Origin().size() == 3 );
   DOCTEST_CHECK( pt2.Origin()[ 0 ] == -5 );
   DOCTEST_CHECK( pt2.Origin()[ 1 ] == -6 );
   DOCTEST_CHECK( pt2.Origin()[ 2 ] == -2 );
   DOCTEST_CHECK( pt2.Runs().size() == 43 );
   DOCTEST_CHECK( pt2.NumberOfPixels() == 359 );
   DOCTEST_CHECK( pt2.ProcessingDimension() == 1 );
   DOCTEST_CHECK_FALSE( pt2.HasWeights() );
   DOCTEST_CHECK( pt.Runs()[ 0 ].coordinates == pt2.Runs()[ 0 ].coordinates );

   dip::PixelTable pt3( "rectangular", dip::FloatArray{ 22.2, 33.3 }, 0 );
   DOCTEST_REQUIRE( pt3.Sizes().size() == 2 );
   DOCTEST_CHECK( pt3.Sizes()[ 0 ] == 22 );
   DOCTEST_CHECK( pt3.Sizes()[ 1 ] == 33 );
   DOCTEST_REQUIRE( pt3.Origin().size() == 2 );
   DOCTEST_CHECK( pt3.Origin()[ 0 ] == -11 );
   DOCTEST_CHECK( pt3.Origin()[ 1 ] == -16 );
   DOCTEST_CHECK( pt3.Runs().size() == 33 );
   DOCTEST_CHECK( pt3.NumberOfPixels() == 22*33 );
   DOCTEST_CHECK( pt3.ProcessingDimension() == 0 );
   DOCTEST_CHECK_FALSE( pt3.HasWeights() );

   dip::PixelTable pt4( "diamond", dip::FloatArray{ 10.1, 12.7, 5.3 }, 2 );
   DOCTEST_REQUIRE( pt4.Sizes().size() == 3 );
   DOCTEST_CHECK( pt4.Sizes()[ 0 ] == 11 );
   DOCTEST_CHECK( pt4.Sizes()[ 1 ] == 13 );
   DOCTEST_CHECK( pt4.Sizes()[ 2 ] == 5 );
   DOCTEST_REQUIRE( pt4.Origin().size() == 3 );
   DOCTEST_CHECK( pt4.Origin()[ 0 ] == -5 );
   DOCTEST_CHECK( pt4.Origin()[ 1 ] == -6 );
   DOCTEST_CHECK( pt4.Origin()[ 2 ] == -2 );
   DOCTEST_CHECK( pt4.Runs().size() == 67 );
   DOCTEST_CHECK( pt4.NumberOfPixels() == 127 );
   DOCTEST_CHECK( pt4.ProcessingDimension() == 2 );
   DOCTEST_CHECK_FALSE( pt4.HasWeights() );

   dip::PixelTable pt5( "line", dip::FloatArray{ 14.1, -4.2, 7.9 }, 0 );
   DOCTEST_CHECK( pt5.NumberOfPixels() == 14 );
   DOCTEST_CHECK( pt5.ProcessingDimension() == 0 );
   DOCTEST_REQUIRE( pt5.Sizes().size() == 3 );
   DOCTEST_CHECK( pt5.Sizes()[ 0 ] == 14 );
   DOCTEST_CHECK( pt5.Sizes()[ 1 ] == 4 );
   DOCTEST_CHECK( pt5.Sizes()[ 2 ] == 8 );
   DOCTEST_REQUIRE( pt5.Origin().size() == 3 );
   DOCTEST_CHECK( pt5.Origin()[ 0 ] == -7 );
   DOCTEST_CHECK( pt5.Origin()[ 1 ] == -1 );
   DOCTEST_CHECK( pt5.Origin()[ 2 ] == -4 );
   DOCTEST_REQUIRE( pt5.Runs().size() == 8 );
   DOCTEST_CHECK( pt5.Runs()[ 0 ].length == pt5.Runs()[ 4 ].length ); // the line has two identical halves
   DOCTEST_CHECK( pt5.Runs()[ 1 ].length == pt5.Runs()[ 5 ].length ); // the line has two identical halves
   DOCTEST_CHECK( pt5.Runs()[ 2 ].length == pt5.Runs()[ 6 ].length ); // the line has two identical halves
   DOCTEST_CHECK( pt5.Runs()[ 3 ].length == pt5.Runs()[ 7 ].length ); // the line has two identical halves
   DOCTEST_CHECK_FALSE( pt5.HasWeights() );
}

#endif // DIP__ENABLE_DOCTEST
