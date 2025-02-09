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

#include <limits>
#include <sstream>
#include <vector>
#include <utility>

#include "pydip.h"
#include "diplib/distribution.h"
#include "diplib/analysis.h"
#include "diplib/transform.h"
#include "diplib/distance.h"
#include "diplib/detection.h"
#include "diplib/microscopy.h"
#include "diplib/neighborlist.h"

namespace pybind11 {
namespace detail {

DIP_OUTPUT_TYPE_CASTER( SubpixelLocationResult, "SubpixelLocationResult", "coordinates value", src.coordinates, src.value )
DIP_OUTPUT_TYPE_CASTER( ColocalizationCoefficients, "ColocalizationCoefficients", "M1 M2", src.M1, src.M2 )
DIP_OUTPUT_TYPE_CASTER( RadonCircleParameters, "RadonCircleParameters", "origin radius", src.origin, src.radius )

} // namespace detail
} // namespace pybind11


void init_analysis( py::module& m ) {

   // diplib/distribution.h
   auto distr = py::class_< dip::Distribution >( m, "Distribution", doc_strings::dip·Distribution );
   distr.def(
         "__repr__", []( dip::Distribution const& self ) {
            std::ostringstream os;
            os << "<Distribution with " << self.Size() << " samples, and " << self.ValuesPerSample() << " values per sample>";
            return os.str();
         } );
   distr.def(
         "__str__", []( dip::Distribution const& self ) {
            std::ostringstream os;
            os << self;
            return os.str();
         } );
   distr.def(
         "__getitem__", []( dip::Distribution const& self, dip::uint index ) {
            auto const& sample = self[ index ];
            return py::make_tuple( sample.X(), sample.Y() );
         }, "index"_a );
   distr.def( py::self += py::self );
   distr.def( "Empty", &dip::Distribution::Empty, doc_strings::dip·Distribution·Empty·C );
   distr.def( "Size", &dip::Distribution::Size, doc_strings::dip·Distribution·Size·C );
   distr.def( "ValuesPerSample", &dip::Distribution::ValuesPerSample, doc_strings::dip·Distribution·ValuesPerSample·C );
   distr.def( "Rows", &dip::Distribution::Rows, doc_strings::dip·Distribution·Rows·C );
   distr.def( "Columns", &dip::Distribution::Columns, doc_strings::dip·Distribution·Columns·C );
   distr.def( "XUnits", py::overload_cast<>( &dip::Distribution::XUnits, py::const_), doc_strings::dip·Distribution·XUnits·C );
   distr.def( "X", &dip::Distribution::X, doc_strings::dip·Distribution·X·C );
   distr.def( "Y", &dip::Distribution::Y, "index"_a = 0, doc_strings::dip·Distribution·Y·dip·uint··C );
   distr.def( "Cumulative", &dip::Distribution::Cumulative, doc_strings::dip·Distribution·Cumulative );
   distr.def( "Sum", &dip::Distribution::Sum, "index"_a = 0, doc_strings::dip·Distribution·Sum·dip·uint··C );
   distr.def( "Integrate", &dip::Distribution::Integrate, doc_strings::dip·Distribution·Integrate );
   distr.def( "Integral", &dip::Distribution::Integral, "index"_a = 0, doc_strings::dip·Distribution·Integral·dip·uint··C );
   distr.def( "NormalizeIntegral", &dip::Distribution::NormalizeIntegral, doc_strings::dip·Distribution·NormalizeIntegral );
   distr.def( "Differentiate", &dip::Distribution::Differentiate, doc_strings::dip·Distribution·Differentiate );
   distr.def( "MaximumLikelihood", &dip::Distribution::MaximumLikelihood, doc_strings::dip·Distribution·MaximumLikelihood );

   // diplib/analysis.h
   m.def( "Find", &dip::Find, "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·Find·Image·CL·Image·CL );
   m.def( "SubpixelLocation", &dip::SubpixelLocation,
          "in"_a, "position"_a, "polarity"_a = dip::S::MAXIMUM, "method"_a = dip::S::PARABOLIC_SEPARABLE, doc_strings::dip·SubpixelLocation·Image·CL·UnsignedArray·CL·String·CL·String·CL );
   m.def( "SubpixelMaxima", &dip::SubpixelMaxima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE, doc_strings::dip·SubpixelMaxima·Image·CL·Image·CL·String·CL );
   m.def( "SubpixelMinima", &dip::SubpixelMinima,
          "in"_a, "mask"_a = dip::Image{}, "method"_a = dip::S::PARABOLIC_SEPARABLE, doc_strings::dip·SubpixelMinima·Image·CL·Image·CL·String·CL );
   m.def( "MeanShift", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat >( &dip::MeanShift ),
          "meanShiftVectorResult"_a, "start"_a, "epsilon"_a = 1e-3, doc_strings::dip·MeanShift·Image·CL·FloatArray·CL·dfloat· );
   m.def( "MeanShift", py::overload_cast< dip::Image const&, dip::FloatCoordinateArray const&, dip::dfloat >( &dip::MeanShift ),
          "meanShiftVectorResult"_a, "startArray"_a, "epsilon"_a = 1e-3, doc_strings::dip·MeanShift·Image·CL·FloatCoordinateArray·CL·dfloat· );
   m.def( "GaussianMixtureModel", py::overload_cast< dip::Image const&, dip::uint, dip::uint, dip::uint, dip::StringSet const& >( &dip::GaussianMixtureModel ),
          "in"_a, "dimension"_a = 2, "numberOfGaussians"_a = 2, "maxIter"_a = 20, "flags"_a = dip::StringSet{}, doc_strings::dip·GaussianMixtureModel·Image·CL·Image·L·dip·uint··dip·uint··dip·uint··StringSet·CL );
   m.def( "GaussianMixtureModel", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::uint, dip::uint, dip::StringSet const& >( &dip::GaussianMixtureModel ),
          "in"_a, py::kw_only(), "out"_a, "dimension"_a = 2, "numberOfGaussians"_a = 2, "maxIter"_a = 20, "flags"_a = dip::StringSet{}, doc_strings::dip·GaussianMixtureModel·Image·CL·Image·L·dip·uint··dip·uint··dip·uint··StringSet·CL );
   m.def( "CrossCorrelationFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const&, dip::String const&, dip::String const& >( &dip::CrossCorrelationFT ),
          "in1"_a, "in2"_a, "in1Representation"_a = dip::S::SPATIAL, "in2Representation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "normalize"_a = dip::S::NORMALIZE, doc_strings::dip·CrossCorrelationFT·Image·CL·Image·CL·Image·L·String·CL·String·CL·String·CL·String·CL );
   m.def( "CrossCorrelationFT", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::String const&, dip::String const&, dip::String const& >( &dip::CrossCorrelationFT ),
          "in1"_a, "in2"_a, py::kw_only(), "out"_a, "in1Representation"_a = dip::S::SPATIAL, "in2Representation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "normalize"_a = dip::S::NORMALIZE, doc_strings::dip·CrossCorrelationFT·Image·CL·Image·CL·Image·L·String·CL·String·CL·String·CL·String·CL );
   m.def( "AutoCorrelationFT", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::AutoCorrelationFT ),
          "in"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·AutoCorrelationFT·Image·CL·Image·L·String·CL·String·CL );
   m.def( "AutoCorrelationFT", py::overload_cast< dip::Image const&, dip::Image&, dip::String const&, dip::String const& >( &dip::AutoCorrelationFT ),
          "in"_a, py::kw_only(), "out"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·AutoCorrelationFT·Image·CL·Image·L·String·CL·String·CL );
   m.def( "FindShift", &dip::FindShift,
          "in1"_a, "in2"_a, "method"_a = "MTS", "parameter"_a = 0, "maxShift"_a = dip::UnsignedArray{ std::numeric_limits< dip::uint >::max() }, doc_strings::dip·FindShift·Image·CL·Image·CL·String·CL·dfloat··UnsignedArray· );
   m.def( "FourierMellinMatch2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::String const& >( &dip::FourierMellinMatch2D ),
          "in1"_a, "in2"_a, "interpolationMethod"_a = dip::S::LINEAR, "correlationMethod"_a = dip::S::PHASE,
          "Finds the scaling, translation and rotation between two 2D images using the\n"
          "Fourier Mellin transform. Returns only the transformed image, to also obtain\n"
          "the transformation matrix, see `FourierMellinMatch2Dparams()`." );
   m.def( "FourierMellinMatch2Dparams", []( dip::Image const& in1, dip::Image const& in2, dip::String const& interpolationMethod, dip::String const& correlationMethod ) {
                dip::Image out;
                auto params = FourierMellinMatch2D( in1, in2, out, interpolationMethod, correlationMethod );
                return py::make_tuple( out, params );
          },
          "in1"_a, "in2"_a, "interpolationMethod"_a = dip::S::LINEAR, "correlationMethod"_a = dip::S::PHASE,
          "Finds the scaling, translation and rotation between two 2D images using the\n"
          "Fourier Mellin transform. Returns a tuple, the first element is the\n"
          "transformed image, the second element is the transformation matrix." );
   m.def( "FourierMellinMatch2D", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::String const& >( &dip::FourierMellinMatch2D ),
          "in1"_a, "in2"_a, py::kw_only(), "out"_a, "interpolationMethod"_a = dip::S::LINEAR, "correlationMethod"_a = dip::S::PHASE, doc_strings::dip·FourierMellinMatch2D·Image·CL·Image·CL·Image·L·String·CL·String·CL );

   m.def( "StructureTensor", py::overload_cast< dip::Image const&, dip::Image const&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::StructureTensor ),
          "in"_a, "mask"_a = dip::Image{}, "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "tensorSigmas"_a = dip::FloatArray{ 5.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·StructureTensor·Image·CL·Image·CL·Image·L·FloatArray·CL·FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "StructureTensor", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::FloatArray const&, dip::FloatArray const&, dip::String const&, dip::StringArray const&, dip::dfloat >( &dip::StructureTensor ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "tensorSigmas"_a = dip::FloatArray{ 5.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·StructureTensor·Image·CL·Image·CL·Image·L·FloatArray·CL·FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "StructureTensorAnalysis", py::overload_cast< dip::Image const&, dip::StringArray const& >( &dip::StructureTensorAnalysis ),
          "in"_a, "outputs"_a, doc_strings::dip·StructureTensorAnalysis·Image·CL·ImageRefArray·L·StringArray·CL );
   m.def( "StructureTensorAnalysis", py::overload_cast< dip::Image const&, dip::ImageRefArray&, dip::StringArray const& >( &dip::StructureTensorAnalysis ),
          "in"_a, py::kw_only(), "out"_a, "outputs"_a, doc_strings::dip·StructureTensorAnalysis·Image·CL·ImageRefArray·L·StringArray·CL );
   m.def( "StructureAnalysis", &dip::StructureAnalysis,
          "in"_a, "mask"_a = dip::Image{}, "scales"_a = std::vector< dip::dfloat >{}, "feature"_a = "energy",
          "gradientSigmas"_a = dip::FloatArray{ 1.0 }, "method"_a = dip::S::BEST, "boundaryCondition"_a = dip::StringArray{}, "truncation"_a = 3.0, doc_strings::dip·StructureAnalysis·Image·CL·Image·CL·std·vectorltdfloatgt·CL·String·CL·FloatArray·CL·String·CL·StringArray·CL·dfloat· );
   m.def( "MonogenicSignal", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::dfloat, dip::String const&, dip::String const& >( &dip::MonogenicSignal ),
          "in"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 24.0 }, "bandwidth"_a = 0.41, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·MonogenicSignal·Image·CL·Image·L·FloatArray·CL·dfloat··String·CL·String·CL );
   m.def( "MonogenicSignal", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::dfloat, dip::String const&, dip::String const& >( &dip::MonogenicSignal ),
          "in"_a, py::kw_only(), "out"_a, "wavelengths"_a = dip::FloatArray{ 3.0, 24.0 }, "bandwidth"_a = 0.41, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, doc_strings::dip·MonogenicSignal·Image·CL·Image·L·FloatArray·CL·dfloat··String·CL·String·CL );
   m.def( "MonogenicSignalAnalysis", py::overload_cast< dip::Image const&, dip::StringArray const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::MonogenicSignalAnalysis ),
          "in"_a, "outputs"_a, "noiseThreshold"_a = 0.2, "frequencySpreadThreshold"_a = 0.5, "sigmoidParameter"_a = 10, "deviationGain"_a = 1.5, "polarity"_a = dip::S::BOTH, doc_strings::dip·MonogenicSignalAnalysis·Image·CL·ImageRefArray·L·StringArray·CL·dfloat··dfloat··dfloat··dfloat··String·CL );
   m.def( "MonogenicSignalAnalysis", py::overload_cast< dip::Image const&, dip::ImageRefArray&, dip::StringArray const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::MonogenicSignalAnalysis ),
          "in"_a, py::kw_only(), "out"_a, "outputs"_a, "noiseThreshold"_a = 0.2, "frequencySpreadThreshold"_a = 0.5, "sigmoidParameter"_a = 10, "deviationGain"_a = 1.5, "polarity"_a = dip::S::BOTH, doc_strings::dip·MonogenicSignalAnalysis·Image·CL·ImageRefArray·L·StringArray·CL·dfloat··dfloat··dfloat··dfloat··String·CL );
   m.def( "OrientationSpace", py::overload_cast< dip::Image const&, dip::uint, dip::dfloat, dip::dfloat, dip::uint >( &dip::OrientationSpace ),
          "in"_a, "order"_a = 8, "radCenter"_a = 0.1, "radSigma"_a = 0.8, "orientations"_a = 0, doc_strings::dip·OrientationSpace·Image·CL·Image·L·dip·uint··dfloat··dfloat··dip·uint· );
   m.def( "OrientationSpace", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::dfloat, dip::dfloat, dip::uint >( &dip::OrientationSpace ),
          "in"_a, py::kw_only(), "out"_a, "order"_a = 8, "radCenter"_a = 0.1, "radSigma"_a = 0.8, "orientations"_a = 0, doc_strings::dip·OrientationSpace·Image·CL·Image·L·dip·uint··dfloat··dfloat··dip·uint· );
   m.def( "PairCorrelation", []( dip::Image const& object, dip::Image const& mask, dip::uint probes, dip::uint length, dip::String const& sampling, dip::StringSet const& options ) {
                return dip::PairCorrelation( object, mask, RandomNumberGenerator(), probes, length, sampling, options );
          },
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{}, doc_strings::dip·PairCorrelation·Image·CL·Image·CL·Random·L·dip·uint··dip·uint··String·CL·StringSet·CL );
   m.def( "ProbabilisticPairCorrelation", []( dip::Image const& object, dip::Image const& mask, dip::uint probes, dip::uint length, dip::String const& sampling, dip::StringSet const& options ) {
                return dip::ProbabilisticPairCorrelation( object, mask, RandomNumberGenerator(), probes, length, sampling, options );
          },
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, "options"_a = dip::StringSet{}, doc_strings::dip·ProbabilisticPairCorrelation·Image·CL·Image·CL·Random·L·dip·uint··dip·uint··String·CL·StringSet·CL );
   m.def( "Semivariogram", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::Semivariogram ),
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, doc_strings::dip·Semivariogram·Image·CL·Image·CL·Random·L·dip·uint··dip·uint··String·CL );
   m.def( "ChordLength", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::uint, dip::String const& >( &dip::ChordLength ),
          "object"_a, "mask"_a = dip::Image{}, "probes"_a = 1000000, "length"_a = 100, "sampling"_a = dip::S::RANDOM, doc_strings::dip·ChordLength·Image·CL·Image·CL·Random·L·dip·uint··dip·uint··String·CL );
   m.def( "DistanceDistribution", &dip::DistanceDistribution,
          "object"_a, "region"_a, "length"_a = 100, doc_strings::dip·DistanceDistribution·Image·CL·Image·CL·dip·uint· );
   m.def( "Granulometry", &dip::Granulometry,
          "in"_a, "mask"_a = dip::Image{}, "scales"_a = std::vector< dip::dfloat >{}, "type"_a = "isotropic", "polarity"_a = dip::S::OPENING, "options"_a = dip::StringSet{}, doc_strings::dip·Granulometry·Image·CL·Image·CL·std·vectorltdfloatgt·CL·String·CL·String·CL·StringSet·CL );
   m.def( "FractalDimension", &dip::FractalDimension, "in"_a, "eta"_a = 0.5, doc_strings::dip·FractalDimension·Image·CL·dfloat· );

   // diplib/transform.h
   m.def( "FourierTransform", py::overload_cast< dip::Image const&, dip::StringSet const&, dip::BooleanArray >( &dip::FourierTransform ),
          "in"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·FourierTransform·Image·CL·Image·L·StringSet·CL·BooleanArray· );
   m.def( "FourierTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::StringSet const&, dip::BooleanArray >( &dip::FourierTransform ),
          "in"_a, py::kw_only(), "out"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·FourierTransform·Image·CL·Image·L·StringSet·CL·BooleanArray· );
   m.def( "InverseFourierTransform", py::overload_cast< dip::Image const&, dip::StringSet, dip::BooleanArray >( &dip::InverseFourierTransform ),
          "in"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·InverseFourierTransform·Image·CL·Image·L·StringSet··BooleanArray· );
   m.def( "InverseFourierTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::StringSet, dip::BooleanArray >( &dip::InverseFourierTransform ),
          "in"_a, py::kw_only(), "out"_a, "options"_a = dip::StringSet{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·InverseFourierTransform·Image·CL·Image·L·StringSet··BooleanArray· );
   m.def( "OptimalFourierTransformSize", &dip::OptimalFourierTransformSize, "size"_a, "which"_a = dip::S::LARGER, "purpose"_a = dip::S::REAL, doc_strings::dip·OptimalFourierTransformSize·dip·uint··dip·String·CL·dip·String·CL );
   m.def( "RieszTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const&, dip::BooleanArray >( &dip::RieszTransform ),
          "in"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "process"_a = dip::BooleanArray{}, doc_strings::dip·RieszTransform·Image·CL·Image·L·String·CL·String·CL·BooleanArray· );
   m.def( "RieszTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::String const&, dip::String const&, dip::BooleanArray >( &dip::RieszTransform ),
          "in"_a, py::kw_only(), "out"_a, "inRepresentation"_a = dip::S::SPATIAL, "outRepresentation"_a = dip::S::SPATIAL, "process"_a = dip::BooleanArray{}, doc_strings::dip·RieszTransform·Image·CL·Image·L·String·CL·String·CL·BooleanArray· );
   m.def( "StationaryWaveletTransform", py::overload_cast< dip::Image const&, dip::uint, dip::StringArray const&, dip::BooleanArray const& >( &dip::StationaryWaveletTransform ),
          "in"_a, "nLevels"_a = 4, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·StationaryWaveletTransform·Image·CL·Image·L·dip·uint··StringArray·CL·BooleanArray·CL );
   m.def( "StationaryWaveletTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::StringArray const&, dip::BooleanArray const& >( &dip::StationaryWaveletTransform ),
          "in"_a, py::kw_only(), "out"_a, "nLevels"_a = 4, "boundaryCondition"_a = dip::StringArray{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·StationaryWaveletTransform·Image·CL·Image·L·dip·uint··StringArray·CL·BooleanArray·CL );
   m.def( "HaarWaveletTransform", py::overload_cast< dip::Image const&, dip::uint, dip::String const&, dip::BooleanArray >( &dip::HaarWaveletTransform ),
          "in"_a, "nLevels"_a = 4, "direction"_a = dip::S::FORWARD, "process"_a = dip::BooleanArray{}, doc_strings::dip·HaarWaveletTransform·Image·CL·Image·L·dip·uint··String·CL·BooleanArray· );
   m.def( "HaarWaveletTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const&, dip::BooleanArray >( &dip::HaarWaveletTransform ),
          "in"_a, py::kw_only(), "out"_a, "nLevels"_a = 4, "direction"_a = dip::S::FORWARD, "process"_a = dip::BooleanArray{}, doc_strings::dip·HaarWaveletTransform·Image·CL·Image·L·dip·uint··String·CL·BooleanArray· );

   // diplib/distance.h
   m.def( "EuclideanDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::EuclideanDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::SEPARABLE, doc_strings::dip·EuclideanDistanceTransform·Image·CL·Image·L·String·CL·String·CL );
   m.def( "EuclideanDistanceTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::String const&, dip::String const& >( &dip::EuclideanDistanceTransform ),
          "in"_a, py::kw_only(), "out"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::SEPARABLE, doc_strings::dip·EuclideanDistanceTransform·Image·CL·Image·L·String·CL·String·CL );
   m.def( "VectorDistanceTransform", py::overload_cast< dip::Image const&, dip::String const&, dip::String const& >( &dip::VectorDistanceTransform ),
          "in"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST, doc_strings::dip·VectorDistanceTransform·Image·CL·Image·L·String·CL·String·CL );
   m.def( "VectorDistanceTransform", py::overload_cast< dip::Image const&, dip::Image&, dip::String const&, dip::String const& >( &dip::VectorDistanceTransform ),
          "in"_a, py::kw_only(), "out"_a, "border"_a = dip::S::BACKGROUND, "method"_a = dip::S::FAST, doc_strings::dip·VectorDistanceTransform·Image·CL·Image·L·String·CL·String·CL );
   m.def( "GreyWeightedDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Metric, dip::String const& >( &dip::GreyWeightedDistanceTransform ),
          "grey"_a, "bin"_a, "mask"_a = dip::Image{}, "metric"_a = dip::Metric{}, "mode"_a = dip::S::FASTMARCHING, doc_strings::dip·GreyWeightedDistanceTransform·Image·CL·Image·CL·Image·CL·Image·L·Metric··String·CL );
   m.def( "GreyWeightedDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::Image&, dip::Metric, dip::String const& >( &dip::GreyWeightedDistanceTransform ),
          "grey"_a, "bin"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "metric"_a = dip::Metric{}, "mode"_a = dip::S::FASTMARCHING, doc_strings::dip·GreyWeightedDistanceTransform·Image·CL·Image·CL·Image·CL·Image·L·Metric··String·CL );
   m.def( "GeodesicDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::GeodesicDistanceTransform ),
          "marker"_a, "condition"_a, doc_strings::dip·GeodesicDistanceTransform·Image·CL·Image·CL·Image·L );
   m.def( "GeodesicDistanceTransform", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image& >( &dip::GeodesicDistanceTransform ),
          "marker"_a, "condition"_a, py::kw_only(), "out"_a, doc_strings::dip·GeodesicDistanceTransform·Image·CL·Image·CL·Image·L );

   // diplib/detection.h
   m.def( "HoughTransformCircleCenters", py::overload_cast< dip::Image const&, dip::Image const&, dip::UnsignedArray const& >( &dip::HoughTransformCircleCenters ),
          "in"_a, "gv"_a, "range"_a = dip::UnsignedArray{}, doc_strings::dip·HoughTransformCircleCenters·Image·CL·Image·CL·Image·L·UnsignedArray·CL );
   m.def( "HoughTransformCircleCenters", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::UnsignedArray const& >( &dip::HoughTransformCircleCenters ),
          "in"_a, "gv"_a, py::kw_only(), "out"_a, "range"_a = dip::UnsignedArray{}, doc_strings::dip·HoughTransformCircleCenters·Image·CL·Image·CL·Image·L·UnsignedArray·CL );
   m.def( "FindHoughMaxima", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat >( &dip::FindHoughMaxima ),
          "in"_a, "distance"_a = 10.0, "fraction"_a = 0.1, doc_strings::dip·FindHoughMaxima·Image·CL·dfloat··dfloat· );
   m.def( "PointDistanceDistribution", py::overload_cast< dip::Image const&, dip::CoordinateArray const&, dip::UnsignedArray >( &dip::PointDistanceDistribution ),
          "in"_a, "points"_a, "range"_a = dip::UnsignedArray{}, doc_strings::dip·PointDistanceDistribution·Image·CL·CoordinateArray·CL·UnsignedArray· );
   m.def( "FindHoughCircles", py::overload_cast< dip::Image const&, dip::Image const&, dip::UnsignedArray const&, dip::dfloat, dip::dfloat >( &dip::FindHoughCircles ),
          "in"_a, "gv"_a, "range"_a = dip::UnsignedArray{}, "distance"_a = 10.0, "fraction"_a = 0.1, doc_strings::dip·FindHoughCircles·Image·CL·Image·CL·UnsignedArray·CL·dfloat··dfloat· );

   m.def( "RadonTransformCircles", []( dip::Image const& in, dip::Range radii, dip::dfloat sigma, dip::dfloat threshold, dip::String const& mode, dip::StringSet const& options ) {
                 dip::Image out;
                 dip::RadonCircleParametersArray params = dip::RadonTransformCircles( in, out, radii, sigma, threshold, mode, options );
                 return py::make_tuple( out, params );
          }, "in"_a, "radii"_a = dip::Range{ 10, 30 }, "sigma"_a = 1.0, "threshold"_a = 1.0, "mode"_a = dip::S::FULL, "options"_a = dip::StringSet{ dip::S::NORMALIZE, dip::S::CORRECT },
          "Detects hyperspheres (circles, spheres) using the generalized Radon transform.\n"
          "Returns a tuple, the first element is the parameter space (the `out` image),\n"
          "the second element is a list of `dip.RadonCircleParameters` containing the\n"
          "parameters of the detected circles." );
   m.def( "RadonTransformCircles", py::overload_cast< dip::Image const&, dip::Image&, dip::Range, dip::dfloat, dip::dfloat, dip::String const&, dip::StringSet const& >( dip::RadonTransformCircles ),
          "in"_a, py::kw_only(), "out"_a, "radii"_a = dip::Range{ 10, 30 }, "sigma"_a = 1.0, "threshold"_a = 1.0, "mode"_a = dip::S::FULL, "options"_a = dip::StringSet{ dip::S::NORMALIZE, dip::S::CORRECT },
          doc_strings::dip·RadonTransformCircles·Image·CL·Image·L·Range··dfloat··dfloat··String·CL·StringSet·CL );

   m.def( "HarrisCornerDetector", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::HarrisCornerDetector ),
          "in"_a, "kappa"_a = 0.04, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HarrisCornerDetector·Image·CL·Image·L·dfloat··FloatArray·CL·StringArray·CL );
   m.def( "HarrisCornerDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::HarrisCornerDetector ),
          "in"_a, py::kw_only(), "out"_a, "kappa"_a = 0.04, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·HarrisCornerDetector·Image·CL·Image·L·dfloat··FloatArray·CL·StringArray·CL );
   m.def( "ShiTomasiCornerDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::StringArray const& >( &dip::ShiTomasiCornerDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ShiTomasiCornerDetector·Image·CL·Image·L·FloatArray·CL·StringArray·CL );
   m.def( "ShiTomasiCornerDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::StringArray const& >( &dip::ShiTomasiCornerDetector ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·ShiTomasiCornerDetector·Image·CL·Image·L·FloatArray·CL·StringArray·CL );
   m.def( "NobleCornerDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::StringArray const& >( &dip::NobleCornerDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·NobleCornerDetector·Image·CL·Image·L·FloatArray·CL·StringArray·CL );
   m.def( "NobleCornerDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::StringArray const& >( &dip::NobleCornerDetector ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·NobleCornerDetector·Image·CL·Image·L·FloatArray·CL·StringArray·CL );
   m.def( "WangBradyCornerDetector", py::overload_cast< dip::Image const&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::WangBradyCornerDetector ),
          "in"_a, "threshold"_a = 0.1, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·WangBradyCornerDetector·Image·CL·Image·L·dfloat··FloatArray·CL·StringArray·CL );
   m.def( "WangBradyCornerDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::FloatArray const&, dip::StringArray const& >( &dip::WangBradyCornerDetector ),
          "in"_a, py::kw_only(), "out"_a, "threshold"_a = 0.1, "sigmas"_a = dip::FloatArray{ 2.0 }, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·WangBradyCornerDetector·Image·CL·Image·L·dfloat··FloatArray·CL·StringArray·CL );

   m.def( "FrangiVesselness", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::FloatArray, dip::String const&, dip::StringArray const& >( &dip::FrangiVesselness ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "parameters"_a = dip::FloatArray{}, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·FrangiVesselness·Image·CL·Image·L·FloatArray·CL·FloatArray··String·CL·StringArray·CL );
   m.def( "FrangiVesselness", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::FloatArray, dip::String const&, dip::StringArray const& >( &dip::FrangiVesselness ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "parameters"_a = dip::FloatArray{}, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·FrangiVesselness·Image·CL·Image·L·FloatArray·CL·FloatArray··String·CL·StringArray·CL );
   m.def( "MatchedFiltersLineDetector2D", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::MatchedFiltersLineDetector2D ),
          "in"_a, "sigma"_a = 2.0, "length"_a = 10.0, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MatchedFiltersLineDetector2D·Image·CL·Image·L·dip·dfloat··dip·dfloat··String·CL·StringArray·CL );
   m.def( "MatchedFiltersLineDetector2D", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::String const&, dip::StringArray const& >( &dip::MatchedFiltersLineDetector2D ),
          "in"_a, py::kw_only(), "out"_a, "sigma"_a = 2.0, "length"_a = 10.0, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·MatchedFiltersLineDetector2D·Image·CL·Image·L·dip·dfloat··dip·dfloat··String·CL·StringArray·CL );
   m.def( "DanielssonLineDetector", py::overload_cast< dip::Image const&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::DanielssonLineDetector ),
          "in"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·DanielssonLineDetector·Image·CL·Image·L·dip·FloatArray·CL·String·CL·StringArray·CL );
   m.def( "DanielssonLineDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::FloatArray const&, dip::String const&, dip::StringArray const& >( &dip::DanielssonLineDetector ),
          "in"_a, py::kw_only(), "out"_a, "sigmas"_a = dip::FloatArray{ 2.0 }, "polarity"_a = dip::S::WHITE, "boundaryCondition"_a = dip::StringArray{}, doc_strings::dip·DanielssonLineDetector·Image·CL·Image·L·dip·FloatArray·CL·String·CL·StringArray·CL );
   m.def( "RORPOLineDetector", py::overload_cast< dip::Image const&, dip::uint, dip::String const& >( &dip::RORPOLineDetector ),
          "in"_a, "length"_a = 15, "polarity"_a = dip::S::WHITE, doc_strings::dip·RORPOLineDetector·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "RORPOLineDetector", py::overload_cast< dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::RORPOLineDetector ),
          "in"_a, py::kw_only(), "out"_a, "length"_a = 15, "polarity"_a = dip::S::WHITE, doc_strings::dip·RORPOLineDetector·Image·CL·Image·L·dip·uint··String·CL );

   // diplib/microscopy.h
   m.def( "BeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::BeerLambertMapping ),
          "in"_a, "background"_a, doc_strings::dip·BeerLambertMapping·Image·CL·Image·L·Image·Pixel·CL );
   m.def( "BeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image&, dip::Image::Pixel const& >( &dip::BeerLambertMapping ),
          "in"_a, py::kw_only(), "out"_a, "background"_a, doc_strings::dip·BeerLambertMapping·Image·CL·Image·L·Image·Pixel·CL );
   m.def( "InverseBeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image::Pixel const& >( &dip::InverseBeerLambertMapping ),
          "in"_a, "background"_a = dip::Image::Pixel{ 255 }, doc_strings::dip·InverseBeerLambertMapping·Image·CL·Image·L·Image·Pixel·CL );
   m.def( "InverseBeerLambertMapping", py::overload_cast< dip::Image const&, dip::Image&, dip::Image::Pixel const& >( &dip::InverseBeerLambertMapping ),
          "in"_a, py::kw_only(), "out"_a, "background"_a = dip::Image::Pixel{ 255 }, doc_strings::dip·InverseBeerLambertMapping·Image·CL·Image·L·Image·Pixel·CL );
   m.def( "UnmixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::UnmixStains ),
          "in"_a, "stains"_a, doc_strings::dip·UnmixStains·Image·CL·Image·L·std·vectorltImage·Pixelgt·CL );
   m.def( "UnmixStains", py::overload_cast< dip::Image const&, dip::Image&, std::vector< dip::Image::Pixel > const& >( &dip::UnmixStains ),
          "in"_a, py::kw_only(), "out"_a, "stains"_a, doc_strings::dip·UnmixStains·Image·CL·Image·L·std·vectorltImage·Pixelgt·CL );
   m.def( "MixStains", py::overload_cast< dip::Image const&, std::vector< dip::Image::Pixel > const& >( &dip::MixStains ),
          "in"_a, "stains"_a, doc_strings::dip·MixStains·Image·CL·Image·L·std·vectorltImage·Pixelgt·CL );
   m.def( "MixStains", py::overload_cast< dip::Image const&, dip::Image&, std::vector< dip::Image::Pixel > const& >( &dip::MixStains ),
          "in"_a, py::kw_only(), "out"_a, "stains"_a, doc_strings::dip·MixStains·Image·CL·Image·L·std·vectorltImage·Pixelgt·CL );

   m.def( "MandersOverlapCoefficient", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MandersOverlapCoefficient ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MandersOverlapCoefficient·Image·CL·Image·CL·Image·CL );
   m.def( "IntensityCorrelationQuotient", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::IntensityCorrelationQuotient ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, doc_strings::dip·IntensityCorrelationQuotient·Image·CL·Image·CL·Image·CL );
   m.def( "MandersColocalizationCoefficients", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat >( &dip::MandersColocalizationCoefficients ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, "threshold1"_a = 0.0, "threshold2"_a = 0.0, doc_strings::dip·MandersColocalizationCoefficients·Image·CL·Image·CL·Image·CL·dfloat··dfloat· );
   m.def( "CostesColocalizationCoefficients", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::CostesColocalizationCoefficients ),
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, doc_strings::dip·CostesColocalizationCoefficients·Image·CL·Image·CL·Image·CL );
   m.def( "CostesSignificanceTest", []( dip::Image const& channel1, dip::Image const& channel2, dip::Image const& mask, dip::UnsignedArray blockSizes, dip::uint repetitions ) {
                 return dip::CostesSignificanceTest( channel1, channel2, mask, RandomNumberGenerator(), std::move( blockSizes ), repetitions );
          },
          "channel1"_a, "channel2"_a, "mask"_a = dip::Image{}, "blockSizes"_a = dip::UnsignedArray{ 3 }, "repetitions"_a = 200,
          "Computes Costes' test of significance of true colocalization.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );

   m.def( "IncoherentOTF", py::overload_cast< dip::UnsignedArray const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::IncoherentOTF ),
          "sizes"_a = dip::UnsignedArray{ 256, 256 }, "defocus"_a = 0.0, "oversampling"_a = 1.0, "amplitude"_a = 1.0, "method"_a = dip::S::STOKSETH, doc_strings::dip·IncoherentOTF·UnsignedArray·CL·dfloat··dfloat··dfloat··String·CL );
   m.def( "IncoherentOTF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::IncoherentOTF ),
          py::kw_only(), "out"_a, "defocus"_a = 0.0, "oversampling"_a = 1.0, "amplitude"_a = 1.0, "method"_a = dip::S::STOKSETH, doc_strings::dip·IncoherentOTF·Image·L·dfloat··dfloat··dfloat··String·CL );
   m.def( "IncoherentPSF", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::IncoherentPSF ),
          "oversampling"_a = 1.0, "amplitude"_a = 1.0, doc_strings::dip·IncoherentPSF·Image·L·dfloat··dfloat· );
   m.def( "IncoherentPSF", py::overload_cast< dip::Image&, dip::dfloat, dip::dfloat >( &dip::IncoherentPSF ),
          py::kw_only(), "out"_a, "oversampling"_a = 1.0, "amplitude"_a = 1.0, doc_strings::dip·IncoherentPSF·Image·L·dfloat··dfloat· );
   m.def( "ExponentialFitCorrection", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::dfloat, dip::String const& >( &dip::ExponentialFitCorrection ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = -1.0, "fromWhere"_a = "first plane", "hysteresis"_a = 1.0, "weighting"_a = "none", doc_strings::dip·ExponentialFitCorrection·Image·CL·Image·CL·Image·L·dfloat··String·CL·dfloat··String·CL );
   m.def( "ExponentialFitCorrection", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::dfloat, dip::String const& >( &dip::ExponentialFitCorrection ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "percentile"_a = -1.0, "fromWhere"_a = "first plane", "hysteresis"_a = 1.0, "weighting"_a = "none", doc_strings::dip·ExponentialFitCorrection·Image·CL·Image·CL·Image·L·dfloat··String·CL·dfloat··String·CL );
   m.def( "AttenuationCorrection", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::AttenuationCorrection ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "background"_a = 0.0, "threshold"_a = 0.0, "NA"_a = 1.4, "refIndex"_a = 1.518, "method"_a = "DET", doc_strings::dip·AttenuationCorrection·Image·CL·Image·L·dfloat··dfloat··dfloat··dfloat··dfloat··dfloat··String·CL );
   m.def( "AttenuationCorrection", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::String const& >( &dip::AttenuationCorrection ),
          "in"_a, py::kw_only(), "out"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "background"_a = 0.0, "threshold"_a = 0.0, "NA"_a = 1.4, "refIndex"_a = 1.518, "method"_a = "DET", doc_strings::dip·AttenuationCorrection·Image·CL·Image·L·dfloat··dfloat··dfloat··dfloat··dfloat··dfloat··String·CL );
   m.def( "SimulatedAttenuation", py::overload_cast< dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat >( &dip::SimulatedAttenuation ),
          "in"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "NA"_a = 1.4, "refIndex"_a = 1.518, "oversample"_a = 1, "rayStep"_a = 1, doc_strings::dip·SimulatedAttenuation·Image·CL·Image·L·dfloat··dfloat··dfloat··dfloat··dip·uint··dfloat· );
   m.def( "SimulatedAttenuation", py::overload_cast< dip::Image const&, dip::Image&, dip::dfloat, dip::dfloat, dip::dfloat, dip::dfloat, dip::uint, dip::dfloat >( &dip::SimulatedAttenuation ),
          "in"_a, py::kw_only(), "out"_a, "fAttenuation"_a = 0.01, "bAttenuation"_a = 0.01, "NA"_a = 1.4, "refIndex"_a = 1.518, "oversample"_a = 1, "rayStep"_a = 1, doc_strings::dip·SimulatedAttenuation·Image·CL·Image·L·dfloat··dfloat··dfloat··dfloat··dip·uint··dfloat· );

}
