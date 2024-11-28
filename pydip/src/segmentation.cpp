/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022-2024, Cris Luengo.
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

#include <sstream>
#include <utility>
#include <vector>

#include "pydip.h"
#include "diplib/segmentation.h"
#include "diplib/graph.h"
#include "diplib/regions.h"
#include "diplib/label_map.h"
#include "diplib/measurement.h"
#include "diplib/neighborlist.h"

void init_segmentation( py::module& m ) {

   // diplib/segmentation.h
   m.def( "KMeansClustering", []( dip::Image const& in, dip::uint nClusters ) {
             return KMeansClustering( in, RandomNumberGenerator(), nClusters );
          },
          "in"_a, "nClusters"_a = 2,
          "Applies k-means clustering to an image, yielding `nClusters` labeled regions.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "KMeansClustering", []( dip::Image const& in, dip::Image& out, dip::uint nClusters ) {
             KMeansClustering( in, out, RandomNumberGenerator(), nClusters );
          },
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2,
          "Applies k-means clustering to an image, yielding `nClusters` labeled regions.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Image const&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, "nClusters"_a = 2, doc_strings::dip·MinimumVariancePartitioning·Image·CL·Image·L·dip·uint· );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2, doc_strings::dip·MinimumVariancePartitioning·Image·CL·Image·L·dip·uint· );
   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1, doc_strings::dip·IsodataThreshold·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "nThresholds"_a = 1, doc_strings::dip·IsodataThreshold·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·OtsuThreshold·Image·CL·Image·CL·Image·L );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, doc_strings::dip·OtsuThreshold·Image·CL·Image·CL·Image·L );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·MinimumErrorThreshold·Image·CL·Image·CL·Image·L );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, doc_strings::dip·MinimumErrorThreshold·Image·CL·Image·CL·Image·L );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1, doc_strings::dip·GaussianMixtureModelThreshold·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "nThresholds"_a = 1, doc_strings::dip·GaussianMixtureModelThreshold·Image·CL·Image·CL·Image·L·dip·uint· );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "sigma"_a = 4.0, doc_strings::dip·TriangleThreshold·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "sigma"_a = 4.0, doc_strings::dip·TriangleThreshold·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "distance"_a = 2.0, "sigma"_a = 4.0, doc_strings::dip·BackgroundThreshold·Image·CL·Image·CL·Image·L·dfloat··dfloat· );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "distance"_a = 2.0, "sigma"_a = 4.0, doc_strings::dip·BackgroundThreshold·Image·CL·Image·CL·Image·L·dfloat··dfloat· );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "volumeFraction"_a = 0.5, doc_strings::dip·VolumeThreshold·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "volumeFraction"_a = 0.5, doc_strings::dip·VolumeThreshold·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY, doc_strings::dip·FixedThreshold·Image·CL·Image·L·dfloat··dfloat··dfloat··String·CL );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY, doc_strings::dip·FixedThreshold·Image·CL·Image·L·dfloat··dfloat··dfloat··String·CL );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::RangeThreshold ),
          "in"_a, "lowerBound"_a, "upperBound"_a, "output"_a = dip::S::BINARY, "foreground"_a = 1.0, "background"_a = 0.0, doc_strings::dip·RangeThreshold·Image·CL·Image·L·dfloat··dfloat··String·CL·dfloat··dfloat· );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::RangeThreshold ),
          "in"_a, py::kw_only(), "out"_a, "lowerBound"_a, "upperBound"_a, "output"_a = dip::S::BINARY, "foreground"_a = 1.0, "background"_a = 0.0, doc_strings::dip·RangeThreshold·Image·CL·Image·L·dfloat··dfloat··String·CL·dfloat··dfloat· );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, "lowThreshold"_a, "highThreshold"_a, doc_strings::dip·HysteresisThreshold·Image·CL·Image·L·dfloat··dfloat· );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, py::kw_only(), "out"_a, "lowThreshold"_a, "highThreshold"_a, doc_strings::dip·HysteresisThreshold·Image·CL·Image·L·dfloat··dfloat· );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, "thresholds"_a, doc_strings::dip·MultipleThresholds·Image·CL·Image·L·FloatArray·CL );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, py::kw_only(), "out"_a, "thresholds"_a, doc_strings::dip·MultipleThresholds·Image·CL·Image·L·FloatArray·CL );
   m.def( "Threshold", []( dip::Image const& in, dip::Image const& mask, dip::String const& method, dip::dfloat parameter ) {
             dip::Image out;
             dip::dfloat threshold = Threshold( in, mask, out, method, parameter );
             return py::make_tuple( out, threshold );
          }, "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity,
          "Automated threshold using `method`.\n"
          "Returns a tuple, the first element is the thresholded image, the second one\n"
          "is the threshold value." );
   m.def( "Threshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::dfloat >( &dip::Threshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity, doc_strings::dip·Threshold·Image·CL·Image·CL·Image·L·String·CL·dfloat· );
   m.def( "PerObjectEllipsoidFit", []( dip::Image const& in,
                                       std::pair< dip::uint, dip::uint > sizeBounds,
                                       dip::dfloat minEllipsoidFit,
                                       std::pair< dip::dfloat, dip::dfloat > aspectRatioBounds,
                                       std::pair< dip::dfloat, dip::dfloat > thresholdBounds ) {
             return dip::PerObjectEllipsoidFit( in, { sizeBounds.first, sizeBounds.second, minEllipsoidFit, aspectRatioBounds.first,
                                                      aspectRatioBounds.second, thresholdBounds.first, thresholdBounds.second } );
          }, "in"_a, "sizeBounds"_a = std::pair< dip::uint, dip::uint >{ 6, 30000 }, "minEllipsoidFit"_a = 0.88,
          "aspectRatioBounds"_a = std::pair< dip::dfloat, dip::dfloat >{ 1.0, 10.0 }, "thresholdBounds"_a = std::pair< dip::dfloat, dip::dfloat >{ 0.0, 255.0 },
          "Finds a per-object threshold such that found objects are maximally\nellipsoidal.\n"
          "Like the C++ function, but with individual input values rather than a single\n"
          "`dip::PerObjectEllipsoidFitParameters` object collecting all algorithm parameters." );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Canny ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1 }, "lower"_a = 0.5, "upper"_a = 0.9, "selection"_a = dip::S::ALL, doc_strings::dip·Canny·Image·CL·Image·L·FloatArray·CL·dfloat··dfloat··String·CL );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Canny ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1 }, "lower"_a = 0.5, "upper"_a = 0.9, "selection"_a = dip::S::ALL, doc_strings::dip·Canny·Image·CL·Image·L·FloatArray·CL·dfloat··dfloat··String·CL );
   m.def( "Superpixels", []( dip::Image const& in, dip::dfloat density, dip::dfloat compactness, dip::String const& method, dip::StringSet const& flags ) {
             return dip::Superpixels( in, RandomNumberGenerator(), density, compactness, method, flags );
          },
          "in"_a, "density"_a = 0.005, "compactness"_a = 1.0, "method"_a = dip::S::CW, "flags"_a = dip::StringSet{},
          "Generates superpixels (oversegmentation)\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "Superpixels", []( dip::Image const& in, dip::Image& out, dip::dfloat density, dip::dfloat compactness, dip::String const& method, dip::StringSet const& flags ) {
             dip::Superpixels( in, out, RandomNumberGenerator(), density, compactness, method, flags );
          },
          "in"_a, py::kw_only(), "out"_a, "density"_a = 0.005, "compactness"_a = 1.0, "method"_a = dip::S::CW, "flags"_a = dip::StringSet{},
          "Generates superpixels (oversegmentation)\n"
          "Like the C++ function, but using an internal `dip::Random` object." );

   // diplib/graph.h
   auto graph = py::class_< dip::Graph >( m, "Graph", doc_strings::dip·Graph );
   graph.def( py::init<>() );
   graph.def( py::init< dip::uint, dip::uint >(), "nVertices"_a, "nEdges"_a = 0, doc_strings::dip·Graph·Graph·dip·uint··dip·uint· );
   graph.def( py::init< dip::Image const&, dip::uint, dip::String const& >(), "image"_a, "connectivity"_a = 1, "weights"_a = "difference", doc_strings::dip·Graph·Graph·Image·CL·dip·uint··String·CL );
   graph.def( "__repr__", []( dip::Graph const& self ) {
      std::ostringstream os;
      os << "<Graph with " << self.NumberOfVertices() << " vertices and " << self.NumberOfEdges() << " edges>";
      return os.str();
   } );
   graph.def( "NumberOfVertices", &dip::Graph::NumberOfVertices, doc_strings::dip·Graph·NumberOfVertices·C );
   graph.def( "NumberOfEdges", &dip::Graph::NumberOfEdges, doc_strings::dip·Graph·NumberOfEdges·C );
   graph.def( "CountEdges", &dip::Graph::CountEdges, doc_strings::dip·Graph·CountEdges·C );
   graph.def( "EdgeVertex", &dip::Graph::EdgeVertex,  "edge"_a, "which"_a, doc_strings::dip·Graph·EdgeVertex·EdgeIndex··bool··C );
   graph.def( "OtherVertex", &dip::Graph::OtherVertex, "edge"_a, "vertex"_a, doc_strings::dip·Graph·OtherVertex·EdgeIndex··VertexIndex··C );
   graph.def( "EdgeWeight", &dip::Graph::EdgeWeight, "edge"_a, doc_strings::dip·Graph·EdgeWeight·EdgeIndex··C );
   graph.def( "IsValidEdge", &dip::Graph::IsValidEdge, "edge"_a, doc_strings::dip·Graph·IsValidEdge·EdgeIndex··C );
   graph.def( "EdgeIndices", &dip::Graph::EdgeIndices, "v"_a, doc_strings::dip·Graph·EdgeIndices·VertexIndex··C );
   graph.def( "VertexValue", &dip::Graph::VertexValue, "v"_a, doc_strings::dip·Graph·VertexValue·VertexIndex··C );
   graph.def( "AddEdge", &dip::Graph::AddEdge, "v1"_a, "v2"_a, "weight"_a, doc_strings::dip·Graph·AddEdge·VertexIndex··VertexIndex··dfloat· );
   graph.def( "AddEdgeSumWeight", &dip::Graph::AddEdgeSumWeight, "v1"_a, "v2"_a, "weight"_a, doc_strings::dip·Graph·AddEdgeSumWeight·VertexIndex··VertexIndex··dfloat· );
   graph.def( "DeleteEdge", py::overload_cast< dip::Graph::VertexIndex, dip::Graph::VertexIndex >( &dip::Graph::DeleteEdge ), "v1"_a, "v2"_a, doc_strings::dip·Graph·DeleteEdge·VertexIndex··VertexIndex· );
   graph.def( "DeleteEdge", py::overload_cast< dip::Graph::EdgeIndex >( &dip::Graph::DeleteEdge ), "edge"_a, doc_strings::dip·Graph·DeleteEdge·EdgeIndex· );
   graph.def( "Neighbors", &dip::Graph::Neighbors, "v"_a, doc_strings::dip·Graph·Neighbors·VertexIndex· );
   // graph.def( "UpdateEdgeWeights", &dip::Graph::UpdateEdgeWeights, doc_strings::dip·Graph·UpdateEdgeWeights·C );
   graph.def( "UpdateEdgeWeights", static_cast< void ( dip::Graph::* )() const >( &dip::Graph::UpdateEdgeWeights ), doc_strings::dip·Graph·UpdateEdgeWeights·C );
   graph.def( "MinimumSpanningForest", &dip::Graph::MinimumSpanningForest, "roots"_a = std::vector< dip::Graph::VertexIndex >{}, doc_strings::dip·Graph·MinimumSpanningForest·std·vectorgtVertexIndexlt·CL·C );
   graph.def( "RemoveLargestEdges", &dip::Graph::RemoveLargestEdges, "number"_a, doc_strings::dip·Graph·RemoveLargestEdges·dip·uint· );

   auto dgraph = py::class_< dip::DirectedGraph >( m, "DirectedGraph", doc_strings::dip·DirectedGraph );
   dgraph.def( py::init<>() );
   dgraph.def( py::init< dip::uint, dip::uint >(), "nVertices"_a, "nEdges"_a = 0, doc_strings::dip·DirectedGraph·DirectedGraph·dip·uint··dip·uint· );
   dgraph.def( py::init< dip::Image const&, dip::uint, dip::String const& >(), "image"_a, "connectivity"_a = 1, "weights"_a = "difference", doc_strings::dip·DirectedGraph·DirectedGraph·Image·CL·dip·uint··String·CL );
   dgraph.def( py::init< dip::Graph const& >(), "graph"_a, doc_strings::dip·DirectedGraph·DirectedGraph·Graph·CL );
   dgraph.def( "__repr__", []( dip::DirectedGraph const& self ) {
      std::ostringstream os;
      os << "<DirectedGraph with " << self.NumberOfVertices() << " vertices and " << self.NumberOfEdges() << " edges>";
      return os.str();
   } );
   dgraph.def( "NumberOfVertices", &dip::DirectedGraph::NumberOfVertices, doc_strings::dip·DirectedGraph·NumberOfVertices·C );
   dgraph.def( "NumberOfEdges", &dip::DirectedGraph::NumberOfEdges, doc_strings::dip·DirectedGraph·NumberOfEdges·C );
   dgraph.def( "CountEdges", &dip::DirectedGraph::CountEdges, doc_strings::dip·DirectedGraph·CountEdges·C );
   dgraph.def( "SourceVertex", &dip::DirectedGraph::SourceVertex,  "edge"_a, doc_strings::dip·DirectedGraph·SourceVertex·EdgeIndex··C );
   dgraph.def( "TargetVertex", &dip::DirectedGraph::TargetVertex, "edge"_a, doc_strings::dip·DirectedGraph·TargetVertex·EdgeIndex··C );
   dgraph.def( "SiblingEdge", &dip::DirectedGraph::SiblingEdge, "edge"_a, doc_strings::dip·DirectedGraph·SiblingEdge·EdgeIndex··C );
   dgraph.def( "EdgeWeight", &dip::DirectedGraph::EdgeWeight, "edge"_a, doc_strings::dip·DirectedGraph·EdgeWeight·EdgeIndex··C );
   dgraph.def( "IsValidEdge", &dip::DirectedGraph::IsValidEdge, "edge"_a, doc_strings::dip·DirectedGraph·IsValidEdge·EdgeIndex··C );
   dgraph.def( "EdgeIndices", &dip::DirectedGraph::EdgeIndices, "v"_a, doc_strings::dip·DirectedGraph·EdgeIndices·VertexIndex··C );
   dgraph.def( "VertexValue", &dip::DirectedGraph::VertexValue, "v"_a, doc_strings::dip·DirectedGraph·VertexValue·VertexIndex··C );
   dgraph.def( "AddEdge", &dip::DirectedGraph::AddEdge, "source"_a, "target"_a, "weight"_a, doc_strings::dip·DirectedGraph·AddEdge·VertexIndex··VertexIndex··dfloat· );
   dgraph.def( "AddEdgeSumWeight", &dip::DirectedGraph::AddEdgeSumWeight, "source"_a, "target"_a, "weight"_a, doc_strings::dip·DirectedGraph·AddEdgeSumWeight·VertexIndex··VertexIndex··dfloat· );
   dgraph.def( "AddEdgePair", &dip::DirectedGraph::AddEdgePair, "v1"_a, "v2"_a, "weight"_a, doc_strings::dip·DirectedGraph·AddEdgePair·VertexIndex··VertexIndex··dfloat· );
   dgraph.def( "AddEdgePairSumWeight", &dip::DirectedGraph::AddEdgePairSumWeight, "v1"_a, "v2"_a, "weight"_a, doc_strings::dip·DirectedGraph·AddEdgePairSumWeight·VertexIndex··VertexIndex··dfloat· );
   dgraph.def( "DeleteEdge", py::overload_cast< dip::DirectedGraph::VertexIndex, dip::DirectedGraph::VertexIndex >( &dip::DirectedGraph::DeleteEdge ), "source"_a, "target"_a, doc_strings::dip·DirectedGraph·DeleteEdge·VertexIndex··VertexIndex· );
   dgraph.def( "DeleteEdgePair", py::overload_cast< dip::DirectedGraph::VertexIndex, dip::DirectedGraph::VertexIndex >( &dip::DirectedGraph::DeleteEdgePair ), "v1"_a, "v2"_a, doc_strings::dip·DirectedGraph·DeleteEdge·VertexIndex··VertexIndex· );
   dgraph.def( "DeleteEdge", py::overload_cast< dip::DirectedGraph::EdgeIndex >( &dip::DirectedGraph::DeleteEdge ), "edge"_a, doc_strings::dip·DirectedGraph·DeleteEdge·EdgeIndex· );
   dgraph.def( "DeleteEdgePair", py::overload_cast< dip::DirectedGraph::EdgeIndex >( &dip::DirectedGraph::DeleteEdgePair ), "edge"_a, doc_strings::dip·DirectedGraph·DeleteEdge·EdgeIndex· );
   dgraph.def( "Neighbors", &dip::DirectedGraph::Neighbors, "v"_a, doc_strings::dip·DirectedGraph·Neighbors·VertexIndex· );
   // dgraph.def( "UpdateEdgeWeights", &dip::DirectedGraph::UpdateEdgeWeights, doc_strings::dip·DirectedGraph·UpdateEdgeWeights·C );
   dgraph.def( "UpdateEdgeWeights", static_cast< void ( dip::DirectedGraph::* )() const >( &dip::DirectedGraph::UpdateEdgeWeights ), doc_strings::dip·DirectedGraph·UpdateEdgeWeights·C );

   m.def( "MinimumSpanningForest", &dip::MinimumSpanningForest, "graph"_a, "roots"_a, doc_strings::dip·MinimumSpanningForest·Graph·CL·std·vectorgtGraph·VertexIndexlt·CL );
   m.def( "GraphCut", &dip::GraphCut, "graph"_a, "sourceIndex"_a, "sinkIndex"_a, doc_strings::dip·GraphCut·DirectedGraph·L·DirectedGraph·VertexIndex··DirectedGraph·VertexIndex· );
   m.def( "Label", py::overload_cast< dip::Graph const& >( &dip::Label ), "graph"_a, doc_strings::dip·Label·Graph·CL );
   m.def( "Label", py::overload_cast< dip::DirectedGraph const& >( &dip::Label ), "graph"_a, doc_strings::dip·Label·DirectedGraph·CL );

   // diplib/regions.h
   m.def( "Label", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringArray, dip::String const& >( &dip::Label ),
          "binary"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::S::ALL, doc_strings::dip·Label·Image·CL·Image·L·dip·uint··dip·uint··dip·uint··StringArray··String·CL );
   m.def( "Label", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::uint, dip::StringArray, dip::String const& >( &dip::Label ),
          "binary"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::S::ALL, doc_strings::dip·Label·Image·CL·Image·L·dip·uint··dip·uint··dip·uint··StringArray··String·CL );
   m.def( "ListObjectLabels", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const& >( &dip::ListObjectLabels ),
          "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE, "region"_a = "", doc_strings::dip·ListObjectLabels·Image·CL·Image·CL·String·CL·String·CL );
   // Deprecated function May 4, 2024 (after release 3.4.3) TODO: remove eventually.
   m.def( "GetObjectLabels", []( dip::Image const& label, dip::Image const& mask, dip::String const& background ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "`GetObjectLabels()` is deprecated, use `ListObjectLabels() instead`.", 1 );
          return dip::ListObjectLabels( label, mask, background );
   }, "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE, doc_strings::dip·ListObjectLabels·Image·CL·Image·CL·String·CL·String·CL );
   m.def( "Relabel", py::overload_cast< dip::Image const& >( &dip::Relabel ),
          "label"_a, doc_strings::dip·Relabel·Image·CL·Image·L );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Relabel ),
          "label"_a, py::kw_only(), "out"_a, doc_strings::dip·Relabel·Image·CL·Image·L );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Graph const& >( &dip::Relabel ),
          "label"_a, "graph"_a, doc_strings::dip·Relabel·Image·CL·Image·L·Graph·CL );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Image&, dip::Graph const& >( &dip::Relabel ),
          "label"_a, py::kw_only(), "out"_a, "graph"_a, doc_strings::dip·Relabel·Image·CL·Image·L·Graph·CL );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::DirectedGraph const& >( &dip::Relabel ),
          "label"_a, "graph"_a, doc_strings::dip·Relabel·Image·CL·Image·L·DirectedGraph·CL );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Image&, dip::DirectedGraph const& >( &dip::Relabel ),
          "label"_a, py::kw_only(), "out"_a, "graph"_a, doc_strings::dip·Relabel·Image·CL·Image·L·DirectedGraph·CL );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, "threshold"_a, "connectivity"_a = 0, doc_strings::dip·SmallObjectsRemove·Image·CL·Image·L·dip·uint··dip·uint· );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a, "connectivity"_a = 0, doc_strings::dip·SmallObjectsRemove·Image·CL·Image·L·dip·uint··dip·uint· );
   m.def( "GrowRegions", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint >( &dip::GrowRegions ),
          "label"_a, "mask"_a = dip::Image{}, "connectivity"_a = -1, "iterations"_a = 0, doc_strings::dip·GrowRegions·Image·CL·Image·CL·Image·L·dip·sint··dip·uint· );
   m.def( "GrowRegions", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::sint, dip::uint >( &dip::GrowRegions ),
          "label"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 0, doc_strings::dip·GrowRegions·Image·CL·Image·CL·Image·L·dip·sint··dip·uint· );
   m.def( "GrowRegionsWeighted", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat >( &dip::GrowRegionsWeighted ),
          "label"_a, "grey"_a, "mask"_a = dip::Image{}, "distance"_a = dip::infinity, doc_strings::dip·GrowRegionsWeighted·Image·CL·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "GrowRegionsWeighted", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::GrowRegionsWeighted ),
          "label"_a, "grey"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "distance"_a = dip::infinity, doc_strings::dip·GrowRegionsWeighted·Image·CL·Image·CL·Image·CL·Image·L·dfloat· );
   // Deprecated functions April 25, 2024 (after release 3.4.3) TODO: remove eventually.
   m.def( "GrowRegionsWeighted", []( dip::Image const& label, dip::Image const& grey, dip::Image const& mask, dip::Metric const& /*metric*/ ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "The `metric` argument is deprecated, and will be ignored.", 1 );
          return dip::GrowRegionsWeighted( label, grey, mask );
   }, "label"_a, "grey"_a, "mask"_a = dip::Image{}, "metric"_a, doc_strings::dip·GrowRegionsWeighted·Image·CL·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "GrowRegionsWeighted", []( dip::Image const& label, dip::Image const& grey, dip::Image const& mask, dip::Image& out, dip::Metric const& /*metric*/ ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "The `metric` argument is deprecated, and will be ignored.", 1 );
          dip::GrowRegionsWeighted( label, grey, mask, out );
   }, "label"_a, "grey"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "metric"_a, doc_strings::dip·GrowRegionsWeighted·Image·CL·Image·CL·Image·CL·Image·L·dfloat· );
   m.def( "SplitRegions", py::overload_cast< dip::Image const&, dip::uint >( &dip::SplitRegions ),
          "label"_a, "connectivity"_a = 0, doc_strings::dip·SplitRegions·Image·CL·Image·L·dip·uint· );
   m.def( "SplitRegions", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::SplitRegions ),
          "label"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, doc_strings::dip·SplitRegions·Image·CL·Image·L·dip·uint· );
   m.def( "MakeRegionsConvex2D", py::overload_cast< dip::Image const&, dip::String const& >( &dip::MakeRegionsConvex2D ),
          "label"_a, "mode"_a = dip::S::FILLED, doc_strings::dip·MakeRegionsConvex2D·Image·CL·Image·L·String·CL );
   m.def( "MakeRegionsConvex2D", py::overload_cast< dip::Image const&, dip::Image&, dip::String const& >( &dip::MakeRegionsConvex2D ),
          "label"_a, py::kw_only(), "out"_a, "mode"_a = dip::S::FILLED, doc_strings::dip·MakeRegionsConvex2D·Image·CL·Image·L·String·CL );
   m.def( "GetLabelBoundingBox", py::overload_cast< dip::Image const&, dip::LabelType >( &dip::GetLabelBoundingBox ),
          "label"_a, "objectID"_a, doc_strings::dip·GetLabelBoundingBox·Image·CL·LabelType· );
   m.def( "RegionAdjacencyGraph", py::overload_cast< dip::Image const&, dip::String const& >( &dip::RegionAdjacencyGraph ),
          "label"_a, "mode"_a = "touching", doc_strings::dip·RegionAdjacencyGraph·Image·CL·String·CL );
   m.def( "RegionAdjacencyGraph", py::overload_cast< dip::Image const&, dip::Measurement::IteratorFeature const&, dip::String const& >( &dip::RegionAdjacencyGraph ),
          "label"_a, "featureValues"_a, "mode"_a = "touching", doc_strings::dip·RegionAdjacencyGraph·Image·CL·Measurement·IteratorFeature·CL·String·CL );

   // diplib/label_map.h
   auto lmap = py::class_< dip::LabelMap >( m, "LabelMap", doc_strings::dip·LabelMap );
   lmap.def( py::init< dip::LabelType >(), "maxLabel"_a, doc_strings::dip·LabelMap·LabelMap·LabelType· );
   lmap.def( py::init< std::vector< dip::LabelType > const& >(), "labels"_a, doc_strings::dip·LabelMap·LabelMap·T·std·vectorgtUnsignedIntegerTypelt·CL );
   lmap.def( "__repr__", []( dip::LabelMap const& self ) {
      std::ostringstream os;
      os << "<LabelMap with " << self.Size() << " labels>";
      return os.str();
   } );
   lmap.def( "__len__", []( dip::LabelMap const& self ) { return self.Size(); } );
   lmap.def( "Size", &dip::LabelMap::Size, doc_strings::dip·LabelMap·Size·C );
   lmap.def( "DestroyUnknownLabels", &dip::LabelMap::DestroyUnknownLabels, doc_strings::dip·LabelMap·DestroyUnknownLabels );
   lmap.def( "PreserveUnknownLabels", &dip::LabelMap::PreserveUnknownLabels, doc_strings::dip·LabelMap·PreserveUnknownLabels );
   lmap.def( "Apply", py::overload_cast< dip::Image const& >( &dip::LabelMap::Apply, py::const_ ), "in"_a, doc_strings::dip·LabelMap·Apply·Image·CL·Image·L·C );
   lmap.def( "Apply", py::overload_cast< dip::Image const&, dip::Image& >( &dip::LabelMap::Apply, py::const_ ),
             "in"_a, py::kw_only(), "out"_a, doc_strings::dip·LabelMap·Apply·Image·CL·Image·L·C );
   lmap.def( "Apply", py::overload_cast< dip::Measurement const& >( &dip::LabelMap::Apply, py::const_ ), "in"_a, doc_strings::dip·LabelMap·Apply·Measurement·CL·C );
   lmap.def( "Negate", &dip::LabelMap::Negate, doc_strings::dip·LabelMap·Negate );
   lmap.def( "Relabel", &dip::LabelMap::Relabel, doc_strings::dip·LabelMap·Relabel );
   lmap.def( "Contains", &dip::LabelMap::Contains, doc_strings::dip·LabelMap·Contains·LabelType··C );
   lmap.def( "Count", &dip::LabelMap::Count, doc_strings::dip·LabelMap·Count·C );
   lmap.def( "__getitem__", []( dip::LabelMap const& self, dip::LabelType label ) { return self[ label ]; }, doc_strings::dip·LabelMap·operatorsqbra·LabelType··C );
   lmap.def( "__setitem__", []( dip::LabelMap& self, dip::LabelType label, dip::LabelType target ) { self[ label ] = target; } );
   lmap.def( "__iand__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self &= rhs; return self; }, py::is_operator(), doc_strings::dip·LabelMap·operatorandeq·LabelMap·CL );
   lmap.def( "__and__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs & rhs; }, py::is_operator(), doc_strings::dip·operatorand·LabelMap··LabelMap·CL );
   lmap.def( "__ior__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self |= rhs; return self; }, py::is_operator(), doc_strings::dip·LabelMap·operatororeq·LabelMap·CL );
   lmap.def( "__or__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs | rhs; }, py::is_operator(), doc_strings::dip·operatoror·LabelMap··LabelMap·CL );
   lmap.def( "__ixor__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self ^= rhs; return self; }, py::is_operator(), doc_strings::dip·LabelMap·operatorxoreq·LabelMap·CL );
   lmap.def( "__xor__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs ^ rhs; }, py::is_operator(), doc_strings::dip·operatorxor·LabelMap··LabelMap·CL );
   lmap.def( "__invert__", []( dip::LabelMap const& self ) { return ~self; }, py::is_operator(), doc_strings::dip·operatorneg·LabelMap· );
}
