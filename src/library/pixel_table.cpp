/*
 * DIPlib 3.0
 * This file contains definitions of functions declared in pixel_table.h.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib/pixel_table.h"
#include "diplib/iterators.h"

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

// Construct a pixel table from a unit circle
PixelTable::PixelTable(
      String const& shape,
      FloatArray size, // by copy
      dip::uint procDim
) {
   // Make sure filter is at least 1px in each dimension
   for( auto& s : size ) {
      s = std::max( 1.0, s );
   }
   dip::uint nDims = size.size();
   DIP_THROW_IF( procDim >= nDims, E::PARAMETER_OUT_OF_RANGE );
   procDim_ = procDim;

   if( shape == "rectangular" ) {
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

   } else if( shape == "elliptic" ) {
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
      IntegerArray cor = origin_;
      for( ;; ) {

         // Find the first position along the line that lies within the ellipse
         dfloat radius = 0.0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != procDim ) {
               dfloat tmp = static_cast< dfloat >( cor[ ii ] ) / size[ ii ];
               radius += tmp * tmp;
            }
         }
         if( radius <= 1.0 ) {
            // TODO: this bit below must be possible to do without a loop!?
            dip::sint length = origin_[ procDim ];
            dfloat tmp = static_cast< dfloat >( length ) / size[ procDim ];
            tmp *= tmp;
            while( ( radius + tmp ) > 1.0 ) {
               ++length;
               if( length > 0 ) {
                  break;
               }
               tmp = static_cast< dfloat >( length ) / size[ procDim ];
               tmp *= tmp;
            }

            // Fill next pixel table run
            if((( radius + tmp ) <= 1.0 ) && ( length <= 0 )) {
               IntegerArray coordinate = cor;
               coordinate[ procDim ] = length;
               dip::uint len = static_cast< dip::uint >( 2 * -length + 1 );
               runs_.emplace_back( coordinate, len);
               nPixels_ += len;
            }
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

   } else if( shape == "diamond" ) {
      // Same as "elliptic" but with L1 norm.

      // Initialize sizes and origin
      sizes_.resize( nDims, 0 );
      origin_.resize( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         sizes_[ ii ] = ( static_cast< dip::uint >( size[ ii ] ) / 2 ) * 2 + 1;
         origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
         size[ ii ] /= 2;
      }

      // Fill the pixel table runs
      IntegerArray cor = origin_;
      for( ;; ) {

         // Find the first position along the line that lies within the diamond
         dfloat radius = 0.0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            if( ii != procDim ) {
               radius += static_cast< dfloat >( std::abs( cor[ ii ] )) / size[ ii ];
            }
         }
         if( radius <= 1.0 ) {
            // TODO: this bit below must be possible to do without a loop!?
            dip::sint length = origin_[ procDim ];
            dfloat tmp = static_cast< dfloat >( std::abs( length )) / size[ procDim ];
            while( ( radius + tmp ) > 1.0 ) {
               ++length;
               if( length > 0 ) {
                  break;
               }
               tmp = static_cast< dfloat >( std::abs( length )) / size[ procDim ];
            }

            // Fill next pixel table run
            if( ( ( radius + tmp ) <= 1.0 ) && ( length <= 0 ) ) {
               IntegerArray coordinate = cor;
               coordinate[ procDim ] = length;
               dip::uint len = static_cast< dip::uint >( 2 * -length + 1 );
               runs_.emplace_back( coordinate, len);
               nPixels_ += len;
            }
         }

         // some nD looping bookkeeping stuff
         dip::uint ii = 0;
         for( ; ii < nDims; ++ii ) {
            if( ii == procDim ) {
               continue;
            }
            ++cor[ ii ];
            if( cor[ ii ] >= origin_[ ii ] + static_cast< dip::sint >( sizes_[ ii ] ) ) {
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
      DIP_THROW( "Neighborhood shape name not recognized" );
   }
}

// Construct a pixel table from a binary image
PixelTable::PixelTable(
      Image const& mask,
      IntegerArray const& origin,
      dip::uint procDim
) {
   DIP_THROW_IF( !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( mask.TensorElements() != 1, E::MASK_NOT_SCALAR );
   DIP_THROW_IF( mask.DataType() != DT_BIN, E::MASK_NOT_BINARY );
   dip::uint nDims = mask.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( procDim >= nDims, E::PARAMETER_OUT_OF_RANGE );
   procDim_ = procDim;
   sizes_ = mask.Sizes();
   if( origin.empty() ) {
      origin_.resize( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin_[ ii ] = -static_cast< dip::sint >( sizes_[ ii ] ) / 2;
      }
   } else {
      DIP_THROW_IF( origin.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
      origin_ = origin;
   }
   ImageIterator< dip::bin > it( mask, procDim );
   do {
      IntegerArray position( nDims, 0 );
      position += it.Coordinates();       // direct assignment does not work, as the one is UnsignedArray and the other IntegerArray.
      position += origin_;
      dip::sint start = position[ procDim ];
      dip::uint length = 0;
      auto data = it.GetLineIterator();
      do {
         if( *data ) {
            ++length;
         } else {
            if( length ) {
               position[ procDim ] = start + ( data.Coordinate() - length );
               runs_.emplace_back( position, length );
               nPixels_ += length;
            }
            length = 0;
         }
      } while( ++data );
      if( length ) {
         position[ procDim ] = start + ( data.Coordinate() - length );
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
         dfloat* data = static_cast< dfloat* >( out.Pointer( position ));
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
         dip::bin* data = static_cast< dip::bin* >( out.Pointer( position ));
         for( dip::uint ii = 0; ii < run.length; ++ii ) {
            *data = true;
            data += stride;
         }
      }
   }
}

#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the PixelTable class") {
   PixelTable pt( "elliptic", FloatArray{ 10.1, 12.7, 5.3 }, 1 );
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

   Image img = pt.AsImage(); // convert to image
   PixelTable pt2( img, {}, 1 ); // convert back to pixel table, should be exactly the same table.
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

   PixelTable pt3( "rectangular", FloatArray{ 22.2, 33.3 }, 0 );
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

   PixelTable pt4( "diamond", FloatArray{ 10.1, 12.7, 5.3 }, 2 );
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
}

#endif // DIP__ENABLE_DOCTEST

// Shift the origin
void PixelTable::ShiftOrigin( IntegerArray const& shift ) {
   dip::uint nDims = origin_.size();
   DIP_THROW_IF( shift.size() != nDims, E::ARRAY_ILLEGAL_SIZE );
   origin_ += shift;
   for( auto& run : runs_ ) {
      run.coordinates -= shift;
   }
}

// Add weights from an image
void PixelTable::AddWeights( Image const& image ) {
   // TODO: the old dip_GreyValuesInPixelTable goes here
}

// Add weights from distances
void PixelTable::AddDistanceToOriginAsWeights() {
   // TODO
}


} // namespace dip
