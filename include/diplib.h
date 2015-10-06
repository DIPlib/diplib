/*
 * DIPlib 3.0
 * This file contains definitions for all the main classes and functions.
 * Include additional files after this one for specialized functionality.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIPLIB_H
#define DIPLIB_H

/// The dip namespace contains all the library funcionality.
namespace dip {
   class Image;      // Forward declaration, for use by some functions
                     // declared in header files that are loaded before
                     // dip_image.h.
}

#include "dip_error.h"
#include "dip_datatype.h"
#include "dip_support.h"
#include "dip_tensor.h"
#include "dip_pixel.h"
#include "dip_image.h"

#endif // DIPLIB_H
