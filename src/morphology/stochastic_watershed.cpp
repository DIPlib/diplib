/*
 * (c)2017-2022, Cris Luengo.
 * Based on Exact stochastic watershed code: (c)2013, Filip Malmberg.
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

#include "diplib/morphology.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/generation.h"
#include "diplib/graph.h"
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/random.h"
#include "diplib/union_find.h"

namespace dip {

namespace {

template< typename T >
class Matrix {
   public:
      Matrix() = default;
      Matrix( dip::uint xSize, dip::uint ySize, T value = {} ) : m_( xSize * ySize, value ), xSize_( xSize ), ySize_( ySize ) {}
      Matrix( Matrix&& ) = default;
      Matrix& operator=( Matrix&& ) = default;
      Matrix( Matrix const& ) = default;
      Matrix& operator=( Matrix const& ) = default;
      ~Matrix() = default;

      T& at( dip::uint x, dip::uint y ) {
         DIP_ASSERT( x < xSize_ );
         DIP_ASSERT( y < ySize_ );
         return m_[ x * ySize_ + y ];
      }
      T const& at( dip::uint x, dip::uint y ) const {
         DIP_ASSERT( x < xSize_ );
         DIP_ASSERT( y < ySize_ );
         return m_[ x * ySize_ + y ];
      }

   private:
      std::vector< T > m_{};
      dip::uint xSize_ = 0;
      dip::uint ySize_ = 0;
};

template< typename T >
class SymmetricMatrix {
   public:
      SymmetricMatrix() = default;
      SymmetricMatrix( dip::uint size, T value = {} ) : m_( size * ( size + 1 ) / 2, value ), size_( size ) {}
      SymmetricMatrix( SymmetricMatrix&& ) = default;
      SymmetricMatrix& operator=( SymmetricMatrix&& ) = default;
      SymmetricMatrix( SymmetricMatrix const& ) = default;
      SymmetricMatrix& operator=( SymmetricMatrix const& ) = default;
      ~SymmetricMatrix() = default;

      T& at( dip::uint x, dip::uint y ) {
         if( y < x ) {
            std::swap( x, y );
         }
         DIP_ASSERT( y < size_ );
         return m_[ ( 2 * size_ - 1 - x ) * x / 2 + y ];
      }
      T const& at( dip::uint x, dip::uint y ) const {
         if( y < x ) {
            std::swap( x, y );
         }
         DIP_ASSERT( y < size_ );
         return m_[ ( 2 * size_ - 1 - x ) * x / 2 + y ];
      }

   private:
      std::vector< T > m_{};
      dip::uint size_ = 0;
};

template< typename T >
class SparseTable {
   public:
      SparseTable() = default;
      SparseTable( std::vector< T > sequence ) {
         DIP_ASSERT( !sequence.empty() );
         sequence_ = std::move( sequence );
         if( sequence_.size() == 1 ) {
            sparseMatrix_ = { 1, 1 };
         } else {
            // Sparse Table Algorithm
            dip::uint jMax = static_cast< dip::uint >( std::ceil( std::log2( sequence_.size() )));
            sparseMatrix_ = { sequence_.size(), jMax };
            // Fill in first column
            for( dip::uint i = 0; i < sequence_.size(); ++i ) {
               sparseMatrix_.at( i, 0 ) = i;
            }
            // Fill in rest
            for( dip::uint j = 1; j < jMax; ++j ) {
               for( dip::uint i = 0; i < sequence_.size(); ++i ) {
                  if( i + ( dip::uint( 1 ) << j ) - 1 < sequence_.size() ) {
                     dip::uint m = i + ( dip::uint( 1 ) << ( j - 1 ));
                     if( sequence_[ sparseMatrix_.at( i, j - 1 ) ] < sequence_[ sparseMatrix_.at( m, j - 1 ) ] ) {
                        sparseMatrix_.at( i, j ) = sparseMatrix_.at( i, j - 1 );
                     } else {
                        sparseMatrix_.at( i, j ) = sparseMatrix_.at( m, j - 1 );
                     }
                  }
               }
            }
         }
      }
      SparseTable( SparseTable&& ) = default;
      SparseTable& operator=( SparseTable&& ) = default;
      SparseTable( SparseTable const& ) = default;
      SparseTable& operator=( SparseTable const& ) = default;
      ~SparseTable() = default;

      dip::uint at( dip::uint x, dip::uint y ) const {
         return sparseMatrix_.at( x, y );
      }

      dip::uint getEntry( dip::uint x, dip::uint y ) const {
         if( y < x ) {
            std::swap( x, y );
         }
         dip::uint k = static_cast< dip::uint >( std::floor( std::log2( y - x )));
         y -= ( dip::uint( 1 ) << k ) - 1;
         dip::uint i = sparseMatrix_.at( x, k );
         dip::uint j = sparseMatrix_.at( y, k );
         DIP_ASSERT( i < sequence_.size() );
         DIP_ASSERT( j < sequence_.size() );
         return ( sequence_[ i ] < sequence_[ j ] ) ? i : j;
      }

   private:
      std::vector< T > sequence_{};
      Matrix< dip::uint > sparseMatrix_{};
};

template< typename T >
class LookUpTable {
   public:
      LookUpTable() = default;
      LookUpTable( std::vector< T > sequence ) : sequence_( std::move( sequence )) {
         DIP_ASSERT( !sequence_.empty() );
         // Normalize sequence
         T minVal = *min_element( sequence_.begin(), sequence_.end() );
         for( auto& s : sequence_ ) {
            s -= minVal;
         }
         if( sequence_.size() == 1 ) {
            table_ = { 1 };
         } else {
            sparseTable_ = { sequence_ };
            table_ = { sequence_.size() };
            for( dip::uint i = 0; i < sequence_.size(); ++i ) {
               table_.at( i, i ) = i;
               for( dip::uint j = i + 1; j < sequence_.size(); ++j ) {
                  dip::uint k = static_cast< dip::uint >( std::floor( std::log2( j - i )));
                  dip::uint m = j - ( dip::uint( 1 ) << k ) + 1;
                  if( sequence_[ sparseTable_.at( i, k ) ] < sequence_[ sparseTable_.at( m, k ) ] ) {
                     table_.at( i, j ) = sparseTable_.at( i, k );
                  } else {
                     table_.at( i, j ) = sparseTable_.at( m, k );
                  }
               }
            }
         }
      }
      LookUpTable( LookUpTable&& ) = default;
      LookUpTable& operator=( LookUpTable&& ) = default;
      LookUpTable( LookUpTable const& ) = default;
      LookUpTable& operator=( LookUpTable const& ) = default;
      ~LookUpTable() = default;

      bool isInitialized() const {
         return !sequence_.empty();
      }

      dip::uint getEntry( dip::uint x, dip::uint y ) const {
         return table_.at( x, y );
      }

   private:
      std::vector< T > sequence_{};
      SymmetricMatrix< dip::uint > table_{};
      SparseTable< T > sparseTable_{};
};

class Block {
   public:
      Block() = default;
      Block( LookUpTable< dip::uint > lut, dip::uint sequenceLength, dip::uint firstIndex )
         : lut( std::move( lut ) ), lastIndex_internal_( sequenceLength - 1 ), firstIndex_external_( firstIndex ) {}
      Block( Block&& ) = default;
      Block& operator=( Block&& ) = default;
      Block( Block const& ) = default;
      Block& operator=( Block const& ) = default;
      ~Block() = default;

      dip::uint getIndexOfMinVal() const {
         return firstIndex_external_ + lut.getEntry( 0, lastIndex_internal_ );
      }
      dip::uint getIndexOfMinValBeforeEntry( dip::uint x ) const {
         return firstIndex_external_ + lut.getEntry( 0, x - firstIndex_external_ );
      }
      dip::uint getIndexOfMinValAfterEntry( dip::uint x ) const {
         return firstIndex_external_ + lut.getEntry( x - firstIndex_external_, lastIndex_internal_ );
      }
      dip::uint getIndexOfMinValBetweenEntries( dip::uint x, dip::uint y ) const {
         return firstIndex_external_ + lut.getEntry( x - firstIndex_external_, y - firstIndex_external_ );
      }

   private:
      LookUpTable< dip::uint > lut{};
      dip::uint lastIndex_internal_ = 0;
      dip::uint firstIndex_external_ = 0;
};

class RangeMinimumQuery {
   public:
      RangeMinimumQuery( std::vector< dip::uint > data ) : data_( std::move( data )) {
         dip::uint nelem = data_.size();
         luts_.resize( static_cast< dip::uint >( std::ceil( std::sqrt( static_cast< dfloat >( nelem )))));
         blockLength_ = static_cast< dip::uint >( std::ceil( std::log2( nelem ) / 2 ));
         dip::uint numBlocks = static_cast< dip::uint >( std::ceil( static_cast< dfloat >( nelem ) / static_cast< dfloat >( blockLength_ )));
         blocks_.resize( numBlocks );
         createBlocks();
         createSparseTableForBlockMinima();
      }
      RangeMinimumQuery( RangeMinimumQuery&& ) = default;
      RangeMinimumQuery& operator=( RangeMinimumQuery&& ) = default;
      RangeMinimumQuery( RangeMinimumQuery const& ) = default;
      RangeMinimumQuery& operator=( RangeMinimumQuery const& ) = default;
      ~RangeMinimumQuery() = default;

      dip::uint getIndexOfMinimum( dip::uint p1, dip::uint p2 ) const {
         if( p1 > p2 ) {
            std::swap( p1, p2 );
         }
         dip::uint p1_block = getBlock( p1 );
         dip::uint p2_block = getBlock( p2 );
         DIP_ASSERT( p1_block < blocks_.size() );
         DIP_ASSERT( p2_block < blocks_.size() );

         // Query if indices are in the same block
         if( p1_block == p2_block ) {
            return blocks_[ p1_block ].getIndexOfMinValBetweenEntries( p1, p2 );
         }

         // Query if indices lie in different blocks
         dip::uint p1_index = blocks_[ p1_block ].getIndexOfMinValAfterEntry( p1 );
         dip::uint p2_index = blocks_[ p2_block ].getIndexOfMinValBeforeEntry( p2 );
         dip::uint indexOfMinimum = ( data_[ p1_index ] < data_[ p2_index ] ) ? p1_index : p2_index;
         if( p2_block - p1_block > 2 ) {
            dip::uint minBlock = blockMinima_.getEntry( p1_block + 1, p2_block - 1 );
            dip::uint p3_index = blocks_[ minBlock ].getIndexOfMinVal();
            if( data_[ p3_index ] < data_[ indexOfMinimum ] ) {
               return p3_index;
            }
         } else if( p2_block - p1_block == 2 ) {
            dip::uint p4_index = blocks_[ p1_block + 1 ].getIndexOfMinVal();
            if( data_[ p4_index ] < data_[ indexOfMinimum ] ) {
               return p4_index;
            }
         }
         return indexOfMinimum;
      }

   private:
      std::vector< dip::uint > data_;
      dip::uint blockLength_;
      std::vector< LookUpTable< dip::uint >> luts_;
      std::vector< Block > blocks_;
      SparseTable< dip::uint > blockMinima_;

      void createBlocks() {
         dip::uint pos = 0;
         dip::uint currentBlock = 0;
         for( ; currentBlock < blocks_.size() - 1; ++currentBlock ) {
            std::vector< dip::uint > sequence( blockLength_ );
            sequence[ 0 ] = data_[ pos ];
            ++pos;
            dip::uint blockID = 0;
            for( dip::uint j = 0; j < blockLength_ - 1; ++j ) {
               sequence[ j + 1 ] = data_[ pos ];
               if( data_[ pos ] > data_[ pos - 1 ] ) {
                  blockID += dip::uint( 1 ) << j;
               }
               ++pos;
            }
            DIP_ASSERT( blockID < luts_.size() );
            if( !luts_[ blockID ].isInitialized() ) {
               luts_[ blockID ] = { std::move( sequence ) };
            }
            blocks_[ currentBlock ] = { luts_[ blockID ], blockLength_, currentBlock * blockLength_ };
         }
         // Last block
         dip::uint currentBlockLength = data_.size() - pos;
         std::vector< dip::uint > sequence( currentBlockLength );
         sequence[ 0 ] = data_[ pos ];
         ++pos;
         dip::uint blockID = 0;
         for( dip::uint j = 0; j < currentBlockLength - 1; ++j ) {
            sequence[ j + 1 ] = data_[ pos ];
            if( data_[ pos ] > data_[ pos - 1 ] ) {
               blockID += dip::uint( 1 ) << j;
            }
            ++pos;
         }
         if( currentBlockLength == blockLength_ ) {
            DIP_ASSERT( blockID < luts_.size() );
            if( !luts_[ blockID ].isInitialized() ) {
               luts_[ blockID ] = { std::move( sequence ) };
            }
            blocks_[ currentBlock ] = { luts_[ blockID ], blockLength_, currentBlock * blockLength_ };
         } else {
            luts_.emplace_back( std::move( sequence ));
            blocks_[ currentBlock ] = { luts_.back(), currentBlockLength, currentBlock * blockLength_ };
         }
      }

      dip::uint getBlock( dip::uint index ) const {
         return index / blockLength_;
      }

      void createSparseTableForBlockMinima() {
         std::vector< dip::uint > minima( blocks_.size() );
         for( dip::uint i = 0; i < blocks_.size(); ++i ) {
            minima[ i ] = data_[ blocks_[ i ].getIndexOfMinVal() ];
         }
         blockMinima_ = { minima };
      }
};

/// \brief Solves the lowest common ancestor problem for a tree.
class LowestCommonAncestorSolver {
   public:
   /// \brief The constructor takes a `graph`, which must not have any cycles in it (it must be a tree). The
   /// easiest way to turn an arbitrary graph into a tree is to compute the MST (see \ref dip::Graph::MinimumSpanningForest).
   LowestCommonAncestorSolver( Graph const& graph );
   LowestCommonAncestorSolver( LowestCommonAncestorSolver&& ) = default;
   LowestCommonAncestorSolver& operator=( LowestCommonAncestorSolver&& ) = default;
   // Prevent copying, that might go wrong because we use a shared pointer, `rmq_` might be shared...
   LowestCommonAncestorSolver( LowestCommonAncestorSolver const& ) = delete;
   LowestCommonAncestorSolver& operator=( LowestCommonAncestorSolver const& ) = delete;
   ~LowestCommonAncestorSolver() = default;

   /// \brief Returns the vertex that is the nearest common ancestor to vertices `a` and `b`.
   dip::uint GetLCA( dip::uint a, dip::uint b ) const;

   /// \brief Returns the value associated to the vertex `index`. TODO: describe this value!
   dfloat GetLogF( dip::uint index ) const {
      DIP_ASSERT( index < logF_.size() );
      return logF_[ index ];
   }

   private:
   std::vector< dip::uint > tourArray_;
   std::vector< dip::uint > R_;
   std::vector< dfloat > logF_;
   std::shared_ptr< RangeMinimumQuery > rmq_; // Hidden implementation to avoid all that cruft in these headers...
};

constexpr dip::uint notVisited = std::numeric_limits< dip::uint >::max();

dip::uint LowestCommonAncestorSolver::GetLCA( dip::uint a, dip::uint b ) const {
   DIP_ASSERT( a < R_.size() );
   DIP_ASSERT( b < R_.size() );
   dip::uint i = R_[ a ];
   dip::uint j = R_[ b ];
   DIP_ASSERT( i != notVisited );
   DIP_ASSERT( j != notVisited );
   if( j < i ) {
      std::swap( i, j );
   }
   return tourArray_[ rmq_->getIndexOfMinimum( i, j ) ];
}

LowestCommonAncestorSolver::LowestCommonAncestorSolver( Graph const& graph )
      : R_( graph.NumberOfVertices(), notVisited ), logF_( graph.NumberOfVertices(), 0.0 ) {
   // Euler tour
   dip::uint nelem = graph.NumberOfVertices();
   tourArray_.reserve( 2 * nelem );
   std::vector< dip::uint > eulerDepth;
   eulerDepth.reserve( 2 * nelem );
   std::vector< dip::uint > D( nelem, 0 );
   std::stack< dip::uint > Q;
   Q.push( 0 ); // Push root onto LIFO queue
   logF_[ 0 ] = 0.0;
   while( !Q.empty() ) {
      dip::uint vertex = Q.top();
      Q.pop();
      tourArray_.push_back( vertex );
      eulerDepth.push_back( D[ vertex ] );
      if( R_[ vertex ] == notVisited ) {
         R_[ vertex ] = tourArray_.size() - 1;
         for( auto edge : graph.EdgeIndices( vertex ) ) {
            Graph::VertexIndex otherVertex = graph.OtherVertex( edge, vertex );
            if( R_[ otherVertex ] == notVisited ) {
               logF_[ otherVertex ] = logF_[ vertex ] + std::log( 1 - graph.EdgeWeight( edge ));
               D[ otherVertex ] = D[ vertex ] + 1;
               Q.push( vertex );
               Q.push( otherVertex );
            }
         }
      }
   }
   // Create Range minimum query data structure
   rmq_ = std::make_shared< RangeMinimumQuery >( std::move( eulerDepth ));
}


class ExactSWLineFilter : public Framework::ScanLineFilter {
   public:
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint /**/ ) override { return 100; } // TODO: this is absolutely a wild guess...
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         sfloat* out = static_cast< sfloat* >( params.outBuffer[ 0 ].buffer );
         auto stride = params.outBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength - 1;
         dip::uint dim = params.dimension;
         dip::uint nDims = sizes_.size();
         DIP_ASSERT( params.position.size() == nDims );
         dip::uint index = Image::Index( params.position, sizes_ );
         UnsignedArray indexStrides( nDims );
         indexStrides[ 0 ] = 1;
         for( dip::uint jj = 1; jj < nDims; ++jj ) {
            indexStrides[ jj ] = indexStrides[ jj - 1 ] * sizes_[ jj - 1 ];
         }
         BooleanArray process( nDims, true );
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            process[ jj ] = params.position[ jj ] < ( sizes_[ jj ] - 1 );
         }
         for( dip::uint ii = 0; ii < length; ++ii, index += indexStrides[ dim ], out += stride ) {
            *out = ComputePixel( index, process, indexStrides, nDims );
         }
         process[ dim ] = false; // For the last pixel in the array, we don't have a next.
         *out = ComputePixel( index, process, indexStrides, nDims );
      }
      ExactSWLineFilter( LowestCommonAncestorSolver const& lca, UnsignedArray const& sizes ) : lca_( lca ), sizes_( sizes ) {}
   private:
      LowestCommonAncestorSolver const& lca_;
      UnsignedArray const& sizes_;

      sfloat ComputePixel( dip::uint index, BooleanArray const& process, UnsignedArray const& indexStrides, dip::uint nDims ) {
         dfloat logPv = 0.0;
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            if( process[ jj ] ) {
               dip::uint neighborIndex = index + indexStrides[ jj ];
               dip::uint rootIndex = lca_.GetLCA( index, neighborIndex );
               dfloat logFthis = lca_.GetLogF( index );
               dfloat logFthat = lca_.GetLogF( neighborIndex );
               dfloat logFroot = lca_.GetLogF( rootIndex );
               dfloat logOneMinusPe = logFthis + logFthat - 2 * logFroot;
               logPv = logPv + logOneMinusPe;
            }
         }
         return static_cast< sfloat >( 1.0 - std::exp( logPv ));
      }
};

