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

#include "diplib/neighborhood.h"
#include "diplib/iterators.h"
#include "diplib/math.h"

namespace dip {

dip::PixelTable Kernel::PixelTable( UnsignedArray const& imsz, dip::uint procDim ) const {
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

void NeighborList::ConstructConnectivity( dip::uint dimensionality, dip::uint connectivity, FloatArray pixelSize ) {
   DIP_THROW_IF( dimensionality < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity > dimensionality, E::PARAMETER_OUT_OF_RANGE );
   if( connectivity == 0 ) {
      connectivity = dimensionality;
   }
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
   ImageIterator <dfloat> it( metric );
   do {
      if( *it > 0 ) {
         IntegerArray coords{ it.Coordinates() };
         coords -= offset;
         DIP_THROW_IF( !coords.any(), "Metric image must have a distance of 0 in the middle" );
         neighbors_.push_back( { coords, *it } );
      }
   } while( ++it );
}

} // namespace dip
