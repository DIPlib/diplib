/*
 * DIPlib 3.0
 * This file contains definitions for all the main classes and functions.
 * Include additional files after this one for specialized functionality.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIPLIB_H
#define DIPLIB_H


#include "diplib/library/dimension_array.h"
#include "diplib/library/types.h"
#include "diplib/library/numeric.h"
#include "diplib/library/clamp_cast.h"
#include "diplib/library/error.h"
#include "diplib/library/datatype.h"
#include "diplib/library/tensor.h"
#include "diplib/library/physical_dimensions.h"
#include "diplib/library/image.h"
#include "diplib/library/operators.h"


/// \file
/// \brief This is the main include file for the *DIPlib* library.
///
/// It declares all classes, functions, macros and constants that form the basic
/// library infrastructure. Everything is declared within the `#dip` namespace.
/// This header file is the core of *DIPlib*. To access image processing or analysis
/// functionality, include their corresponding header files.


/// \defgroup infrastructure The library infrastructure
/// \brief The nuts and bolts that make it all work

/// \defgroup filtering Image filters
/// \brief Functions that implement linear and non-linear filters

#endif // DIPLIB_H
