/*
 * DIPlib 3.0
 * This file contains definitions for the separable framework.
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

void Separable(
      Image const& in,
      Image& out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint nTensorElements,
      BooleanArray process,
      UnsignedArray border,
      BoundaryConditionArray boundaryConditions,
      SeparableFilter lineFilter,
      void const* functionParameters,
      std::vector< void* > const& functionVariables,
      SeparableOptions opts
) {

}

} // namespace Framework
} // namespace dip
