/*
 * DIPlib 3.0
 * This file contains the definition for the Label function.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <set>

#include "diplib.h"
#include "diplib/regions.h"


namespace dip {

dip::uint Label(
      Image const& binary,
      Image& out,
      dip::uint connectivity,
      String mode,
      dip::uint minSize,
      dip::uint maxSize,
      BoundaryConditionArray bc
) {
   // TODO
   return 0;
}

} // namespace dip
