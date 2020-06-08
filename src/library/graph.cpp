/*
 * DIPlib 3.0
 * This file contains definitions for graph algorithms.
 *
 * (c)2013, Filip Malmberg.
 * (c)2019, Cris Luengo.
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
#include "diplib/graph.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

#include <queue>
#include <stack>

namespace dip {

namespace {

template< typename TPI >
class CreateGraphLineFilter : public Framework::ScanLineFilter {
   public:
      CreateGraphLineFilter( Graph& graph, UnsignedArray const& sizes, IntegerArray const& strides, bool useDifferences )
            : graph_( graph ), sizes_( sizes ), strides_( strides ), useDifferences_( useDifferences ) {}
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength - 1;
         dip::uint dim = params.dimension;
         dip::uint nDims = sizes_.size();
         DIP_ASSERT( params.position.size() == nDims );
         DIP_ASSERT( strides_[ dim ] == stride );
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
         for( dip::uint ii = 0; ii < length; ++ii, index += indexStrides[ dim ], in += stride ) {
            // Add to graph_ links to each of the *forward* neighbors (i.e. those that you can reach by incrementing
            // one of the coordinates). The other neighbors are already linked to when those neighbors were processed
            dfloat value = static_cast< dfloat >( in[ 0 ] );
            graph_.VertexValue( index ) = value;
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               if( process[ jj ] ) {
                  dip::uint neighborIndex = index + indexStrides[ jj ];
                  dfloat neighborValue = static_cast< dfloat >( in[ strides_[ jj ]] );
                  dfloat weight = useDifferences_ ? std::abs( value - neighborValue ) : ( value + neighborValue ) / 2;
                  graph_.AddEdgeNoCheck( index, neighborIndex, weight );
               }
            }
         }
         process[ dim ] = false;
         dfloat value = static_cast< dfloat >( in[ 0 ] );
         graph_.VertexValue( index ) = value;
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            if( process[ jj ] ) {
               dip::uint neighborIndex = index + indexStrides[ jj ];
               dfloat neighborValue = static_cast< dfloat >( in[ strides_[ jj ]] );
               dfloat weight = useDifferences_ ? std::abs( value - neighborValue ) : ( value + neighborValue ) / 2;
               graph_.AddEdgeNoCheck( index, neighborIndex, weight );
            }
         }
      }
   private:
      Graph& graph_;
      UnsignedArray const& sizes_;
      IntegerArray const& strides_;
      bool useDifferences_;
};

} // namespace

Graph::Graph( Image const& image, dip::uint connectivity, String const& weights )
      : Graph( image.NumberOfPixels(), 2 * image.Dimensionality() ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !image.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( image.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity != 1, E::NOT_IMPLEMENTED );
   bool useDifferences;
   DIP_STACK_TRACE_THIS( useDifferences = BooleanFromString( weights, "difference", "average" ));
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, CreateGraphLineFilter, ( *this, image.Sizes(), image.Strides(), useDifferences ), image.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( image, {}, image.DataType(), *lineFilter,
         Framework::ScanOption::NoMultiThreading + Framework::ScanOption::NeedCoordinates ));
}

Graph Graph::MinimumSpanningForest( std::vector< dip::uint > const& roots ) const {
#ifdef DIP__ENABLE_ASSERT
   for( auto r : roots ) {
      DIP_ASSERT( r < NumberOfVertices() );
   }
#endif
   Graph msf( NumberOfVertices() );
   for( dip::uint ii = 0; ii < NumberOfVertices(); ++ii ) {
      msf.vertices_[ ii ].value = vertices_[ ii ].value;
   }
   std::vector< bool > visited( NumberOfVertices(), false );
   auto Comparator = [ & ]( EdgeIndex lhs, EdgeIndex rhs ){ return edges_[ lhs ].weight > edges_[ rhs ].weight; }; // NOTE! inverted order to give higher priority to lower weights
   std::priority_queue< EdgeIndex, std::vector< EdgeIndex >, decltype( Comparator ) > queue( Comparator );
   if( roots.empty() ) {
      visited[ 0 ] = true;
      for( auto index : vertices_[ 0 ].edges ) {
         queue.push( index );
      }
   } else {
      for( auto q : roots ) {
         if( !visited[ q ] ) {
            visited[ q ] = true;
            for( auto index : vertices_[ q ].edges ) {
               queue.push( index );
            }
         }
      }
   }
   while( !queue.empty() ) {
      EdgeIndex edgeIndex = queue.top();
      queue.pop();
      VertexIndex q = edges_[ edgeIndex ].vertices[ 0 ];
      if( visited[ q ] ) {
         q = edges_[ edgeIndex ].vertices[ 1 ]; // try the other end then
      }
      if( !visited[ q ] ) {
         visited[ q ] = true;
         msf.AddEdgeNoCheck( edges_[ edgeIndex ] );
         for( auto index : vertices_[ q ].edges ) {
            queue.push( index );
         }
      }
   }
   return msf;
}

void Graph::RemoveLargestEdges( dip::uint number ) {
   if( number == 0 ) {
      // Nothing to do
      return;
   }
   // Generate list of valid edges
   std::vector< EdgeIndex > indices;
   indices.reserve( edges_.size() );
   for( EdgeIndex ii = 0; ii < edges_.size(); ++ii ) {
      if( edges_[ ii ].IsValid() ) {
         indices.push_back( ii );
      }
   }
   // Sort indices to edges, largest first
   number = std::min( number, indices.size() ); // If number is too large, we will delete all edges.
   dip::sint element = static_cast< dip::sint >( number ) - 1;
   std::nth_element( indices.begin(), indices.begin() + element, indices.end(), [ this ]( EdgeIndex lhs, EdgeIndex rhs ){
      return edges_[ lhs ].weight > edges_[ rhs ].weight;
   } );
   // Delete largest edges
   for( dip::uint ii = 0; ii < number; ++ii ) {
      DeleteEdge( indices[ ii ] );
   }
}

namespace {

template< typename T >
class Matrix {
   public:
      Matrix() = default;
      Matrix( dip::uint xSize, dip::uint ySize, T value = {} ) : m_( xSize * ySize, value ), xSize_( xSize ), ySize_( ySize ) {}

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
      std::vector< T > m_;
      dip::uint xSize_ = 0;
      dip::uint ySize_ = 0;
};

template< typename T >
class SymmetricMatrix {
   public:
      SymmetricMatrix() = default;
      SymmetricMatrix( dip::uint size, T value = {} ) : m_( size * ( size + 1 ) / 2, value ), size_( size ) {}

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
      std::vector< T > m_;
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
      std::vector< T > sequence_;
      Matrix< dip::uint > sparseMatrix_;
};

template< typename T >
class LookUpTable {
   public:
      LookUpTable() = default;
      LookUpTable( std::vector< T > sequence ) {
         DIP_ASSERT( !sequence.empty() );
         sequence_ = std::move( sequence );
         // Normalize sequence
         T minVal = *min_element( sequence_.begin(), sequence_.end() );
         for( auto& s : sequence_ ) {
            s -= minVal;
         }
         if( sequence_.size() == 1 ) {
            table_ = { 1 };
         } else {
            sparseTable_ = { sequence_ };
            //table_ = { sequence_.size(), sequence_.size() }; // TODO: This is a huge table!? Only half of it is used.
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
      bool isInitialized() const {
         return !sequence_.empty();
      }

      dip::uint getEntry( dip::uint x, dip::uint y ) const {
         return table_.at( x, y );
      }

   private:
      std::vector< T > sequence_;
      SymmetricMatrix< dip::uint > table_;
      SparseTable< T > sparseTable_;
};

class Block {
   public:
      LookUpTable< dip::uint > lut;

      Block() = default;
      Block( LookUpTable< dip::uint > lut, dip::uint sequenceLength, dip::uint firstIndex ) {
         this->lut = std::move( lut );
         lastIndex_internal_ = sequenceLength - 1;
         firstIndex_external_ = firstIndex;
      }

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
      dip::uint lastIndex_internal_ = 0;
      dip::uint firstIndex_external_ = 0;
};

} // namespace

class RangeMinimumQuery {
   public:
      RangeMinimumQuery( std::vector< dip::uint > data )  {
         data_ = std::move( data );
         dip::uint nelem = data_.size();
         luts_.resize( static_cast< dip::uint >( std::ceil( std::sqrt( static_cast< dfloat >( nelem )))));
         blockLength_ = static_cast< dip::uint >( std::ceil( std::log2( nelem ) / 2 ));
         dip::uint numBlocks = static_cast< dip::uint >( std::ceil( static_cast< dfloat >( nelem ) / static_cast< dfloat >( blockLength_ )));
         blocks_.resize( numBlocks );
         createBlocks();
         createSparseTableForBlockMinima();
      }

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

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing dip::Graph") {
   dip::Image img( { 4, 5 }, 1, dip::DT_UINT8 );
   img.Fill( 0 );
   img.At( 0 ) = 10;
   img.At( 1 ) = 12;
   img.At( 2 ) = 15;

   // Test graph creation
   dip::Graph graph( img, 1, "difference" );
   DOCTEST_REQUIRE( graph.NumberOfVertices() == 20 );
   DOCTEST_REQUIRE( graph.Edges().size() == 16 + 15 ); // 4*4 vertical edges + 3*5 horizontal edges
   auto const& edges = graph.Edges();
   // Check 1st vertex
   DOCTEST_REQUIRE( graph.EdgeIndices( 0 ).size() == 2 );
   auto edge1 = edges[ graph.EdgeIndices( 0 )[ 0 ]];
   auto edge2 = edges[ graph.EdgeIndices( 0 )[ 1 ]];
   if( edge1.vertices[ 1 ] != 1 ) {
      std::swap( edge1, edge2 );
   }
   DOCTEST_CHECK( edge1.vertices[ 0 ] == 0 );
   DOCTEST_CHECK( edge1.vertices[ 1 ] == 1 );
   DOCTEST_CHECK( edge1.weight == 2.0 );
   DOCTEST_CHECK( edge2.vertices[ 0 ] == 0 );
   DOCTEST_CHECK( edge2.vertices[ 1 ] == 4 );
   DOCTEST_CHECK( edge2.weight == 10.0 );
   // Check 2nd vertex
   DOCTEST_REQUIRE( graph.EdgeIndices( 1 ).size() == 3 );
   edge1 = edges[ graph.EdgeIndices( 1 )[ 0 ]];
   edge2 = edges[ graph.EdgeIndices( 1 )[ 1 ]];
   auto edge3 = edges[ graph.EdgeIndices( 1 )[ 2 ]];
   if( edge1.vertices[ 1 ] != 2 ) {
      std::swap( edge1, edge2 );
      if( edge1.vertices[ 1 ] != 2 ) {
         std::swap( edge1, edge3 );
      }
   }
   if( edge2.vertices[ 1 ] != 5 ) {
      std::swap( edge2, edge3 );
   }
   DOCTEST_CHECK( edge1.vertices[ 0 ] == 1 );
   DOCTEST_CHECK( edge1.vertices[ 1 ] == 2 );
   DOCTEST_CHECK( edge1.weight == 3.0 );
   DOCTEST_CHECK( edge2.vertices[ 0 ] == 1 );
   DOCTEST_CHECK( edge2.vertices[ 1 ] == 5 );
   DOCTEST_CHECK( edge2.weight == 12.0 );

   // Test MinimumSpanningForest()
   graph = graph.MinimumSpanningForest();
   DOCTEST_REQUIRE( graph.Edges().size() == 19 );
   DOCTEST_REQUIRE( graph.NumberOfVertices() == 20 );
   auto edges0 = graph.EdgeIndices( 0 ); // Joined to 1 and 4
   auto edges1 = graph.EdgeIndices( 1 ); // Joined to 0 and 2
   auto edges2 = graph.EdgeIndices( 2 ); // Joined to 1
   DOCTEST_REQUIRE( edges0.size() == 2 );
   DOCTEST_REQUIRE( edges1.size() == 2 );
   DOCTEST_REQUIRE( edges2.size() == 1 );
   auto v1 = graph.OtherVertex( edges0[ 0 ], 0 );
   auto v2 = graph.OtherVertex( edges0[ 1 ], 0 );
   if( v1 == 1 ) {
      DOCTEST_CHECK( v2 == 4 );
   } else {
      DOCTEST_CHECK( v1 == 4 );
      DOCTEST_CHECK( v2 == 1 );
   }
   v1 = graph.OtherVertex( edges1[ 0 ], 1 );
   v2 = graph.OtherVertex( edges1[ 1 ], 1 );
   if( v1 == 0 ) {
      DOCTEST_CHECK( v2 == 2 );
   } else {
      DOCTEST_CHECK( v1 == 2 );
      DOCTEST_CHECK( v2 == 0 );
   }
   DOCTEST_CHECK( graph.OtherVertex( edges2[ 0 ], 2 ) == 1 );
}

#endif // DIP__ENABLE_DOCTEST
