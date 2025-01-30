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
#include <vector>

#include "pydip.h"
#include "diplib/histogram.h"
#include "diplib/distribution.h"
#include "diplib/lookup_table.h"

#if defined(__clang__)
// Clang gives a bogus diagnostic here for `py::self -= py::self`
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#endif

namespace pybind11 {
namespace detail {

// Cast Python string to dip::Histogram::Configuration::Mode
template<>
class type_caster< dip::Histogram::Configuration::Mode > {
   public:
      using type = dip::Histogram::Configuration::Mode;

      bool load( handle src, bool /*convert*/ ) {
         if( !src ) {
            return false;
         }
         if( PYBIND11_BYTES_CHECK( src.ptr() ) || PyUnicode_Check( src.ptr() )) {
            auto mode = src.cast< dip::String >();
            if( mode == "COMPUTE_BINSIZE" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE; }
            else if( mode == "COMPUTE_BINS" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_BINS; }
            else if( mode == "COMPUTE_LOWER" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_LOWER; }
            else if( mode == "COMPUTE_UPPER" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_UPPER; }
            else if( mode == "ESTIMATE_BINSIZE" ) { value = dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE; }
            else if( mode == "ESTIMATE_BINSIZE_AND_LIMITS" ) { value = dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE_AND_LIMITS; }
            else if( mode == "IS_COMPLETE" ) { value = dip::Histogram::Configuration::Mode::IS_COMPLETE; }
            else { return false; }
            return true;
         }
         return false;
      }

