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

#ifndef DIP_GRAPH_H
#define DIP_GRAPH_H

#include <array>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#include "diplib.h"


/// \file
/// \brief Representing and working with an image as a graph
/// See \ref infrastructure.


namespace dip {


/// \addtogroup infrastructure


/// \brief A non-directed, edge-weighted graph.
///
/// Vertices are identified by an index, these indices are expected to be consecutive. Each vertex contains a list
/// of indices to edges, and has an optional value.
/// Edges are represented by indices to two vertices, and a double precision floating-point weight.
/// If the two vertex indices for an edge are the same, the edge is not valid; this situation arises when an edge is
/// deleted.
///
/// If converting an image to a graph, each pixel is converted to a vertex. The pixel's linear index (see \ref pointers)
/// is the vertex index.
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
         EdgeList edges;               ///< The list of indices to edges
         mutable dfloat value = 0.0;   ///< The value associated to each vertex

         Vertex() = default;

         /// \brief Construct a vertex with reserved space for the given number of edges.
         explicit Vertex( dip::uint nEdges ) {
            edges.reserve( nEdges );
         }
      };

      /// \brief An edge in the graph
      ///
      /// If both vertices are 0, the edge is not valid (never used or deleted). Otherwise, `vertices[1] > vertices[0]`.
      struct Edge {
         std::array< VertexIndex, 2 > vertices = {{ 0, 0 }};   ///< The two vertices joined by this edge
         mutable dfloat weight = 0.0;                          ///< The weight of this edge
         bool IsValid() const {
            return vertices[ 0 ] != vertices[ 1 ];
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
      /// `connectivity` indicates which pixels are considered neighbors. Currently, only a connectivity of 1 is
      /// allowed. This makes neighbors the pixels at a city-block distance of 1 (in 2D, there are 4 such neighbors,
      /// in 3D there are 6).
      ///
      /// By default, the edge weights are given by the absolute difference between the two pixel values.
      /// If `weights` is `"average"`, the edge weights are given by the average of the two pixel values.
      ///
      /// Vertex values are set to the corresponding pixel value.
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

      /// \brief Get the indices to the edges that join vertex `v`.
      DIP_NODISCARD EdgeList const& EdgeIndices( VertexIndex v ) const {
         DIP_ASSERT( v < vertices_.size() );
         return vertices_[ v ].edges;
      }

      /// \brief Returns a reference to the value of the vertex `v`. This value is mutable even if the graph is `const`.
      DIP_NODISCARD dfloat& VertexValue( VertexIndex v ) const {
         DIP_ASSERT( v < vertices_.size() );
         return vertices_[ v ].value;
      }

      /// \brief Add an edge between vertices `v1` and `v2`, with weight `weight`. If the edge already exists,
      /// update the weight of the edge to be `weight`.
      void AddEdge( VertexIndex v1, VertexIndex v2, dfloat weight ) {
         DIP_THROW_IF( v1 == v2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge = FindEdge( v1, v2 );
         if( edge == edges_.size() ) {
            // Didn't find the edge, create it.
            AddEdgeNoCheck( v1, v2, weight );
         } else {
            // Found the edge, update the weight
            edges_[ edge ].weight = weight;
         }
      }

      /// \brief Add an edge between vertices `v1` and `v2`, with weight `weight`. If the edge already exists,
      /// update the weight of the edge by adding `weight` to the existing weight.
      void AddEdgeSumWeight( VertexIndex v1, VertexIndex v2, dfloat weight ) {
         DIP_THROW_IF( v1 == v2, "Cannot create an edge between a vertex and itself" );
         EdgeIndex edge = FindEdge( v1, v2 );
         if( edge == edges_.size() ) {
            // Didn't find the edge, create it.
            AddEdgeNoCheck( v1, v2, weight );
         } else {
            // Found the edge, update the weight
            edges_[ edge ].weight += weight;
         }
      }

      /// \brief Delete the edge between vertices `v1` and `v2`.
      void DeleteEdge( VertexIndex v1, VertexIndex v2 ) {
         EdgeIndex edge = FindEdge( v1, v2 );
         if( edge < edges_.size() ) {
            DeleteEdge( edge );
         }
      }

      /// \brief Delete the edge `edge`.
      void DeleteEdge( EdgeIndex edge ) {
         DIP_ASSERT( edge < edges_.size() );
         auto& v1 = vertices_[ edges_[ edge ].vertices[ 0 ]];
         auto it = FindEdge( v1, edge );
         if( it != v1.edges.end() ) {
            v1.edges.erase( it );
         }
         auto& v2 = vertices_[ edges_[ edge ].vertices[ 1 ]];
         it = FindEdge( v2, edge );
         if( it != v2.edges.end() ) {
            v2.edges.erase( it );
         }
         edges_[ edge ].vertices = { 0, 0 };
      }

      /// \brief Returns a list of indices to neighboring vertices. The list is created. `EdgeIndices` is
      /// a more efficient, but less convenient, function.
      std::vector< VertexIndex > Neighbors( VertexIndex v ) {
         DIP_ASSERT( v < vertices_.size() );
         std::vector< VertexIndex > neighbors;
         neighbors.reserve( vertices_[ v ].edges.size() );
         for( auto edge : vertices_[ v ].edges ) {
            neighbors.push_back( OtherVertex( edge, v ));
         }
         return neighbors;
      }

      // Adds an edge. Doesn't check for duplicates. If the edge already exists, disaster ensues.
      // And if v1 >= v2, disaster ensues.
      void AddEdgeNoCheck( Edge edge ) {
         EdgeIndex ii = edges_.size();
         vertices_[ edge.vertices[ 0 ]].edges.push_back( ii );
         vertices_[ edge.vertices[ 1 ]].edges.push_back( ii );
         edges_.push_back( edge );
      }
      void AddEdgeNoCheck( VertexIndex v1, VertexIndex v2, dfloat weight ) {
         AddEdgeNoCheck( {{ v1, v2 }, weight } );
      }

      /// \brief Re-computes edge weights as the absolute difference between vertex values.
      void UpdateEdgeWeights() const {
         for( auto& edge: edges_ ) {
            edge.weight = std::abs( vertices_[ edge.vertices[ 0 ]].value - vertices_[ edge.vertices[ 1 ]].value );
         }
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

      // Return the index to the edge that joins vertices `v1` and `v2`.
      // If the edge doesn't exist, returns edges_.size().
      // `v1` and `v2` might be swapped by this function too.
      EdgeIndex FindEdge( VertexIndex& v1, VertexIndex& v2 ) {
         DIP_ASSERT( v1 < vertices_.size() );
         DIP_ASSERT( v2 < vertices_.size() );
         if( v1 > v2 ) {
            std::swap( v1, v2 );
         }
         for( auto edge : vertices_[ v1 ].edges ) {
            if( edges_[ edge ].vertices[ 1 ] == v2 ) {
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

/// \endgroup

} // namespace dip

#endif //DIP_GRAPH_H
