/*
 * DIPlib 3.0
 * This file contains definitions for the full framework.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <new>
#include <iostream>

#include "diplib.h"
#include "diplib/framework.h"

#include "copy_buffer.h"

namespace dip {
namespace Framework {

void Full(
      Image const& in,
      Image& out,
      DataType inBufferTypes,
      DataType outBufferTypes,
      DataType outImageTypes,
      dip::uint nTensorElements,
      BoundaryConditionArray boundaryConditions,
      PixelTable const& pixelTable,
      FullLineFilter* lineFilter,
      FullOptions opts
) {
   // TODO
}

} // namespace Framework
} // namespace dip
