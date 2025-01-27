/*
 * (c)2018, Cris Luengo.
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

#include "diplib/segmentation.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/overload.h"

#include "../histogram/threshold_algorithms.h"

/* Algorithm:
  - Compute Sum() projections.
  - For each projection, compute mean, variance, optimal partition (Otsu), and variances of the two partitions.
  - Create a "partition" object using the data for the dimension whose partition most reduces the variance.
  - Build a priority queue for partition objects. Priority = decrease of variance if partition is split.
  - Handle partition objects in descending priority:
     - Take the top projection object.
     - Split along the best dimension.
     - Re-compute the associated projections.
     - Add 2 new projection objects to the priority queue.
  - Each of the partition objects on the queue are leaves of the k-d tree.
  - Each time we take a partition off the queue, we add a branch node to the k-d tree.
*/

namespace dip {

namespace {

using ProjectionType = dfloat;
using Projection = std::vector< ProjectionType >;
using ProjectionArray = std::vector< Projection >;

typedef ProjectionArray ComputeSumProjectionsFunction( Image const&, UnsignedArray const&, UnsignedArray const& );

template< typename TPI >
ProjectionArray ComputeSumProjections(
      Image const& img,
      UnsignedArray const& leftEdges,
      UnsignedArray const& rightEdges
) {
   DIP_ASSERT( img.DataType() == DataType( TPI( 0 )));
   dip::uint nDims = img.Dimensionality();
   ProjectionArray out( nDims );
   UnsignedArray sizes( nDims );
   for( dip::uint dim = 0; dim < nDims; ++dim ) {
      DIP_ASSERT( leftEdges[ dim ] <= rightEdges[ dim ] );
      sizes[ dim ] = rightEdges[ dim ] - leftEdges[ dim ] + 1;
      out[ dim ].resize( sizes[ dim ] );
      std::fill( out[ dim ].begin(), out[ dim ].end(), 0 ); // resize initializes new values to 0, but we don't know what was there before.
   }
   ImageIterator< TPI > it( img, leftEdges, sizes );
   do {
      for( dip::uint dim = 0; dim < nDims; ++dim ) {
         dip::uint ii = it.Coordinates()[ dim ];
         out[ dim ][ ii ] += static_cast< ProjectionType >( *it );
      }
   } while( ++it );
   return out;
}

class KDTree {
   private:

      struct Partition {
         dip::uint nPixels;         // number of pixels represented in this partition
         UnsignedArray leftEdges;   // the left edges of the partition (i.e. top-left corner)
         UnsignedArray rightEdges;  // the right edges of the partition (i.e. bottom-right corner)
         UnsignedArray mean;        // location of the mean
         dip::uint optimalDim;      // dimension along which to split, if needed
         dip::uint threshold;       // location at which to split
         dfloat variance;           // weighted variance along optimalDim
         dfloat splitVariances;     // sum of weighted variances along optimalDim if split
         Image const& image;
         ComputeSumProjectionsFunction* computeSumProjections;

         explicit Partition( Image const& img ) : image( img ) {} // NOLINT(*-pro-type-member-init)

         void SetRootPartition() {
            nPixels = image.NumberOfPixels();
            dip::uint nDims = image.Dimensionality();
            leftEdges.resize( nDims, 0 );
            rightEdges = image.Sizes();
            rightEdges -= 1;
            DIP_OVL_ASSIGN_NONCOMPLEX( computeSumProjections, ComputeSumProjections, image.DataType() );
            FindOptimalSplit( computeSumProjections( image, leftEdges, rightEdges ));
         }

         // Computes optimal split for this partition
         void FindOptimalSplit( ProjectionArray const& projections ) {
            dip::uint nDims = image.Dimensionality();
            mean = leftEdges;
            optimalDim = 0;
            threshold = 0;
            variance = 0;
            splitVariances = 1e6; // larger than variance, will be overwritten for sure
            for( dip::uint ii = 0; ii < nDims; ++ii ) {
               if( projections[ ii ].size() > 1 ) {
                  ComputeVariances( ii, projections[ ii ] );
               }
            }
            // if variance == 0, we can't split this node
         }

         // Splits this partition along `optimalDim`, putting the right half into `other`
         void Split( Partition& other ) {
            DIP_ASSERT( variance > 0 ); // Otherwise we don't have a possible split
            //std::cout << "Splitting along dimension " << optimalDim << ", threshold = " << threshold << '\n';
            //std::cout << "leftEdge = " << leftEdges[ optimalDim ] << ", rightEdge = " << rightEdges[ optimalDim ] << '\n';
            dip::uint n = nPixels / ( rightEdges[ optimalDim ] - leftEdges[ optimalDim ] + 1 );
            dip::uint leftSize = threshold - leftEdges[ optimalDim ] + 1;
            dip::uint rightSize = rightEdges[ optimalDim ] - threshold;
            other.nPixels = n * rightSize;
            nPixels = n * leftSize;
            other.leftEdges = leftEdges;
            other.leftEdges[ optimalDim ] = threshold + 1;
            other.rightEdges = rightEdges;
            rightEdges[ optimalDim ] = threshold;
            other.computeSumProjections = computeSumProjections;
            FindOptimalSplit( computeSumProjections( image, leftEdges, rightEdges ));
            other.FindOptimalSplit( computeSumProjections( other.image, other.leftEdges, other.rightEdges ));
         }

