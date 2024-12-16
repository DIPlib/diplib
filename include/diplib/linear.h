/*
 * (c)2017-2024, Cris Luengo.
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

#ifndef DIP_LINEAR_H
#define DIP_LINEAR_H

#include <cmath>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/kernel.h"


/// \file
/// \brief Functions that implement linear filters.
/// See \ref linear.


namespace dip {


/// \group linear Linear filters
/// \ingroup filtering
/// \brief Linear smoothing, sharpening and derivative filters.
/// \addtogroup

/// \brief Describes a 1D filter
///
/// The weights are in `filter`. If `isComplex`, then the values in `filter` are interpreted as real/imaginary
/// pairs. In this case, `filter` must have an even length, with each two consecutive elements representing a
/// single filter weight. The `filter.data()` pointer can thus be cast to \ref dip::dcomplex.
///
/// The origin is placed either at the index given by `origin` if it's non-negative, or at index
/// `filter.size() / 2` if `origin` is negative. Note that `filter.size() / 2` is either the middle pixel
/// if the filter is odd in length, or the pixel to the right of the center if it is even in length:
///
/// size of `filter` | `origin` | origin location
/// ---------------- |:--------:|:---------------:
/// any              |    `1`   | `x 0 x x x x`
/// any              |    `5`   | `x x x x x 0`
/// any odd value    |   `-1`   | `x x 0 x x  `
/// any even value   |   `-1`   | `x x x 0 x x`
///
/// Note that, if positive, `origin` must be an index to one of the samples in the `filter` array:
/// `origin < filter.size()`.
///
/// `symmetry` indicates the filter shape: `"general"` (or an empty string) indicates no symmetry.
/// `"even"` indicates even symmetry, `"odd"` indicates odd symmetry, and `"conj"` indicates complex conjugate
/// symmetry. In these three cases, the filter represents the left half of the full filter,
/// with the rightmost element at the origin (and not repeated). The full filter is thus always odd in size.
/// `"d-even"`, `"d-odd"` and `"d-conj"` are similar, but duplicate the rightmost element, yielding
/// an even-sized filter. The origin for the symmetric filters is handled identically to the general filter case.
///
/// The following table summarizes the result of using various `symmetry` values. The `filter` array in all
/// cases has *n* elements represented in this example as [*a*,*b*,*c*].
///
/// `symmetry`  | resulting array                    | resulting array length
/// ----------- | ---------------------------------- | -----------------------
/// `"general"` | [*a*,*b*,*c*]                      | *n*
/// `"even"`    | [*a*,*b*,*c*,*b*,*a*]              | 2*n* - 1
/// `"odd"`     | [*a*,*b*,*c*,-*b*,-*a*]            | 2*n* - 1
/// `"conj"`    | [*a*,*b*,*c*,*b*^\*^,*a*^\*^]        | 2*n* - 1
/// `"d-even"`  | [*a*,*b*,*c*,*c*,*b*,*a*]          | 2*n*
/// `"d-odd"`   | [*a*,*b*,*c*,-*c*,-*b*,-*a*]       | 2*n*
/// `"d-conj"`  | [*a*,*b*,*c*,*c*^\*^,*b*^\*^,*a*^\*^] | 2*n*
///
/// The convolution is applied to each tensor component separately, which is always the correct behavior for linear
/// filters.
struct DIP_NO_EXPORT OneDimensionalFilter {
   std::vector< dfloat > filter; ///< Filter weights.
   dip::sint origin = -1;        ///< Origin of the filter if non-negative.
   String symmetry;              ///< Filter shape: `""` == `"general"`, `"even"`, `"odd"`, `"conj"`, `"d-even"`, `"d-odd"` or `"d-conj"`.
   bool isComplex = false;       ///< If true, `filter` contains complex data.
};

/// \brief An array of 1D filters
using OneDimensionalFilterArray = std::vector< OneDimensionalFilter >;

/// \brief Separates a linear filter (convolution kernel) into a set of 1D filters that can be applied using
/// \ref dip::SeparableConvolution.
///
/// If `filter` does not represent a separable kernel, the output \ref dip::OneDimensionalFilterArray object is
/// empty (it's `empty` method returns true, and it's `size` method return 0).
DIP_EXPORT OneDimensionalFilterArray SeparateFilter( Image const& filter );

/// \brief Applies a convolution with a filter kernel (PSF) that is separable.
///
/// `filter` is an array with exactly one \ref dip::OneDimensionalFilter element for each dimension of `in`.
/// Alternatively, it can have a single element, which will be used unchanged for each dimension.
/// For the dimensions that are not processed (`process` is `false` for those dimensions),
/// the `filter` array can have nonsensical data or a zero-length filter weights array.
/// Any `filter` array that is zero size or the equivalent of `{1}` will not be applied either.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// `process` indices which dimensions to process, and can be `{}` to indicate all dimensions are to be processed.
///
/// \see dip::SeparateFilter, dip::Convolution, dip::GeneralConvolution, dip::ConvolveFT, dip::Framework::Separable
DIP_EXPORT void SeparableConvolution(
      Image const& in,
      Image& out,
      OneDimensionalFilterArray const& filterArray,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {}
);
DIP_NODISCARD inline Image SeparableConvolution(
      Image const& in,
      OneDimensionalFilterArray const& filter,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {}
) {
   Image out;
   SeparableConvolution( in, out, filter, boundaryCondition, std::move( process ));
   return out;
}

/// \brief Applies a convolution with a filter kernel (PSF) by multiplication in the Fourier domain.
///
/// `filter` is an image, and must be equal in size or smaller than `in`. If both `in` and `filter`
/// are real, `out` will be real too, otherwise it will have a complex type.
///
/// As elsewhere, the origin of `filter` is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image.
///
/// If `in` or `filter` is already Fourier transformed, set `inRepresentation` or `filterRepresentation`
/// to `"frequency"`. Similarly, if `outRepresentation` is `"frequency"`, the output will not be
/// inverse-transformed, so will be in the frequency domain. These three values are `"spatial"` by default.
/// If any of these three values is `"frequency"`, then `out` will be complex, no checks are made to
/// see if the inputs in frequency domain have the complex conjugate symmetry required for the result
/// to be real-valued. Use \ref dip::Image::Real if you expect the output to be real-valued in this case.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension.
/// See \ref dip::BoundaryCondition for a description of each option. It is ignored unless `inRepresentation`,
/// `filterRepresentation` and `outRepresentation` are all `"spatial"`. If the array is empty (the default),
/// then a periodic boundary condition is imposed. This is the natural boundary condition for the method,
/// the image will be Fourier transformed as-is. For other boundary conditions, the image will be padded before
/// the transform is applied. The padding will extend the image by at least half the size of `filter` in all
/// dimensions, and the padding will make the image size a multiple of small integers so that it is cheaper
/// to compute the Fourier transform (see \ref dip::OptimalFourierTransformSize).
/// The output image will be cropped to the size of the input.
///
/// \see dip::Convolution, dip::GeneralConvolution, dip::SeparableConvolution, dip::FourierTransform
DIP_EXPORT void ConvolveFT(
      Image const& in,
      Image const& filter,
      Image& out,
      String const& inRepresentation = S::SPATIAL,
      String const& filterRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image ConvolveFT(
      Image const& in,
      Image const& filter,
      String const& inRepresentation = S::SPATIAL,
      String const& filterRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   ConvolveFT( in, filter, out, inRepresentation, filterRepresentation, outRepresentation, boundaryCondition );
   return out;
}

/// \brief Applies a convolution with a filter kernel (PSF) by direct implementation of the convolution sum.
///
/// `filter` is an image, and must be equal in size or smaller than `in`.
///
/// As elsewhere, the origin of `filter` is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image.
///
/// Note that this is a really expensive way to compute the convolution for any `filter` that has more than a
/// small amount of non-zero values. It is always advantageous to try to separate your filter into a set of 1D
/// filters (see \ref dip::SeparateFilter and \ref dip::SeparableConvolution). If this is not possible, use
/// \ref dip::ConvolveFT with larger filters to compute the convolution in the Fourier domain.
///
/// Also, if all non-zero filter weights have the same value, \ref dip::Uniform implements a more efficient
/// algorithm. If `filter` is a binary image, \ref dip::Uniform is called.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// \see dip::Convolution, dip::ConvolveFT, dip::SeparableConvolution, dip::SeparateFilter, dip::Uniform
DIP_EXPORT void GeneralConvolution(
      Image const& in,
      Image const& filter,
      Image& out,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image GeneralConvolution(
      Image const& in,
      Image const& filter,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   GeneralConvolution( in, filter, out, boundaryCondition );
   return out;
}

/// \brief Applies a convolution with a filter kernel (PSF).
///
/// Calls either \ref SeparableConvolution, \ref ConvolveFT or \ref GeneralConvolution depending on `method` and
/// the properties of `filter`. `method` can be one of:
///
/// - `"separable"`: Attempts to separate `filter` into 1D kernels using \ref SeparateFilter, and applies
///   \ref SeparableConvolution if successful. Throws an exception if the filter is not separable.
/// - `"fourier"`: Calls \ref ConvolveFT.
/// - `"direct"`: Calls \ref GeneralConvolution.
/// - `"best"`: Uses the method that is most efficient given the sizes of `in` and `filter`, and whether `filter`
///   is separable or not. It estimates the cost of each of the methods using simple models that have been fitted
///   to timing data generated on one specific computer. These costs might not match actual costs on other machines,
///   but form a suitable default. For applications where performance is critical, it is recommended to time the
///   operations on the target machine, and explicitly select the best algorithm.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// When calling \ref ConvolveFT, it never leaves `boundaryCondition` empty, to force the function to pad the
/// image and use the same boundary condition that other methods would use. This ensures that the function doesn't
/// produce different results for a different choice of method. To prevent padding, call \ref ConvolveFT directly.
///
/// \see dip::GeneralConvolution, dip::ConvolveFT, dip::SeparableConvolution, dip::SeparateFilter, dip::Uniform
DIP_EXPORT void Convolution(
      Image const& in,
      Image const& filter,
      Image& out,
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image Convolution(
      Image const& in,
      Image const& filter,
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Convolution( in, filter, out, method, boundaryCondition );
   return out;
}

/// \brief Applies a convolution with a kernel with uniform weights, leading to an average (mean) filter.
///
/// The size and shape of the kernel is given by `kernel`, which you can define through a default
/// shape with corresponding sizes, or through a binary image. See \ref dip::Kernel.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// \see dip::ConvolveFT, dip::SeparableConvolution, dip::GeneralConvolution
DIP_EXPORT void Uniform(
      Image const& in,
      Image& out,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
);
DIP_NODISCARD inline Image Uniform(
      Image const& in,
      Kernel const& kernel = {},
      StringArray const& boundaryCondition = {}
) {
   Image out;
   Uniform( in, out, kernel, boundaryCondition );
   return out;
}

/// \brief Finite impulse response implementation of the Gaussian filter and its derivatives
///
/// Convolves the image with a 1D Gaussian kernel along each dimension. For each dimension,
/// provide a value in `sigmas` and `derivativeOrder`. The zeroth-order derivative is a plain
/// smoothing, no derivative is computed. Derivatives with order up to 3 can be computed with
/// this function. For higher-order derivatives, use \ref dip::GaussFT.
///
/// The value of sigma determines the smoothing effect. For values smaller than about 0.8, the
/// result is an increasingly poor approximation to the Gaussian filter. Use \ref dip::GaussFT for
/// very small sigmas. Conversely, for very large sigmas it is more efficient to use \ref dip::GaussIIR,
/// which runs in a constant time with respect to the sigma. Dimensions where sigma is 0 or
/// negative are not processed, even if the derivative order is non-zero.
///
/// For the smoothing filter (`derivativeOrder` is 0), the size of the kernel is given by
/// `2 * std::ceil( truncation * sigma ) + 1`. The default value for `truncation` is 3, which assures a good
/// approximation of the Gaussian kernel without unnecessary expense. It is possible to reduce
/// computation slightly by decreasing this parameter, but it is not recommended. For derivatives,
/// the value of `truncation` is increased by `0.5 * derivativeOrder`.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// \see dip::Gauss, dip::GaussIIR, dip::GaussFT, dip::Derivative, dip::FiniteDifference, dip::Uniform
DIP_EXPORT void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GaussFIR(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   GaussFIR( in, out, std::move( sigmas ), std::move( derivativeOrder ), boundaryCondition, truncation );
   return out;
}

/// \brief Fourier implementation of the Gaussian filter and its derivatives
///
/// Convolves the image with a Gaussian kernel by multiplication in the Fourier domain.
/// For each dimension, provide a value in `sigmas` and `derivativeOrder`. The value of sigma determines
/// the smoothing effect. The zeroth-order derivative is a plain smoothing, no derivative is computed.
///
/// The values of `sigmas` are translated to the Fourier domain, and a Fourier-domain Gaussian is computed.
/// Frequencies above `std::ceil(( truncation + 0.5 * derivativeOrder ) * FDsigma )` are set to 0. It is a relatively
/// minute computational difference if `truncation` were to be infinity, so it is not worth while to try to
/// speed up the operation by decreasing `truncation`.
///
/// Dimensions where sigma is 0 or negative are not smoothed. Note that it is possible to compute a derivative
/// without smoothing in the Fourier domain.
///
/// If `in` is already Fourier transformed, set `inRepresentation` to `"frequency"`.
/// Similarly, if `outRepresentation` is `"frequency"`, the output will not be inverse-transformed,
/// and so will be in the frequency domain. These two values are `"spatial"` by default.
/// If any of these values is `"frequency"`, then `out` will be complex, no checks are made to
/// see if the inputs in frequency domain have the complex conjugate symmetry required for the result
/// to be real-valued. Use \ref dip::Image::Real if you expect the output to be real-valued in this case.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
/// The default empty boundary condition indicates no boundary extension is to be applied, the convolution will be
/// circular (periodic boundary condition) as is natural with the DFT convolution. Specifying a boundary condition
/// will cause the input image to be padded to a good DFT size (a product of small integers,
/// see \ref dip::OptimalFourierTransformSize) that is large enough to prevent visible effects of the circular
/// convolution. Thus, specifying `"periodic"` as the boundary condition could, depending on the sizes of the image,
/// speed up the operation compared to leaving the boundary condition empty.
///
/// If `inRepresentation` is `"frequency"`, then `boundaryCondition` is ignored.
///
/// If `outRepresentation` is `"frequency"`, then the padding caused by `boundaryCondition` will affect the output size.
/// Leave `boundaryCondition` empty in this case if a predictable output size is needed.
///
/// \see dip::Gauss, dip::GaussFIR, dip::GaussIIR, dip::Derivative, dip::FiniteDifference, dip::Uniform
DIP_EXPORT void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      dfloat truncation = 3,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      StringArray const& boundaryCondition = {} // Added here later, this is why its location is not consistent with other filtering functions.
);
DIP_NODISCARD inline Image GaussFT(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      dfloat truncation = 3,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   GaussFT( in, out, std::move( sigmas ), std::move( derivativeOrder ), truncation, inRepresentation, outRepresentation, boundaryCondition );
   return out;
}

/// \brief Infinite impulse response implementation of the Gaussian filter and its derivatives
///
/// Convolves the image with an IIR 1D Gaussian kernel along each dimension. For each dimension,
/// provide a value in `sigmas` and `derivativeOrder`. The zeroth-order derivative is a plain
/// smoothing, no derivative is computed. Derivatives with order up to 4 can be computed with this
/// function. For higher-order derivatives, use \ref dip::GaussFT.
///
/// The value of sigma determines the smoothing effect. For smaller values, the result is an
/// increasingly poor approximation to the Gaussian filter. This function is efficient only for
/// very large sigmas. Dimensions where sigma is 0 or negative are not processed, even if the
/// derivative order is non-zero.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// The `filterOrder` and `designMethod` determine how the filter is implemented. By default,
/// `designMethod` is "discrete time fit". This is the method described in van Vliet et al. (1998).
/// `filterOrder` can be between 1 and 5, with 3 producing good results, and increasing order producing
/// better results. When computing derivatives, a higher `filterOrder` is necessary. By default,
/// `filterOrder` is `3 + derivativeOrder`, capped at 5. The alternative `designMethod` is "forward backward".
/// This is the method described in Young and van Vliet (1995). Here `filterOrder` can be between 3 and 5.
///
/// \see dip::Gauss, dip::GaussFIR, dip::GaussFT, dip::Derivative, dip::FiniteDifference, dip::Uniform
///
/// !!! literature
///     - I.T. Young and L.J. van Vliet, "Recursive implementation of the Gaussian filter", Signal Processing,
///       44(2):139-151, 1995.
///     - L.J. van Vliet, I.T. Young and P.W. Verbeek, "Recursive Gaussian Derivative Filters",
///       in: Proc. 14^th^ Int. Conference on Pattern Recognition, IEEE Computer Society Press, 1998, 509-514.
DIP_EXPORT void GaussIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      UnsignedArray filterOrder = {}, // means compute order depending on derivativeOrder.
      String const& designMethod = S::DISCRETE_TIME_FIT,
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GaussIIR(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      UnsignedArray filterOrder = {},
      String const& designMethod = S::DISCRETE_TIME_FIT,
      dfloat truncation = 3
) {
   Image out;
   GaussIIR( in, out, std::move( sigmas ), std::move( derivativeOrder ), boundaryCondition, std::move( filterOrder ), designMethod, truncation );
   return out;
}

/// \brief Convolution with a Gaussian kernel and its derivatives
///
/// Convolves the image with a Gaussian kernel. For each dimension, provide a value in `sigmas` and
/// `derivativeOrder`. The value of sigma determines the smoothing effect. The zeroth-order derivative
/// is a plain smoothing, no derivative is computed. Dimensions where sigma is 0 or negative are not
/// smoothed. Only the "FT" method can compute the derivative along a dimension where sigma is zero or negative.
///
/// How the convolution is computed depends on the value of `method`:
///
/// - `"FIR"`: Finite impulse response implementation, see \ref dip::GaussFIR.
/// - `"IIR"`: Infinite impulse response implementation, see \ref dip::GaussIIR.
/// - `"FT"`: Fourier domain implementation, see \ref dip::GaussFT.
/// - `"best"`: Picks the best method, according to the values of `sigmas` and `derivativeOrder`:
///     - if any `derivativeOrder` is larger than 3, use the FT method,
///     - else if any `sigmas` is smaller than 0.8, use the FT method,
///     - else if any `sigmas` is larger than 10, use the IIR method,
///     - else use the FIR method.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// \see dip::GaussFIR, dip::GaussFT, dip::GaussIIR, dip::Derivative, dip::FiniteDifference, dip::Uniform
DIP_EXPORT void Gauss(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Gauss(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      UnsignedArray derivativeOrder = { 0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   Gauss( in, out, std::move( sigmas ), std::move( derivativeOrder ), method, boundaryCondition, truncation );
   return out;
}

/// \brief Finite difference derivatives
///
/// Computes derivatives using the finite difference method. Set a `derivativeOrder` for each dimension.
/// Derivatives of order up to 2 can be computed with this function. The zeroth-order derivative implies either
/// a smoothing is applied (`smoothFlag == "smooth"`) or the dimension is not processed at all.
///
/// The smoothing filter is `[1,2,1]/4` (as in the Sobel filter), the first order derivative is `[1,0,-1]/2`
/// (central difference), and the second order derivative is `[1,-2,1]` (which is the composition of twice the
/// non-central difference `[1,-1]`). Thus, computing the first derivative twice does not yield the same result
/// as computing the second derivative directly.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// Set `process` to false for those dimensions that should not be filtered.
///
/// \see dip::Derivative, dip::SobelGradient
DIP_EXPORT void FiniteDifference(
      Image const& in,
      Image& out,
      UnsignedArray derivativeOrder = { 0 },
      String const& smoothFlag = S::SMOOTH,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {}
);
DIP_NODISCARD inline Image FiniteDifference(
      Image const& in,
      UnsignedArray derivativeOrder = { 0 },
      String const& smoothFlag = S::SMOOTH,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {}
) {
   Image out;
   FiniteDifference( in, out, std::move( derivativeOrder ), smoothFlag, boundaryCondition, std::move( process ));
   return out;
}

/// \brief The Sobel derivative filter
///
/// This function applies the generalization of the Sobel derivative filter to arbitrary dimensions. Along the
/// dimension `dimension`, the central difference is computed, and along all other dimensions, the triangular
/// smoothing filter `[1,2,1]/4` is applied.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// This function calls \ref dip::FiniteDifference.
inline void SobelGradient(
      Image const& in,
      Image& out,
      dip::uint dimension = 0,
      StringArray const& boundaryCondition = {}
) {
   DIP_THROW_IF( dimension >= in.Dimensionality(), E::INVALID_PARAMETER );
   UnsignedArray derivativeOrder( in.Dimensionality(), 0 );
   derivativeOrder[ dimension ] = 1;
   FiniteDifference( in, out, derivativeOrder, S::SMOOTH, boundaryCondition );
}
DIP_NODISCARD inline Image SobelGradient(
      Image const& in,
      dip::uint dimension = 0,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   SobelGradient( in, out, dimension, boundaryCondition );
   return out;
}

/// \brief Computes derivatives
///
/// This function provides an interface to the various derivative filters in *DIPlib*.
///
/// For each dimension, provide a value in `sigmas` and `derivativeOrder`. The value of sigma determines
/// the smoothing effect. The zeroth-order derivative is a plain smoothing, no derivative is computed.
/// If `method` is `"best"`, `"gaussfir"` or `"gaussiir"`, dimensions where sigma is 0 or negative are not processed,
/// even if the derivative order is non-zero. That is, sigma must be positive for the dimension(s) where
/// the derivative is to be computed.
///
/// `method` indicates which derivative filter is used:
///
/// - `"best"`: A Gaussian derivative, see \ref dip::Gauss.
/// - `"gaussfir"`: The FIR implementation of the Gaussian derivative, see \ref dip::GaussFIR.
/// - `"gaussiir"`: The IIR implementation of the Gaussian derivative, see \ref dip::GaussIIR.
/// - `"gaussft"`: The FT implementation of the Gaussian derivative, see \ref dip::GaussFT.
/// - `"finitediff"`: A finite difference derivative, see \ref dip::FiniteDifference.
///
/// A finite difference derivative is an approximation to the derivative operator on the discrete grid.
/// In contrast, convolving an image with the derivative of a Gaussian provides the exact derivative of
/// the image convolved with a Gaussian:
///
/// $$ \frac{\partial G}{\partial x} \ast f = \frac{\partial}{\partial x}(G \ast f) $$
///
/// Thus (considering the regularization provided by the Gaussian smoothing is beneficial) it is always
/// better to use Gaussian derivatives than finite difference derivatives.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
DIP_EXPORT void Derivative(
      Image const& in,
      Image& out,
      UnsignedArray derivativeOrder,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Derivative(
      Image const& in,
      UnsignedArray derivativeOrder,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   Derivative( in, out, std::move( derivativeOrder ), std::move( sigmas ), method, boundaryCondition, truncation );
   return out;
}

#define DIP_DERIVATIVE_OPERATOR( functionName_, dimension_, order_ ) { \
      DIP_THROW_IF( in.Dimensionality() <= ( dimension_ ), E::DIMENSIONALITY_NOT_SUPPORTED ); \
      UnsignedArray derivativeOrder( in.Dimensionality(), 0 ); \
      derivativeOrder[ dimension_ ] = order_; \
      DIP_STACK_TRACE_THIS( Derivative( in, out, std::move( derivativeOrder ), std::move( sigmas ))); \
   } \
   DIP_NODISCARD inline Image functionName_( Image const& in, FloatArray sigmas = { 1.0 } ) { \
      Image out; functionName_( in, out, std::move( sigmas )); return out; }

/// \brief Computes the first derivative along x, see \ref dip::Derivative.
inline void Dx( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dx, 0, 1 )

/// \brief Computes the first derivative along y, see \ref dip::Derivative.
inline void Dy( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dy, 1, 1 )

/// \brief Computes the first derivative along z, see \ref dip::Derivative.
inline void Dz( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dz, 2, 1 )

/// \brief Computes the second derivative along x, see \ref dip::Derivative.
inline void Dxx( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dxx, 0, 2 )

/// \brief Computes the second derivative along y, see \ref dip::Derivative.
inline void Dyy( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dyy, 1, 2 )

/// \brief Computes the second derivative along z, see \ref dip::Derivative.
inline void Dzz( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dzz, 2, 2 )

#undef DIP_DERIVATIVE_OPERATOR

// Same, but for 2nd order cross derivatives. Dimension 2 is always largest of the two!
#define DIP_DERIVATIVE_OPERATOR( functionName_, dimension1_, dimension2_ ) { \
      DIP_THROW_IF( in.Dimensionality() <= ( dimension2_ ), E::DIMENSIONALITY_NOT_SUPPORTED ); \
      UnsignedArray derivativeOrder( in.Dimensionality(), 0 ); \
      derivativeOrder[ dimension1_ ] = 1; derivativeOrder[ dimension2_ ] = 1; \
      DIP_STACK_TRACE_THIS( Derivative( in, out, std::move( derivativeOrder ), std::move( sigmas ))); \
   } \
   DIP_NODISCARD inline Image functionName_( Image const& in, FloatArray sigmas = { 1.0 } ) { \
      Image out; functionName_( in, out, std::move( sigmas )); return out; }

/// \brief Computes the first derivative along x and y, see \ref dip::Derivative.
inline void Dxy( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dxy, 0, 1 )

/// \brief Computes the first derivative along x and z, see \ref dip::Derivative.
inline void Dxz( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dxz, 0, 2 )

/// \brief Computes the first derivative along y and z, see \ref dip::Derivative.
inline void Dyz( Image const& in, Image& out, FloatArray sigmas = { 1.0 } )
DIP_DERIVATIVE_OPERATOR( Dyz, 1, 2 )

#undef DIP_DERIVATIVE_OPERATOR

/// \brief Computes the gradient of the image, resulting in an *N*-vector image, if the input was *N*-dimensional.
///
/// Each tensor component corresponds to the first derivative along the given dimension: `out[ 0 ]` is the
/// derivative along *x* (dimension with index 0), `out[ 1 ]` is the derivative along *y* (dimension with index 1),
/// etc.
///
/// The input image must be scalar.
///
/// Set `process` to false for those dimensions along which no derivative should be taken. For example, if `in` is
/// a 3D image, and `process` is `{true,false,false}`, then `out` will be a scalar image, containing only the
/// derivative along the *x* axis.
///
/// By default uses Gaussian derivatives in the computation. Set `method = "finitediff"` for finite difference
/// approximations to the gradient. See \ref dip::Derivative for more information on the other parameters.
///
/// \see dip::Derivative, dip::Hessian, dip::GradientMagnitude, dip::GradientDirection
DIP_EXPORT void Gradient(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Gradient(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Gradient( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the gradient magnitude of the image, equivalent to `dip::Norm( dip::Gradient( in ))`.
///
/// For non-scalar images, applies the operation to each image channel. See \ref dip::Gradient for information on the parameters.
///
/// By default this function uses Gaussian derivatives in the computation. Set `method = "finitediff"` for
/// finite difference approximations to the gradient. See \ref dip::Derivative for more information on the other
/// parameters.
///
/// \see dip::Gradient, dip::Norm, dip::Derivative, dip::GradientDirection
DIP_EXPORT void GradientMagnitude(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GradientMagnitude(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   GradientMagnitude( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the direction of the gradient of the image, equivalent to `dip::Angle( dip::Gradient( in ))`.
///
/// The input image must be scalar. For a 2D gradient, the output is scalar also, containing the angle of the
/// gradient to the x-axis. For a 3D gradient, the output has two tensor components, containing the azimuth and
/// inclination. See \ref dip::Angle for an explanation.
///
/// See \ref dip::Gradient for information on the parameters.
///
/// By default this function uses Gaussian derivatives in the computation. Set `method = "finitediff"` for
/// finite difference approximations to the gradient. See \ref dip::Derivative for more information on the other
/// parameters.
///
/// \see dip::Gradient, dip::Angle, dip::Derivative, dip::GradientMagnitude
DIP_EXPORT void GradientDirection(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GradientDirection(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   GradientDirection( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the curl (rotation) of the 2D or 3D vector field `in`.
///
/// Curl is defined as by $\mathrm{curl}\,\mathbf{f} = \nabla \times \mathbf{f}$, for a 3-vector
/// $\mathbf{f}=(f_x, f_y, f_z)$ (the vector image `in`), resulting in a 3-vector with components:
///
/// \begin{eqnarray*}
///      (\mathrm{curl}\,\mathbf{f})_x &=& \frac{\partial}{\partial y} f_z - \frac{\partial}{\partial z} f_y \, ,
///   \\ (\mathrm{curl}\,\mathbf{f})_y &=& \frac{\partial}{\partial z} f_x - \frac{\partial}{\partial x} f_z \, ,
///   \\ (\mathrm{curl}\,\mathbf{f})_z &=& \frac{\partial}{\partial x} f_y - \frac{\partial}{\partial y} f_x \, .
/// \end{eqnarray*}
///
/// For the 2D case, $f_z$ is assumed to be zero, and only the z-component of the curl is computed, yielding
/// a scalar output.
///
/// `in` is expected to be a 2D or 3D image with a 2-vector or a 3-vector tensor representation, respectively.
/// However, the image can have more dimensions if they are excluded from processing through `process`.
/// See \ref dip::Gradient for information on the parameters.
///
/// By default this function uses Gaussian derivatives in the computation. Set `method = "finitediff"` for
/// finite difference approximations to the gradient. See \ref dip::Derivative for more information on the other
/// parameters.
///
/// \see dip::Gradient, dip::Divergence
DIP_EXPORT void Curl(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Curl(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Curl( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the divergence of the vector field `in`.
///
/// Divergence is defined as
///
/// $$ \mathrm{div}\,\mathbf{f} = \nabla \cdot \mathbf{f} =
///    \frac{\partial}{\partial x}f_x + \frac{\partial}{\partial y}f_y + \frac{\partial}{\partial z}f_z \; , $$
///
/// with $\mathbf{f}=(f_x, f_y, f_z)$ the vector image `in`. This concept naturally extends to any number
/// of dimensions.
///
/// `in` is expected to have as many dimensions as tensor components. However, the image
/// can have more dimensions if they are excluded from processing through `process`.
/// See \ref dip::Gradient for information on the parameters.
///
/// By default this function uses Gaussian derivatives in the computation. Set `method = "finitediff"` for
/// finite difference approximations to the gradient. See \ref dip::Derivative for more information on the other
/// parameters.
///
/// \see dip::Gradient, dip::Curl
DIP_EXPORT void Divergence(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Divergence(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Divergence( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the Hessian of the image, resulting in a symmetric *NxN* tensor image, if the input
/// was *N*-dimensional.
///
/// The Hessian of input image $f$ is given by $\mathbf{H} = \nabla \nabla^T f$, with tensor components
///
/// $$ \mathbf{H}_{i,j} = \frac{\partial^2}{\partial u_i \partial u_j} f \; . $$
///
/// Each tensor component corresponds to one of the second-order derivatives.
/// Note that $H$ is a symmetric matrix (order of differentiation does not matter). Duplicate entries
/// are not stored in the symmetric tensor image.
///
/// Image dimensions for which `process` is false do not participate in the set of dimensions that form the
/// Hessian matrix. Thus, a 5D image with only two dimensions selected by the `process` array will yield a 2-by-2
/// Hessian matrix.
///
/// By default this function uses Gaussian derivatives in the computation. Set `method = "finitediff"` for
/// finite difference approximations to the gradient. See \ref dip::Derivative for more information on the other
/// parameters.
///
/// The input image must be scalar.
///
/// \see dip::Derivative, dip::Gradient, dip::Laplace
DIP_EXPORT void Hessian(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Hessian(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Hessian( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the Laplacian of the image, equivalent to `dip::Trace( dip::Hessian( in ))`, but more efficient.
///
/// The Laplacian of input image $f$ is written as $\nabla\cdot\nabla f = \nabla^2 f = \Delta f$, and given
/// by
///
/// $$ \Delta f = \sum_i \frac{\partial^2}{\partial u_i^2} f \; . $$
///
/// See \ref dip::Gradient for information on the parameters.
///
/// If `method` is "finitediff", it does not add second order derivatives, but instead computes a convolution
/// with a 3x3(x3x...) kernel where all elements are -1 and the middle element is $3^d - 1$ (with $d$ the number
/// of image dimensions). That is, the kernel sums to 0. For a 2D image, this translates to the well-known kernel:
///
/// $$ \begin{bmatrix}
///      -1 & -1 & -1
///   \\ -1 &  8 & -1
///   \\ -1 & -1 & -1
/// \end{bmatrix} $$
///
/// \see dip::Derivative, dip::Gradient, dip::Hessian, dip::Trace
DIP_EXPORT void Laplace(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Laplace(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Laplace( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Computes the second derivative in the gradient direction.
///
/// The second derivative in the gradient direction is computed by Raleigh quotient of the
/// Hessian matrix and the gradient vector:
///
/// $$ f_{gg} = \frac{ \nabla^T \! f \; \nabla \nabla^T \! f \; \nabla f } { \nabla^T \! f \; \nabla f } \; . $$
///
/// This function is equivalent to:
/// ```cpp
/// Image g = dip::Gradient( in, ... );
/// Image H = dip::Hessian( in, ... );
/// Image Dgg = dip::Transpose( g ) * H * g;
/// Dgg /= dip::Transpose( g ) * g;
/// ```
///
/// See \ref dip::Derivative for how derivatives are computed, and the meaning of the parameters. See \ref dip::Gradient
/// or \ref dip::Hessian for the meaning of the `process` parameter
///
/// \see dip::Dxx, dip::Dyy, dip::Dzz
DIP_EXPORT void Dgg(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Dgg(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   Dgg( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Adds the second derivative in the gradient direction to the Laplacian.
///
/// This function computes `dip::Laplace( in ) + dip::Dgg( in )`, but avoiding computing the second derivatives twice.
///
/// The zero-crossings of the result correspond to the edges in the image, just as they do for the individual
/// Laplace and Dgg operators. However, the localization is improved by an order of magnitude with respect to
/// the individual operators.
///
/// See \ref dip::Laplace and \ref dip::Dgg for more information.
///
/// !!! literature
///     - L.J. van Vliet, "Grey-Scale Measurements in Multi-Dimensional Digitized Images", PhD Thesis, Delft University
///       of Technology, 1993.
///     - P.W. Verbeek and L.J. van Vliet, "On the location error of curved edges in low-pass filtered 2-D and 3-D images",
///       IEEE Transactions on Pattern Analysis and Machine Intelligence 16(7):726-733, 1994.
DIP_EXPORT void LaplacePlusDgg(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image LaplacePlusDgg(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   LaplacePlusDgg( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Subtracts the second derivative in the gradient direction from the Laplacian.
///
/// This function computes `dip::Laplace( in ) - dip::Dgg( in )`, but avoiding computing the second derivatives twice.
///
/// For two-dimensional images, this is equivalent to the second order derivative in the direction perpendicular
/// to the gradient direction.
///
/// See \ref dip::Laplace and \ref dip::Dgg for more information.
DIP_EXPORT void LaplaceMinusDgg(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image LaplaceMinusDgg(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   LaplaceMinusDgg( in, out, std::move( sigmas ), method, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Sharpens `in` by subtracting the Laplacian of the image.
///
/// The actual operation applied is:
/// ```cpp
/// out = in - dip::Laplace( in ) * weight;
/// ```
///
/// See \ref dip::Laplace and \ref dip::Derivative for information on the parameters.
///
/// \see dip::Laplace, dip::UnsharpMask
DIP_EXPORT void Sharpen(
      Image const& in,
      Image& out,
      dfloat weight = 1.0,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image Sharpen(
      Image const& in,
      dfloat weight = 1.0,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   Sharpen( in, out, weight, std::move( sigmas ), method, boundaryCondition, truncation );
   return out;
}

/// \brief Sharpens `in` by subtracting the smoothed image.
///
/// The actual operation applied is:
/// ```cpp
/// out = in * ( 1+weight ) - dip::Gauss( in ) * weight;
/// ```
///
/// See \ref dip::Gauss for information on the parameters.
///
/// \see dip::Gauss, dip::Sharpen
DIP_EXPORT void UnsharpMask(
      Image const& in,
      Image& out,
      dfloat weight = 1.0,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image UnsharpMask(
      Image const& in,
      dfloat weight = 1.0,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   UnsharpMask( in, out, weight, std::move( sigmas ), method, boundaryCondition, truncation );
   return out;
}


/// \brief Finite impulse response implementation of the Gabor filter
///
/// Convolves the image with an FIR 1D Gabor kernel along each dimension. For each dimension,
/// provide a value in `sigmas` and `frequencies`.
/// The value of sigma determines the amount of local averaging. For smaller values, the result is more
/// precise spatially, but less selective of frequencies. Dimensions where sigma is 0 or negative are not processed.
///
/// Frequencies are in the range [0, 0.5), with 0.5 being the frequency corresponding to a period of 2 pixels.
///
/// The output is complex-valued. Typically, the magnitude is the interesting part of the result.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// Set `process` to false for those dimensions that should not be filtered. This is equivalent to setting
/// `sigmas` to 0 for those dimensions.
///
/// This function is relatively slow compared to \ref dip::GaborIIR, even for small sigmas. Prefer to use the IIR
/// implementation.
///
/// \see dip::Gabor2D, dip::GaborIIR
DIP_EXPORT void GaborFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GaborFIR(
      Image const& in,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
) {
   Image out;
   GaborFIR( in, out, std::move( sigmas ), frequencies, boundaryCondition, std::move( process ), truncation );
   return out;
}

/// \brief Recursive infinite impulse response implementation of the Gabor filter
///
/// Convolves the image with an IIR 1D Gabor kernel along each dimension. For each dimension,
/// provide a value in `sigmas` and `frequencies`.
/// The value of sigma determines the amount of local averaging. For smaller values, the result is more
/// precise spatially, but less selective of frequencies. Dimensions where sigma is 0 or negative are not processed.
///
/// Frequencies are in the range [0, 0.5), with 0.5 being the frequency corresponding to a period of 2 pixels.
///
/// The output is complex-valued. Typically, the magnitude is the interesting part of the result.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// Set `process` to false for those dimensions that should not be filtered. This is equivalent to setting
/// `sigmas` to 0 for those dimensions.
///
/// !!! warning
///     The `filterOrder` parameter is not yet implemented. It is ignored and assumed 0 for each dimension.
///
/// \see dip::Gabor2D, dip::GaborFIR
///
/// !!! literature
///     - I.T. Young, L.J. van Vliet and M. van Ginkel, "Recursive Gabor filtering",
///       IEEE Transactions on Signal Processing 50(11):2798-2805, 2002.
DIP_EXPORT void GaborIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      IntegerArray const& filterOrder = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image GaborIIR(
      Image const& in,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      IntegerArray const& filterOrder = {},
      dfloat truncation = 3
) {
   Image out;
   GaborIIR( in, out, std::move( sigmas ), frequencies, boundaryCondition, std::move( process ), filterOrder, truncation );
   return out;
}

/// \brief 2D Gabor filter with direction parameter
///
/// Convolves the 2D image with a Gabor kernel. This is a convenience wrapper around \ref dip::GaborIIR.
/// The value of sigma determines the amount of local averaging, and can be different for each dimension.
/// For smaller values, the result is more precise spatially, but less selective of frequencies.
///
/// `frequency` is in the range [0, 0.5), with 0.5 being the frequency corresponding to a period of 2 pixels.
/// `direction` is the filter direction, in the range [0, 2&pi;].
///
/// The output is complex-valued. Typically, the magnitude is the interesting part of the result.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See \ref dip::BoundaryCondition.
///
/// To use cartesian frequency coordinates, see \ref dip::GaborIIR.
inline void Gabor2D(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 5.0, 5.0 },
      dfloat frequency = 0.1,
      dfloat direction = dip::pi,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF( frequency >= 0.5, "Frequency must be < 0.5" );
   FloatArray frequencies = { frequency * std::cos( direction ), frequency * std::sin( direction ) };
   GaborIIR( in, out, std::move( sigmas ), frequencies, boundaryCondition, {}, {}, truncation );
}
DIP_NODISCARD inline Image Gabor2D(
      Image const& in,
      FloatArray sigmas = { 5.0, 5.0 },
      dfloat frequency = 0.1,
      dfloat direction = dip::pi,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   Gabor2D( in, out, std::move( sigmas ), frequency, direction, boundaryCondition, truncation );
   return out;
}

/// \brief Applies a log-Gabor filter bank
///
/// A log-Gabor filter is a Gabor filter computed on the logarithm of the frequency, leading to a shorter tail of
/// the Gaussian, in the frequency domain, towards the lower frequencies. The origin (DC component) is thus never
/// included in the filter. This gives it better scale selection properties than the traditional Gabor filter.
///
/// This function generates a filter bank with `wavelengths.size()` times `nOrientations` filters. The width of
/// the filters in the angular axis is determined by the number of orientations used, and their locations are
/// always equally distributed over &pi; radian, starting at 0. The radial location (scales) of the filters
/// is determined by `wavelengths` (in pixels), which determines the center for each scale filter. The widths of
/// the filters in this direction are determined by the `bandwidth` parameter; the default value of 0.75 corresponds
/// approximately to one octave, 0.55 to two octaves, and 0.41 to three octaves.
///
/// `wavelengths.size()` and `nOrientations` must be at least 1.
/// If `nOrientations` is 1, no orientation filtering is applied, the filters become purely real. These filters
/// can be defined for images of any dimensionality. For more than one orientation, the filters are complex-valued
/// in the spatial domain, and can only be created for 2D images. See \ref dip::MonogenicSignal for a generalization
/// to arbitrary dimensionality.
///
/// If `in` is not forged, its sizes will be used to generate the filters, which will be returned. Thus, this is
/// identical to (but slightly cheaper than) using a delta pulse image as input.
///
/// The filters are always generated in the frequency domain. If `outRepresentation` is `"spatial"`, the inverse
/// Fourier transform will be applied to bring the result back to the spatial domain. Otherwise, it should be
/// `"frequency"`, and no inverse transform will be applied. Likewise, `inRepresentation` specifies whether `in`
/// has already been converted to the frequency domain or not.
///
/// Out will be a tensor image with `wavelengths.size()` tensor rows and `nFrequencyScales` tensor columns.
/// The data type will be either single-precision float or single-precision complex, depending on the selected
/// parameters.
///
/// !!! literature
///     - D.J. Field, "Relations between the statistics of natural images and the response properties of cortical cells",
///       Journal of the Optical Society of America A 4(12):2379-2394, 1987.
///     - P. Kovesi, ["What Are Log-Gabor Filters and Why Are They Good?"](https://www.peterkovesi.com/matlabfns/PhaseCongruency/Docs/convexpl.html) (retrieved July 25, 2018).
DIP_EXPORT void LogGaborFilterBank(
      Image const& in,
      Image& out,
      FloatArray const& wavelengths = { 3.0, 6.0, 12.0, 24.0 },
      dfloat bandwidth = 0.75, // ~1 octave
      dip::uint nOrientations = 6,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL
);
DIP_NODISCARD inline Image LogGaborFilterBank(
      Image const& in,
      FloatArray const& wavelengths = { 3.0, 6.0, 12.0, 24.0 },
      dfloat bandwidth = 0.75,
      dip::uint nOrientations = 6,
      String const& inRepresentation = S::SPATIAL,
      String const& outRepresentation = S::SPATIAL
) {
   Image out;
   LogGaborFilterBank( in, out, wavelengths, bandwidth, nOrientations, inRepresentation, outRepresentation );
   return out;
}


/// \brief Computes the normalized convolution with a Gaussian kernel: a Gaussian convolution for missing or
/// uncertain data.
///
/// The normalized convolution is a convolution that handles missing or uncertain data. `mask` is an image, expected
/// to be in the range [0,1], that indicates the confidence in each of the values of `in`. Missing values are indicated
/// by setting the corresponding value in `mask` to 0.
///
/// The normalized convolution is then `Convolution( in * mask ) / Convolution( mask )`.
///
/// This function applies convolutions with a Gaussian kernel, using \ref dip::Gauss. See that function for the meaning
/// of the parameters. `boundaryCondition` defaults to `"add zeros"`, the normalized convolution then takes pixels
/// outside of the image domain as missing values.
///
/// !!! literature
///     - H. Knutsson and C. F. Westin, "Normalized and differential convolution", Proceedings of IEEE Conference on
///       Computer Vision and Pattern Recognition, New York, NY, 1993, pp. 515-523.
DIP_EXPORT void NormalizedConvolution(
      Image const& in,
      Image const& mask,
      Image& out,
      FloatArray const& sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = { S::ADD_ZEROS },
      dfloat truncation = 3
);
DIP_NODISCARD inline Image NormalizedConvolution(
      Image const& in,
      Image const& mask,
      FloatArray const& sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = { S::ADD_ZEROS },
      dfloat truncation = 3
) {
   Image out;
   NormalizedConvolution( in, mask, out, sigmas, method, boundaryCondition, truncation );
   return out;
}

/// \brief Computes the normalized differential convolution with a Gaussian kernel: a derivative operator for missing
/// or uncertain data.
///
/// The normalized convolution is a convolution that handles missing or uncertain data. `mask` is an image, expected
/// to be in the range [0,1], that indicates the confidence in each of the values of `in`. Missing values are indicated
/// by setting the corresponding value in `mask` to 0.
///
/// The normalized differential convolution is defined here as the derivative of the normalized convolution with a
/// Gaussian kernel:
///
/// $$ \frac{\partial}{\partial x} \frac{(f \, m) \ast g}{m \ast g}
///    = \frac{(f \, m) \ast \frac{\partial}{\partial x} g}{m \ast g}
///    - \frac{(f \, m) \ast g}{m \ast g} \frac{m \ast \frac{\partial}{\partial x} g}{m \ast g} \; . $$
///
/// $\ast$ is the convolution operator, $f$ is `in`, $m$ is `mask`, and $g$ is the Gaussian kernel
///
/// The derivative is computed along `dimension`.
///
/// This function uses \ref dip::Gauss. See that function for the meaning of the parameters. `boundaryCondition` defaults
/// to `"add zeros"`, the normalized convolution then takes pixels outside of the image domain as missing values.
///
/// !!! literature
///     - H. Knutsson and C. F. Westin, "Normalized and differential convolution", Proceedings of IEEE Conference on
///       Computer Vision and Pattern Recognition, New York, NY, 1993, pp. 515-523.
DIP_EXPORT void NormalizedDifferentialConvolution(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint dimension = 0,
      FloatArray const& sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = { S::ADD_ZEROS },
      dfloat truncation = 3
);
DIP_NODISCARD inline Image NormalizedDifferentialConvolution(
      Image const& in,
      Image const& mask,
      dip::uint dimension = 0,
      FloatArray const& sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = { S::ADD_ZEROS },
      dfloat truncation = 3
) {
   Image out;
   NormalizedDifferentialConvolution( in, mask, out, dimension, sigmas, method, boundaryCondition, truncation );
   return out;
}


/// \brief Computes the mean shift vector for each pixel in the image
///
/// The output image is a vector image, indicating the step to take to move the window center to its center of
/// mass. Repeatedly following the vector will lead to a local maximum of the image `in`. `in` must be scalar
/// and real-valued.
///
/// The mean shift at a given location $x$ is then given by
///
/// $$ s = \frac{ \sum_i{(x-x_i) w(x-x_i) f(x_i)} }{ \sum_i{w(x-x_i) f(x_i)} }
///       = \frac{ \left ( -x w \right) \ast f }{ w \ast f } \; , $$
///
/// where $f$ is the image, $w$ is a windowing function, and $\ast$ indicates convolution.
///
/// We use a Gaussian window with sizes given by `sigmas`. A Gaussian window causes slower convergence than a
/// uniform window, but yields a smooth trajectory and more precise results (according to Comaniciu
/// and Meer, 2002). It also allows us to rewrite the above (with $g_\sigma$ the Gaussian window with
/// parameter $\sigma$) as
///
/// $$ s = \frac{ \left ( -x g_\sigma \right) \ast f }{ g_\sigma \ast f }
///       = \frac{ \left ( \sigma^2 \nabla g_\sigma \right) \ast f }{ g_\sigma \ast f } \; . $$
///
/// Thus, we can write this filter as `dip::Gradient(in, sigmas) / dip::Gauss(in, sigmas) * sigmas * sigmas`.
/// See \ref dip::Derivative for more information on the parameters. Do not use `method = "finitediff"`,
/// as it will lead to nonsensical results.
///
/// \see dip::MeanShift, dip::Gradient, dip::Gauss
///
/// !!! literature
///     - D. Comaniciu and P. Meer, "Mean Shift: A Robust Approach Toward Feature Space Analysis",
///       IEEE Transactions on Pattern Analysis and Machine Intelligence 24(5):603-619, 2002.
DIP_EXPORT void MeanShiftVector(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
);
DIP_NODISCARD inline Image MeanShiftVector(
      Image const& in,
      FloatArray sigmas = { 1.0 },
      String const& method = S::BEST,
      StringArray const& boundaryCondition = {},
      dfloat truncation = 3
) {
   Image out;
   MeanShiftVector( in, out, std::move( sigmas ), method, boundaryCondition, truncation );
   return out;
}


/// \endgroup

} // namespace dip

#endif // DIP_LINEAR_H
