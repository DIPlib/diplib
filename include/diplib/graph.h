/*
 * (c)2013, Filip Malmberg.
 * (c)2019-2024, Cris Luengo.
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

#ifndef DIP_GRAPH_H
#define DIP_GRAPH_H

#include <array>
#include <cstdlib>
#include <utility>

#include "diplib.h"
#include "diplib/label_map.h"


/// \file
/// \brief Representing graphs and working with an image as a graph.
/// See \ref graphs.


namespace dip {


/// \group graphs Graphs
/// \ingroup segmentation
/// \brief Representing graphs and working with an image as a graph.
///
/// \see dip::RegionAdjacencyGraph, dip::Relabel(dip::Image const&, dip::Image&, dip::Graph const&)
///
/// \addtogroup


/// \brief A non-directed, edge-weighted graph.
///
/// Vertices are identified by an index, these indices are expected to be consecutive. Each vertex contains a list
/// of indices to edges, and has an optional value.
///
/// Edges are represented by indices to two vertices, and a double precision floating-point weight.
/// Not all edges are actually used; use \ref Edge::IsValid to test this.
class DIP_NO_EXPORT Graph {
   public:
      /// Type for indices to vertices
      using VertexIndex = dip::uint;
      /// Type for indices to edges
      using EdgeIndex = dip::uint;
      /// Type for list of edge indices
      using EdgeList = std::vector< EdgeIndex >;

      /// \brief A vertex in the graph
      struct Vertex {
         EdgeList edges{};             ///< The list of indices to edges
         mutable dfloat value = 0.0;   ///< The value associated to each vertex

         Vertex() = default;

         /// \brief Construct a vertex with reserved space for the given number of edges.
         explicit Vertex( dip::uint nEdges, dfloat value = 0.0 ) : edges(), value( value ) {
            edges.reserve( nEdges );
         }
      };

      /// \brief An edge in the graph
      ///
      /// If both vertices are 0, the edge is not valid (never used or deleted). Otherwise, `vertices[0] < vertices[1]`.
      struct Edge {
         std::array< VertexIndex, 2 > vertices = {{ 0, 0 }};   ///< The two vertices joined by this edge
         mutable dfloat weight = 0.0;                          ///< The weight of this edge
         bool IsValid() const {
            return vertices[ 0 ] < vertices[ 1 ];
         }
      };

      Graph() = default;

      /// \brief Construct a graph with `nVertices` vertices. Vertices are identified by their index, which
      /// is in the range [0,`nVertices`]. `nEdges` is the expected number of edges for each vertex, and is used
      /// to reserve space for them.
      explicit Graph( dip::uint nVertices, dip::uint nEdges = 0 ) : vertices_( nVertices, Vertex( nEdges )) {
         edges_.reserve( nVertices * nEdges / 2 );
      }

      /// \brief Construct a graph for the given image.
      ///
      /// Each pixel becomes a vertex in the graph, the vertex's index is equal to the linear index (see \ref pointers)
      /// of the pixel in the image (that is, vertices are stored in the same order as the pixels in the image with
      /// normal strides). Vertex values are set to the corresponding pixel value.
      ///
      /// An edge will connect each pixel to each of its neighbors.
      ///
      /// `connectivity` indicates which pixels are considered neighbors. Currently, only a connectivity of 1 is
      /// allowed. This makes neighbors the pixels at a city-block distance of 1 (in 2D, there are 4 such neighbors,
      /// in 3D there are 6).
      ///
      /// The value of `weights` is:
      ///  - `"difference"` (default): the edge weights are given by the absolute difference between the two pixel values.
      ///  - `"average"`: the edge weights are given by the average of the two pixel values.
      ///  - `"zero"`: the edge weights are all set to 0, use \ref UpdateEdgeWeights to compute weights in some other way,
      ///     or manually set the weights.
      DIP_EXPORT explicit Graph( Image const& image, dip::uint connectivity = 1, String const& weights = "difference" );

      /// \brief Returns the number of vertices in the graph.
      DIP_NODISCARD dip::uint NumberOfVertices() const {
         return vertices_.size();
      };

      /// \brief Returns the number of edges in the graph, including invalid edges.
      DIP_NODISCARD dip::uint NumberOfEdges() const {
         return edges_.size();
      };

      /// \brief Counts the number of valid edges in the graph.
      DIP_NODISCARD dip::uint CountEdges() const {
         dip::uint count = 0;
         for( auto& e: edges_ ) {
            if( e.IsValid() ) {
               ++count;
            }
         }
         return count;
      };

      /// \brief Gets the set of edges in the graph. The weights of the edges are mutable, they can be directly modified.
      /// Not all edges connect vertices, use \ref Edge::IsValid to test.
      DIP_NODISCARD std::vector< Edge > const& Edges() const {
         return edges_;
      }

      /// \brief Gets the index to one of the two vertices that are joined by an edge.
      /// `which` is 0 or 1 to specify which of the two vertices to return.
      DIP_NODISCARD VertexIndex EdgeVertex( EdgeIndex edge, bool which ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].vertices[ which ];
      }

      /// \brief Finds the index to the vertex that is joined to the vertex with index `vertex` through the edge with
      /// index `edge`.
      DIP_NODISCARD VertexIndex OtherVertex( EdgeIndex edge, VertexIndex vertex ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].vertices[ ( edges_[ edge ].vertices[ 0 ] != vertex ) ? 0 : 1 ];
      }

      /// \brief Returns a reference to the weight of the edge with index `edge`. This value is mutable even
      /// if the graph is `const`.
      DIP_NODISCARD dfloat& EdgeWeight( EdgeIndex edge ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].weight;
      }

      /// \brief Returns `true` if the edge is a valid edge.
      DIP_NODISCARD bool IsValidEdge( EdgeIndex edge ) const {
         return edge < edges_.size() ? edges_[ edge ].IsValid() : false;
      }

      /// \brief Get the indices to the edges that join vertex `vertex`.
      DIP_NODISCARD EdgeList const& EdgeIndices( VertexIndex vertex ) const {
         DIP_ASSERT( vertex < vertices_.size() );
         return vertices_[ vertex ].edges;
      }

      /// \brief Returns a reference to the value of the vertex `vertex`. This value is mutable even if the graph is `const`.
      DIP_NODISCARD dfloat& VertexValue( VertexIndex vertex ) const {
         DIP_ASSERT( vertex < vertices_.size() );
         return vertices_[ vertex ].value;
      }

      /// \brief Adds a vertex to the graph with the given weight and space reserved for the given number of edges.
      /// Returns the index to the new vertex.
      VertexIndex AddVertex( dip::uint nEdges = 0, dfloat weight = 0.0 ) {
         vertices_.emplace_back( nEdges, weight );
         return vertices_.size() - 1;
      }

      /// \brief Add an edge between vertices `vertex1` and `vertex2`, with weight `weight`. If the edge already exists,
      /// update the weight of the edge to be `weight`.
      void AddEdge( VertexIndex vertex1, VertexIndex vertex2, dfloat weight ) {
         DIP_THROW_IF( vertex1 == vertex2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge = FindEdge( vertex1, vertex2 );
         if( edge == edges_.size() ) {
            // Didn't find the edge, create it.
            AddEdgeNoCheck( vertex1, vertex2, weight );
         } else {
            // Found the edge, update the weight
            edges_[ edge ].weight = weight;
         }
      }

      /// \brief Add an edge between vertices `vertex1` and `vertex2`, with weight `weight`. If the edge already exists,
      /// update the weight of the edge by adding `weight` to the existing weight.
      void AddEdgeSumWeight( VertexIndex vertex1, VertexIndex vertex2, dfloat weight ) {
         DIP_THROW_IF( vertex1 == vertex2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge = FindEdge( vertex1, vertex2 );
         if( edge == edges_.size() ) {
            // Didn't find the edge, create it.
            AddEdgeNoCheck( vertex1, vertex2, weight );
         } else {
            // Found the edge, update the weight
            edges_[ edge ].weight += weight;
         }
      }

      /// \brief Delete the edge between vertices `vertex1` and `vertex2`.
      void DeleteEdge( VertexIndex vertex1, VertexIndex vertex2 ) {
         EdgeIndex edge = FindEdge( vertex1, vertex2 );
         if( edge < edges_.size() ) {
            DeleteEdge( edge );
         }
      }

      /// \brief Delete the edge `edge`.
      void DeleteEdge( EdgeIndex edge ) {
         DIP_ASSERT( edge < edges_.size() );
         auto& vertex1 = vertices_[ edges_[ edge ].vertices[ 0 ]];
         auto it = FindEdge( vertex1, edge );
         if( it != vertex1.edges.end() ) {
            vertex1.edges.erase( it );
         }
         auto& vertex2 = vertices_[ edges_[ edge ].vertices[ 1 ]];
         it = FindEdge( vertex2, edge );
         if( it != vertex2.edges.end() ) {
            vertex2.edges.erase( it );
         }
         edges_[ edge ].vertices = { 0, 0 };
      }

      /// \brief Returns a list of indices to neighboring vertices. The list is created. `EdgeIndices` is
      /// a more efficient, but less convenient, function.
      std::vector< VertexIndex > Neighbors( VertexIndex vertex ) {
         DIP_ASSERT( vertex < vertices_.size() );
         std::vector< VertexIndex > neighbors;
         neighbors.reserve( vertices_[ vertex ].edges.size() );
         for( auto edge : vertices_[ vertex ].edges ) {
            neighbors.push_back( OtherVertex( edge, vertex ));
         }
         return neighbors;
      }

      // Adds an edge. Doesn't check for duplicates. If the edge already exists, disaster ensues.
      // And if vertex1 >= vertex2, disaster ensues.
      void AddEdgeNoCheck( Edge edge ) {
         DIP_ASSERT( edge.vertices[ 0 ] < edge.vertices[ 1 ] );
         EdgeIndex ii = edges_.size();
         vertices_[ edge.vertices[ 0 ]].edges.push_back( ii );
         vertices_[ edge.vertices[ 1 ]].edges.push_back( ii );
         edges_.push_back( edge );
      }
      void AddEdgeNoCheck( VertexIndex vertex1, VertexIndex vertex2, dfloat weight ) {
         AddEdgeNoCheck( {{ vertex1, vertex2 }, weight } );
      }

      /// \brief Re-computes edge weights using the function `func`, called as `dfloat func(dfloat val1, dfloat val2)`,
      /// where the two inputs to `func` are the value of the two vertices.
      template< typename F >
      void UpdateEdgeWeights( F func ) const {
         for( auto& edge: edges_ ) {
            edge.weight = func( vertices_[ edge.vertices[ 0 ]].value, vertices_[ edge.vertices[ 1 ]].value );
         }
      }

      /// \brief Re-computes edge weights as the absolute difference between vertex values.
      void UpdateEdgeWeights() const {
         UpdateEdgeWeights( []( dfloat val1, dfloat val2 ) { return std::abs( val1 - val2 ); } );
      }

      /// \brief Computes the minimum spanning forest (MSF) using Prim's algorithm.
      /// See \ref dip::MinimumSpanningForest for details. Does not modify `*this*.
      // NOTE: We're keeping this for backwards-compatibility.
      DIP_NODISCARD Graph MinimumSpanningForest( std::vector< VertexIndex > const& roots = {} ) const;

      /// \brief Removes `number` edges with the largest weights from the graph.
      ///
      /// If the graph is a minimum spanning tree, it will be converted to a minimum spanning forest with
      /// `number + 1` trees. This is a segmentation of the tree into the `number + 1` regions with smallest
      /// trees.
      DIP_EXPORT void RemoveLargestEdges( dip::uint number );

   private:
      std::vector< Vertex > vertices_{};
      std::vector< Edge > edges_{};

      // Return an iterator to the edge index within `vertex`. Returns `vertex.end()` if not found.
      static EdgeList::iterator FindEdge( Vertex& vertex, EdgeIndex edge ) {
         EdgeList::iterator it = vertex.edges.begin();
         for( ; ( it != vertex.edges.end() ) && ( *it != edge ); ++it ) {}
         return it;
      }

      // Return the index to the edge that joins vertices `vertex1` and `vertex2`.
      // If the edge doesn't exist, returns edges_.size().
      // `vertex1` and `vertex2` might be swapped by this function too.
      EdgeIndex FindEdge( VertexIndex& vertex1, VertexIndex& vertex2 ) {
         DIP_ASSERT( vertex1 < vertices_.size() );
         DIP_ASSERT( vertex2 < vertices_.size() );
         DIP_ASSERT( vertex1 != vertex2 );
         if( vertex1 > vertex2 ) {
            std::swap( vertex1, vertex2 );
         }
         for( auto edge : vertices_[ vertex1 ].edges ) {
            if( edges_[ edge ].vertices[ 1 ] == vertex2 ) {
               return edge;
            }
         }
         return edges_.size();
      }

};

/// \brief A directed, edge-weighted graph.
///
/// Vertices are identified by an index, these indices are expected to be consecutive. Each vertex contains a list
/// of indices to outgoing edges, and has an optional value.
///
/// Edges are represented by indices to two vertices (a source and a target), a double precision floating-point weight,
/// and an index to the sibling edge (the one that connects the same vertices in the other direction).
/// Not all edges are actually used; use \ref Edge::IsValid to test this.
class DIP_NO_EXPORT DirectedGraph {
   public:
      /// Type for indices to vertices
      using VertexIndex = dip::uint;
      /// Type for indices to edges
      using EdgeIndex = dip::uint;
      /// Type for list of edge indices
      using EdgeList = std::vector< EdgeIndex >;

      /// \brief A vertex in the graph
      struct Vertex {
         EdgeList edges{};             ///< The list of indices to outgoing edges
         mutable dfloat value = 0.0;   ///< The value associated to each vertex

         Vertex() = default;

         /// \brief Construct a vertex with reserved space for the given number of edges.
         explicit Vertex( dip::uint nEdges, dfloat value = 0.0 ) : edges(), value( value ) {
            edges.reserve( nEdges );
         }
      };

      /// \brief An edge in the graph
      ///
      /// If both vertices are equal, the edge is not valid (never used or deleted).
      ///
      /// If `Edge::sibling` is *this, there is no sibling.
      struct Edge {
         VertexIndex source = 0;       ///< The vertex that the edge starts at
         VertexIndex target = 0;       ///< The vertex that the edge goes to
         mutable dfloat weight = 0.0;  ///< The weight of this edge
         EdgeIndex sibling = 0;        ///< The index to the sibling edge (the one that connects target to source)
         bool IsValid() const {
            return source != target;
         }
      };

      DirectedGraph() = default;

      /// \brief Construct a directed graph with `nVertices` vertices. Vertices are identified by their index, which
      /// is in the range [0,`nVertices`]. `nEdges` is the expected number of edges for each vertex, and is used
      /// to reserve space for them.
      explicit DirectedGraph( dip::uint nVertices, dip::uint nEdges = 0 ) : vertices_( nVertices, Vertex( nEdges )) {
         edges_.reserve( nVertices * nEdges );
      }

      /// \brief Construct a directed graph for the given image.
      ///
      /// Each pixel becomes a vertex in the graph, the vertex's index is equal to the linear index (see \ref pointers)
      /// of the pixel in the image (that is, vertices are stored in the same order as the pixels in the image with
      /// normal strides). Vertex values are set to the corresponding pixel value.
      ///
      /// An edge will connect each pixel to each of its neighbors. Each edge is guaranteed to have a sibling.
      ///
      /// `connectivity` indicates which pixels are considered neighbors. Currently, only a connectivity of 1 is
      /// allowed. This makes neighbors the pixels at a city-block distance of 1 (in 2D, there are 4 such neighbors,
      /// in 3D there are 6).
      ///
      /// The value of `weights` is:
      ///  - `"difference"` (default): the edge weights are given by the absolute difference between the two pixel values.
      ///  - `"average"`: the edge weights are given by the average of the two pixel values.
      ///  - `"zero"`: the edge weights are all set to 0, use \ref UpdateEdgeWeights to compute weights in some other way,
      ///     or manually set the weights.
      ///
      /// If `extraEdges` is `"graphcut"`, then space will be reserved for the additional 2 vertices and all the edges
      /// and that the graph cut segmentation algorithm adds to the graph that this function makes.
      /// By default, `extraEdges` is `"none"`, and the memory allocated is just enough to contain the edges and
      /// vertices created by this function. It is always possible to add additional edges or vertices, but this
      /// incurs a reallocation cost.
      DIP_EXPORT explicit DirectedGraph( Image const& image, dip::uint connectivity = 1, String const& weights = "difference", String const& extraEdges = "none" );

      /// \brief Constructs a directed graph from an undirected graph.
      DIP_EXPORT explicit DirectedGraph( Graph const& graph );

      /// \brief Returns the number of vertices in the graph.
      DIP_NODISCARD dip::uint NumberOfVertices() const {
         return vertices_.size();
      };

      /// \brief Returns the number of edges in the graph, including invalid edges.
      DIP_NODISCARD dip::uint NumberOfEdges() const {
         return edges_.size();
      };

      /// \brief Counts the number of valid edges in the graph.
      DIP_NODISCARD dip::uint CountEdges() const {
         dip::uint count = 0;
         for( auto& e: edges_ ) {
            if( e.IsValid() ) {
               ++count;
            }
         }
         return count;
      };

      /// \brief Gets the set of edges in the graph. The weights of the edges are mutable, they can be directly modified.
      /// Not all edges connect vertices, use \ref Edge::IsValid to test.
      DIP_NODISCARD std::vector< Edge > const& Edges() const {
         return edges_;
      }

      /// \brief Finds the index to the vertex that is the source (origin) of the given edge.
      DIP_NODISCARD VertexIndex SourceVertex( EdgeIndex edge ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].source;
      }

      /// \brief Finds the index to the vertex that is the target (destination) of the given edge.
      DIP_NODISCARD VertexIndex TargetVertex( EdgeIndex edge ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].target;
      }

      /// \brief Finds the index to the edge that is the sibling (connects the same vertices in the other direction)
      /// of the given edge. If it returns `edge`, there is no sibling.
      DIP_NODISCARD EdgeIndex SiblingEdge( EdgeIndex edge ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].sibling;
      }

      /// \brief Returns a reference to the weight of the edge with index `edge`. This value is mutable even
      /// if the graph is `const`.
      DIP_NODISCARD dfloat& EdgeWeight( EdgeIndex edge ) const {
         DIP_ASSERT( edge < edges_.size() );
         return edges_[ edge ].weight;
      }

      /// \brief Returns `true` if the edge is a valid edge.
      DIP_NODISCARD bool IsValidEdge( EdgeIndex edge ) const {
         return edge < edges_.size() ? edges_[ edge ].IsValid() : false;
      }

      /// \brief Get the indices to the edges that start at vertex `vertex`.
      DIP_NODISCARD EdgeList const& EdgeIndices( VertexIndex vertex ) const {
         DIP_ASSERT( vertex < vertices_.size() );
         return vertices_[ vertex ].edges;
      }

      /// \brief Returns a reference to the value of the vertex `vertex`. This value is mutable even if the graph is `const`.
      DIP_NODISCARD dfloat& VertexValue( VertexIndex vertex ) const {
         DIP_ASSERT( vertex < vertices_.size() );
         return vertices_[ vertex ].value;
      }

      /// \brief Adds a vertex to the graph with the given weight and space reserved for the given number of edges.
      /// Returns the index to the new vertex.
      VertexIndex AddVertex( dip::uint nEdges = 0, dfloat weight = 0.0 ) {
         vertices_.emplace_back( nEdges, weight );
         return vertices_.size() - 1;
      }

      /// \brief Add an edge from vertex `source` to `target`, with weight `weight`. If the edge already exists,
      /// update its weight to be `weight`.
      void AddEdge( VertexIndex source, VertexIndex target, dfloat weight ) {
            DIP_THROW_IF( source == target, "Cannot create an edge between a vertex and itself" );
            EdgeIndex edge = FindEdge( source, target );
            if( edge == edges_.size() ) {
               // Didn't find the edge, create it.
               AddEdgeNoCheck( source, target, weight );
            } else {
               // Found the edge, update the weight
               edges_[ edge ].weight = weight;
            }
         }

      /// \brief Add an edge from vertex `source` to `target`, with weight `weight`. If the edge already exists,
      /// update its weight by adding `weight` to the existing weight.
      void AddEdgeSumWeight( VertexIndex source, VertexIndex target, dfloat weight ) {
            DIP_THROW_IF( source == target, "Cannot create an edge between a vertex and itself" );
            EdgeIndex edge = FindEdge( source, target );
            if( edge == edges_.size() ) {
               // Didn't find the edge, create it.
               AddEdgeNoCheck( source, target, weight );
            } else {
               // Found the edge, update the weight
               edges_[ edge ].weight += weight;
            }
         }

      /// \brief Add an edge pair between vertices `vertex1` and `vertex2`, both with weight `weight`.
      /// If the edges already exist, update their weights to be `weight`.
      void AddEdgePair( VertexIndex vertex1, VertexIndex vertex2, dfloat weight ) {
         AddEdgePair( vertex1, vertex2, weight, weight );
      }

      /// \brief Add an edge pair between vertices `vertex1` and `vertex2`, with weight `weight1` and `weight2` respectively.
      /// If the edges already exist, update their weights to be `weight`.
      void AddEdgePair( VertexIndex vertex1, VertexIndex vertex2, dfloat weight1, dfloat weight2 ) {
         DIP_THROW_IF( vertex1 == vertex2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge1 = FindEdge( vertex1, vertex2 );
         EdgeIndex edge2 = ( edge1 == edges_.size() ) ? FindEdge( vertex2, vertex1 ) : edges_[ edge1 ].sibling;
         if(( edge1 == edges_.size() ) && ( edge2 == edges_.size() )) {
            // Create the pair
            AddEdgePairNoCheck( vertex1, vertex2, weight1, weight2 );
         } else if( edge1 == edges_.size() ) {
            // edge2 exists, create edge1
            AddEdgeNoCheck( vertex1, vertex2, weight1, edge2 );
            edges_[ edge2 ].weight = weight2;
         } else if( edge2 == edges_.size() ) {
            // edge1 exists, create edge2
            AddEdgeNoCheck( vertex2, vertex1, weight2, edge1 );
            edges_[ edge1 ].weight = weight1;
         } else {
            // Found both edges
            edges_[ edge1 ].weight = weight1;
            edges_[ edge2 ].weight = weight2;
         }
      }

      /// \brief Add an edge pair between vertices `vertex1` and `vertex2`, both with weight `weight`.
      /// If the edges already exist, update their weight by adding the new weight to the existing weight.
      void AddEdgePairSumWeight( VertexIndex vertex1, VertexIndex vertex2, dfloat weight ) {
         AddEdgePairSumWeight( vertex1, vertex2, weight, weight );
      }

      /// \brief Add an edge pair between vertices `vertex1` and `vertex2`, with weight `weight1` and `weight2` respectively.
      /// If the edges already exist, update their weight by adding the new weight to the existing weight.
      void AddEdgePairSumWeight( VertexIndex vertex1, VertexIndex vertex2, dfloat weight1, dfloat weight2 ) {
         DIP_THROW_IF( vertex1 == vertex2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge1 = FindEdge( vertex1, vertex2 );
         EdgeIndex edge2 = ( edge1 == edges_.size() ) ? FindEdge( vertex2, vertex1 ) : edges_[ edge1 ].sibling;
         if(( edge1 == edges_.size() ) && ( edge2 == edges_.size() )) {
            // Create the pair
            AddEdgePairNoCheck( vertex1, vertex2, weight1, weight2 );
         } else if( edge1 == edges_.size() ) {
            // edge2 exists, create edge1
            AddEdgeNoCheck( vertex1, vertex2, weight1, edge2 );
            edges_[ edge2 ].weight += weight2;
         } else if( edge2 == edges_.size() ) {
            // edge1 exists, create edge2
            AddEdgeNoCheck( vertex2, vertex1, weight2, edge1 );
            edges_[ edge1 ].weight += weight1;
         } else {
            // Found both edges
            edges_[ edge1 ].weight += weight1;
            edges_[ edge2 ].weight += weight2;
         }
      }

      /// \brief Delete the edge from vertex `source` to `target`.
      void DeleteEdge( VertexIndex source, VertexIndex target ) {
         EdgeIndex edge = FindEdge( source, target );
         if( edge < edges_.size() ) {
            DeleteEdge( edge );
         }
      }

      /// \brief Delete both edges between vertex `vertex1` and `vertex2`.
      void DeleteEdgePair( VertexIndex vertex1, VertexIndex vertex2 ) {
         EdgeIndex edge = FindEdge( vertex1, vertex2 );
         if( edge < edges_.size() ) {
            DeleteEdgePair( edge );
         } else {
            edge = FindEdge( vertex2, vertex1 );
            if( edge < edges_.size() ) {
               DeleteEdge( edge );
            }
         }
      }

      /// \brief Delete the edge `edge`.
      void DeleteEdge( EdgeIndex edge ) {
         DIP_ASSERT( edge < edges_.size() );
         auto& vertex1 = vertices_[ edges_[ edge ].source ];
         auto it = FindEdge( vertex1, edge );
         if( it != vertex1.edges.end() ) {
            vertex1.edges.erase( it );
         }
         edges_[ edge ].target = edges_[ edge ].source;
         auto sibling = edges_[ edge ].sibling;
         if( sibling == edge ) {
            return;
         }
         edges_[ sibling ].sibling = sibling;
      }

      /// \brief Delete the edge `edge` and its sibling.
      void DeleteEdgePair( EdgeIndex edge ) {
         DIP_ASSERT( edge < edges_.size() );
         auto& vertex1 = vertices_[ edges_[ edge ].source ];
         auto it = FindEdge( vertex1, edge );
         if( it != vertex1.edges.end() ) {
            vertex1.edges.erase( it );
         }
         edges_[ edge ].target = edges_[ edge ].source;
         auto sibling = edges_[ edge ].sibling;
         if( sibling == edge ) {
            return;
         }
         auto& vertex2 = vertices_[ edges_[ sibling ].source ];
         it = FindEdge( vertex2, sibling );
         if( it != vertex2.edges.end() ) {
            vertex2.edges.erase( it );
         }
         edges_[ sibling ].target = edges_[ sibling ].source;
      }

      /// \brief Returns a list of indices to neighboring vertices. The list is created. `EdgeIndices` is
      /// a more efficient, but less convenient, function.
      std::vector< VertexIndex > Neighbors( VertexIndex vertex ) {
         DIP_ASSERT( vertex < vertices_.size() );
         std::vector< VertexIndex > neighbors;
         neighbors.reserve( vertices_[ vertex ].edges.size() );
         for( auto edge : vertices_[ vertex ].edges ) {
            neighbors.push_back( edges_[edge].target );
         }
         return neighbors;
      }

      // Adds an edge. Doesn't check for duplicates. If the edges already exist, disaster ensues.
      // Not publicly documented, so don't use this.
      void AddEdgeNoCheck( VertexIndex source, VertexIndex target, dfloat weight ) {
         EdgeIndex ii = edges_.size();
         EdgeIndex sibling = FindEdge( target, source ); // Will equal ii if there's no such edge.
         edges_.push_back( { source, target, weight, sibling } );
         vertices_[ source ].edges.push_back( ii );
         if( sibling != ii ) {
            edges_[ sibling ].sibling = ii;
         }
      }

      // Adds an edge. Doesn't check for duplicates. If the edges already exist, disaster ensues.
      // This version doesn't search for the sibling.
      // Not publicly documented, so don't use this.
      void AddEdgeNoCheck( VertexIndex source, VertexIndex target, dfloat weight, EdgeIndex sibling ) {
         EdgeIndex ii = edges_.size();
         edges_.push_back( { source, target, weight, sibling } );
         vertices_[ source ].edges.push_back( ii );
         edges_[ sibling ].sibling = ii;
      }

      // Adds an edge pair. Doesn't check for duplicates. If the edges already exist, disaster ensues.
      // Not publicly documented, so don't use this.
      void AddEdgePairNoCheck( VertexIndex vertex1, VertexIndex vertex2, dfloat weight1, dfloat weight2 ) {
         EdgeIndex ii = edges_.size();
         edges_.push_back( { vertex1, vertex2, weight1, ii + 1 } ); // This is edge ii
         edges_.push_back( { vertex2, vertex1, weight2, ii } );     // This is edge ii + 1
         vertices_[ vertex1 ].edges.push_back( ii );
         vertices_[ vertex2 ].edges.push_back( ii + 1 );
      }

      /// \brief Re-computes edge weights using the function `func`, called as `dfloat func(dfloat source, dfloat target)`,
      /// where the two inputs to `func` are the value of the two vertices. The sibling edge, if it exists,
      /// gets the same value, it is computed only once. So `func` has to be symmetric.
      template< typename F >
      void UpdateEdgeWeights( F func ) const {
         for( dip::uint ii = 0; ii < edges_.size(); ++ii ) {
            auto& edge = edges_[ ii ];
            if( edge.sibling >= ii ) { // else it's already filled out
               dfloat weight = func( vertices_[ edge.source ].value, vertices_[ edge.target ].value );
               edge.weight = func( vertices_[ edge.source ].value, vertices_[ edge.target ].value );
               if( edge.sibling != ii ) {
                  edges_[ edge.sibling ].weight = weight;
               }
            }
         }
      }

      /// \brief Re-computes edge weights as the absolute difference between vertex values.
      void UpdateEdgeWeights() const {
         UpdateEdgeWeights( []( dfloat val1, dfloat val2 ) { return std::abs( val1 - val2 ); } );
      }

      /// \brief Sets the vertex weight to 1 if the vertex is connected to vertex `root`, to 0 otherwise.
      DIP_EXPORT void IsConnectedTo( VertexIndex root );

   private:
      std::vector< Vertex > vertices_{};
      std::vector< Edge > edges_{};

      // Return an iterator to the edge index within `vertex`. Returns `vertex.end()` if not found.
      static EdgeList::iterator FindEdge( Vertex& vertex, EdgeIndex edge ) {
         EdgeList::iterator it = vertex.edges.begin();
         for( ; ( it != vertex.edges.end() ) && ( *it != edge ); ++it ) {}
         return it;
      }

      // Return the index to the edge that joins vertex `source` to `target`.
      // If the edge doesn't exist, returns edges_.size().
      EdgeIndex FindEdge( VertexIndex source, VertexIndex target ) {
         DIP_ASSERT( source < vertices_.size() );
         DIP_ASSERT( target < vertices_.size() );
         DIP_ASSERT( source != target );
         for( auto edge : vertices_[ source ].edges ) {
            if( edges_[ edge ].target == target ) {
               return edge;
            }
         }
         return edges_.size();
      }

};

/// \brief Computes the minimum spanning forest (MSF) of a graph using Prim's algorithm.
///
/// If `roots` is an empty set, the vertex with index 0 is used as the root, and the resulting graph
/// will be a minimum spanning tree (MST). If multiple roots are given, each one will spawn a tree.
///
/// The output graph only contains edges reachable from the given roots. Any components not connected
/// to the roots will not remain in the graph (the vertices will be copied over, but not connected).
DIP_NODISCARD DIP_EXPORT Graph MinimumSpanningForest( Graph const& graph, std::vector< Graph::VertexIndex > const& roots = {} );

DIP_NODISCARD inline Graph Graph::MinimumSpanningForest( std::vector< VertexIndex > const& roots ) const {
   return dip::MinimumSpanningForest( *this, roots );
}

/// \brief Computes the minimum cut of the graph, separating the source node from the sink node.
///
/// `graph` is a directed graph where each edge has a reverse sibling. That is, each pair of connected
/// vertices must be connected by two edges in opposite direction. The edge weights represent the maximum
/// flow that can run through it in its natural direction, and will determine where the cut happens.
///
/// `sourceIndex` and `sinkIndex` are indices pointing at the source and sink nodes (vertices).
/// These are the two marker nodes that will be separated by the cut. If there is more than one marker node
/// for either source or sink, create a new node that is strongly connected (very large edge weight) to all
/// these marker nodes, and use this new node as the source or sink.
///
/// `graph` is expected to have a path from `sourceIndex` to `sinkIndex`. If no such path exists,
/// there's nothing to cut.
///
/// The function will find the maximum flow through the graph from source to sink. This will naturally saturate
/// a set of edges. This set of edges is the minimum cut that will separate the source from the sink.
/// The algorithm used to compute the maximum flow is the one proposed by Boykov and Kolmogorov. This algorithm
/// was shown to be faster than older algorithms when applied to a 2D image converted to a graph using a 4-connected
/// neighborhood.
///
/// The input graph will be modified as follows: The edge weights will reflect the residual of the maximum flow
/// from the source node to the sink node (i.e. how much more capacity each edge has). The edges with a zero
/// residual (and their sibling) will be removed; these edges compose the minimum cut, and removing them
/// separates the source from the sink in separate connected components. \ref Label(DirectedGraph const&)
/// can identify the two components created by this cut.
///
/// !!! literature
///     - Y. Boykov and V. Kolmogorov, "An Experimental Comparison of Min-Cut/Max-Flow Algorithms for Energy Minimization in Vision",
///       IEEE Transactions on Pattern Analysis and Machine Intelligence 26(9):1124-1137, 2004.
DIP_EXPORT void GraphCut( DirectedGraph& graph, DirectedGraph::VertexIndex sourceIndex, DirectedGraph::VertexIndex sinkIndex );

/// \brief Connected component analysis of a graph.
///
/// The output can be used to relabel the image that the graph was constructed from. It maps the graph's vertex
/// indices to labels, where each connected component in the graph represents a label.
///
/// See also \ref Relabel(Image const&, Image&, Graph const&).
DIP_NODISCARD DIP_EXPORT LabelMap Label( Graph const& graph );

/// \brief Connected component analysis of a graph.
///
/// The output can be used to relabel the image that the graph was constructed from. It maps the graph's vertex
/// indices to labels, where each connected component in the graph represents a label.
///
/// See also \ref Relabel(Image const&, Image&, DirectedGraph const&).
DIP_NODISCARD DIP_EXPORT LabelMap Label( DirectedGraph const& graph );

/// \endgroup

} // namespace dip

#endif //DIP_GRAPH_H
