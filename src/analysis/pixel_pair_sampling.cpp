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
#include <tuple>
#include <utility>
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

//
// --- Infrastructure ---
//

class PixelPairFunction {
public:
   virtual ~PixelPairFunction() = default;
   virtual void SetNumberOfThreads( dip::uint /*nThreads*/ ) {}
   virtual void Update(
         void const* dataPtr1,
         void const* dataPtr2,
         dip::uint distance,
         dip::uint thread
   ) = 0;
};

void RandomPixelPairSampler(
      Image const& object, // unsigned integer type
      Image const& mask,   // might or might not be forged
      Random& random,
      PixelPairFunction* pixelPairFunction,
      dip::uint nProbes,
      dip::uint maxLength
) {
   bool hasMask = mask.IsForged();
   dip::uint nDims = object.Dimensionality();
   UnsignedArray const& sizes = object.Sizes();
   // Multithreading
   dip::uint nThreads = GetNumberOfThreads();
   if( nProbes < 100 * nThreads ) {
      // If there's not enough work per thread, don't start threads
      // NOTE! Hard-coded threshold, seems to work fine on my particular machine...
      nThreads = 1;
   }
   pixelPairFunction->SetNumberOfThreads( nThreads );
   //
   std::vector< Random > randomArray( nThreads - 1, random );
   std::vector< UniformRandomGenerator > uniformRandomGeneratorArray;
   uniformRandomGeneratorArray.emplace_back( random );
   for( dip::uint ii = 1; ii < nThreads; ++ii ) {
      randomArray[ ii - 1 ].SetStream( random() ); // Using a random value from the original stream. This is the same as random.Split().
      uniformRandomGeneratorArray.emplace_back( randomArray[ ii - 1 ] );
   }
   DIP_PARALLEL_ERROR_DECLARE
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   DIP_PARALLEL_ERROR_START
      #pragma omp master
   nThreads = static_cast< dip::uint >( omp_get_num_threads() ); // Get the number of threads actually created, could be fewer than the original nThreads.
      #pragma omp barrier
      dip::uint thread = static_cast< dip::uint >( omp_get_thread_num() );
      UniformRandomGenerator& uniformRandomGenerator = uniformRandomGeneratorArray[ thread ];
      UnsignedArray coords1( nDims );
      UnsignedArray coords2( nDims );
      for( dip::uint probe = 0; probe < nProbes / nThreads; ++probe ) {
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
         dip::uint distance{};
         do {
            distance = 0;
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               coords2[ ii ] = static_cast< dip::uint >( uniformRandomGenerator(
                     static_cast< dfloat >( topLeft[ ii ] ), static_cast< dfloat >( botRight[ ii ] ))); // computes floor
               dip::uint d = coords2[ ii ] >= coords1[ ii ] ? coords2[ ii ] - coords1[ ii ] : coords1[ ii ] - coords2[ ii ];
               distance += d * d;
            }
            isInMask = false;
            if(( distance != 0 ) && ( distance <= maxLength * maxLength )) {
               isInMask = hasMask ? static_cast< bool >( *static_cast< bin* >( mask.Pointer( coords2 ))) : true;
            }
         } while( !isInMask );
         distance = static_cast< dip::uint >( std::round( std::sqrt( distance )));
         pixelPairFunction->Update( object.Pointer( coords1 ), object.Pointer( coords2 ), distance, thread );
      }
   DIP_PARALLEL_ERROR_END
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
   dip::uint step = 1; // same step size along all dimensions
   if( nProbes > 0 ) {
      // The number of probes is computed as follows: We sample the image every `step` pixels
      // along each dimension, obtaining a set of grid points. For each grid point we obtain
      // pixel pairs by walking up to `maxLength` pixels in each dimension. Each pixel pair
      // is a probe.
      dip::uint nGridPoints = div_ceil( nProbes, nDims * ( maxLength + 1 ));
      dfloat stepLength = static_cast< dfloat >( object.NumberOfPixels() ) / static_cast< dfloat >( nGridPoints );
      stepLength = std::pow( stepLength, 1.0 / static_cast< dfloat >( nDims ));
      stepLength = std::max( std::round( stepLength ), 1.0 ); // step must be at least 1
      step = static_cast< dip::uint >( stepLength );
   }
   dip::Image stepObject = ( step > 1 ) ? Subsampling( object, { step } ) : object.QuickCopy();
   dip::Image stepMask = ( hasMask && step > 1 ) ? Subsampling( mask, { step } ) : mask.QuickCopy();
   // Multithreading
   // TODO: parallelize the loop below. Should be easy, but we need to step away from the image iterator, unfortunately.
   pixelPairFunction->SetNumberOfThreads( 1 );
   // Iterate over subsampled image.
   GenericJointImageIterator< 2 > it( { stepObject, stepMask } );
   do {
      // `it` is the first point of a set of pairs, we get the other by walking up to `maxLength` pixels along each image dimension
      bin const* maskPtr = hasMask ? static_cast< bin const* >( it.Pointer< 1 >() ) : nullptr;
      if( !hasMask || *maskPtr ) {
         uint8 const* dataPtr = static_cast< uint8 const* >( it.Pointer< 0 >() ); // using uint8 to iterate more easily, we're not dereferencing this
         // Iterate over image dimensions
         for( dip::uint dim = 0; dim < nDims; ++dim ) {
            dip::uint size = object.Size( dim );
            dip::uint pos = it.Coordinates()[ dim ] * step;
            dip::uint maxDist = std::min( maxLength, size - pos - 1 );
            dip::sint dataStride = object.Stride( dim ) * static_cast< dip::sint >( object.DataType().SizeOf() );
            dip::sint maskStride = hasMask ? mask.Stride( dim ) : 0;
            dip::uint8 const* dataPtr2 = dataPtr;
            bin const* maskPtr2 = maskPtr;
            // Iterate over pixels at all distances from this pixel
            for( dip::uint distance = 1; distance <= maxDist; ++distance ) {
               if( !hasMask || *maskPtr2 ) {
                  pixelPairFunction->Update( dataPtr, dataPtr2, distance, 0 );
               }
               dataPtr2 += dataStride;
               maskPtr2 += maskStride;
            }
         }
      }
   } while( ++it );
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
   PairCorrelationFunction(
         Image const& object,
         Distribution& distribution,         // distribution.Rows()==nPhases
         std::vector< dip::uint >& counts,
         PhaseLookupTable const& phaseLookupTable,
         bool covariance                     // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
   ) : distribution_( distribution ), counts_( counts ), phaseLookupTable_( phaseLookupTable ), covariance_( covariance ) {
      DIP_OVL_ASSIGN_UINT( GetUIntPixelValue_, UIntPixelValueReader, object.DataType() );
   }

   void SetNumberOfThreads( dip::uint nThreads ) override {
      threadData_.resize( nThreads, distribution_ ); // Initialize with copies
      threadCounts_.resize( nThreads, counts_ );     // Initialize with copies
   }

   void Update(
         void const* dataPtr1,
         void const* dataPtr2,
         dip::uint distance,
         dip::uint thread
   ) override {
      dip::uint phase1 = GetUIntPixelValue_( dataPtr1 );
      dip::uint phase2 = GetUIntPixelValue_( dataPtr2 );
      ++( threadCounts_[ thread ][ distance ] );
      dip::uint index1 = phaseLookupTable_.at( phase1 );
      if( covariance_ ) {
         if( phase1 == phase2 ) {
            threadData_[ thread ][ distance ].Y( index1, index1 ) += 1.0;
         } else {
            dip::uint index2 = phaseLookupTable_.at( phase2 );
            // To make sure the matrix remains symmetric, we assign half the hit to each phase.
            threadData_[ thread ][ distance ].Y( index1, index2 ) += 0.5;
            threadData_[ thread ][ distance ].Y( index2, index1 ) += 0.5;
         }
      } else {
         if( phase1 == phase2 ) {
            threadData_[ thread ][ distance ].Y( index1 ) += 1.0;
         }
      }
   }

   void Finalize() {
      distribution_ = threadData_[ 0 ];
      std::copy( threadCounts_[ 0 ].begin(), threadCounts_[ 0 ].end(), counts_.begin() );
      for( dip::uint ii = 1; ii < threadData_.size(); ++ii ) {
         distribution_ += threadData_[ ii ];
         std::transform( counts_.begin(), counts_.end(), threadCounts_[ ii ].begin(), counts_.begin(), std::plus<>() );
      }
   }

