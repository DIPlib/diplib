/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022, Cris Luengo.
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
#include "diplib/transform.h"
#include "diplib/distance.h"
#include "diplib/detection.h"
#include "diplib/microscopy.h"

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
   distr.def( "MaximumLikelihood", &dip::Distribution::MaximumLikelihood );

   // diplib/analysis.h
   auto loc = py::class_< dip::SubpixelLocationResult >( m, "SubpixelLocationResult", "The result of a call to dip.SubpixelLocation." );
   loc.def_readonly( "coordinates", &dip::SubpixelLocationResult::coordinates );
   loc.def_readonly( "value", &dip::SubpixelLocationResult::value );
   loc.def( "__repr__", []( dip::SubpixelLocationResult const& self ) {
               std::ostringstream os;
               os << "<SubpixelLocationResult: coordinates=" << self.coordinates << ", value=" << self.value << '>';
               return os.str();
            } );

   m.def( "Find", &dip::Find,
          "in"_a, "mask"_a = dip::Image{} );
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
   m.def( "GaussianMixtureModel", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringSet const& >( &dip::GaussianMixtureModel ),
          "in"_a, "dimension"_a = 2, "numberOfGaussians"_a = 2, "maxIter"_a = 20, "flags"_a = dip::StringSet{} );
   m.def( "CrossCorrelationFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const&, dip::String const& >( &dip::CrossCorrelationFT ),
          "in1"_a, "in2"_a, "in1Representation"_a = dip::S::SPATIAL, "in2Representation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "normalize"_a = dip::S::NORMALIZE );
   m.def( "AutoCorrelationFT", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::AutoCorrelationFT ),
          "in"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL );
   m.def( "FindShift", &dip::FindShift,
          "in1"_a, "in2"_a, "method"_a = "MTS", "parameter"_a = 0, "maxShift"_a = dip::UnsignedArray{ std::numeric_limits< dip::uint >::max() } );
   m.def( "FourierMellinMatch2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const& >( &dip::FourierMellinMatch2D ),
          "in1"_a, "in2"_a, "interpolationMethod"_a = dip::S::LINEAR, "correlationMethod"_a = dip::S::PHASE,
          "Returns only the transformed image, to also obtain the transformation matrix,\n"
          "see `FourierMellinMatch2Dparams()`." );
   m.def( "FourierMellinMatch2Dparams", []( dip::Image const& in1, dip::Image const& in2, dip::String const& interpolationMethod, dip::String const& correlationMethod){
             dip::Image out;
             auto params = FourierMellinMatch2D(in1, in2, out, interpolationMethod, correlationMethod);
             return py::make_tuple( out, params ).release();
          },
          "in1"_a, "in2"_a, "interpolationMethod"_a = dip::S::LINEAR, "correlationMethod"_a = dip::S::PHASE,
          "Returns a tuple, the first element is the transformed image, the second\n"
          "element is the transformation matrix." );

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
   m.def( "OrientationSpace", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::uint >( &dip::OrientationSpace ),
          "in"_a, "order"_a = 8, "radCenter"_a = 0.1, "radSigma"_a = 0.8, "orientations"_a = 0 );
   m.def( "PairCorrelation", []( dip::Image const& object, dip::Image const& mask, dip::uint probes, dip::uint length, dip::String const& sampling, dip::StringSet const& options ){
             return dip::PairCorrelation( object, mask, RandomNumberGenerator(), probes, length, sampling, options );
          },
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{},
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "ProbabilisticPairCorrelation", []( dip::Image const& object, dip::Image const& mask, dip::uint probes, dip::uint length, dip::String const& sampling, dip::StringSet const& options ) {
             return dip::ProbabilisticPairCorrelation( object, mask, RandomNumberGenerator(), probes, length, sampling, options );
          },
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{},
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "Semivariogram", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::Semivariogram ),
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM );
   m.def( "ChordLength", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::ChordLength ),
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM );
   m.def( "DistanceDistribution", &dip::DistanceDistribution,
          "object"_a, "region"_a, "length"_a = 100 );
   m.def( "Granulometry", &dip::Granulometry,
          "in"_a, "mask"_a = dip::Image{}, "scales"_a = std::vector< dip::dfloat >{}, "type"_a = "isotropic", "polarity"_a = dip::S::OPENING, "options"_a = dip::StringSet{} );
   m.def( "FractalDimension", &dip::FractalDimension, "in"_a, "eta"_a = 0.5 );

   // diplib/transform.h
   m.def( "FourierTransform", py::overload_cast< dip::Image const&, dip::StringSet const&, dip::BooleanArray const& >( &dip::FourierTransform ),
          "in"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{} );
   m.def( "OptimalFourierTransformSize", &dip::OptimalFourierTransformSize, "size"_a, "which"_a = "larger" );
   m.def( "RieszTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const&, dip::BooleanArray const& >( &dip::RieszTransform ),
          "in"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "process"_a = dip::BooleanArray{} );
   m.def( "StationaryWaveletTransform", py::overload_cast< dip::Image const&, dip::uint, dip::StringArray const&, dip::BooleanArray const& >( &dip::StationaryWaveletTransform ),
          "in"_a, "nLevels"_a = 4, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{} );

   // diplib/distance.h
   m.def( "EuclideanDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::SEPARABLE );
   m.def( "VectorDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::VectorDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST );
   m.def( "GreyWeightedDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Metric const&, dip::String const& >( &dip::GreyWeightedDistanceTransform ),
          "grey"_a, "bin"_a, "mask"_a = dip::Image{}, "metric"_a = dip::Metric{}, "mode"_a = dip::S::FASTMARCHING );
   m.def( "GeodesicDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::GeodesicDistanceTransform ),
          "marker"_a, "condition"_a );

   // diplib/detection.h
   auto rcp = py::class_< dip::RadonCircleParameters >( m, "RadonCircleParameters", "The result of a call to dip.SubpixelLocation." );
   rcp.def_readonly( "origin", &dip::RadonCircleParameters::origin );
   rcp.def_readonly( "radius", &dip::RadonCircleParameters::radius );
   rcp.def( "__repr__", []( dip::RadonCircleParameters const& self ) {
               std::ostringstream os;
               os << "<RadonCircleParameters: origin=" << self.origin << ", radius=" << self.radius << '>';
               return os.str();
            } );

   m.def( "HoughTransformCircleCenters", py::overload_cast< dip::Image const&, dip::Image const&, dip::UnsignedArray const& >( &dip::HoughTransformCircleCenters ),
          "in"_a, "gv"_a, "range"_a = dip::UnsignedArray{} );
   m.def( "FindHoughMaxima", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::FindHoughMaxima ),
          "in"_a, "distance"_a = 10.0, "fraction"_a = 0.1 );
   m.def( "PointDistanceDistribution", py::overload_cast< dip::Image const&, dip::CoordinateArray const&, dip::UnsignedArray >( &dip::PointDistanceDistribution ),
          "in"_a, "points"_a, "range"_a = dip::UnsignedArray{} );
   m.def( "FindHoughCircles", py::overload_cast< dip::Image const&, dip::Image const&, dip::UnsignedArray const&, dip::dfloat, dip::dfloat >( &dip::FindHoughCircles ),
          "in"_a, "gv"_a, "range"_a = dip::UnsignedArray{}, "distance"_a = 10.0, "fraction"_a = 0.1 );
          
   m.def( "RadonTransformCircles", []( dip::Image const& in, dip::Range radii, dip::dfloat sigma, dip::dfloat threshold, dip::String const& mode, dip::StringSet const& options ) {
             dip::Image out;
             dip::RadonCircleParametersArray params = dip::RadonTransformCircles( in, out, radii, sigma, threshold, mode, options );
             return py::make_tuple( out, params ).release();
          }, "in"_a, "radii"_a = dip::Range{ 10, 30 }, "sigma"_a = 1.0, "threshold"_a = 1.0, "mode"_a = dip::S::FULL, "options"_a = dip::StringSet{ dip::S::NORMALIZE, dip::S::CORRECT },
          "Returns a tuple, the first element is the parameter space (the `out` image),\n"
          "the second element is a list of `dip.RadonCircleParameters` containing the\n"
          "parameters of the detected circles." );

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

   // diplib/microscopy.h
   m.def( "BeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::BeerLambertMapping ),
          "in"_a, "background"_a );
   m.def( "InverseBeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::InverseBeerLambertMapping ),
          "in"_a, "background"_a = dip::Image::Pixel{ 255 } );
   m.def( "UnmixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::UnmixStains ),
          "in"_a, "stains"_a );
   m.def( "MixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::MixStains ),
          "in"_a, "stains"_a );

   m.def( "MandersOverlapCoefficient", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MandersOverlapCoefficient ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{} );
   m.def( "IntensityCorrelationQuotient", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::IntensityCorrelationQuotient ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{} );
   m.def( "MandersColocalizationCoefficients", []( dip::Image const& channel1, dip::Image const& channel2, dip::Image const& mask, dip::dfloat threshold1, dip::dfloat threshold2 ){
             auto out = dip::MandersColocalizationCoefficients( channel1, channel2, mask, threshold1, threshold2 );
             return py::make_tuple( out.M1, out.M2 ).release();
          }, "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, "threshold1"_a = 0.0, "threshold2"_a = 0.0,
          "Instead of a `dip::ColocalizationCoefficients` object, returns a tuple with\n"
          "the `M1` and `M2` values." );
   m.def( "CostesColocalizationCoefficients", []( dip::Image const& channel1, dip::Image const& channel2, dip::Image const& mask ){
             auto out = dip::CostesColocalizationCoefficients( channel1, channel2, mask );
             return py::make_tuple( out.M1, out.M2 ).release();
          }, "channel1"_a, "channel2"_a, "mask"_a = dip::Image{},
          "Instead of a `dip::ColocalizationCoefficients` object, returns a tuple with\n"
          "the `M1` and `M2` values." );
   m.def( "CostesSignificanceTest", []( dip::Image const& channel1, dip::Image const& channel2, dip::Image const& mask, dip::UnsignedArray blockSizes, dip::uint repetitions ) {
             return dip::CostesSignificanceTest( channel1, channel2, mask, RandomNumberGenerator(), std::move( blockSizes ), repetitions );
          },
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, "blockSizes"_a = dip::UnsignedArray{ 3 }, "repetitions"_a = 200,
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "IncoherentOTF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::IncoherentOTF ),
          "out"_a, "defocus"_a = 0.0, "oversampling"_a = 1.0, "amplitude"_a = 1.0, "method"_a = "Stokseth" );
   m.def( "IncoherentPSF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat >( &dip::IncoherentPSF ),
          "out"_a, "oversampling"_a = 1.0, "amplitude"_a = 1.0 );
   m.def( "ExponentialFitCorrection", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::dfloat, dip::String const& >( &dip::ExponentialFitCorrection ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = -1.0, "fromWhere"_a = "first plane", "hysteresis"_a = 1.0, "weighting"_a = "none" );
   m.def( "AttenuationCorrection", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::AttenuationCorrection ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "background"_a = 0.0, "threshold"_a = 0.0, "NA"_a = 1.4, "refIndex"_a = 1.518, "method"_a = "DET" );
   m.def( "SimulatedAttenuation", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat >( &dip::SimulatedAttenuation ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "NA"_a = 1.4, "refIndex"_a = 1.518, "oversample"_a = 1, "rayStep"_a = 1 );

}
