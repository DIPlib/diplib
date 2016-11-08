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
      ImageConstRefArray const& c_in,
      ImageRefArray& c_out,
      DataTypeArray const& inBufferTypes,
      DataTypeArray const& outBufferTypes,
      DataTypeArray const& outImageTypes,
      UnsignedArray const& nTensorElements,
      BoundaryConditionArray boundaryConditions,
      PixelTable const& pixelTable,
      FullFilter lineFilter,
      void const* functionParameters,
      std::vector< void* > const& functionVariables,
      FullOptions opts
) {
   // TODO
}

} // namespace Framework
} // namespace dip
