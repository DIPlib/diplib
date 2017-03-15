/*
 * DIPlib 3.0
 * This file contains declarations for linear image filters
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

#ifndef DIP_LINEAR_H
#define DIP_LINEAR_H

#include "diplib.h"
#include "diplib/boundary.h"


/// \file
/// \brief Declares functions that implement linear filters.


namespace dip {


/// \defgroup linear Linear filters
/// \ingroup filtering
/// \brief Functions that implement linear filters
/// \{

/// \brief Describes a 1D filter
///
/// The weights are in `filter`. The origin is placed either at the index given by `origin`, if it's non-negative, or
/// at index `filter.size() / 2` if `origin` is negative. This location is either the middle pixel if the filter is
/// odd in length, or the pixel to the right of the center if it is even in length:
///
///     filter size is odd :      filter data :  x x x x x        orgin = -1
///                                                  ^
///                                                  0
///
///     filter size is even :     filter data :  x x x x x x      orgin = -1
///                                                    ^
///                                                    0
///     origin specified :        filter data :  x x x x x x      orgin = 1
///                                                ^
///                                                0
///
/// Note that `origin` must be an index to one of the samples in the `filter` array
///
/// `symmetry` indicates the filter shape: `"general"` (or an empty string) indicates no symmetry.
/// `"even"` indicates even symmetry, and `"odd"` indicates odd symmetry. In both cases, the filter represents
/// the left half of the full filter, with the righmost element at the origin (and not repeated). The full filter
/// is thus always odd in size. `"d-even"` and `"d-odd"` are similar, but duplicate the rightmost element, yielding
/// an even-sized filter. The origin for the symmetric filters is handled identically to the general filter case.
///
///     filter array:                a  b  c              array has N elements
///
///     symmetry = "general":        a  b  c              filter size = N
///     symmetry = "even":           a  b  c  b  a        filter size = N + N - 1
///     symmetry = "odd":            a  b  c -b -a        filter size = N + N - 1
///     symmetry = "d-even":         a  b  c  c  b  a     filter size = N + N
///     symmetry = "d-odd":          a  b  c -c -b -a     filter size = N + N
///
/// The convolution is applied to each tensor component separately, which is always the correct behavior for linear
/// filters.
struct OneDimensionalFilter {
   FloatArray filter;            ///< Filter weights
   dip::sint origin = -1;        ///< Origin of the filter if non-negative
   String symmetry = "";         ///< Filter shape: `""` == `"general"`, `"even"`, `"odd"`, `"d-even"` or `"d-odd"`
};

/// \brief An array of 1D filters
using OneDimensionalFilterArray = std::vector< OneDimensionalFilter >;

// TODO: Implement code to separate an image into 1D filters to be applied with SeparableConvolution
OneDimensionalFilterArray SeparateFilter( Image const& filter );

/// \brief Applies a convolution with a filter kernel (PSF) that is separable.
///
/// `filter` is an array with exactly one element for each dimension of `in`. Alternatively, it can have a single
/// element, which will be used unchanged for each dimension. For the dimensions that are not processed (`process` is
/// `false` for those dimensions), the `filter` array can have non-sensical data or a zero-length filter weights array.
/// Any `filter` array that is zero size or the equivalent of `{1}` will not be applied either.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// \see dip::SeparateFilter, dip::GeneralConvolution, dip::ConvolveFT, dip::Framework::Separable
void SeparableConvolution(
      Image const& in,                    ///< Input image
      Image& out,                         ///< Output image
      OneDimensionalFilterArray const& filterArray, ///< The filter
      StringArray const& boundaryCondition = {}, ///< The boundary condition
      BooleanArray process = {}           ///< Which dimensions to process, can be `{}` to indicate all dimensions are to be processed
);
inline Image SeparableConvolution(
      Image const& in,
      OneDimensionalFilterArray const& filter,
      StringArray const& boundaryCondition = {},
      BooleanArray const& process = {}
) {
   Image out;
   SeparableConvolution( in, out, filter, boundaryCondition, process );
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
/// to `"frequency"` (actually, any string different from `"spatial"` will do). Similarly, if
/// `outRepresentation` is `"frequency"`, the output will not be inverse-transformed, so will be in
/// the frequency domain.
///
/// \see dip::GeneralConvolution, dip::SeparableConvolution
void ConvolveFT(
      Image const& in,
      Image const& filter,
      Image& out,
      String const& inRepresentation = "spatial",
      String const& filterRepresentation = "spatial",
      String const& outRepresentation = "spatial"
);
inline Image ConvolveFT(
      Image const& in,
      Image const& filter,
      String const& inRepresentation = "spatial",
      String const& filterRepresentation = "spatial",
      String const& outRepresentation = "spatial"
) {
   Image out;
   ConvolveFT( in, filter, out, inRepresentation, filterRepresentation, outRepresentation );
   return out;
}

/// \brief Applies a convolution with a filter kernel (PSF) by direct implementation of the convolution sum.
///
/// `filter` is an image, and must be equal in size or smaller than `in`. `filter` must be real-valued.
///
/// As elsewhere, the origin of `filter` is in the middle of the image, on the pixel to the right of
/// the center in case of an even-sized image.
///
/// Note that this is a really expensive way to compute the convolution for any `filter` that has more than a
/// small amount of non-zero values. It is always advantageous to try to separate your filter into a set of 1D
/// filters (see `dip::SeparateFilter` and `dip::SeparableConvolution`). If this is not possible, use
/// `dip::ConvolveFT` with larger filters to compute the convolution in the Fourier domain.
///
/// Also, if all non-zero filter weights have the same value, `dip::Uniform` implements a more efficient
/// algorithm. If `filter` is a binary image, `dip::Uniform` is called.
///
/// `boundaryCondition` indicates how the boundary should be expanded in each dimension. See `dip::BoundaryCondition`.
///
/// \see dip::ConvolveFT, dip::SeparableConvolution, dip::SeparateFilter, dip::Uniform
void GeneralConvolution(
      Image const& in,
      Image const& filter,
      Image& out,
      StringArray const& boundaryCondition = {}
);
inline Image GeneralConvolution(
      Image const& in,
      Image const& filter,
      StringArray const& boundaryCondition = {}
) {
   Image out;
   GeneralConvolution( in, filter, out, boundaryCondition );
   return out;
}

void Uniform(
      Image const& in,
      Image& out,
      FloatArray filterSize = { 7 },
      String const& filterShape = "elliptic",
      StringArray const& boundaryCondition = {}
);
void Uniform(
      Image const& in,
      Image const& neighborhood,
      Image& out,
      StringArray const& boundaryCondition = {},
      String const& mode = "" // set to "convolution" to mirror `neighborhood`
);

void Gauss(
      Image const& in,
      Image& out,
      FloatArray const& sigmas = { 1 },
      IntegerArray const& derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {}, // ignored if GaussFT
      BooleanArray const& process = {},
      String const& method = "optimal" // "FIR","IIR","FT","optimal"
);

void GaussFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      IntegerArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3 // truncation gets automatically increased for higher-order derivatives
);

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      IntegerArray derivativeOrder = { 0 },
      dfloat truncation = 0
);

void GaussIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas = { 1 },
      IntegerArray derivativeOrder = { 0 },
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      IntegerArray filterOrder = {},
      dip::uint designMethod = 0, // should be a string
      dfloat truncation = 3 // truncation gets automatically increased for higher-order derivatives
);

void GaborFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      FloatArray frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      dfloat truncation = 3
);

void GaborIIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      FloatArray frequencies,
      StringArray const& boundaryCondition = {},
      BooleanArray process = {},
      IntegerArray filterOrder = {},
      dfloat truncation = 3
);

void OrientedGauss(
      Image const& in,
      Image& out,
      FloatArray,
      FloatArray
);

void SobelGradient( // this is a call to FiniteDifference
      Image const& in,
      Image& out,
      dip::uint dimension = 0,
      StringArray const& boundaryCondition = {}
);

void FiniteDifference(
      Image const& in,
      Image& out,
      IntegerArray derivativeOrder = { 0 },
      bool smoothFlag = true, // Should be a string
      StringArray const& boundaryCondition = {},
      BooleanArray process = {}
);

/// \}

} // namespace dip

#endif // DIP_LINEAR_H
