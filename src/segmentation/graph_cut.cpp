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
#include <cstdlib>
#include <limits>
#include <utility>
#include <vector>

#include "diplib.h"


namespace dip {

namespace {

using VertexIndex = DirectedGraph::VertexIndex;
using EdgeIndex = DirectedGraph::EdgeIndex;
using EdgeList = DirectedGraph::EdgeList;

constexpr VertexIndex ROOT = std::numeric_limits< VertexIndex >::max(); // The node is the root of the source or sink tree
constexpr VertexIndex NO_PARENT = ROOT - 1; // The node doesn't have a parent
constexpr VertexIndex ORPHAN = ROOT - 2;    // The node is an orphan
constexpr VertexIndex MAX_VERTEX_INDEX = ROOT - 3; // This is the largest vertex index we can use
constexpr uint8 SOURCE = 1; // This node belongs to the source tree (S in the paper)
constexpr uint8 SINK = 2;   // This node belongs to the sink tree (T in the paper)

struct FlowGraph  {
   // Augments a dip::DirectedGraph with some additional information needed to compute the max-flow.
   //
   // The search trees (S starts in the source node, T starts in the sink node) are defined by a parent
   // pointer from each node down the tree. We cannot traverse the tree starting at the root, we can only
   // go to the root starting at any node in the tree. `edges[ ii ].parent` points to the parent node,
   // and `edges[ ii ].parentEdge` points to the edge that links node `ii` to its parent.

   // A vertex in the graph.
   struct Vertex {
      VertexIndex parent = NO_PARENT; // Each vertex points to its parent in the tree
      EdgeIndex parentEdge = 0;       // This is the edge that leads from `parent` to here
      uint8 root = 0;                 // This is the root of the tree (either `SOURCE` or `SINK`, 0 indicates no tree)
      bool isActive = false;          // When an active vertex is made non-active, it is not removed from the queue; thus,
                                      //    when popping from the queue, check this value to see if the vertex still is active
   };

   explicit FlowGraph( DirectedGraph& graph ) : graph( graph ), vertices( graph.NumberOfVertices() ) {
      for( EdgeIndex ii = 0; ii < graph.NumberOfEdges(); ii++ ) {
         if( graph.IsValidEdge( ii )) {
            if( graph.SiblingEdge( ii ) == ii ) {
               DIP_THROW( "Not all edges in the directed graph have a sibling." );
            }
         }
      }
   }

   bool ReduceResidual( EdgeIndex edge, dfloat flow ) {
      // Pushes a flow through edge, reducing its residual and increasing that of its sibling.
      // Returns true if the edge became saturated
      graph.EdgeWeight( edge ) -= flow;
      graph.EdgeWeight( graph.SiblingEdge( edge )) += flow;
      DIP_ASSERT( graph.EdgeWeight( edge ) >= 0 );
      return graph.EdgeWeight( edge ) == 0;
   }

   bool ReduceReverseResidual( EdgeIndex edge, dfloat flow ) {
      // Pushes a flow through edge in reverse, increasing its residual and decreasing that of its sibling.
      // Returns true if the sibling edge became saturated
      return ReduceResidual( graph.SiblingEdge( edge ), flow );
   }

   dfloat Residual( EdgeIndex edge ) const {
      // Returns the residual (remaining flow capacity)
      return graph.EdgeWeight( edge );
   }

   dfloat ReverseResidual( EdgeIndex edge ) const {
      // Returns the residual (remaining flow capacity) of the sibling edge.
      return graph.EdgeWeight( graph.SiblingEdge( edge ));
   }

   bool IsOrphan( VertexIndex node ) const {
      // An orphan node is any node whose ancestor in the tree is orphaned
      while( vertices[ node ].parent <= MAX_VERTEX_INDEX ) {
         node = vertices[ node ].parent;
      }
      return vertices[ node ].parent == ORPHAN;
   }

   bool IsSaturated( EdgeIndex edge ) const {
      return ( Residual( edge ) == 0 ) || ( ReverseResidual( edge ) == 0 );
   }

   DirectedGraph& graph;
   std::vector< Vertex > vertices;
};

template< typename T >
class DeQueue {
   public:
      using ValueType = T;

      DeQueue( dip::uint maxLength ) : array( maxLength ) {}

      void PushFront( ValueType value ) {
         DIP_ASSERT( !Full() );
         if( Empty() ) {
            PushFirst( value );
         } else {
            if( first == 0 ) {
               first = array.size() - 1;
            } else {
               first -= 1;
            }
            array[ first ] = value;
            count += 1;
         }
      }

      void PushBack( ValueType value ) {
         DIP_ASSERT( !Full() );
         if( Empty() ) {
            PushFirst( value );
         } else {
            last += 1;
            if( last == array.size() ) {
               last = 0;
            }
            array[ last ] = value;
            count += 1;
         }
      }

      ValueType Front() {
         DIP_ASSERT( !Empty() );
         return array[ first ];
      }

