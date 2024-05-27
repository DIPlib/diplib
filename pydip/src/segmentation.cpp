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
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "KMeansClustering", []( dip::Image const& in, dip::Image& out, dip::uint nClusters ) {
             KMeansClustering( in, out, RandomNumberGenerator(), nClusters );
          },
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Image const&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, "nClusters"_a = 2 );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2 );
   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1 );
   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "nThresholds"_a = 1 );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1 );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "nThresholds"_a = 1 );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "sigma"_a = 4.0 );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "sigma"_a = 4.0 );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "distance"_a = 2.0, "sigma"_a = 4.0 );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "distance"_a = 2.0, "sigma"_a = 4.0 );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "volumeFraction"_a = 0.5 );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "volumeFraction"_a = 0.5 );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::RangeThreshold ),
          "in"_a, "lowerBound"_a, "upperBound"_a, "output"_a = dip::S::BINARY, "foreground"_a = 1.0, "background"_a = 0.0 );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::RangeThreshold ),
          "in"_a, py::kw_only(), "out"_a, "lowerBound"_a, "upperBound"_a, "output"_a = dip::S::BINARY, "foreground"_a = 1.0, "background"_a = 0.0 );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, "lowThreshold"_a, "highThreshold"_a );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, py::kw_only(), "out"_a, "lowThreshold"_a, "highThreshold"_a );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, "thresholds"_a );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, py::kw_only(), "out"_a, "thresholds"_a );
   m.def( "Threshold", []( dip::Image const& in, dip::Image const& mask, dip::String const& method, dip::dfloat parameter ) {
             dip::Image out;
             dip::dfloat threshold = Threshold( in, mask, out, method, parameter );
             return py::make_tuple( out, threshold );
          }, "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity,
          "Returns a tuple, the first element is the thresholded image, the second one\n"
          "is the threshold value." );
   m.def( "Threshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::dfloat >( &dip::Threshold ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity );
   m.def( "PerObjectEllipsoidFit", []( dip::Image const& in,
                                       std::pair< dip::uint, dip::uint > sizeBounds,
                                       dip::dfloat minEllipsoidFit,
                                       std::pair< dip::dfloat, dip::dfloat > aspectRatioBounds,
                                       std::pair< dip::dfloat, dip::dfloat > thresholdBounds ) {
             return dip::PerObjectEllipsoidFit( in, { sizeBounds.first, sizeBounds.second, minEllipsoidFit, aspectRatioBounds.first,
                                                      aspectRatioBounds.second, thresholdBounds.first, thresholdBounds.second } );
          }, "in"_a, "sizeBounds"_a = std::pair< dip::uint, dip::uint >{ 6, 30000 }, "minEllipsoidFit"_a = 0.88,
          "aspectRatioBounds"_a = std::pair< dip::dfloat, dip::dfloat >{ 1.0, 10.0 }, "thresholdBounds"_a = std::pair< dip::dfloat, dip::dfloat >{ 0.0, 255.0 },
          "Like the C++ function, but with individual input values rather than a single\n"
          "`dip::PerObjectEllipsoidFitParameters` object collecting all algorithm parameters." );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Canny ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1 }, "lower"_a = 0.5, "upper"_a = 0.9, "selection"_a = dip::S::ALL );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Canny ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 1 }, "lower"_a = 0.5, "upper"_a = 0.9, "selection"_a = dip::S::ALL );
   m.def( "Superpixels", []( dip::Image const& in, dip::dfloat density, dip::dfloat compactness, dip::String const& method, dip::StringSet const& flags ) {
             return dip::Superpixels( in, RandomNumberGenerator(), density, compactness, method, flags );
          },
          "in"_a, "density"_a = 0.005, "compactness"_a = 1.0, "method"_a = dip::S::CW, "flags"_a = dip::StringSet{},
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "Superpixels", []( dip::Image const& in, dip::Image& out, dip::dfloat density, dip::dfloat compactness, dip::String const& method, dip::StringSet const& flags ) {
             dip::Superpixels( in, out, RandomNumberGenerator(), density, compactness, method, flags );
          },
          "in"_a, py::kw_only(), "out"_a, "density"_a = 0.005, "compactness"_a = 1.0, "method"_a = dip::S::CW, "flags"_a = dip::StringSet{},
          "Like the C++ function, but using an internal `dip::Random` object." );

   // diplib/graph.h
   auto graph = py::class_< dip::Graph >( m, "Graph", "A non-directed, edge-weighted graph." );
   graph.def( py::init<>() );
   graph.def( py::init< dip::uint, dip::uint >(), "nVertices"_a, "nEdges"_a = 0 );
   graph.def( py::init< dip::Image const&, dip::uint, dip::String const& >(), "image"_a, "connectivity"_a = 1, "weights"_a = "difference" );
   graph.def( "__repr__", []( dip::Graph const& self ) {
      std::ostringstream os;
      os << "<Graph with " << self.NumberOfVertices() << " vertices and " << self.NumberOfEdges() << " edges>";
      return os.str();
   } );
   graph.def( "NumberOfVertices", &dip::Graph::NumberOfVertices );
   graph.def( "NumberOfEdges", &dip::Graph::NumberOfEdges );
   graph.def( "CountEdges", &dip::Graph::CountEdges );
   graph.def( "OtherVertex", &dip::Graph::OtherVertex, "edge"_a, "vertex"_a );
   graph.def( "EdgeWeight", &dip::Graph::EdgeWeight, "edge"_a );
   graph.def( "EdgeIndices", &dip::Graph::EdgeIndices, "v"_a );
   graph.def( "VertexValue", &dip::Graph::VertexValue, "v"_a );
   graph.def( "AddEdge", &dip::Graph::AddEdge, "v1"_a, "v2"_a, "weight"_a );
   graph.def( "AddEdgeSumWeight", &dip::Graph::AddEdgeSumWeight, "v1"_a, "v2"_a, "weight"_a );
   graph.def( "DeleteEdge", py::overload_cast< dip::Graph::VertexIndex, dip::Graph::VertexIndex >( &dip::Graph::DeleteEdge ), "v1"_a, "v2"_a );
   graph.def( "DeleteEdge", py::overload_cast< dip::Graph::EdgeIndex >( &dip::Graph::DeleteEdge ), "edge"_a );
   graph.def( "Neighbors", &dip::Graph::Neighbors, "v"_a );
   graph.def( "UpdateEdgeWeights", &dip::Graph::UpdateEdgeWeights );
   graph.def( "MinimumSpanningForest", &dip::Graph::MinimumSpanningForest, "roots"_a = std::vector< dip::Graph::VertexIndex >{} );
   graph.def( "RemoveLargestEdges", &dip::Graph::RemoveLargestEdges, "number"_a );

   // diplib/regions.h
   m.def( "Label", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringArray, dip::String const& >( &dip::Label ),
          "binary"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::S::ALL );
   m.def( "Label", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::uint, dip::StringArray, dip::String const& >( &dip::Label ),
          "binary"_a, py::kw_only(), "out"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{}, "mode"_a = dip::S::ALL );
   m.def( "ListObjectLabels", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const& >( &dip::ListObjectLabels ),
          "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE );
   m.def( "GetObjectLabels", []( dip::Image const &label, dip::Image const &mask, dip::String const &background ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "`GetObjectLabels()` is deprecated, use `ListObjectLabels() instead`.", 1 );
          return dip::ListObjectLabels( label, mask, background );
   }, "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE );
   m.def( "Relabel", py::overload_cast< dip::Image const& >( &dip::Relabel ),
          "label"_a );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Relabel ),
          "label"_a, py::kw_only(), "out"_a );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Graph const& >( &dip::Relabel ),
          "label"_a, "graph"_a );
   m.def( "Relabel", py::overload_cast< dip::Image const&, dip::Image&, dip::Graph const& >( &dip::Relabel ),
          "label"_a, py::kw_only(), "out"_a, "graph"_a );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, "threshold"_a, "connectivity"_a = 0 );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a, "connectivity"_a = 0 );
   m.def( "GrowRegions", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint >( &dip::GrowRegions ),
          "label"_a, "mask"_a = dip::Image{}, "connectivity"_a = -1, "iterations"_a = 0 );
   m.def( "GrowRegions", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::sint, dip::uint >( &dip::GrowRegions ),
          "label"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "connectivity"_a = -1, "iterations"_a = 0 );
   m.def( "GrowRegionsWeighted", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat >( &dip::GrowRegionsWeighted ),
          "label"_a, "grey"_a, "mask"_a = dip::Image{}, "distance"_a = dip::infinity );
   m.def( "GrowRegionsWeighted", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat >( &dip::GrowRegionsWeighted ),
          "label"_a, "grey"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "distance"_a = dip::infinity );
   m.def( "GrowRegionsWeighted", []( dip::Image const& label, dip::Image const& grey, dip::Image const& mask, dip::Metric const& /*metric*/ ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "The `metric` argument is deprecated, and will be ignored.", 1 );
          return dip::GrowRegionsWeighted( label, grey, mask );
   }, "label"_a, "grey"_a, "mask"_a = dip::Image{}, "metric"_a );
   m.def( "GrowRegionsWeighted", []( dip::Image const& label, dip::Image const& grey, dip::Image const& mask, dip::Image& out, dip::Metric const& /*metric*/ ) {
          PyErr_WarnEx( PyExc_DeprecationWarning, "The `metric` argument is deprecated, and will be ignored.", 1 );
          dip::GrowRegionsWeighted( label, grey, mask, out );
   }, "label"_a, "grey"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "metric"_a );
   m.def( "SplitRegions", py::overload_cast< dip::Image const&, dip::uint >( &dip::SplitRegions ),
          "label"_a, "connectivity"_a = 0 );
   m.def( "SplitRegions", py::overload_cast< dip::Image const&, dip::Image&, dip::uint >( &dip::SplitRegions ),
          "label"_a, py::kw_only(), "out"_a, "connectivity"_a = 0 );
   m.def( "MakeRegionsConvex2D", py::overload_cast< dip::Image const&, dip::String const& >( &dip::MakeRegionsConvex2D ),
          "label"_a, "mode"_a = dip::S::FILLED );
   m.def( "MakeRegionsConvex2D", py::overload_cast< dip::Image const&, dip::Image&, dip::String const& >( &dip::MakeRegionsConvex2D ),
          "label"_a, py::kw_only(), "out"_a, "mode"_a = dip::S::FILLED );
   m.def( "GetLabelBoundingBox", py::overload_cast< dip::Image const&, dip::LabelType >( &dip::GetLabelBoundingBox ),
          "label"_a, "objectID"_a );
   m.def( "RegionAdjacencyGraph", py::overload_cast< dip::Image const&, dip::String const& >( &dip::RegionAdjacencyGraph ),
          "label"_a, "mode"_a = "touching" );
   m.def( "RegionAdjacencyGraph", py::overload_cast< dip::Image const&, dip::Measurement::IteratorFeature const&, dip::String const& >( &dip::RegionAdjacencyGraph ),
          "label"_a, "featureValues"_a, "mode"_a = "touching" );

   // diplib/label_map.h
   auto lmap = py::class_< dip::LabelMap >( m, "LabelMap", "Represents a set of labels (object IDs), and maps them to new ones." );
   lmap.def( py::init< dip::LabelType >(), "maxLabel"_a );
   lmap.def( py::init< std::vector< dip::LabelType > const& >(), "labels"_a );
   lmap.def( "__repr__", []( dip::LabelMap const& self ) {
      std::ostringstream os;
      os << "<LabelMap with " << self.Size() << " labels>";
      return os.str();
   } );
   lmap.def( "__len__", []( dip::LabelMap const& self ) { return self.Size(); } );
   lmap.def( "Size", &dip::LabelMap::Size );
   lmap.def( "DestroyUnknownLabels", &dip::LabelMap::DestroyUnknownLabels );
   lmap.def( "PreserveUnknownLabels", &dip::LabelMap::PreserveUnknownLabels );
   lmap.def( "Apply", py::overload_cast< dip::Image const& >( &dip::LabelMap::Apply, py::const_ ), "in"_a );
   lmap.def( "Apply", py::overload_cast< dip::Image const&, dip::Image& >( &dip::LabelMap::Apply, py::const_ ),
             "in"_a, py::kw_only(), "out"_a );
   lmap.def( "Apply", py::overload_cast< dip::Measurement const& >( &dip::LabelMap::Apply, py::const_ ), "in"_a );
   lmap.def( "Negate", &dip::LabelMap::Negate );
   lmap.def( "Relabel", &dip::LabelMap::Relabel );
   lmap.def( "Contains", &dip::LabelMap::Contains );
   lmap.def( "Count", &dip::LabelMap::Count );
   lmap.def( "__getitem__", []( dip::LabelMap const& self, dip::LabelType label ) { return self[ label ]; } );
   lmap.def( "__setitem__", []( dip::LabelMap& self, dip::LabelType label, dip::LabelType target ) { self[ label ] = target; } );
   lmap.def( "__iand__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self &= rhs; return self; }, py::is_operator() );
   lmap.def( "__and__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs & rhs; }, py::is_operator() );
   lmap.def( "__ior__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self |= rhs; return self; }, py::is_operator() );
   lmap.def( "__or__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs | rhs; }, py::is_operator() );
   lmap.def( "__ixor__", []( dip::LabelMap& self, dip::LabelMap const& rhs ) { self ^= rhs; return self; }, py::is_operator() );
   lmap.def( "__xor__", []( dip::LabelMap const& lhs, dip::LabelMap const& rhs ) { return lhs ^ rhs; }, py::is_operator() );
   lmap.def( "__invert__", []( dip::LabelMap const& self ) { return ~self; }, py::is_operator() );
}