void ExactStochasticWatershed(
      Image const& in,
      Image& out,
      dfloat density
) {
   // Calculate minimum spanning tree
   Graph graph( in, 1, "average" );
   DIP_STACK_TRACE_THIS( graph = MinimumSpanningForest( graph ));

   // Compute Stochastic Watershed weights
   {
      dfloat nSeeds = static_cast< dfloat >( in.NumberOfPixels() ) * density;
      dip::uint nVertices = graph.NumberOfVertices();
      DIP_ASSERT( nVertices > 0 );
      Graph result( nVertices );
      UnionFind< Graph::VertexIndex, dip::uint, std::plus<> > ds( nVertices, 1, std::plus<>() );
      auto const& edges = graph.Edges();
      auto Comparator = [ & ]( Graph::EdgeIndex lhs, Graph::EdgeIndex rhs ) { return edges[ lhs ].weight < edges[ rhs ].weight; };
      std::vector< Graph::EdgeIndex > edgeIndices( edges.size() );
      std::iota( edgeIndices.begin(), edgeIndices.end(), 0 );
      std::sort( edgeIndices.begin(), edgeIndices.end(), Comparator );
      for( auto index : edgeIndices ) {
         auto edge = edges[ index ];
         Graph::VertexIndex p_index = ds.FindRoot( edge.vertices[ 0 ] );
         Graph::VertexIndex q_index = ds.FindRoot( edge.vertices[ 1 ] );
         dfloat p_size = static_cast< dfloat >( ds.Value( p_index )) / static_cast< dfloat >( nVertices );
         dfloat q_size = static_cast< dfloat >( ds.Value( q_index )) / static_cast< dfloat >( nVertices );
         // Calculate edge weight based on p_size and q_size
         edge.weight = 1 - std::pow( 1 - p_size, nSeeds ) - std::pow( 1 - q_size, nSeeds ) + std::pow( 1 - ( p_size + q_size ), nSeeds );
         result.AddEdgeNoCheck( edge );
         ds.Union( p_index, q_index );
      }
      std::swap( graph, result );
   }

   DIP_START_STACK_TRACE
      // Compute support data
      LowestCommonAncestorSolver lca( graph );
      // Calculate boundary probability for all pixels
      out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DONT_ALLOW );
      ExactSWLineFilter lineFilter( lca, out.Sizes() );
      Framework::ScanSingleOutput( out, DT_SFLOAT, lineFilter, Framework::ScanOption::NeedCoordinates );
   DIP_END_STACK_TRACE
}

} // namespace

