/*
 * (c)2017-2022, Cris Luengo.
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

#include "diplib/nonlinear.h"

#include <cmath>
#include <memory>
#include <numeric>
#include <vector>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/framework.h"
#include "diplib/morphology.h"
#include "diplib/overload.h"
#include "diplib/pixel_table.h"


// TODO: a binary specialization would just count the number of `true` pixels.


namespace dip {

namespace {

// Weight balanced tree to implement an order statistic tree. But each node has a count to avoid creating nodes for duplicate values.
// See https://en.wikipedia.org/wiki/Weight-balanced_tree
// See https://en.wikipedia.org/wiki/Order_statistic_tree
// Balancing as described in Y. Hirai, K. Yamamoto, "Balancing weight-balanced trees", Journal of Functional Programming 21(3):287.
//    https://yoichihirai.com/bst.pdf
template< typename T >
class OrderStatisticTree {

   public:

      // Constructor. By default we can't store anything in here
      OrderStatisticTree() = default;

      // Destructor
      ~OrderStatisticTree() = default;

      // Default move operators should do the right thing
      OrderStatisticTree( OrderStatisticTree&& ) = default;
      OrderStatisticTree& operator=( OrderStatisticTree&& ) = default;

      // We can only copy the default-initialized tree, not one with data (that would be difficult to do with all those pointers)
      OrderStatisticTree( OrderStatisticTree const& other ) {
         DIP_ASSERT( other.nodes.empty() );
         DIP_ASSERT( nodes.empty() );
         DIP_ASSERT( root == nullptr );
         DIP_ASSERT( free == nullptr );
         ( void ) other;
      };
      OrderStatisticTree& operator=( OrderStatisticTree const& other ) { // NOLINT(*-unhandled-self-assignment)
         DIP_ASSERT( other.nodes.empty() );
         ( void ) other;
         nodes.clear();
         root = nullptr;
         free = nullptr;
      }

      // Removes everything from the tree, resizes to max `N` nodes
      void Clear( dip::uint N ) {
         nodes.resize( N );
         root = nullptr;
         free = &nodes[ 0 ];
         Node* p = free;
         for( dip::uint ii = 1; ii < N; ++ii ) {
            p->parent = &nodes[ ii ];
            p = p->parent;
         }
         p->parent = nullptr;
      }

      // Insert a value into the tree. There is no balancing of the tree, so this by itself could be inefficient
      void Insert( T value ) {
         DIP_ASSERT( free != nullptr );

         // Insert at root if empty tree
         if( root == nullptr ) {
            Node* p = NewNode( value );
            root = p;
            p->parent = nullptr;
            return;
         }

         // Find insertion point
         Node* q = root;
         dip::uint I{};
         while( true ) {
            //DIP_ASSERT( q != nullptr );
            if( value == q->value ) {
               // Don't make a new node, just increment the count
               ++( q->count );
               // Update size up the tree
               while( q != nullptr ) {
                  ++( q->size );
                  q = q->parent;
               }
               return;
            }
            I = value > q->value ? 1 : 0;
            if( q->child[ I ] == nullptr ) {
               break;
            }
            q = q->child[ I ];
         }

         // Insert it
         Node* p = NewNode( value );
         q->child[ I ] = p;
         p->parent = q;

         // Update size up the tree
         while( q != nullptr ) {
            ++( q->size );
            q = q->parent;
         }

         // Rebalance the tree
         Rebalance( p );
      }

      // Remove a value from the tree; the value must be in it.
      // How we delete improves balance of the tree, so we don't explicitly call Rebalance().
      void Remove( T value ) {
         // Which node to remove?
         Node* p = Find( value );
         DIP_ASSERT( p != nullptr );
         DIP_ASSERT( p->value == value );
         DIP_ASSERT( p->count > 0 );

         if( p->count > 1 ) {
            --( p->count );
            while( p != nullptr ) {
               DIP_ASSERT( p->size > 0 );
               --( p->size );
               p = p->parent;
            }
            return;
         }

         // Remember who the parent is
         Node* parent = p->parent;

         // Remove the node
         if( p->child[ 0 ] == nullptr ) {
            // Put p's right child in its place
            ReplaceChild( p->parent, p, p->child[ 1 ] );
         } else if( p->child[ 1 ] == nullptr ) {
            // Put p's left child in its place
            ReplaceChild( p->parent, p, p->child[ 0 ] );
         } else {
            // Both children live; which one is smaller?
            dip::uint I = p->child[ 0 ]->size > p->child[ 1 ]->size ? 1 : 0;
            //    Note: Names in comments below assume I==1, things are reversed for the case I==0.
            // Put p's predecessor in its place
            Node* q = PreOrSuccessor( p, I );
            // p's right child becomes the predecessor's right child
            DIP_ASSERT( q->child[ I ] == nullptr );
            q->child[ I ] = p->child[ I ];
            q->child[ I ]->parent = q;
            q->size += q->child[ I ]->size;
            // If q is not a direct child of p.
            Node* r = q->parent;
            if( r != p ) {
               // Update size for r's parent and up until p
               Node* rr = r->parent;
               while( rr != p ) {
                  rr->size -= q->count;
                  rr = rr->parent;
               }
               // Put q's potential left child in its place
               DIP_ASSERT( r->child[ I ] == q );
               ReplaceChild( r, q, q->child[ 1 - I ] );
               // Put p's left child as q's left child
               ReplaceChildByIndex( q, 1 - I, p->child[ 1 - I ] );
            }
            // And finally put q in p's place
            ReplaceChild( parent, p, q );
         }

         // Add the node to the free list
         p->parent = free;
         free = p;

         // Update size up the tree (parent has already been updated!)
         if( parent != nullptr ) {
            p = parent->parent;
            while( p != nullptr ) {
               DIP_ASSERT( p->size > 0 );
               --( p->size );
               p = p->parent;
            }
         }
      }

      // Find the value for the given rank, in the range [0, N).
      T Select( dip::uint k ) const {
         DIP_ASSERT( root != nullptr );
         DIP_ASSERT( root->size == nodes.size() ); // Have we kept the size up to date correctly?
         DIP_THROW_IF( k >= nodes.size(), dip::E::PARAMETER_OUT_OF_RANGE );
         Node* p = root;
         dip::uint nUp = 0;
         while( p != nullptr ) {
            dip::uint nLeft = Size( p->child[ 0 ] );
            if( nUp + nLeft > k ) {
               p = p->child[ 0 ];
            } else {
               nUp += nLeft + p->count;
               if( nUp > k ) {
                  return p->value;
               }
               p = p->child[ 1 ];
            }
         }
         //PrintTree();
         DIP_THROW_ASSERTION( "Internal error: OrderStatisticTree<> is likely inconsistent" );
      }

      // Used for testing everything is OK.
      void ValidateTree() const {
         ValidateNode( root );
      }

   private:

      // Delta is the weight balance. weight = size + 1. A node is in balance if
      //    delta * weight(child[0]) >= weight(child[1])
      // and
      //    delta * weight(child[1]) >= weight(child[0])
      static constexpr dip::uint delta = 3;
      // Gamma is the second weight balance, used to decide between a single or a double rotation
      static constexpr dip::uint gamma = 2;


      struct Node {
         Node* parent = nullptr;
         Node* child[ 2 ] = { nullptr, nullptr }; // child[ 0 ] is left, child[ 1 ] is right, child[ 0 ] <= child[ 1 ]
         dip::uint size = 0;
         dip::uint count = 1; // number of elements with this value
         T value = 0;
      };

      std::vector< Node > nodes; // fixed container for all N nodes that the tree will ever contain
      Node* root = nullptr;      // pointer to root of tree
      Node* free = nullptr;      // links all free nodes through their parent pointer

      Node* NewNode( T value ) {
         DIP_ASSERT( free != nullptr );
         Node* p = free;
         free = p->parent;
         p->value = value;
         p->child[ 0 ] = nullptr;
         p->child[ 1 ] = nullptr;
         p->size = 1;
         p->count = 1;
         return p;
      }

      void ValidateNode( Node* p ) const {
         if( p ) {
            DIP_ASSERT( Size( p->child[ 0 ] ) + Size( p->child[ 1 ] ) + p->count == p->size );
            ValidateNode( p->child[ 0 ] );
            ValidateNode( p->child[ 1 ] );
         }
      }

      // Find the node with `value`, or nullptr if not found
      Node* Find( T value ) const {
         if( root == nullptr ) {
            return nullptr;
         }
         return Find( value, root );
      }

      // Idem, starting from node `p`
      Node* Find( T value, Node* p ) const {
         while( true ) {
            if(( p == nullptr ) || ( p->value == value )) {
               return p;
            }
            dip::uint I = value > p->value ? 1 : 0;
            p = p->child[ I ];
         }
      }

      // Returns the node that comes next (successor) or previously (predecessor) in in-order traversal,
      // or nullptr if p has no right child.
      // I==1: Predecessor(), I==0: Successor()
      Node* PreOrSuccessor( Node* p, dip::uint I ) const {
         if( p->child[ 1 - I ] == nullptr ) {
            return nullptr;
         }
         p = p->child[ 1 - I ];
         while( p->child[ I ] != nullptr ) {
            p = p->child[ I ];
         }
         return p;
      }

      // Replace p's old_child with new_child
      void ReplaceChild( Node* p, Node* old_child, Node* new_child ) {
         if( p == nullptr ) {
            DIP_ASSERT( root == old_child );
            root = new_child;
            if( new_child != nullptr ) {
               new_child->parent = nullptr;
            }
         } else {
            if( p->child[ 0 ] == old_child ) {
               ReplaceChildByIndex( p, 0, new_child );
            } else {
               DIP_ASSERT( p->child[ 1 ] == old_child );
               ReplaceChildByIndex( p, 1, new_child );
            }
         }
      }

      // Replace p->child[ I ] with new_child. `p` must be a node
      void ReplaceChildByIndex( Node* p, dip::uint I, Node* new_child ) {
         DIP_ASSERT( p != nullptr );
         p->child[ I ] = new_child;
         p->size = Size( p->child[ 0 ] ) + Size( p->child[ 1 ] ) + p->count;
         if( new_child != nullptr ) {
            new_child->parent = p;
         }
      }

      // Returns the `size` member of node p if it exists, otherwise 0
      dip::uint Size( Node* p ) const {
         return p == nullptr ? 0 : p->size;
      }

      // Rebalance the tree, starting at node p and moving up to the root
      void Rebalance( Node* p ) {
         DIP_ASSERT( p != nullptr );
         while( p != nullptr ) {
            Node* q = p->parent;
            dip::uint leftWeight = Size( p->child[ 0 ] ) + 1;
            dip::uint rightWeight = Size( p->child[ 1 ] ) + 1;
            if( !(( delta * leftWeight ) >= rightWeight ) || !(( delta * rightWeight ) >= leftWeight )) {
               if( leftWeight > rightWeight ) {
                  // Rotate right
                  Node* r = p->child[ 0 ];
                  DIP_ASSERT( r != nullptr );
                  if(( Size( r->child[ 1 ] ) + 1 ) < ( Size( r->child[ 0 ] ) + 1 ) * gamma ) {
                     // Single rotation
                     Rotate( p, 0 );
                  } else {
                     // Double rotation
                     RotateDouble( p, 0 );
                  }
               } else {
                  // Rotate left
                  Node* r = p->child[ 1 ];
                  DIP_ASSERT( r != nullptr );
                  if(( Size( r->child[ 0 ] ) + 1 ) < ( Size( r->child[ 1 ] ) + 1 ) * gamma ) {
                     // Single rotation
                     Rotate( p, 1 );
                  } else {
                     // Double rotation
                     RotateDouble( p, 1 );
                  }
               }
            }
            p = q;
         }
      }

      // p is replaced by q = p->child[ I ], p becomes q->child[ 1 - I ]
      // Left rotation: I = 1
      // Right rotation: I = 0
      void Rotate( Node* p, dip::uint I ) {
         DIP_ASSERT( p != nullptr );
         Node* q = p->child[ I ];
         DIP_ASSERT( q != nullptr );
         Node* x = q->child[ 1 - I ];
         Node* parent = p->parent;
         // x moves to p->child[ I ]
         p->child[ I ] = x;
         p->size -= q->size;
         if( x ) {
            x->parent = p;
            p->size += x->size;
         }
         // p becomes q->child[ 1 - I ]
         q->child[ 1 - I ] = p;
         p->parent = q;
         q->size = p->size + Size( q->child[ I ] ) + q->count;
         // p's parent points to q
         ReplaceChild( parent, p, q );
      }

      // p is replaced by r = p->child[I]->child[1-I], p becomes r->child[1-I], q = p->child[I] becomes r->child[I]
      // Left double rotation: I = 1
      // Right double rotation: I = 0
      void RotateDouble( Node* p, dip::uint I ) {
         DIP_ASSERT( p != nullptr );
         Node* q = p->child[ I ];
         DIP_ASSERT( q != nullptr );
         Node* r = q->child[ 1 - I ];
         DIP_ASSERT( r != nullptr );
         Node* x = r->child[ 1 - I ];
         Node* y = r->child[ I ];
         Node* parent = p->parent;
         // x moves to p->child[ I ];
         p->child[ I ] = x;
         p->size -= q->size;
         if( x ) {
            x->parent = p;
            p->size += x->size;
         }
         // y moves to q->child[ 1 - I ];
         q->child[ 1 - I ] = y;
         q->size -= r->size;
         if( y ) {
            y->parent = q;
            q->size += y->size;
         }
         // p becomes r->child[ 1 - I ];
         r->child[ 1 - I ] = p;
         p->parent = r;
         // q becomes r->child[ I ];
         r->child[ I ] = q;
         q->parent = r;
         // Update r's size
         r->size = p->size + q->size + r->count;
         // Move r into p's position
         ReplaceChild( parent, p, r );
      }

};

template< typename TPI >
class RankLineFilter : public Framework::FullLineFilter {
   public:
      RankLineFilter( dip::uint rank ) : rank_( rank ) {}
      void SetNumberOfThreads( dip::uint threads, PixelTableOffsets const& pixelTable ) override {
         dip::uint nKernelPixels = pixelTable.NumberOfPixels();
         dip::uint nRuns = pixelTable.Runs().size();
         useBinaryTreeMethod_ = UseBinaryTreeMethod( nKernelPixels, nRuns );
         if( useBinaryTreeMethod_ ) {
            trees_.resize( threads );
         } else {
            buffers_.resize( threads );
            offsets_ = pixelTable.Offsets();
         }
      }
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint /**/, dip::uint nKernelPixels, dip::uint nRuns ) override {
         if( UseBinaryTreeMethod( nKernelPixels, nRuns )) {
            // Very rough guess here... Where did I get those 10 from?
            return 10 * nKernelPixels * static_cast< dip::uint >( std::round( std::log( nKernelPixels )))
                   + static_cast< dip::uint >( std::round(
                         static_cast< dfloat >( lineLength * nRuns ) * ( 2 * std::log( nKernelPixels ) + 10 )
                   ));
         }
         return lineLength * (
               nKernelPixels // copying
               + 3 * nKernelPixels * static_cast< dip::uint >( std::round( std::log( nKernelPixels )))  // sorting
               + 2 * nKernelPixels + nRuns );   // iterating over pixel table
      }
      void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         if( useBinaryTreeMethod_ ) {
            PixelTableOffsets const& pixelTable = params.pixelTable;
            auto& tree = trees_[ params.thread ];
            tree.Clear( pixelTable.NumberOfPixels() );
            for( auto offset: pixelTable ) {
               tree.Insert( in[ offset ] );
            }
            *out = tree.Select( rank_ );
            for( dip::uint ii = 1; ii < length; ++ii ) {
               for( auto run: pixelTable.Runs() ) {
                  tree.Remove( in[ run.offset ] );
               }
               for( auto run: pixelTable.Runs()) {
                  tree.Insert( in[ run.offset + static_cast< dip::sint >( run.length ) * inStride ] );
               }
               in += inStride;
               out += outStride;
               *out = tree.Select( rank_ );
            }
         } else {
            buffers_[ params.thread ].resize( offsets_.size() );
            for( dip::uint ii = 0; ii < length; ++ii ) {
               TPI* buffer = buffers_[ params.thread ].data();
               for( auto offset : offsets_ ) {
                  *buffer = in[ offset ];
                  ++buffer;
               }
               auto ourGuy = buffers_[ params.thread ].begin() + static_cast< dip::sint >( rank_ );
               std::nth_element( buffers_[ params.thread ].begin(), ourGuy, buffers_[ params.thread ].end() );
               *out = *ourGuy;
               in += inStride;
               out += outStride;
            }
         }
      }
   private:
      dip::uint rank_;
      std::vector< OrderStatisticTree< TPI >> trees_;
      std::vector< std::vector< TPI >> buffers_;
      std::vector< dip::sint > offsets_;
      bool useBinaryTreeMethod_ = false;
      // A Heuristic to determine which algorithm to use
      bool UseBinaryTreeMethod( dip::uint nKernelPixels, dip::uint nRuns ) {
         // Data collected on Cris' desktop (Apply M1) for a sufficiently large image, a rectangular kernel,
         // and a floating-point image, suggests the following:
         return nKernelPixels / nRuns > 11;
         // TODO: this might depend also on the data type?
      }
};