         // Computes the mean, variance, and threshold for dimension `dim`. If this split is better than the
         // current one, replaces `optimalDim`, `threshold`, `variance` and `splitVariances`.
         void ComputeVariances( dip::uint dim, Projection const& projection ) {
            ProjectionType const* data = projection.data();
            dip::uint nBins = projection.size();
            dip::uint maxInd = OtsuThreshold( data, nBins );
            if( maxInd == nBins ) {
               // This is an issue! Let's try splitting half-way the range.
               maxInd = nBins / 2;
            }
            // Find the variances for this dimension and this split
            // w0, w1, w2 are the probabilities of the full histogram and each of the halves when thresholded between maxInd and maxInd+1
            dfloat w0 = 0;
            dfloat w1 = 0;
            dfloat w2 = 0;
            // m0, m1, m2 are the corresponding first order moments
            dfloat m0 = 0;
            dfloat m1 = 0;
            dfloat m2 = 0;
            // mm0, mm1, mm2 are the corresponding second order moments
            dfloat mm0 = 0;
            dfloat mm1 = 0;
            dfloat mm2 = 0;
            for( dip::uint ii = 0; ii < nBins; ++ii ) {
               dfloat tmp = static_cast< dfloat >( data[ ii ] );
               w0 += tmp;
               if( ii <= maxInd ) {
                  w1 += tmp;
               } else {
                  w2 += tmp;
               }
               tmp *= static_cast< dfloat >( ii );
               m0 += tmp;
               if( ii <= maxInd ) {
                  m1 += tmp;
               } else {
                  m2 += tmp;
               }
               tmp *= static_cast< dfloat >( ii );
               mm0 += tmp;
               if( ii <= maxInd ) {
                  mm1 += tmp;
               } else {
                  mm2 += tmp;
               }
            }
            // Fill in the mean value for this dimension as it is before being split
            if( w0 > 0 ) {
               mean[ dim ] = leftEdges[ dim ] + static_cast< dip::uint >( round_cast( m0 / w0 ));
            }
            // Compute weighted variances mm0 = w0 * s0^2. We use biased variances (divide by w0 instead of w0-1) because that's what Otsu does too.
            if(( w1 > 0 ) && ( w2 > 0 )) { // If this is false, we cannot split the node along this dimension.
               mm0 -= ( m0 * m0 ) / w0;
               mm1 -= ( m1 * m1 ) / w1;
               mm2 -= ( m2 * m2 ) / w2;
               if(( variance - splitVariances ) < ( mm0 - mm1 - mm2 )) {
                  variance = mm0;
                  splitVariances = mm1 + mm2;
                  optimalDim = dim;
                  threshold = leftEdges[ dim ] + maxInd;
               }
            }
         }
      };

      struct Node {
         std::unique_ptr< Partition > partition; // can be deleted for non-leaf nodes, info only useful at the leaves.
         dip::uint dimension = 0;   // along which dimension to threshold
         dip::uint threshold = 0;   // the threshold value, as an index
         dip::uint left = 0;        // index for left child (value <= threshold) -- 0 if leaf node
         dip::uint right = 0;       // index for right child (value > threshold) -- 0 if leaf node
         LabelType label = 0;       // 0 if not leaf node
         explicit Node( LabelType lab ) : label( lab ) {};
      };
      std::vector< Node > nodes;
      LabelType lastLabel = 0; // also equal to nClusters.
      dip::Image const& image;

      void SplitPartition( dip::uint index ) {
         DIP_ASSERT( nodes[ index ].left == 0 );
         DIP_ASSERT( nodes[ index ].right == 0 );
         DIP_ASSERT( nodes[ index ].partition->variance > 0 ); // This means it can be split
         nodes[ index ].left = nodes.size();
         nodes.emplace_back( nodes[ index ].label );
         nodes[ index ].right = nodes.size();
         nodes.emplace_back( ++lastLabel );
         Node& node = nodes[ index ]; // Don't take reference to node before emplace_back calls.
         Node& left = nodes[ node.left ];
         Node& right = nodes[ node.right ];
         node.label = 0;
         DIP_ASSERT( node.partition );
         node.dimension = node.partition->optimalDim;
         node.threshold = node.partition->threshold;
         left.partition = std::move( node.partition );               // move Partition data from node to left child
         right.partition = std::make_unique< Partition >( image );   // right child gets a new Partition object.
         left.partition->Split( *right.partition );                  // Splits the partition data
      }

