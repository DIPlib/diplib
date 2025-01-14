/*
 * (c)2018-2024, Cris Luengo.
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

#include "diplib/analysis.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "diplib.h"
#include "diplib/distribution.h"
#include "diplib/generic_iterators.h"
#include "diplib/geometry.h"
#include "diplib/multithreading.h"
#include "diplib/overload.h"
#include "diplib/random.h"
#include "diplib/regions.h"
#include "diplib/private/robin_map.h"

namespace dip {

namespace {

using UIntPixelValueReaderFunction = dip::uint ( * )( void const* );

template< typename TPI >
dip::uint UIntPixelValueReader( void const* data ) {
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
      Random& random,
      Distribution& distribution,
      std::vector< dip::uint >& counts,
      PhaseLookupTable const& phaseLookupTable,
      dip::uint nProbes
) {
   UIntPixelValueReaderFunction GetUIntPixelValue{};
   DIP_OVL_ASSIGN_UINT( GetUIntPixelValue, UIntPixelValueReader, object.DataType() );
   bool hasMask = mask.IsForged();
   dip::uint nDims = object.Dimensionality();
   // Multithreading
   dip::uint nThreads = GetNumberOfThreads();
   if( nProbes < 10 * nThreads ) {
      // If there's not enough work per thread, don't start threads
      // NOTE! Hard-coded threshold, seems to work fine on my particular machine...
      // TODO: this threshold probably also depends on the image size.
      nThreads = 1;
   }
   std::vector< Distribution > threadDistributions( nThreads, distribution );
   std::vector< std::vector< dip::uint >> threadCounts( nThreads, counts );
   // Create random generators for each thread
   std::vector< Random > randomArray( nThreads - 1, random );
   std::vector< UniformRandomGenerator > uniformRandomGeneratorArray;
   std::vector< GaussianRandomGenerator > normalRandomGeneratorArray;
   uniformRandomGeneratorArray.emplace_back( random );
   if( nDims > 3 ) {
      normalRandomGeneratorArray.emplace_back( random );
   }
   for( dip::uint ii = 1; ii < nThreads; ++ii ) {
      randomArray[ ii - 1 ].SetStream( random() ); // Using a random value from the original stream. This is the same as random.Split().
      uniformRandomGeneratorArray.emplace_back( randomArray[ ii - 1 ] );
      if( nDims > 3 ) {
         normalRandomGeneratorArray.emplace_back( randomArray[ ii - 1 ] );
      }
   }
   //
   UnsignedArray const& sizes = object.Sizes();
   FloatArray maxpos{ sizes };   // upper limit for coordinates
   maxpos -= 1;
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
      nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
      #pragma omp barrier
      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );
      UniformRandomGenerator& uniformRandomGenerator = uniformRandomGeneratorArray[ thread ];
      FloatArray origin( nDims );
      FloatArray direction( nDims, 1 );
      UnsignedArray pointInt( nDims );
      FloatArray pointFloat( nDims );
      for( dip::uint probe = 0; probe < nProbes / nThreads; ++probe ) {
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
            GaussianRandomGenerator& normalRandomGenerator = normalRandomGeneratorArray[ thread ];
            dfloat norm{};
            do {
               norm = 0;
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
               dfloat distB = origin[ ii ] / direction[ ii ];
               dfloat distE = ( maxpos[ ii ] - origin[ ii ] ) / direction[ ii ];
               if( direction[ ii ] < 0 ) {
                  std::swap( distB, distE );
                  distB = -distB;
                  distE = -distE;
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
         dip::uint d2 = GetUIntPixelValue( object.Pointer( pointInt ));
         bin m2 = hasMask ? *static_cast< bin const* >( mask.Pointer( pointInt )) : bin( true );
         dip::uint length = 1;
         for( dip::uint rr = 1; rr < totalLengthInt; ++rr ) {
            // Next point on the line by adding direction to pointFloat and rounding it to nearest integer point
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               pointFloat[ ii ] += direction[ ii ];
               DIP_ASSERT( pointFloat[ ii ] >= -0.499 );
               DIP_ASSERT( pointFloat[ ii ] <= maxpos[ ii ] + 0.499 );
               pointInt[ ii ] = static_cast< dip::uint >( std::round( pointFloat[ ii ] ));
            }
            dip::uint d1 = GetUIntPixelValue( object.Pointer( pointInt ));
            dip::uint m1 = hasMask ? *static_cast< bin const* >( mask.Pointer( pointInt )) : bin( true );
            // We want to measure the len of the line in the same phase, in the same object
            if( d2 == d1 && m2 == m1 ) {
               ++length;
            } else {
               if( m2 ) { // Only count chord length inside a masked area
                  UpdateDistribution( threadDistributions[ thread ], threadCounts[ thread ], phaseLookupTable, d2, length );
               }
               d2 = d1;
               m2 = m1;
               length = 1;
            }
         }
         if( m2 ) { // Only count chord length inside a masked area
            UpdateDistribution( threadDistributions[ thread ], threadCounts[ thread ], phaseLookupTable, d2, length );
         }
      }
   DIP_PARALLEL_ERROR_END
   // Collect data from threads into output
   distribution = threadDistributions[ 0 ];
   std::copy( threadCounts[ 0 ].begin(), threadCounts[ 0 ].end(), counts.begin() );
   for( dip::uint ii = 1; ii < threadDistributions.size(); ++ii ) {
      distribution += threadDistributions[ ii ];
      std::transform( counts.begin(), counts.end(), threadCounts[ ii ].begin(), counts.begin(), std::plus<>() );
   }
}

void GridPixelPairSampler(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      Distribution& distribution,
      std::vector< dip::uint >& counts,
      PhaseLookupTable const& phaseLookupTable,
      dip::uint nProbes
) {
   UIntPixelValueReaderFunction GetUIntPixelValue{};
   DIP_OVL_ASSIGN_UINT( GetUIntPixelValue, UIntPixelValueReader, object.DataType() );
   bool hasMask = mask.IsForged();
   dip::uint nDims = object.Dimensionality();
   dip::uint step = 1; // same step size along all dimensions
   if( nProbes > 0 ) {
      // The number of probes is computed as the number of lines across the image, along each dimension,
      // obtained when sampling once every `step` pixels along every dimension.
      dfloat stepLength = 0;
      for( dip::uint dim = 0; dim < nDims; ++dim ) {
         stepLength += 1.0 / static_cast< dfloat >( object.Size( dim ));
      }
      stepLength *= static_cast< dfloat >( object.NumberOfPixels() ) / static_cast< dfloat >( nProbes );
      stepLength = std::pow( stepLength, 1.0 / static_cast< dfloat >( nDims - 1 ));
      stepLength = std::max( std::round( stepLength ), 1.0 ); // step must be at least 1
      step = static_cast< dip::uint >( stepLength );
   }
   dip::Image stepObject = ( step > 1 ) ? Subsampling( object, { step } ) : object.QuickCopy();
   dip::Image stepMask = ( hasMask && step > 1 ) ? Subsampling( mask, { step } ) : mask.QuickCopy();
   // Iterate over image dimensions
   // TODO: parallelize the two loops below. Should be easy, but we need to step away from the image iterator, unfortunately.
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      // Iterate over subsampled image with processing dimension.
      // This leads us to the start of each image line on the grid.
      GenericJointImageIterator< 2 > it( { stepObject, stepMask }, dim );
      dip::uint size = object.Size( dim );
      dip::sint dataStride = object.Stride( dim ) * static_cast< dip::sint >( object.DataType().SizeOf() );
      dip::sint maskStride = hasMask ? mask.Stride( dim ) : 0;
      do {
         uint8 const* dataPtr = static_cast< uint8 const* >( it.Pointer< 0 >() );
         bin const* maskPtr = hasMask ? static_cast< bin const* >( it.Pointer< 1 >() ) : nullptr;
         // Walk along this line and find phase changes
         dip::uint d2 = GetUIntPixelValue( dataPtr );
         bin m2 = hasMask ? *maskPtr : bin( true );
         dip::uint length = 1;
         for( dip::uint rr = 1; rr < size; ++rr ) {
            // Next point on the line
            dataPtr += dataStride;
            maskPtr += maskStride;
            dip::uint d1 = GetUIntPixelValue( dataPtr );
            bin m1 = hasMask ? *maskPtr : bin( true );
            // We want to measure the len of the line in the same phase, in the same object
            if( d2 == d1 && m2 == m1 ) {
               ++length;
            } else {
               if( m2 ) { // Only count chord length inside a masked area
                  UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
               }
               d2 = d1;
               m2 = m1;
               length = 1;
            }
         }
         if( m2 ) { // Only count chord length inside a masked area
            UpdateDistribution( distribution, counts, phaseLookupTable, d2, length );
         }
      } while( ++it );
   }
}

} // namespace

Distribution ChordLength(
      Image const& c_object,
      Image const& mask,
      Random& random,
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
   std::vector< LabelType > phases;
   DIP_STACK_TRACE_THIS( phases = ListObjectLabels( object, mask, S::INCLUDE )); // Will test mask for us -- doesn't allow singleton expansion, so we don't need to here either
   PhaseLookupTable phaseLookupTable;
   for( dip::uint ii = 0; ii < phases.size(); ++ii ) {
      phaseLookupTable.emplace( phases[ ii ], ii );
   }

   // Parse options
   bool useRandom{};
   DIP_STACK_TRACE_THIS( useRandom = BooleanFromString( sampling, S::RANDOM, S::GRID ));

   // Create output
   dip::uint nPhases = phases.size();
   Distribution distribution( length, nPhases );
   distribution.SetSampling( c_object.PixelSize(), 1 );
   std::vector< dip::uint >counts( nPhases, 0 );

   // Fill output
   if( useRandom ) {
      RandomPixelPairSampler( object, mask, random, distribution, counts, phaseLookupTable, probes );
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
