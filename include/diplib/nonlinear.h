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
#include "diplib/kernel.h"


/// \file
/// \brief Declares functions that implement non-linear filters.


namespace dip {


/// \defgroup nonlinear Non-linear filters
/// \ingroup filtering
/// \brief Non-linear filters for noise reduction, detection, etc., excluding morphological filters.
/// \{


/// \brief Applies a percentile filter (A.K.A. rank filter) to `in`.
///
/// Determines the `percentile` percentile within the filter window, and assigns that value to the output pixel.
/// To find out what percentile corresponds to a specific rank, it is necessary to know the number of pixels within
/// the neighborhood. (TODO: add such a function to `dip::Kernel`.)
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
DIP_EXPORT void PercentileFilter(
      Image const& in,
      Image& out,
      dfloat percentile,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
);
inline Image PercentileFilter(
      Image const& in,
      dfloat percentile,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   PercentileFilter( in, out, percentile, kernel, boundaryCondition );
   return out;
}

/// \brief Applies a median filter to `in`.
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// Calls `dip::PercentileFilter` with the `percentile` parameter set to 50.
inline void MedianFilter(
      Image const& in,
      Image& out,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
) {
   PercentileFilter( in, out, 50.0, kernel, boundaryCondition );
}
inline Image MedianFilter(
      Image const& in,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MedianFilter( in, out, kernel, boundaryCondition );
   return out;
}

/// \brief Computes, for each pixel, the sample variance within a filter window around the pixel.
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// Uses `dip::VarianceAccumulator` for the computation.
DIP_EXPORT void VarianceFilter(
      Image const& in,
      Image& out,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
);
inline Image VarianceFilter(
      Image const& in,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   VarianceFilter( in, out, kernel, boundaryCondition );
   return out;
}


// TODO: functions to port:
/*
   dip_RankContrastFilter (dip_rankfilters.h)
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
