/*
 * (c)2024, Cris Luengo.
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

#ifndef DIP_PYDIP_ACCUMULATORS_H
#define DIP_PYDIP_ACCUMULATORS_H

#include "pydip.h"
#include "diplib/accumulators.h"

namespace pybind11 {
namespace detail {

DIP_OUTPUT_TYPE_CASTER( CovarianceAccumulator, "CovarianceValues", "Number MeanX MeanY VarianceX VarianceY StandardDeviationX StandardDeviationY Covariance Correlation Slope", src.Number(), src.MeanX(), src.MeanY(), src.VarianceX(), src.VarianceY(), src.StandardDeviationX(), src.StandardDeviationY(), src.Covariance(), src.Correlation(), src.Slope() )
DIP_OUTPUT_TYPE_CASTER( MomentAccumulator, "MomentValues", "zerothOrder firstOrder secondOrder plainSecondOrder", src.Sum(), src.FirstOrder(), src.SecondOrder(), src.PlainSecondOrder() )
DIP_OUTPUT_TYPE_CASTER( QuartilesResult, "QuartilesResult", "minimum lowerQuartile median upperQuartile maximum", src.minimum, src.lowerQuartile, src.median, src.upperQuartile, src.maximum )
DIP_OUTPUT_TYPE_CASTER( StatisticsAccumulator, "StatisticsValues", "mean standardDev variance skewness kurtosis number", src.Mean(), src.StandardDeviation(), src.Variance(), src.Skewness(), src.ExcessKurtosis(), src.Number() )
DIP_OUTPUT_TYPE_CASTER( MinMaxAccumulator, "MinMaxValues", "minimum maximum", src.Minimum(), src.Maximum() )

} // namespace detail
} // namespace pybind11

#endif //DIP_PYDIP_ACCUMULATORS_H
