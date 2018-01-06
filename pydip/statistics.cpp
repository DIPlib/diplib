/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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
          }, "in"_a, "mask"_a = dip::Image{} );
   m.def( "SampleStatistics", []( dip::Image const& in, dip::Image const& mask ) {
                dip::StatisticsAccumulator acc = dip::SampleStatistics( in, mask );
                return py::make_tuple( acc.Mean(), acc.Variance(), acc.Skewness(), acc.ExcessKurtosis() ).release();
          }, "in"_a, "mask"_a = dip::Image{} );
   m.def( "Covariance", []( dip::Image const& in, dip::Image const& mask ) {
                dip::CovarianceAccumulator acc = dip::Covariance( in, mask );
                return py::make_tuple( acc.Covariance(), acc.Correlation() ).release();
          }, "in"_a, "mask"_a = dip::Image{} );
   m.def( "CenterOfMass", &dip::CenterOfMass, "in"_a, "mask"_a = dip::Image{} );
   m.def( "Moments", []( dip::Image const& in, dip::Image const& mask ) {
             dip::MomentAccumulator acc = dip::Moments( in, mask );
             return py::make_tuple( acc.Sum(), acc.FirstOrder(), acc.SecondOrder() ).release();
          }, "in"_a, "mask"_a = dip::Image{} );
   m.def( "Mean", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::Mean ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = "", "process"_a = dip::BooleanArray{} );
   m.def( "Sum", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Sum ), // Fails to resolve!
          static_cast< dip::Image( * )( dip::Image const&, dip::Image const&, dip::BooleanArray const& ) >( &dip::Sum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Product", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Product ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "MeanSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanSquare ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "SumSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumSquare ),
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
   m.def( "All", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::All ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
   m.def( "Any", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Any ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{} );
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
   m.def( "Entropy", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Entropy ),
          "in"_a, "mask"_a = dip::Image{}, "nBins"_a = 256 );
   m.def( "EstimateNoiseVariance", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::EstimateNoiseVariance ),
          "in"_a, "mask"_a = dip::Image{} );
}
