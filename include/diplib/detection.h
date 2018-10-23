/*
 * DIPlib 3.0
 * This file contains declarations for detection functions
 *
 * (c)2018, Cris Luengo.
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

#ifndef DIP_DETECTION_H
#define DIP_DETECTION_H

#include "diplib.h"


/// \file
/// \brief Functions for feature detection.
/// \see detection


namespace dip {


/// \defgroup detection Detection
/// \brief %Feature detection algorithms.
///
/// For edge detection see:
/// - `dip::GradientMagnitude`
/// - `dip::MorphologicalGradientMagnitude`
/// - `dip::MultiScaleMorphologicalGradient`
/// - `dip::Canny`
/// - `dip::MonogenicSignalAnalysis`
///
/// For dot detection see:
///  - `dip::Laplace`
///  - `dip::Tophat`


/// \defgroup detection_corners Corner detectors
/// \ingroup detection
/// \brief Corner detection algorithms
/// \{

/// \brief Harris corner detector
///
/// The Harris corner detector is defined as
/// \f[ \text{Det}(M) - \kappa \text{Tr}(M)^2 \; ,\f]
/// where \f$M\f$ is the structure tensor, and \f$\kappa\f$ is a constant typically set to 0.04, in this function
/// controlled by parameter `kappa`. Harris and Stephens noted in their paper that corners are locations in the image
/// where both eigenvalues of \f$M\f$ are large. But they considered eigenvalue computation too expensive, and therefore
/// proposed this cheaper alternative. `dip::ShiTomasiCornerDetector` returns the smallest eigenvalue of \f$M\f$.
///
/// The structure tensor \f$M\f$ is computed using `dip::StructureTensor`, with `gradientSigmas` equal to 1.0 and
/// `tensorSigmas` set through this function's `sigmas` parameter.
///
/// This function generalizes the corner measure above to any number of dimensions. `in` must be scalar and real-valued.
///
/// This function is equivalent to:
/// ```cpp
///     dip::Image M = StructureTensor( in, {}, { 1.0 }, sigmas, S::BEST, boundaryCondition );
///     Image out = dip::Determinant( M ) - k * dip::Square( dip::Trace( M ));
///     dip::ClipLow( out, out, 0 );
/// ```
///
/// **Literature**
/// - C. Harris and M. Stephens, "A combined corner and edge detector", Proceedings of the 4th Alvey Vision Conference, pp. 147–151, 1988.
DIP_EXPORT void HarrisCornerDetector(
      Image const& in,
      Image& out,
      dfloat kappa = 0.04,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
);
inline Image HarrisCornerDetector(
      Image const& in,
      dfloat kappa = 0.04,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
) {
   Image out;
   HarrisCornerDetector( in, out, kappa, sigmas, boundaryCondition );
   return out;
}

/// \brief Shi-Tomasi corner detector
///
/// The Shi-Tomasi corner detector is defined as
/// \f[ \text{min}(\lambda_1, \lambda_2) \; ,\f]
/// where the \f$\lambda\f$ are the eigenvalues of \f$M\f$, the structure tensor. Corners are locations in the image
/// where both eigenvalues of \f$M\f$ are large.
///
/// The structure tensor \f$M\f$ is computed using `dip::StructureTensor`, with `gradientSigmas` equal to 1.0 and
/// `tensorSigmas` set through this function's `sigmas` parameter.
///
/// This function generalizes the corner measure above to any number of dimensions. `in` must be scalar and real-valued.
///
/// This function is equivalent to:
/// ```cpp
///     dip::Image M = StructureTensor( in, {}, { 1.0 }, sigmas, S::BEST, boundaryCondition );
///     out = dip::SmallestEigenvalue( M );
/// ```
///
/// **Literature**
/// - J. Shi and C. Tomasi, "Good features to track", 9th IEEE Conference on Computer Vision and Pattern Recognition, pp. 593–600, 1994.
DIP_EXPORT void ShiTomasiCornerDetector(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
);
inline Image ShiTomasiCornerDetector(
      Image const& in,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
) {
   Image out;
   ShiTomasiCornerDetector( in, out, sigmas, boundaryCondition );
   return out;
}

/// \brief Noble's corner detector
///
/// Noble defined a corner detector as
/// \f[ \text{Det}(M) / \text{Tr}(M) \; ,\f]
/// This is similar to the Harris corner detector (see `dip::Harris`), except it has no parameter to tune. The ratio
/// of the deteminant to the trace is equivalent to the harmonic mean of the eigenvalues.
///
/// This function generalizes the corner measure above to any number of dimensions. `in` must be scalar and real-valued.
///
/// This function is equivalent to:
/// ```cpp
///     dip::Image M = StructureTensor( in, {}, { 1.0 }, sigmas, S::BEST, boundaryCondition );
///     Image out = dip::SafeDivide( dip::Determinant( M ), dip::Trace( M ));
/// ```
///
/// **Literature**
/// - J.A. Noble, "Finding corners", Proceedings of the Alvey Vision Conference, pp. 37.1-37.8, 1987.
DIP_EXPORT void NobleCornerDetector(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
);
inline Image NobleCornerDetector(
      Image const& in,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
) {
   Image out;
   NobleCornerDetector( in, out, sigmas, boundaryCondition );
   return out;
}

/// \brief Wang-Brady corner detector
///
/// Wang and Brady (1995) define a corner operator as
/// \f[ \begin{cases}
///       \Gamma = \left( \frac{\delta^2 F}{\delta \bf{t}^2} \right)^2 - s|\nabla F|^2 = \text{maximum}
///   \\  \frac{\delta^2 F}{\delta \bf{n}^2} = 0
///   \\  |\nabla F|^2 > T_1 , \Gamma > T_2
/// \end{cases} \f]
///
/// Here, \f$\Gamma\f$ is composed of the square of the second derivative of the image \f$F\f$ in the contour direction
/// (\f$\bf{t}\f$ is the unit vector perpendicular to the gradient), and the square norm of the gradient. The first
/// term is a measure for curvature, the second term is a measure for edgeness. \f$s\f$ is a threshold (in this
/// function defined through `threshold`) that determines how much larger the curvature must be compared to the
/// edgeness. Typical values are in the range 0.0 to 0.5, the default is 0.1.
///
/// The second equation indicates that the second derivative in the gradient direction must be zero (the zero crossing
/// of the second derivative indicates the exact sub-pixel location of the edge). The third equation indicates two
/// thresholds that must be satisfied. This function computes only \f$\Gamma\f$, the thresholding must be applied
/// separately.
///
/// This function generalizes the corner measure above to any number of dimensions. `in` must be scalar and real-valued.
///
/// Gradients are computed using Gaussian derivatives, with the `sigmas` parameter. This function is equivalent to:
/// ```cpp
///     Image out = dip::Square( dip::LaplaceMinusDgg( in, sigmas )) - threshold * dip::SquareNorm( dip::Gradient( in, sigmas ));
///     dip::ClipLow( out, out, 0 );
/// ```
///
/// **Literature**
/// - H. Wang and M. Brady, "Real-time corner detection algorithm for motion estimation", %Image and Vision Computing 13(9):695–703, 1995.
DIP_EXPORT void WangBradyCornerDetector(
      Image const& in,
      Image& out,
      dfloat threshold = 0.1,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
);
inline Image WangBradyCornerDetector(
      Image const& in,
      dfloat threshold = 0.1,
      FloatArray const& sigmas = { 2.0 },
      StringArray const& boundaryCondition = {}
) {
   Image out;
   WangBradyCornerDetector( in, out, threshold, sigmas, boundaryCondition );
   return out;
}

/// \}


/// \defgroup detection_lines Line detectors
/// \ingroup detection
/// \brief Line detection algorithms
///
/// See `dip::MonogenicSignalAnalysis` for yet another way to detect lines.
/// \{

/// \brief Frangi vessel detector, single scale (Hessian based)
///
/// Frangi's vesselness measure is based on the eigenvalues of the Hessian matrix. The core concept is that
/// one eigenvalue must be significantly smaller than the others for a local region to resemble a line.
///
/// `sigmas` are used for the computation of the Hessian (which uses Gaussian gradients, see `dip::Hessian`),
/// and determine the scale. To detect wider vessels, increase `sigmas`.
///
/// `parameters` are the two (*&beta;* and *c* in 2D) or three (*&alpha;*, *&beta;* and *c* in 3D) thresholds used
/// in the method. An empty array indicates the default values (`{0.5, 15}` in 2D and `{0.5, 0.5, 500}` in 3D).
///
/// `polarity` indicates whether to look for light lines on a dark brackground (`"white"`) or dark lines on a light
/// background (`"black"`). The sign of the one (2D) or two (3D) larger eigenvalues are examined at each pixel to
/// determine the polarity of the line, if the signs don't match, the pixel is set to 0.
///
/// `in` must be scalar, real-valued, and either 2D or 3D. This function has not been generalized to
/// other dimensionalities.
///
/// The complete multi-scale vessel detector simply applies this function at multiple scales and takes the maximum
/// response at each scale. Even though the original paper didn't mention this, best restuls are obtained when
/// scaling the input image with the square of the sigma:
/// ```cpp
///     std::vector<double> scales = { 1, 2, 4, 8 };
///     dip::Image out = dip::FrangiVesselness( in * ( scales[ 0 ] * scales[ 0 ] ), { scales[ 0 ] } );
///     for( size_t ii = 1; ii < scales.size(); ++ii ) {
///        dip::Supremum( out,  dip::FrangiVesselness( in * ( scales[ ii ] * scales[ ii ] ), { scales[ ii ] } ), out );
///     }
/// ```
///
/// **Literature**
/// - A.F. Frangi, W.J. Niessen, K.L. Vincken and M.A. Viergever, "Multiscale Vessel Enhancement Filtering",
///   in: Medical %Image Computing and Computer-Assisted Intervention (MICCAI’98), LNCS 1496:130-137, 1998.
DIP_EXPORT void FrangiVesselness(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 2.0 },
      FloatArray parameters = {},
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
);
inline Image FrangiVesselness(
      Image const& in,
      FloatArray const& sigmas = { 2.0 },
      FloatArray const& parameters = {}, // for 3D: { 0.5, 0.5, 500 }; for 2D: { 0.5, 15 }
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   FrangiVesselness( in, out, sigmas, parameters, polarity, boundaryCondition );
   return out;
}

/// \brief Matched filters for line detection in 2D
///
/// Matched filters are a filter bank designed to match the shape being detected. In this case, it is a line-line
/// filter of length `length`, with a Gaussian profile (`sigma` determines the width). The filter has an average of
/// zero so that it yields a zero response in flat areas. It is created at 12 different orientations (thus using 15
/// degree steps to cover the full 180 degree half-circle), and the maximum response over all orientations is returned.
///
/// `polarity` indicates whether to look for light lines on a dark brackground (`"white"`) or dark lines on a light
/// background (`"black"`). `in` must be scalar, real-valued, and 2D.
///
/// **Literature**
/// - S. Chaudhuri, S. Chatterjee, N. Katz, M. Nelson, and M. Goldbaum, "Detection of Blood Vessels in Retinal Images
///   Using Two-Dimensional Matched Filters", IEEE Transactions on Medical Imaging 8(3):263-269, 1989
DIP_EXPORT void MatchedFiltersLineDetector2D(
      Image const& in,
      Image& out,
      dip::dfloat sigma = 2.0,
      dip::dfloat length = 10.0,
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
);
inline Image MatchedFiltersLineDetector2D(
      Image const& in,
      dip::dfloat sigma = 2.0,
      dip::dfloat length = 10.0,
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   MatchedFiltersLineDetector2D( in, out, sigma, length, polarity, boundaryCondition );
   return out;
}

/// \brief Danielsson's Hessian-based line detector
///
/// This is a different approach to detecting lines based on the Hessian matrix (2nd order derivatives) compared
/// to Frangi's vesselness measure (`dip::FrangiVesselness`). It is perfectly isotropic, but has some response
/// also to edges, especially in 2D.
///
/// `sigmas` are used for the computation of the Hessian (which uses Gaussian gradients, see `dip::Hessian`),
/// and determine the scale. To detect wider lines, increase `sigmas`.
///
/// `polarity` indicates whether to look for light lines on a dark brackground (`"white"`) or dark lines on a light
/// background (`"black"`). `in` must be scalar, real-valued, and either 2D or 3D.
///
/// **Literature**
/// - P.E. Danielson, Q. Lin and Q.Z. Ye, "Efficient detection of second degree variations in 2D and 3D images",
///   Journal of Visual Communication and %Image Representation 12, 255–305, 2001.
DIP_EXPORT void DanielssonLineDetector(
      Image const& in,
      Image& out,
      dip::FloatArray const& sigmas = { 2.0 },
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
);
inline Image DanielssonLineDetector(
      Image const& in,
      dip::FloatArray const& sigmas = { 2.0 },
      String const& polarity = S::WHITE,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   DanielssonLineDetector( in, out, sigmas, polarity, boundaryCondition );
   return out;
}

/// \brief Line detector based on robust path openings
///
/// RORPO stands for Ranking the Orientation Responses of Path Operators. It filters `in` with 4 (2D)
/// or 7 (3D) different directions of path openings, ranks the results point-wise, and compares appropriate
/// ranks to determine if a pixel belongs to a line or not.
///
/// `length` is the length of the path operator. Longer paths make for a more selective filter that requires
/// lines to be straighter.
///
/// `polarity` indicates whether to look for light lines on a dark brackground (`"white"`) or dark lines on a light
/// background (`"black"`). `in` must be scalar, real-valued, and either 2D or 3D.
///
/// **Literature**
/// - O. Merveille, H. Talbot, L. Najman, and N. Passat, "Curvilinear Structure Analysis by Ranking the Orientation
///   Responses of Path Operators", IEEE Transactions on Pattern Analysis and Machine Intelligence 40(2):304-317, 2018.
DIP_EXPORT void RORPOLineDetector(
      Image const& in,
      Image& out,
      dip::uint length = 15,
      String const& polarity = S::WHITE
);
inline Image RORPOLineDetector(
      Image const& in,
      dip::uint length = 15,
      String const& polarity = S::WHITE
) {
   Image out;
   RORPOLineDetector( in, out, length, polarity );
   return out;
}


/// \}

} // namespace dip

#endif // DIP_DETECTION_H
