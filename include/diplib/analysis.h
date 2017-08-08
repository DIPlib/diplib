/*
 * DIPlib 3.0
 * This file contains declarations for assorted analysis functions
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

#ifndef DIP_ANALYSIS_H
#define DIP_ANALYSIS_H

#include "diplib.h"


/// \file
/// \brief Declares functions for assorted analysis functions.


namespace dip {


/// \defgroup analysis Analysis
/// \brief Assorted analysis tools.
/// \{

/// \brief Contains the result of the function `dip::SubpixelLocation`.
struct SubpixelLocationResult {
   FloatArray coordinates;
   dfloat value;
};

/// \brief Contains the result of the functions `dip::SubpixelMaxima` and `dip::SubpixelMinima`.
using SubpixelLocationArray = std::vector< SubpixelLocationResult >;

/// \brief Gets coordinates of a local extremum with sub-pixel precision
///
/// Determines the sub-pixel location of a local maximum or minimum close to `position`. `position` should point
/// to a pixel whose value is larger than its direct neighbors' (if `polarity` is `"maximum"`) or smaller than
/// its direct neighbors' (`polarity` is `"minimum"`).
///
/// The `coordinates` member of the output struct will contain the the sub-pixel location of this local
/// extremum. The `value` member will contain the interpolated grey value at the location of the extremum.
///
/// `method` determines which method is used. These are the allowed values:
///  - `"linear"`: Computes the center of gravity of 3 pixels around the extremum, in each dimension independently.
///    The value at the extremum is that of the pixel at `position`.
///  - `"parabolic"`: Fits a parabola to a 3-pixel-wide block around the extremum (for up to 3D only).
///  - `"parabolic separable"`: Fits a parabola in each dimension independently. The value at the extremum
///    is the maximum/minimum of the values obtained in each of the 1D processes, and thus not equivalent
///    to the grey value obtained by true interpolation.
///  - `"gaussian"`: Fits a Gaussian to a 3-pixel-wide block around the extremum (for up to 3D only).
///  - `"gaussian separable"`: Fits a Gaussian in each dimension independently. The value at the extremum
///    is the maximum/minimum of the values obtained in each of the 1D processes, and thus not equivalent
///    to the grey value obtained by true interpolation.
///
/// The image `in` must be scalar and real-valued.
DIP_EXPORT SubpixelLocationResult SubpixelLocation(
      Image const& in,
      UnsignedArray const& position,
      String const& polarity = "maximum",
      String const& method = "parabolic separable"
);

/// \brief Gets coordinates of local maxima with sub-pixel precision
///
/// Detects local maxima in the image, and returns their coordinates, with sub-pixel precision.
/// Only pixels where `mask` is on will be examined. Local maxima are detected using `dip::Maxima`,
/// then their position is determined more precisely using `dip::SubpixelLocation`.
///
/// A local maximum can not touch the edge of the image. That is, its integer location must be one
/// pixel away from the edge.
///
/// See `dip::SubpixelLocation` for the definition of the `method` parameter.
DIP_EXPORT SubpixelLocationArray SubpixelMaxima(
      Image const& in,
      Image const& mask = {},
      String const& method = "parabolic separable"
);

/// \brief Gets coordinates of local minima with sub-pixel precision
///
/// Detects local minima in the image, and returns their coordinates, with sub-pixel precision.
/// Only pixels where `mask` is on will be examined. Local minima are detected using `dip::Minima`,
/// then their position is determined more precisely using `dip::SubpixelLocation`.
///
/// A local minimum can not touch the edge of the image. That is, its integer location must be one
/// pixel away from the edge.
///
/// See `dip::SubpixelLocation` for the definition of the `method` parameter.
DIP_EXPORT SubpixelLocationArray SubpixelMinima(
      Image const& in,
      Image const& mask = {},
      String const& method = "parabolic separable"
);

/// \brief Calculates the cross-correlation between two images of equal size.
///
/// The returned image is the cross-correlation normalized in such a way that only
/// the phase information is of importance. The computation performed is
/// `out = (%dip::Conjugate(in1)*in2)/%dip::SquareModulus(in1)` in the Fourier Domain.
/// This results as a very sharp peak in the spatial domain. If `normalize` is set
/// to `"don't normalize"`, the regular cross-correlation (not normalized by the square
/// modulus) is returned.
///
/// As elsewhere, the origin is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image. Thus, for `in1==in2`, only this pixel will be set.
///
/// If `in1` or `in2` is already Fourier transformed, set `in1Representation` or `in2Representation`
/// to `"frequency"`. Similarly, if `outRepresentation` is `"frequency"`, the output will not be
/// inverse-transformed, so will be in the frequency domain.
///
/// `in1` and `in2` must be scalar images with the same dimensionality and sizes. `out` will be real-valued
/// if `outRepresentation` is `"spatial"`.
DIP_EXPORT void CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& in1Representation = "spatial",
      String const& in2Representation = "spatial",
      String const& outRepresentation = "spatial",
      String const& normalize = "normalize"
);
inline Image CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      String const& in1Representation = "spatial",
      String const& in2Representation = "spatial",
      String const& outRepresentation = "spatial",
      String const& normalize = "normalize"
) {
   Image out;
   CrossCorrelationFT( in1, in2, out, in1Representation, in2Representation, outRepresentation, normalize );
   return out;
}

/// \brief Estimates the (sub-pixel) global shift between `in1` and `in2`.
///
/// The numbers found represent the shift of `in2` with respect to `in1`, or equivalently, the position of the
/// top-left pixel of `in2` in the coordinate system of `in1`. Shifting `in1` by the returned shift aligns the
/// two images. See `dip::Shift`.
///
/// There are various methods that can be used to determine the shift, see below.
/// For the methods that require that the shift be small, first the integer pixel is calculated using cross
/// correlation, and both images are cropped to the common part. `maxShift` can be used to restrict the shift
/// found. This is useful, for example, when the images contain a repeating pattern. Noise in the images can
/// cause a shift of several pattern periods to be "optimal" (e.g. the cross-correlation yields a larger value),
/// even if a much smaller shift would also explain the differences. In this case, set `maxShift` to be slightly
/// smaller than the pattern period.
///
/// Valid strings for `method` are:
///
///  - `"integer only"`: The cross-correlation method simply computes the cross-correlation, using
///    `dip::CrossCorrelationFT`, and then finds the position of the pixel with the largest value.
///    This method works for any number of dimensions. `parameter` is ignored.
///
///  - `"CC"`: The cross-correlation method computes the cross-correlation, using `dip::CrossCorrelationFT`,
///    and then uses `dip::SubpixelLocation` to find the location of the largest peak with sub-pixel precision.
///    This method works for any number of dimensions. `parameter` is ignored.
///
///  - `"NCC"`: As `"CC"`, but using the normalized cross-correlation, which makes the peak much sharper
///    (Luengo Hendriks, 1998). This method works for any number of dimensions. `parameter` is ignored.
///
///  - `"CPF"`: The CPF method (see Luengo Hendriks (1998), where it is called FFTS) uses the phase of the
///    cross-correlation (as calculated by `dip::CrossCorrelationFT`) to estimate the shift. `parameter` sets
///    the largest frequency used in this estimation. The maximum value that makes sense is `sqrt(0.5)`.
///    Any larger value will give the same result. Choose smaller values to ignore the higher frequencies, which
///    have a smaller SNR and are more affected by aliasing. If `parameter` is <= 0, the optimal found for images
///    sub-sampled by a factor four will be used (parameter = 0.2).
///    This method only supports 2-D images.
///
///  - `"MTS"`: The MTS method (see Luengo Hendriks (1998), where it is called GRS) uses a first order Taylor
///    approximation of the equation `in1(t) = in2(t-s)` at scale `parameter`. If `parameter` is <= 0, a scale
///    of 1 will be used. This means that the images will be smoothed with a Gaussian kernel of 1. This method is
///    more accurate than CPF.
///    This method supports images with a dimensionality between 1 and 3.
///
///  - `"ITER"`: The ITER method is an iterative version of the MTS method. It is known that a single estimation
///    with MTS has a bias due to truncation of the Taylor expansion series (Pham et al., 2005). The bias can be
///    expressed as a polynomial of the subpixel displacements. As a result, if the MTS method is applied iteratively,
///    and the shift is refined after each iteration, the bias eventually becomes negligible. By using just 3
///    iterations, and noticing that log(bias_increment) is a linear sequence,  it is possible to correct for the
///    bias up to O(1e-6).<br>
///    Set `parameter` to 0 for normal behavior. `parameter` in the range (0,0.1] specifies the desired accuracy.
///    A negative `parameter` causes `round(-parameter)` iterations to be run.
///    This method supports images with a dimensionality between 1 and 3.
///
///  - `"PROJ"`: The PROJ method computes the shift in each dimension separately, applying the ITER method on the
///    projection of the image onto each axis. It is fast and fairly accurate for high SNR. Should not be used for
///    low SNR. `parameter` is passed unchanged to the ITER method. This method supports images with any number of
///    dimensions.
///
/// **Literature**:
///  - C.L. Luengo Hendriks, "Improved Resolution in Infrared Imaging Using Randomly Shifted Images", M.Sc. Thesis,
///    Delft University of Technology, The Netherlands, 1998.
///  - T.Q. Pham, M. Bezuijen, L.J. van Vliet, K. Schutte and C.L. Luengo Hendriks, "Performance of Optimal
///    Registration Estimators", In: Visual Information Processing XIV, Proceedings of SPIE 5817, 2005.
DIP_EXPORT FloatArray FindShift(
      Image const& in1,
      Image const& in2,
      String const& method = "MTS",
      dfloat parameter = 0,
      dip::uint maxShift = std::numeric_limits< dip::uint >::max()
);

// TODO: functions to port:
/*
   dip_PairCorrelation (dip_analysis.h)
   dip_ProbabilisticPairCorrelation (dip_analysis.h)
   dip_ChordLength (dip_analysis.h)
   dip_RadialDistribution (dip_analysis.h)

   dip_StructureAnalysis (dip_analysis.h)

   dip_OrientationSpace (dip_structure.h)
   dip_ExtendedOrientationSpace (dip_structure.h)

   dip_StructureTensor2D (dip_structure.h) (trivial using existing functionality, generalize to nD)
   dip_StructureTensor3D (dip_structure.h) (see dip_StructureTensor2D)
   dip_CurvatureFromTilt (dip_structure.h)

   dip_OSEmphasizeLinearStructures (dip_structure.h)
   dip_DanielsonLineDetector (dip_structure.h)
   dip_Canny (dip_detection.h) (or in diplib/segmentation.h?)
*/

/// \}

} // namespace dip

#endif // DIP_ANALYSIS_H
