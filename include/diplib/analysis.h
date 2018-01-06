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


/// \brief Calculates the cross-correlation between two images of equal size.
///
/// The returned image is the cross-correlation normalized in such a way that only
/// the phase information is of importance. The computation performed is
/// `out = (%Conjugate(in1)*in2)/%SquareModulus(in1)` in the Fourier Domain
/// (see `dip::Conjugate`, `dip::SquareModulus`).
/// This results as a very sharp peak in the spatial domain. If `normalize` is set
/// to `"don't normalize"`, the regular cross-correlation (not normalized by the square
/// modulus) is returned.
///
/// Note that this normalization is not related to what is commonly referred to as
/// "normalized cross-correlation", where the input images are whitened before the
/// cross-correlations is computed. The method is instead related to the "phase correlation"
/// as proposed by Kuglin and Hines (1975), except that they divide by the modulus of each
/// of the images in the Fourier Domain, instead of the square modulus of the first image
/// as we do here. The difference is not important if the two images are obtained under
/// identical circumstances.
///
/// As elsewhere, the origin is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image. Thus, for `in1==in2`, only this pixel will be set.
///
/// If `in1` or `in2` is already Fourier transformed, set `in1Representation` or `in2Representation`
/// to `"frequency"`. Similarly, if `outRepresentation` is `"frequency"`, the output will not be
/// inverse-transformed, so will be in the frequency domain.
///
/// `in1` and `in2` must be scalar images with the same dimensionality and sizes.
///
/// `out` will be real-valued if `outRepresentation` is `"spatial"`, under the assumption that
/// `in1` and `in2` are similar except for a shift.
///
/// **Literature**:
///  - C.D. Kuglin and D.C. Hines, "The phase correlation image alignment method", International
///    Conference on Cybernetics and Society (IEEE), pp 163-165, 1975.
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
///    See the notes in `dip::CrossCorrelationFT` regarding the normalization of the cross-correlation, which
///    is not as what is commonly referred to as NCC.
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
      UnsignedArray maxShift = {}
);


/// \brief Computes the structure tensor.
///
/// The structure tensor is a tensor image that contains, at each pixel, information about the local image structure.
/// The eigenvalues of the structure tensor are larger when there are stronger gradients locally. That is, if all
/// eigenvalues are small, the image is locally uniform. If one one eigenvalue is large, then there is a unique line
/// or edge orienatation (in 2D), or a plane-like edge or structure (in 3D). In 3D, if two eigenvalues are large
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
/// should have values between 0 and 1 (or be binary). Normalized convolution is used to compute the derivatives and
/// local averaging, and thereby fill in the missing values of the input image. [This is not yet implemented.]
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
/// --------------|-------------------
/// `l1`          | The largest eigenvalue.
/// `l2`          | The smallest eigenvalue.
/// `orientation` | Orientation. Lies in the interval (-pi/2, pi/2).
/// `energy`      | Sum of the two eigenvalues `l1` and `l2`.
/// `anisotropy1` | Measure for local anisotropy: `( l1 - l2 ) / ( l1 + l2 )`.
/// `anisotropy2` | Measure for local anisotropy: `1 - l2 / l1`, where l1 > 0.
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
      Image* anisotropy2 = nullptr
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
/// --------------|-------------------
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
///  - For 2D inputs: `"l1"`, `"l2"`, `"orientation"`, `"energy"`, `"anisotropy1"`, `"anisotropy2"`.
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
   ImageRefArray refOut;
   for( auto& o : out ) {
      refOut.emplace_back( o );
   }
   DIP_STACK_TRACE_THIS( StructureTensorAnalysis( in, refOut, outputs ));
   return out;
}

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

// TODO: functions to port:
/*
   dip_PairCorrelation (dip_analysis.h)
   dip_ProbabilisticPairCorrelation (dip_analysis.h)
   dip_ChordLength (dip_analysis.h)
   dip_RadialDistribution (dip_analysis.h)

   dip_StructureAnalysis (dip_analysis.h)

   dip_OrientationSpace (dip_structure.h)
   dip_ExtendedOrientationSpace (dip_structure.h)

   dip_CurvatureFromTilt (dip_structure.h)

   dip_OSEmphasizeLinearStructures (dip_structure.h)
   dip_DanielsonLineDetector (dip_structure.h)
*/

/// \}

} // namespace dip

#endif // DIP_ANALYSIS_H
