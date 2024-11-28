/*
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

#include "diplib/regions.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/graph.h"
#include "diplib/label_map.h"
#include "diplib/measurement.h"
#include "diplib/overload.h"
#include "diplib/statistics.h"

namespace dip {

namespace {

template< typename TPI >
class TouchingRegionAdjacencyGraphLineFilter : public Framework::ScanLineFilter {
   public:
      TouchingRegionAdjacencyGraphLineFilter( Graph& graph, std::vector< dfloat >& boundaryLength, UnsignedArray const& sizes, IntegerArray const& strides )
            : graph_( graph ), boundaryLength_( boundaryLength ), sizes_( sizes ), strides_( strides ) {}
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // We iterate from 0 to N-1.
         // We look in directions in which we're not the last line.
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength - 1;
         dip::uint dim = params.dimension;
         dip::uint nDims = sizes_.size();
         DIP_ASSERT( params.position.size() == nDims );
         DIP_ASSERT( strides_[ dim ] == stride );
         BooleanArray process( nDims, true );
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            process[ jj ] = params.position[ jj ] < ( sizes_[ jj ] - 1 );
         }
         // Add to graph_ links to each of the *forward* neighbors (i.e. those that you can reach by incrementing
         // one of the coordinates). The other neighbors are already linked to when those neighbors were processed
         for( dip::uint ii = 0; ii < length; ++ii, in += stride ) {
            DoPixel( in, nDims, process );
         }
         // Do the same for the last pixel on the line
         process[ dim ] = false;
         DoPixel( in, nDims, process );
      }
   private:
      Graph& graph_;
      std::vector< dfloat >& boundaryLength_;
      UnsignedArray const& sizes_;
      IntegerArray const& strides_;

      void DoPixel( TPI const* in, dip::uint nDims, BooleanArray const& process ) {
         dip::uint label = static_cast< dip::uint >( in[ 0 ] );
         if( label == 0 ) { return; }
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            if( process[ jj ] ) {
               dip::uint neighborLabel = static_cast< dip::uint >( in[ strides_[ jj ]] );
               if(( neighborLabel != 0 ) && ( neighborLabel != label )) {
                  graph_.AddEdgeSumWeight( label, neighborLabel, 1 ); // Add 1 to the weight, this is the count of boundary pixels
                  boundaryLength_[ label ] += 1;
                  boundaryLength_[ neighborLabel ] += 1;
               }
            }
         }
      }
};

template< typename TPI >
class WatershedRegionAdjacencyGraphLineFilter : public Framework::ScanLineFilter {
   public:
      WatershedRegionAdjacencyGraphLineFilter( Graph& graph, std::vector< dfloat >& boundaryLength, UnsignedArray const& sizes, IntegerArray const& strides )
            : graph_( graph ), boundaryLength_( boundaryLength ), sizes_( sizes ), strides_( strides ) {}
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         // We iterate from 1 to N-1.
         // We look in directions in which we're not the first or last line.
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint stride = params.inBuffer[ 0 ].stride;
         dip::uint length = params.bufferLength - 1;
         dip::uint dim = params.dimension;
         dip::uint nDims = sizes_.size();
         DIP_ASSERT( params.position.size() == nDims );
         DIP_ASSERT( strides_[ dim ] == stride );
         BooleanArray process( nDims, true );
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            process[ jj ] = ( params.position[ jj ] > 0 ) && ( params.position[ jj ] < ( sizes_[ jj ] - 1 ));
         }
         // Here we add to graph_ links from a label to the left to one on the right of the current pixel.
         // But only if the current pixel is a background pixel.
         // First for the first pixel on the line
         process[ dim ] = false;
         DoPixel( in, nDims, process );
         in += stride;
         // Now for the bulk of the line
         process[ dim ] = true;
         for( dip::uint ii = 1; ii < length; ++ii, in += stride ) {
            DoPixel( in, nDims, process );
         }
         // And finally for the last pixel of the line
         process[ dim ] = false;
         DoPixel( in, nDims, process );
      }
   private:
      Graph& graph_;
      std::vector< dfloat >& boundaryLength_;
      UnsignedArray const& sizes_;
      IntegerArray const& strides_;

      void DoPixel( TPI const* in, dip::uint nDims, BooleanArray const& process ) {
         if( in[ 0 ] == 0 ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               if( process[ jj ] ) {
                  dip::uint label1 = static_cast< dip::uint >( in[ -strides_[ jj ]] );
                  dip::uint label2 = static_cast< dip::uint >( in[ strides_[ jj ]] );
                  if(( label1 > 0 ) && ( label2 > 0 ) && ( label1 != label2 )) {
                     graph_.AddEdgeSumWeight( label1, label2, 1 ); // Add 1 to the weight, this is the count of boundary pixels
                     boundaryLength_[ label1 ] += 1;
                     boundaryLength_[ label2 ] += 1;
                  }
               }
            }
         }
      }
};

Graph RegionAdjacencyGraphInternal( Image const& label, String const& mode, std::vector< dfloat >& boundaryLength ) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( label.Dimensionality() < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   bool touching{};
   DIP_STACK_TRACE_THIS( touching = BooleanFromString( mode, "touching", "watershed" ));
   dip::uint nVertices = dip::Maximum( label ).As< dip::uint >() + 1;
   Graph graph( nVertices );
   boundaryLength.resize( nVertices, 0 );
   std::unique_ptr< Framework::ScanLineFilter > lineFilter;
   if( touching ) {
      DIP_OVL_NEW_UINT( lineFilter, TouchingRegionAdjacencyGraphLineFilter, ( graph, boundaryLength, label.Sizes(), label.Strides() ), label.DataType() );
   } else {
      DIP_OVL_NEW_UINT( lineFilter, WatershedRegionAdjacencyGraphLineFilter, ( graph, boundaryLength, label.Sizes(), label.Strides() ), label.DataType() );
   }
   DIP_STACK_TRACE_THIS( Framework::ScanSingleInput( label, {}, label.DataType(), *lineFilter,
                                                     Framework::ScanOption::NoMultiThreading + Framework::ScanOption::NeedCoordinates ));
   return graph;
}

} // namespace

Graph RegionAdjacencyGraph( Image const& label, String const& mode ) {
   Graph graph;
   std::vector< dfloat > boundaryLength;
   DIP_STACK_TRACE_THIS( graph = RegionAdjacencyGraphInternal( label, mode, boundaryLength ));
   for( auto& edge: graph.Edges() ) {
      if( edge.IsValid() ) {
         edge.weight = 1.0 - std::max(
               edge.weight / boundaryLength[ edge.vertices[ 0 ]],
               edge.weight / boundaryLength[ edge.vertices[ 1 ]] );
      }
   }
   return graph;
}

Graph RegionAdjacencyGraph( Image const& label, Measurement::IteratorFeature const& featureValues, String const& mode ) {
   Graph graph;
   std::vector< dfloat > ignore;
   DIP_STACK_TRACE_THIS( graph = RegionAdjacencyGraphInternal( label, mode, ignore ));
   auto it = featureValues.FirstObject();
   do {
      graph.VertexValue( it.ObjectID() ) = *it;
   } while ( ++it );
   graph.UpdateEdgeWeights();
   return graph;
}

void Relabel( Image const& label, Image& out, Graph const& graph ) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   LabelMap lut = Label( graph );
   lut.Apply( label, out );
}

void Relabel( Image const& label, Image& out, DirectedGraph const& graph ) {
   DIP_THROW_IF( !label.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   LabelMap lut = Label( graph );
   lut.Apply( label, out );
}

} // namespace dip
