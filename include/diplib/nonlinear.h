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


/// \brief Applies a percentile filter to `in`.
///
/// Determines the `percentile` percentile within the filter window, and assigns that value to the output pixel.
/// See also `dip::RankFilter`, which does the same thing but uses a rank instead of a percentile as input argument.
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

/// \brief The median filter, a non-linear smoothing filter.
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


/// \brief Selects, for each pixel, a value from within the filter window, where a control image is minimal or maximal.
///
/// For each pixel, within the filter window, looks for the pixel with the lowest value (`mode` is `"minimum"`) or
/// highest value (`mode` is `"maximum"`), and takes the value from `in` at that location as the output value. To
/// prevent a stair-case effect in the output, where many pixels use the same input value, a `threshold` can be
/// specified. If it is a positive value, then the lowest (or highest) value found must be `threshold` lower (or
/// higher) than the central pixel, otherwise the central pixel is used.
///
/// Ties are solved by picking the value closest to the central pixel. Multiple control pixels with the same value
/// and at the same distance to the central pixel are solved arbitrarily (in the current implementation, the first
/// of these pixels encountered is used).
///
/// The Kuwahara-Nagao operator (see `dip::Kuwahara`) is implemented in terms of the `%SelectionFilter`:
///
/// ```cpp
///     Image value = dip::Uniform( in, kernel );
///     Image control = dip::VarianceFilter( in, kernel );
///     kernel.Mirror();
///     Image out = dip::SelectionFilter( value, control, kernel );
/// ```
///
/// Note that the following reproduces the result of the erosion (albeit in a very costly manner):
///
/// ```cpp
///     Image out = dip::SelectionFilter( in, in, kernel );
/// ```
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// `control` must be a real-valued scalar image. `in` can be of any data type and tensor size. `out` will be of
/// the same size, tensor size, and data type as `in`.
DIP_EXPORT void SelectionFilter(
      Image const& in,
      Image const& control,
      Image& out,
      Kernel const& kernel = {},
      dfloat threshold = 0.0,
      String const& mode = "minimum",
      StringArray const& boundaryCondition = {}
);
inline Image SelectionFilter(
      Image const& in,
      Image const& control,
      Kernel const& kernel = {},
      dfloat threshold = 0.0,
      String const& mode = "minimum",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   SelectionFilter( in, control, out, kernel, threshold, mode, boundaryCondition );
   return out;
}

/// \brief The Kuwahara-Nagao operator, a non-linear edge-preserving smoothing filter.
///
/// For each pixel, shifts the filtering window such that the variance within the window is minimal, then
/// computes the average value as the output. The shift of the window is always such that the pixel under
/// consideration stays within the window.
///
/// In the two original papers describing the method (Kuwahara et al., 1980; Nagao and Matsuyama, 1979), a limited
/// number of sub-windows within the filtering window were examined (4 and 8, respectively). This function implements
/// a generalized version that allows as many different shifts are pixels are in the filtering window (Bakker et al.,
/// 1999).
///
/// As described by Bakker (2002), this operator produces artificial boundaries in flat regions. This is because,
/// due to noise, one position of the filtering window will have the lowest variance in its neighborhood, and therefore
/// that position will be selected for all output pixels in the neighborhood. The solution we implement here is
/// requiring that the variance at the minimum be lower than the variance when the window is not shifted. The parameter
/// `threshold` controls how much lower the minimum must be. If the neighborhood is uniform w.r.t. this threhsold
/// parameter, then the filtering window is not shifted.
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// **Literature**
/// - M. Kuwahara, K. Hachimura and M. Kinoshita, "Image Enhancement and Left Ventricular Contour Extraction Techniques
///   Applied to Radioisotope Angiocardiograms", Automedica 3:107-119, 1980.
/// - M. Nagao and T. Matsuyama, "Edge Preserving Smoothing", Computer Graphics and Image Processing 9:394-407, 1979.
/// - P. Bakker, P.W. Verbeek and L.J. van Vliet, "Edge preserving orientation adaptive filtering", in: CVPR’99 2:535–540, 1999.
/// - P. Bakker, "Image structure analysis for seismic interpretation", PhD Thesis, Delft University of Technology,
///   The Netherlands, 2002.
///
/// \see dip::SelectionFilter.
DIP_EXPORT void Kuwahara(
      Image const& in,
      Image& out,
      Kernel kernel = {},
      dfloat threshold = 0.0,
      StringArray const& boundaryCondition = {}
);
inline Image Kuwahara(
      Image const& in,
      Kernel const& kernel = {},
      dfloat threshold = 0.0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Kuwahara( in, out, kernel, threshold, boundaryCondition );
   return out;
}


// TODO: functions to port:
/*
   dip_RankContrastFilter (dip_rankfilters.h)
   dip_Sigma (dip_filtering.h)
   dip_BiasedSigma (dip_filtering.h)
   dip_GaussianSigma (dip_filtering.h) (compare dip_BilateralFilter)
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
