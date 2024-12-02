/*
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

#include "diplib/graph.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/label_map.h"
#include "diplib/overload.h"
#include "diplib/union_find.h"


namespace dip {

namespace {

void AddEdgeToGraph( Graph& graph, Graph::VertexIndex v1, Graph::VertexIndex v2, dfloat weight) {
   graph.AddEdgeNoCheck( v1, v2, weight );
}

void AddEdgeToGraph( DirectedGraph& graph, DirectedGraph::VertexIndex v1, DirectedGraph::VertexIndex v2, dfloat weight) {
   graph.AddEdgePairNoCheck( v1, v2, weight );
}

template< typename GraphType, typename TPI >
class CreateGenericGraphLineFilter : public Framework::ScanLineFilter {
   public:
      CreateGenericGraphLineFilter( GraphType& graph, UnsignedArray const& sizes, IntegerArray const& strides, bool useDifferences )
            : graph_( graph ), sizes_( sizes ), strides_( strides ), useDifferences_( useDifferences ) {}
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
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
                  AddEdgeToGraph( graph_, index, neighborIndex, weight );
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
               AddEdgeToGraph( graph_, index, neighborIndex, weight );
            }
         }
      }
   private:
      GraphType& graph_;
      UnsignedArray const& sizes_;
      IntegerArray const& strides_;
      bool useDifferences_;
};

template< typename TPI >
using CreateGraphLineFilter = CreateGenericGraphLineFilter< Graph, TPI >;

template< typename TPI >
using CreateDirectedGraphLineFilter = CreateGenericGraphLineFilter< DirectedGraph, TPI >;

} // namespace

Graph::Graph( Image const& image, dip::uint connectivity, String const& weights )
      : Graph( image.NumberOfPixels(), 2 * image.Dimensionality() ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !image.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( image.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity != 1, E::NOT_IMPLEMENTED );
   bool useDifferences{};
   DIP_STACK_TRACE_THIS( useDifferences = BooleanFromString( weights, "difference", "average" ));
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, CreateGraphLineFilter, ( *this, image.Sizes(), image.Strides(), useDifferences ), image.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( image, {}, image.DataType(), *lineFilter,
         Framework::ScanOption::NoMultiThreading + Framework::ScanOption::NeedCoordinates ));
}

DirectedGraph::DirectedGraph( Image const& image, dip::uint connectivity, String const& weights )
      : DirectedGraph( image.NumberOfPixels(), 2 * image.Dimensionality() ) {
   DIP_THROW_IF( !image.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !image.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !image.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( image.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( connectivity != 1, E::NOT_IMPLEMENTED );
   bool useDifferences{};
   DIP_STACK_TRACE_THIS( useDifferences = BooleanFromString( weights, "difference", "average" ));
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, CreateDirectedGraphLineFilter, ( *this, image.Sizes(), image.Strides(), useDifferences ), image.DataType() );
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( image, {}, image.DataType(), *lineFilter,
         Framework::ScanOption::NoMultiThreading + Framework::ScanOption::NeedCoordinates ));
}

DirectedGraph::DirectedGraph( Graph const& graph )
   : DirectedGraph( graph.NumberOfVertices() ) {
   for( dip::uint ii = 0; ii < graph.NumberOfVertices(); ++ii ) {
      vertices_[ ii ].value = graph.VertexValue( ii );
   }
   auto const& e = graph.Edges();
   for( dip::uint ii = 0; ii < graph.NumberOfEdges(); ++ii ) {
      if( e[ ii ].IsValid() ) {
         AddEdgePairNoCheck( e[ ii ].vertices[ 0 ], e[ ii ].vertices[ 1 ], e[ ii ].weight );
      }
   }
}

Graph MinimumSpanningForest( Graph const& graph, std::vector< Graph::VertexIndex > const& roots ) {
#ifdef DIP_CONFIG_ENABLE_ASSERT
   for( auto r : roots ) {
      DIP_ASSERT( r < graph.NumberOfVertices() );
   }
#endif
   using EdgeIndex = Graph::EdgeIndex;
   using VertexIndex = Graph::VertexIndex;
   Graph msf( graph.NumberOfVertices() );
   for( dip::uint ii = 0; ii < graph.NumberOfVertices(); ++ii ) {
      msf.VertexValue( ii ) = graph.VertexValue( ii );
   }
   std::vector< bool > visited( graph.NumberOfVertices(), false );
   auto Comparator = [ & ]( EdgeIndex lhs, EdgeIndex rhs ) {
      return graph.EdgeWeight( lhs ) > graph.EdgeWeight( rhs );
   }; // NOTE! inverted order to give higher priority to lower weights
   std::priority_queue< EdgeIndex, std::vector< EdgeIndex >, decltype( Comparator ) > queue( Comparator );
   if( roots.empty() ) {
      visited[ 0 ] = true;
      for( auto index : graph.EdgeIndices( 0 )) {
         queue.push( index );
      }
   } else {
      for( auto q : roots ) {
         if( !visited[ q ] ) {
            visited[ q ] = true;
            for( auto index : graph.EdgeIndices( q )) {
               queue.push( index );
            }
         }
      }
   }
   while( !queue.empty() ) {
      EdgeIndex edgeIndex = queue.top();
      queue.pop();
      VertexIndex q = graph.EdgeVertex( edgeIndex, 0 );
      if( visited[ q ] ) {
         q = graph.EdgeVertex( edgeIndex, 1 ); // try the other end then
      }
      if( !visited[ q ] ) {
         visited[ q ] = true;
         msf.AddEdgeNoCheck( graph.Edges()[ edgeIndex ] );
         for( auto index : graph.EdgeIndices( q )) {
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

LabelMap Label( Graph const& graph ) {
   SimpleUnionFind< Graph::EdgeIndex > regions( graph.NumberOfVertices() );
   for( auto& edge: graph.Edges() ) {
      if( edge.IsValid() ) {
         regions.Union( edge.vertices[ 0 ], edge.vertices[ 1 ] );
      }
   }
   regions.Relabel();
   return LabelMap( regions );
}

LabelMap Label( DirectedGraph const& graph ) {
   SimpleUnionFind< DirectedGraph::EdgeIndex > regions( graph.NumberOfVertices() );
   for( auto& edge: graph.Edges() ) {
      if( edge.IsValid() ) {
         regions.Union( edge.source, edge.target );
      }
   }
   regions.Relabel();
   return LabelMap( regions );
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
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
   graph = dip::MinimumSpanningForest( graph );
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

DOCTEST_TEST_CASE("[DIPlib] testing dip::DirectedGraph") {
   dip::Image img( { 4, 5 }, 1, dip::DT_UINT8 );
   img.Fill( 0 );
   img.At( 0 ) = 10;
   img.At( 1 ) = 12;
   img.At( 2 ) = 15;
   // Create a directed graph directly
   dip::DirectedGraph graph1( img, 1, "difference" );
   // Create an undirected graph (tested elsewhere) and convert to directed graph
   dip::DirectedGraph graph2 = dip::DirectedGraph( dip::Graph( img, 1, "difference" ));
   // We expect the two to produce identical results (with edges and vertices stored in same order).
   DOCTEST_REQUIRE( graph1.NumberOfVertices() == graph2.NumberOfVertices() );
   for( dip::uint ii = 0; ii < graph1.NumberOfVertices(); ++ii ) {
      DOCTEST_CHECK( graph1.VertexValue( ii ) == graph2.VertexValue( ii ));
      auto const& edges1 = graph1.EdgeIndices( ii );
      auto const& edges2 = graph2.EdgeIndices( ii );
      DOCTEST_REQUIRE( edges1.size() == edges2.size() );
      for( dip::uint jj = 0; jj < edges1.size(); ++jj ) {
         DOCTEST_CHECK( edges1[ jj ] == edges2[ jj ] );
      }
   }
   DOCTEST_REQUIRE( graph1.NumberOfEdges() == graph2.NumberOfEdges() );
   for( dip::uint ii = 0; ii < graph1.NumberOfEdges(); ++ii ) {
      DOCTEST_CHECK( graph1.SourceVertex( ii ) == graph2.SourceVertex( ii ));
      DOCTEST_CHECK( graph1.TargetVertex( ii ) == graph2.TargetVertex( ii ));
      DOCTEST_CHECK( graph1.SiblingEdge( ii ) == graph2.SiblingEdge( ii ));
      DOCTEST_CHECK( graph1.EdgeWeight( ii ) == graph2.EdgeWeight( ii ));
   }
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
