/*
 * DIPlib 3.0
 * This file contains functionality to copy a pixel buffer with cast.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "dip_datatype.h"

namespace dip {

// This function is not available to the library user.
void CopyBuffer(
      void* inBuffer,
      DataType inType,
      dip::sint inStride,
      dip::sint inTensorStride,
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements
);

// Neither are these.
void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dip::sint value
);
void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dfloat value
);
void FillBuffer(
      void* outBuffer,
      DataType outType,
      dip::sint outStride,
      dip::sint outTensorStride,
      dip::uint pixels,
      dip::uint tensorElements,
      dcomplex value
);

} // namespace dip
