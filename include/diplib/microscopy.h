/*
 * DIPlib 3.0
 * This file contains declarations for microscopy-related functionality
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

#ifndef DIP_MICROSCOPY_H
#define DIP_MICROSCOPY_H

#include "diplib.h"


/// \file
/// \brief Microscopy-related functionality.
/// \see microscopy


namespace dip {


/// \defgroup microscopy Microscopy
/// \brief Assorted tools useful in microscopy, some presumably also in astronomy and other applications.
/// \{


/// \brief Applies a logarithmic mapping to a transmittance image to obtain an absorbance image
///
/// The Beer-Lambert law describes how light is attenuated as it travels through an absorbing medium.
/// In brightfield microscopy, this law describes the relationship between the intensity of the transmitted
/// light and the absorbance of the stains on the slide, which provide contrast. The absorbance is influenced
/// by different factors, but for a given dye molecule, the concentration is directly proportional to the
/// absorbance. Thus, estimating absorbance yields an estimate of the relative dye concentration at each image
/// pixel.
///
/// This function applies the following mapping to the intensities \f$I\f$ in image `in`:
///
/// \f[ A = -log_{10}(1-I/I_0) \f]
///
/// with \f$I_0\f$ the intensity of the illumination (`background`), and \f$A\f$ the absorbance written to `out`).
///
/// `background` can be a single value or one value per tensor element (channel) in `in`. `out` will have the same
/// number of tensor elements. It should be estimated from a background region in the image, or from a
/// calibration image taken without a slide in the optical path.
///
/// `in` must be real-valued. Values outside of the range [0,`background`] will be clipped.
/// `out` will be a floating-point type (do not force it to be an integer type, as the output values are typically
/// quite small).
DIP_EXPORT void BeerLambertMapping(
      Image const& in,
      Image& out,
      Image::Pixel const& background
);
inline Image BeerLambertMapping(
      Image const& in,
      Image::Pixel const& background
) {
   Image out;
   BeerLambertMapping( in, out, background );
   return out;
}

/// \brief Applies an exponential mapping to an absorbance image to obtain a transmittance image
///
/// Applies the inverse mapping of `dip::BeerLambertMapping`, simulating the image obtained under a brightfield
/// microscope given the stain densities in the image `in`. `background` is the illumination intensity, values of
/// 0 in the input will be mapped to the value of `background`, whereas larger input values will be mapped to darker
/// values.
///
/// `in` must be real-valued, negative values will be clipped to 0. `out` will be a floating-point type, unless
/// it was protected before calling this function (see `dip::Image::Protect`).
DIP_EXPORT void InverseBeerLambertMapping(
      Image const& in,
      Image& out,
      Image::Pixel const& background = { 255 }
);
inline Image InverseBeerLambertMapping(
      Image const& in,
      Image::Pixel const& background = { 255 }
) {
   Image out;
   InverseBeerLambertMapping( in, out, background );
   return out;
}

/// \brief Unmixes stains in a brightfield absorbance image or a fluorescence emission image.
///
/// **Brightfield**
///
/// A color image, obtained from a brightfield microscope, and converted to an absorbance image by
/// `dip::BeerLambertMapping`, can be separated into individual stains as long as there are no more
/// stains than channels. For an RGB image, up to three stains can be separated. For a multi-spectral
/// image, this number is larger.
///
/// The stain unmixing process requires knowledge of the absorption spectrum of each of the dyes on the slide.
/// These are usually determined using slides especially prepared with a single dye. Alternatively, find small
/// regions in the image where each stain is on its own (not mixed with other dyes). Below is a table
/// with values for some common dyes, which can be used as a first approximation. However, these
/// absorbance values depend on the tissue, tissue preparation and staining protocols, and imaging equipment.
/// Consequently, best results are always obtained with project-specific values.
///
/// The absorption of the dyes in each channel combine linearly with the density of each of the dyes:
///
/// \f[ A_R = S_{R,1} d_1 + S_{R,2} d_2 + S_{R,3} d_3 + ... \f]
///
/// with \f$S_{R,n}\f$ the absorbance of dye \f$n\f$ in the red channel, \f$d_n\f$ the density (concentration) of
/// dye \f$n\f$, and \f$A_R\f$ the total absorbance in the red channel. In matrix notation this leads to
///
/// \f[ A = \mathbf{S} d \f]
///
/// Here, \f$A\f$ is a pixel in the multi-channel absorbance image (`in`), \f$\mathbf{S}\f$ is a matrix that
/// combines absorbance for each dye and each channel, and \f$d\f$ is a vector with the density for each
/// dye (a pixel in `out`). To find \f$d\f$, this linear set of equations needs to be solved. This process
/// is described by Ruifrok (2001). This function computes a Moore-Penrose pseudo-inverse of \f$\mathbf{S}\f$,
/// and applies a per-pixel matrix multiplication with `in` to obtain `out`.
///
/// `stains` is a `std::vector` that contains each of the columns of matrix \f$\mathbf{S}\f$. That is, each element
/// of `stains` is the values of one column of \f$\mathbf{S}\f$, which we refer to as a *stain vector*. These
/// stain vectors are represented by a `dip::Image::Pixel` with the same number of tensor elements as `in`.
/// `stains` cannot have more elements than channels (tensor elements) are in `in`.
/// `out` will contain one channel for each stain. For example, assuming an RGB image with 3 channels, `stains` can
/// have one, two or three elements, each element being a `dip::Image::Pixel` with exactly 3 elements (corresponding
/// to the 3 RGB channels).
///
/// Best results are obtained when each element of `stains` is normalized (i.e. the norm of each stain vector is 1);
/// this function does not normalize these stain vectors. The standard brightfield stain vectors given below are
/// normalized.
///
/// Example:
/// ```cpp
///     dip::Image img = dip::ImageReadTIFF( "brightfield.tif" );
///     img = dip::BeerLambertMapping( img, { 255 } );
///     img = dip::UnmixStains( img, {{ 0.644, 0.717, 0.267 }, { 0.268, 0.570, 0.776 }} );
///     dip::Image nuclei = img[ 0 ];
///     dip::Image dab = img[ 1 ];
/// ```
///
/// **Fluorescence**
///
/// The explanation above translates to fluorescence imaging, replacing 'absorbance' with 'emission'. In the case
/// of fluorescence, `dip::BeerLambertMapping` should not be used.
/// Typically, fluorescence imaging systems are set up such that each channel collects light only from a single dye,
/// but in practice it is not always possible to use dyes with perfectly separated emission spectra. Therefore, there
/// will be cross-talk, i.e. light from one dye is partially recorded in a channel set up for a different dye.
///
/// Again, it is possible to measure the emission intensity in each channel (or channel cross-talk ratios) using
/// slides prepared for the purpose, with a single dye.
///
/// In multi-spectral fluorescence imaging, channels are not set up specifically for each dye. Instead, the spectrum
/// is divided up into a set of channels. Each dye will be visible in a subset of these channels. Measuring the
/// emission strength for each dye in each channel again leads to the data to be written in `stains` to estimate
/// dye densities using this function.
///
/// **Standard brightfield stain vectors**
///
/// Stain name        | RGB absorbance triplet
/// ----------------- | ----------------------
/// AEC               | 0.274, 0.679, 0.680
/// Alican blue       | 0.875, 0.458, 0.158
/// Aniline blue      | 0.853, 0.509, 0.113
/// Azocarmine        | 0.071, 0.977, 0.198
/// DAB               | 0.268, 0.570, 0.776
/// Eosin             | 0.093, 0.954, 0.283
/// Fast blue         | 0.749, 0.606, 0.267
/// Fast red          | 0.214, 0.851, 0.478
/// Feulgen           | 0.464, 0.830, 0.308
/// Hematoxylin       | 0.644, 0.717, 0.267
/// Hematoxylin + PAS | 0.553, 0.754, 0.354
/// Methyl blue       | 0.799, 0.591, 0.105
/// Methyl green      | 0.799, 0.591, 0.105
/// Methylene blue    | 0.553, 0.754, 0.354
/// Orange-G          | 0.107, 0.368, 0.923
/// PAS               | 0.175, 0.972, 0.155
/// Ponceau-Fuchsin   | 0.107, 0.368, 0.923
///
/// **Literature**
///  - A.C. Ruifrok and D.A. Johnston, "Quantification of histochemical staining by color deconvolution,"
///    Analytical and Quantitative Cytology and Histology 23(4):291-299, 2001.
///  - Stain color triplets taken from CellProfiler, [`unmixcolors.py`](https://github.com/CellProfiler/CellProfiler/blob/master/cellprofiler/modules/unmixcolors.py) module.
DIP_EXPORT void UnmixStains(
      Image const& in,
      Image& out,
      std::vector< Image::Pixel > const& stains
);
inline Image UnmixStains(
      Image const& in,
      std::vector< Image::Pixel > const& stains
) {
   Image out;
   UnmixStains( in, out, stains );
   return out;
}

/// \brief Composes a color image given stain densities and stain absorbance values (brightfield) or stain emission
/// values (fluorescence)
///
/// This function does the opposite of what `dip::UnmixStains` does: it applies the per-pixel
/// matrix multiplication \f$A = \mathbf{S} d\f$ to obtain \f$A\f$ (`out`) from  \f$d\f$ (`in`) and
/// \f$\mathbf{S}\f$ (composed from the values in `stains`).
///
/// `stains` is a vector with these absorbance/emission values, and should have the same number of
/// elements as channels (tensor elements) in the image `in`. Each element of the vector should have the
/// same number of channels, and these dictate the number of channels in the output image `out`. If `out`
/// has three channels, it will be tagged as an RGB image. Call `dip::InverseBeerLambertMapping` with `out`
/// to create an image as seen through a brightfield microscope:
///
/// Example:
/// ```cpp
///     dip::Image img( { 1024, 1024 }, 2 );
///     dip::DrawBandlimitedBall( img, 300, { 400, 500 }, { 1.0, 0.0 } );
///     dip::DrawBandlimitedBall( img, 200, { 500, 600 }, { 0.0, 1.0 } );
///     img = dip::MixStains( img, {{ 0.644, 0.717, 0.267 }, { 0.268, 0.570, 0.776 }} );
///     img = dip::InverseBeerLambertMapping( img, { 255 } );
/// ```
///
/// If there are more stains than channels, this process is irreversible (that is, it will not be possible
/// to unmix the stains again).
DIP_EXPORT void MixStains(
      Image const& in,
      Image& out,
      std::vector< Image::Pixel > const& stains
);
inline Image MixStains(
      Image const& in,
      std::vector< Image::Pixel > const& stains
) {
   Image out;
   MixStains( in, out, stains );
   return out;
}


/// \brief Generates an incoherent OTF (optical transfer function)
///
/// This function implements the formulae for a (defocused) incoherent OTF as described by Castleman.
///
/// The `defocus` is defined as the maximum defocus path length error divided by the wave length
/// (See Castleman for details).
/// When `defocus` is unequal to zero, either the Stokseth approximation or the Hopkins approximation
/// is used, depending on the value of `method` (which can be either `"Stokseth"` or `"Hopkins"`).
/// The summation over the Bessel functions in the Hopkins formluation is stopped when the change is
/// smaller than 0.0001 (this is a compile-time constant).
///
/// `oversampling` is the oversampling rate. If set to 1, the OTF is sampled at the Nyquist rate. Increase
/// the value to sample more densely.
///
/// `amplitude` is the value of the OTF at the origin, and thus equivalent to the integral over the PSF.
///
/// `out` will be scalar and of type `DT_SFLOAT`. It should have 1 or 2 dimensions, its sizes will be preserved.
/// If `out` has no sizes, a 256x256 image will be generated.
///
/// **Literature**
/// - K.R. Castleman, "Digital image processing", Second Edition, Prentice Hall, Englewood Cliffs, 1996.
DIP_EXPORT void IncoherentOTF(
      Image& out,
      dfloat defocus = 0,
      dfloat oversampling = 1,
      dfloat amplitude = 1,
      String const& method = "Stokseth"
);
inline Image IncoherentOTF(
      dfloat defocus = 0,
      dfloat oversampling = 1,
      dfloat amplitude = 1,
      String const& method = "Stokseth"
) {
   Image out;
   IncoherentOTF( out, defocus, oversampling, amplitude, method );
   return out;
}

/// \brief Generates an incoherent PSF (point spread function)
///
/// This function generates an incoherent in-focus point spread function of a diffraction limited objective.
///
/// `oversampling` is the oversampling rate. If set to 1, the OTF is sampled at the Nyquist rate. Increase
/// the value to sample more densely.
///
/// `amplitude` is the integral over the PSF.
///
/// `out` will be scalar and of type `DT_SFLOAT`. It should have 1 or 2 dimensions, its sizes will be preserved.
/// For 1D images, the PSF returned is a single line through the middle of a 2D PSF.
/// If `out` has no sizes, a square image of size `ceil(19*oversampling)` will be generated.
///
/// **Literature**
/// - K.R. Castleman, "Digital image processing", Second Edition, Prentice Hall, Englewood Cliffs, 1996.
DIP_EXPORT void IncoherentPSF(
      Image& out,
      dfloat oversampling = 1,
      dfloat amplitude = 1
);
inline Image IncoherentPSF(
      dfloat oversampling = 1,
      dfloat amplitude = 1
) {
   Image out;
   IncoherentPSF( out, oversampling, amplitude );
   return out;
}


/// \brief Wiener Deconvolution using estimates of signal and noise power
///
/// If \f$G\f$ is the Fourier transform of `in`, \f$H\f$ is the Fourier transform of `psf`,
/// and \f$F\f$ is the Fourier transform of `out`, then this function estimates the \f$F\f$ that optimally
/// (in the least squares sense) satisfies \f$G = FH\f$ (that is, `in` is the result of the convolution of
/// `out` with `psf`).
///
/// Finding `out` requires knowledge of the power spectrum of the signal and the noise. The Wiener deconvolution
/// filter is defined in the frequency domain as
/// \f[ H_\text{inv} = \frac{H^* S}{ H^* H S + N } \;, \f]
/// where \f$S\f$ is `signalPower`, and \f$N\f$ is `noisePower`. These functions are typically not known, but:
///
///  - `signalPower` can be estimated as the Fourier transform of the autocorrelation of `in`. If a raw image
///    is passed for this argument (`%dip::Image{}`), then it will be computed as such.
///
///  - `noisePower` can be estimated as a flat function. A 0D image can be given here, it will be expanded to
///    the size of the other images. `noisePower` should not be zero anywhere, as that might lead to division
///    by zero and consequently meaningless results.
///
/// The other syntax for `%dip::WienerDeconvolution` takes an estimate of the noise-to-signal
/// ratio instead of the signal and noise power spectra. Note that \f$H_\text{inv}\f$ can be rewritten as
/// \f[ H_\text{inv} = \frac{H^*}{ H^* H  + \frac{N}{S} } = \frac{H^*}{ H^* H  + K } \;, \f]
/// where \f$K\f$ is the noise-to-signal ratio.
///
/// `psf` is given in the spatial domain, and will be zero-padded to the size of `in` and Fourier transformed.
/// The PSF (point spread function) should sum to one in order to preserve the mean image intensity.
/// If the OTF (optical transfer function, the Fourier transform of the PSF) is known, it is possible to pass
/// that as `psf`; add the string `"OTF"` to `options`.
///
/// All input images must be real-valued and scalar, except if the OFT is given instead of the PSF, in which
/// case `psf` could be complex-valued.
DIP_EXPORT void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image const& signalPower,
      Image const& noisePower,
      Image& out,
      StringSet const& options = {}
);
inline Image WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image const& signalPower,
      Image const& noisePower,
      StringSet const& options = {}
) {
   Image out;
   WienerDeconvolution( in, psf, signalPower, noisePower, out, options );
   return out;
}

/// \brief Wiener Deconvolution using an estimate of noise-to-signal ratio
///
/// See the description of the function with the same name above. The difference here is that a single number,
/// `regularization`, is given instead of the signal and noise power spectra. We then set \f$K\f$ (the
/// noise-to-signal ratio) to `regularization * dip::Maximum(P)`, with `P` equal to \f$H^* H\f$.
DIP_EXPORT void WienerDeconvolution(
      Image const& in,
      Image const& psf,
      Image& out,
      dfloat regularization = 1e-4,
      StringSet const& options = {}
);
inline Image WienerDeconvolution(
      Image const& in,
      Image const& psf,
      dfloat regularization = 1e-4,
      StringSet const& options = {}
) {
   Image out;
   WienerDeconvolution( in, psf, out, regularization, options );
   return out;
}


/// \brief 3D fluorescence attenuation correction using an exponential fit
///
/// This routine implements a simple correction of absorption, reflection and bleaching in 3D fluorescence
/// imaging based upon the assumption that the sum of these effects result in a exponential extinction of
/// the signal as a function of depth. Only pixels within `mask`, if given are taken into account to determine
/// this attenuation, but the whole image is corrected.
///
/// The attenuation is estimated based on the mean of the non-masked pixels as a function of depth. If
/// `percentile` is in the valid range [0, 100], the corresponding percentile is used instead of the mean.
/// An exponential function is fitted to these values. The starting point of the fit is determined by `fromWhere`,
/// which can be `"first plane"`, `"first max"`, or `"global max"`. In the case of `"first max"`, the first maximum
/// is found with `point[z+1] > hysteresis * point[z]`.
///
/// If the mean variant is chosen, one can choose to apply a variance weighting to the fit by setting `weighting`
/// to `"variance"`.
///
/// `in` must be a 3D, scalar and real-valued image. For images with fewer than 3 dimensions, the input is returned
/// unchanged.
///
/// **Literature**
/// - K.C. Strasters, H.T.M. van der Voort, J.M. Geusebroek, and A.W.M. Smeulders,
/// "Fast attenuation correction in fluorescence confocal imaging: a recursive approach", BioImaging 2(2):78-92, 1994.
DIP_EXPORT void ExponentialFitCorrection(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat percentile = -1.0,
      String const& fromWhere = "first plane",
      dfloat hysteresis = 0.0,
      String const& weighting = "none"
);
inline Image ExponentialFitCorrection(
      Image const& in,
      Image const& mask,
      dfloat percentile = -1.0,
      String const& fromWhere = "first plane",
      dfloat hysteresis = 0.0,
      String const& weighting = "none"
) {
   Image out;
   ExponentialFitCorrection( in, mask, out, percentile, fromWhere, hysteresis, weighting );
   return out;
}

/// \brief 3D fluorescence attenuation correction using one of three iterative algorithms
///
/// This function implements an attenuation correction using three different recursive attenuation correction
/// algorithms. The method is selected with the `method` parameter, which must be one of `"DET"`, `"LT2"` or `"LT1"`.
/// DET stands for Directional Extinction Tracking. LT2 is the Two Light Cone convolutions method, and LT1 is the
/// One Light Cone convolution.
///
/// The DET algorithm is the most accurate one, since it takes both forward and backward attenuation
/// into account (specified through the `fAttenuation` and `bAttenuation` parameters). It is however considerably
/// slower that the LT2 and LT1 algorithms, which take only forward attenuation into account (`fAttenuation`).
/// These last two algorithms assume a constant attenuation (`background`) for pixels with an intensity lower
/// than the `threshold`.
///
/// The system is characterized by parameters `NA` (numerical aperture) and `refIndex` (refractive index of the
/// medium), as well as the pixel size information in `in` (the x and y pixel size must be the same, the z size
/// must be have the same units, but can be different).
///
/// `in` must be a 3D, scalar and real-valued image. For images with fewer than 3 dimensions, the input is returned
/// unchanged.
///
/// **Literature**
/// - K.C. Strasters, H.T.M. van der Voort, J.M. Geusebroek, and A.W.M. Smeulders,
/// "Fast attenuation correction in fluorescence confocal imaging: a recursive approach", BioImaging 2(2):78-92, 1994.
DIP_EXPORT void AttenuationCorrection(
      Image const& in,
      Image& out,
      dfloat fAttenuation = 0.01,
      dfloat bAttenuation = 0.01,
      dfloat background = 0.0,
      dfloat threshold = 0.0,
      dfloat NA = 1.4,           // typical 63x oil immersion lens
      dfloat refIndex = 1.518,   // ideal immersion oil
      String const& method = "DET"
);
inline Image AttenuationCorrection(
      Image const& in,
      dfloat fAttenuation = 0.01,
      dfloat bAttenuation = 0.01,
      dfloat background = 0.0,
      dfloat threshold = 0.0,
      dfloat NA = 1.4,           // typical 63x oil immersion lens
      dfloat refIndex = 1.518,   // ideal immersion oil
      String const& method = "DET"
) {
   Image out;
   AttenuationCorrection( in, out, fAttenuation, bAttenuation, background, threshold, NA, refIndex, method );
   return out;
}

/// \brief 3D fluorescence attenuation simulation
///
/// Simulates an attenuation based on the model of a CSLM, using a ray tracing method. **NOTE:** this function is
/// extremely slow, and its running time grows exponentially with the number of slices.
///
/// The system is characterized by parameters `NA` (numerical aperture) and `refIndex` (refractive index of the
/// medium), as well as the pixel size information in `in` (the x and y pixel size must be the same, the z size
/// must be have the same units, but can be different).
///
/// `fAttenuation` and `bAttenuation` are the forward and backward attentuation factors, respectively.
///
/// The ray tracing method uses a step size of `rayStep`, and a ray casting oversampling of `oversample`.
///
/// `in` must be a 3D, scalar and real-valued image. For images with fewer than 3 dimensions, the input is returned
/// unchanged.
///
/// **Literature**
/// - K.C. Strasters, H.T.M. van der Voort, J.M. Geusebroek, and A.W.M. Smeulders,
/// "Fast attenuation correction in fluorescence confocal imaging: a recursive approach", BioImaging 2(2):78-92, 1994.
DIP_EXPORT void SimulatedAttenuation(
      Image const& in,
      Image& out,
      dfloat fAttenuation = 0.01,
      dfloat bAttenuation = 0.01,
      dfloat NA = 1.4,           // typical 63x oil immersion lens
      dfloat refIndex = 1.518,   // ideal immersion oil
      dip::uint oversample = 1,
      dfloat rayStep = 1
);
inline Image SimulatedAttenuation(
      Image const& in,
      dfloat fAttenuation = 0.01,
      dfloat bAttenuation = 0.01,
      dfloat NA = 1.4,           // typical 63x oil immersion lens
      dfloat refIndex = 1.518,   // ideal immersion oil
      dip::uint oversample = 1,
      dfloat rayStep = 1
) {
   Image out;
   SimulatedAttenuation( in, out, fAttenuation, bAttenuation, NA, refIndex, oversample, rayStep );
   return out;
}

/// \}

} // namespace dip

#endif // DIP_MICROSCOPY_H
