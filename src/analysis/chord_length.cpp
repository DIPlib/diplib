/*
 * DIPlib 3.0
 * This file contains the definition for dip::ChordLength
 *
 * (c)2018, Cris Luengo.
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

#include "diplib.h"
#include "diplib/private/robin_map.h"
#include "diplib/analysis.h"
#include "diplib/regions.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"
#include "diplib/random.h"

namespace dip {

namespace {

using UIntPixelValueReaderFunction = dip::uint ( * )( void const* );

template< typename TPI >
static dip::uint UIntPixelValueReader( void const* data ) {
   return static_cast< dip::uint >( *static_cast< TPI const* >( data ));
}

using PhaseLookupTable = tsl::robin_map< dip::uint, dip::uint >;

void UpdateDistribution(
      Distribution& distribution,
      std::vector< dip::uint >& counts,
      PhaseLookupTable const& phaseLookupTable,
      dip::uint phase,
      dip::uint length

) {
   if(( length > 0 ) && ( length - 1 < distribution.Size() )) {
      // We are interested in the phase of the points
      dip::uint index = phaseLookupTable.at( phase );
      distribution[ length - 1 ].Y( index ) += 1.0;
      ++( counts[ index ] );
   }
}

void RandomPixelPairSampler(
      Image const& object,
      Image const& mask,
      Distribution& distribution,
      std::vector< dip::uint >& counts,
      PhaseLookupTable const& phaseLookupTable,
      dip::uint nProbes
) {
   UIntPixelValueReaderFunction GetUIntPixelValue;
   DIP_OVL_ASSIGN_UINT( GetUIntPixelValue, UIntPixelValueReader, object.DataType() );
   bool hasMask = mask.IsForged();
   Random random( 0 );
   UniformRandomGenerator uniformRandomGenerator( random );
   GaussianRandomGenerator normalRandomGenerator( random );
   dip::uint nDims = object.Dimensionality();
   UnsignedArray const& sizes = object.Sizes();
   FloatArray maxpos{ sizes };   // upper limit for coordinates
   maxpos -= 1;
   FloatArray origin( nDims );
   FloatArray direction( nDims, 1 );
   UnsignedArray pointInt( nDims );
   FloatArray pointFloat( nDims );
   for( dip::uint probe = 0; probe < nProbes; ++probe ) { // TODO: trivially parallelizable.
      // A point
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = uniformRandomGenerator( 0, maxpos[ ii ] );
      }
      // A direction
      if( nDims == 2 ) {
         // This is the easy case
         dfloat phi = uniformRandomGenerator( 0, 2 * pi );
         direction[ 0 ] = cos( phi );
         direction[ 1 ] = sin( phi );
      } else if( nDims == 3 ) {
         // https://math.stackexchange.com/a/44691/414894
         // http://mathworld.wolfram.com/SpherePointPicking.html
         dfloat phi = uniformRandomGenerator( 0, 2 * pi );
         dfloat z = uniformRandomGenerator( -1, 1 );
         dfloat u = std::sqrt( 1 - z * z );
         direction[ 0 ] = u * cos( phi );
         direction[ 1 ] = u * sin( phi );
         direction[ 2 ] = z;
      } else if( nDims > 3 ) {
         // Pick a normally distributed point and normalize
         dfloat norm = 0;
         do {
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               direction[ ii ] = normalRandomGenerator( 0, 1 );
               norm += direction[ ii ] * direction[ ii ];
            }
         } while( norm == 0 ); // highly unlikely, but we need to do this anway
         norm = std::sqrt( norm );
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            direction[ ii ] /= norm;
         }
      } // else : ( nDims == 1 ) : direction is always 1
      // Given a point and a direction, find the two points where this line crosses the image boundary
      bool first = true;
      dfloat distanceBegin = 0;
      dfloat distanceEnd = 0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         // We're sure at least one direction[ii] is not zero
         if( direction[ ii ] != 0 ) {
            dfloat distB, distE;
            if( direction[ ii ] > 0 ) {
               distB = ( origin[ ii ] ) / direction[ ii ];
               distE = ( maxpos[ ii ] - origin[ ii ] ) / direction[ ii ];
            } else {
               distB = ( maxpos[ ii ] - origin[ ii ] ) / -direction[ ii ];
               distE = ( -origin[ ii ] ) / direction[ ii ];
            }
            distanceBegin = first ? distB : std::min( distB, distanceBegin );
            distanceEnd = first ? distE : std::min( distE, distanceEnd );
            first = false;
         }
      }
      dfloat totalLength = 0.0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         double end = origin[ ii ] + direction[ ii ] * distanceEnd;
         double begin = origin[ ii ] - direction[ ii ] * distanceBegin;
         DIP_ASSERT( end >= -0.499 );
         DIP_ASSERT( end <= maxpos[ ii ] + 0.499 );
         DIP_ASSERT( begin >= -0.499 );
         DIP_ASSERT( begin <= maxpos[ ii ] + 0.499 );
         pointFloat[ ii ] = begin;
         pointInt[ ii ] = static_cast< dip::uint >( std::round( begin ));
         dfloat dist = end - begin;
         totalLength += dist * dist;
      }
      totalLength = sqrt( totalLength );
      dip::uint totalLengthInt = static_cast< dip::uint >( totalLength );
      // Walk along this line and find phase changes
      dip::uint d1 = GetUIntPixelValue( object.Pointer( pointInt ));
      bin m1 = hasMask ? *static_cast< bin const* >( mask.Pointer( pointInt )) : bin( true );
      dip::uint d2 = d1;
      bin m2 = m1;
      dip::uint length = 0;
      for( dip::uint rr = 0; rr < totalLengthInt; ++rr ) {
         // We want to measure the len of the line in the same phase, in the same object
         if( d2 == d1 && m2 == m1 ) {
            ++length;
         } else {
            UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
            // Only count chord length inside a masked area
            length = ( m1 ? 1 : 0 );
         }
         // Update to the next point on the line by adding direction to pointFloat and rounding it to nearest integer point
         d2 = d1;
         m2 = m1;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            pointFloat[ ii ] += direction[ ii ];
            DIP_ASSERT( pointFloat[ ii ] >= -0.499 );
            DIP_ASSERT( pointFloat[ ii ] <= maxpos[ ii ] + 0.499 );
            pointInt[ ii ] = static_cast< dip::uint >( std::round( pointFloat[ ii ] ));
         }
         d1 = GetUIntPixelValue( object.Pointer( pointInt ));
         m1 = hasMask ? *static_cast< bin const* >( mask.Pointer( pointInt )) : bin( true );
      }
      UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
   }
}

void GridPixelPairSampler(
      Image const& object,
      Image const& mask,
      Distribution& distribution,
      std::vector< dip::uint >& counts,
      PhaseLookupTable const& phaseLookupTable,
      dip::uint nProbes
) {
   UIntPixelValueReaderFunction GetUIntPixelValue;
   DIP_OVL_ASSIGN_UINT( GetUIntPixelValue, UIntPixelValueReader, object.DataType() );
   bool hasMask = mask.IsForged();
   dip::uint nDims = object.Dimensionality();
   UnsignedArray coords( nDims );
   dip::uint step = 1; // how many lines to skip
   if( nProbes > 0 ) {
      dip::uint nLines = 0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         UnsignedArray lines = coords;
         lines[ ii ] = 1;
         nLines += lines.product();
      }
      step = div_floor( nLines, nProbes );
      step = std::max< dip::uint >( step, 1 ); // step must be at least 1.
   }
   // Iterate over image dimensions
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      GenericJointImageIterator< 2 > it( { object, mask }, dim );
      dip::uint size = it.ProcessingDimensionSize();
      dip::sint dataStride = it.ProcessingDimensionStride< 0 >() * static_cast< dip::sint >( object.DataType().SizeOf() );
      dip::sint maskStride = hasMask ? it.ProcessingDimensionStride< 1 >() : 0;
      // Iterate over `fraction` image lines
      do {
         void const* dataPtr = it.Pointer< 0 >();
         bin const* maskPtr = hasMask ? static_cast< bin const* >( it.Pointer< 1 >() ) : nullptr;
         // Walk along this line and find phase changes
         dip::uint d1 = GetUIntPixelValue( dataPtr );
         bin m1 = hasMask ? *maskPtr : bin( true );
         dip::uint d2 = d1;
         bin m2 = m1;
         dip::uint length = 0;
         for( dip::uint rr = 0; rr < size; ++rr ) {
            // We want to measure the len of the line in the same phase, in the same object
            if( d2 == d1 && m2 == m1 ) {
               ++length;
            } else {
               UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
               // Only count chord length inside a masked area
               length = ( m1 ? 1 : 0 );
            }
            // Update to the next point on the line
            d2 = d1;
            m2 = m1;
            dataPtr = static_cast< uint8 const* >( dataPtr ) + dataStride;
            maskPtr += maskStride;
            d1 = GetUIntPixelValue( dataPtr );
            m1 = hasMask ? *maskPtr : bin( true );
         }
         UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
         for( dip::uint ii = 0; ( ii < step ) && it; ++ii ) {
            ++it; // TODO: this does not skip appropriately in 3D and higher dims.
         }
      } while( it );
   }
}

} // namespace

Distribution ChordLength(
      Image const& c_object,
      Image const& mask,
      dip::uint probes,
      dip::uint length,
      String const& sampling
) {
   DIP_THROW_IF( !c_object.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_object.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_object.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( c_object.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image object = c_object.QuickCopy();
   if( object.DataType().IsBinary() ) {
      object.Convert( DT_UINT8 );
   }
   UnsignedArray phases;
   DIP_STACK_TRACE_THIS( phases = GetObjectLabels( object, mask, S::INCLUDE )); // Will test mask for us -- doesn't allow singleton expansion, so we don't need to here either
   PhaseLookupTable phaseLookupTable;
   for( dip::uint ii = 0; ii < phases.size(); ++ii ) {
      phaseLookupTable.emplace( phases[ ii ], ii );
   }

   // Parse options
   bool random;
   DIP_STACK_TRACE_THIS( random = BooleanFromString( sampling, S::RANDOM, S::GRID ));

   // Create output
   dip::uint nPhases = phases.size();
   Distribution distribution( length, nPhases );
   distribution.SetSampling( c_object.PixelSize(), 1 );
   std::vector< dip::uint >counts( nPhases, 0 );

   // Fill output
   if( random ) {
      RandomPixelPairSampler( object, mask, distribution, counts, phaseLookupTable, probes );
   } else {
      GridPixelPairSampler( object, mask, distribution, counts, phaseLookupTable, probes );
   }

   // Process the intermediate output results
   for( dip::uint ii = 0; ii < nPhases; ++ii ) {
      dfloat count = static_cast< dfloat >( counts[ ii ] );
      for( auto it = distribution.Ybegin( ii ); it != distribution.Yend( ii ); ++it ) {
         *it /= count;
      }
   }

   return distribution;
}

} // namespace dip