      ValueType PopFront() {
         DIP_ASSERT( !Empty() );
         ValueType value = array[ first ];
         first = first + 1;
         if( first == array.size() ) {
            first = 0;
         }
         count -= 1;
         return value;
      }

      dip::uint Size() const {
         return count;
      }

      bool Empty() const {
         return count == 0;
      }

      bool Full() const {
         return count == array.size();
      }

   private:
      std::vector< ValueType > array;
      dip::uint first = 0; // the first node that's there
      dip::uint last = 0;  // the last node that's there
      dip::uint count = 0;

      void PushFirst( ValueType value ) {
         first = 0;
         last = 0;
         count = 1;
         array[ 0 ] = value;
      }
};

EdgeIndex grow(
   FlowGraph& flowGraph,
   DeQueue< VertexIndex >& activeNodes
) {
   while( !activeNodes.Empty() ) {
      VertexIndex active = activeNodes.Front();
      if( !flowGraph.vertices[ active ].isActive ) {
         //std::cout << "Node on active list was not active: " << active << '\n';
         activeNodes.PopFront();
         continue;
      }
      // Iterate over neighbors of `active` that have capacity left
      //std::cout << "Processing active node: " << active << '\n';
      for( EdgeIndex edge : flowGraph.graph.EdgeIndices( active )) {
         if( !flowGraph.graph.IsValidEdge( edge )) {
            //std::cout << "      (invalid)";
            continue;
         }
         dfloat residual = ( flowGraph.vertices[ active ].root == SOURCE )
                           ? flowGraph.Residual( edge )
                           : flowGraph.ReverseResidual( edge );
         //std::cout << "   Processing edge: " << edge << ", residual = " << residual << '\n';
         if( residual <= 0 ) {
            //std::cout << "      (no residual)";
            continue;
         }
         VertexIndex neighbor = flowGraph.graph.TargetVertex( edge );
         if( flowGraph.vertices[ neighbor ].root == 0 ) {
            // Neighbor is unaffiliated: add it to this tree, and make it active
            //std::cout << "      (free node " << neighbor << ", added to tree)\n";
            flowGraph.vertices[ neighbor ].root = flowGraph.vertices[ active ].root;
            flowGraph.vertices[ neighbor ].parent = active;
            flowGraph.vertices[ neighbor ].parentEdge = edge;
            flowGraph.vertices[ neighbor ].isActive = true;
            activeNodes.PushBack( neighbor );
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
      activeNodes.PopFront();
   }
   // We can't find any more paths
   //std::cout << "No more active nodes\n";
   return flowGraph.graph.NumberOfEdges();
}

void finalize( FlowGraph& flowGraph ) {
   for( dip::uint edge = 0; edge < flowGraph.graph.NumberOfEdges(); ++edge ) {
      if( flowGraph.graph.IsValidEdge( edge )) {
         if( flowGraph.IsSaturated( edge )) {
            //std::cout << "Deleting edge number " << edge << '\n';
            flowGraph.graph.DeleteEdgePair( edge ); // Delete edges with a residual flow of 0 (they're saturated)
         }
      }
   }
}

void augment(
   FlowGraph& flowGraph,
   EdgeIndex pathEdge,
   DeQueue< VertexIndex >& orphanNodes
) {
   // The two vertices of this edge should each be in different trees
   VertexIndex sourceParent = flowGraph.graph.SourceVertex( pathEdge );
   VertexIndex sinkParent = flowGraph.graph.TargetVertex( pathEdge );
   //std::cout << "Augmenting edge " << pathEdge << ", connecting nodes " << sourceParent << " and " << sinkParent << '\n';
   DIP_ASSERT(( flowGraph.vertices[ sourceParent ].root > 0 ) &&
              ( flowGraph.vertices[ sinkParent ].root > 0 ) &&
              ( flowGraph.vertices[ sourceParent ].root != flowGraph.vertices[ sinkParent ].root ));
   // Find out how much flow we can push though this path
   if( flowGraph.vertices[ sourceParent ].root == SINK ) {
      pathEdge = flowGraph.graph.SiblingEdge( pathEdge );
      std::swap( sourceParent, sinkParent );
      DIP_ASSERT( sourceParent == flowGraph.graph.SourceVertex( pathEdge ));
      DIP_ASSERT( sinkParent == flowGraph.graph.TargetVertex( pathEdge ));
   }
   //std::cout << "    sourceParent = " << sourceParent << ", sinkParent = " << sinkParent << '\n';
   dfloat min_residual = flowGraph.Residual( pathEdge );
   //std::cout << "   Initial min_residual = " << min_residual << '\n';
   VertexIndex q = sourceParent;
   //std::cout << "Going upstream to the source\n";
   while( true ) {
      // Go upstream to the source
      VertexIndex p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      min_residual = std::min( min_residual, flowGraph.Residual( edge ));
      q = p;
   }
   //std::cout << "   Current min_residual = " << min_residual << '\n';
   q = sinkParent;
   //std::cout << "Going downstream to the sink\n";
   while( true ) {
      // Go downstream to the sink
      VertexIndex p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      min_residual = std::min( min_residual, flowGraph.ReverseResidual( edge ));
      q = p;
   }
   //std::cout << "   Final min_residual = " << min_residual << '\n';
   DIP_ASSERT( min_residual > 0 );
   // Push the flow through the path
   flowGraph.ReduceResidual( pathEdge, min_residual );
   q = sourceParent;
   //std::cout << "Going upstream to the source\n";
   while( true ) {
      // Go upstream to the source
      VertexIndex p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      // Flow goes from p to q, p is nearer the root of the tree
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      bool saturated = flowGraph.ReduceResidual( edge, min_residual );
      if( saturated ) {
         //std::cout << "   Edge " << edge << " is saturated, node " << q << " orphaned (parent was " << p << ")\n";
         flowGraph.vertices[ q ].parent = ORPHAN;
         orphanNodes.PushFront( q );  // nodes closer to the root should be processed earlier in adopt()
      }
      q = p;
   }
   q = sinkParent;
   //std::cout << "Going downstream to the sink\n";
   while( true ) {
      // Go downstream to the sink
      VertexIndex p = flowGraph.vertices[ q ].parent;
      if( p > MAX_VERTEX_INDEX ) {
         break;
      }
      // Flow goes from q to p, p is nearer the root of the tree
      EdgeIndex edge = flowGraph.vertices[ q ].parentEdge;
      bool saturated = flowGraph.ReduceReverseResidual( edge, min_residual );
      if( saturated ) {
         //std::cout << "   Edge " << edge << " is saturated, node " << q << " orphaned (parent was " << p << ")\n";
         flowGraph.vertices[ q ].parent = ORPHAN;
         orphanNodes.PushFront( q );  // nodes closer to the root should be processed earlier in adopt()
      }
      q = p;
   }
}

void adopt_next(
   FlowGraph& flowGraph,
   DeQueue< VertexIndex >& orphanNodes,
   DeQueue< VertexIndex >& activeNodes
) {
   DIP_ASSERT( !orphanNodes.Empty() );
   VertexIndex orphan = orphanNodes.PopFront();
   bool isSource = flowGraph.vertices[ orphan ].root == SOURCE;
   // Try to find new parent for orphan
   for( EdgeIndex edge : flowGraph.graph.EdgeIndices( orphan )) {
      if( !flowGraph.graph.IsValidEdge( edge )) {
         continue;
      }
      // The neighbor must have the same root
      VertexIndex neighbor = flowGraph.graph.TargetVertex( edge );
      if( flowGraph.vertices[ orphan ].root != flowGraph.vertices[ neighbor ].root ) {
         continue;
      }
      // The edge must have capacity left
      EdgeIndex incoming_edge = flowGraph.graph.SiblingEdge( edge );
      dfloat residual = isSource ? flowGraph.Residual( incoming_edge )
                                 : flowGraph.ReverseResidual( incoming_edge );
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
      flowGraph.vertices[ orphan ].parentEdge = incoming_edge;
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
      VertexIndex neighbor = flowGraph.graph.TargetVertex( edge );
      if( flowGraph.vertices[ orphan ].root != flowGraph.vertices[ neighbor ].root ) {
         continue;
      }
      // If the edge has capacity left to flow from neighbor into orphan, the neighbor becomes active
      if( !flowGraph.vertices[ neighbor ].isActive ) {
         dfloat residual = isSource ? flowGraph.ReverseResidual( edge )
                                    : flowGraph.Residual( edge );
         if( residual > 0 ) {
            flowGraph.vertices[ neighbor ].isActive = true;
            activeNodes.PushBack( neighbor );
         }
      }
      // If the neighbor is a child, make it an orphan
      if( flowGraph.vertices[ neighbor ].parent == orphan ) {
         flowGraph.vertices[ neighbor ].parent = ORPHAN;
         orphanNodes.PushBack( neighbor );  // We process this one after everything else
      }
   }
   // Step 2: reset the node
   flowGraph.vertices[ orphan ].parent = NO_PARENT;
   flowGraph.vertices[ orphan ].parentEdge = 0;
   flowGraph.vertices[ orphan ].root = 0;
   flowGraph.vertices[ orphan ].isActive = false;
}

} // namespace

void GraphCut( DirectedGraph& graph, DirectedGraph::VertexIndex sourceIndex, DirectedGraph::VertexIndex sinkIndex ) {
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
   DeQueue< VertexIndex > activeNodes( graph.NumberOfVertices() );
   activeNodes.PushBack( sourceIndex );
   activeNodes.PushBack( sinkIndex );
   DeQueue< VertexIndex > orphanNodes( graph.NumberOfVertices() );
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
         finalize( flowGraph );
         return;
      }
      // Have flow go through this path; this might detach some nodes from the trees, which become orphans
      //std::cout << "   - augment:\n";
      augment( flowGraph, pathEdge, orphanNodes );
      // Re-attach the orphans if possible
      //std::cout << "   - adopt:\n";
      while( !orphanNodes.Empty() ) {
         adopt_next( flowGraph, orphanNodes, activeNodes );
      }
   }
}

} // namespace dip
