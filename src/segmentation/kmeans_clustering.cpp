/*
 * (c)2018-2021, Cris Luengo.
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
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/random.h"

namespace dip {

namespace {

struct Cluster {
   FloatArray mean;
   FloatArray newMean;
   dfloat norm = 0.0;
   LabelType label = 0;
   explicit Cluster( dip::uint nDim ) : mean( nDim, 0.0 ), newMean( nDim, 0.0 ) {}
};

using ClusterArray = std::vector< Cluster >;

template< typename TPI >
class ClusteringLineFilter : public Framework::ScanLineFilter {
   public:
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // Either we have one input image, or one output image.
         TPI const* in = nullptr;
         LabelType* out = nullptr;
         dip::sint inStride = 0;
         dip::sint outStride = 0;
         if( !params.inBuffer.empty() ) {
            in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
            inStride = params.inBuffer[ 0 ].stride;
         } else {
            out = static_cast< LabelType* >( params.outBuffer[ 0 ].buffer );
            outStride = params.outBuffer[ 0 ].stride;
         }
         DIP_ASSERT(( out == nullptr ) ^ ( in == nullptr )); // make sure one and only one is nullptr
         dip::uint bufferLength = params.bufferLength;
         dip::uint scanDim = params.dimension;
         auto& pos = params.position;
         dip::uint nDims = pos.size();
         // Initialise the cluster array
         std::vector< dfloat > distCache( clusters_.size(), 0.0 );
         for( dip::uint ii = 0; ii < clusters_.size(); ++ii ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               if( jj != scanDim ) {
                  dfloat dist = clusters_[ ii ].mean[ jj ] - static_cast< dfloat >( pos[ jj ] );
                  distCache[ ii ] += dist * dist;
               }
            }
         }
         // Process the scan line
         for( dip::uint xx = pos[ scanDim ]; xx < pos[ scanDim ] + bufferLength; ++xx ) {
            // Find the nearest cluster center
            dip::uint nearest = 0;
            dfloat nearestDist = std::numeric_limits< dfloat >::max();
            for( dip::uint ii = 0; ii < clusters_.size(); ++ii ) {
               dfloat dist = clusters_[ ii ].mean[ scanDim ] - static_cast< dfloat >( xx );
               dist = dist * dist + distCache[ ii ];
               if( dist < nearestDist ) {
                  nearest = ii;
                  nearestDist = dist;
               }
            }
            if( out ) {
               // Write cluster label to output image
               *out = clusters_[ nearest ].label;
               out += outStride;
            } else {
               // Update the new mean of nearest mean
               for( dip::uint ii = 0; ii < nDims; ++ii ) {
                  clusters_[ nearest ].newMean[ ii ] += static_cast< dfloat >( *in ) * static_cast< dfloat >( pos[ ii ] );
               }
               clusters_[ nearest ].newMean[ scanDim ] += static_cast< dfloat >( *in ) * static_cast< dfloat >( xx );
               clusters_[ nearest ].norm += static_cast< dfloat >( *in );
               in += inStride;
            }
         }
      }
      ClusteringLineFilter( ClusterArray& clusters ) : clusters_( clusters ) {}
   private:
      ClusterArray& clusters_;
};

dfloat Clustering(
      Image const& in,
      Image& out,
      ClusterArray& clusters,
      bool write  // if `!write`, `out` is left unused
) {
   DataType ovlDataType = in.DataType();
   if( ovlDataType.IsBinary() ) {
      ovlDataType = DT_UINT8; // Reading binary images as if they were uint8.
   }
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   DIP_OVL_NEW_REAL( lineFilter, ClusteringLineFilter, ( clusters ), ovlDataType );
   ImageConstRefArray inImages;
   ImageRefArray outImages;
   DataTypeArray inBufferTypes;
   DataTypeArray outBufferTypes;
   DataTypeArray outImageTypes;
   UnsignedArray nTensorElements;
   if( write ) {
      // We write cluster labels to `out`
      outImages.emplace_back( out );
      outBufferTypes.push_back( DT_LABEL );
      outImageTypes.push_back( DT_LABEL );
      nTensorElements.push_back( 1 );
      // Forge `out` -- we're not passing `in` to `Scan`, so it won't know how large to make `out`.
      out.ReForge( in, DT_LABEL, Option::AcceptDataTypeChange::DONT_ALLOW );
   } else {
      // We update clusters based on `in`
      inImages.emplace_back( in );
      inBufferTypes.push_back( ovlDataType );
   }
   DIP_STACK_TRACE_THIS( Framework::Scan( inImages, outImages, inBufferTypes, outBufferTypes, outImageTypes, nTensorElements, *lineFilter,
                                          Framework::ScanOption::NeedCoordinates + Framework::ScanOption::NoMultiThreading ));

   // Process cluster information
   dfloat change = 0;
   dfloat maxval = 0;
   if( !write ) {
      //std::cout << "Cluster means: ";
      for( auto& c : clusters ) {
         if( c.norm != 0.0 ) {
            for( dip::uint jj = 0; jj < c.mean.size(); jj++ ) {
               dfloat val = c.newMean[ jj ] / c.norm;
               maxval = std::max( std::abs( val ), maxval );
               dfloat dist = val - c.mean[ jj ];
               change += dist * dist;
               c.mean[ jj ] = val;
               c.newMean[ jj ] = 0.0;
            }
         } else {
            std::fill( c.newMean.begin(), c.newMean.end(), 0.0 );
         }
         c.norm = 0.0;
         //std::cout << c.mean << " ; ";
      }
      //std::cout << "change = " << change << '\n';
   }
   return change <= 1e-10 * maxval ? 0.0 : change;
}

void LabelClusters(
      ClusterArray& clusters
) {
   // Fill an array with the distances of the cluster means to the origin
   dip::uint size = clusters.size();
   FloatArray distances( size, 0.0 );
   for( dip::uint ii = 0; ii < size; ++ii ) {
      dfloat dist = 0.0;
      for( auto val : clusters[ ii ].mean ) {
         dist += val * val;
      }
      distances[ ii ] = dist;
   }

   // Sort this array
   auto indices = distances.sorted_indices();
   for( dip::uint ii = 0; ii < size; ++ii ) {
      clusters[ indices[ ii ]].label = static_cast< LabelType >( ii + 1 );
   }
}

} // namespace

CoordinateArray KMeansClustering(
      Image const& in,
      Image& out,
      Random& random,
      dip::uint nClusters
) {
   // Check the image
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( nClusters < 2, "Number of clusters must be 2 or larger" );
   DIP_THROW_IF( nClusters > std::numeric_limits< LabelType >::max(), "Number of clusters is too large" );

   // Allocate the cluster array
   dip::uint nDims = in.Dimensionality();
   ClusterArray clusters( nClusters, Cluster( nDims ));

   // Randomly initialise the clusters
   UniformRandomGenerator generator( random );
   for( auto& cluster : clusters ) {
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         cluster.mean[ jj ] = generator( 0, static_cast< dfloat >( in.Size( jj )));
      }
   }

   // Do cluster iterations
   while( Clustering( in, out, clusters, false ) > 0.0 ) {};
   LabelClusters( clusters );
   Clustering( in, out, clusters, true );

   // Copy over cluster centers to output array
   CoordinateArray coords( clusters.size() );
   for( auto& cluster : clusters ) {
      coords[ cluster.label - 1 ] = UnsignedArray( cluster.mean );
   }
   return coords;
}

} // namespace dip
