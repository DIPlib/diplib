/*
 * DIPlib 3.0
 * This file contains the definition for the CumulativeSum function.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/math.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

void CumulativeSum(
      Image const& in,
      Image const& mask,
      Image& out,
      BooleanArray process
) {
   // TODO: This is a separable filter.
}

} // namespace dip
