/*
 * (c)2024, Cris Luengo.
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
#include <deque>
#include <limits>
#include <queue>
#include <vector>

#include "diplib.h"


namespace dip {

namespace {

using VertexIndex = Graph::VertexIndex;
using EdgeIndex = Graph::EdgeIndex;
using EdgeList = Graph::EdgeList;

constexpr VertexIndex ROOT = std::numeric_limits< VertexIndex >::max(); // The node is the root of the source or sink tree
constexpr VertexIndex NO_PARENT = ROOT - 1; // The node doesn't have a parent
constexpr VertexIndex ORPHAN = ROOT - 2;    // The node is an orphan
constexpr VertexIndex MAX_VERTEX_INDEX = ROOT - 3; // This is the largest vertex index we can use
constexpr uint8 SOURCE = 1; // This node belongs to the source tree (S in the paper)
constexpr uint8 SINK = 2;   // This node belongs to the sink tree (T in the paper)

struct FlowGraph  {
   // A version of dip::Graph that encodes residual flow through the graph (bidirectional).
   // Vertices have additional data needed to compute the max-flow solution.
   //
   // The search trees (S starts in the source node, T starts in the sink node) are defined by a parent
   // pointer from each node down the tree. We cannot traverse the tree starting at the root, we can only
   // go to the root starting at any node in the tree. `edges[ ii ].parent` points to the parent node,
   // and `edges[ ii ].parentEdge` points to the edge that links node `ii` to its parent.
   //
   // Which vertices connect to which other vertices using which edges is determined by a linked dip::Graph,
   // to avoid copying all those arrays.

   // A vertex in the graph.
   struct Vertex {
      VertexIndex parent = NO_PARENT; // Each vertex points to its parent in the tree
      EdgeIndex parentEdge = 0;       // This is the edge that leads to `parent`
      uint8 root = 0;                 // This is the root of the tree (either `SOURCE` or `SINK`, 0 indicates no tree
      bool isActive = false;          // When an active vertex is made non-active, it is not removed from the queue; thus,
                                      //    when popping from the queue, check this value to see if the vertex still is active
   };

   // An edge in the graph. `residual` is how much can still flow from vertex[0] to vertex[1].
   // `reverseResidual` is how much can still flow from vertex[1] to vertex[0].
   // When decreasing one, increase the other.
   //
   // TODO: An alternative implementation is to record the flow, flow direction, and capacity.
   //       Flow direction could be the sign if flow. The residual in the direction of flow would
   //       be `capacity - abs(flow)`, in the other direction it would be
   struct Edge {
      dfloat residual = 0;
      dfloat reverseResidual = 0;
   };

   explicit FlowGraph( Graph const& graph ) : graph( graph ), vertices( graph.NumberOfVertices() ), edges( graph.NumberOfEdges() ) {
      for( EdgeIndex ii = 0; ii < graph.NumberOfEdges(); ii++ ) {
         edges[ ii ].residual = edges[ ii ].reverseResidual = graph.EdgeWeight( ii );
      }
   }

   bool ReduceResidual( EdgeIndex edge, VertexIndex source, dfloat flow ) {
      // Pushes a flow through edge from source, reducing the residual in that direction (and increasing it in the other)
      // Returns true if the edge became saturated
      if( graph.EdgeVertex( edge, 0 ) == source ) {
         edges[ edge ].residual -= flow;
         edges[ edge ].reverseResidual += flow;
         DIP_ASSERT( edges[ edge ].residual >= 0 );
         return edges[ edge ].residual == 0;
      } else {
         DIP_ASSERT( graph.EdgeVertex( edge, 1 ) == source );
         edges[ edge ].reverseResidual -= flow;
         edges[ edge ].residual += flow;
         DIP_ASSERT( edges[ edge ].reverseResidual >= 0 );
         return edges[ edge ].reverseResidual == 0;
      }
   }

   dfloat Residual( EdgeIndex edge, VertexIndex source ) const {
      // Returns the residual (remaining flow capacity) from source.
      if( graph.EdgeVertex( edge, 0 ) == source ) {
         return edges[ edge ].residual;
      } else {
         DIP_ASSERT( graph.EdgeVertex( edge, 1 ) == source );
         return edges[ edge ].reverseResidual;
      }
   }

   dfloat ReverseResidual( EdgeIndex edge, VertexIndex sink ) const {
      // Returns the residual (remaining flow capacity) to sink.
      if( graph.EdgeVertex( edge, 1 ) == sink ) {
         return edges[ edge ].residual;
      } else {
         DIP_ASSERT( graph.EdgeVertex( edge, 0 ) == sink );
         return edges[ edge ].reverseResidual;
      }
   }

   bool IsOrphan( VertexIndex node ) const {
      // An orphan node is any node whose ancestor in the tree is orphaned
      while( vertices[ node ].parent <= MAX_VERTEX_INDEX ) {
         node = vertices[ node ].parent;
      }
      return vertices[ node ].parent == ORPHAN;
   }

   bool IsSaturated( EdgeIndex edge ) const {
      return ( edges[ edge ].residual == 0 ) || ( edges[ edge ].reverseResidual == 0 );
   }

   Graph const& graph;
   std::vector< Vertex > vertices;
   std::vector< Edge > edges;
};

EdgeIndex grow(
   FlowGraph& flowGraph,
   std::queue< VertexIndex >& activeNodes
) {
   while( !activeNodes.empty() ) {
      VertexIndex active = activeNodes.front();
      if( !flowGraph.vertices[ active ].isActive ) {
         //std::cout << "Node on active list was not active: " << active << '\n';
         activeNodes.pop();
         continue;
      }
      // Iterate over neighbors of `active` that have capacity left
      //std::cout << "Processing active node: " << active << '\n';
      for( EdgeIndex edge : flowGraph.graph.EdgeIndices( active )) {
      //std::cout << "   Processing edge: " << edge << '\n';
         if( !flowGraph.graph.IsValidEdge( edge )) {
            //std::cout << "      (invalid)";
            continue;
         }
         dfloat residual = ( flowGraph.vertices[ active ].root == SOURCE )
                           ? flowGraph.Residual( edge, active )
                           : flowGraph.ReverseResidual( edge, active );
         if( residual <= 0 ) {
            //std::cout << "      (no residual)";
            continue;
         }
         VertexIndex neighbor = flowGraph.graph.OtherVertex( edge, active );
         if( flowGraph.vertices[ neighbor ].root == 0 ) {
            // Neighbor is unaffiliated: add it to this tree, and make it active
            //std::cout << "      (free node " << neighbor << ", added to tree)";
            flowGraph.vertices[ neighbor ].root = flowGraph.vertices[ active ].root;
            flowGraph.vertices[ neighbor ].parent = active;
            flowGraph.vertices[ neighbor ].parentEdge = edge;
            flowGraph.vertices[ neighbor ].isActive = true;
            activeNodes.push( neighbor );
         } else {
            if( flowGraph.vertices[ neighbor ].root != flowGraph.vertices[ active ].root ) {
               // The neighbor belongs to the other tree, we've found a path!
               //std::cout << "      (neighbor node is in opposite tree: " << neighbor << '\n';
               return edge;
            }
         }
      }
      //std::cout << "   Deactivating node\n";
      flowGraph.vertices[ active ].isActive = false;
      activeNodes.pop();
   }
   // We can't find any more paths
   //std::cout << "No more active nodes\n";
   return flowGraph.graph.NumberOfEdges();
}

Graph finalize( FlowGraph const& flowGraph ) {
   Graph output( flowGraph.graph );  // Makes a copy of the original graph
   for( dip::uint edge = 0; edge < flowGraph.edges.size(); ++edge ) {
      if( flowGraph.IsSaturated( edge )) {
         //std::cout << "Deleting edge number " << edge << '\n';
         output.DeleteEdge( edge ); // Delete edges with a residual flow of 0 (they're saturated)
      }
      // TODO: do we want to save the flow through each edge?
   }
   return output;
}

void augment(
   FlowGraph& flowGraph,
   EdgeIndex pathEdge,
   std::deque< VertexIndex >& orphanNodes
) {
   // The two vertices of this edge should each be in different trees
   VertexIndex v1 = flowGraph.graph.EdgeVertex( pathEdge, 0 );
   VertexIndex v2 = flowGraph.graph.EdgeVertex( pathEdge, 1 );
   //std::cout << "Augmenting edge " << pathEdge << ", connecting nodes " << v1 << " and " << v2 << '\n';
   uint8 v1_root = flowGraph.vertices[ v1 ].root;
   uint8 v2_root = flowGraph.vertices[ v2 ].root;
   DIP_ASSERT(( v1_root > 0 ) && ( v2_root > 0 ) && ( v1_root != v2_root ));
   // Find out how much flow we can push though this path
   VertexIndex sourceParent = ( v1_root == SOURCE ) ? v1 : v2;
   VertexIndex sinkParent = ( v1_root == SINK ) ? v1 : v2;
   //std::cout << "    sourceParent = " << sourceParent << ", sinkParent = " << sinkParent << '\n';
   dfloat min_residual = flowGraph.Residual( pathEdge, sourceParent );
   //std::cout << "   Initial min_residual = " << min_residual << '\n';
   VertexIndex p;
   VertexIndex q = sourceParent;
   //std::cout << "Going upstream to the source\n";
   while( true ) {
      // Go upstream to the source
      p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      min_residual = std::min( min_residual, flowGraph.Residual( edge, p ));
      q = p;
   }
   //std::cout << "   Current min_residual = " << min_residual << '\n';
   q = sinkParent;
   //std::cout << "Going downstream to the sink\n";
   while( true ) {
      // Go downstream to the sink
      p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      min_residual = std::min( min_residual, flowGraph.Residual( edge, q ));
      q = p;
   }
   //std::cout << "   Final min_residual = " << min_residual << '\n';
   // Push the flow through the path
   flowGraph.ReduceResidual( pathEdge, sourceParent, min_residual );
   q = sourceParent;
   //std::cout << "Going upstream to the source\n";
   while( true ) {
      // Go upstream to the source
      p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      // Flow goes from p to q, p is nearer the root of the tree
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      bool saturated = flowGraph.ReduceResidual( edge, p, min_residual );
      if( saturated ) {
         //std::cout << "   Edge " << edge << " is saturated, node " << q << " orphaned (parent was " << p << ")\n";
         flowGraph.vertices[ q ].parent = ORPHAN;
         orphanNodes.push_front( q );  // nodes closer to the root should be processed earlier in adopt()
      }
      q = p;
   }
   q = sinkParent;
   //std::cout << "Going downstream to the sink\n";
   while( true ) {
      // Go downstream to the sink
      p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      // Flow goes from q to p, p is nearer the root of the tree
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      bool saturated = flowGraph.ReduceResidual( edge, q, min_residual );
      if( saturated ) {
         //std::cout << "   Edge " << edge << " is saturated, node " << q << " orphaned (parent was " << p << ")\n";
         flowGraph.vertices[ q ].parent = ORPHAN;
         orphanNodes.push_front( q );  // nodes closer to the root should be processed earlier in adopt()
      }
      q = p;
   }
}

void adopt_next(
   FlowGraph& flowGraph,
   std::deque< VertexIndex >& orphanNodes,
   std::queue< VertexIndex >& activeNodes
) {
   DIP_ASSERT( !orphanNodes.empty() );
   VertexIndex orphan = orphanNodes.front();
   orphanNodes.pop_front();
   bool isSource = flowGraph.vertices[ orphan ].root == SOURCE;
   // Try to find new parent for orphan
   for( EdgeIndex edge : flowGraph.graph.EdgeIndices( orphan )) {
      if( !flowGraph.graph.IsValidEdge( edge )) {
         continue;
      }
      // The neighbor must have the same root
      VertexIndex neighbor = flowGraph.graph.OtherVertex( edge, orphan );
      if( flowGraph.vertices[ orphan ].root != flowGraph.vertices[ neighbor ].root ) {
         continue;
      }
      // The edge must have capacity left
      dfloat residual = isSource ? flowGraph.Residual( edge, neighbor )
                                 : flowGraph.ReverseResidual( edge, neighbor );
      if( residual <= 0 ) {
         continue;
      }
      // The neighbor must not be in an orphaned sub-tree
      if( flowGraph.IsOrphan( neighbor )) {
         continue;
      }
      // We found a new parent!
      //std::cout << "   Orphan " << orphan << " re-attached\n";
      flowGraph.vertices[ orphan ].parent = neighbor;
      flowGraph.vertices[ orphan ].parentEdge = edge;
      return;
   }
   // We didn't find a parent, orphan becomes a free node
   //std::cout << "   Orphan " << orphan << " becomes a free node\n";
   // Step 1: update neighbors
   for( EdgeIndex edge : flowGraph.graph.EdgeIndices( orphan )) {
      if( !flowGraph.graph.IsValidEdge( edge )) {
         continue;
      }
      // The neighbor must have the same root
      VertexIndex neighbor = flowGraph.graph.OtherVertex( edge, orphan );
      if( flowGraph.vertices[ orphan ].root != flowGraph.vertices[ neighbor ].root ) {
         continue;
      }
      // If the edge has capacity left to flow from neighbor into orphan, the neighbor becomes active
      if( !flowGraph.vertices[ neighbor ].isActive ) {
         dfloat residual = isSource ? flowGraph.Residual( edge, neighbor )
                                    : flowGraph.ReverseResidual( edge, neighbor );
         if( residual > 0 ) {
            flowGraph.vertices[ neighbor ].isActive = true;
            activeNodes.push( neighbor );
         }
      }
      // If the neighbor is a child, make it an orphan
      if( flowGraph.vertices[ neighbor ].parent == orphan ) {
         flowGraph.vertices[ neighbor ].parent = ORPHAN;
         orphanNodes.push_back( neighbor );  // We process this one after everything else
      }
   }
   // Step 2: reset the node
   flowGraph.vertices[ orphan ].parent = NO_PARENT;
   flowGraph.vertices[ orphan ].parentEdge = 0;
   flowGraph.vertices[ orphan ].root = 0;
   flowGraph.vertices[ orphan ].isActive = false;
}

} // namespace

Graph GraphCut( Graph const& graph, Graph::VertexIndex sourceIndex, Graph::VertexIndex sinkIndex ) {
   DIP_THROW_IF( graph.NumberOfVertices() > MAX_VERTEX_INDEX, "Graph has too many vertices" ); // This is soooooo unlikely!
   DIP_THROW_IF( sourceIndex >= graph.NumberOfVertices(), E::INDEX_OUT_OF_RANGE );
   DIP_THROW_IF( sinkIndex >= graph.NumberOfVertices(), E::INDEX_OUT_OF_RANGE );
   FlowGraph flowGraph( graph );
   flowGraph.vertices[ sourceIndex ].parent = ROOT;
   flowGraph.vertices[ sourceIndex ].root = SOURCE;
   flowGraph.vertices[ sourceIndex ].isActive = true;
   flowGraph.vertices[ sinkIndex ].parent = ROOT;
   flowGraph.vertices[ sinkIndex ].root = SINK;
   flowGraph.vertices[ sinkIndex ].isActive = true;
   std::queue< VertexIndex > activeNodes; // TODO: a dedicated circular queue might be better
   activeNodes.push( sourceIndex );
   activeNodes.push( sinkIndex );
   std::deque< VertexIndex > orphanNodes;
   // S = tree starting at source_node
   // T = tree starting at sink_node
   while( true ) {
      //std::cout << "== Main loop iteration ==\n";
      // Grow both trees until they meet at one edge, `pathEdge`, which describes a path from source to sink
      //std::cout << "   - grow:\n";
      EdgeIndex pathEdge = grow( flowGraph, activeNodes );
      if( pathEdge == graph.NumberOfEdges() ) {
         // There's no more paths to be found, we're done
         //std::cout << "   - finalize:\n";
         return finalize( flowGraph );
      }
      // Have flow go through this path; this might detach some nodes from the trees, which become orphans
      //std::cout << "   - augment:\n";
      augment( flowGraph, pathEdge, orphanNodes );
      // Re-attach the orphans if possible
      //std::cout << "   - adopt:\n";
      while( !orphanNodes.empty() ) {
         adopt_next( flowGraph, orphanNodes, activeNodes );
      }
   }
}

} // namespace dip
