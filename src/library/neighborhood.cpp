/*
 * DIPlib 3.0
 * This file contains definitions of a few functions from dip::Kernel and dip::NeighborList
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

#include "diplib/kernel.h"
#include "diplib/neighborlist.h"
#include "diplib/pixel_table.h"
#include "diplib/iterators.h"
#include "diplib/math.h"

namespace dip {

dip::PixelTable Kernel::PixelTable( dip::uint nDims, dip::uint procDim ) const {
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   dip::PixelTable pixelTable;
   if( IsCustom() ) {
      DIP_THROW_IF( image_.Dimensionality() > nDims, E::DIMENSIONALITIES_DONT_MATCH );
      Image kernel = image_.QuickCopy();
      kernel.ExpandDimensionality( nDims );
      if( kernel.DataType().IsBinary()) {
         DIP_START_STACK_TRACE
            pixelTable = dip::PixelTable{ kernel, {}, procDim };
         DIP_END_STACK_TRACE
      } else {
         DIP_START_STACK_TRACE
            pixelTable = dip::PixelTable{ IsFinite( kernel ), {}, procDim };
            pixelTable.AddWeights( kernel );
         DIP_END_STACK_TRACE
      }
   } else {
      FloatArray sz = params_;
      DIP_START_STACK_TRACE
         ArrayUseParameter( sz, nDims, 1.0 );
         pixelTable = dip::PixelTable{ ShapeString(), sz, procDim };
         if( shape_ == ShapeCode::LEFT_LINE ) {
            IntegerArray shift = pixelTable.Runs()[ 0 ].coordinates; // need to make a copy, since ShiftOrigin modifies it, causing only the first run to be shifted.
            pixelTable.ShiftOrigin( shift );
         }
      DIP_END_STACK_TRACE
   }
   if( !shift_.empty() ) {
      IntegerArray shift = shift_;
      DIP_START_STACK_TRACE
         ArrayUseParameter( shift, nDims, dip::sint( 0 ));
         pixelTable.ShiftOrigin( shift_ );
      DIP_END_STACK_TRACE
   }
   if( mirror_ ) {
      pixelTable.Mirror();
   }
   return pixelTable;
}

dip::uint Kernel::NumberOfPixels( dip::uint nDims ) const {
   return PixelTable( nDims, 0 ).NumberOfPixels();
}

namespace {

void FixUpPixelSizeArray( FloatArray& pixelSize, dip::uint nDims ) {
   dip::uint k = pixelSize.size();
   if(( k > 0 ) && ( k < nDims )) {
      // If we have at least one element coming in, but not nDims, we replicate the last element to all new elements
      pixelSize.resize( nDims, pixelSize[ k - 1 ] );
   } else {
      // Otherwise we just resize: if k == nDims, nothing happens, if k > nDims, array is cropped, if k == 0, it is filled with 1.
      pixelSize.resize( nDims, 1.0 );
   }
}

} // namespace

void NeighborList::ConstructConnectivity( dip::uint dimensionality, dip::uint connectivity, FloatArray pixelSize ) {
   DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity > dimensionality, E::PARAMETER_OUT_OF_RANGE );
   if( connectivity == 0 ) {
      connectivity = dimensionality;
   }
   FixUpPixelSizeArray( pixelSize, dimensionality );
   for( auto& pxsz : pixelSize ) {
      pxsz *= pxsz;
   }
   IntegerArray coords( dimensionality, -1 );
   for( ;; ) {
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

void NeighborList::ConstructChamfer( dip::uint dimensionality, dip::uint maxDistance, FloatArray pixelSize ) {
   DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( maxDistance < 1, E::PARAMETER_OUT_OF_RANGE );
   FixUpPixelSizeArray( pixelSize, dimensionality );
   if( dimensionality == 1 ) {
      neighbors_.push_back( {{ -1 }, pixelSize[ 0 ] } );
      neighbors_.push_back( {{  1 }, pixelSize[ 0 ] } );
      // Other values for maxDistance make no sense -- ignore
      return;
   } else if( dimensionality == 2 ) {
      dfloat dx = pixelSize[ 0 ];
      dfloat dy = pixelSize[ 1 ];
      dfloat dxy = std::hypot( dx, dy );
      if( maxDistance == 1 ) {
         dx *= 0.9481;
         dy *= 0.9481;
         dxy *= 1.3408 / std::sqrt( 2.0 );
         neighbors_.push_back( {{ -1, -1 }, dxy } );
         neighbors_.push_back( {{  0, -1 }, dy } );
         neighbors_.push_back( {{  1, -1 }, dxy } );
         neighbors_.push_back( {{ -1,  0 }, dx } );
         neighbors_.push_back( {{  1,  0 }, dx } );
         neighbors_.push_back( {{ -1,  1 }, dxy } );
         neighbors_.push_back( {{  0,  1 }, dy } );
         neighbors_.push_back( {{  1,  1 }, dxy } );
         return;
      } else if( maxDistance == 2 ) {
         dfloat dxxy = std::hypot( 2 * dx, dy );
         dfloat dxyy = std::hypot( dx, 2 * dy );
         dx *= 0.9801;
         dy *= 0.9801;
         dxy *= 1.4060 / std::sqrt( 2.0 );
         dxxy *= 2.2044 / std::sqrt( 5.0 );
         dxyy *= 2.2044 / std::sqrt( 5.0 );
         neighbors_.push_back( {{ -1, -2 }, dxyy } );
         neighbors_.push_back( {{  1, -2 }, dxyy } );
         neighbors_.push_back( {{ -2, -1 }, dxxy } );
         neighbors_.push_back( {{ -1, -1 }, dxy } );
         neighbors_.push_back( {{  0, -1 }, dy } );
         neighbors_.push_back( {{  1, -1 }, dxy } );
         neighbors_.push_back( {{  2, -1 }, dxxy } );
         neighbors_.push_back( {{ -1,  0 }, dx } );
         neighbors_.push_back( {{  1,  0 }, dx } );
         neighbors_.push_back( {{ -2,  1 }, dxxy } );
         neighbors_.push_back( {{ -1,  1 }, dxy } );
         neighbors_.push_back( {{  0,  1 }, dy } );
         neighbors_.push_back( {{  1,  1 }, dxy } );
         neighbors_.push_back( {{  2,  1 }, dxxy } );
         neighbors_.push_back( {{ -1,  2 }, dxyy } );
         neighbors_.push_back( {{  1,  2 }, dxyy } );
         return;
      }
   } else if( dimensionality == 3 ) {
      dfloat dx = pixelSize[ 0 ];
      dfloat dy = pixelSize[ 1 ];
      dfloat dz = pixelSize[ 2 ];
      dfloat dxy = std::hypot( dx, dy );
      dfloat dxz = std::hypot( dx, dz );
      dfloat dyz = std::hypot( dy, dz );
      dfloat dxyz = std::hypot( dx, dyz );
      if( maxDistance == 1 ) {
         dx *= 0.8939539326;
         dy *= 0.8939539326;
         dz *= 0.8939539326;
         dxy *= 1.340863402 / std::sqrt( 2.0 );
         dxz *= 1.340863402 / std::sqrt( 2.0 );
         dyz *= 1.340863402 / std::sqrt( 2.0 );
         dxyz *= 1.587920248 / std::sqrt( 3.0 );
         neighbors_.push_back( {{ -1, -1, -1}, dxyz } );
         neighbors_.push_back( {{  0, -1, -1}, dyz } );
         neighbors_.push_back( {{  1, -1, -1}, dxyz } );
         neighbors_.push_back( {{ -1,  0, -1}, dxz } );
         neighbors_.push_back( {{  0,  0, -1}, dz } );
         neighbors_.push_back( {{  1,  0, -1}, dxz } );
         neighbors_.push_back( {{ -1,  1, -1}, dxyz } );
         neighbors_.push_back( {{  0,  1, -1}, dyz } );
         neighbors_.push_back( {{  1,  1, -1}, dxyz } );
         neighbors_.push_back( {{ -1, -1,  0}, dxy } );
         neighbors_.push_back( {{  0, -1,  0}, dy } );
         neighbors_.push_back( {{  1, -1,  0}, dxy } );
         neighbors_.push_back( {{ -1,  0,  0}, dx } );
         neighbors_.push_back( {{  1,  0,  0}, dx } );
         neighbors_.push_back( {{ -1,  1,  0}, dxy } );
         neighbors_.push_back( {{  0,  1,  0}, dy } );
         neighbors_.push_back( {{  1,  1,  0}, dxy } );
         neighbors_.push_back( {{ -1, -1,  1}, dxyz } );
         neighbors_.push_back( {{  0, -1,  1}, dyz } );
         neighbors_.push_back( {{  1, -1,  1}, dxyz } );
         neighbors_.push_back( {{ -1,  0,  1}, dxz } );
         neighbors_.push_back( {{  0,  0,  1}, dz } );
         neighbors_.push_back( {{  1,  0,  1}, dxz } );
         neighbors_.push_back( {{ -1,  1,  1}, dxyz } );
         neighbors_.push_back( {{  0,  1,  1}, dyz } );
         neighbors_.push_back( {{  1,  1,  1}, dxyz } );
         return;
      } else if( maxDistance == 2 ) {
         dfloat dxyy = std::hypot( dx, 2 * dy );
         dfloat dxzz = std::hypot( dx, 2 * dz );
         dfloat dxxy = std::hypot( dy, 2 * dx );
         dfloat dyzz = std::hypot( dy, 2 * dz );
         dfloat dxxz = std::hypot( dz, 2 * dx );
         dfloat dyyz = std::hypot( dz, 2 * dy );
         dfloat dxxyz = std::hypot( 2 * dx, dyz );
         dfloat dxyyz = std::hypot( 2 * dy, dxz );
         dfloat dxyzz = std::hypot( 2 * dz, dxy );
         dfloat dxxyyz = std::hypot( 2 * dx, dyyz );
         dfloat dxxyzz = std::hypot( 2 * dx, dyzz );
         dfloat dxyyzz = std::hypot( 2 * dz, dxyy );
         dx *= 0.9556;
         dy *= 0.9556;
         dz *= 0.9556;
         dxy *= 1.3956 / std::sqrt( 2.0 );
         dxz *= 1.3956 / std::sqrt( 2.0 );
         dyz *= 1.3956 / std::sqrt( 2.0 );
         dxyz *= 1.7257 / std::sqrt( 3.0 );
         dxyy *= 2.1830 / std::sqrt( 5.0 );
         dxzz *= 2.1830 / std::sqrt( 5.0 );
         dxxy *= 2.1830 / std::sqrt( 5.0 );
         dyzz *= 2.1830 / std::sqrt( 5.0 );
         dxxz *= 2.1830 / std::sqrt( 5.0 );
         dyyz *= 2.1830 / std::sqrt( 5.0 );
         dxxyz *= 2.3885 / std::sqrt( 6.0 );
         dxyyz *= 2.3885 / std::sqrt( 6.0 );
         dxyzz *= 2.3885 / std::sqrt( 6.0 );
         dxxyyz *= 2.9540 / std::sqrt( 9.0 );
         dxxyzz *= 2.9540 / std::sqrt( 9.0 );
         dxyyzz *= 2.9540 / std::sqrt( 9.0 );
         neighbors_.push_back( {{ -1, -2, -2 }, dxyyzz } );
         neighbors_.push_back( {{  1, -2, -2 }, dxyyzz } );
         neighbors_.push_back( {{ -2, -1, -2 }, dxxyzz } );
         neighbors_.push_back( {{ -1, -1, -2 }, dxyzz } );
         neighbors_.push_back( {{  0, -1, -2 }, dyzz } );
         neighbors_.push_back( {{  1, -1, -2 }, dxyzz } );
         neighbors_.push_back( {{  2, -1, -2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  0, -2 }, dxzz } );
         neighbors_.push_back( {{  1,  0, -2 }, dxzz } );
         neighbors_.push_back( {{ -2,  1, -2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  1, -2 }, dxyzz } );
         neighbors_.push_back( {{  0,  1, -2 }, dyzz } );
         neighbors_.push_back( {{  1,  1, -2 }, dxyzz } );
         neighbors_.push_back( {{  2,  1, -2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  2, -2 }, dxyyzz } );
         neighbors_.push_back( {{  1,  2, -2 }, dxyyzz } );
         neighbors_.push_back( {{ -2, -2, -1 }, dxxyyz } );
         neighbors_.push_back( {{ -1, -2, -1 }, dxyyz } );
         neighbors_.push_back( {{  0, -2, -1 }, dyyz } );
         neighbors_.push_back( {{  1, -2, -1 }, dxyyz } );
         neighbors_.push_back( {{  2, -2, -1 }, dxxyyz } );
         neighbors_.push_back( {{ -2, -1, -1 }, dxxyz } );
         neighbors_.push_back( {{ -1, -1, -1 }, dxyz } );
         neighbors_.push_back( {{  0, -1, -1 }, dyz } );
         neighbors_.push_back( {{  1, -1, -1 }, dxyz } );
         neighbors_.push_back( {{  2, -1, -1 }, dxxyz } );
         neighbors_.push_back( {{ -2,  0, -1 }, dxxz } );
         neighbors_.push_back( {{ -1,  0, -1 }, dxz } );
         neighbors_.push_back( {{  0,  0, -1 }, dz } );
         neighbors_.push_back( {{  1,  0, -1 }, dxz } );
         neighbors_.push_back( {{  2,  0, -1 }, dxxz } );
         neighbors_.push_back( {{ -2,  1, -1 }, dxxyz } );
         neighbors_.push_back( {{ -1,  1, -1 }, dxyz } );
         neighbors_.push_back( {{  0,  1, -1 }, dyz } );
         neighbors_.push_back( {{  1,  1, -1 }, dxyz } );
         neighbors_.push_back( {{  2,  1, -1 }, dxxyz } );
         neighbors_.push_back( {{ -2,  2, -1 }, dxxyyz } );
         neighbors_.push_back( {{ -1,  2, -1 }, dxyyz } );
         neighbors_.push_back( {{  0,  2, -1 }, dyyz } );
         neighbors_.push_back( {{  1,  2, -1 }, dxyyz } );
         neighbors_.push_back( {{  2,  2, -1 }, dxxyyz } );
         neighbors_.push_back( {{ -1, -2,  0 }, dxyy } );
         neighbors_.push_back( {{  1, -2,  0 }, dxyy } );
         neighbors_.push_back( {{ -2, -1,  0 }, dxxy } );
         neighbors_.push_back( {{ -1, -1,  0 }, dxy } );
         neighbors_.push_back( {{  0, -1,  0 }, dy } );
         neighbors_.push_back( {{  1, -1,  0 }, dxy } );
         neighbors_.push_back( {{  2, -1,  0 }, dxxy } );
         neighbors_.push_back( {{ -1,  0,  0 }, dx } );
         neighbors_.push_back( {{  1,  0,  0 }, dx } );
         neighbors_.push_back( {{ -2,  1,  0 }, dxxy } );
         neighbors_.push_back( {{ -1,  1,  0 }, dxy } );
         neighbors_.push_back( {{  0,  1,  0 }, dy } );
         neighbors_.push_back( {{  1,  1,  0 }, dxy } );
         neighbors_.push_back( {{  2,  1,  0 }, dxxy } );
         neighbors_.push_back( {{ -1,  2,  0 }, dxyy } );
         neighbors_.push_back( {{  1,  2,  0 }, dxyy } );
         neighbors_.push_back( {{ -2, -2,  1 }, dxxyyz } );
         neighbors_.push_back( {{ -1, -2,  1 }, dxyyz } );
         neighbors_.push_back( {{  0, -2,  1 }, dyyz } );
         neighbors_.push_back( {{  1, -2,  1 }, dxyyz } );
         neighbors_.push_back( {{  2, -2,  1 }, dxxyyz } );
         neighbors_.push_back( {{ -2, -1,  1 }, dxxyz } );
         neighbors_.push_back( {{ -1, -1,  1 }, dxyz } );
         neighbors_.push_back( {{  0, -1,  1 }, dyz } );
         neighbors_.push_back( {{  1, -1,  1 }, dxyz } );
         neighbors_.push_back( {{  2, -1,  1 }, dxxyz } );
         neighbors_.push_back( {{ -2,  0,  1 }, dxxz } );
         neighbors_.push_back( {{ -1,  0,  1 }, dxz } );
         neighbors_.push_back( {{  0,  0,  1 }, dz } );
         neighbors_.push_back( {{  1,  0,  1 }, dxz } );
         neighbors_.push_back( {{  2,  0,  1 }, dxxz } );
         neighbors_.push_back( {{ -2,  1,  1 }, dxxyz } );
         neighbors_.push_back( {{ -1,  1,  1 }, dxyz } );
         neighbors_.push_back( {{  0,  1,  1 }, dyz } );
         neighbors_.push_back( {{  1,  1,  1 }, dxyz } );
         neighbors_.push_back( {{  2,  1,  1 }, dxxyz } );
         neighbors_.push_back( {{ -2,  2,  1 }, dxxyyz } );
         neighbors_.push_back( {{ -1,  2,  1 }, dxyyz } );
         neighbors_.push_back( {{  0,  2,  1 }, dyyz } );
         neighbors_.push_back( {{  1,  2,  1 }, dxyyz } );
         neighbors_.push_back( {{  2,  2,  1 }, dxxyyz } );
         neighbors_.push_back( {{ -1, -2,  2 }, dxyyzz } );
         neighbors_.push_back( {{  1, -2,  2 }, dxyyzz } );
         neighbors_.push_back( {{ -2, -1,  2 }, dxxyzz } );
         neighbors_.push_back( {{ -1, -1,  2 }, dxyzz } );
         neighbors_.push_back( {{  0, -1,  2 }, dyzz } );
         neighbors_.push_back( {{  1, -1,  2 }, dxyzz } );
         neighbors_.push_back( {{  2, -1,  2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  0,  2 }, dxzz } );
         neighbors_.push_back( {{  1,  0,  2 }, dxzz } );
         neighbors_.push_back( {{ -2,  1,  2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  1,  2 }, dxyzz } );
         neighbors_.push_back( {{  0,  1,  2 }, dyzz } );
         neighbors_.push_back( {{  1,  1,  2 }, dxyzz } );
         neighbors_.push_back( {{  2,  1,  2 }, dxxyzz } );
         neighbors_.push_back( {{ -1,  2,  2 }, dxyyzz } );
         neighbors_.push_back( {{  1,  2,  2 }, dxyyzz } );
         return;
      }
   }
   // Higher dimensions, or maxDistance != 1, 2
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

void NeighborList::ConstructImage( dip::uint dimensionality, Image const& c_metric ) {
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

namespace {

bool IsProcessed( IntegerArray const& coords, dip::uint procDim ) {
   dip::uint ii = coords.size();
   while( ii > 0 ) {
      --ii;
      if( ii != procDim ) {
         if( coords[ ii ] > 0 ) {
            return false;
         } else if( coords[ ii ] < 0 ) {
            return true;
         } // If 0, it depends on a previous coordinate.
      }
   }
   return coords[ procDim ] < 0;
   // Note that coords[ procDim ] will not be 0 here, as coords={0,0,0,...} is never a part of the neighborhood.
}

} // namespace

NeighborList NeighborList::SelectBackward( dip::uint procDim ) const {
   if( procDim >= neighbors_[ 0 ].coords.size() ) {
      procDim = 0;
   }
   NeighborList out;
   for( auto& neighbor : neighbors_ ) {
      if( IsProcessed( neighbor.coords, procDim )) {
         out.neighbors_.push_back( neighbor );
      }
   }
   return out;
}

NeighborList NeighborList::SelectForward( dip::uint procDim ) const {
   if( procDim >= neighbors_[ 0 ].coords.size() ) {
      procDim = 0;
   }
   NeighborList out;
   for( auto& neighbor : neighbors_ ) {
      if( !IsProcessed( neighbor.coords, procDim )) {
         out.neighbors_.push_back( neighbor );
      }
   }
   return out;
}

} // namespace dip


#ifdef DIP__ENABLE_DOCTEST

DOCTEST_TEST_CASE("[DIPlib] testing the NeighborList class") {
   dip::dfloat x = 1.2;
   dip::dfloat y = 1.6;
   dip::dfloat diag = std::hypot( x, y );
   dip::dfloat horse_v = std::hypot( x, 2*y );
   dip::dfloat horse_h = std::hypot( 2*x, y );
   dip::PixelSize pxsz{ dip::PhysicalQuantityArray{ x * dip::Units::Meter(), y * dip::Units::Meter() }};
   dip::NeighborList list( dip::Metric( "connected", 2, pxsz ), 2 );
   DOCTEST_REQUIRE( list.Size() == 8 );
   auto it = list.begin();
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( diag ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( y ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( diag ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( x ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( x ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( diag ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( y ));
   DOCTEST_CHECK( *( it++ ) == doctest::Approx( diag ));
   dip::IntegerArray strides{ 1, 10 };
   dip::IntegerArray offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 8 );
   auto ot = offsets.begin();
   DOCTEST_CHECK( *( ot++ ) == -1 -10 );
   DOCTEST_CHECK( *( ot++ ) == +0 -10 );
   DOCTEST_CHECK( *( ot++ ) == +1 -10 );
   DOCTEST_CHECK( *( ot++ ) == -1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == +1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == -1 +10 );
   DOCTEST_CHECK( *( ot++ ) == +0 +10 );
   DOCTEST_CHECK( *( ot++ ) == +1 +10 );

   list = dip::NeighborList( dip::Metric( "chamfer", 2, pxsz ), 2 );
   DOCTEST_REQUIRE( list.Size() == 16 );
   it = list.begin();
   DOCTEST_CHECK( std::abs( horse_v - *( it++ )) < 0.1 ); // Note chamfer metric is always slightly smaller than Euclidean one
   DOCTEST_CHECK( std::abs( horse_v - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_h - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( diag    - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( y       - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( diag    - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_h - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( x       - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( x       - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_h - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( diag    - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( y       - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( diag    - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_h - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_v - *( it++ )) < 0.1 );
   DOCTEST_CHECK( std::abs( horse_v - *( it++ )) < 0.1 );
   offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 16 );
   ot = offsets.begin();
   DOCTEST_CHECK( *( ot++ ) == -1 -20 );
   DOCTEST_CHECK( *( ot++ ) == +1 -20 );
   DOCTEST_CHECK( *( ot++ ) == -2 -10 );
   DOCTEST_CHECK( *( ot++ ) == -1 -10 );
   DOCTEST_CHECK( *( ot++ ) == +0 -10 );
   DOCTEST_CHECK( *( ot++ ) == +1 -10 );
   DOCTEST_CHECK( *( ot++ ) == +2 -10 );
   DOCTEST_CHECK( *( ot++ ) == -1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == +1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == -2 +10 );
   DOCTEST_CHECK( *( ot++ ) == -1 +10 );
   DOCTEST_CHECK( *( ot++ ) == +0 +10 );
   DOCTEST_CHECK( *( ot++ ) == +1 +10 );
   DOCTEST_CHECK( *( ot++ ) == +2 +10 );
   DOCTEST_CHECK( *( ot++ ) == -1 +20 );
   DOCTEST_CHECK( *( ot++ ) == +1 +20 );

   dip::Image m( { 3, 3 }, 1, dip::DT_UINT8 );
   dip::uint8* ptr = ( dip::uint8* )m.Origin();
   for( dip::uint ii = 1; ii <= 9; ++ii ) {
      *( ptr++ ) = ( dip::uint8 )ii;
   }
   ptr[ -5 ] = 0;
   list = dip::NeighborList( m, 2 );
   DOCTEST_REQUIRE( list.Size() == 8 );
   it = list.begin();
   DOCTEST_CHECK( *( it++ ) == 1 );
   DOCTEST_CHECK( *( it++ ) == 2 );
   DOCTEST_CHECK( *( it++ ) == 3 );
   DOCTEST_CHECK( *( it++ ) == 4 );
   DOCTEST_CHECK( *( it++ ) == 6 );
   DOCTEST_CHECK( *( it++ ) == 7 );
   DOCTEST_CHECK( *( it++ ) == 8 );
   DOCTEST_CHECK( *( it++ ) == 9 );
   offsets = list.ComputeOffsets( strides );
   DOCTEST_REQUIRE( offsets.size() == 8 );
   ot = offsets.begin();
   DOCTEST_CHECK( *( ot++ ) == -1 -10 );
   DOCTEST_CHECK( *( ot++ ) == +0 -10 );
   DOCTEST_CHECK( *( ot++ ) == +1 -10 );
   DOCTEST_CHECK( *( ot++ ) == -1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == +1 + 0 );
   DOCTEST_CHECK( *( ot++ ) == -1 +10 );
   DOCTEST_CHECK( *( ot++ ) == +0 +10 );
   DOCTEST_CHECK( *( ot++ ) == +1 +10 );
}

#endif // DIP__ENABLE_DOCTEST
