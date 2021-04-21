/*
 * DIPlib 3.0
 * This file contains definitions for pixel pair estimation functions
 *
 * (c)2018-2021, Cris Luengo.
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
#include "diplib/saturated_arithmetic.h"

namespace dip {

namespace {

//
// --- Infrastructure ---
//

// Reading UInt value

using UIntPixelValueReaderFunction = dip::uint ( * )( void const* );

template< typename TPI >
static dip::uint UIntPixelValueReader( void const* data ) {
   return static_cast< dip::uint >( *static_cast< TPI const* >( data ));
}

// Reading Float value

using FloatPixelValueReaderWithOffsetFunction = dfloat ( * )( void const*, dip::sint );

template< typename TPI >
dfloat FloatPixelValueReaderWithOffset( void const* data, dip::sint offset ) {
   return static_cast< dfloat >( *( static_cast< TPI const* >( data ) + offset ));
}

using FloatPixelValueReaderFunction = dfloat ( * )( void const* );

template< typename TPI >
dfloat FloatPixelValueReader( void const* data ) {
   return static_cast< dfloat >( *static_cast< TPI const* >( data ));
}


class PixelPairFunction {
   public:
      virtual void UpdateRandom(
            UnsignedArray const& coords1,
            UnsignedArray const& coords2,
            dip::uint distance
      ) = 0;
      virtual void UpdateGrid(
            void const* dataPtr1,
            void const* dataPtr2,
            dip::uint distance
      ) = 0;
      // virtual void SetNumberOfThreads( dip::uint  threads ) { ( void )threads; }
};

void RandomPixelPairSampler(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      PixelPairFunction* pixelPairFunction,
      dip::uint nProbes,
      dip::uint maxLength
) {
   bool hasMask = mask.IsForged();
   Random random( 0 );
   UniformRandomGenerator uniformRandomGenerator( random );
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
      distance = static_cast< dip::uint >( std::round( std::sqrt( distance )));
      pixelPairFunction->UpdateRandom( coords1, coords2, distance );
   }
}

void GridPixelPairSampler(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      PixelPairFunction* pixelPairFunction,
      dip::uint nProbes,
      dip::uint maxLength
) {
   bool hasMask = mask.IsForged();
   dip::uint nDims = object.Dimensionality();
   UnsignedArray coords( nDims );
   dip::uint nPixels = object.Sizes().product();
   dip::uint nGridPoints = nPixels;
   dip::uint step = 1; // how many lines to skip
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
            if( !hasMask || *maskPtr ) {
               // Iterate over pixels at all distances from this pixel
               dip::uint max = std::min( maxLength, size - ii - 1 );
               void const* dataPtr2 = dataPtr;
               bin const* maskPtr2 = maskPtr;
               for( dip::uint distance = 0; distance <= max; ++distance ) {
                  if( !hasMask || *maskPtr2 ) {
                     pixelPairFunction->UpdateGrid( dataPtr, dataPtr2, distance );
                  }
                  dataPtr2 = static_cast< uint8 const* >( dataPtr2 ) + dataStride;
                  maskPtr2 += maskStride;
               }
            }
            dataPtr = static_cast< uint8 const* >( dataPtr ) + dataStride;
            maskPtr += maskStride;
         }
         for( dip::uint ii = 0; ( ii < step ) && it; ++ii ) {
            ++it; // TODO: this does not skip appropriately in 3D and higher dims.
         }
      } while( it );
   }
}

void NormalizeDistribution(
      Distribution& distribution,
      std::vector< dip::uint >& counts
) {
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

} // namespace

//
// --- Pair Correlation ---
//

namespace {

using PhaseLookupTable = tsl::robin_map< dip::uint, dip::uint >;

class PairCorrelationFunction : public PixelPairFunction {
   public:
      virtual void UpdateRandom(
            UnsignedArray const& coords1,
            UnsignedArray const& coords2,
            dip::uint distance
      ) override {
         UpdateGrid( object_.Pointer( coords1 ), object_.Pointer( coords2 ), distance );
      }

      virtual void UpdateGrid(
            void const* dataPtr1,
            void const* dataPtr2,
            dip::uint distance
      ) override {
         dip::uint phase1 = GetUIntPixelValue_( dataPtr1 );
         dip::uint phase2 = GetUIntPixelValue_( dataPtr2 );
         ++( counts_[ distance ] );
         dip::uint index1 = phaseLookupTable_.at( phase1 );
         if( covariance_ ) {
            if( phase1 == phase2 ) {
               distribution_[ distance ].Y( index1, index1 ) += 1.0;
            } else {
               dip::uint index2 = phaseLookupTable_.at( phase2 );
               // To make sure the matrix remains symmetric, we assign half the hit to each phase.
               distribution_[ distance ].Y( index1, index2 ) += 0.5;
               distribution_[ distance ].Y( index2, index1 ) += 0.5;
            }
         } else {
            if( phase1 == phase2 ) {
               distribution_[ distance ].Y( index1 ) += 1.0;
            }
         }
      }

      PairCorrelationFunction(
            Image const& object,
            Distribution& distribution,         // distribution.Rows()==nPhases
            std::vector< dip::uint >& counts,
            PhaseLookupTable const& phaseLookupTable,
            bool covariance                     // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
      ) : object_( object ), distribution_( distribution ), counts_( counts ), phaseLookupTable_( phaseLookupTable ),
          covariance_( covariance ) {
         DIP_OVL_ASSIGN_UINT( GetUIntPixelValue_, UIntPixelValueReader, object.DataType() );
      }

   private:
      Image const& object_;
      Distribution& distribution_;
      std::vector< dip::uint >& counts_;
      PhaseLookupTable const& phaseLookupTable_;
      bool covariance_;
      UIntPixelValueReaderFunction GetUIntPixelValue_;
};

enum class PairCorrelationNormalization{ None, Volume, VolumeSquare };

std::pair< bool, PairCorrelationNormalization > ParsePairCorrelationOptions( StringSet const& options ) {
   PairCorrelationNormalization normalization = PairCorrelationNormalization::None;
   bool normalize = false;
   bool normalize2 = false;
   bool covariance = false;
   for( auto& o: options ) {
      if( o == "covariance" ) {
         covariance = true;
      } else if( o == "normalize volume" ) {
         normalization = PairCorrelationNormalization::Volume;
         normalize = true;
      } else if( o == "normalize volume^2" ) {
         normalization = PairCorrelationNormalization::VolumeSquare;
         normalize2 = true;
      } else {
         DIP_THROW_INVALID_FLAG( o );
      }
   }
   DIP_THROW_IF( normalize && normalize2, E::ILLEGAL_FLAG_COMBINATION );
   return { covariance, normalization };
}

void NormalizePairCorrelationDistribution(
      Distribution& distribution,
      dip::uint nPhases,
      bool covariance,
      PairCorrelationNormalization normalization
) {
   if( normalization != PairCorrelationNormalization::None ) {
      if( covariance ) {
         FloatArray volumeFractions( nPhases );
         dip::uint linearIndex = 0;
         for( dip::uint ii = 0; ii < nPhases; ++ii ) {
            dfloat volumeFraction = distribution[ 0 ].Y( ii, ii );
            if( volumeFraction != 0.0 ) {
               if( normalization == PairCorrelationNormalization::VolumeSquare ) {
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
            if( normalization == PairCorrelationNormalization::VolumeSquare ) {
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
   bool covariance;
   PairCorrelationNormalization normalization;
   DIP_STACK_TRACE_THIS( std::tie( covariance, normalization ) = ParsePairCorrelationOptions( options ));

   // Create output
   dip::uint nPhases = phases.size();
   Distribution distribution( length + 1, nPhases, covariance ? nPhases : 1 );
   distribution.SetSampling( c_object.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   PairCorrelationFunction pixelPairFunction( object, distribution, counts, phaseLookupTable, covariance );
   if( random ) {
      RandomPixelPairSampler( object, mask, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( object, mask, &pixelPairFunction, probes, length );
   }

   // Process the intermediate output results
   NormalizeDistribution( distribution, counts );
   NormalizePairCorrelationDistribution( distribution, nPhases, covariance, normalization );

   return distribution;
}

//
// --- Probabilistic Pair Correlation ---
//

namespace {

class ProbabilisticPairCorrelationFunction : public PixelPairFunction {
   public:
      virtual void UpdateRandom(
            UnsignedArray const& coords1,
            UnsignedArray const& coords2,
            dip::uint distance
      ) override {
         UpdateGrid( phases_.Pointer( coords1 ), phases_.Pointer( coords2 ), distance );
      }

      virtual void UpdateGrid(
            void const* dataPtr1,
            void const* dataPtr2,
            dip::uint distance
      ) override {
         ++( counts_[ distance ] );
         if( covariance_ ) {
            for( dip::uint phase1 = 0; phase1 < nPhases_; ++phase1 ) {
               dfloat prob1 = GetFloatPixelValue_( dataPtr1, phases_.TensorStride() * static_cast< dip::sint >( phase1 ));
               for( dip::uint phase2 = phase1; phase2 < nPhases_; ++phase2 ) {
                  dfloat prob2 = GetFloatPixelValue_( dataPtr2, phases_.TensorStride() * static_cast< dip::sint >( phase2 ));
                  distribution_[ distance ].Y( phase1, phase2 ) += prob1 * prob2;
                  if( phase1 != phase2 ) {
                     distribution_[ distance ].Y( phase2, phase1 ) += prob1 * prob2;
                  }
               }
            }
         } else {
            for( dip::uint ii = 0; ii < nPhases_; ++ii ) {
               dfloat prob1 = GetFloatPixelValue_( dataPtr1, phases_.TensorStride() * static_cast< dip::sint >( ii ));
               dfloat prob2 = GetFloatPixelValue_( dataPtr2, phases_.TensorStride() * static_cast< dip::sint >( ii ));
               distribution_[ distance ].Y( ii ) += prob1 * prob2;
            }
         }
      }

      ProbabilisticPairCorrelationFunction(
            Image const& phases,
            Distribution& distribution,         // distribution_.Rows()==nPhases
            std::vector< dip::uint >& counts,
            bool covariance                     // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
      ) : phases_( phases ), distribution_( distribution ), counts_( counts ), covariance_( covariance ) {
         DIP_OVL_ASSIGN_FLOAT( GetFloatPixelValue_, FloatPixelValueReaderWithOffset, phases.DataType() );
         nPhases_ = phases.TensorElements();
      }

   private:
      Image const& phases_;
      Distribution& distribution_;
      std::vector< dip::uint >& counts_;
      dip::uint nPhases_;
      bool covariance_;
      FloatPixelValueReaderWithOffsetFunction GetFloatPixelValue_;
};

} // namespace

Distribution ProbabilisticPairCorrelation(
      Image const& phases,
      Image const& mask,
      dip::uint probes,
      dip::uint length,
      String const& sampling,
      StringSet const& options
) {
   DIP_THROW_IF( !phases.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !phases.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( phases.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Parse options
   bool random;
   DIP_STACK_TRACE_THIS( random = BooleanFromString( sampling, S::RANDOM, S::GRID ));
   bool covariance;
   PairCorrelationNormalization normalization;
   DIP_STACK_TRACE_THIS( std::tie( covariance, normalization ) = ParsePairCorrelationOptions( options ));

   // Create output
   dip::uint nPhases = phases.TensorElements();
   Distribution distribution( length + 1, nPhases, covariance ? nPhases : 1 );
   distribution.SetSampling( phases.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   ProbabilisticPairCorrelationFunction pixelPairFunction( phases, distribution, counts, covariance );
   if( random ) {
      RandomPixelPairSampler( phases, mask, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( phases, mask, &pixelPairFunction, probes, length );
   }

   // Process the intermediate output results
   NormalizeDistribution( distribution, counts );
   NormalizePairCorrelationDistribution( distribution, nPhases, covariance, normalization );
   // TODO: The old DIPlib code used the same normalization as for PairCorrelation, but that is not correct?
   // Here distribution[0]/counts[0] is not the volume fraction, but the square of the volume fraction. I think.

   return distribution;
}

//
// --- Semivariogram ---
//

namespace {

class SemivariogramFunction : public PixelPairFunction {
   public:
      virtual void UpdateRandom(
            UnsignedArray const& coords1,
            UnsignedArray const& coords2,
            dip::uint distance
      ) override {
         UpdateGrid( image_.Pointer( coords1 ), image_.Pointer( coords2 ), distance );
      }

      virtual void UpdateGrid(
            void const* dataPtr1,
            void const* dataPtr2,
            dip::uint distance
      ) override {
         ++( counts_[ distance ] );
         dfloat diff = GetFloatPixelValue_( dataPtr1 ) - GetFloatPixelValue_( dataPtr2 );
         distribution_[ distance ].Y() += 0.5 * diff * diff;
      }

      SemivariogramFunction(
            Image const& in,
            Distribution& distribution,         // distribution_.Rows()==nPhases
            std::vector< dip::uint >& counts
      ) : image_( in ), distribution_( distribution ), counts_( counts ) {
         DIP_OVL_ASSIGN_REAL( GetFloatPixelValue_, FloatPixelValueReader, in.DataType() );
      }

   private:
      Image const& image_;
      Distribution& distribution_;
      std::vector< dip::uint >& counts_;
      FloatPixelValueReaderFunction GetFloatPixelValue_;
};

} // namespace

Distribution Semivariogram(
      Image const& in,
      Image const& mask,
      dip::uint probes,
      dip::uint length,
      String const& sampling
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Parse options
   bool random;
   DIP_STACK_TRACE_THIS( random = BooleanFromString( sampling, S::RANDOM, S::GRID ));

   // Create output
   Distribution distribution( length + 1, 1 );
   distribution.SetSampling( in.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   SemivariogramFunction pixelPairFunction( in, distribution, counts );
   if( random ) {
      RandomPixelPairSampler( in, mask, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( in, mask, &pixelPairFunction, probes, length );
   }

   // Process the intermediate output results
   NormalizeDistribution( distribution, counts );

   return distribution;
}

} // namespace dip
