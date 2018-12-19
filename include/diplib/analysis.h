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
#include "distribution.h"


/// \file
/// \brief Functions for assorted analysis functions.
/// \see analysis


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
///  - `"integer"`: Doesn't look for sub-pixel locations, returning the integer coordinates of the extremum.
///
/// The image `in` must be scalar and real-valued.
DIP_EXPORT SubpixelLocationResult SubpixelLocation(
      Image const& in,
      UnsignedArray const& position,
      String const& polarity = S::MAXIMUM,
      String const& method = dip::S::PARABOLIC_SEPARABLE
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
      String const& method = dip::S::PARABOLIC_SEPARABLE
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
      String const& method = dip::S::PARABOLIC_SEPARABLE
);

/// \brief Finds the coordinates of a local maximum close to `start`
///
/// The mean shift method iteratively finds the center of gravity of a Gaussian-shaped neighborhood around `start`,
/// moving `start` to this location, until convergence. The location that the process converges to is a local
/// maximum. Convergence is assumed when the magnitude of the shift is smaller than `epsilon`.
///
/// To speed up the computation within this function, the output of `dip::MeanShiftVector` is used. This filter
/// pre-computes the center of gravity of all neighborhoods in the image. Specify the neighborhood sizes in that
/// function.
DIP_EXPORT FloatArray MeanShift(
      Image const& meanShiftVectorResult,
      FloatArray const& start,
      dfloat epsilon = 1e-3
);

/// \brief Finds the coordinates of local a maximum close to each point in `start`.
///
/// Repeatedly calls the `dip::MeanShift` function described above, for each point in `start`. Duplicate maxima
/// are not removed, such that `out[ii]` is the local maximum arrived to from `startArray[ii]`.
inline FloatCoordinateArray MeanShift(
      Image const& meanShiftVectorResult,
      FloatCoordinateArray const& startArray,
      dfloat epsilon = 1e-3
) {
   FloatCoordinateArray out;
   out.reserve( startArray.size() );
   for( auto& start : startArray ) {
      out.push_back( MeanShift( meanShiftVectorResult, start, epsilon ));
   }
   return out;
}


/// \brief Calculates the cross-correlation between two images of equal size.
///
/// The `normalize` flag determines whether to compute the regular cross-correlation (`"don't normalize"`)
/// or to normalize the frequency-domain inputs so that only the phase information is of importance
/// (`"normalize"` or `"phase"`). This results as a very sharp peak in the spatial domain.
/// Note that this normalization is not related to what is commonly referred to as "normalized cross-correlation",
/// where the input images are whitened (in the spatial domain) before the cross-correlations is computed. The
/// method is instead related to the "phase correlation" as proposed by Kuglin and Hines (1975).
///
/// When `normalize` is `"normalize"`, the computation performed in the Fourier Domain is
/// `out = (in1*dip::Conjugate(in2))/dip::SquareModulus(in1)` (as per Luengo & van Vliet, 2000).
///  (see `dip::Conjugate`, `dip::SquareModulus`).
/// When `normalize` is set to `"phase"`, the computation is
/// `out = (in1*dip::Conjugate(in2))/(dip::Modulus(in1)*dip::Modulus(in2))` (as per Kuglin & Hines, 1975).
/// These two approaches are identical if the only difference between `in1` and `in2` is a shift, except that
/// `"normalize"` is computationally cheaper than `"phase"`. If the images are obtained with different
/// modalities, or if important differences exist in the images, the `"phase"` method might be the better choice.
///
/// As elsewhere, the origin is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image. Thus, for `in1==in2`, only this pixel will be set.
/// See `dip::FindShift` with the `"CC"`, `"NCC"` or `"PC"`  methods for localizing this peak.
///
/// If `in1` or `in2` is already Fourier transformed, set `in1Representation` or `in2Representation`
/// to `"frequency"`. Similarly, if `outRepresentation` is `"frequency"`, the output will not be
/// inverse-transformed, so will be in the frequency domain.
///
/// `in1` and `in2` must be scalar images with the same dimensionality and sizes. Spatial-domain
/// images must be real-valued, and frequency-domain images are assumed (but not tested) to be
/// conjugate-symmetric (i.e. be Fourier transforms of real-valued images).
/// `out` will be real-valued if `outRepresentation` is `"spatial"`.
///
/// **Literature**
///  - C.D. Kuglin and D.C. Hines, "The phase correlation image alignment method", International
///    Conference on Cybernetics and Society (IEEE), pp 163-165, 1975.
///  - C.L. Luengo Hendriks and L.J. van Vliet, "Improving resolution to reduce aliasing in an undersampled image sequence",
///    in: Proceedings of SPIE 3965:214–222, 2000.
DIP_EXPORT void CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& in1Representation = S::SPATIAL,
      String const& in2Representation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      String const& normalize = S::NORMALIZE
);
inline Image CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      String const& in1Representation = S::SPATIAL,
      String const& in2Representation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      String const& normalize = S::NORMALIZE
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
/// smaller than the pattern period along each dimension.
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
///    See the notes in `dip::CrossCorrelationFT` regarding the normalization of the cross-correlation using the
///    `"normalize"` flag, which is not as what is commonly referred to as "normalized cross-correlation".
///
///  - `"PC"`: As `"CC"`, but using phase correlation. This method works for any number of dimensions.
///    `parameter` is ignored. See the notes in `dip::CrossCorrelationFT` regarding the normalization of the
///    cross-correlation using the `"phase"` flag.
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
/// `in1` and `in2` must be scalar images with the same dimensionality and sizes.
///
/// **Literature**
///  - C.L. Luengo Hendriks, "Improved Resolution in Infrared Imaging Using Randomly Shifted Images", M.Sc. Thesis,
///    Delft University of Technology, The Netherlands, 1998.
///  - T.Q. Pham, M. Bezuijen, L.J. van Vliet, K. Schutte and C.L. Luengo Hendriks, "Performance of Optimal
///    Registration Estimators", In: Visual Information Processing XIV, Proceedings of SPIE 5817, 2005.
DIP_EXPORT FloatArray FindShift(
      Image const& in1,
      Image const& in2,
      String const& method = "MTS",
      dfloat parameter = 0,
      UnsignedArray maxShift = {}
);