void ComputeRankFilter(
      Image const& in,
      Image& out,
      Kernel const& kernel,
      dip::uint rank,
      BoundaryConditionArray const& bc
) {
   DIP_START_STACK_TRACE
      DataType dtype = in.DataType();
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_NONCOMPLEX( lineFilter, RankLineFilter, ( rank ), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, kernel, *lineFilter, Framework::FullOption::AsScalarImage );
   DIP_END_STACK_TRACE
}

} // namespace

void RankFilter(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      dip::uint rank,
      String const& order,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !se.IsFlat(), E::KERNEL_NOT_BINARY );
   Kernel kernel{};
   dip::uint nPixels{};
   DIP_START_STACK_TRACE
      kernel = se.Kernel();
      nPixels = kernel.NumberOfPixels( in.Dimensionality() );
   DIP_END_STACK_TRACE
   DIP_THROW_IF(( rank < 1 ) || ( rank > nPixels ), E::PARAMETER_OUT_OF_RANGE );
   DIP_START_STACK_TRACE
      if( !BooleanFromString( order, S::INCREASING, S::DECREASING )) {
         rank = nPixels - rank + 1;
      }
   DIP_END_STACK_TRACE
   if( rank == 1 ) {
      DIP_STACK_TRACE_THIS( Erosion( in, out, se, boundaryCondition ));
   } else if( rank == nPixels ) {
      DIP_STACK_TRACE_THIS( Dilation( in, out, se, boundaryCondition ));
   } else {
      DIP_START_STACK_TRACE
         BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
         if( bc.empty() ) {
            if( rank <= nPixels / 2 ) {
               bc.push_back( BoundaryCondition::ADD_MAX_VALUE );
            } else {
               bc.push_back( BoundaryCondition::ADD_MIN_VALUE );
            }
         }
         ComputeRankFilter( in, out, kernel, rank - 1, bc );
      DIP_END_STACK_TRACE
   }
}

