/*
 * DIPlib 3.0
 * This file contains definitions for pairwise correlation functions
 *
 * (c)2018, Cris Luengo.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include <unordered_map>

#include "diplib.h"
#include "diplib/analysis.h"
#include "diplib/regions.h"
#include "diplib/generic_iterators.h"
#include "diplib/overload.h"
#include "diplib/random.h"
#include "diplib/saturated_arithmetic.h"

namespace dip {

namespace {

// For uint types
template< typename TPI >
dip::uint ReadPixelUInt( Image const& object, UnsignedArray const& coords ) {
   return static_cast< dip::uint >( *static_cast< TPI const* >( object.Pointer( coords )));
}
// For float types
template< typename TPI >
dfloat ReadPixelFloat( Image const& object, UnsignedArray const& coords ) {
   return static_cast< dfloat >( *static_cast< TPI const* >( object.Pointer( coords )));
}

void RandomPairCorrelation(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      Distribution& distribution,  // distribution.Rows()==nPhases
      std::vector< dip::uint >& counts,
      std::unordered_map< dip::uint, dip::uint > const& phaseLookupTable,
      dip::uint nProbes,
      bool covariance      // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
) {
   dip::uint ( *PixelReader )( Image const& object, UnsignedArray const& coords );
   DIP_OVL_ASSIGN_UINT( PixelReader, ReadPixelUInt, object.DataType() );
   bool hasMask = mask.IsForged();
   Random random( 0 );
   UniformRandomGenerator uniformRandomGenerator( random );
   dip::uint maxLength = distribution.Size() - 1;
   dip::uint nDims = object.Dimensionality();
   UnsignedArray const& sizes = object.Sizes();
   UnsignedArray coords1( nDims );
   UnsignedArray coords2( nDims );
   for( dip::uint probe = 0; probe < nProbes; ++probe ) { // TODO: trivially parallelizable.
      bool isInMask = true;

      // First point
      do {
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            coords1[ ii ] = static_cast< dip::uint >( uniformRandomGenerator( 0, static_cast< dfloat >( sizes[ ii ] ))); // computes floor
         }
         isInMask = hasMask ? static_cast< bool >( *static_cast< bin* >( mask.Pointer( coords1 ))) : true;
      } while( !isInMask );
      dip::uint phase1 = PixelReader( object, coords1 );

      // Second point, probe within a region of side maxLength around first point
      UnsignedArray topLeft( nDims );
      UnsignedArray botRight( nDims );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         topLeft[ ii ] = coords1[ ii ] > maxLength ? coords1[ ii ] - maxLength : 0u;
         botRight[ ii ] = std::min( coords1[ ii ] + maxLength + 1, sizes[ ii ] );
      }
      dip::uint distance;
      do {
         distance = 0;
         for( dip::uint ii = 0; ii < nDims; ++ii ) {
            coords2[ ii ] = static_cast< dip::uint >( uniformRandomGenerator(
                  static_cast< dfloat >( topLeft[ ii ] ), static_cast< dfloat >( botRight[ ii ] ))); // computes floor
            dip::uint d = coords2[ ii ] >= coords1[ ii ] ? coords2[ ii ] - coords1[ ii ] : coords1[ ii ] - coords2[ ii ];
            distance += d * d;
         }
         if( distance > maxLength * maxLength ) {
            isInMask = false;
         } else {
            isInMask = hasMask ? static_cast< bool >( *static_cast< bin* >( mask.Pointer( coords2 ))) : true;
         }
      } while( !isInMask );
      dip::uint phase2 = PixelReader( object, coords2 );
      distance = static_cast< dip::uint >( std::round( std::sqrt( distance )));

      // Update `distribution` and `counts` with new points
      ++( counts[ distance ] );
      dip::uint index1 = phaseLookupTable.at( phase1 );
      if( covariance ) {
         if( phase1 == phase2 ) {
            distribution[ distance ].Y( index1, index1 ) += 1.0;
         } else {
            dip::uint index2 = phaseLookupTable.at( phase2 );
            // To make sure the matrix remains symmetric, we assign half the hit to each phase.
            distribution[ distance ].Y( index1, index2 ) += 0.5;
            distribution[ distance ].Y( index2, index1 ) += 0.5;
         }
      } else {
         if( phase1 == phase2 ) {
            distribution[ distance ].Y( index1 ) += 1.0;
         }
      }
   }
}

// For uint types
template< typename TPI >
dip::uint GetPixelUInt( void const* data ) {
   return static_cast< dip::uint >( *static_cast< TPI const* >( data ));
}
// For float types
template< typename TPI >
dfloat GetPixelFloat( void const* data ) {
   return static_cast< dfloat >( *static_cast< TPI const* >( data ));
}

void GridPairCorrelation(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      Distribution& distribution,  // distribution.Rows()==nPhases
      std::vector< dip::uint >& counts,
      std::unordered_map< dip::uint, dip::uint > const& phaseLookupTable,
      dip::uint nProbes,
      bool covariance      // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
) {
   dip::uint ( *PixelReader )( void const* );
   DIP_OVL_ASSIGN_UINT( PixelReader, GetPixelUInt, object.DataType() );
   bool hasMask = mask.IsForged();
   dip::uint maxLength = distribution.Size() - 1;
   dip::uint nDims = object.Dimensionality();
   UnsignedArray coords( nDims );
   dip::uint nPixels = object.Sizes().product();
   dip::uint nGridPoints = nPixels;
   dip::uint step = 1; // how many lines to
   if( nProbes > 0 ) {
      nGridPoints = div_ceil( nProbes, nDims * ( maxLength + 1 ));
      dfloat fraction = std::sqrt( static_cast< dfloat >( nPixels ) / static_cast< dfloat >( nGridPoints ));
      step = static_cast< dip::uint >( floor_cast( 1.0 / fraction ));
      step = std::max< dip::uint >( step, 1 ); // step must be at least 1, this test should never trigger.
   }
   // Iterate over image dimensions
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      GenericJointImageIterator< 2 > it( { object, mask }, dim );
      dip::uint size = it.ProcessingDimensionSize();
      dip::sint dataStride = it.ProcessingDimensionStride< 0 >() * static_cast< dip::sint >( object.DataType().SizeOf() );
      dip::sint maskStride = hasMask ? it.ProcessingDimensionStride< 1 >() : 0;
      dip::uint nLinesInGrid = div_floor( nPixels / size, step );
      dip::uint nPointsPerLine = div_ceil( nGridPoints, nLinesInGrid );
      nPointsPerLine = std::min( nPointsPerLine, size ); // don't do more points than we have in the line, this should not trigger.
      dip::uint lineStep = div_floor( size, nPointsPerLine );
      dip::uint lastPoint = lineStep * nPointsPerLine;
      // Iterate over `fraction` image lines
      do {
         // Iterate over `fraction` pixels in this image line
         void const* dataPtr = it.Pointer< 0 >();
         bin const* maskPtr = hasMask ? static_cast< bin const* >( it.Pointer< 1 >() ) : nullptr;
         for( dip::uint ii = 0; ii < lastPoint; ii += lineStep ) {
            dip::uint phase1 = PixelReader( dataPtr );
            if( !hasMask || *maskPtr ) {
               dip::uint index1 = phaseLookupTable.at( phase1 );
               // Iterate over pixels at all distances from this pixel
               dip::uint max = std::min( maxLength, size - ii - 1 );
               void const* dataPtr2 = dataPtr;
               bin const* maskPtr2 = maskPtr;
               for( dip::uint distance = 0; distance <= max; ++distance ) {
                  dip::uint phase2 = PixelReader( dataPtr2 );
                  if( !hasMask || *maskPtr2 ) {
                     // Update `distribution` and `counts` with new points
                     ++( counts[ distance ] );
                     if( covariance ) {
                        if( phase1 == phase2 ) {
                           distribution[ distance ].Y( index1, index1 ) += 1.0;
                        } else {
                           dip::uint index2 = phaseLookupTable.at( phase2 );
                           // To make sure the matrix remains symmetric, we assign half the hit to each phase.
                           distribution[ distance ].Y( index1, index2 ) += 0.5;
                           distribution[ distance ].Y( index2, index1 ) += 0.5;
                        }
                     } else {
                        if( phase1 == phase2 ) {
                           distribution[ distance ].Y( index1 ) += 1.0;
                        }
                     }
                  }
                  dataPtr2 = static_cast< uint8 const* >( dataPtr2 ) + dataStride;
                  maskPtr2 += maskStride;
               }
            }
            dataPtr = static_cast< uint8 const* >( dataPtr ) + dataStride;
            maskPtr += maskStride;
         }
         for( dip::uint ii = 0; ( ii < step ) && it; ++ii ) {
            ++it;
         }
      } while( it );
   }
}

} // namespace

Distribution PairCorrelation(
      Image const& c_object,
      Image const& mask,
      dip::uint probes,
      dip::uint length,
      String const& sampling,
      StringSet const& options
) {
   DIP_THROW_IF( !c_object.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_object.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_object.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   Image object = c_object.QuickCopy();
   if( object.DataType().IsBinary() ) {
      object.Convert( DT_UINT8 );
   }
   UnsignedArray phases;
   DIP_STACK_TRACE_THIS( phases = GetObjectLabels( object, mask, S::INCLUDE )); // Will test mask for us -- doesn't allow singleton expansion, so we don't need to here either
   std::unordered_map< dip::uint, dip::uint > phaseLookupTable;
   for( dip::uint ii = 0; ii < phases.size(); ++ii ) {
      phaseLookupTable.emplace( phases[ ii ], ii );
   }
   // Parse options
   bool random;
   DIP_STACK_TRACE_THIS( random = BooleanFromString( sampling, S::RANDOM, S::GRID ));
   bool normalize = false;
   bool normalize2 = false;
   bool covariance = false;
   for( auto& o: options ) {
      if( o == "covariance" ) {
         covariance = true;
      } else if( o == "normalize volume" ) {
         normalize = true;
      } else if( o == "normalize volume^2" ) {
         normalize2 = true;
      } else {
         DIP_THROW_INVALID_FLAG( o );
      }
   }
   DIP_THROW_IF( normalize && normalize2, E::ILLEGAL_FLAG_COMBINATION );
   // Create output
   dip::uint nPhases = phases.size();
   Distribution distribution( length + 1, nPhases, covariance ? nPhases : 1 );
   dfloat d = 0.0;
   for( auto it = distribution.Xbegin(); it != distribution.Xend(); ++it, ++d ) {
      *it = d;
   }
   std::vector< dip::uint >counts( length + 1, 0 ); // We can determine counts from `distribution`, but why?

   // Fill output
   if( random ) {
      RandomPairCorrelation( object, mask, distribution, counts, phaseLookupTable, probes, covariance );
   } else {
      GridPairCorrelation( object, mask, distribution, counts, phaseLookupTable, probes, covariance );
   }

   // Process the intermediate output results
   {
      auto dit = distribution.begin();
      auto cit = counts.begin();
      dip::uint n = distribution.ValuesPerSample();
      //dip::uint totalCount = 0;
      for( ; cit != counts.end(); ++dit, ++cit ) {
         for( dip::uint ii = 0; ii < n; ++ii ) {
            dit->Y( ii ) /= static_cast< dfloat >( *cit );
            //totalCount += *cit;
         }
      }
      //std::cout << "TOTAL COUNT = " << totalCount << '\n';
   }
   if( normalize || normalize2 ) {
      if( covariance ) {
         FloatArray volumeFractions( nPhases );
         dip::uint linearIndex = 0;
         for( dip::uint ii = 0; ii < nPhases; ++ii ) {
            dfloat volumeFraction = distribution[ 0 ].Y( ii, ii );
            if( volumeFraction != 0.0 ) {
               if( normalize2 ) {
                  volumeFraction *= volumeFraction;
               }
               for( dip::uint jj = 0; jj < nPhases; ++jj ) {
                  for( auto it = distribution.Ybegin( linearIndex ); it != distribution.Yend( linearIndex ); ++it ) {
                     *it /= volumeFraction;
                  }
                  ++linearIndex;
               }
            } else {
               linearIndex += nPhases;
            }
         }
      } else {
         for( dip::uint ii = 0; ii < nPhases; ++ii ) {
            dfloat volumeFraction = distribution[ 0 ].Y( ii );
            if( normalize2 ) {
               volumeFraction *= volumeFraction;
            }
            if( volumeFraction != 0.0 ) {
               for( auto it = distribution.Ybegin( ii ); it != distribution.Yend( ii ); ++it ) {
                  *it /= volumeFraction;
               }
            }
         }
      }
   }

   return distribution;
}

Distribution ProbabilisticPairCorrelation(
      ImageArray const& phases,
      Image const& mask,
      dip::uint probes,
      dip::uint length,
      String const& sampling,
      StringSet const& options
) {
   // TODO
   DIP_THROW( E::NOT_IMPLEMENTED );
}

} // namespace dip
