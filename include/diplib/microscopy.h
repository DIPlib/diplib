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
/// \f[ A_R = S_{1,R} d_1 + S_{2,R} d_2 + S_{3,R} d_3 + ... \f]
///
/// with \f$S_{n,R}\f$ the absorbance of dye \f$n\f$ in the red channel, \f$d_n\f$ the density (concentration) of
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
/// `stains` is a vector with these absorbance values, one for each dye. `stains` contains all the values
/// for matrix \f$\mathbf{S}\f$. It cannot have more elements than channels (tensor elements) are in `in`.
/// Each element of the vector is a `dip::Image::Pixel` with the same number of tensor elements as `in`.
/// `out` will contain one channel for each stain.
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
/// `stains` is `stains` is a vector with these absorbance/emission values, and should have the same number of
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



// TODO: functions to port:
/*
   dip_IncoherentPSF (dip_microscopy.h)
   dip_IncoherentOTF (dip_microscopy.h)
   dip_ExponentialFitCorrection (dip_microscopy.h)
   dip_AttenuationCorrection (dip_microscopy.h)
   dip_SimulatedAttenuation (dip_microscopy.h)
   dip_RestorationTransform (dip_restoration.h)
   dip_TikhonovRegularizationParameter (dip_restoration.h)
   dip_TikhonovMiller (dip_restoration.h)
   dip_Wiener (dip_restoration.h)
   dip_PseudoInverse (dip_restoration.h)
*/

// TODO: add brightfield stain unmixing

/// \}

} // namespace dip

#endif // DIP_MICROSCOPY_H
