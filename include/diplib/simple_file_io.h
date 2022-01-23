/*
 * (c)2019-2021, Cris Luengo.
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
#ifdef DIP_CONFIG_HAS_DIPJAVAIO
#include "diplib/javaio.h"
#endif


/// \file
/// \brief Functions for reading and writing images from/to files.
/// See \ref file_io.


namespace dip {


/// \addtogroup file_io

/// \brief Reads the image in a file `filename`, and puts it in `out`.
///
/// `format` can be one of:
///
/// - `"ics"`: The file is an ICS file, use \ref dip::ImageReadICS.
/// - `"tiff"`: The file is a TIFF file, use \ref dip::ImageReadTIFF. Reads only the first image plane.
/// - `"jpeg"`: The file is a JPEG file, use \ref dip::ImageReadJPEG.
/// - `"npy"`: The file is a NumPy NPY file, use \ref dip::ImageReadNPY.
/// - `"bioformats"`: Use \ref dip::javaio::ImageReadJavaIO to read the file with the *Bio-Formats* library.
/// - `""`: Select the format by looking at the file name extension or the file's first few bytes. This is the default.
///
/// Information about the file and all metadata are returned in the \ref dip::FileInformation output argument.
///
/// If *DIPjavaio* is not linked against, the `"bioformats"` format will not exist. Note that when linking
/// against the *DIPjavaio* library, `DIP_CONFIG_HAS_DIPJAVAIO` should be defined (but might need to be defined manually
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
         DIP_THROW_IF( !ImageIsICS( filename ), "File has an ICS extension but is not an ICS file" );
         format = "ics";
      } else if( StringCompareCaseInsensitive( format, "tif" ) || StringCompareCaseInsensitive( format, "tiff" )) {
         DIP_THROW_IF( !ImageIsTIFF( filename ), "File has a TIFF extension but is not a TIFF file" );
         format = "tiff";
      } else if( StringCompareCaseInsensitive( format, "jpg" ) || StringCompareCaseInsensitive( format, "jpeg" )) {
         DIP_THROW_IF( !ImageIsJPEG( filename ), "File has a JPEG extension but is not a JPEG file" );
         format = "jpeg";
      } else if( StringCompareCaseInsensitive( format, "npy" )) {
         DIP_THROW_IF( !ImageIsNPY( filename ), "File has an NPY extension but is not an NPY file" );
         format = "npy";
      } else if( ImageIsICS( filename )) {
         format = "ics";
      } else if( ImageIsTIFF( filename )) {
         format = "tiff";
      } else if( ImageIsJPEG( filename )) {
         format = "jpeg";
      } else if( ImageIsNPY( filename )) {
         format = "npy";
      } else {
#ifdef DIP_CONFIG_HAS_DIPJAVAIO
         format = "bioformats";
#else
         DIP_THROW( "File not of a recognized format" );
#endif
      }
   }
   FileInformation info;
   if( format == "ics" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadICS( out, filename ));
   } else if( format == "tiff" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadTIFF( out, filename ));
   } else if( format == "jpeg" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadJPEG( out, filename ));
   } else if( format == "npy" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadNPY( out, filename ));
   }
#ifdef DIP_CONFIG_HAS_DIPJAVAIO
   else if( format == "bioformats" ) {
      DIP_STACK_TRACE_THIS( info = javaio::ImageReadJavaIO( out, filename, javaio::bioformatsInterface ));
   }
#endif
   else {
      DIP_THROW_INVALID_FLAG( format );
   }
   return info;
}
DIP_NODISCARD inline Image ImageRead(
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
///
/// - `"ics"` or `"icsv2"`: Create an ICS version 2 file, use \ref dip::ImageWriteICS.
/// - `"icsv1"`: Create an ICS version 1 file, use \ref dip::ImageWriteICS.
/// - `"tiff"`: Create a TIFF file, use \ref dip::ImageWriteTIFF.
/// - `"jpeg"`: Create a JPEG file, use \ref dip::ImageWriteJPEG.
/// - `"npy"`: Create a NumPy NPY file, use \ref dip::ImageWriteNPY.
/// - `""`: Select the format by looking at the file name extension. If no extension is
///   present, it defaults to ICS version 2. This is the default.
///
/// The ICS format can store any image, with all its information, such that reading the file using \ref dip::ImageRead
/// or \ref dip::ImageReadICS yields an image that is identical (except the strides might be different).
///
/// The TIFF format can store 2D images, as well as 3D images as a series of 2D slides (not yet implemented).
/// A limited set of color spaces are recognized, other color images are stored without color space information.
/// Complex data is not supported, other data types are. But note that images other than 8-bit or 16-bit unsigned
/// integer lead to files that are not recognized by most readers.
///
/// The JPEG format can store 2D images. Tensor images are always tagged as sRGB. Most metadata will be lost.
/// Image data is converted to 8-bit unsigned integer, without scaling.
///
/// The NPY format stores raw pixel data for a scalar image. Tensor images cannot be written. All metadata will be lost.
///
/// `compression` determines the compression method used when writing the pixel data. It can be one of the
/// following strings:
///
/// - `"none"`: no compression.
/// - `""`: gzip compression (default). TIFF files with gzip compression are not universally recognized.
/// - `"LZW"`, `"PackBits"`, `"JPEG"`: compression formats supported only by the TIFF format.
///
/// For the JPEG and NPY formats, `compression` is ignored.
///
/// Use the filetype-specific functions directly for more control over how the image is written. See those
/// functions for more information about the file types and how images are written to them.
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
      } else if( StringCompareCaseInsensitive( format, "npy" )) {
         format = "npy";
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
   } else if( format == "npy" ) {
      DIP_STACK_TRACE_THIS( ImageWriteNPY( image, filename ));
   } else {
      DIP_THROW_INVALID_FLAG( format );
   }
}

/// \endgroup

} // namespace dip

#endif // DIP_SIMPLE_FILE_IO_H