void PercentileFilter(
      Image const& in,
      Image& out,
      dfloat percentile,
      Kernel const& kernel,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( kernel.HasWeights(), E::KERNEL_NOT_BINARY );
   DIP_THROW_IF(( percentile < 0.0 ) || ( percentile > 100.0 ), E::PARAMETER_OUT_OF_RANGE );
   DIP_START_STACK_TRACE
      dip::uint nPixels = kernel.NumberOfPixels( in.Dimensionality() );
      dip::uint rank = RankFromPercentile( percentile, nPixels );
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      ComputeRankFilter( in, out, kernel, rank, bc );
   DIP_END_STACK_TRACE
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"

DOCTEST_TEST_CASE("[DIPlib] testing OrderStatisticTree<>") {
   std::vector< int > data( 21 );
   std::iota( data.begin(), data.end(), -10 );  // data contains values -10 to 10.
   dip::OrderStatisticTree< int > tree;
   tree.Clear( 11 );
   for( dip::uint ii = 0; ii < 11; ++ii ) {
      tree.Insert( data[ ii ] );
   }
   DOCTEST_CHECK_NOTHROW( tree.ValidateTree() );
   DOCTEST_CHECK( tree.Select( 5 ) == -5 );
   for( dip::uint ii = 0; ii < 5; ++ii ) {
      tree.Remove( data[ ii ] );
   }
   DOCTEST_CHECK_NOTHROW( tree.ValidateTree() );
   for( dip::uint ii = 11; ii < 16; ++ii ) {
      tree.Insert( data[ ii ] );
   }
   DOCTEST_CHECK_NOTHROW( tree.ValidateTree() );
   DOCTEST_CHECK( tree.Select( 5 ) == 0 );
   for( dip::uint ii = 5; ii < 10; ++ii ) {
      tree.Remove( data[ ii ] );
   }
   DOCTEST_CHECK_NOTHROW( tree.ValidateTree() );
   for( dip::uint ii = 16; ii < 21; ++ii ) {
      tree.Insert( data[ ii ] );
   }
   DOCTEST_CHECK_NOTHROW( tree.ValidateTree() );
   DOCTEST_CHECK( tree.Select( 5 ) == 5 );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
