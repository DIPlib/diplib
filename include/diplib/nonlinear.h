/*
 * DIPlib 3.0
 * This file contains declarations for non-linear image filters
 *
 * (c)2017-2019, Cris Luengo.
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
/// \brief Non-linear filters.
/// \see nonlinear


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
/// Uses `dip::FastVarianceAccumulator` for the computation.
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
/// Nonetheless, this can used to implement color morphology, for example (note there are much better approaches to
/// build the `control` image):
///
/// ```cpp
///     // Image in is a color image
///     Image control = dip::SumTensorElements( in );
///     Image out = dip::SelectionFilter( in, control, kernel, 0.0, "maximum" );
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
      String const& mode = S::MINIMUM,
      StringArray const& boundaryCondition = {}
);
inline Image SelectionFilter(
      Image const& in,
      Image const& control,
      Kernel const& kernel = {},
      dfloat threshold = 0.0,
      String const& mode = S::MINIMUM,
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
/// `threshold` controls how much lower the minimum must be. If the neighborhood is uniform w.r.t. this threshold
/// parameter, then the filtering window is not shifted.
///
/// The size and shape of the filter window is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See `dip::Kernel`.
///
/// If `in` is non-scalar (e.g. a color image), then the variance is computed per-channel, and the maximum variance
/// at each pixel (i.e. the maximum across tensor elements) is used to direct the filtering for all channels.
/// If the Kuwahara filter were applied to each channel independently, false colors would appear.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// \literature
/// <li>M. Kuwahara, K. Hachimura and M. Kinoshita, "Image enhancement and left ventricular contour extraction techniques
///     applied to radioisotope angiocardiograms", Automedica 3:107-119, 1980.
/// <li>M. Nagao and T. Matsuyama, "Edge preserving smoothing", Computer Graphics and %Image Processing 9:394-407, 1979.
/// <li>P. Bakker, P.W. Verbeek and L.J. van Vliet, "Edge preserving orientation adaptive filtering", in: CVPR’99 2:535–540, 1999.
/// <li>P. Bakker, "Image structure analysis for seismic interpretation", PhD Thesis, Delft University of Technology,
///     The Netherlands, 2002.
/// \endliterature
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


/// \brief Non-maximum suppression, as used in the Canny edge detector.
///
/// `out` contains the value of `gradmag` where `gradmag` is a local maximum in the orientation
/// specified by the vector image `gradient`. Note that `gradmag` does not need to be the magnitude
/// of `gradient`, and that only the direction of the vectors (or orientation) is used.
///
/// `gradmag` and `gradient` must be of the same floating-point type (i.e. they are either
/// `dip::DT_SFLOAT` or `dip::DT_DFLOAT`). `gradmag` must be scalar, and `gradient` must have as
/// many tensor elements as spatial dimensions. In the 1D case, `gradient` is not used.
///
/// If `gradmag` is not forged, the magnitude (`dip::Norm`) of `gradient` is used instead.
///
/// `mask`, if forged, must be a binary scalar image. Only those pixels are evaluated that are set in `mask`.
///
/// All three input images (if forged) must have the same spatial dimensions.
///
/// `mode` can be "interpolate" or "round". The interpolating mode is only valid in 2D; the gradient magnitude
/// is interpolated to take into account all information present in the direction of the gradient. The rounding
/// mode rounds the angles to point to the nearest neighbor.
/// For higher-dimensional images, gradients are always rounded.
DIP_EXPORT void NonMaximumSuppression(
      Image const& gradmag,
      Image const& gradient,
      Image const& mask,
      Image& out,
      String const& mode = S::INTERPOLATE
);
inline Image NonMaximumSuppression(
      Image const& gradmag,
      Image const& gradient,
      Image const& mask,
      String const& mode = S::INTERPOLATE
) {
   Image out;
   NonMaximumSuppression( gradmag, gradient, mask, out, mode );
   return out;
}

/// \brief Given a sparse binary image `bin`, moves each set pixel to the pixel in the 3x3 neighborhood with
/// lowest `weight`.
///
/// The neighborhood used is 3x3 in 2D, or 3x3x3 in 3D.
/// In other words, the connectivity is equal to `bin.Dimensionality()`.
///
/// Note that the output doesn't necessarily have the same number of set pixels as the `bin` input. However,
/// it will not have more. To move pixels over a larger distance, call this function repeatedly.
///
/// `out` will have the same properties as `bin`. `bin` must be binary, scalar, and have at least one dimension.
/// `weights` must be real-valued, scalar, and of the same sizes as `bin`. No singleton expansion is applied.
DIP_EXPORT void MoveToLocalMinimum(
      Image const& bin,
      Image const& weights,
      Image& out
);
inline Image MoveToLocalMinimum(
      Image const& bin,
      Image const& weights
) {
   Image out;
   MoveToLocalMinimum( bin, weights, out );
   return out;
}


/// \brief Applies Perona-Malik anisotropic diffusion
///
/// Applies `iterations` steps of the anisotropic diffusion as proposed by Perona and Malik:
///
/// \f[ I^{t+1} = I^t + \lambda \sum_\eta \left( c_\eta^t \nabla_\eta I^t \right) \; , \f]
///
/// where \f$\lambda\f$ is set with the `lambda` parameter, \f$\eta\f$ are the each of the cardinal directions,
/// \f$\nabla_\eta\f$ is the finite difference in direction \f$\eta\f$,
///
/// \f[ c_\eta^t = g\left( \| \nabla_\eta I^t \| \right) \; , \f]
///
/// and \f$g\f$ is a monotonically decreasing function, selected with the `g` parameter, and modulated
/// by the `K` parameter:
///  - `"Gauss"`: \f$ g(x) = \exp(-(\frac{x}{K})^2) \f$
///  - `"quadratic"`: \f$ g(x) = 1 / (1 + (\frac{x}{K})^2) \f$
///  - `"exponential"`: \f$ g(x) = \exp(-\frac{x}{K}) \f$
///
/// The diffusion is generalized to any image dimensionality. `in` must be scalar and real-valued.
///
/// \literature
/// <li>P. Perona and J. Malik, "Scale-space and edge detection using anisotropic diffusion",
///     IEEE Transactions on Pattern Analysis and Machine Intelligence 12(7):629:639, 1990.
/// \endliterature
DIP_EXPORT void PeronaMalikDiffusion(
      Image const& in,
      Image& out,
      dip::uint iterations = 5,
      dfloat K = 10,
      dfloat lambda = 0.25,
      String const& g = "Gauss"
);
inline Image PeronaMalikDiffusion(
      Image const& in,
      dip::uint iterations = 5,
      dfloat K = 10,
      dfloat lambda = 0.25,
      String const& g = "Gauss"
) {
   Image out;
   PeronaMalikDiffusion( in, out, iterations, K, lambda, g );
   return out;
}

/// \brief Applies iterative generic anisotropic diffusion using Gaussian derivatives
///
/// Applies `iterations` steps of the generic anisotropic diffusion equation:
///
/// \f[ I^{t+1} = I^t + \lambda \, \mathrm{div} \left( c^t \nabla I^t \right) \; , \f]
///
/// where \f$\lambda\f$ is set with the `lambda` parameter, \f$\nabla\f$ and \f$\mathrm{div}\f$ are computed using
/// Gaussian gradients (`dip::Gradient` and `dip::Divergence`),
///
/// \f[ c^t = g\left( \| \nabla I^t \| \right) \; , \f]
///
/// and \f$g\f$ is a monotonically decreasing function, selected with the `g` parameter, and modulated
/// by the `K` parameter:
///  - `"Gauss"`: \f$ g(x) = \exp(-(\frac{x}{K})^2) \f$
///  - `"quadratic"`: \f$ g(x) = 1 / (1 + (\frac{x}{K})^2) \f$
///  - `"exponential"`: \f$ g(x) = \exp(-\frac{x}{K}) \f$
///
/// Note that the parameters here are identical to those in `dip::PeronaMalik`. The Perona-Malik diffusion
/// is a discrete differences approximation to the generic anisotropic diffusion equation. This function uses Gaussian
/// gradients as a discretization strategy.
///
/// The diffusion is generalized to any image dimensionality. `in` must be scalar and real-valued.
DIP_EXPORT void GaussianAnisotropicDiffusion(
      Image const& in,
      Image& out,
      dip::uint iterations = 5,
      dfloat K = 10,
      dfloat lambda = 0.25,
      String const& g = "Gauss"
);
inline Image GaussianAnisotropicDiffusion(
      Image const& in,
      dip::uint iterations = 5,
      dfloat K = 10,
      dfloat lambda = 0.25,
      String const& g = "Gauss"
) {
   Image out;
   GaussianAnisotropicDiffusion( in, out, iterations, K, lambda, g );
   return out;
}

/// \brief Applies iterative robust anisotropic diffusion
///
/// Applies `iterations` steps of the robust anisotropic diffusion using Tukey's biweight (Black et al., 1998):
///
/// \f[ I^{t+1} = I^t + \lambda \sum_\eta \psi ( \nabla_\eta I^t, \sigma ) \; , \f]
///
/// where \f$\lambda\f$ is set with the `lambda` parameter, \f$\eta\f$ are each of the cardinal directions,
/// \f$\nabla_\eta\f$ is the finite difference in direction \f$\eta\f$, and
///
/// \f[
///    \psi(x,\sigma) =
///       \begin{cases}
///          x\,\left(1-\frac{x^2}{\sigma^2}\right)^2, & \text{if}\ |x| < \sigma
///       \\ 0, & \text{otherwise}
///       \end{cases}
/// \f]
///
/// \f$\sigma\f$ is set by the `sigma` parameter.
///
/// The diffusion is generalized to any image dimensionality. `in` must be scalar and real-valued.
///
/// \literature
/// <li>M.J. Black, G. Sapiro, D.H. Marimont and D. Heeger, "Robust anisotropic diffusion",
///     IEEE Transactions on %Image Processing 7(3):421-432, 1998.
/// \endliterature
inline void RobustAnisotropicDiffusion(
      Image const& in,
      Image& out,
      dip::uint iterations = 5,
      dfloat sigma = 10,
      dfloat lambda = 0.25
) {
   PeronaMalikDiffusion( in, out, iterations, sigma, lambda, "Tukey" );
}
inline Image RobustAnisotropicDiffusion(
      Image const& in,
      dip::uint iterations = 5,
      dfloat sigma = 10,
      dfloat lambda = 0.25
) {
   Image out;
   RobustAnisotropicDiffusion( in, out, iterations, sigma, lambda );
   return out;
}

/// \brief Applies iterative coherence enhancing (anisotropic) diffusion
///
/// Applies `iterations` steps of the coherence enhancing diffusion:
///
/// \f[ I^{t+1} = I^t + \lambda \, \mathrm{div} \left( D \nabla I^t \right) \; , \f]
///
/// where \f$\lambda\f$ is set with the `lambda` parameter, and \f$D\f$ is the diffusion tensor, derived from
/// the structure tensor (see `dip::StructureTensor`). `derivativeSigma` and `regularizationSigma`
/// are the sigmas for the Gaussian derivatives and smoothing in the structure tensor. The gradient and
/// divergence are computed using Gaussian derivatives also, using a sigma of 0.5.
///
/// `flags` allows the selection of different computational options:
/// - `"const"`: \f$D\f$ is taken as constant, simplifying the computation from
///   \f$ \frac{\partial}{\partial x} \left( D_{xx} \frac{\partial}{\partial x} I^t \right) \f$
///   to \f$ D_{xx} \frac{\partial^2}{\partial x^2} I^t \f$, reducing the number of filters to apply from
///   4 to 3. The opposite is `"variable"`, which is the default.
/// - `"all"`: \f$D\f$ is obtained in a simple manner from the structure tensor, where all eigenvalues of \f$D\f$
///   are adjusted. The opposite is `"first"`, which is the default. See below for more information.
/// - `"resample"`: the output is twice the size of the input. Computations are always done on the larger image,
///   this flag returns the larger image instead of the subsampled one.
///
/// This function can be applied to images with two or more dimensions. `in` must be scalar and real-valued.
/// The `"first"` flag is only supported for 2D images, if `in` has more dimensions, the `"first"` flag is
/// ignored and `"all"` is assumed.
///
/// In `"all"` mode, \f$D\f$ is composed from the eigen decomposition of the structure tensor \f$S\f$:
///
/// \f[ S = V \, E \, V^T \; \rightarrow \; D = V \, E' \, V^T \; , \f]
///
/// with
///
/// \f[ E' = \frac{1}{\mathrm{trace}\,E^{-1}} \, E^{-1} \f]
///
/// In `"first"` mode, \f$D\f$ is composed similarly, but the two eigenvalues of \f$D\f$, \f$d_i\f$, are determined
/// from the eigenvalues \f$\mu_i\f$ of \f$S\f$ (with \f$\mu_1 \ge \mu_2\f$) as follows:
///
/// \f{eqnarray*}{
///       d_1 &=& \alpha
///    \\ d_2 &=& \begin{cases}
///                    \alpha + ( 1.0 - \alpha ) \exp\left(\frac{-c}{(\mu_1 - \mu_2)^2}\right) \, ,
///                                & \text{if}\ \frac{\mu_1 - \mu_2}{\mu_1 + \mu_2} > \alpha \; \text{(high anisotropy)}
///                 \\ \alpha \, , & \text{otherwise}
///               \end{cases}
/// \f}
///
/// \f$\alpha\f$ is a magic number set to 0.01, and \f$c\f$ is set to the median of all \f$\mu_2^2\f$
/// values across the image (as proposed by Lucas van Vliet).
///
/// \literature
/// <li>J. Weickert, "Anisotropic diffusion in image processing", Teubner (Stuttgart), pages 95 and 127, 1998.
/// \endliterature
DIP_EXPORT void CoherenceEnhancingDiffusion(
      Image const& in,
      Image& out,
      dfloat derivativeSigma = 1,
      dfloat regularizationSigma = 3,
      dip::uint iterations = 5,
      StringSet const& flags = {}
);
inline Image CoherenceEnhancingDiffusion(
      Image const& in,
      dfloat derivativeSigma = 1,
      dfloat regularizationSigma = 3,
      dip::uint iterations = 5,
      StringSet const& flags = {}
) {
   Image out;
   CoherenceEnhancingDiffusion( in, out, derivativeSigma, regularizationSigma, iterations, flags );
   return out;
}

/// \brief Adaptive Gaussian filtering.
///
/// One or more parameter images control the adaptivity.
/// The meaning of the parameter images depend on the dimensionality of the input image.
/// The current implementation only supports 2D and 3D images.
///
/// - 2D:
///     - `params[0]` is the angle of the orientation
///     - `params[1]` (optional) is a tensor image with the local kernel scale
///
/// - 3D (with 1D structures):
///     - `params[0]` is the polar coordinate phi of the first orientation
///     - `params[1]` is the polar coordinate theta of the first orientation
///     - `params[2]` (optional) is a tensor image with the local kernel scale
///
/// - 3D (with 2D structures):
///     - `params[0]` is the polar coordinate phi of the first orientation
///     - `params[1]` is the polar coordinate theta of the first orientation
///     - `params[2]` is the polar coordinate phi of the second orientation
///     - `params[3]` is the polar coordinate theta of the second orientation
///     - `params[4]` (optional) is a tensor image with the local kernel scale
///
/// For intrinsic 1D structures, pass one set of polar coordinates. For intrinsic 2d structures, pass two.
///
/// The kernel scale parameter image is interpreted as follows.
/// Each input tensor element corresponds with a tensor row in the scale image.
/// Each tensor column in the scale image corresponds with a convolution kernel dimension.
/// As an example, consider a 2D RGB image. The scale tensor is then interpreted as:
///
/// ```txt
///     | R_kx R_ky |
///     | G_kx G_ky |
///     | B_kx B_ky |
/// ```
///
/// The kernel is first scaled and then rotated before it is applied.
/// The scale parameter image is automatically expanded if the image or the tensor is too small.
/// If the scale tensor has one element, it is expanded to all input tensor elements and kernel dimensions.
/// If the scale tensor has a single column, each element is expanded to all kernel dimensions.
/// For more information on scaling, also see "Structure-adaptive applicability function" in Pham et al. (2006).
///
/// The sigma for each kernel dimension is passed by `sigmas`.
/// For intrinsic 1D structures, the first value is along the contour, the second perpendicular to it.
/// For intrinsic 2D structures, the first two are in the plane, whereas the other is perpendicular to them.
/// If a value is zero, no convolution is done is this direction.
///
/// Together with `sigmas`, the `orders`, `truncation` and `exponents` parameters define the gaussian kernel.
/// `interpolationMethod` can be `"linear"` (default) or `"zero order"` (faster).
/// As of yet, `boundaryCondition` can only be "mirror" or "add zeros".
///
/// # Example:
///
/// ```cpp
///     dip::Image in = dip::ImageReadTIFF( "erika.tif" );     // Defined in "diplib/file_io.h"
///     dip::Image st = dip::StructureTensor( in, {}, 1, 3 );  // Defined in "diplib/analysis.h"
///     dip::ImageArray params = dip::StructureTensorAnalysis( st, { "orientation" } );
///     dip::Image out = dip::AdaptiveBanana( in, dip::CreateImageConstRefArray( params ), { 2, 0 } );
/// ```
///
/// \literature
/// <li>T.Q. Pham, L.J. van Vliet and K. Schutte, "Robust fusion of irregularly sampled data using adaptive normalized
///     convolution", EURASIP Journal on Applied Signal Processing, article ID 83268, 2006.
/// \endliterature
///
/// \see dip::AdaptiveBanana, dip::StructureTensorAnalysis2D, dip::StructureTensorAnalysis3D
DIP_EXPORT void AdaptiveGauss(
      Image const& in,
      ImageConstRefArray const& params,
      Image& out,
      FloatArray const& sigmas = { 5.0, 1.0 },
      UnsignedArray const& orders = { 0 },
      dfloat truncation = 2.0,
      UnsignedArray const& exponents = { 0 },
      String const& interpolationMethod = S::LINEAR,
      String const& boundaryCondition = S::SYMMETRIC_MIRROR
);
inline Image AdaptiveGauss(
      Image const& in,
      ImageConstRefArray const& params,
      FloatArray const& sigmas = { 5.0, 1.0 },
      UnsignedArray const& orders = { 0 },
      dfloat truncation = 2.0,
      UnsignedArray const& exponents = { 0 },
      String const& interpolationMethod = S::LINEAR,
      String const& boundaryCondition = S::SYMMETRIC_MIRROR
) {
   Image out;
   AdaptiveGauss( in, params, out, sigmas, orders, truncation, exponents, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Adaptive Gaussian filtering using curvature.
///
/// The parameter images control the adaptivity.
/// The current implementation only supports 2D images:
/// - `params[0]` is the angle of the orientation
/// - `params[1]` is the curvature
/// - `params[2]` (optional) is a tensor image with the local kernel scale
///
/// The kernel scale parameter image is interpreted as follows.
/// Each input tensor element corresponds with a tensor row in the scale image.
/// Each tensor column in the scale image corresponds with a convolution kernel dimension.
/// As an example, consider a 2D RGB image. The scale tensor is then interpreted as:
///
/// ```txt
///     | R_kx R_ky |
///     | G_kx G_ky |
///     | B_kx B_ky |
/// ```
///
/// The kernel is first scaled and then rotated before it is applied.
/// The scale parameter image is automatically expanded if the image or the tensor is too small.
/// If the scale tensor has one element, it is expanded to all input tensor elements and kernel dimensions.
/// If the scale tensor has a single column, each element is expanded to all kernel dimensions.
/// For more information on scaling, also see "Structure-adaptive applicability function" in in Pham et al. (2006).
///
/// The sigma for each kernel dimension is passed by `sigmas`. The first value is along the contour,
/// the second perpendicular to it. If a value is zero, no convolution is done is this direction.
///
/// Together with `sigmas`, the `orders`, `truncation` and `exponents` parameters define the gaussian kernel.
/// `interpolationMethod` can be `"linear"` (default) or `"zero order"` (faster).
/// As of yet, `boundaryCondition` can only be "mirror" or "add zeros".
///
/// # Example:
///
/// ```cpp
///     dip::Image in = dip::ImageReadTIFF( "erika.tif" );     // Defined in "diplib/file_io.h"
///     dip::Image st = dip::StructureTensor( in, {}, 1, 3 );  // Defined in "diplib/analysis.h"
///     dip::ImageArray params = dip::StructureTensorAnalysis( st, { "orientation", "curvature" } );
///     dip::Image out = dip::AdaptiveBanana( in, dip::CreateImageConstRefArray( params ), { 2, 0 } );
/// ```
///
/// \literature
/// <li>T.Q. Pham, L.J. van Vliet and K. Schutte, "Robust fusion of irregularly sampled data using adaptive normalized
///     convolution", EURASIP Journal on Applied Signal Processing, article ID 83268, 2006.
/// \endliterature
///
/// \see dip::AdaptiveGauss, dip::StructureTensorAnalysis2D
DIP_EXPORT void AdaptiveBanana(
      Image const& in,
      ImageConstRefArray const& params,
      Image& out,
      FloatArray const& sigmas = { 5.0, 1.0 },
      UnsignedArray const& orders = { 0 },
      dfloat truncation = 2.0,
      UnsignedArray const& exponents = { 0 },
      String const& interpolationMethod = S::LINEAR,
      String const& boundaryCondition = S::SYMMETRIC_MIRROR
);
inline Image AdaptiveBanana(
      Image const& in,
      ImageConstRefArray const& params,
      FloatArray const& sigmas = { 5.0, 1.0 },
      UnsignedArray const& orders = { 0 },
      dfloat truncation = 2.0,
      UnsignedArray const& exponents = { 0 },
      String const& interpolationMethod = S::LINEAR,
      String const& boundaryCondition = S::SYMMETRIC_MIRROR
) {
   Image out;
   AdaptiveBanana( in, params, out, sigmas, orders, truncation, exponents, interpolationMethod, boundaryCondition );
   return out;
}

/// \brief Bilateral filter, brute-force full kernel implementation
///
/// The bilateral filter is a non-linear edge-preserving smoothing filter. It locally averages input pixels,
/// weighting them with both the spatial distance to the origin as well as the intensity difference with the
/// pixel at the origin. The weights are Gaussian, and therefore there are two sigmas as parameters. The
/// spatial sigma can be defined differently for each image dimension in `spatialSigma`. `tonalSigma` determines
/// what similar intensities are. `truncation` applies to the spatial dimension only, and determines, together
/// with `spatialSigma`, the size of the neighborhood and thus its computational cost.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// If `in` is not scalar, each tensor element will be filtered independently. For color images, this leads to
/// false colors at edges.
///
/// The optional image `estimate`, if forged, is used as the tonal center when computing the kernel at each pixel.
/// That is, each point in the kernel is computed based on the distance of the corresponding pixel value in `in`
/// to the value of the pixel at the origin of the kernel in `estimate`. If not forged, `in` is used for `estimate`.
/// `estimate` must be real-valued and have the same sizes and number of tensor elements as `in`.
///
/// \literature
/// <li>C. Tomasi and R. Manduchi, "Bilateral filtering for gray and color images", Proceedings of the 1998 IEEE
///     International Conference on Computer Vision, Bombay, India.
/// \endliterature
DIP_EXPORT void FullBilateralFilter(
      Image const& in,
      Image const& estimate,
      Image& out,
      FloatArray spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
);
inline Image FullBilateralFilter(
      Image const& in,
      Image const& estimate = {},
      FloatArray const& spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   FullBilateralFilter( in, estimate, out, spatialSigmas, tonalSigma, truncation, boundaryCondition );
   return out;
}

/// \brief Quantized (piecewise linear) bilateral filter
///
/// The bilateral filter is a non-linear edge-preserving smoothing filter. It locally averages input pixels,
/// weighting them with both the spatial distance to the origin as well as the intensity difference with the
/// pixel at the origin. The weights are Gaussian, and therefore there are two sigmas as parameters. The
/// spatial sigma can be defined differently for each image dimension in `spatialSigma`. `tonalSigma` determines
/// what similar intensities are. `truncation` applies to the spatial dimension only, and determines, together
/// with `spatialSigma`, the size of the neighborhood and thus its computational cost.
///
/// This version of the filter applies a piece-wise linear approximation as described by Durand and Dorsey,
/// but without subsampling. This requires a significant amount of memory, and is efficient only for larger
/// spatial sigmas.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// `in` must be scalar and real-valued.
///
/// The optional image `estimate`, if forged, is used as the tonal center when computing the kernel at each pixel.
/// That is, each point in the kernel is computed based on the distance of the corresponding pixel value in `in`
/// to the value of the pixel at the origin of the kernel in `estimate`. If not forged, `in` is used for `estimate`.
/// `estimate` must be real-valued and have the same sizes and number of tensor elements as `in`.
///
/// \literature
/// <li>F. Durand and J. Dorsey, "Fast bilateral filtering for the display of high-dynamic-range images",
///     ACM Transactions on Graphics 21(3), 2002.
/// \endliterature
DIP_EXPORT void QuantizedBilateralFilter(
      Image const& in,
      Image const& estimate,
      Image& out,
      FloatArray spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      FloatArray tonalBins = {},
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
);
inline Image QuantizedBilateralFilter(
      Image const& in,
      Image const& estimate = {},
      FloatArray const& spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      FloatArray const& tonalBins = {},
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   QuantizedBilateralFilter( in, estimate, out, spatialSigmas, tonalSigma, tonalBins, truncation, boundaryCondition );
   return out;
}

/// \brief Separable bilateral filter, a very fast approximation
///
/// The bilateral filter is a non-linear edge-preserving smoothing filter. It locally averages input pixels,
/// weighting them with both the spatial distance to the origin as well as the intensity difference with the
/// pixel at the origin. The weights are Gaussian, and therefore there are two sigmas as parameters. The
/// spatial sigma can be defined differently for each image dimension in `spatialSigma`. `tonalSigma` determines
/// what similar intensities are. `truncation` applies to the spatial dimension only, and determines, together
/// with `spatialSigma`, the size of the neighborhood and thus its computational cost.
///
/// This version of the filter applies a 1D bilateral filter along each of the image dimensions, approximating
/// the result of the bilateral filter with a much reduced computational cost.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// If `in` is not scalar, each tensor element will be filtered independently. For color images, this leads to
/// false colors at edges.
///
/// The optional image `estimate`, if forged, is used as the tonal center when computing the kernel at each pixel.
/// That is, each point in the kernel is computed based on the distance of the corresponding pixel value in `in`
/// to the value of the pixel at the origin of the kernel in `estimate`. If not forged, `in` is used for `estimate`.
/// `estimate` must be real-valued and have the same sizes and number of tensor elements as `in`.
///
/// \literature
/// <li>T.Q. Pham and L.J. van Vliet, "Separable bilateral filter for fast video processing", IEEE International
///     Conference on Multimedia and Expo, 2005.
/// \endliterature
DIP_EXPORT void SeparableBilateralFilter(
      Image const& in,
      Image const& estimate,
      Image& out,
      BooleanArray const& process = {},
      FloatArray spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
);
inline Image SeparableBilateralFilter(
      Image const& in,
      Image const& estimate = {},
      BooleanArray const& process = {},
      FloatArray const& spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   SeparableBilateralFilter( in, estimate, out, process, spatialSigmas, tonalSigma, truncation, boundaryCondition );
   return out;
}

/// \brief Bilateral filter, convenience function that allows selecting an implementation
///
/// The `method` can be set to one of the following:
/// - `"full"`: the brute-force implementation, using the full kernel, calls `dip::FullBilateralFilter`.
/// - `"xysep"` (default): xy-separable approximation, calls `dip::SeparableBilateralFilter`.
/// - `"pwlinear"`: piecewise linear approximation (quantized), calls `dip::QuantizedBilateralFilter`.
///   The bins are automatically computed.
///
/// See the linked functions for details on the other parameters.
// TODO: Implement Paris and Durand (2006): http://people.csail.mit.edu/sparis/bf/
// https://people.csail.mit.edu/sparis/publi/2009/ijcv/Paris_09_Fast_Approximation.pdf
// TODO: Implement cross-bilateral filter
// TODO: Implement bilateral filters correctly for tensor images (how to define distance? Simple answer: weigh all tensor elements equally. Is there a reason to do it differently?)
DIP_EXPORT void BilateralFilter(
      Image const& in,
      Image const& estimate,
      Image& out,
      FloatArray const& spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      String const& method = "xysep",
      StringArray const& boundaryCondition = {}
);
inline Image BilateralFilter(
      Image const& in,
      Image const& estimate = {},
      FloatArray const& spatialSigmas = { 2.0 },
      dfloat tonalSigma = 30.0,
      dfloat truncation = 2.0,
      String const& method = "xysep",
      StringArray const& boundaryCondition = {}
) {
   Image out;
   BilateralFilter( in, estimate, out, spatialSigmas, tonalSigma, truncation, method, boundaryCondition );
   return out;
}

/// \}

} // namespace dip

#endif // DIP_NONLINEAR_H
