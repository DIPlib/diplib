/*
 * DIPlib 3.0
 * This file contains definitions for all the main classes and functions.
 * Include additional files after this one for specialized functionality.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIPLIB_H
#define DIPLIB_H

/// \file
/// This is the main include file for the DIPlib library.
/// It contains definitions of the core classes in the \ref dip namespace,
/// declarations of the most commonly used library functions, and
/// definitions of some simple inline functions. Include other header
/// files as needed for additional library functionality. The documentation
/// will indicate which header file to include for each function.
///
/// These are the header files included through diplib.h:
///   * dip_datatype.h
///   * dip_dimensionarray.h
///   * dip_error.h
///   * dip_image.h
///   * dip_math.h
///   * dip_operators.h
///   * dip_support.h
///   * dip_tensor.h
///   * dip_types.h

#include "dip_operators.h" // This file includes all the others hierarchically.

#endif // DIPLIB_H
