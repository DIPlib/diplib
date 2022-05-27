/*
 * (c)2017-2022, Flagship Biosciences, Inc., written by Cris Luengo.
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
#include "diplib/statistics.h"

void init_statistics( py::module& m ) {

   m.def( "Count", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Count ), "in"_a, "mask"_a = dip::Image{} );
   m.def( "MaximumPixel", &dip::MaximumPixel, "in"_a, "mask"_a = dip::Image{}, "positionFlag"_a = dip::S::FIRST );
   m.def( "MinimumPixel", &dip::MinimumPixel, "in"_a, "mask"_a = dip::Image{}, "positionFlag"_a = dip::S::FIRST );
   m.def( "CumulativeSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::CumulativeSum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MaximumAndMinimum", []( dip::Image const& in, dip::Image const& mask ) {
                dip::MinMaxAccumulator acc = dip::MaximumAndMinimum( in, mask );
                return py::make_tuple( acc.Minimum(), acc.Maximum() ).release();
          }, "in"_a, "mask"_a = dip::Image{},
          "Instead of returning a `dip::MinMaxAccumulator` object, returns a tuple with\n"
          "the minimum and maximum values.");
   m.def( "SampleStatistics", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::SampleStatistics ),
          "in"_a, "mask"_a = dip::Image{} );
   m.def( "Covariance", &dip::Covariance, "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "PearsonCorrelation", &dip::PearsonCorrelation, "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "SpearmanRankCorrelation", &dip::SpearmanRankCorrelation, "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "CenterOfMass", &dip::CenterOfMass, "in"_a, "mask"_a = dip::Image{} );
   m.def( "Moments", &dip::Moments, "in"_a, "mask"_a = dip::Image{} );

   m.def( "Mean", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::Mean ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = "", "process"_a = dip::BooleanArray{} );
   m.def( "Sum", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Sum ), // Fails to resolve!
          static_cast< dip::Image( * )( dip::Image const&, dip::Image const&, dip::BooleanArray const& ) >( &dip::Sum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "GeometricMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::GeometricMean ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Product", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Product ), // Fails to resolve!
          static_cast< dip::Image( * )( dip::Image const&, dip::Image const&, dip::BooleanArray const& ) >( &dip::Product ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanSquare ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumSquare ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Variance", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::Variance ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{} );
   m.def( "StandardDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::StandardDeviation ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{} );
   m.def( "Maximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Maximum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Minimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Minimum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MaximumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MaximumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MinimumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MinimumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Percentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::BooleanArray const& >( &dip::Percentile ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = 50.0, "process"_a = dip::BooleanArray{} );
   m.def( "Median", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Median ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MedianAbsoluteDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MedianAbsoluteDeviation ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "All", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::All ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Any", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Any ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );

   m.def( "PositionMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMaximum ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST );
   m.def( "PositionMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMinimum ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST );
   m.def( "PositionPercentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint, dip::String const& >( &dip::PositionPercentile ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = 50, "dim"_a = 0, "mode"_a = dip::S::FIRST );
   m.def( "PositionMedian", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMedian ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST );

   m.def( "RadialSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialSum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{} );
   m.def( "RadialMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMean ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{} );
   m.def( "RadialMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMinimum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{} );
   m.def( "RadialMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMaximum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{} );

   m.def( "MeanError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MeanError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "MeanSquareError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MeanSquareError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "RootMeanSquareError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::RootMeanSquareError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "MeanAbsoluteError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MeanAbsoluteError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "MaximumAbsoluteError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MaximumAbsoluteError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "MeanRelativeError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MeanRelativeError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "MaximumRelativeError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::MaximumRelativeError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "IDivergence", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::IDivergence ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "InProduct", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const& >( &dip::InProduct ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{} );
   m.def( "LnNormError", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat >( &dip::LnNormError ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "order"_a = 2.0 );
   m.def( "PSNR", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat >( &dip::PSNR ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "peakSignal"_a = 0.0 );
   m.def( "SSIM", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::dfloat, dip::dfloat, dip::dfloat >( &dip::SSIM ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "sigma"_a = 1.5, "K1"_a = 0.01, "K2"_a = 0.03 );
   m.def( "MutualInformation", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image const&, dip::uint >( &dip::MutualInformation ),
          "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "nBins"_a = 256 );

   m.def( "SpatialOverlap", &dip::SpatialOverlap, "in"_a, "reference"_a );
   m.def( "DiceCoefficient", &dip::DiceCoefficient, "in"_a, "reference"_a );
   m.def( "JaccardIndex", &dip::JaccardIndex, "in"_a, "reference"_a );
   m.def( "Specificity", &dip::Specificity, "in"_a, "reference"_a );
   m.def( "Sensitivity", &dip::Sensitivity, "in"_a, "reference"_a );
   m.def( "Accuracy", &dip::Accuracy, "in"_a, "reference"_a );
   m.def( "Precision", &dip::Precision, "in"_a, "reference"_a );
   m.def( "HausdorffDistance", &dip::HausdorffDistance, "in"_a, "reference"_a );
   m.def( "ModifiedHausdorffDistance", &dip::ModifiedHausdorffDistance, "in"_a, "reference"_a );
   m.def( "SumOfMinimalDistances", &dip::SumOfMinimalDistances, "in"_a, "reference"_a );
   m.def( "ComplementWeightedSumOfMinimalDistances", &dip::ComplementWeightedSumOfMinimalDistances, "in"_a, "reference"_a );

   m.def( "Entropy", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Entropy ),
          "in"_a, "mask"_a = dip::Image{}, "nBins"_a = 256 );
   m.def( "EstimateNoiseVariance", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::EstimateNoiseVariance ),
          "in"_a, "mask"_a = dip::Image{} );

   // dip::StatisticsAccumulator
   auto statsAcc = py::class_< dip::StatisticsAccumulator >( m, "StatisticsValues", "Statistics values." );
   statsAcc.def( "__repr__", []( dip::StatisticsAccumulator const& s ) {
                    std::ostringstream os;
                    os << "<StatisticsValues: ";
                    os << "mean=" << s.Mean();
                    os << ", standardDev=" << s.StandardDeviation();
                    os << ", variance=" << s.Variance();
                    os << ", skewness=" << s.Skewness();
                    os << ", kurtosis=" << s.ExcessKurtosis();
                    os << ", number=" << s.Number();
                    os << '>';
                    return os.str();
                 } );
   statsAcc.def_property_readonly( "mean", &dip::StatisticsAccumulator::Mean );
   statsAcc.def_property_readonly( "standardDev", &dip::StatisticsAccumulator::StandardDeviation );
   statsAcc.def_property_readonly( "variance", &dip::StatisticsAccumulator::Variance );
   statsAcc.def_property_readonly( "skewness", &dip::StatisticsAccumulator::Skewness );
   statsAcc.def_property_readonly( "kurtosis", &dip::StatisticsAccumulator::ExcessKurtosis );
   statsAcc.def_property_readonly( "number", &dip::StatisticsAccumulator::Number );

   // dip::CovarianceAccumulator
   auto covAcc = py::class_< dip::CovarianceAccumulator >( m, "CovarianceValues", "Covariance values." );
   covAcc.def( "__repr__", []( dip::CovarianceAccumulator const& s ) {
                  std::ostringstream os;
                  os << "<CovarianceValues: ";
                  os << "Number=" << s.Number();
                  os << ", MeanX=" << s.MeanX();
                  os << ", MeanY=" << s.MeanY();
                  os << ", VarianceX=" << s.VarianceX();
                  os << ", VarianceY=" << s.VarianceY();
                  os << ", StandardDeviationX=" << s.StandardDeviationX();
                  os << ", StandardDeviationY=" << s.StandardDeviationY();
                  os << ", Covariance=" << s.Covariance();
                  os << ", Correlation=" << s.Correlation();
                  os << ", Slope=" << s.Slope();
                  os << '>';
                  return os.str();
               } );
   covAcc.def_property_readonly( "Number", &dip::CovarianceAccumulator::Number );
   covAcc.def_property_readonly( "MeanX", &dip::CovarianceAccumulator::MeanX );
   covAcc.def_property_readonly( "MeanY", &dip::CovarianceAccumulator::MeanY );
   covAcc.def_property_readonly( "VarianceX", &dip::CovarianceAccumulator::VarianceX );
   covAcc.def_property_readonly( "VarianceY", &dip::CovarianceAccumulator::VarianceY );
   covAcc.def_property_readonly( "StandardDeviationX", &dip::CovarianceAccumulator::StandardDeviationX );
   covAcc.def_property_readonly( "StandardDeviationY", &dip::CovarianceAccumulator::StandardDeviationY );
   covAcc.def_property_readonly( "Covariance", &dip::CovarianceAccumulator::Covariance );
   covAcc.def_property_readonly( "Correlation", &dip::CovarianceAccumulator::Correlation );
   covAcc.def_property_readonly( "Slope", &dip::CovarianceAccumulator::Slope );

   // dip::MomentAccumulator
   auto momentAcc = py::class_< dip::MomentAccumulator >( m, "MomentValues", "Moment values." );
   momentAcc.def( "__repr__", []( dip::MomentAccumulator const& s ) {
                     std::ostringstream os;
                     os << "<MomentValues: ";
                     os << "zerothOrder=" << s.Sum();
                     os << ", firstOrder=" << s.FirstOrder();
                     os << ", secondOrder=" << s.SecondOrder();
                     os << ", plainSecondOrder=" << s.PlainSecondOrder();
                     os << '>';
                     return os.str();
                  } );
   momentAcc.def_property_readonly( "zerothOrder", &dip::MomentAccumulator::Sum );
   momentAcc.def_property_readonly( "firstOrder", &dip::MomentAccumulator::FirstOrder );
   momentAcc.def_property_readonly( "secondOrder", &dip::MomentAccumulator::SecondOrder );
   momentAcc.def_property_readonly( "plainSecondOrder", &dip::MomentAccumulator::PlainSecondOrder );

   // dip::SpatialOverlapMetrics
   auto overlap = py::class_< dip::SpatialOverlapMetrics >( m, "SpatialOverlapMetrics", "Metrics describing spatial overlap." );
   overlap.def( "__repr__", []( dip::SpatialOverlapMetrics const& s ) {
                   std::ostringstream os;
                   os << "<SpatialOverlapMetrics: ";
                   os << "truePositives=" << s.truePositives;
                   os << ", trueNegatives=" << s.trueNegatives;
                   os << ", falsePositives=" << s.falsePositives;
                   os << ", falseNegatives=" << s.falseNegatives;
                   os << ", diceCoefficient=" << s.diceCoefficient;
                   os << ", jaccardIndex=" << s.jaccardIndex;
                   os << ", sensitivity=" << s.sensitivity;
                   os << ", specificity=" << s.specificity;
                   os << ", fallout=" << s.fallout;
                   os << ", accuracy=" << s.accuracy;
                   os << ", precision=" << s.precision;
                   os << '>';
                   return os.str();
                } );
   overlap.def_readonly( "truePositives", &dip::SpatialOverlapMetrics::truePositives );
   overlap.def_readonly( "trueNegatives", &dip::SpatialOverlapMetrics::trueNegatives );
   overlap.def_readonly( "falsePositives", &dip::SpatialOverlapMetrics::falsePositives );
   overlap.def_readonly( "falseNegatives", &dip::SpatialOverlapMetrics::falseNegatives );
   overlap.def_readonly( "diceCoefficient", &dip::SpatialOverlapMetrics::diceCoefficient );
   overlap.def_readonly( "jaccardIndex", &dip::SpatialOverlapMetrics::jaccardIndex );
   overlap.def_readonly( "sensitivity", &dip::SpatialOverlapMetrics::sensitivity );
   overlap.def_readonly( "specificity", &dip::SpatialOverlapMetrics::specificity );
   overlap.def_readonly( "fallout", &dip::SpatialOverlapMetrics::fallout );
   overlap.def_readonly( "accuracy", &dip::SpatialOverlapMetrics::accuracy );
   overlap.def_readonly( "precision", &dip::SpatialOverlapMetrics::precision );

}
