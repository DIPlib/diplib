/*
 * DIPlib 3.0
 * This file contains declarations for reading images through Java
 *
 * (c)2019, Wouter Caarls.
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

#ifndef DIP_JAVAIO_H
#define DIP_JAVAIO_H

#include "diplib.h"
#include "diplib/file_io.h"
#include "diplib/javaio/export.h"

/// \file
/// \brief Declares the functionality for \ref dipjavaio.

namespace dip {

/// \group dipjavaio *DIPjavaio*
/// \ingroup file_io
/// \brief An interface to Java file I/O functionality
/// \addtogroup

/// \brief Contains all functionality for \ref dipjavaio.
namespace javaio {

constexpr char const* bioformatsInterface = "org/diplib/BioFormatsInterface";

/// \brief Reads the image in a file `filename` recognized by a Java `interface` and puts it in `out`.
///
/// `interface` can be one of:
///
/// - `"org/diplib/BioFormatsInterface"`: The openmicroscopy.org Bio-Formats package (default).
///
/// Information about the file and all metadata are returned in the \ref dip::FileInformation output argument.
DIPJAVAIO_EXPORT FileInformation ImageReadJavaIO(
      Image& out,
      String const& filename,
      String const& interface = bioformatsInterface
);
inline Image ImageReadJavaIO(
      String const& filename,
      String const& interface = bioformatsInterface
) {
   Image out;
   ImageReadJavaIO( out, filename, interface );
   return out;
}

/// \endgroup

} // namespace javaio

} // namespace dip

#endif // DIP_JAVAIO_H
