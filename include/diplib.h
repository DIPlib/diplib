/*
 * DIPlib 3.0
 * This file contains definitions for all the main classes and functions.
 * Include additional files after this one for specialized functionality.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DIPLIB_H
#define DIPLIB_H


#include "diplib/library/dimension_array.h"
#include "diplib/library/types.h"
#include "diplib/library/sample_iterator.h"
#include "diplib/library/numeric.h"
#include "diplib/library/clamp_cast.h"
#include "diplib/library/error.h"
#include "diplib/library/datatype.h"
#include "diplib/library/tensor.h"
#include "diplib/library/physical_dimensions.h"
#include "diplib/library/image.h"
#include "diplib/library/image_views.h"
#include "diplib/library/operators.h"
#include "diplib/library/stringparams.h"


/// \file
/// \brief This is the main include file for the *DIPlib* library.
///
/// This header file is the core of *DIPlib*. It declares all classes, functions,
/// macros and constants that form the basic library infrastructure. It does so
/// by including all the header files in the \link_to_include_diplib_library directory.
/// Everything is declared within the `#dip` namespace.
///
/// To access image processing or analysis functionality, include their
/// corresponding header files.

/// \brief The `dip` namespace contains all the library functionality.
namespace dip {

/// \defgroup infrastructure The library infrastructure
/// \brief The nuts and bolts that make it all work
/// \{

/// \brief Holds information about the *DIPlib* binary.
///
/// Information in this structure can be used to display library information
/// to the user (e.g. in a splash screen). It also contains information that
/// can be used to determine at run time the properties of the DIPlib library
/// that the application links to. Refer to `dip::libraryInformation`.
///
/// The string `type` starts either with "Release" or "Debug", indicating
/// the whether the library was compiled with optimizations enabled and no
/// debug information (release) or without optimizations and with debug
/// information (debug). Next it lists a series of compile-time options,
/// separated by comma. The options are:
///  - "with OpenMP": indicates multithreading is built in.
///  - "recording stack traces": indicates that exceptions report a stack trace,
///    rather than only show the function that threw it.
///  - "asserts enabled": indicates additional run-time tests for consistency
///    are executed.
///  - "Unicode support": indicates e.g. units are output using Unicode.
///  - "ICS support": indicates ICS file reading and writing is available.
///  - "TIFF support": indicates TIFF file reading and writing is available.
struct DIP_NO_EXPORT LibraryInformation {
   String name;         ///< The library name
   String description;  ///< A short description string
   String copyright;    ///< Copyright string for the library
   String URL;          ///< Library website, with contact information etc.
   String version;      ///< The library version number
   String date;         ///< Compilation date
   String type;         ///< Describes options enabled during compilation
};

/// \brief Constant that holds information about the *DIPlib* binary.
DIP_EXPORT extern const LibraryInformation libraryInformation;

/// \}

} // namespace dip

/// \defgroup filtering Filtering
/// \brief Linear and non-linear filters for smoothing, sharpening and detection

/// \dir include/diplib
/// \brief Contains include files for the various modules in DIPlib

/// \dir include/diplib/library
/// \brief Contains core include files for DIPlib, they are all included by `diplib.h`

#endif // DIPLIB_H
