/*
 * (c)2017-2022, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "pydip.h"
#include "accumulators.h" // IWYU pragma: keep
#include "diplib/statistics.h"


namespace pybind11 {
namespace detail {

DIP_OUTPUT_TYPE_CASTER( SpatialOverlapMetrics, "SpatialOverlapMetrics", "truePositives trueNegatives falsePositives falseNegatives diceCoefficient jaccardIndex sensitivity specificity fallout accuracy precision", src.truePositives, src.trueNegatives, src.falsePositives, src.falseNegatives, src.diceCoefficient, src.jaccardIndex, src.sensitivity, src.specificity, src.fallout, src.accuracy, src.precision )

} // namespace detail
} // namespace pybind11


void init_statistics( py::module& m ) {

   m.def( "Count", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Count ), "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·Count·Image·CL·Image·CL );
   m.def( "MaximumPixel", &dip::MaximumPixel, "in"_a, "mask"_a = dip::Image{}, "positionFlag"_a = dip::S::FIRST, doc_strings::dip·MaximumPixel·Image·CL·Image·CL·String·CL );
   m.def( "MinimumPixel", &dip::MinimumPixel, "in"_a, "mask"_a = dip::Image{}, "positionFlag"_a = dip::S::FIRST, doc_strings::dip·MinimumPixel·Image·CL·Image·CL·String·CL );
   m.def( "CumulativeSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::CumulativeSum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·CumulativeSum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "CumulativeSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::CumulativeSum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·CumulativeSum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MaximumAndMinimum", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::MaximumAndMinimum ),
          "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·MaximumAndMinimum·Image·CL·Image·CL );
   m.def( "Quartiles", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::Quartiles ), "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·Quartiles·Image·CL·Image·CL );
   m.def( "SampleStatistics", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::SampleStatistics ),
          "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·SampleStatistics·Image·CL·Image·CL );
   m.def( "Covariance", &dip::Covariance, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·Covariance·Image·CL·Image·CL·Image·CL );
   m.def( "PearsonCorrelation", &dip::PearsonCorrelation, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·PearsonCorrelation·Image·CL·Image·CL·Image·CL );
   m.def( "SpearmanRankCorrelation", &dip::SpearmanRankCorrelation, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·SpearmanRankCorrelation·Image·CL·Image·CL·Image·CL );
   m.def( "CenterOfMass", &dip::CenterOfMass, "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·CenterOfMass·Image·CL·Image·CL );
   m.def( "Moments", &dip::Moments, "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·Moments·Image·CL·Image·CL );

   m.def( "Mean", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::Mean ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = "", "process"_a = dip::BooleanArray{}, doc_strings::dip·Mean·Image·CL·Image·CL·Image·L·String·CL·BooleanArray·CL );
   m.def( "Mean", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String const&, dip::BooleanArray const& >( &dip::Mean ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "mode"_a = "", "process"_a = dip::BooleanArray{}, doc_strings::dip·Mean·Image·CL·Image·CL·Image·L·String·CL·BooleanArray·CL );
   m.def( "Sum", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Sum ), // Fails to resolve!
          static_cast< dip::Image( * )( dip::Image const&, dip::Image const&, dip::BooleanArray const& ) >( &dip::Sum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Sum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Sum", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Sum ), // Fails to resolve!
          static_cast< void( * )( dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& ) >( &dip::Sum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Sum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "GeometricMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::GeometricMean ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·GeometricMean·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "GeometricMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::GeometricMean ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·GeometricMean·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Product", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Product ), // Fails to resolve!
          static_cast< dip::Image( * )( dip::Image const&, dip::Image const&, dip::BooleanArray const& ) >( &dip::Product ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Product·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Product", //py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Product ), // Fails to resolve!
          static_cast< void( * )( dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& ) >( &dip::Product ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Product·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MeanAbs ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MeanModulus ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::SumAbs ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::SumModulus ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanSquare ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanSquare·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MeanSquare ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanSquare·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumSquare ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumSquare·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumSquare", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::SumSquare ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumSquare·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MeanSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanSquareModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MeanSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MeanSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MeanSquareModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::SumSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumSquareModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "SumSquareModulus", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::SumSquareModulus ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·SumSquareModulus·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Variance", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::Variance ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{}, doc_strings::dip·Variance·Image·CL·Image·CL·Image·L·String··BooleanArray·CL );
   m.def( "Variance", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String, dip::BooleanArray const& >( &dip::Variance ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{}, doc_strings::dip·Variance·Image·CL·Image·CL·Image·L·String··BooleanArray·CL );
   m.def( "StandardDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::String const&, dip::BooleanArray const& >( &dip::StandardDeviation ),
          "in"_a, "mask"_a = dip::Image{}, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{}, doc_strings::dip·StandardDeviation·Image·CL·Image·CL·Image·L·String··BooleanArray·CL );
   m.def( "StandardDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::String, dip::BooleanArray const& >( &dip::StandardDeviation ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "mode"_a = dip::S::FAST, "process"_a = dip::BooleanArray{}, doc_strings::dip·StandardDeviation·Image·CL·Image·CL·Image·L·String··BooleanArray·CL );
   m.def( "Maximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Maximum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Maximum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Maximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::Maximum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Maximum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Minimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Minimum ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Minimum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Minimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::Minimum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Minimum·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MaximumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MaximumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MaximumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MaximumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MaximumAbs ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MaximumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MinimumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MinimumAbs ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MinimumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MinimumAbs", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MinimumAbs ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MinimumAbs·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Percentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::BooleanArray const& >( &dip::Percentile ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = 50.0, "process"_a = dip::BooleanArray{}, doc_strings::dip·Percentile·Image·CL·Image·CL·Image·L·dfloat··BooleanArray·CL );
   m.def( "Percentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::BooleanArray const& >( &dip::Percentile ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "percentile"_a = 50.0, "process"_a = dip::BooleanArray{}, doc_strings::dip·Percentile·Image·CL·Image·CL·Image·L·dfloat··BooleanArray·CL );
   m.def( "Median", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Median ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Median·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Median", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::Median ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Median·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MedianAbsoluteDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::MedianAbsoluteDeviation ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·MedianAbsoluteDeviation·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "MedianAbsoluteDeviation", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::MedianAbsoluteDeviation ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·MedianAbsoluteDeviation·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "All", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::All ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·All·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "All", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::All ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·All·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Any", py::overload_cast< dip::Image const&, dip::Image const&, dip::BooleanArray const& >( &dip::Any ),
          "in"_a, "mask"_a = dip::Image{}, "process"_a = dip::BooleanArray{}, doc_strings::dip·Any·Image·CL·Image·CL·Image·L·BooleanArray·CL );
   m.def( "Any", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::BooleanArray const& >( &dip::Any ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "process"_a = dip::BooleanArray{}, doc_strings::dip·Any·Image·CL·Image·CL·Image·L·BooleanArray·CL );

   m.def( "PositionMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMaximum ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMaximum·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "PositionMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::PositionMaximum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMaximum·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "PositionMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMinimum ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMinimum·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "PositionMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::PositionMinimum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMinimum·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "PositionPercentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::uint, dip::String const& >( &dip::PositionPercentile ),
          "in"_a, "mask"_a = dip::Image{}, "percentile"_a = 50, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionPercentile·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "PositionPercentile", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::uint, dip::String const& >( &dip::PositionPercentile ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "percentile"_a = 50, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionPercentile·Image·CL·Image·CL·Image·L·dfloat··dip·uint··String·CL );
   m.def( "PositionMedian", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint, dip::String const& >( &dip::PositionMedian ),
          "in"_a, "mask"_a = dip::Image{}, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMedian·Image·CL·Image·CL·Image·L·dip·uint··String·CL );
   m.def( "PositionMedian", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::uint, dip::String const& >( &dip::PositionMedian ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "dim"_a = 0, "mode"_a = dip::S::FIRST, doc_strings::dip·PositionMedian·Image·CL·Image·CL·Image·L·dip·uint··String·CL );

   m.def( "RadialSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialSum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialSum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialSum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialSum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialSum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMean ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMean·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMean", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMean ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMean·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMinimum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMinimum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMinimum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMinimum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMinimum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMaximum ),
          "in"_a, "mask"_a = dip::Image{}, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMaximum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );
   m.def( "RadialMaximum", py::overload_cast< dip::Image const&, dip::Image const&, dip::Image&, dip::dfloat, dip::String const&, dip::FloatArray const& >( &dip::RadialMaximum ),
          "in"_a, "mask"_a = dip::Image{}, py::kw_only(), "out"_a, "binSize"_a = 1, "maxRadius"_a = dip::S::OUTERRADIUS, "center"_a = dip::FloatArray{}, doc_strings::dip·RadialMaximum·Image·CL·Image·CL·Image·L·dfloat··String·CL·FloatArray·CL );

   m.def( "MeanError", &dip::MeanError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MeanError·Image·CL·Image·CL·Image·CL );
   m.def( "MeanSquareError", &dip::MeanSquareError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MeanSquareError·Image·CL·Image·CL·Image·CL );
   m.def( "RootMeanSquareError", &dip::RootMeanSquareError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·RootMeanSquareError·Image·CL·Image·CL·Image·CL );
   m.def( "MeanAbsoluteError", &dip::MeanAbsoluteError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MeanAbsoluteError·Image·CL·Image·CL·Image·CL );
   m.def( "MaximumAbsoluteError", &dip::MaximumAbsoluteError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MaximumAbsoluteError·Image·CL·Image·CL·Image·CL );
   m.def( "MeanRelativeError", &dip::MeanRelativeError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MeanRelativeError·Image·CL·Image·CL·Image·CL );
   m.def( "MaximumRelativeError", &dip::MaximumRelativeError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·MaximumRelativeError·Image·CL·Image·CL·Image·CL );
   m.def( "IDivergence", &dip::IDivergence, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·IDivergence·Image·CL·Image·CL·Image·CL );
   m.def( "InProduct", &dip::InProduct, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, doc_strings::dip·InProduct·Image·CL·Image·CL·Image·CL );
   m.def( "LnNormError", &dip::LnNormError, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "order"_a = 2.0, doc_strings::dip·LnNormError·Image·CL·Image·CL·Image·CL·dfloat· );
   m.def( "PSNR", &dip::PSNR, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "peakSignal"_a = 0.0, doc_strings::dip·PSNR·Image·CL·Image·CL·Image·CL·dfloat· );
   m.def( "SSIM", &dip::SSIM, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "sigma"_a = 1.5, "K1"_a = 0.01, "K2"_a = 0.03, doc_strings::dip·SSIM·Image·CL·Image·CL·Image·CL·dfloat··dfloat··dfloat· );
   m.def( "MutualInformation", &dip::MutualInformation, "in1"_a, "in2"_a, "mask"_a = dip::Image{}, "nBins"_a = 256, doc_strings::dip·MutualInformation·Image·CL·Image·CL·Image·CL·dip·uint· );

   m.def( "SpatialOverlap", &dip::SpatialOverlap, "in"_a, "reference"_a, doc_strings::dip·SpatialOverlap·Image·CL·Image·CL );
   m.def( "DiceCoefficient", &dip::DiceCoefficient, "in"_a, "reference"_a, doc_strings::dip·DiceCoefficient·Image·CL·Image·CL );
   m.def( "JaccardIndex", &dip::JaccardIndex, "in"_a, "reference"_a, doc_strings::dip·JaccardIndex·Image·CL·Image·CL );
   m.def( "Specificity", &dip::Specificity, "in"_a, "reference"_a, doc_strings::dip·Specificity·Image·CL·Image·CL );
   m.def( "Sensitivity", &dip::Sensitivity, "in"_a, "reference"_a, doc_strings::dip·Sensitivity·Image·CL·Image·CL );
   m.def( "Accuracy", &dip::Accuracy, "in"_a, "reference"_a, doc_strings::dip·Accuracy·Image·CL·Image·CL );
   m.def( "Precision", &dip::Precision, "in"_a, "reference"_a, doc_strings::dip·Precision·Image·CL·Image·CL );
   m.def( "HausdorffDistance", &dip::HausdorffDistance, "in"_a, "reference"_a, doc_strings::dip·HausdorffDistance·Image·CL·Image·CL );
   m.def( "ModifiedHausdorffDistance", &dip::ModifiedHausdorffDistance, "in"_a, "reference"_a, doc_strings::dip·ModifiedHausdorffDistance·Image·CL·Image·CL );
   m.def( "SumOfMinimalDistances", &dip::SumOfMinimalDistances, "in"_a, "reference"_a, doc_strings::dip·SumOfMinimalDistances·Image·CL·Image·CL );
   m.def( "ComplementWeightedSumOfMinimalDistances", &dip::ComplementWeightedSumOfMinimalDistances, "in"_a, "reference"_a, doc_strings::dip·ComplementWeightedSumOfMinimalDistances·Image·CL·Image·CL );

   m.def( "Entropy", py::overload_cast< dip::Image const&, dip::Image const&, dip::uint >( &dip::Entropy ),
          "in"_a, "mask"_a = dip::Image{}, "nBins"_a = 256, doc_strings::dip·Entropy·Image·CL·Image·CL·dip·uint· );
   m.def( "EstimateNoiseVariance", py::overload_cast< dip::Image const&, dip::Image const& >( &dip::EstimateNoiseVariance ),
          "in"_a, "mask"_a = dip::Image{}, doc_strings::dip·EstimateNoiseVariance·Image·CL·Image·CL );

}
