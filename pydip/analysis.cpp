/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017-2018, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "pydip.h"
#include "diplib/distribution.h"
#include "diplib/analysis.h"
#include "diplib/detection.h"
#include "diplib/distance.h"
#include "diplib/microscopy.h"
#include "diplib/regions.h"
#include "diplib/segmentation.h"

void init_analysis( py::module& m ) {

   // diplib/distribution.h
   auto distr = py::class_< dip::Distribution >( m, "Distribution", "" );
   distr.def( "__repr__", []( dip::Distribution const& self ) {
      std::ostringstream os;
      os << "<Distribution with " << self.Size() << " samples, and " << self.ValuesPerSample() << " values per sample>";
      return os.str();
   } );
   distr.def( "__str__", []( dip::Distribution const& self ) { std::ostringstream os; os << self; return os.str(); } );
   distr.def( "__getitem__", []( dip::Distribution const& self, dip::uint index ) {
                                    auto const& sample = self[ index ];
                                    return py::make_tuple( sample.X(), sample.Y() ).release();
                             }, "index"_a );
   distr.def( py::self += py::self );
   distr.def( "Empty", &dip::Distribution::Empty );
   distr.def( "Size", &dip::Distribution::Size );
   distr.def( "ValuesPerSample", &dip::Distribution::ValuesPerSample );
   distr.def( "Rows", &dip::Distribution::Rows );
   distr.def( "Columns", &dip::Distribution::Columns );
   distr.def( "XUnits", py::overload_cast<>( &dip::Distribution::XUnits, py::const_ ));
   distr.def( "X", &dip::Distribution::X );
   distr.def( "Y", &dip::Distribution::Y, "index"_a = 0 );
   distr.def( "Cumulative", &dip::Distribution::Cumulative );
   distr.def( "Sum", &dip::Distribution::Sum, "index"_a = 0 );
   distr.def( "Integrate", &dip::Distribution::Integrate );
   distr.def( "Integral", &dip::Distribution::Integral, "index"_a = 0 );
   distr.def( "NormalizeIntegral", &dip::Distribution::NormalizeIntegral );
   distr.def( "Differentiate", &dip::Distribution::Differentiate );

   // diplib/analysis.h

   auto loc = py::class_< dip::SubpixelLocationResult >( m, "SubpixelLocationResult", "The result of a call to PyDIP.SubpixelLocation." );
   loc.def_readonly( "coordinates", &dip::SubpixelLocationResult::coordinates );
   loc.def_readonly( "value", &dip::SubpixelLocationResult::value );
   loc.def( "__repr__", []( dip::SubpixelLocationResult const& self ) {
               std::ostringstream os;
               os << "<SubpixelLocationResult at " << self.coordinates << " with value " << self.value << ">";
               return os.str();
            } );

   m.def( "SubpixelLocation", &dip::SubpixelLocation,
          "in"_a, "position"_a, "polarity"_a = dip::S::MAXIMUM, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "SubpixelMaxima", &dip::SubpixelMaxima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "SubpixelMinima", &dip::SubpixelMinima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE );
   m.def( "MeanShift", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat >( &dip::MeanShift ),
          "meanShiftVectorResult"_a, "start"_a, "epsilon"_a = 1e-3 );
   m.def( "MeanShift", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::dfloat >( &dip::MeanShift ),
          "meanShiftVectorResult"_a, "startArray"_a, "epsilon"_a = 1e-3 );
   m.def( "CrossCorrelationFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const&, dip::String const& >( &dip::CrossCorrelationFT ),
          "in1"_a, "in2"_a, "in1Representation"_a = dip::S::SPATIAL, "in2Representation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "normalize"_a = dip::S::NORMALIZE );
   m.def( "FindShift", &dip::FindShift,
          "in1"_a, "in2"_a, "method"_a = "MTS", "parameter"_a = 0, "maxShift"_a = dip::UnsignedArray{ std::numeric_limits< dip::uint >::max() } );
   m.def( "FourierMellinMatch2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const& >( &dip::FourierMellinMatch2D ),
          "in1"_a, "in2"_a, "interpolationMethod"_a = dip::S::LINEAR );

   m.def( "StructureTensor", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::StructureTensor ),
          "in"_a, "mask"_a = dip::Image{}, "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "tensorSigmas"_a = dip::FloatArray{ 5.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "StructureTensorAnalysis", py::overload_cast< dip::Image const&, dip::StringArray const& >( &dip::StructureTensorAnalysis ),
          "in"_a, "outputs"_a );
   m.def( "StructureAnalysis", &dip::StructureAnalysis,
          "in"_a, "mask"_a = dip::Image{}, "scales"_a = std::vector< dip::dfloat >{}, "feature"_a = "energy",
          "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0 );
   m.def( "MonogenicSignal", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::String const&, dip::String const& >( &dip::MonogenicSignal ),
          "in"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 24.0 }, "bandwidth"_a = 0.41, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL );
   m.def( "MonogenicSignalAnalysis", py::overload_cast< dip::Image const&, dip::StringArray const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::MonogenicSignalAnalysis ),
          "in"_a, "outputs"_a, "noiseThreshold"_a = 0.2, "frequencySpreadThreshold"_a = 0.5, "sigmoidParameter"_a = 10, "deviationGain"_a = 1.5, "polarity"_a = dip::S::BOTH );

   m.def( "PairCorrelation", &dip::PairCorrelation,
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{} );
   m.def( "ProbabilisticPairCorrelation", &dip::ProbabilisticPairCorrelation,
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{} );
   m.def( "Semivariogram", &dip::Semivariogram,
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM );
   m.def( "ChordLength", &dip::ChordLength,
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM );
   m.def( "DistanceDistribution", &dip::DistanceDistribution,
          "object"_a, "region"_a, "length"_a = 100 );
   m.def( "Granulometry", &dip::Granulometry,
          "in"_a, "mask"_a = dip::Image{}, "scales"_a = std::vector< dip::dfloat >{}, "type"_a = "isotropic", "polarity"_a = dip::S::OPENING, "options"_a = dip::StringSet{} );
   m.def( "FractalDimension", &dip::FractalDimension, "in"_a, "eta"_a = 0.5 );

   // diplib/detection.h

   m.def( "HarrisCornerDetector", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::HarrisCornerDetector ),
          "in"_a, "kappa"_a = 0.04, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "ShiTomasiCornerDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::StringArray const& >( &dip::ShiTomasiCornerDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "NobleCornerDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::StringArray const& >( &dip::NobleCornerDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "WangBradyCornerDetector", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::WangBradyCornerDetector ),
          "in"_a, "threshold"_a = 0.1, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{} );

   m.def( "FrangiVesselness", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::FrangiVesselness ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "parameters"_a = dip::FloatArray{}, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "MatchedFiltersLineDetector2D", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::MatchedFiltersLineDetector2D ),
          "in"_a, "sigma"_a = 2.0, "length"_a = 10.0, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "DanielssonLineDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::DanielssonLineDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "RORPOLineDetector", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::RORPOLineDetector ),
          "in"_a, "length"_a = 15, "polarity"_a = dip::S::WHITE );

   // diplib/distance.h

   m.def( "EuclideanDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::SEPARABLE );
   m.def( "VectorDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::VectorDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST );
   m.def( "GreyWeightedDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Metric const&, dip::String const& >( &dip::GreyWeightedDistanceTransform ),
          "grey"_a, "bin"_a, "mask"_a = dip::Image{}, "metric"_a = dip::Metric{ dip::S::CHAMFER, 2 }, "outputMode"_a = "GDT" );

   // diplib/microscopy.h

   m.def( "BeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::BeerLambertMapping ),
          "in"_a, "background"_a );
   m.def( "InverseBeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::InverseBeerLambertMapping ),
          "in"_a, "background"_a = dip::Image::Pixel{ 255 } );
   m.def( "UnmixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::UnmixStains ),
          "in"_a, "stains"_a );
   m.def( "MixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::MixStains ),
          "in"_a, "stains"_a );
   m.def( "IncoherentOTF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::IncoherentOTF ),
          "out"_a, "defocus"_a = 0.0, "oversampling"_a = 1.0, "amplitude"_a = 1.0, "method"_a = "Stokseth" );
   m.def( "IncoherentPSF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat >( &dip::IncoherentPSF ),
          "out"_a, "oversampling"_a = 1.0, "amplitude"_a = 1.0 );
   m.def( "ExponentialFitCorrection", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::dfloat, dip::String const& >( &dip::ExponentialFitCorrection ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = -1.0, "fromWhere"_a = "first plane", "hysteresis"_a = 0.0, "weighting"_a = "none" );
   m.def( "AttenuationCorrection", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::AttenuationCorrection ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "background"_a = 0.0, "threshold"_a = 0.0, "NA"_a = 1.4, "refIndex"_a = 1.518, "method"_a = "DET" );
   m.def( "SimulatedAttenuation", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat >( &dip::SimulatedAttenuation ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "NA"_a = 1.4, "refIndex"_a = 1.518, "oversample"_a = 1, "rayStep"_a = 1 );

   // diplib/regions.h

   m.def( "Label", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringArray const& >( &dip::Label ),
          "binary"_a, "connectivity"_a = 0, "minSize"_a = 0, "maxSize"_a = 0, "boundaryCondition"_a = dip::StringArray{} );
   m.def( "GetObjectLabels", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const& >( &dip::GetObjectLabels ),
          "label"_a, "mask"_a = dip::Image{}, "background"_a = dip::S::EXCLUDE );
   m.def( "Relabel", py::overload_cast< dip::Image const& >( &dip::Relabel ), "label"_a );
   m.def( "SmallObjectsRemove", py::overload_cast< dip::Image const&, dip::uint, dip::uint >( &dip::SmallObjectsRemove ),
          "in"_a, "threshold"_a, "connectivity"_a = 0 );
   m.def( "GrowRegions", py::overload_cast< dip::Image const&, dip::Image const&, dip::sint, dip::uint >( &dip::GrowRegions ),
          "label"_a, "mask"_a = dip::Image{}, "connectivity"_a = -1, "iterations"_a = 0 );
   m.def( "GrowRegionsWeighted", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Metric const& >( &dip::GrowRegionsWeighted ),
          "label"_a, "grey"_a, "mask"_a = dip::Image{}, "metric"_a = dip::Metric{ dip::S::CHAMFER, 2 } );

   // diplib/segmentation.h
   m.def( "KMeansClustering", py::overload_cast< dip::Image const&, dip::uint >( &dip::KMeansClustering ),
          "in"_a, "nClusters"_a = 2 );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Image const&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, "nClusters"_a = 2 );
   m.def( "IsodataThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "nThresholds"_a = 1 );
   m.def( "OtsuThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::OtsuThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MinimumErrorThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "TriangleThreshold", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::TriangleThreshold ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "distance"_a = 2.0 );
   m.def( "VolumeThreshold", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat >( &dip::VolumeThreshold ),
          "in"_a, "mask"_a = dip::Image{}, "volumeFraction"_a = 0.5 );
   m.def( "FixedThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::FixedThreshold ),
          "in"_a, "threshold"_a, "foreground"_a = 1.0, "background"_a = 0.0, "output"_a = dip::S::BINARY );
   m.def( "RangeThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const&, dip::dfloat, dip::dfloat >( &dip::RangeThreshold ),
          "in"_a, "lowerBound"_a, "upperBound"_a, "output"_a = dip::S::BINARY, "foreground"_a = 1.0, "background"_a = 0.0 );
   m.def( "HysteresisThreshold", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::HysteresisThreshold ),
          "in"_a, "lowThreshold"_a, "highThreshold"_a );
   m.def( "MultipleThresholds", py::overload_cast< dip::Image const&, dip::FloatArray const& >( &dip::MultipleThresholds ),
          "in"_a, "thresholds"_a );
   m.def( "Threshold", []( dip::Image const& in, dip::String const& method, dip::dfloat parameter ) {
             dip::Image out;
             dip::dfloat threshold = Threshold( in, out, method, parameter );
             return py::make_tuple( out, threshold ).release();
          }, "in"_a, "method"_a = dip::S::OTSU, "parameter"_a = dip::infinity );
   m.def( "Canny", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::dfloat, dip::String const& >( &dip::Canny ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 1 }, "lower"_a = 0.5, "upper"_a = 0.9, "selection"_a = dip::S::ALL );
}