void StochasticWatershed(
      Image const& c_in,
      Image& out,
      Random& random,
      dip::uint nSeeds,
      dip::uint nIterations,
      dfloat noise,
      String const& seeds
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( c_in.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( nSeeds == 0, E::INVALID_PARAMETER );
   dfloat density = static_cast< dfloat >( nSeeds ) / static_cast< dfloat >( c_in.NumberOfPixels() );
   if(( seeds == S::EXACT ) || ( nIterations == 0 )) {
      if( noise > 0 ) {
         Image tmp( c_in.Sizes(), 3, DT_SFLOAT );
         Image noisy;
         for( dip::uint ii = 0; ii < 3; ++ii ) {
            UniformNoise( c_in, noisy, random, 0.0, noise );
            Image tmp_out = tmp[ ii ];
            tmp_out.Protect();
            ExactStochasticWatershed( noisy, tmp_out, density );
         }
         tmp.Protect();
         Gauss( tmp, tmp, { 0.8 }, { 0 }, "fir" );
         GeometricMeanTensorElement( tmp, out );
      } else {
         ExactStochasticWatershed( c_in, out, density );
      }
      return;
   }
   bool poisson = seeds == S::POISSON;
   Image in = c_in; // NOLINT(*-unnecessary-copy-initialization)
   if( out.Aliases( in )) {
      out.Strip();
   }
   out.ReForge( in, DT_LABEL, Option::AcceptDataTypeChange::DO_ALLOW );
   out.Fill( 0 );
   Image grid = in.Similar( DT_BIN );
   Image edges = in.Similar( DT_BIN );
   Image noisy = noise > 0.0 ? in.Similar( DataType::SuggestFloat( in.DataType() )) : in.QuickCopy();
   for( dip::uint iter = 0; iter < nIterations; ++iter ) {
      if( poisson ) {
         FillPoissonPointProcess( grid, random, density );
      } else {
         FillRandomGrid( grid, random, density, seeds, S::ROTATION );
      }
      if( noise > 0.0 ) {
         UniformNoise( in, noisy, random, 0.0, noise );
      }
      SeededWatershed( noisy, grid, {}, edges, 1, -1 /* no merging */ );
      out += edges;
   }
}

} // namespace dip
