/*
 * DIPlib 3.0
 * This file contains declarations for reading and writing images from/to files
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_FILE_IO_H
#define DIP_FILE_IO_H

#include "diplib.h"


/// \file
/// \brief Declares functions for reading and writing images from/to files.


namespace dip {


/// \defgroup file_io Image File I/O
/// \brief Reading images from files and writing them to files.
/// \{

// TODO: functions to port:
/*
   dipio_ImageRead (dipio_image.h) (should always return color image)
   dipio_ImageReadColourSeries (dipio_image.h) (should be named ImageReadSeries)
   dipio_ImageReadROI (dipio_image.h)
   dipio_ImageFileGetInfo (dipio_image.h)
   dipio_ImageWrite (dipio_image.h)
   dipio_FileGetExtension (dipio_image.h)
   dipio_FileAddExtension (dipio_image.h)
   dipio_FileCompareExtension (dipio_image.h)
   dipio_ImageFindForReading (dipio_image.h)
   dipio_ImageReadICS (dipio_ics.h)
   dipio_ImageReadICSInfo (dipio_ics.h)
   dipio_ImageIsICS (dipio_ics.h)
   dipio_AppendRawData (dipio_ics.h)
   dipio_ImageWriteICS (dipio_ics.h)
   dipio_ImageReadTIFF (dipio_tiff.h) (needs to support tiled images at some point!)
   dipio_ImageReadTIFFInfo (dipio_tiff.h)
   dipio_ImageIsTIFF (dipio_tiff.h)
   dipio_ImageWriteTIFF (dipio_tiff.h)
*/

/// \}

} // namespace dip

#endif // DIP_FILE_IO_H
