/*
 * DIPlib 3.0
 * This file contains declarations for deconvolution functions
 *
 * (c)2017-2022, Cris Luengo.
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

#ifndef DIP_DECONVOLUTION_H
#define DIP_DECONVOLUTION_H

#include "diplib.h"


/// \file
/// \brief Deconvolution algorithms.
/// See \ref deconvolution.


namespace dip {


/// \group deconvolution Deconvolution
/// \ingroup filtering
/// \brief Deconvolution algorithms (inverse filtering).
/// \addtogroup


/// \brief Wiener deconvolution using estimates of signal and noise power spectra.
///
/// Assuming some original image $f$, a known convolution kernel $h$ (given by `psf`), a noise realization $n$,
/// and an observed image $g = h * f + n$ (given by `in`), the Wiener deconvolution filter is the linear filter
/// $h_\text{inv}$ that, when convolved with $g$, yields an image $\hat{f} = h_\text{inv} * g$ such that the mean
/// square error between $f$ and $\hat{f}$ is minimized.
///
/// Finding $\hat{f}$ (returned in `out`) requires knowledge of the power spectra of the signal and the noise.
/// The Wiener deconvolution filter is defined in the frequency domain as
///
/// $$ H_\text{inv} = \frac{H^* S}{ H^* H S + N } \; , $$
///
/// where $G$ is the Fourier transform of `in`, $H$ is the Fourier transform of `psf`, $S$ is `signalPower`,
/// and $N$ is `noisePower`. These $S$ and $N$ are typically not known, but:
///
/// - `signalPower` can be estimated as the Fourier transform of the autocorrelation of `in`. If a raw image
///   is passed for this argument (`dip::Image{}`), then it will be computed as such.
///
/// - `noisePower` can be estimated as a flat function, assuming white noise. A 0D image can be given here,
///   it will be expanded to the size of the other images. `noisePower` should not be zero anywhere, as that
///   might lead to division by zero and consequently meaningless results.
///
/// The other syntax for `dip::WienerDeconvolution` takes an estimate of the noise-to-signal
/// ratio instead of the signal and noise power spectra. Note that $H_\text{inv}$ can be rewritten as
///
/// $$ H_\text{inv} = \frac{H^*}{ H^* H  + \frac{N}{S} } = \frac{H^*}{ H^* H + K } \; , $$
///
/// where $K$ is the noise-to-signal ratio. If $K$ is a constant, then the Wiener deconvolution filter is
/// equivalent to the Tikhonov regularized inverse filter.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
///
/// If `"pad"` is in `options`, then `in` is padded by the size of `psf` in all directions (padded area
/// is filled by mirroring at the image border). This significantly reduces the effects of the periodicity
/// of the frequency-domain convolution. `"pad"` cannot be combined with `"OTF"`.
///
/// !!! literature
///     - G.M.P. van Kempen, "Image Restoration in Fluorescence Microscopy",
///       PhD Thesis, Delft University of Technology, Delft, The Netherlands, 1998.
DIP_EXPORT void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image const& signalPower,
      Image const& noisePower,
      Image& out,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image const& signalPower,
      Image const& noisePower,
      StringSet const& options = { S::PAD }
) {
   Image out;
   WienerDeconvolution( in, psf, signalPower, noisePower, out, options );
   return out;
}

/// \brief Wiener deconvolution using an estimate of noise-to-signal ratio.
///
/// See the description of the function with the same name above. The difference here is that a single number,
/// `regularization`, is given instead of the signal and noise power spectra. We then set $K$ (the
/// noise-to-signal ratio) to `regularization * dip::Maximum(P)`, with `P` equal to $H^* H$.
///
/// This formulation of the Wiener deconvolution filter is equivalent to the Tikhonov regularized inverse filter.
///
/// !!! literature
///     - G.M.P. van Kempen, "Image Restoration in Fluorescence Microscopy",
///       PhD Thesis, Delft University of Technology, Delft, The Netherlands, 1998.
DIP_EXPORT void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 1e-4,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image WienerDeconvolution(
      Image const& in,
      Image const& psf,
      dfloat regularization = 1e-4,
      StringSet const& options = { S::PAD }
) {
   Image out;
   WienerDeconvolution( in, psf, out, regularization, options );
   return out;
}

/// \brief Tikhonov-Miller deconvolution.
///
/// Assuming some original image $f$, a known convolution kernel $h$ (given by `psf`), a noise realization $n$,
/// and an observed image $g = h * f + n$ (given by `in`), the Tikhonov-Miller deconvolution filter is the linear filter
/// $h_\text{inv}$ that, when convolved with $g$, yields an image $\hat{f} = h_\text{inv} * g$ that minimizes the Tikhonov
/// functional,
///
/// $$ \Theta(\hat{f}) = \left\lVert h * \hat{f} - g \right\rVert^2 + \lambda \left\lVert c * \hat{f} \right\rVert^2 \; , $$
///
/// where $\lambda$ is the regularization parameter (given by `regularization`), and $c$ is the regularization kernel,
/// for which we use an ideal Laplace kernel here. $\hat{f}$ is returned in `out`.
///
/// In the frequency domain, the Tikhonov-Miller deconvolution filter is defined as
///
/// $$ H_\text{inv} = \frac{H^*}{ H^* H  + \lambda C^* C } \; , $$
///
/// where $G$ is the Fourier transform of `in`, $H$ is the Fourier transform of `psf`, and $C$ is the regularization
/// kernel in the frequency domain.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
///
/// If `"pad"` is in `options`, then `in` is padded by the size of `psf` in all directions (padded area
/// is filled by mirroring at the image border). This significantly reduces the effects of the periodicity
/// of the frequency-domain convolution. `"pad"` cannot be combined with `"OTF"`.
///
/// !!! literature
///     - G.M.P. van Kempen, "Image Restoration in Fluorescence Microscopy",
///       PhD Thesis, Delft University of Technology, Delft, The Netherlands, 1998.
DIP_EXPORT void TikhonovMiller(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 0.1,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image TikhonovMiller(
      Image const& in,
      Image const& psf,
      dfloat regularization = 0.1,
      StringSet const& options = { S::PAD }
) {
   Image out;
   TikhonovMiller( in, psf, out, regularization, options );
   return out;
}

/// \brief Iterative Constrained Tikhonov-Miller (ICTM) deconvolution.
///
/// Assuming some original image $f$, a known convolution kernel $h$ (given by `psf`), a noise realization $n$,
/// and an observed image $g = h * f + n$ (given by `in`), ICTM deconvolution finds the $\hat{f}$ (returned in `out`)
/// that minimizes the Tikhonov functional,
///
/// $$ \Theta(\hat{f}) = \left\lVert h * \hat{f} - g \right\rVert^2 + \lambda \left\lVert c * \hat{f} \right\rVert^2 \; , $$
///
/// where $\lambda$ is the regularization parameter (given by `regularization`), and $c$ is the regularization kernel,
/// for which we use an ideal Laplace kernel here. $\hat{f}$ is returned in `out`.
///
/// If `stepSize` is 0 (the default), ICTM uses the conjugate gradient method to estimate $\hat{f}$. In this case, it
/// uses the results of Verveer and Jovin to estimate the optimal step size for each step.
///
/// If a positive step size is given (a value in the range (0, 1]), then ICTM uses gradient descent (with steepest
/// descent) and a fixed step size. This is much simpler code, with quicker steps, but converges much more slowly and
/// can even diverge under certain circumstances. It is provided here because this is a common implementation in other
/// software packages; we do not recommend using it.
///
/// The iterative algorithm is stopped when the maximum difference of $\Theta(\hat{f})$ between two steps (ignoring
/// the regularization term) is less than `tolerance` times the maximum absolute value in $g$.
///
/// `maxIterations` provides an additional stopping condition, in case the algorithm does not converge quickly
/// enough. In a way, providing a small maximum number of iterations is an additional form of regularization.
/// Setting `maxIterations` to 0 runs the algorithm until convergence.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
///
/// If `"pad"` is in `options`, then `in` is padded by the size of `psf` in all directions (padded area
/// is filled by mirroring at the image border). This significantly reduces the effects of the periodicity
/// of the frequency-domain convolution. `"pad"` cannot be combined with `"OTF"`.
///
/// !!! literature
///     - G.M.P. van Kempen, "Image Restoration in Fluorescence Microscopy",
///       PhD Thesis, Delft University of Technology, Delft, The Netherlands, 1998.
///     - P.J. Verveer and T.M. Jovin, "Acceleration of the ICTM image restoration algorithm",
///       Journal of Microscopy 188(3):191-195, 1997.
DIP_EXPORT void IterativeConstrainedTikhonovMiller(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 0.1,
      dfloat tolerance = 1e-6,
      dip::uint maxIterations = 30,
      dfloat stepSize = 0.0,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image IterativeConstrainedTikhonovMiller(
      Image const& in,
      Image const& psf,
      dfloat regularization = 0.1,
      dfloat tolerance = 1e-6,
      dip::uint maxIterations = 30,
      dfloat stepSize = 0.0,
      StringSet const& options = { S::PAD }
) {
   Image out;
   IterativeConstrainedTikhonovMiller( in, psf, out, regularization, tolerance, maxIterations, stepSize, options );
   return out;
}

/// \brief Richardson-Lucy (RL) deconvolution, also sometimes called the expectation maximization (EM) method.
///
/// Assuming some original image $f$, a known convolution kernel $h$ (given by `psf`), and an observed image
/// $g = P(h * f)$ (given by `in`), where $P(x)$ is Poisson noise with mean $x$, RL deconvolution finds the
/// $\hat{f}$ (returned in `out`) with maximal likelihood, given by
///
/// $$ log p(g | \hat{f}) = \sum g \log(h * \hat{f}) - h * \hat{f} \; , $$
///
/// This is the basic, non-regularized Richardson-Lucy deconvolution, which requires `regularization` to be
/// set to 0.
///
/// The `nIterations` parameter serves as regularization, the iterative process must be stopped before the noise
/// gets amplified too much. Even when using the regularization parameter, there is no ideal way to see if the
/// algorithm has converged.
///
/// If `regularization` is positive, total variation (TV) regularization is added, according to Dey et al. In this
/// case, a term $\lambda \sum |\nabla\hat{f}|$ is added to the expression above, with $\lambda$ equal to
/// `regularization`. This should be a small value, 0.01 is a good start point. Note that TV regularization tends
/// to introduce a stair-case effect, as it penalizes slow transitions but allows sharp jumps.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
///
/// If `"pad"` is in `options`, then `in` is padded by the size of `psf` in all directions (padded area
/// is filled by mirroring at the image border). This significantly reduces the effects of the periodicity
/// of the frequency-domain convolution. `"pad"` cannot be combined with `"OTF"`.
///
/// !!! literature
///     - G.M.P. van Kempen, "Image Restoration in Fluorescence Microscopy",
///       PhD Thesis, Delft University of Technology, Delft, The Netherlands, 1998.
///     - W.H. Richardson, "Bayesian-based iterative method of image restoration",
///       Journal of the Optical Society of America 62(1):55–59, 1972.
///     - L.B. Lucy, "An iterative technique for the rectification of observed distributions",
///       Astronomical Journal 79(6):745–754, 1974.
///     - N. Dey, L. Blanc-Féraud, C. Zimmer, P. Roux, Z. Kam, J. Olivo-Marin, J. Zerubia,
///       "Richardson–Lucy algorithm with total variation regularization for 3D confocal microscope deconvolution",
///       Microscopy Research & Technique 69(4):260–266, 2006.
DIP_EXPORT void RichardsonLucy(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 0.0,
      dip::uint nIterations = 30,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image RichardsonLucy(
      Image const& in,
      Image const& psf,
      dfloat regularization = 0.0,
      dip::uint nIterations = 30,
      StringSet const& options = { S::PAD }
) {
   Image out;
   RichardsonLucy( in, psf, out, regularization, nIterations, options );
   return out;
}

/// \brief Fast Iterative Shrinkage-Thresholding (FISTA) deconvolution.
///
/// Assuming some original image $f$, a known convolution kernel $h$ (given by `psf`), a noise realization $n$,
/// and an observed image $g = h * f + n$ (given by `in`), FISTA deconvolution finds the $\hat{f}$ (returned in `out`)
/// that minimizes the functional
///
/// $$ \Theta(\hat{f}) = \left\lVert h * \hat{f} - g \right\rVert^2 + \lambda \left\lVert W(\hat{f}) \right\rVert_1 \; , $$
///
/// where $\lambda$ is the regularization parameter (given by `regularization`), and $W(\hat{f})$ is a wavelet transform
/// of $\hat{f}$. The $l_1$ regularization is applied in some wavelet domain, assuming that the image has a sparse
/// representation in the wavelet domain. We use the Haar wavelet, due to its computational simplicity (it is also
/// the wavelet used by Beck and Teboulle). $\hat{f}$ is returned in `out`.
///
/// The iterative algorithm is stopped when the maximum difference of $\Theta(\hat{f})$ between two steps (ignoring
/// the regularization term) is less than `tolerance` times the maximum absolute value in $g$.
///
/// `maxIterations` provides an additional stopping condition, in case the algorithm does not converge quickly
/// enough. In a way, providing a small maximum number of iterations is an additional form of regularization.
/// Setting `maxIterations` to 0 runs the algorithm until convergence.
///
/// `nScales` determines how many scales of the Haar wavelet to compute. It defaults to 3, as used by Beck and Teboulle.
/// Increasing this value might be useful for very large images.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
///
/// If `"pad"` is in `options`, then `in` is padded by the size of `psf` in all directions (padded area
/// is filled by mirroring at the image border). This significantly reduces the effects of the periodicity
/// of the frequency-domain convolution. `"pad"` cannot be combined with `"OTF"`.
///
/// !!! literature
///     - A. Beck, M. Teboulle, "A fast iterative shrinkage-thresholding algorithm for linear inverse problems",
///       SIAM Journal on Imaging Sciences 2(1):183–202, 2009.
DIP_EXPORT void FastIterativeShrinkageThresholding(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 0.1,
      dfloat tolerance = 1e-6,
      dip::uint maxIterations = 30,
      dip::uint nScales = 3,
      StringSet const& options = { S::PAD }
);
DIP_NODISCARD inline Image FastIterativeShrinkageThresholding(
      Image const& in,
      Image const& psf,
      dfloat regularization = 0.1,
      dfloat tolerance = 1e-6,
      dip::uint maxIterations = 30,
      dip::uint nScales = 3,
      StringSet const& options = { S::PAD }
) {
   Image out;
   FastIterativeShrinkageThresholding( in, psf, out, regularization, tolerance, maxIterations, nScales, options );
   return out;
}

/// \endgroup

} // namespace dip

#endif // DIP_DECONVOLUTION_H
