/*
 * (c)2019-2021, Wouter Caarls.
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
/// - `"org/diplib/BioFormatsInterface"`: The *Bio-Formats* package (default).
///   Can read [over 160 different image file formats](https://bio-formats.readthedocs.io/en/latest/supported-formats.html).
///   Limitation: each x-y plane of a (multi-dimensional) image must occupy no more than 2 GB when uncompressed,
///   and each individual dimension must be smaller than 2^31^-1. Metadata is currently not read.
///
/// If the file contains multiple images, select the desired one by setting `imageNumber`. Note that it is the
/// interface that decides what an image is. For example, *Bio-Formats* will consider multi-page TIFF files
/// to be either a single 3D image or a series of individual images depedning on some internal logic.
///
/// Information about the file and all metadata are returned in the \ref dip::FileInformation output argument.
DIPJAVAIO_EXPORT FileInformation ImageReadJavaIO(
      Image& out,
      String const& filename,
      String const& interface = bioformatsInterface,
      dip::uint imageNumber = 0
);
DIP_NODISCARD inline Image ImageReadJavaIO(
      String const& filename,
      String const& interface = bioformatsInterface,
      dip::uint imageNumber = 0
) {
   Image out;
   ImageReadJavaIO( out, filename, interface, imageNumber );
   return out;
}

/// \endgroup

} // namespace javaio

} // namespace dip

#endif // DIP_JAVAIO_H
