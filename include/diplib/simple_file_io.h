/*
 * DIPlib 3.0
 * This file contains definitions for a simple file reading and writing interface
 *
 * (c)2019, Cris Luengo.
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

#ifndef DIP_SIMPLE_FILE_IO_H
#define DIP_SIMPLE_FILE_IO_H

#include "diplib.h"
#include "diplib/file_io.h"
#ifdef DIP__HAS_JAVAIO
#include "diplib/javaio.h"
#endif


/// \file
/// \brief Functions for reading and writing images from/to files.
/// \see file_io


namespace dip {


/// \addtogroup file_io
/// \{

/// \brief Reads the image in a file `filename`, and puts it in `out`.
///
/// `format` can be one of:
/// - `"ics"`: The file is an ICS file, use `dip::ImageReadICS`.
/// - `"tiff"`: The file is a TIFF file, use `dip::ImageReadTIFF`. Reads only the first image plane.
/// - `"jpeg"`: The file is a JPEG file, use `dip::ImageReadJPEG`.
/// - `"bioformats"`: Use `dip::javaio::ImageReadJavaIO` to read the file with the *Bio-Formats* library.
/// - `""`: Select the format by looking at the file name extension. This is the default.
///
/// Information about the file and all metadata are returned in the `FileInformation` output argument.
///
/// If *DIPjavaio* is not linked against, the `"bioformats"` format will not exist. Note that when linking
/// against the *DIPjavaio* library, `DIP__HAS_JAVAIO` should be defined (but might need to be defined manually
/// if not using *CMake*).
///
/// Use the filetype-specific functions directly for more control over how the image is read.
inline FileInformation ImageRead(
      Image& out,
      String const& filename,
      String format = ""
) {
   if( format.empty() ) {
      format = FileGetExtension( filename );
      if( StringCompareCaseInsensitive( format, "ics" ) || StringCompareCaseInsensitive( format, "ids" )) {
         format = "ics";
      } else if( StringCompareCaseInsensitive( format, "tif" ) || StringCompareCaseInsensitive( format, "tiff" )) {
         format = "tiff";
      } else if( StringCompareCaseInsensitive( format, "jpg" ) || StringCompareCaseInsensitive( format, "jpeg" )) {
         format = "jpeg";
      } else {
         format = "bioformats";
      }
   }
   FileInformation info;
   if( format == "ics" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadICS( out, filename ));
   } else if( format == "tiff" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadTIFF( out, filename ));
   } else if( format == "jpeg" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadJPEG( out, filename ));
   }
#ifdef DIP__HAS_JAVAIO
   else if( format == "bioformats" ) {
      DIP_STACK_TRACE_THIS( info = javaio::ImageReadJavaIO( out, filename, javaio::bioformatsInterface ));
   }
#endif
   else {
      DIP_THROW_INVALID_FLAG( format );
   }
   return info;
}
inline Image ImageRead(
      String const& filename,
      String const& format = ""
) {
   Image out;
   ImageRead( out, filename, format );
   return out;
}

/// \brief Writes `image` to file.
///
/// `format` can be one of:
/// - `"ics"` or `"icsv2"`: Create an ICS version 2 file, use `dip::ImageWriteICS`.
/// - `"icsv1"`: Create an ICS version 1 file, use `dip::ImageWriteICS`.
/// - `"tiff"`: Create a TIFF file, use `dip::ImageWriteTIFF`.
/// - `"jpeg"`: Create a JPEG file, use `dip::ImageWriteJPEG`.
/// - `""`: Select the format by looking at the file name extension.
///         If no extension is present, it defaults to ICS version 2.
///         This is the default.
///
/// The ICS format can store any image, with all its information, such that reading the file using `dip::ImageRead`
/// or `dip::ImageReadICS` yields an image that is identical (except the strides might be different).
///
/// The TIFF format can store 2D images, as well as 3D images as a series of 2D slides (not yet implemented). Most
/// metadata will be lost. Complex data is not supported, other data types are. But note that images other than
/// 8-bit or 16-bit unsigned integer lead to files that are not recognized by most readers.
///
/// The JPEG format can store 2D images. Tensor images are always tagged as RGB. Most metadata will be lost.
/// Image data is converted to 8-bit unsigned integer, without scaling.
///
/// `compression` determines the compression method used when writing the pixel data. It can be one of the
/// following strings:
///  - `"none"`: no compression.
///  - `""`: gzip compression (default). TIFF files with gzip compression are not universally recognized.
///  - `"LZW"`, `"PackBits"`, `"JPEG"`: compression formats supported only by the TIFF format.
///
/// For the JPEG format, `compression` is ignored.
///
/// Use the filetype-specific functions directly for more control over how the image is written. See those
/// functions for more information about the file types.
inline void ImageWrite(
      Image const& image,
      String const& filename,
      String format = "",
      String const& compression = ""
) {
   if( format.empty() ) {
      format = FileGetExtension( filename );
      if( format.empty() || StringCompareCaseInsensitive( format, "ics" )) {
         format = "icsv2";
      } else if( StringCompareCaseInsensitive( format, "tif" ) || StringCompareCaseInsensitive( format, "tiff" )) {
         format = "tiff";
      } else if( StringCompareCaseInsensitive( format, "jpg" ) || StringCompareCaseInsensitive( format, "jpeg" )) {
         format = "jpeg";
      } else {
         DIP_THROW( "File extension not recognized" );
      }
   }
   StringSet options = {};
   if(( format == "ics" ) || ( format == "icsv2" )) {
      format = "ics";
   } else if( format == "icsv1" ) {
      format = "ics";
      options.insert( "v1" );
   }
   if( format == "ics" ) {
      if( compression.empty() ) {
         options.insert( "gzip" );
      } else if( compression == "none" ) {
         options.insert( "uncompressed" );
      } else {
         DIP_THROW_INVALID_FLAG( compression );
      }
      DIP_STACK_TRACE_THIS( ImageWriteICS( image, filename, {}, 0, options ));
   } else if( format == "tiff" ) {
      DIP_STACK_TRACE_THIS( ImageWriteTIFF( image, filename, compression ));
   } else if( format == "jpeg" ) {
      DIP_STACK_TRACE_THIS( ImageWriteJPEG( image, filename ));
   } else {
      DIP_THROW_INVALID_FLAG( format );
   }
}

/// \}

} // namespace dip

#endif // DIP_SIMPLE_FILE_IO_H