      static handle cast( dip::Histogram::Configuration::Mode const& src, return_value_policy /*policy*/, handle /*parent*/) {
         switch( src ) {
            case dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE:
               return py::cast( "COMPUTE_BINSIZE" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_BINS:
               return py::cast( "COMPUTE_BINS" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_LOWER:
               return py::cast( "COMPUTE_LOWER" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_UPPER:
               return py::cast( "COMPUTE_UPPER" ).release();
            case dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE:
               return py::cast( "ESTIMATE_BINSIZE" ).release();
            case dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE_AND_LIMITS:
               return py::cast( "ESTIMATE_BINSIZE_AND_LIMITS" ).release();
            case dip::Histogram::Configuration::Mode::IS_COMPLETE:
               return py::cast( "IS_COMPLETE" ).release();
         }
         return py::cast( "Unrecognized configuration mode!?" ).release();
      }

      PYBIND11_TYPE_CASTER( type, _( "Mode" ));
};

DIP_OUTPUT_TYPE_CASTER( RegressionParameters, "RegressionParameters", "intercept slope", src.intercept, src.slope )
DIP_OUTPUT_TYPE_CASTER( GaussianParameters, "GaussianParameters", "position amplitude sigma", src.position, src.amplitude, src.sigma )

} // namespace detail
} // namespace pybind11

namespace {

char const* Format( bool v ) {
   return v ? "%" : "";
}

dip::String ConfigRepr( dip::Histogram::Configuration const& s ) {
   std::ostringstream os;
   os << "<Histogram.Configuration, ";
   switch( s.mode ) {
      case dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE:
         os << '[' << s.lowerBound << Format( s.lowerIsPercentile )
            << ',' << s.upperBound << Format( s.upperIsPercentile )
            << "], " << s.nBins << " bins";
         break;
      case dip::Histogram::Configuration::Mode::COMPUTE_BINS:
         os << '[' << s.lowerBound << Format( s.lowerIsPercentile )
            << ',' << s.upperBound << Format( s.upperIsPercentile )
            << "], bin width " << s.binSize;
         break;
      case dip::Histogram::Configuration::Mode::COMPUTE_LOWER:
         os << "[?," << s.upperBound << Format( s.upperIsPercentile )
            << "], " << s.nBins << " bins of width " << s.binSize;
         break;
      case dip::Histogram::Configuration::Mode::COMPUTE_UPPER:
         os << '[' << s.lowerBound << Format( s.lowerIsPercentile )
            << ",?], " << s.nBins << " bins of width " << s.binSize;
         break;
      case dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE:
         os << '[' << s.lowerBound << Format( s.lowerIsPercentile )
            << ',' << s.upperBound << Format( s.upperIsPercentile )
            << "], bin width estimated with Freedman-Diaconis rule";
         break;
      case dip::Histogram::Configuration::Mode::ESTIMATE_BINSIZE_AND_LIMITS:
         os << "bin width estimated with Freedman-Diaconis rule, limits adjusted to exclude outliers";
         break;
      case dip::Histogram::Configuration::Mode::IS_COMPLETE:
         os << '[' << s.lowerBound << ',' << s.upperBound
            << "], " << s.nBins << " bins, bin width " << s.binSize
            << " (complete)";
         break;
   }
   os << '>';
   return os.str();
}

} // namespace

void init_histogram( py::module& m ) {

   // diplib/histogram.h
   auto hist = py::class_< dip::Histogram >( m, "Histogram", doc_strings::dip·Histogram );

   auto conf = py::class_< dip::Histogram::Configuration >( hist, "Configuration", doc_strings::dip·Histogram·Configuration );
   conf.def_readwrite( "lowerBound", &dip::Histogram::Configuration::lowerBound, doc_strings::dip·Histogram·Configuration·lowerBound );
   conf.def_readwrite( "upperBound", &dip::Histogram::Configuration::upperBound, doc_strings::dip·Histogram·Configuration·upperBound );
   conf.def_readwrite( "nBins", &dip::Histogram::Configuration::nBins, doc_strings::dip·Histogram·Configuration·nBins );
   conf.def_readwrite( "binSize", &dip::Histogram::Configuration::binSize, doc_strings::dip·Histogram·Configuration·binSize );
   conf.def_readwrite( "mode", &dip::Histogram::Configuration::mode, doc_strings::dip·Histogram·Configuration·mode );
   conf.def_readwrite( "lowerIsPercentile", &dip::Histogram::Configuration::lowerIsPercentile, doc_strings::dip·Histogram·Configuration·lowerIsPercentile );
   conf.def_readwrite( "upperIsPercentile", &dip::Histogram::Configuration::upperIsPercentile, doc_strings::dip·Histogram·Configuration·upperIsPercentile );
   conf.def_readwrite( "excludeOutOfBoundValues", &dip::Histogram::Configuration::excludeOutOfBoundValues, doc_strings::dip·Histogram·Configuration·excludeOutOfBoundValues );
   conf.def( py::init<>(), doc_strings::dip·Histogram·Configuration·Configuration );
   conf.def( py::init< dip::dfloat, dip::dfloat, dip::dfloat >(), "lowerBound"_a, "upperBound"_a, "binSize"_a, doc_strings::dip·Histogram·Configuration·Configuration·dfloat··dfloat··dfloat· );
   conf.def( py::init< dip::dfloat, dip::dfloat, dip::uint >(), "lowerBound"_a, "upperBound"_a, "nBins"_a, doc_strings::dip·Histogram·Configuration·Configuration·dfloat··dfloat··dip·uint· );
   conf.def( py::init< dip::dfloat, dip::uint, dip::dfloat >(), "lowerBound"_a, "nBins"_a, "binSize"_a, doc_strings::dip·Histogram·Configuration·Configuration·dfloat··dip·uint··dfloat· );
   conf.def( py::init< dip::dfloat, dip::dfloat >(), "lowerBound"_a, "upperBound"_a, "The number of bins defaults to 256, the bin size is computed." );
   conf.def( py::init< dip::DataType >(), "dataType"_a, doc_strings::dip·Histogram·Configuration·Configuration·DataType· );
   conf.def( "__repr__", &ConfigRepr );
   hist.def( "OptimalConfiguration", &dip::Histogram::OptimalConfiguration, doc_strings::dip·Histogram·OptimalConfiguration );
   hist.def( "OptimalConfigurationWithFullRange", &dip::Histogram::OptimalConfigurationWithFullRange, doc_strings::dip·Histogram·OptimalConfigurationWithFullRange );

   hist.def( py::init< dip::Image const&, dip::Image const&, dip::Histogram::ConfigurationArray >(),
             "input"_a, "mask"_a = dip::Image{}, "configuration"_a = dip::Histogram::ConfigurationArray{}, doc_strings::dip·Histogram·Histogram·Image·CL·Image·CL·ConfigurationArray· );
   hist.def( py::init( []( dip::Image const& input, dip::Image const& mask, dip::FloatArray const& bounds, dip::uint nBins, bool boundsArePercentile ) {
                DIP_THROW_IF( bounds.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                dip::Histogram::Configuration config( bounds[ 0 ], bounds[ 1 ], nBins );
                if( boundsArePercentile ) {
                   config.lowerIsPercentile = config.upperIsPercentile = boundsArePercentile;
                }
                return dip::Histogram( input, mask, config );
             } ),
             "input"_a, "mask"_a = dip::Image{}, "bounds"_a = dip::FloatArray{ 0, 255 }, "nBins"_a = 256, "boundsArePercentile"_a = false,
             "A constructor that creates a `dip::Histogram::Configuration` object for you." );
   hist.def( py::init< dip::Image const&, dip::Image const&, dip::Image const&, dip::Histogram::ConfigurationArray >(),
             "input1"_a, "input2"_a, "mask"_a = dip::Image{}, "configuration"_a = dip::Histogram::ConfigurationArray{}, doc_strings::dip·Histogram·Histogram·Image·CL·Image·CL·Image·CL·ConfigurationArray· );
   hist.def( py::init( []( dip::Image const& input1,
                           dip::Image const& input2,
                           dip::Image const& mask,
                           dip::FloatArray const& bounds1,
                           dip::FloatArray const& bounds2,
                           dip::uint nBins1,
                           dip::uint nBins2,
                           bool boundsArePercentile ) {
                DIP_THROW_IF( bounds1.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                DIP_THROW_IF( bounds2.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                dip::Histogram::ConfigurationArray config{
                   { bounds1[ 0 ], bounds1[ 1 ], nBins1 },
                   { bounds2[ 0 ], bounds2[ 1 ], nBins2 }
                };
                if( boundsArePercentile ) {
                   config[ 0 ].lowerIsPercentile = config[ 0 ].upperIsPercentile = boundsArePercentile;
                   config[ 1 ].lowerIsPercentile = config[ 1 ].upperIsPercentile = boundsArePercentile;
                }
                return dip::Histogram( input1, input2, mask, config );
             } ),
             "input1"_a, "input2"_a, "mask"_a = dip::Image{}, "bounds1"_a = dip::FloatArray{ 0, 255 }, "bounds2"_a = dip::FloatArray{ 0, 100 },
             "nBins1"_a = 256, "nBins2"_a = 256, "boundsArePercentile"_a = false,
             "A constructor that creates a `dip::Histogram::Configuration` object for you." );
   hist.def( py::init< dip::Histogram::ConfigurationArray >(), "configuration"_a, doc_strings::dip·Histogram·Histogram·ConfigurationArray· );
   hist.def( "__repr__", []( dip::Histogram const& self ) {
      if( !self.IsInitialized() ) {
         return std::string{ "<Uninitialized histogram>" };
      }
      std::ostringstream os;
      os << "<Histogram, sizes " << self.GetImage().Sizes() << '>';
      return os.str();
   } );
   hist.def( "__str__", []( dip::Histogram const& self ) {
      std::ostringstream os;
      os << self;
      return os.str();
   } );
   hist.def( "IsInitialized", &dip::Histogram::IsInitialized, doc_strings::dip·Histogram·IsInitialized·C );
   hist.def( "Copy", &dip::Histogram::Copy, doc_strings::dip·Histogram·Copy·C );
   hist.def( "ReverseLookup", py::overload_cast< dip::Image const&, dip::BooleanArray >( &dip::Histogram::ReverseLookup ),
             "input"_a, "excludeOutOfBoundValues"_a = dip::BooleanArray{ false }, doc_strings::dip·Histogram·ReverseLookup·Image·CL·Image·L·BooleanArray· );
   hist.def( "ReverseLookup", py::overload_cast< dip::Image const&, dip::Image&, dip::BooleanArray >( &dip::Histogram::ReverseLookup ),
             "input"_a, py::kw_only(), "out"_a, "excludeOutOfBoundValues"_a = dip::BooleanArray{ false }, doc_strings::dip·Histogram·ReverseLookup·Image·CL·Image·L·BooleanArray· );
   hist.def( py::self += py::self, doc_strings::dip·Histogram·operatorpluseq·Histogram·CL );
   hist.def( py::self + py::self, doc_strings::dip·operatorplus·Histogram·CL·Histogram·CL );
   hist.def( py::self -= py::self, doc_strings::dip·Histogram·operatorminuseq·Histogram·CL );
   hist.def( py::self - py::self, doc_strings::dip·operatorminus·Histogram·CL·Histogram·CL ); // NOLINT(*-redundant-expression)
   hist.def( "Dimensionality", &dip::Histogram::Dimensionality, doc_strings::dip·Histogram·Dimensionality·C );
   hist.def( "Bins", &dip::Histogram::Bins, "dim"_a = 0, doc_strings::dip·Histogram·Bins·dip·uint··C );
   hist.def( "BinSize", &dip::Histogram::BinSize, "dim"_a = 0, doc_strings::dip·Histogram·BinSize·dip·uint··C );
   hist.def( "LowerBound", &dip::Histogram::LowerBound, "dim"_a = 0, doc_strings::dip·Histogram·LowerBound·dip·uint··C );
   hist.def( "UpperBound", &dip::Histogram::UpperBound, "dim"_a = 0, doc_strings::dip·Histogram·UpperBound·dip·uint··C );
   hist.def( "BinBoundaries", &dip::Histogram::BinBoundaries, "dim"_a = 0, doc_strings::dip·Histogram·BinBoundaries·dip·uint··C );
   hist.def( "BinCenters", &dip::Histogram::BinCenters, "dim"_a = 0, doc_strings::dip·Histogram·BinCenters·dip·uint··C );
   hist.def( "BinCenter", &dip::Histogram::BinCenter, "bin"_a, "dim"_a = 0, doc_strings::dip·Histogram·BinCenter·dip·uint··dip·uint··C );
   hist.def( "Bin", py::overload_cast< dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "value"_a, doc_strings::dip·Histogram·Bin·dfloat··C );
   hist.def( "Bin", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "x_value"_a, "y_value"_a, doc_strings::dip·Histogram·Bin·dfloat··dfloat··C );
   hist.def( "Bin", py::overload_cast< dip::dfloat, dip::dfloat, dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "x_value"_a, "y_value"_a, "z_value"_a, doc_strings::dip·Histogram·Bin·dfloat··dfloat··dfloat··C );
   hist.def( "Bin", py::overload_cast< dip::FloatArray const& >( &dip::Histogram::Bin, py::const_ ), "value"_a, doc_strings::dip·Histogram·Bin·FloatArray·CL·C );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x ) -> dip::Histogram::CountType { return self.At( x ); }, doc_strings::dip·Histogram·At·dip·uint··C );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x, dip::uint y ) -> dip::Histogram::CountType { return self.At( x, y ); }, doc_strings::dip·Histogram·At·dip·uint··dip·uint··C );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x, dip::uint y, dip::uint z ) -> dip::Histogram::CountType { return self.At( x, y, z ); }, doc_strings::dip·Histogram·At·dip·uint··dip·uint··dip·uint··C );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::UnsignedArray const& bin ) -> dip::Histogram::CountType { return self.At( bin ); }, doc_strings::dip·Histogram·At·UnsignedArray·CL·C );
   hist.def( "GetImage", &dip::Histogram::GetImage, doc_strings::dip·Histogram·GetImage·C );
   hist.def( "Count", &dip::Histogram::Count, doc_strings::dip·Histogram·Count·C );
   hist.def( "Cumulative", &dip::Histogram::Cumulative, doc_strings::dip·Histogram·Cumulative );
   hist.def( "GetMarginal", &dip::Histogram::GetMarginal, "dim"_a, doc_strings::dip·Histogram·GetMarginal·dip·uint··C );
   hist.def( "Smooth", py::overload_cast< dip::FloatArray >( &dip::Histogram::Smooth ), "sigma"_a = dip::FloatArray{ 1 }, doc_strings::dip·Histogram·Smooth·FloatArray· );

   m.def( "CumulativeHistogram", &dip::CumulativeHistogram, "in"_a, doc_strings::dip·CumulativeHistogram·Histogram·CL );
   m.def( "Smooth", py::overload_cast< dip::Histogram const&, dip::FloatArray const& >( &dip::Smooth ),
          "in"_a, "sigma"_a = dip::FloatArray{ 1 }, doc_strings::dip·Smooth·Histogram·CL·FloatArray·CL );
   m.def( "Mean", py::overload_cast< dip::Histogram const& >( &dip::Mean ), "in"_a, doc_strings::dip·Mean·Histogram·CL );
   m.def( "Covariance", py::overload_cast< dip::Histogram const& >( &dip::Covariance ), "in"_a, doc_strings::dip·Covariance·Histogram·CL );
   m.def( "MarginalPercentile", &dip::MarginalPercentile, "in"_a, "percentile"_a = 50, doc_strings::dip·MarginalPercentile·Histogram·CL·dfloat· );
   m.def( "MarginalMedian", &dip::MarginalMedian, "in"_a, doc_strings::dip·MarginalMedian·Histogram·CL );
   m.def( "Mode", py::overload_cast< dip::Histogram const& >( &dip::Mode ), "in"_a, doc_strings::dip·Mode·Histogram·CL );
   m.def( "PearsonCorrelation", py::overload_cast< dip::Histogram const& >( &dip::PearsonCorrelation ), "in"_a, doc_strings::dip·PearsonCorrelation·Histogram·CL );
   m.def( "Regression", py::overload_cast< dip::Histogram const& >( &dip::Regression ), "in"_a, doc_strings::dip·Regression·Histogram·CL );
   m.def( "MutualInformation", py::overload_cast< dip::Histogram const& >( &dip::MutualInformation ), "in"_a, doc_strings::dip·MutualInformation·Histogram·CL );
   m.def( "Entropy", py::overload_cast< dip::Histogram const& >( &dip::Entropy ), "in"_a, doc_strings::dip·Entropy·Histogram·CL );
   m.def( "GaussianMixtureModel", py::overload_cast< dip::Histogram const&, dip::uint, dip::uint >( &dip::GaussianMixtureModel ),
          "in"_a, "numberOfGaussians"_a, "maxIter"_a = 20, doc_strings::dip·GaussianMixtureModel·Histogram·CL·dip·uint··dip·uint· );

   m.def( "IsodataThreshold", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "nThresholds"_a = 1, doc_strings::dip·IsodataThreshold·Histogram·CL·dip·uint· );
   m.def( "OtsuThreshold", py::overload_cast< dip::Histogram const& >( &dip::OtsuThreshold ),
          "in"_a, doc_strings::dip·OtsuThreshold·Histogram·CL );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Histogram const& >( &dip::MinimumErrorThreshold ),
          "in"_a, doc_strings::dip·MinimumErrorThreshold·Histogram·CL );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "nThresholds"_a = 1, doc_strings::dip·GaussianMixtureModelThreshold·Histogram·CL·dip·uint· );
   m.def( "TriangleThreshold", py::overload_cast< dip::Histogram const&, dip::dfloat >( &dip::TriangleThreshold ),
          "in"_a, "sigma"_a = 4.0, doc_strings::dip·TriangleThreshold·Histogram·CL·dfloat· );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Histogram const&, dip::dfloat, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "distance"_a = 2.0, "sigma"_a = 4.0, doc_strings::dip·BackgroundThreshold·Histogram·CL·dfloat··dfloat· );
   m.def( "KMeansClustering", []( dip::Histogram const& in, dip::uint nClusters ) {
             return KMeansClustering( in, RandomNumberGenerator(), nClusters );
          },
          "in"_a, "nClusters"_a = 2,
          "Partitions a (multi-dimensional) histogram into `nClusters` partitions using\nk-means clustering.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "KMeansClustering", []( dip::Histogram const& in, dip::Histogram& out, dip::uint nClusters ) {
             KMeansClustering( in, out, RandomNumberGenerator(), nClusters );
          },
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2,
          "Partitions a (multi-dimensional) histogram into `nClusters` partitions using\nk-means clustering.\n"
          "Like the C++ function, but using an internal `dip::Random` object." );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, "nClusters"_a = 2, doc_strings::dip·MinimumVariancePartitioning·Histogram·CL·Histogram·L·dip·uint· );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Histogram const&, dip::Histogram&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, py::kw_only(), "out"_a, "nClusters"_a = 2, doc_strings::dip·MinimumVariancePartitioning·Histogram·CL·Histogram·L·dip·uint· );
   m.def( "EqualizationLookupTable", &dip::EqualizationLookupTable,
          "in"_a, doc_strings::dip·EqualizationLookupTable·Histogram·CL );
   m.def( "MatchingLookupTable", &dip::MatchingLookupTable,
          "in"_a, "example"_a, doc_strings::dip·MatchingLookupTable·Histogram·CL·Histogram·CL );

   m.def( "PerObjectHistogram", &dip::PerObjectHistogram,
          "grey"_a, "label"_a, "mask"_a = dip::Image{}, "configuration"_a = dip::Histogram::Configuration{}, "mode"_a = dip::S::FRACTION, "background"_a = dip::S::EXCLUDE, doc_strings::dip·PerObjectHistogram·Image·CL·Image·CL·Image·CL·Histogram·Configuration··String·CL·String·CL );

   // These next two functions are the old implementation of `dip.Histogram`, which we keep
   // here for backwards compatibility. Setting `dip.Histogram = dip.Histogram_old` in Python
   // will allow old programs that use these functions to continue working.
   m.def( "Histogram_old", []( dip::Image const& input, dip::Image const& mask, dip::uint nBins ) {
             dip::Histogram::Configuration config( input.DataType() );
             config.nBins = nBins;
             config.mode = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE;
             dip::Histogram histogram( input, mask, config );
             dip::Image im = histogram.GetImage();
             std::vector< dip::FloatArray > bins( histogram.Dimensionality() );
             for( dip::uint ii = 0; ii < bins.size(); ++ii ) {
                bins[ ii ] = histogram.BinCenters( ii );
             }
             return py::make_tuple( im, bins );
          }, "input"_a, "mask"_a = dip::Image{}, "nBins"_a = 256,
          "This function exists for backwards compatibility. It takes an image `input`,\n"
          "and optional `mask`, and computes a histogram with `nBins` bins. The output is\n"
          "a tuple with an image containing the bins, and a list of lists containing the\n"
          "bin centers.\n\n"
          "Use `dip.Histogram = dip.Histogram_old` to allow an older program to run\n"
          "without modification." );
   m.def( "Histogram_old", []( dip::Image const& input1, dip::Image const& input2, dip::Image const& mask ) {
             dip::Histogram histogram( input1, input2, mask );
             dip::Image im = histogram.GetImage();
             std::vector< dip::FloatArray > bins( 2 );
             bins[ 0 ] = histogram.BinCenters( 0 );
             bins[ 1 ] = histogram.BinCenters( 1 );
             return py::make_tuple( im, bins );
          }, "input1"_a, "input2"_a, "mask"_a = dip::Image{},
          "Overload that takes two scalar images and outputs a 2D histogram." );

   // diplib/lookup_table.h
   auto lut = py::class_< dip::LookupTable >( m, "LookupTable", doc_strings::dip·LookupTable );
   lut.def( py::init< dip::Image, dip::FloatArray >(), "values"_a, "index"_a = dip::FloatArray{}, doc_strings::dip·LookupTable·LookupTable·Image··FloatArray· );
   lut.def( "__repr__", []( dip::LookupTable const& self ) {
      std::ostringstream os;
      os << "<LookupTable, " << self.DataType();
      if( self.HasIndex() ) {
         os << ", with index";
      }
      os << '>';
      return os.str();
   } );
   lut.def( "HasIndex", &dip::LookupTable::HasIndex, doc_strings::dip·LookupTable·HasIndex·C );
   lut.def( "DataType", &dip::LookupTable::DataType, doc_strings::dip·LookupTable·DataType·C );
   lut.def( "SetOutOfBoundsValue", py::overload_cast< dip::dfloat >( &dip::LookupTable::SetOutOfBoundsValue ),
            "value"_a, doc_strings::dip·LookupTable·SetOutOfBoundsValue·dfloat· );
   lut.def( "SetOutOfBoundsValue", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::LookupTable::SetOutOfBoundsValue ),
            "lowerValue"_a, "upperValue"_a, doc_strings::dip·LookupTable·SetOutOfBoundsValue·dfloat··dfloat· );
   lut.def( "KeepInputValueOnOutOfBounds", &dip::LookupTable::KeepInputValueOnOutOfBounds, doc_strings::dip·LookupTable·KeepInputValueOnOutOfBounds );
   lut.def( "ClampOutOfBoundsValues", &dip::LookupTable::ClampOutOfBoundsValues, doc_strings::dip·LookupTable·ClampOutOfBoundsValues );
   lut.def( "Apply", py::overload_cast< dip::Image const&, dip::String const& >( &dip::LookupTable::Apply, py::const_ ),
            "in"_a, "interpolation"_a = dip::S::LINEAR, doc_strings::dip·LookupTable·Apply·Image·CL·Image·L·String·CL·C );
   lut.def( "Apply", py::overload_cast< dip::Image const&, dip::Image&, dip::String const& >( &dip::LookupTable::Apply, py::const_ ),
            "in"_a, py::kw_only(), "out"_a, "interpolation"_a = dip::S::LINEAR, doc_strings::dip·LookupTable·Apply·Image·CL·Image·L·String·CL·C );
   lut.def( "Apply", py::overload_cast< dip::dfloat, dip::String const& >( &dip::LookupTable::Apply, py::const_ ),
            "value"_a, "interpolation"_a = dip::S::LINEAR, doc_strings::dip·LookupTable·Apply·dfloat··String·CL·C );
   lut.def( "Convert", &dip::LookupTable::Convert,
            "dataType"_a, doc_strings::dip·LookupTable·Convert·dip·DataType· );

   // This next function is the old implementation of `dip.LookupTable`, which we keep
   // here for backwards compatibility. Setting `dip.LookupTable = dip.LookupTable_old` in Python
   // will allow old programs that use this function to continue working.
   m.def( "LookupTable_old", []( dip::Image const& in,
                                 dip::Image const& lut,
                                 dip::FloatArray const& index,
                                 dip::String const& interpolation,
                                 dip::String const& mode,
                                 dip::dfloat lowerValue,
                                 dip::dfloat upperValue ) {
             dip::LookupTable lookupTable( lut, index );
             if( mode == "clamp" ) {
                lookupTable.ClampOutOfBoundsValues(); // is the default...
             } else if( mode == "values" ) {
                lookupTable.SetOutOfBoundsValue( lowerValue, upperValue );
             } else if( mode == "keep" ) {
                lookupTable.KeepInputValueOnOutOfBounds();
             } else {
                DIP_THROW_INVALID_FLAG( mode );
             }
             return lookupTable.Apply( in, interpolation );
          }, "in"_a, "lut"_a, "index"_a = dip::FloatArray{}, "interpolation"_a = dip::S::LINEAR, "mode"_a = "clamp", "lowerValue"_a = 0.0, "upperValue"_a = 0.0,
          "This function exists for backwards compatibility. It takes an image `lut`, and\n"
          "optional `index`, that are converted to a `dip::LookupTable` object, parameters\n"
          "are set, and the lookup table is applied to `in`, returning the result.\n\n"
          "Use `dip.LookupTable = dip.LookupTable_old` to allow an older program to run\n"
          "without modification." );

}

#if defined(__clang__)
   #pragma GCC diagnostic pop
#endif
