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


/// \file
/// \brief This is the main include file for the *DIPlib* library.
///
/// It declares all classes, functions, macros and constants that form the basic
/// library infrastructure. Everything is declared within the `#dip` namespace.
/// This header file is the core of *DIPlib*. To access image processing or analysis
/// functionality, include their corresponding header files.


/// \defgroup infrastructure The library infrastructure
/// \brief The nuts and bolts that make it all work

/// \defgroup filtering Filtering
/// \brief Linear and non-linear filters for smoothing, sharpening and detection

#endif // DIPLIB_H