/// \brief Finds the scaling, translation and rotation between two 2D images using the Fourier Mellin transform
///
/// The output array represents a 2x3 affine transform matrix (in column-major order) that can be used as input
/// to `dip::AffineTransform` to transform `in2` to match `in1`. `out` is the `in2` image transformed in this way.
///
/// The two images must be real-valued, scalar and two-dimensional, and have the same sizes.
///
/// `interpolationMethod` has a restricted set of options: `"linear"`, `"3-cubic"`, or `"nearest"`.
/// See \ref interpolation_methods for their definition. If `in` is binary, `interpolationMethod` will be
/// ignored, nearest neighbor interpolation will be used.
///
/// Example:
/// ```cpp
///     dip::Image in1, in2, out;
///     // assign to in1 and in2 two similar images
///     auto T = FourierMellinMatch2D( in1, in2, out );
///     // out is a transformed version of in2, as per
///     dip::AffineTransform( in2, out, T );
/// ```
DIP_EXPORT FloatArray FourierMellinMatch2D(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& interpolationMethod = S::LINEAR
);
inline Image FourierMellinMatch2D(
      Image const& in1,
      Image const& in2,
      String const& interpolationMethod = S::LINEAR
) {
   Image out;
   FourierMellinMatch2D( in1, in2, out, interpolationMethod );
   return out;
}


