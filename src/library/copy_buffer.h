/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/boundary.h"

namespace dip {

// Copies pixels from one 1D buffer to another, converting data type using clamp_cast.
//
// This function is not available to the library user.
void CopyBuffer(
      void const* inBuffer,
      DataType inType,
      dip::sint inStride,
      dip::sint inTensorStride,
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,          // number of pixels in the two buffers
      dip::uint tensorElements,  // number of samples per pixel
      std::vector< dip::sint > const& lookUpTable // it this is null, simply copy over the tensor as is; otherwise use this to determine which tensor values to copy where
);

// Expands the boundary of a 1D buffer, which extends `boundary` pixels on either side.
// That is, the total number of pixels in the buffer is `pixels` + 2*`boundary`, but the `buffer` pointer
// points at the middle `pixels` elements, which are filled in. This function fills out the other 2*`boundary`
// pixels according to `bc`.
//
// This function is not available to the library user.
void ExpandBuffer(
      void* buffer,
      DataType type,
      dip::sint stride,
      dip::sint tensorStride,
      dip::uint pixels,          // number of pixels in the buffer
      dip::uint tensorElements,  // number of samples per pixel
      dip::uint boundary,        // number of pixels before and after buffer to fill
      BoundaryCondition bc
);

// Fills one 1D buffer with a constant value `value`.
//
// Available types `inT` are: dip::sint, dip::dfloat, dip::dcomplex.
// Make sure you don't use a different type, or you'll get linker errors.
//
// This function is not available to the library user.
template< typename inT >
void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      inT value
);

} // namespace dip
