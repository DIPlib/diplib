/*
 * DIPlib 3.0
 * This file contains declarations for non-linear image filters
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#ifndef DIP_NONLINEAR_H
#define DIP_NONLINEAR_H

#include "diplib.h"


/// \file
/// \brief Declares functions that implement non-linear filters.


namespace dip {


/// \defgroup nonlinear Non-linear filters
/// \ingroup filtering
/// \brief Non-linear filters for noise reduction, detection, etc., excluding morphological filters.
/// \{

// TODO: functions to port:
/*
   dip_PercentileFilter (dip_rankfilter.h)
   dip_MedianFilter (dip_rankfilter.h)
   dip_RankContrastFilter (dip_rankfilters.h)
   dip_VarianceFilter (dip_filtering.h)
   dip_Kuwahara (dip_filtering.h)
   dip_GeneralisedKuwahara (dip_filtering.h)
   dip_KuwaharaImproved (dip_filtering.h) (merge into dip_Kuwahara)
   dip_GeneralisedKuwaharaImproved (dip_filtering.h) (merge into dip_GeneralisedKuwahara)
   dip_Sigma (dip_filtering.h)
   dip_BiasedSigma (dip_filtering.h)
   dip_GaussianSigma (dip_filtering.h)
   dip_NonMaximumSuppression (dip_filtering.h)
   dip_ArcFilter (dip_bilateral.h)
   dip_Bilateral (dip_bilateral.h) (all three flavours into one function)
   dip_BilateralFilter (dip_bilateral.h) (all three flavours into one function)
   dip_QuantizedBilateralFilter (dip_bilateral.h) (all three flavours into one function)
   dip_AdaptiveGauss (dip_adaptive.h)
   dip_AdaptiveBanana (dip_adaptive.h)
   dip_StructureAdaptiveGauss (dip_adaptive.h)
   dip_AdaptivePercentile (dip_adaptive.h)
   dip_AdaptivePercentileBanana (dip_adaptive.h)
   dip_PGST3DLine (dip_pgst.h) (this could have a better name!)
   dip_PGST3DSurface (dip_pgst.h) (this could have a better name!)
*/

/// \}

} // namespace dip

#endif // DIP_NONLINEAR_H
