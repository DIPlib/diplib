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

/// \brief A data structure with information about an image file.
struct FileInformation {
      String        name;              ///< File name
      String        fileType;          ///< File type (currently, "ICS" or "TIFF")
      DataType      dataType;          ///< Data type for all samples
      dip::uint     significantBits;   ///< Number of bits used for each sample
      UnsignedArray sizes;             ///< Size of image in pixels
      dip::uint     tensorElements;    ///< Size of pixel in samples
      String        colorSpace;        ///< Color space
      PixelSize     pixelSize;         ///< Pixel size
      dip::uint     numberOfImages;    ///< Number of images in the file. Only TIFF can have more than 1 here.
      StringArray   history;           ///< Assorted metadata in the file, in the form of strings.
};

/// \brief Read the image in the ICS file `filename` and puts it in `image`.
///
/// The ICS image file format (Image Cytometry Standard) can contain images with any dimensionality
/// and data type also supported by DIPlib, and therefore is used as the default image file format.
///
/// `roi` can be set to read in a subset of the pixels in the file. If only one array element is given,
/// it is used for all dimensions. An empty array indicates that all pixels should be read. Otherwise,
/// the array should have as many elements as dimensions are represented in the file. Tensor dimensions
/// are not included in the `roi` parameter.
///
/// Information about the file and all metadata is returned in the `FileInformation` output argument.
DIP_EXPORT FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      RangeArray roi = {},
      Range channels = {}
);
inline Image ImageReadICS(
      String const& filename,
      RangeArray const& roi = {},
      Range channels = {}
) {
   Image out;
   ImageReadICS( out, filename, roi, channels );
   return out;
}

/// \brief This function is an overload of the previous function that defines the ROI using different
/// parameters.
///
/// The parameters `origin` and `sizes` define a ROI to read in.
/// The ROI is clipped to the image size, so it is safe to specify a ROI that is too large.
/// `spacing` can be used to read in a subset of the pixels of the chosen ROI.
/// These three parameters are handled as in `dip::DefineROI`:
/// If `origin`, `sizes` or `spacing` have only one value, that value is repeated for each dimension.
/// For empty arrays, `origin` defaults to all zeros (i.e. the top left pixel),
/// `sizes` to *image_size* - `origin` (i.e. up to the bottom right pixel),
/// and `spacing` to all ones (i.e. no subsampling).
///
/// Information about the file and all metadata is returned in the `FileInformation` output argument.
DIP_EXPORT FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {}
);
inline Image ImageReadICS(
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {}
) {
   Image out;
   ImageReadICS( out, filename, origin, sizes, spacing );
   return out;
}

/// \brief Reads image information and metadata from the ICS file `filename`.
DIP_EXPORT FileInformation ImageReadICSInfo( String const& filename );

/// \brief Returns true if the file `filename` is an ICS file.
DIP_EXPORT bool ImageIsICS( String const& filename );

/// \brief Writes `image` as an ICS file.
///
/// The ICS image file format (Image Cytometry Standard) can contain images with any dimensionality
/// and data type also supported by DIPlib, and therefore is used as the default image file format.
///
/// This function saves the pixel sizes, tensor dimension, and color space, but not the tensor shape.
/// (TODO: how do we write the tensor shape? Use a History line?)
/// Overwrites any other file with the same name.
///
/// `history` is a set of strings that are written as history lines, and will be recovered by the
/// `dip::ImageReadICSInfo` function.
///
/// Set `sigbits` only if the number of significant bits is different from the full range of the data
/// type of `image` (use 0 otherwise). For example, it can be used to specifiy that a camera has produced
/// 10-bit output, even though the image is of type `dip::DT_UINT16`.
///
/// `options` specifies how the ICS file is written, and can contain one or several of these strings:
///  - `"v1"` or `"v2"`: ICS v1 writes two files: one with extension '.ics', and one with extension '.ids'.
///    The ICS file contains only the header, the IDS file contains only the pixel data. ICS v2 combines
///    these two pieces into a single '.ics' file. `"v2"` is the default.
///  = '"uncompressed"` or '"gzip"`: Determine whether to compress the pixel data or not. `"gzip"` is the default.
DIP_EXPORT void ImageWriteICS(
      Image const& c_image,
      String const& filename,
      StringArray const& history = {},
      dip::uint significantBits = 0,
      StringSet const& options = {}
);


// TODO: functions to port:
/*
   dipio_ImageReadTIFF (dipio_tiff.h) (needs to support tiled images at some point!)
   dipio_ImageReadTIFFInfo (dipio_tiff.h)
   dipio_ImageIsTIFF (dipio_tiff.h)
   dipio_ImageWriteTIFF (dipio_tiff.h)

   dipio_ImageReadColourSeries (dipio_image.h) (should be named ImageReadTIFFSeries)
*/

/// \}

} // namespace dip

#endif // DIP_FILE_IO_H