      // Tail-recursive helper function for `Lookup`
      std::pair<LabelType, dip::uint> LookupStartingAt( dip::uint node, UnsignedArray const& coords, dip::uint procDim ) const {
         auto const& n = nodes[ node ];
         if( n.label != 0 ) {
            return std::make_pair( n.label, n.partition->rightEdges[ procDim ] );
         }
         return LookupStartingAt( coords[ n.dimension ] > n.threshold ? n.right : n.left, coords, procDim );
      }

   public:
      // Create the tree
      KDTree( Image const& img, dip::uint nClusters ) : image( img ) {
         DIP_ASSERT( img.IsForged() );
         DIP_ASSERT( img.IsScalar() );
         // Create root node
         nodes.emplace_back( ++lastLabel );
         nodes[ 0 ].partition = std::make_unique< Partition >( img );
         nodes[ 0 ].partition->SetRootPartition();
         // Create queue
         auto ComparePartitions = [ & ]( dip::uint lhsIndex, dip::uint rhsIndex ) {
            // Implements lhs < rhs
            // The best split has the largest reduction in variances; for equal reduction, the partition with the most pixels is split.
            Partition* lhs = nodes[ lhsIndex ].partition.get();
            Partition* rhs = nodes[ rhsIndex ].partition.get();
            // Sort nodes that can't be split at the end of the queue.
            if( lhs->variance == 0 ) {
               return true;
            }
            if( rhs->variance == 0 ) {
               return false;
            }
            dfloat cmp = ( lhs->variance - lhs->splitVariances ) - ( rhs->variance - rhs->splitVariances );
            return cmp == 0 ? ( lhs->nPixels < rhs->nPixels ) : ( cmp < 0 );
         };
         std::priority_queue< dip::uint, std::vector< dip::uint >, decltype( ComparePartitions ) > queue( ComparePartitions );
         queue.push( 0 );
         while( --nClusters ) {
            dip::uint index = queue.top();
            queue.pop();
            if( nodes[ index ].partition->variance == 0 ) {
               // We can no longer split partitions, we're done!
               return;
            }
            SplitPartition( index );
            queue.push( nodes[ index ].left );
            queue.push( nodes[ index ].right );
         }
      }

      // Look up the label for the given coordinates
      // 2nd value in pair is the last coordinate along `procDim` within this node.
      std::pair<LabelType, dip::uint> Lookup( UnsignedArray const& coords, dip::uint procDim ) const {
         return LookupStartingAt( 0, coords, procDim );
      }

      // Return the centroids
      CoordinateArray Centroids() const {
         CoordinateArray out( lastLabel );
         for( auto const& node : nodes ) {
            if( node.label > 0 ) {
               DIP_ASSERT( node.label - 1 < lastLabel );
               out[ node.label - 1 ] = node.partition->mean;
            }
         }
         return out;
      }
};

class PaintClustersLineFilter : public Framework::ScanLineFilter {
   public:
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         LabelType* out = static_cast< LabelType* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint procDim = params.dimension;
         UnsignedArray pos = params.position;
         dip::uint end = pos[ procDim ] + bufferLength;
         do {
            LabelType label{};
            dip::uint last{};
            std::tie( label, last ) = clusters_.Lookup( pos, procDim );
            for( ; pos[ procDim ] <= last; ++pos[ procDim ], out += outStride ) {
               *out = label;
            }
         } while( pos[ procDim ] < end );
      }
      PaintClustersLineFilter( KDTree const& clusters ) : clusters_( clusters ) {}
   private:
      KDTree const& clusters_;
};

void PaintClusters( Image& labs, KDTree const& clusters ) {
   bool prot = labs.Protect();
   ImageRefArray outImage{ labs };
   DataTypeArray outBufferTypes{ DT_LABEL };
   DataTypeArray outImageTypes{ DT_LABEL };
   PaintClustersLineFilter lineFilter( clusters );
   DIP_STACK_TRACE_THIS( Framework::Scan( {}, outImage, {}, outBufferTypes, outImageTypes, { 1 }, lineFilter,
                                          Framework::ScanOption::NeedCoordinates + Framework::ScanOption::NoMultiThreading ));
   labs.Protect( prot );
}

} // namespace

CoordinateArray MinimumVariancePartitioning(
      Image const& in,
      Image& out,
      dip::uint nClusters
) {
   // Check the image
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( nClusters < 2, "Number of clusters must be 2 or larger" );
   DIP_THROW_IF( nClusters > std::numeric_limits< LabelType >::max(), "Number of clusters is too large" );
   KDTree clusters( in, nClusters );
   out.ReForge( in, DT_LABEL );
   PaintClusters( out, clusters );
   return clusters.Centroids();
}

} // namespace dip
