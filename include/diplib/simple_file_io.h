/*
 * (c)2019-2024, Cris Luengo.
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

#include <cstdio>

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
/// - `"png"`: The file is a PNG file, use \ref dip::ImageReadPNG.
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
///
/// If an exception is thrown saying that the file could not be read as the type indicated by its extension, use the
/// filetype-specific function directly, it will give a more specific reason for why the file could not be read.
/// Especially in the case of TIFF files, which allows data to be stored in an infinite number of ways, the reader
/// cannot be expected to read all possible files.
inline FileInformation ImageRead(
      Image& out,
      String const& filename,
      String format = ""
) {
   if( format.empty() ) {
      format = FileGetExtension( filename );
      if( !format.empty() ) {
         std::FILE* f = std::fopen(filename.c_str(), "rb");
         if( f == nullptr ) {
            DIP_THROW_RUNTIME( "File could not be opened" );
         }
         std::fclose( f );
         if( StringCompareCaseInsensitive( format, "ics" ) || StringCompareCaseInsensitive( format, "ids" )) {
            if( !ImageIsICS( filename ) ) {
               DIP_THROW_RUNTIME( "File has an ICS extension but could not be read as an ICS file" );
            }
            format = "ics";
         } else if( StringCompareCaseInsensitive( format, "tif" ) || StringCompareCaseInsensitive( format, "tiff" )) {
            if( !ImageIsTIFF( filename )) {
               DIP_THROW_RUNTIME( "File has a TIFF extension but could not be read as a TIFF file" );
            }
            format = "tiff";
         } else if( StringCompareCaseInsensitive( format, "jpg" ) || StringCompareCaseInsensitive( format, "jpeg" )) {
            if( !ImageIsJPEG( filename )) {
               DIP_THROW_RUNTIME( "File has a JPEG extension but could not be read as a JPEG file" );
            }
            format = "jpeg";
         } else if( StringCompareCaseInsensitive( format, "png" )) {
            if( !ImageIsPNG( filename )) {
               DIP_THROW_RUNTIME( "File has a PNG extension but could not be read as a PNG file" );
            }
            format = "png";
         } else if( StringCompareCaseInsensitive( format, "npy" )) {
            if( !ImageIsNPY( filename )) {
               DIP_THROW_RUNTIME( "File has an NPY extension but could not be read as an NPY file" );
            }
            format = "npy";
         } else {
            format.clear();
         }
      }
      if( format.empty() ) {
         if( ImageIsICS( filename )) {
            format = "ics";
         } else if( ImageIsTIFF( filename )) {
            format = "tiff";
         } else if( ImageIsJPEG( filename )) {
            format = "jpeg";
         } else if( ImageIsPNG( filename )) {
            format = "png";
         } else if( ImageIsNPY( filename )) {
            format = "npy";
         } else {
#ifdef DIP_CONFIG_HAS_DIPJAVAIO
            format = "bioformats";
#else
            DIP_THROW_RUNTIME( "File doesn't exist or it is not of a recognized format" );
#endif
         }
      }
   }
   FileInformation info;
   if( format == "ics" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadICS( out, filename ));
   } else if( format == "tiff" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadTIFF( out, filename ));
   } else if( format == "jpeg" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadJPEG( out, filename ));
   } else if( format == "png" ) {
      DIP_STACK_TRACE_THIS( info = ImageReadPNG( out, filename ));
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
/// - `"png"`: Create a PNG file, use \ref dip::ImageWritePNG.
/// - `"npy"`: Create a NumPy NPY file, use \ref dip::ImageWriteNPY.
/// - `""`: Select the format by looking at the file name extension. If no extension is
///   present, it uses ICS version 2. This is the default.
///
/// The ICS format can store any image, with all its information, such that reading the file using \ref dip::ImageRead
/// or \ref dip::ImageReadICS yields an image that is identical (except the strides might be different).
///
/// The TIFF format can store 2D images, as well as 3D images as a series of 2D slides (but this is not yet implemented).
/// A limited set of color spaces are recognized, other color images are stored without color space information.
/// Complex data is not supported, other data types are. But note that images other than 8-bit or 16-bit unsigned
/// integer lead to files that are not recognized by most readers.
///
/// The JPEG format can store 2D images with 1 or 3 tensor elements. Tensor images are always tagged as sRGB. Most
/// metadata will be lost.
/// Image data is converted to 8-bit unsigned integer, without scaling.
///
/// The PNG format can store 2D images with 1 to 4 tensor elements. Images with 3 or 4 tensor elements are always
/// tagged as sRGB, those with 1 or 2 as grayscale. The 2nd or 4th tensor element is the alpha channel.
/// Image data is converted to 8-bit unsigned integer, without scaling, unless the image is binary or 16-bit unsigned
/// integer.
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
      } else if( StringCompareCaseInsensitive( format, "png" )) {
         format = "png";
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
   } else if( format == "png" ) {
      DIP_STACK_TRACE_THIS( ImageWritePNG( image, filename, compression == "none" ? 0 : 6 ));
   } else if( format == "npy" ) {
      DIP_STACK_TRACE_THIS( ImageWriteNPY( image, filename ));
   } else {
      DIP_THROW_INVALID_FLAG( format );
   }
}

/// \endgroup

} // namespace dip

#endif // DIP_SIMPLE_FILE_IO_H