private:
   using UIntPixelValueReaderFunction = dip::uint ( * )( void const* );

   template< typename TPI >
   static dip::uint UIntPixelValueReader( void const* data ) {
      return static_cast< dip::uint >( *static_cast< TPI const* >( data ));
   }

   Distribution& distribution_;
   std::vector< Distribution > threadData_;
   std::vector< dip::uint >& counts_;
   std::vector< std::vector< dip::uint >> threadCounts_;
   PhaseLookupTable const& phaseLookupTable_;
   bool covariance_;
   UIntPixelValueReaderFunction GetUIntPixelValue_;
};

enum class PairCorrelationNormalization: dip::uint8 { None, Volume, VolumeSquare };

std::pair< bool, PairCorrelationNormalization > ParsePairCorrelationOptions( StringSet const& options ) {
   PairCorrelationNormalization normalization = PairCorrelationNormalization::None;
   bool normalize = false;
   bool normalize2 = false;
   bool covariance = false;
   for( auto const& o: options ) {
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
      Random& random,
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
   std::vector< LabelType > phases;
   DIP_STACK_TRACE_THIS( phases = ListObjectLabels( object, mask, S::INCLUDE )); // Will test mask for us -- doesn't allow singleton expansion, so we don't need to here either
   PhaseLookupTable phaseLookupTable;
   for( dip::uint ii = 0; ii < phases.size(); ++ii ) {
      phaseLookupTable.emplace( phases[ ii ], ii );
   }

   // Parse options
   bool useRandom{};
   DIP_STACK_TRACE_THIS( useRandom = BooleanFromString( sampling, S::RANDOM, S::GRID ));
   bool covariance{};
   PairCorrelationNormalization normalization{};
   DIP_STACK_TRACE_THIS( std::tie( covariance, normalization ) = ParsePairCorrelationOptions( options ));

   // Create output
   dip::uint nPhases = phases.size();
   Distribution distribution( length + 1, nPhases, covariance ? nPhases : 1 );
   distribution.SetSampling( c_object.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   PairCorrelationFunction pixelPairFunction( object, distribution, counts, phaseLookupTable, covariance );
   if( useRandom ) {
      RandomPixelPairSampler( object, mask, random, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( object, mask, &pixelPairFunction, probes, length );
   }
   pixelPairFunction.Finalize();

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
   ProbabilisticPairCorrelationFunction(
         Image const& phases,
         Distribution& distribution,         // distribution_.Rows()==nPhases
         std::vector< dip::uint >& counts,
         bool covariance                     // if true, distribution.Columns()==nPhases, otherwise distribution.Columns()==1
   ) : phases_( phases ), distribution_( distribution ), counts_( counts ), covariance_( covariance ) {
      DIP_OVL_ASSIGN_FLOAT( GetFloatPixelValue_, FloatPixelValueReaderWithOffset, phases.DataType() );
      nPhases_ = phases.TensorElements();
   }

   void SetNumberOfThreads( dip::uint nThreads ) override {
      threadData_.resize( nThreads, distribution_ ); // Initialize with copies
      threadCounts_.resize( nThreads, counts_ );     // Initialize with copies
   }

   void Update(
         void const* dataPtr1,
         void const* dataPtr2,
         dip::uint distance,
         dip::uint thread
   ) override {
      ++( threadCounts_[ thread ][ distance ] );
      if( covariance_ ) {
         for( dip::uint phase1 = 0; phase1 < nPhases_; ++phase1 ) {
            dfloat prob1 = GetFloatPixelValue_( dataPtr1, phases_.TensorStride() * static_cast< dip::sint >( phase1 ));
            for( dip::uint phase2 = phase1; phase2 < nPhases_; ++phase2 ) {
               dfloat prob2 = GetFloatPixelValue_( dataPtr2, phases_.TensorStride() * static_cast< dip::sint >( phase2 ));
               threadData_[ thread ][ distance ].Y( phase1, phase2 ) += prob1 * prob2;
               if( phase1 != phase2 ) {
                  threadData_[ thread ][ distance ].Y( phase2, phase1 ) += prob1 * prob2;
               }
            }
         }
      } else {
         for( dip::uint ii = 0; ii < nPhases_; ++ii ) {
            dfloat prob1 = GetFloatPixelValue_( dataPtr1, phases_.TensorStride() * static_cast< dip::sint >( ii ));
            dfloat prob2 = GetFloatPixelValue_( dataPtr2, phases_.TensorStride() * static_cast< dip::sint >( ii ));
            threadData_[ thread ][ distance ].Y( ii ) += prob1 * prob2;
         }
      }
   }

   void Finalize() {
      distribution_ = threadData_[ 0 ];
      std::copy( threadCounts_[ 0 ].begin(), threadCounts_[ 0 ].end(), counts_.begin() );
      for( dip::uint ii = 1; ii < threadData_.size(); ++ii ) {
         distribution_ += threadData_[ ii ];
         std::transform( counts_.begin(), counts_.end(), threadCounts_[ ii ].begin(), counts_.begin(), std::plus<>() );
      }
   }

private:
   using FloatPixelValueReaderWithOffsetFunction = dfloat ( * )( void const*, dip::sint );

   template< typename TPI >
   static dfloat FloatPixelValueReaderWithOffset( void const* data, dip::sint offset ) {
      return static_cast< dfloat >( *( static_cast< TPI const* >( data ) + offset ));
   }

   Image const& phases_;
   Distribution& distribution_;
   std::vector< Distribution > threadData_;
   std::vector< dip::uint >& counts_;
   std::vector< std::vector< dip::uint >> threadCounts_;
   dip::uint nPhases_;
   bool covariance_;
   FloatPixelValueReaderWithOffsetFunction GetFloatPixelValue_;
};

} // namespace

Distribution ProbabilisticPairCorrelation(
      Image const& phases,
      Image const& mask,
      Random& random,
      dip::uint probes,
      dip::uint length,
      String const& sampling,
      StringSet const& options
) {
   DIP_THROW_IF( !phases.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !phases.DataType().IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( phases.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Parse options
   bool useRandom{};
   DIP_STACK_TRACE_THIS( useRandom = BooleanFromString( sampling, S::RANDOM, S::GRID ));
   bool covariance{};
   PairCorrelationNormalization normalization{};
   DIP_STACK_TRACE_THIS( std::tie( covariance, normalization ) = ParsePairCorrelationOptions( options ));

   // Create output
   dip::uint nPhases = phases.TensorElements();
   Distribution distribution( length + 1, nPhases, covariance ? nPhases : 1 );
   distribution.SetSampling( phases.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   ProbabilisticPairCorrelationFunction pixelPairFunction( phases, distribution, counts, covariance );
   if( useRandom ) {
      RandomPixelPairSampler( phases, mask, random, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( phases, mask, &pixelPairFunction, probes, length );
   }
   pixelPairFunction.Finalize();

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
   SemivariogramFunction(
         Image const& in,
         Distribution& distribution,         // distribution_.Rows()==nPhases
         std::vector< dip::uint >& counts
   ) : distribution_( distribution ), counts_( counts ) {
      DIP_OVL_ASSIGN_REAL( GetFloatPixelValue_, FloatPixelValueReader, in.DataType() );
   }

   void SetNumberOfThreads( dip::uint nThreads ) override {
      threadData_.resize( nThreads, distribution_ ); // Initialize with copies
   }

   void Update(
         void const* dataPtr1,
         void const* dataPtr2,
         dip::uint distance,
         dip::uint thread
   ) override {
      ++( counts_[ distance ] );
      dfloat diff = GetFloatPixelValue_( dataPtr1 ) - GetFloatPixelValue_( dataPtr2 );
      threadData_[ thread ][ distance ].Y() += 0.5 * diff * diff;
   }

   void Finalize() {
      for( auto const& data: threadData_ ) {
         distribution_ += data;
      }
   }

private:
   using FloatPixelValueReaderFunction = dfloat ( * )( void const* );

   template< typename TPI >
   static dfloat FloatPixelValueReader( void const* data ) {
      return static_cast< dfloat >( *static_cast< TPI const* >( data ));
   }

   Distribution& distribution_;
   std::vector< Distribution > threadData_;
   std::vector< dip::uint >& counts_;
   FloatPixelValueReaderFunction GetFloatPixelValue_;
};

} // namespace

Distribution Semivariogram(
      Image const& in,
      Image const& mask,
      Random& random,
      dip::uint probes,
      dip::uint length,
      String const& sampling
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Parse options
   bool useRandom{};
   DIP_STACK_TRACE_THIS( useRandom = BooleanFromString( sampling, S::RANDOM, S::GRID ));

   // Create output
   Distribution distribution( length + 1, 1 );
   distribution.SetSampling( in.PixelSize() );
   std::vector< dip::uint >counts( length + 1, 0 );

   // Fill output
   SemivariogramFunction pixelPairFunction( in, distribution, counts );
   if( useRandom ) {
      RandomPixelPairSampler( in, mask, random, &pixelPairFunction, probes, length );
   } else {
      GridPixelPairSampler( in, mask, &pixelPairFunction, probes, length );
   }
   pixelPairFunction.Finalize();

   // Process the intermediate output results
   NormalizeDistribution( distribution, counts );

   return distribution;
}

} // namespace dip
