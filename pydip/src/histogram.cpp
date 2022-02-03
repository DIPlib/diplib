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
#include "diplib/histogram.h"
#include "diplib/distribution.h"
#include "diplib/lookup_table.h"

namespace pybind11 {
namespace detail {

// Cast Python string to dip::Histogram::Configuration::Mode
template<>
class type_caster< dip::Histogram::Configuration::Mode > {
   public:
      using type = dip::Histogram::Configuration::Mode;
      bool load( handle src, bool ) {
         if( !src ) {
            return false;
         }
         if( PYBIND11_BYTES_CHECK( src.ptr() ) || PyUnicode_Check( src.ptr() )) {
            auto mode = src.cast< dip::String >();
            if( mode == "COMPUTE_BINSIZE" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE; }
            else if( mode == "COMPUTE_BINS" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_BINS; }
            else if( mode == "COMPUTE_LOWER" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_LOWER; }
            else if( mode == "COMPUTE_UPPER" ) { value = dip::Histogram::Configuration::Mode::COMPUTE_UPPER; }
            else { return false; }
            return true;
         }
         return false;
      }
      static handle cast( dip::Histogram::Configuration::Mode const& src, return_value_policy, handle ) {
         switch( src ) {
            case dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE:
               return py::cast( "COMPUTE_BINSIZE" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_BINS:
               return py::cast( "COMPUTE_BINS" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_LOWER:
               return py::cast( "COMPUTE_LOWER" ).release();
            case dip::Histogram::Configuration::Mode::COMPUTE_UPPER:
               return py::cast( "COMPUTE_UPPER" ).release();
         }
         return py::cast( "Unrecognized configuration mode!?" ).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "Mode" ));
};

} // namespace detail
} // namespace pybind11


void init_histogram( py::module& m ) {

   // diplib/histogram.h
   auto hist = py::class_< dip::Histogram >( m, "Histogram", "Computes and holds histograms." );

   auto conf = py::class_< dip::Histogram::Configuration >( hist, "Configuration", "Configuration information for how the histogram is computed." );
   conf.def_readwrite( "lowerBound", &dip::Histogram::Configuration::lowerBound, "Lower bound for this dimension, corresponds to the lower bound of the first bin." );
   conf.def_readwrite( "upperBound", &dip::Histogram::Configuration::upperBound, "Upper bound for this dimension, corresponds to the upper bound of the last bin." );
   conf.def_readwrite( "nBins", &dip::Histogram::Configuration::nBins, "Number of bins for this dimension." );
   conf.def_readwrite( "binSize", &dip::Histogram::Configuration::binSize, "Size of each bin for this dimension." );
   conf.def_readwrite( "mode", &dip::Histogram::Configuration::mode, "The given value is ignored and replaced by the computed value." );
   conf.def_readwrite( "lowerIsPercentile", &dip::Histogram::Configuration::lowerIsPercentile, "If set, `lowerBound` is replaced by the given percentile pixel value." );
   conf.def_readwrite( "upperIsPercentile", &dip::Histogram::Configuration::upperIsPercentile, "If set, `upperBound` is replaced by the given percentile pixel value." );
   conf.def_readwrite( "excludeOutOfBoundValues", &dip::Histogram::Configuration::excludeOutOfBoundValues, "If set, pixels outside of the histogram bounds are not counted." );
   conf.def( py::init<>() );
   conf.def( py::init< dip::dfloat, dip::dfloat, dip::dfloat >(), "lowerBound"_a, "upperBound"_a, "binSize"_a );
   conf.def( py::init< dip::dfloat, dip::dfloat, dip::uint >(), "lowerBound"_a, "upperBound"_a, "nBins"_a = 256 );
   conf.def( py::init< dip::dfloat, dip::uint, dip::dfloat >(), "lowerBound"_a, "nBins"_a, "binSize"_a );
   conf.def( py::init< dip::DataType >(), "dataType"_a );
   conf.def( "__repr__", []( dip::Histogram::Configuration const& self ) {
                std::ostringstream os;
                switch( self.mode ) {
                   case dip::Histogram::Configuration::Mode::COMPUTE_BINSIZE:
                      os << "<Histogram.Configuration, [" << self.lowerBound << ',' << self.upperBound << "], "
                         << self.nBins << " bins>";
                      break;
                   case dip::Histogram::Configuration::Mode::COMPUTE_BINS:
                      os << "<Histogram.Configuration, [" << self.lowerBound << ',' << self.upperBound << "], bin width "
                         << self.binSize << ">";
                      break;
                   case dip::Histogram::Configuration::Mode::COMPUTE_LOWER:
                      os << "<Histogram.Configuration, [?," << self.upperBound << "], "
                         << self.nBins << " bins of width " << self.binSize << ">";
                      break;
                   case dip::Histogram::Configuration::Mode::COMPUTE_UPPER:
                      os << "<Histogram.Configuration, [" << self.lowerBound << ",?], "
                         << self.nBins << " bins of width " << self.binSize << ">";
                      break;
                }
                return os.str();
             } );

   hist.def( py::init< dip::Image const&, dip::Image const&, dip::Histogram::ConfigurationArray >(),
             "input"_a, "mask"_a = dip::Image{}, "configuration"_a = dip::Histogram::ConfigurationArray{} );
   hist.def( py::init( []( dip::Image const& input, dip::Image const& mask, dip::FloatArray const& bounds, dip::uint nBins, bool boundsArePercentile ) {
                DIP_THROW_IF( bounds.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                dip::Histogram::Configuration config( bounds[ 0 ], bounds[ 1 ], nBins );
                if( boundsArePercentile ) {
                   config.lowerIsPercentile = config.upperIsPercentile = boundsArePercentile;
                }
                return dip::Histogram( input, mask, config );
             } ),
             "input"_a, "mask"_a = dip::Image{}, "bounds"_a = dip::FloatArray{ 0, 255 }, "nBins"_a = 256, "boundsArePercentile"_a = false );
   hist.def( py::init< dip::Image const&, dip::Image const&, dip::Image const&, dip::Histogram::ConfigurationArray >(),
             "input1"_a, "input2"_a, "mask"_a = dip::Image{}, "configuration"_a = dip::Histogram::ConfigurationArray{} );
   hist.def( py::init( []( dip::Image const& input1, dip::Image const& input2, dip::Image const& mask,
                           dip::FloatArray const& bounds1, dip::FloatArray const& bounds2, dip::uint nBins1, dip::uint nBins2, bool boundsArePercentile ) {
                DIP_THROW_IF( bounds1.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                DIP_THROW_IF( bounds2.size() != 2, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
                dip::Histogram::ConfigurationArray config{{ bounds1[ 0 ], bounds1[ 1 ], nBins1 },
                                                          { bounds2[ 0 ], bounds2[ 1 ], nBins2 }};
                if( boundsArePercentile ) {
                   config[0].lowerIsPercentile = config[0].upperIsPercentile = boundsArePercentile;
                   config[1].lowerIsPercentile = config[1].upperIsPercentile = boundsArePercentile;
                }
                return dip::Histogram( input1, input2, mask, config );
             } ),
             "input1"_a, "input2"_a, "mask"_a = dip::Image{}, "bounds1"_a = dip::FloatArray{ 0, 255 }, "bounds2"_a = dip::FloatArray{ 0, 100 },
             "nBins1"_a = 256, "nBins2"_a = 256, "boundsArePercentile"_a = false );
   hist.def( py::init< dip::Histogram::ConfigurationArray >(), "configuration"_a );
   hist.def( "__repr__", []( dip::Histogram const& self ) {
                if( self.IsInitialized() ) {
                   std::ostringstream os;
                   os << "<Histogram, " << self.Dimensionality() << "D>";
                   return os.str();
                }
                return std::string{ "<Histogram, uninitialized>" };
             } );
   hist.def( "__str__", []( dip::Histogram const& self ) { std::ostringstream os; os << self; return os.str(); } );
   hist.def( "IsInitialized", &dip::Histogram::IsInitialized );
   hist.def( "Copy", &dip::Histogram::Copy );
   hist.def( "ReverseLookup", py::overload_cast< dip::Image const&, dip::BooleanArray const& >( &dip::Histogram::ReverseLookup ),
             "input"_a, "excludeOutOfBoundValues"_a = dip::BooleanArray{ false } );
   hist.def( py::self += py::self );
   hist.def( py::self + py::self );
   hist.def( py::self -= py::self );
   hist.def( py::self - py::self );
   hist.def( "Dimensionality", &dip::Histogram::Dimensionality );
   hist.def( "Bins", &dip::Histogram::Bins, "dim"_a = 0 );
   hist.def( "BinSize", &dip::Histogram::BinSize, "dim"_a = 0 );
   hist.def( "LowerBound", &dip::Histogram::LowerBound, "dim"_a = 0 );
   hist.def( "UpperBound", &dip::Histogram::UpperBound, "dim"_a = 0 );
   hist.def( "BinBoundaries", &dip::Histogram::BinBoundaries, "dim"_a = 0 );
   hist.def( "BinCenters", &dip::Histogram::BinCenters, "dim"_a = 0 );
   hist.def( "BinCenter", &dip::Histogram::BinCenter, "bin"_a, "dim"_a = 0 );
   hist.def( "Bin", py::overload_cast< dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "value"_a );
   hist.def( "Bin", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "x_value"_a, "y_value"_a );
   hist.def( "Bin", py::overload_cast< dip::dfloat, dip::dfloat, dip::dfloat >( &dip::Histogram::Bin, py::const_ ), "x_value"_a, "y_value"_a, "z_value"_a );
   hist.def( "Bin", py::overload_cast< dip::FloatArray const& >( &dip::Histogram::Bin, py::const_ ), "value"_a );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x ) -> dip::Histogram::CountType { return self.At( x ); } );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x, dip::uint y ) -> dip::Histogram::CountType { return self.At( x, y ); } );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::uint x, dip::uint y, dip::uint z ) -> dip::Histogram::CountType { return self.At( x, y, z ); } );
   hist.def( "__getitem__", []( dip::Histogram const& self, dip::UnsignedArray const& bin ) -> dip::Histogram::CountType { return self.At( bin ); } );
   hist.def( "GetImage", &dip::Histogram::GetImage );
   hist.def( "Count", &dip::Histogram::Count );
   hist.def( "Cumulative", &dip::Histogram::Cumulative );
   hist.def( "GetMarginal", &dip::Histogram::GetMarginal, "dim"_a );
   hist.def( "Smooth", py::overload_cast< dip::FloatArray >( &dip::Histogram::Smooth ), "sigma"_a = dip::FloatArray{ 1 });

   m.def( "CumulativeHistogram", &dip::CumulativeHistogram, "in"_a );
   m.def( "Smooth", py::overload_cast< dip::Histogram const&, dip::FloatArray const& >( &dip::Smooth ),
          "in"_a, "sigma"_a = dip::FloatArray{ 1 } );
   m.def( "Mean", py::overload_cast< dip::Histogram const& >( &dip::Mean ), "in"_a );
   m.def( "Covariance", py::overload_cast< dip::Histogram const& >( &dip::Covariance ), "in"_a );
   m.def( "MarginalPercentile", &dip::MarginalPercentile, "in"_a, "percentile"_a = 50 );
   m.def( "MarginalMedian", &dip::MarginalMedian, "in"_a );
   m.def( "Mode", py::overload_cast< dip::Histogram const& >( &dip::Mode ), "in"_a );
   m.def( "PearsonCorrelation", py::overload_cast< dip::Histogram const& >( &dip::PearsonCorrelation ), "in"_a );
   m.def( "Regression", py::overload_cast< dip::Histogram const& >( &dip::Regression ), "in"_a );
   m.def( "MutualInformation", py::overload_cast< dip::Histogram const& >( &dip::MutualInformation ), "in"_a );
   m.def( "Entropy", py::overload_cast< dip::Histogram const& >( &dip::Entropy ), "in"_a );

   m.def( "IsodataThreshold", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::IsodataThreshold ),
          "in"_a, "nThresholds"_a = 1 );
   m.def( "OtsuThreshold", py::overload_cast< dip::Histogram const& >( &dip::OtsuThreshold ), "in"_a );
   m.def( "MinimumErrorThreshold", py::overload_cast< dip::Histogram const& >( &dip::MinimumErrorThreshold ), "in"_a );
   m.def( "GaussianMixtureModelThreshold", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::GaussianMixtureModelThreshold ),
          "in"_a, "nThresholds"_a = 1 );
   m.def( "TriangleThreshold", py::overload_cast< dip::Histogram const& >( &dip::TriangleThreshold ), "in"_a );
   m.def( "BackgroundThreshold", py::overload_cast< dip::Histogram const&, dip::dfloat >( &dip::BackgroundThreshold ),
          "in"_a, "distance"_a = 2.0 );
   m.def( "KMeansClustering", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::KMeansClustering ),
          "in"_a, "nClusters"_a = 2 );
   m.def( "MinimumVariancePartitioning", py::overload_cast< dip::Histogram const&, dip::uint >( &dip::MinimumVariancePartitioning ),
          "in"_a, "nClusters"_a = 2 );
   m.def( "EqualizationLookupTable", &dip::EqualizationLookupTable, "in"_a );
   m.def( "MatchingLookupTable", &dip::MatchingLookupTable, "in"_a, "example"_a );

   m.def( "PerObjectHistogram", &dip::PerObjectHistogram, "grey"_a, "label"_a, "mask"_a = dip::Image{},
          "configuration"_a = dip::Histogram::Configuration{}, "mode"_a = dip::S::FRACTION, "background"_a = dip::S::EXCLUDE );

   auto regParams = py::class_< dip::RegressionParameters >( m, "RegressionParameters", "Regression parameters." );
   regParams.def( "__repr__", []( dip::RegressionParameters const& s ) {
                     std::ostringstream os;
                     os << "<RegressionParameters: intercept=" << s.intercept << ", slope=" << s.slope << '>';
                     return os.str();
                  } );
   regParams.def_readonly( "intercept", &dip::RegressionParameters::intercept );
   regParams.def_readonly( "slope", &dip::RegressionParameters::slope );

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
             return py::make_tuple( im, bins ).release();
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
             return py::make_tuple( im, bins ).release();
          }, "input1"_a, "input2"_a, "mask"_a = dip::Image{},
          "Overload that takes two scalar images and outputs a 2D histogram." );

   // diplib/lookup_table.h
   auto lut = py::class_< dip::LookupTable >( m, "LookupTable", "Encapsulates the concept of the look-up table (LUT)." );
   lut.def( py::init< dip::Image, dip::FloatArray >(), "values"_a, "index"_a = dip::FloatArray{} );
   lut.def( "__repr__", []( dip::LookupTable const& self ) {
               std::ostringstream os;
               os << "<LookupTable, " << self.DataType();
               if( self.HasIndex() ) {
                  os << ", with index";
               }
               os << '>';
               return os.str();
            } );
   lut.def( "HasIndex", &dip::LookupTable::HasIndex );
   lut.def( "DataType", &dip::LookupTable::DataType );
   lut.def( "SetOutOfBoundsValue", py::overload_cast< dip::dfloat >( &dip::LookupTable::SetOutOfBoundsValue ), "value"_a );
   lut.def( "SetOutOfBoundsValue", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::LookupTable::SetOutOfBoundsValue ), "lowerValue"_a, "upperValue"_a );
   lut.def( "KeepInputValueOnOutOfBounds", &dip::LookupTable::KeepInputValueOnOutOfBounds );
   lut.def( "ClampOutOfBoundsValues", &dip::LookupTable::ClampOutOfBoundsValues );
   lut.def( "Apply", py::overload_cast< dip::Image const&, dip::String const& >( &dip::LookupTable::Apply, py::const_ ), "in"_a, "interpolation"_a = dip::S::LINEAR );
   lut.def( "Apply", py::overload_cast< dip::dfloat, dip::String const& >( &dip::LookupTable::Apply, py::const_ ), "value"_a, "interpolation"_a = dip::S::LINEAR );
   lut.def( "Convert", &dip::LookupTable::Convert, "dataType"_a );

   // This next function is the old implementation of `dip.LookupTable`, which we keep
   // here for backwards compatibility. Setting `dip.LookupTable = dip.LookupTable_old` in Python
   // will allow old programs that use this function to continue working.
   m.def( "LookupTable_old", []( dip::Image const& in, dip::Image const& lut, dip::FloatArray const& index,
                                 dip::String const& interpolation, dip::String const& mode,
                                 dip::dfloat lowerValue, dip::dfloat upperValue ) {
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