/// \brief Computes the structure tensor.
///
/// The structure tensor is a tensor image that contains, at each pixel, information about the local image structure.
/// The eigenvalues of the structure tensor are larger when there are stronger gradients locally. That is, if all
/// eigenvalues are small, the image is locally uniform. If one eigenvalue is large, then there is a unique line
/// or edge orientation (in 2D), or a plane-like edge or structure (in 3D). In 3D, if two eigenvalues are large
/// then there is a line-like structure. The associated eigenvalues indicate the orientation of this structure.
/// See the literature references below for more information.
///
/// `in` must be a scalar, real-valued image. `out` will be a symmetric NxN tensor image, where N is the number
/// of dimensions in `in`. Out is computed by:
/// ```cpp
///     dip::Image g = dip::Gradient( in, gradientSigmas );
///     dip::Image out = dip::Gauss( g * dip::Transpose( g ), tensorSigmas );
/// ```
///
/// If `mask` is given (not a raw image), then it is interpreted as confidence weights for the input pixels. It
/// should have values between 0 and 1 (or be binary). Normalized differential convolution is used to compute the
/// gradients. This method applies Gaussian gradients taking missing and uncertain values into account. See
/// `dip::NormalizedDifferentialConvolution`.
///
/// See `dip::Gauss` for the meaning of the parameters `method`, `boundaryCondition` and `truncation`.
///
/// The functions `dip::StructureTensorAnalysis2D` and `dip::StructureTensorAnalysis3D` can be used with the output
/// of this function to obtain useful image parameters.
///
/// **Literature**
///  - B. Jahne, "Practical Handbook on Image Processing for Scientific Applications", chapter 13, CRC Press, 1997.
///  - L.J. van Vliet and P.W. Verbeek, "Estimators for Orientation and Anisotropy in Digitized Images",
///     in: J. van Katwijk, J.J. Gerbrands, M.R. van Steen, J.F.M. Tonino (eds.), Proc. First Annual Conference of the
///     Advanced School for Computing and Imaging, pp. 442-450, ASCI, Delft, 1995.
///  - C.F. Westin, "A Tensor Framework for Multidimensional Signal Processing", PhD thesis, Linkoping University, Sweden, 1994.
DIP_EXPORT void StructureTensor(
      Image const& in,
      Image const& mask,
      Image& out,
      FloatArray const& gradientSigmas = { 1.0 },
      FloatArray const& tensorSigmas = { 5.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
inline Image StructureTensor(
      Image const& in,
      Image const& mask = {},
      FloatArray const& gradientSigmas = { 1.0 },
      FloatArray const& tensorSigmas = { 5.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   StructureTensor( in, mask, out, gradientSigmas, tensorSigmas, method, boundaryCondition, truncation );
   return out;
}

/// \brief Computes useful image parameters from the 2D structure tensor.
///
/// `in` must be a 2D, symmetric 2x2 tensor image obtained from `dip::StructureTensor`. This function takes a pointer
/// to output images, instead of taking them by reference. Set pointers to `nullptr` if you do not want the given
/// output computed. Use this function as follows:
/// ```cpp
///     dip::Image st = dip::StructureTensor( img );
///     dip::Image energy, orientation;
///     dip::StructureTensorAnalysis2D( st, nullptr, nullptr, &orientation, &energy );
/// ```
/// (note how the last two parameters were not given, they default to `nullptr`). The code above computes both the
/// orientation and energy values of the structure tensor.
///
/// The output images will be reallocated to be the same size as the input image. They will be scalar and of a
/// floating-point type.
///
/// The output images are defined as follows:
///
/// %Image        | Description
/// --------------|------------
/// `l1`          | The largest eigenvalue.
/// `l2`          | The smallest eigenvalue.
/// `orientation` | Orientation. Lies in the interval (-pi/2, pi/2).
/// `energy`      | Sum of the two eigenvalues `l1` and `l2`.
/// `anisotropy1` | Measure for local anisotropy: `( l1 - l2 ) / ( l1 + l2 )`.
/// `anisotropy2` | Measure for local anisotropy: `1 - l2 / l1`, where `l1 > 0`.
/// `curvature`   | Magnitude of the curvature (1/bending radius).
///
/// Curvature sign cannot be computed because the structure tensor does not distinguish the direction of
/// the gradient.
///
/// Note that `l1` and `l2` will both reference data within the same data segment, and therefore will likely not
/// have normal strides.
///
/// For a 3D structure tensor analysis, see the function `dip::StructureTensorAnalysis3D`.
/// A different interface to this function is available in `dip::StructureTensorAnalysis`.
/// Note that eigenvalues and eigenvectors can also be computed using `dip::Eigenvalues` and `dip::EigenDecomposition`.
DIP_EXPORT void StructureTensorAnalysis2D(
      Image const& in,
      Image* l1 = nullptr,
      Image* l2 = nullptr,
      Image* orientation = nullptr,
      Image* energy = nullptr,
      Image* anisotropy1 = nullptr,
      Image* anisotropy2 = nullptr,
      Image* curvature = nullptr
);

/// \brief Computes useful image parameters from the 3D structure tensor.
///
/// `in` must be a 3D, symmetric 3x3 tensor image obtained from `dip::StructureTensor`. This function takes a pointer
/// to output images, instead of taking them by reference. Set pointers to `nullptr` if you do not want the given
/// output computed. Use this function as follows:
/// ```cpp
///     dip::Image st = dip::StructureTensor( img );
///     dip::Image energy;
///     dip::StructureTensorAnalysis3D( st, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &energy );
/// ```
/// (note how the last two parameters were not given, they default to `nullptr`). The code above computes the
/// energy value of the structure tensor.
///
/// The output images will be reallocated to be the same size as the input image. They will be scalar and of a
/// floating-point type.
///
/// The output images are defined as follows:
///
/// %Image        | Description
/// --------------|------------
/// `l1`          | The largest eigenvalue.
/// `phi1`        | First component of the orientation of the eigenvector for `l1`.
/// `theta1`      | Second component of the orientation of the eigenvector for `l1`.
/// `l2`          | The middle eigenvalue.
/// `phi2`        | First component of the orientation of the eigenvector for `l2`.
/// `theta2`      | Second component of the orientation of the eigenvector for `l2`.
/// `l3`          | The smallest eigenvalue.
/// `phi3`        | First component of the orientation of the eigenvector for `l3`.
/// `theta3`      | Second component of the orientation of the eigenvector for `l3`.
/// `energy`      | Sum of the three eigenvalues `l1`, `l2` and `l3`.
/// `cylindrical` | Measure for local anisotropy: `( l2 - l3 ) / ( l2 + l3 )`.
/// `planar`      | Measure for local anisotropy: `( l1 - l2 ) / ( l1 + l2 )`.
///
/// For a 2D structure tensor analysis, see the function `dip::StructureTensorAnalysis2D`.
/// A different interface to this function is available in `dip::StructureTensorAnalysis`.
/// Note that eigenvalues and eigenvectors can also be computed using `dip::Eigenvalues` and `dip::EigenDecomposition`.
DIP_EXPORT void StructureTensorAnalysis3D(
      Image const& in,
      Image* l1 = nullptr,
      Image* phi1 = nullptr,
      Image* theta1 = nullptr,
      Image* l2 = nullptr,
      Image* phi2 = nullptr,
      Image* theta2 = nullptr,
      Image* l3 = nullptr,
      Image* phi3 = nullptr,
      Image* theta3 = nullptr,
      Image* energy = nullptr,
      Image* cylindrical = nullptr,
      Image* planar = nullptr
);

/// \brief Interface to `dip::StructureTensorAnalysis2D` and `dip::StructureTensorAnalysis3D`.
///
/// `in` is as in `dip::StructureTensorAnalysis2D` or `dip::StructureTensorAnalysis3D`. That is, either a 2D
/// symmetric 2x2 tensor image or a 3D symmetric 3x3 tensor image, real-valued, as obtained from `dip::StructureTensor`.
/// `out` is an array with references to output images. It should have exactly as many elements as `outputs`.
///
/// `outputs` is an array with one or more of the following strings, indicating which outputs are needed:
///  - For 2D inputs: `"l1"`, `"l2"`, `"orientation"`, `"energy"`, `"anisotropy1"`, `"anisotropy2"`, `"curvature"`.
///  - For 3D inputs: `"l1"`, `"phi1"`, `"theta1"`, `"l2"`, `"phi2"`, `"theta2"`, `"l3"`, `"phi3"`, `"theta3"`,
///    `"energy"`, `"cylindrical"`, `"planar"`.
///
/// The order of the strings in `outputs` indicates the order they will be written to the `out` array.
///
/// See the functions `dip::StructureTensorAnalysis2D` and `dip::StructureTensorAnalysis3D` for more information
/// on these outputs.
DIP_EXPORT void StructureTensorAnalysis(
      Image const& in,
      ImageRefArray& out,
      StringArray const& outputs
);
inline ImageArray StructureTensorAnalysis(
      Image const& in,
      StringArray const& outputs
) {
   dip::uint nOut = outputs.size();
   ImageArray out( nOut );
   ImageRefArray refOut = CreateImageRefArray( out );
   DIP_STACK_TRACE_THIS( StructureTensorAnalysis( in, refOut, outputs ));
   return out;
}


/// \brief Analyzes the local structure of the image at multiple scales.
///
/// Computes the structure tensor, smoothed with each of the values in `scales` multiplied by `gradientSigmas`,
/// determines the feature `feature` from it, and averages across the image for each scale. This leads to a series
/// of data points showing how the selected feature changes across scales. `scales` and `gradientSigmas` are
/// given in pixels, the image's pixel size is not taken into account.
///
/// The reason that each element of `scales` is mutliplied by `gradientSigmas` is to allow analysis of
/// non-isotropic images. The `gradientSigmas` corrects for the non-isotropy, allowing `scales` to be a scalar
/// value for each scale.
///
/// `scales` defaults to a series of 10 values geometrically spaced by `sqrt(2)`, and starting at 1.0. Units are
/// pixels.
///
/// `feature` can be any of the strings allowed by `dip::StructureTensorAnalysis`, but `"energy"`, `"anisotropy1"`,
/// and `"anisotropy2"` (in 2D), and `"energy"`, `"cylindrical"`, and `"planar"` (in 2D) make the most sense.
///
/// See `dip::Gauss` for the meaning of the parameters `method`, `boundaryCondition` and `truncation`.
///
/// `in` must be scalar and real-valued, and either 2D or 3D. `mask` must be of the same size, and limits the
/// region over which the structure tensor feature is averaged.
///
/// See `dip::StructureTensor` for more information about the structure tensor.
DIP_EXPORT Distribution StructureAnalysis(
      Image const& in,
      Image const& mask,
      std::vector< dfloat > const& scales = {},
      String const& feature = "energy",
      FloatArray const& gradientSigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);


/// \brief Computes the monogenic signal, a multi-dimensional generalization of the analytic signal.
///
/// The monogenic signal of an *n*-dimensional image has *n*+1 components. The first component has been
/// filtered with the even (symmetric) filter component, and the other *n* components have been filtered
/// with the odd (antisymmetric) fitler components, where each of those filter components is antisymmetric
/// along a different dimension.
///
/// This function splits the frequency spectrum of the image into `wavelengths.size()` components. The radial
/// component of a log-Gabor filter bank will be used. These radial frequency filters are defined by
/// `wavelengths` (in pixels) and `bandwidth`. See `dip::LogGaborFilterBank` for details.
///
/// The filters are always applied in the frequency domain. If `outRepresentation` is `"spatial"`, the inverse
/// Fourier transform will be applied to bring the result back to the spatial domain. Otherwise, it should be
/// `"frequency"`, and no inverse transform will be applied. Likewise, `inRepresentation` specifies whether `in`
/// has already been converted to the frequency domain or not.
///
/// `in` must be scalar and real-valued if given in the spatial domain. If `in` is in the frequency domain, it is
/// expected to be conjugate symmetric, and thus have a real-valued inverse transform (the imaginary part of the
/// inverse transform will be discarded).
/// Out will be a tensor image with `wavelengths.size()` tensor columns and *n*+1 tensor rows. The data type will be
/// single-precision float for spatial-domain output, or single-precision complex for frequency-domain output.
///
/// **Literature**
///  - M. Felsberg and G. Sommer, "The Monogenic Signal", IEEE Transactions on Signal Processing 49(12):3136-3144, 2001.
DIP_EXPORT void MonogenicSignal(
      Image const& in,
      Image& out,
      FloatArray const& wavelengths = { 3.0, 24.0 },
      dfloat bandwidth = 0.41, // ~3 octaves
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL
);
inline Image MonogenicSignal(
      Image const& in,
      FloatArray const& wavelengths = { 3.0, 24.0 },
      dfloat bandwidth = 0.41, // ~3 octaves
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL
) {
   Image out;
   MonogenicSignal( in, out, wavelengths, bandwidth, inRepresentation, outRepresentation );
   return out;
}

/// \brief Computes useful image parameters from the monogenic signal.
///
/// `in` is a tensor image produced by `dip::MonogenicSignal`, for which `in.TensorRows() == in.Dimensionality() + 1`,
/// and `in.TensorColumns() > 1`. In the case of a single column (a single wavelength was given), the congruency output
/// is not possible. `in` is real-valued (single-precision float) and in the spatial domain.
/// `out` is an array with references to output images. It should have exactly as many elements as `outputs`.
///
/// `outputs` is an array with one or more of the following strings, indicating which outputs are needed:
///
/// %Image          | Description
/// ----------------|------------
/// `"congruency"`  | **Phase congruency**, a measure of line/edge significace
/// `"orientation"` | Line/edge orientation (in the range [-pi/2, pi/2]), 2D only
/// `"phase"`       | Phase angle (pi/2 is a white line, 0 is an edge, -pi/2 is a black line)
/// `"enery"`       | Raw phase congruency energy
/// `"symmetry"`    | **Phase symmetry**, a constrast invariant measure of symmetry
/// `"symenergy"`   | Raw phase symmetry energy
///
/// The order of the strings in `outputs` indicates the order they will be written to the `out` array.
///
/// The output images will be reallocated to be the same size as the input image. They will be scalar and of a
/// floating-point type.
///
/// `noiseThreshold` indicates the noise threshold to use on the energy images when computing phase congruency
/// or symmetry.
///
/// Two different algorithms for phase conguency are implemented:
///
/// - Kovesi's method works for images in 2D only, and requires several scales (>2) to be present in the monogenic
///   signal. This method will use the following input arguments:
///     - `frequencySpreadThreshold` is a threshold that avoids high phase congurency values if the frequency
///       spread is not large enough.
///     - `sigmoidParameter` is the parameter to the sigmoid function that weighs congruency with the frequency
///       spread.
///     - `deviationGain` determines how much the calculated phase deviation should be magnified. Larger values
///       create a sharper response to edges, but also reduces this respone's magnitude. Sensible values are in
///       the range [1,2].
///
/// - Felsberg's method works in any number of dimensions, and requires exactly two scales to be present in the
///   monogenic signal. This method ignores the three input arguments specified above.
///
/// Phase symmetry can be computed with any number of dimensions and any number of scales. The `polarity` input
/// argument will be used. It is the string `"white"`, `"black"` or `"both"`, indicating which symmetry features
/// to find.
///
/// The other outputs are computed as part of the computation of phase congruency or phase symmetry. These outputs
/// can be requested without requesting the two main parameters, but might require some of the input arguments
/// discussed above to be set.
///
/// **Literature**
/// - P. Kovesi, "Image features from phase information", Videre: Journal of Computer Vision Research 1(3), 1999.
/// - M. Felsberg and G. Sommer, "A New Extension of Linear Signal Processing for Estimating Local Properties and
///   Detecting Features", DAGM Symposium, 2000.
DIP_EXPORT void MonogenicSignalAnalysis(
      Image const& in,
      ImageRefArray& out,
      StringArray const& outputs,
      dfloat noiseThreshold = 0.2,
      dfloat frequencySpreadThreshold = 0.5,
      dfloat sigmoidParameter = 10,
      dfloat deviationGain = 1.5,
      String const& polarity = S::BOTH
);
inline ImageArray MonogenicSignalAnalysis(
      Image const& in,
      StringArray const& outputs,
      dfloat noiseThreshold = 0.2,
      dfloat frequencySpreadThreshold = 0.5,
      dfloat sigmoidParameter = 10,
      dfloat deviationGain = 1.5,
      String const& polarity = S::BOTH
) {
   dip::uint nOut = outputs.size();
   ImageArray out( nOut );
   ImageRefArray refOut = CreateImageRefArray( out );
   DIP_STACK_TRACE_THIS( MonogenicSignalAnalysis( in, refOut, outputs, noiseThreshold, frequencySpreadThreshold, sigmoidParameter, deviationGain, polarity ));
   return out;
}


/// \brief Estimates the pair correlation function of the different phases in `object`.
///
/// If `object` is a binary image, the image is a regarded as a two-phase image.
/// In case `object` is of an unsigned integer type, the image is regarded as a labeled image,
/// with each integer value encoding a phase (note that 0 is included here).
///
/// Optionally a `mask` image can be provided to select which pixels in `object`
/// should be used to estimate the pair correlation.
///
/// `probes` specifies how many random point pairs should be drawn to estimate the function.
/// `length` specifies the maximum correlation length in pixels.
/// The correlation function can be estimated using a random sampling method (`sampling` is
/// `"random"`), or a grid sampling method (`"grid"`). For grid sampling, `probes` can be 0,
/// in which case all possible pairs along all image axes are used. Otherwise, the grid covers
/// a subset of points, with all possible pairs along all image axes; the actual number of pairs
/// is approximate. In general, the grid method needs a lot more probes to be precise, but
/// it is faster for a given number of probes because it uses sequential memory access.
///
/// `options` can contain one or more of the following strings:
/// - `"covariance"`: Compute covariance instead of correlation.
/// - `"normalize volume"`: Normalizes the distribution using the volume fraction
/// - `"normalize volume^2"`: Normalizes the distribution using the square of the volume fraction
///
/// Without any of the option strings given, the output has `N` values, with `N` the number of phases
/// in the image. Element `i` gives the probability of two points at a given distance both hitting
/// phase number `i`. When normalized using the `"normalize volume"` option, the probability given is
/// that of the second point hitting phase `i`, given that the first point hits phase `i`. This normalization
/// is computed by dividing all values (distances) for element `i` by the volume fraction of phase `i`
/// (which is given by the estimated probability at distance equal to 0). When normalized using the
/// `"normalize volume^2"` option, the values given are expected to tends towards 1, as the square of the
/// volume fraction of phase `i` is the probability of two random points hitting phase `i`.
///
/// These values are read from the output `distribution` by `distribution[distance].Y(i)`, and the
/// corresponding distance in physical units is given by `distribution[distance].X()`. If `object` does not
/// have a pixel size, its pixel sizes are not isotropic, or have no physical units, then the distances given
/// are in pixels. `distance` runs from 0 to `length` (inclusive).
///
/// With the `"covariance"` option, the output distribution has `N`&times;`N` values. The element at
/// `(i,j)` gives the probability for two points at a given distance to land on phases `i` and `j`
/// respectively. The value for `(i,j)` will be identical to that for `(j,i)`. The values at the diagonal
/// will correspond to the pair correlation as described above. The rest of the elements are for the
/// cases where the two points fall in different phases. Normalization breaks the symmetry, since each
/// column `i` is normalized by the volume fraction for phase `i`. That is, `(i,j)` will give the probability
/// that the second point, at a given distance, will hit phase `j` given that the first point hits phase `i`.
DIP_EXPORT Distribution PairCorrelation(
      Image const& object,
      Image const& mask,
      dip::uint probes = 1000000,
      dip::uint length = 100,
      String const& sampling = S::RANDOM,
      StringSet const& options = {}
);

/// \brief Estimates the probabilistic pair correlation function of the different phases in `phases`.
///
/// `phases` is a real-valued image of a floating-point type, with one or more channels (tensor elements).
/// Each channel represents the probability per pixel of one of the phases.
/// The function assumes, but does not check, that these values are with the [0,1] range, and add up to
/// 1 or less.
///
/// All other parameters are as in `dip::PairCorrelation`.
DIP_EXPORT Distribution ProbabilisticPairCorrelation(
      Image const& phases,
      Image const& mask,
      dip::uint probes = 1000000,
      dip::uint length = 100,
      String const& sampling = S::RANDOM,
      StringSet const& options = {}
);

/// \brief Estimates the expected value of half the square difference between field values at a distance `d`.
///
/// The input image `in` must be scalar and real-valued. An isotropic semivariogram is computed as a function
/// of the distance, which is varied from 0 to `length` (in pixels). Therefore, the field in `in` is assumed isotropic
/// and stationary.
///
/// By definition, the semivariogram is zero at the origin (at zero distance). If there is no spatial correlation,
/// the semivariogram is constant for non-zero distance. The magnitude of the jump at the origin is called nugget.
/// The value of the semivariogram in the limit of the distance to infinity is the variance of the field; this
/// value is called the sill. The distance at which the sill is reached (or, more robustly, the distance at which
/// 0.95 times the sill is reached) is called range, and provides a meaningful description of the field.
///
/// Optionally a `mask` image can be provided to select which pixels in `in` should be used to estimate the
/// semivariogram.
///
/// `probes` specifies how many random point pairs should be drawn to estimate the semivariogram.
/// `length` specifies the maximum pair distance in pixels.
/// The semivariogram can be estimated using a random sampling method (`sampling` is `"random"`), or a grid sampling
/// method (`"grid"`). For grid sampling, `probes` can be 0, in which case all possible pairs along all image
/// axes are used. Otherwise, the grid covers a subset of points, with all possible pairs along all image axes;
/// the actual number of pairs is approximate. In general, the grid method needs a lot more probes to be precise,
/// but it is faster for a given number of probes because it uses sequential memory access.
///
/// The output `dip::Distribution` has *x* samples given in physical units. However, if `in` does not
/// have a pixel size, its pixel sizes are not isotropic, or have no physical units, then the distances given
/// are in pixels.
///
/// **Literature**
///  - G. Matheron, "Principles of geostatistics", Economic Geology 58(8):1246, 1963.
DIP_EXPORT Distribution Semivariogram(
      Image const& in,
      Image const& mask,
      dip::uint probes = 1000000,
      dip::uint length = 100,
      String const& sampling = S::RANDOM
);


/// \brief Estimates the chord length distribution of the different phases in `object`.
///
/// If `object` is a binary image, the image is a regarded as a two-phase image.
/// In case `object` is of an unsigned integer type, the image is regarded as a labeled image,
/// with each integer value encoding a phase (note that 0 is included here).
///
/// Optionally a `mask` image can be provided to select which pixels in `object`
/// should be used for the estimation.
///
/// `probes` specifies how many random lines should be drawn to estimate the distribution.
/// `length` specifies the maximum chord length in pixels.
/// The chord length distribution can be estimated using a random sampling method (`sampling` is
/// `"random"`), or a grid sampling method (`"grid"`). For grid sampling, `probes` can be 0,
/// in which case all lines along all image axes are used. Otherwise, the grid covers
/// a subset of lines; the actual number of lines is approximate. In general, the grid method
/// needs a lot more probes to be precise, but it is faster for a given number of probes because
/// it uses sequential memory access.
///
/// The output has `N` values, with `N` the number of phases in the image. Element `i` gives the
/// probability of a chord of length `len` pixels within phase number `i`. The chord length is the
/// length of an intersection of a line with the phase.
/// These values are read from the output `distribution` by `distribution[len].Y(i)`, and the
/// corresponding length in physical units is given by `distribution[len].X()`. If `object` does not
/// have a pixel size, its pixel sizes are not isotropic, or have no physical units, then the distances given
/// are in pixels. `len` runs from 0 to `length` (inclusive).
DIP_EXPORT Distribution ChordLength(
      Image const& object,
      Image const& mask,
      dip::uint probes = 100000,
      dip::uint length = 100,
      String const& sampling = S::RANDOM
);


/// \brief Computes the distribution of distances to the background of `region` for the different phases in `object`.
///
/// If `object` is a binary image, the image is a regarded as a two-phase image.
/// In case `object` is of an unsigned integer type, the image is regarded as a labeled image,
/// with each integer value encoding a phase (note that 0 is included here).
///
/// The `region` image is a binary image, the distances to its background will be computed.
/// If `region` is a labeled image, the distances to the zero-valued pixels will be computed.
/// These distances will take the pixels sizes of `region` into account. If `region` does not
/// have pixels sizes, those of `object` will be used instead.
///
/// The output `dip::Distribution` has *x* value increments given by the pixel size. That is,
/// the distribution has one sample per pixel. `length` is the length of the distribution, and
/// thus limits the distances taken into account.
DIP_EXPORT Distribution DistanceDistribution(
      Image const& object,
      Image const& region,
      dip::uint length = 100
);


/// \brief Computes the granulometric function for an image
///
/// The granulometry yields a volume-weighted, grey-value--weighted, cumulative distribution of object sizes.
/// It can be used to obtain a size distribution of the image without attempting to segment and separate
/// individual objects. It is computed by a series of openings or closings at different scales. The result
/// at each scale is integrated (summed). The obtained series of values is scaled such that the value for
/// scale 0 is 0, and for scale infinity is 1.
///
/// The derivative of the cumulative distribution is a volume-weighted and grey-value--weighted distribution of
/// object sizes. See `dip::Distribution::Differentiate`.
///
/// Grey-value--weighted means that objects with a larger grey-value contrast will be weighted more heavily.
/// Ensuring a uniform grey-value contrast to prevent this characateristic from affecting the estimated size
/// distribution.
///
/// Volume-weighted means that objects are weighted by their volume (area in 2D). By dividing the distribution
/// (note: not the cumulative distribution) by the volume corresponding to each scale, it is possible to
/// approximate a count-based distribution.
///
/// This function implements various granulometries, to be specified through the parameters `type`, `polarity`
/// and `options`. The following `type` values specify the shapes of the structuring element (SE), which determines
/// the measurement type:
///  - `"isotropic"`: An isotropic SE leads to a size distribution dictated by the width of objects.
///  - `"length"`: A line SE leads to a size distribution dictated by the length of objects. We use (constrained)
///    path openings or closings (see Luengo, 2010).
///
/// The `polarity` flag determines whether it is white objects on a black background (`"opening"`) or black objects
/// on a white background (`"closing"`) that are being analyzed.
///
/// The `options` parameter can contain a set of flags that modify how the operations are applied. The allowed flags
/// differ depending on the `type` flag.
///  - For `"isotropic"` granulometries:
///      - `"reconstruction"`: uses openings or closings by reconstruction instead of structural openings or closings.
///        This leads to objects not being broken up in the same way. Objects need to be clearly separated spatially
///        for this to work.
///      - `"shifted"`: uses sub-pixel shifted isotropic strcuturing elements. This allows a finer sampling of the
///        scale axis (see Luengo et al., 2007). Ignored for images with more than 3 dimensions.
///      - `"interpolate"`: interpolates by a factor up to 8x for smaller scales, attempting to avoid SE diameters
///        smaller than 8. This improves precision of the result for small scales (see Luengo et al., 2007).
///      - `"subsample"`: subsamples for larger scales, such that the largest SE diameter is 64. This speeds up
///        computation, at the expense of precision.
///  - For `"length"` granulometries:
///      - `"unconstrained"`: by default, we use constrained path openings or closings, which improves the precision
///        of the measurement, but is a little bit more expensive (see Luengo, 2010). This option causes the use
///        of normal path openings or closings.
///      - `"robust"`: applies path openings or closings in such a way that they are less sensitive to noise.
///
/// `scales` defaults to a series of 12 values geometrically spaced by `sqrt(2)`, and starting at `sqrt(2)`. `scales`
/// are in pixels, the image's pixel size is not taken into account.
///
/// `in` must be scalar and real-valued. `mask` must have the same sizes, and limits the region
/// in which objects are measured.
///
/// **Literature**
///  - C.L. Luengo Hendriks, G.M.P. van Kempen and L.J. van Vliet, "Improving the accuracy of isotropic granulometries",
///    Pattern Recognition Letters 28(7):865-872, 2007.
///  - C.L. Luengo Hendriks, "Constrained and dimensionality-independent path openings",
///    IEEE Transactions on Image Processing 19(6):1587–1595, 2010.
DIP_EXPORT Distribution Granulometry(
      Image const& in,
      Image const& mask,
      std::vector< dfloat > const& scales = {},
      String const& type = "isotropic",
      String const& polarity = S::OPENING,
      StringSet const& options = {}
);


/// \brief Estimates the fractal dimension of the binary image `in` the sliding box method.
///
/// The sliding box method is an enhancement of the classical box counting method that counts many more boxes
/// at each scale, and therefore is more precise. By sliding the box one pixel at a time, it is also not affected
/// by partial boxes (i.e. the boxes at the right and bottom edge of the image that do not fit within the image
/// domain). The counts are computed in an efficient manner, which makes it similar in complexity to counting
/// only the set of tessellating boxes.
///
/// The smallest scale used is a box size of 1, and the largest scale is at most half the smallest image size (i.e.
/// `min(width,height)/2`. In between, scales grow exponentially with a factor `1+eta`. Thus, if `eta` is 1,
/// then each scale uses boxes double the size of the previous scale, and if `eta` is smaller then the steps are
/// smaller and more scales are generated.
///
/// The image `in` must be scalar and binary, and typically is applied to an edge map of objects.
DIP_EXPORT dfloat FractalDimension(
      Image const& in,
      dfloat eta = 0.5
);


/// \}

} // namespace dip

#endif // DIP_ANALYSIS_H
