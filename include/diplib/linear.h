/*
 * DIPlib 3.0
 * This file contains declarations for measurement-related classes
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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
/// `symmetry` indicates the filter shape: `"general"` (or an empty string) indicates no symmetry, and the whole filter
/// array is used. `"even"` indicates even symmetry, and `"odd"` indicates odd symmetry. In the symmetric cases, only
/// the first half of the filter data is used:
///
///     filter data (odd size):      a  b  c  d  e  f  g      orgin = 3
///
///     symmetry = "general":        a  b  c  d  e  f  g
///     symmetry = "even":           a  b  c  d  c  b  a
///     symmetry = "odd":            a  b  c  d -c -b -a
///
///     filter data (even size):     a  b  c  d  e  f         orgin = 3
///
///     symmetry = "general":        a  b  c  d  e  f
///     symmetry = "even":           a  b  c  c  b  a
///     symmetry = "odd":            a  b  c -c -b -a
///
/// The convolution is applied to each tensor component separately, which is always the correct behavior for linear
/// filters.
struct OneDimensionalFilter{
   FloatArray filter;            ///< Filter weights
   dip::sint origin = -1;        ///< Origin of the filter if non-negative
   String symmetry = "";         ///< Filter shape: `""` == `"general"`, `"even"` or `"odd"`
};

/// \brief An array of 1D filters
using OneDimensionalFilterArray = std::vector< OneDimensionalFilter >;


/// \brief Applies a convolution with a filter kernel (PSF) that is separable.
///
/// `filter` is an array with exactly one element for each dimension of `in`. Alternatively, it can have a single
/// element, which will be used unchanged for each dimension. For the dimensions that are not processed (`process` is
/// `false` for those dimensions), the `filter` array can have non-sensical data or a zero-length filter weights array.
/// Any `filter` array that is zero size or the equivalent of `{1}` will not be applied either.
///
/// \see dip::GeneralConvolution, dip::ConvolveFT, dip::Framework::Separable
void SeparableConvolution(
      Image const& in,                    ///< Input image
      Image& out,                         ///< Output image
      OneDimensionalFilterArray const& filterArray, ///< The filter
      StringArray boundaryCondition = {}, ///< The boundary condition
      BooleanArray process = {}           ///< Which dimensions to process, can be `{}` to indicate all dimensions are to be processed
);
inline Image SeparableConvolution(
      Image const& in,
      OneDimensionalFilterArray const& filter,
      StringArray boundaryCondition = {},
      BooleanArray process = {}
) {
   Image out;
   SeparableConvolution( in, out, filter, boundaryCondition, process );
   return out;
}

// TODO: implement code to separate an image into 1D filters to be applied with SeparableConvolution.

void ConvolveFT(
      Image const& in,
      Image const& filter,
      Image& out,
      String const& inRepresentation,
      String const& filterRepresentation,
      String const& outRepresentation
);

void GeneralConvolution(
      Image const& in,
      Image const& filter,
      Image& out,
      StringArray boundaryCondition
);

void Uniform(
      Image const& in,
      Image& out,
      Image& neighborhood,
      StringArray boundaryCondition,
      FloatArray,
      String const& filterShape
);

void Gauss(
      Image const& in,
      Image& out,
      StringArray boundaryCondition,
      BooleanArray,
      FloatArray,
      IntegerArray,
      dfloat
);

void GaussFT(
      Image const& in,
      Image& out,
      FloatArray,
      IntegerArray,
      dfloat
);

void GaussIIR(
      Image const& in,
      Image& out,
      StringArray boundaryCondition,
      BooleanArray,
      FloatArray,
      IntegerArray,
      IntegerArray,
      dip::uint,
      dfloat
);

void GaborIIR(
      Image const& in,
      Image& out,
      StringArray boundaryCondition,
      BooleanArray,
      FloatArray,
      FloatArray,
      IntegerArray,
      dfloat
);

void OrientedGauss(
      Image const& in,
      Image& out,
      FloatArray,
      FloatArray
);

void SobelGradient(
      Image const& in,
      Image& out,
      StringArray boundaryCondition,
      dip::uint
);

void FiniteDifference(
      Image const& in,
      Image& out,
      StringArray boundaryCondition,
      BooleanArray,
      IntegerArray,
      bool
);

/// \}

} // namespace dip

#endif // DIP_LINEAR_H
