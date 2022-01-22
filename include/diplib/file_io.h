/*
 * (c)2017-2021, Cris Luengo.
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
/// \brief Functions for reading and writing images from/to files.
/// See \ref file_io.


namespace dip {


/// \group file_io File I/O
/// \brief Reading images from files and writing them to files.
/// \addtogroup

/// \brief A data structure with information about an image file.
struct FileInformation {
      String                name;              ///< File name
      String                fileType;          ///< File type
      DataType              dataType;          ///< Data type for all samples
      dip::uint             significantBits;   ///< Number of bits used for each sample
      UnsignedArray         sizes;             ///< Size of image in pixels
      dip::uint             tensorElements;    ///< Size of pixel in samples
      String                colorSpace;        ///< Color space
      PixelSize             pixelSize;         ///< Pixel size
      PhysicalQuantityArray origin;            ///< Real-world location of origin pixel
      dip::uint             numberOfImages;    ///< Number of images in the file. Currently only TIFF can have more than 1 here.
      StringArray           history;           ///< Assorted metadata in the file, in the form of strings.
};


/// \brief Read the image in the ICS file `filename` and puts it in `out`.
///
/// The ICS image file format (Image Cytometry Standard) can contain images with any dimensionality
/// and data type also supported by *DIPlib*, and therefore is used as the default image file format.
///
/// The function tries to open `filename` as given first, and if that fails, it appends ".ics" to the
/// name and tries again.
///
/// `roi` can be set to read in a subset of the pixels in the file. If only one array element is given,
/// it is used for all dimensions. An empty array indicates that all pixels should be read. Otherwise,
/// the array should have as many elements as dimensions are represented in the file. Tensor dimensions
/// are not included in the `roi` parameter, but are set through the `channels` parameter.
///
/// If `mode` is `"fast"`, it will attempt to forge `out` with strides matching those in the file, so
/// that reading is much faster. When reading an ROI this is not possible. When `out` has an external
/// interface set it might also be impossible to dictate what the strides will look like. In these cases,
/// the flag is ignored.
///
/// Information about the file and all metadata are returned in the \ref dip::FileInformation output argument.
// TODO: read sensor information also into the history strings
DIP_EXPORT FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      RangeArray const& roi = {},
      Range const& channels = {},
      String const& mode = ""
);
DIP_NODISCARD inline Image ImageReadICS(
      String const& filename,
      RangeArray const& roi = {},
      Range const& channels = {},
      String const& mode = ""
) {
   Image out;
   ImageReadICS( out, filename, roi, channels, mode );
   return out;
}

/// \brief This function is an overload of the previous function that defines the ROI using different
/// parameters.
///
/// The parameters `origin` and `sizes` define a ROI to read in.
/// The ROI is clipped to the image size, so it is safe to specify a ROI that is too large.
/// `spacing` can be used to read in a subset of the pixels of the chosen ROI.
/// These three parameters are handled as in \ref dip::DefineROI:
/// If `origin`, `sizes` or `spacing` have only one value, that value is repeated for each dimension.
/// For empty arrays, `origin` defaults to all zeros (i.e. the top left pixel),
/// `sizes` to *image_size* - `origin` (i.e. up to the bottom right pixel),
/// and `spacing` to all ones (i.e. no subsampling).
///
/// See the first overload for this function to learn about the other parameters.
DIP_EXPORT FileInformation ImageReadICS(
      Image& out,
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {},
      Range const& channels = {},
      String const& mode = ""
);
DIP_NODISCARD inline Image ImageReadICS(
      String const& filename,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {},
      Range const& channels = {},
      String const& mode = ""
) {
   Image out;
   ImageReadICS( out, filename, origin, sizes, spacing, channels, mode );
   return out;
}

/// \brief Reads image information and metadata from the ICS file `filename`, without reading the actual
/// pixel data. See \ref dip::ImageReadICS for more details.
DIP_EXPORT FileInformation ImageReadICSInfo( String const& filename );

/// \brief Returns true if the file `filename` is an ICS file.
DIP_EXPORT bool ImageIsICS( String const& filename );

/// \brief Writes `image` as an ICS file.
///
/// The ICS image file format (Image Cytometry Standard) can contain images with any dimensionality
/// and data type also supported by *DIPlib*, and therefore is used as the default image file format.
/// Any *DIPlib* image can be stored as an ICS file, and read back in to yield the exact same data,
/// with the only limitation that the ICS writer (*libics*) currently throws an exception if the image
/// has more than 10 dimensions. *libics* can be recompiled to handle higher-dimensional images if
/// necessary.
///
/// This function saves the pixel sizes, tensor dimension, color space, and the tensor shape. However,
/// the tensor shape is saved in a custom way and will not be recognized by other software.
/// The ".ics" extension will be added to `filename` if it's not there. Overwrites any other file with the same name.
///
/// `history` is a set of strings that are written as history lines, and will be recovered by the
/// \ref dip::ImageReadICSInfo function.
///
/// Set `significantBits` only if the number of significant bits is different from the full range of the data
/// type of `image` (use 0 otherwise). For example, it can be used to specify that a camera has produced
/// 10-bit output, even though the image is of type \ref dip::DT_UINT16.
///
/// `options` specifies how the ICS file is written, and can contain one or several of these strings:
///
/// - `"v1"` or `"v2"`: ICS v1 writes two files: one with extension '.ics', and one with extension '.ids'.
///   The ICS file contains only the header, the IDS file contains only the pixel data. ICS v2 combines
///   these two pieces into a single '.ics' file. `"v2"` is the default.
/// - '"uncompressed"` or '"gzip"`: Determine whether to compress the pixel data or not. `"gzip"` is the default.
/// - `"fast"`: Writes data in the order in which they are in memory, which is faster.
///
/// Note that the `"fast"` option yields a file with permuted dimensions. The software reading the file must be
/// aware of the possibility of permuted dimensions, and check the "order" tag in the file. If the image has
/// non-contiguous data, then the `"fast"` option is ignored, the image is always saved in the "normal" dimension order
DIP_EXPORT void ImageWriteICS(
      Image const& image,
      String const& filename,
      StringArray const& history = {},
      dip::uint significantBits = 0,
      StringSet const& options = {}
);


/// \brief Reads an image from the TIFF file `filename` and puts it in `out`.
///
/// The function tries to open `filename` as given first, and if that fails, it appends ".tif" and ".tiff" to the
/// name and tries again.
///
/// Multi-page TIFF files contain a series of 2D images, which, if they are the same size, data type and
/// number of samples per pixel, can be regarded as a single 3D image.
/// `imageNumbers` is a range which indicates which images from the multi-page TIFF file to read.
/// If the range indicates a single page, it is read as a 2D image. In this case, `{0}` is the first
/// image. Some Zeiss confocal microscopes write TIFF files (with an ".lsm" extension) in which image
/// planes and thumbnails alternate. A range such as {0,-1,2} reads all image planes skipping the
/// thumbnails.
/// It is currently not possible to read multiple pages from a binary or color-mapped image.
///
/// `roi` can be set to read in a subset of the pixels in the 2D image. If only one array element is given,
/// it is used for both dimensions. An empty array indicates that all pixels should be read. Tensor dimensions
/// are not included in the `roi` parameter, but are set through the `channels` parameter.
/// It is currently not possible to read an ROI from a binary or a color-mapped image.
///
/// Color-mapped (palette) images are read as sRGB images by applying the color map. Set `useColorMap`
/// to `"ignore"` to return the color map indices as pixel values, ignoring the color map.
/// With this option set, it becomes possible to read an ROI of a color-mapped image, or to read a
/// multi-paged color-mapped image.
///
/// The pixels per inch value in the TIFF file will be used to set the pixel size of `out`.
///
/// Color TIFF files produce an image with proper color space name set: either sRGB, CMY, CMYK or Lab. Other
/// multi-channel TIFF files are read as vector images without color space information.
///
/// TIFF is a very flexible file format. We have to limit the types of images that can be read to the
/// more common ones. These are the most obvious limitations:
///
/// - Only 1, 4, 8, 16 and 32 bits per pixel integer grey values are read, as well as 32-bit and 64-bit
///   floating point.
/// - Only 4 and 8 bits per pixel color-mapped images are read.
/// - Class Y images (YCbCr) and Log-compressed images (LogLuv or LogL) are not supported.
/// - Some non-standard compression schemes are not recognized (most notably JPEG2000).
// TODO: How do we return the color map if we choose Option::TIFFColorMap::IGNORE?
//       We should probably create a separate function for this.
DIP_EXPORT FileInformation ImageReadTIFF(
      Image& out,
      String const& filename,
      Range imageNumbers = Range{ 0 },
      RangeArray const& roi = {},
      Range const& channels = {},
      String const& useColorMap = S::APPLY
);
DIP_NODISCARD inline Image ImageReadTIFF(
      String const& filename,
      Range const& imageNumbers = Range{ 0 },
      RangeArray const& roi = {},
      Range const& channels = {},
      String const& useColorMap = S::APPLY
) {
   Image out;
   ImageReadTIFF( out, filename, imageNumbers, roi, channels, useColorMap );
   return out;
}

/// \brief This function is an overload of the previous function that defines the ROI using different
/// parameters.
///
/// The parameters `origin` and `sizes` define a ROI to read in.
/// The ROI is clipped to the image size, so it is safe to specify a ROI that is too large.
/// `spacing` can be used to read in a subset of the pixels of the chosen ROI.
/// These three parameters are handled as in \ref dip::DefineROI:
/// If `origin`, `sizes` or `spacing` have only one value, that value is repeated for each dimension.
/// For empty arrays, `origin` defaults to all zeros (i.e. the top left pixel),
/// `sizes` to *image_size* - `origin` (i.e. up to the bottom right pixel),
/// and `spacing` to all ones (i.e. no subsampling).
///
/// See the first overload for this function to learn about the other parameters.
DIP_EXPORT FileInformation ImageReadTIFF(
      Image& out,
      String const& filename,
      Range const& imageNumbers,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {},
      Range const& channels = {},
      String const& useColorMap = S::APPLY
);
DIP_NODISCARD inline Image ImageReadTIFF(
      String const& filename,
      Range const& imageNumbers,
      UnsignedArray const& origin,
      UnsignedArray const& sizes = {},
      UnsignedArray const& spacing = {},
      Range const& channels = {},
      String const& useColorMap = S::APPLY
) {
   Image out;
   ImageReadTIFF( out, filename, imageNumbers, origin, sizes, spacing, channels, useColorMap );
   return out;
}

/// \brief Reads a set of 2D TIFF images as a single 3D image.
///
/// `filenames` contains the paths to the TIFF files, which are read in the order given, and concatenated along the 3rd
/// dimension. Only the first page of each TIFF file is read.
///
/// Set `useColorMap` to `"ignore"` to return the color map indices as pixel values, ignoring the color map.
/// This option only has effect for TIFF files with a color-mapped (palette) image.
DIP_EXPORT void ImageReadTIFFSeries(
      Image& out,
      StringArray const& filenames,
      String const& useColorMap = S::APPLY
);
DIP_NODISCARD inline Image ImageReadTIFFSeries(
      StringArray const& filenames,
      String const& useColorMap = S::APPLY
) {
   Image out;
   ImageReadTIFFSeries( out, filenames, useColorMap );
   return out;
}

/// \brief Reads image information and metadata from the TIFF file `filename`, without reading the actual
/// pixel data.
DIP_EXPORT FileInformation ImageReadTIFFInfo( String const& filename, dip::uint imageNumber = 0 );

/// \brief Returns true if the file `filename` is a TIFF file.
DIP_EXPORT bool ImageIsTIFF( String const& filename );

/// \brief Writes `image` as a TIFF file.
///
/// The TIFF image file format is very flexible in how data can be written, but is limited to multiple pages
/// of 2D images. A 3D image will be written as a multi-page TIFF file (not yet implemented).
///
/// A tensor image will be written as an image with multiple samples per pixel, but the tensor shape will be lost.
/// If the tensor image has color space information, and it is one of the few color spaces known to the TIFF
/// standard, this information will be stored; images in other color spaces are stored without color space information.
/// No color space transformation will be applied. Recognized color spaces are sRGB, CMY, CMYK and Lab. Linear RGB
/// images are currently also tagged as sRGB, though this might not be ideal. It is recommended to transform any
/// color image to the sRGB color space before saving as TIFF.
///
/// Pixel sizes, if in units of length, will set the pixels per centimeter value in the TIFF file.
///
/// The samples of `image` are written directly to the TIFF file, no matter what their data type is. Complex data
/// are not supported by the TIFF format, but all binary, integer and floating-point types are. However, if the type
/// is not binary, 8-bit or 16-bit unsigned integer, many TIFF readers will not recognize the format. If the image
/// needs to be read by other software, it is recommended to convert the image to \ref dip::DT_UINT8 before saving as
/// TIFF.
///
/// If `filename` does not have an extension, ".tif" will be added. Overwrites any other file with the same name.
///
/// `compression` determines the compression method used when writing the pixel data. It can be one of the
/// following strings:
///
/// - `"none"`: no compression.
/// - `"deflate"` or `""`: uses gzip compression. This is the better compression, but is not universally recognized.
/// - `"LZW"`: uses LZW compression, yielding (typically) only slightly larger files than `"deflate"`. Recognized by
///   most TIFF readers.
/// - `"PackBits"`: uses run-length encoding, the simplest of the compression methods, and required to be recognized
///   by compliant TIFF readers. Even small amounts of noise can cause this method to yield larger files than `"none"`.
/// - `"JPEG"`: uses **lossy** JPEG compression. `jpegLevel` determines the amount of compression applied. `jpegLevel`
///   is an integer between 1 and 100, with increasing numbers yielding larger files and fewer compression artifacts.
DIP_EXPORT void ImageWriteTIFF(
      Image const& image,
      String const& filename,
      String const& compression = "",
      dip::uint jpegLevel = 80
);


/// \brief Reads an image from the JPEG file `filename` and puts it in `out`.
///
/// The function tries to open `filename` as given first, and if that fails, it appends ".jpg" and ".jpeg" to the
/// name and tries again.
///
/// JPEG images are either gray-scale (scalar) or sRGB images, the color space information will be set accordingly.
///
/// The pixels per inch value in the JPEG file will be used to set the pixel size of `out`.
DIP_EXPORT FileInformation ImageReadJPEG(
      Image& out,
      String const& filename
);
DIP_NODISCARD inline Image ImageReadJPEG(
      String const& filename
) {
   Image out;
   ImageReadJPEG( out, filename );
   return out;
}

/// \brief Reads image information and metadata from the JPEG file `filename`, without reading the actual
/// pixel data.
DIP_EXPORT FileInformation ImageReadJPEGInfo( String const& filename );

/// \brief Returns true if the file `filename` is a JPEG file.
DIP_EXPORT bool ImageIsJPEG( String const& filename );

/// \brief Writes `image` as a JPEG file.
///
/// `image` must be 2D, and either scalar or with three tensor elements.
/// If the image has three tensor elements, it will be saved as an sRGB image, even if the color space
/// is not sRGB (no color space conversion is done, the data is simply tagged as sRGB).
/// If the image is not \ref dip::DT_UINT8, it will be converted to it (complex numbers are cast to real values
/// by taking their magnitude, and real numbers are rounded and clamped to the output range), no scaling will
/// be applied.
///
/// If `filename` does not have an extension, ".jpg" will be added. Overwrites any other file with the same name.
///
/// `jpegLevel` determines the amount of compression applied. `jpegLevel` is an integer between 1 and 100, with
/// increasing numbers yielding larger files and fewer compression artifacts.
DIP_EXPORT void ImageWriteJPEG(
      Image const& image,
      String const& filename,
      dip::uint jpegLevel = 80
);


/// \brief Reads a numeric array from the NumPy NPY file `filename` and puts it in `out`.
///
/// The function tries to open `filename` as given first, and if that fails, it appends ".npy" to the
/// name and tries again.
///
/// Only NPY files that contain a numeric array are supported, and only version 1.0 NPY files can be read
/// (note that NumPy only writes later version files for more complex non-numeric arrays).
///
/// Following the handling of PyDIP, the Python bindings, we reverse the indexing of the array, such that
/// the NumPy array's first index is the y axis as the second index is the x axis (this is how 2D arrays are
/// treated everywhere in Python). We generalize this to arbitrary dimensions by reversing the indices.
/// A standard C-order NumPy array this way translates to a DIPlib image with \ref normal_strides.
DIP_EXPORT FileInformation ImageReadNPY(
      Image& out,
      String const& filename
);
DIP_NODISCARD inline Image ImageReadNPY(
      String const& filename
) {
   Image out;
   ImageReadNPY( out, filename );
   return out;
}

/// \brief Reads array information (size and data type) from the NumPy NPY file `filename`, without reading the actual
/// pixel data.
DIP_EXPORT FileInformation ImageReadNPYInfo( String const& filename );

/// \brief Returns true if the file `filename` is a NPY file.
DIP_EXPORT bool ImageIsNPY( String const& filename );

/// \brief Writes `image` as a numeric array to a NumPy NPY file.
///
/// `image` must be scalar, use \ref dip::Image::TensorToSpatial to save a tensor image. Any data type is allowed.
/// Metadata (e.g. pixel sizes) are not stored.
///
/// If `filename` does not have an extension, ".npy" will be added. Overwrites any other file with the same name.
///
/// Following the handling of PyDIP, the Python bindings, we reverse the indexing of the array, such that
/// the NumPy array's first index is the y axis as the second index is the x axis (this is how 2D arrays are
/// treated everywhere in Python). We generalize this to arbitrary dimensions by reversing the indices.
/// A DIPlib image with \ref normal_strides is thus translated to a NumPy array with standard C-order.
DIP_EXPORT void ImageWriteNPY(
      Image const& image,
      String const& filename
);


/// \brief Returns the location of the dot that separates the extension, or `dip::String::npos` if there is no dot.
inline String::size_type FileGetExtensionPosition(
      String const& filename
) {
   auto sep = filename.find_last_of( "/\\:" ); // Path separators.
   auto pos = filename.substr( sep + 1 ).find_last_of( '.' ); // sep + 1 == 0 if no path separator.
   if( pos == String::npos ) {
      return String::npos;
   }
   return sep + 1 + pos;
}

/// \brief Returns true if the file has an extension.
inline bool FileHasExtension(
      String const& filename
) {
   return FileGetExtensionPosition( filename ) != String::npos;
}

/// \brief Gets the extension for the given file name, or an empty string if there's no extension.
inline String FileGetExtension(
      String const& filename
) {
   auto pos = FileGetExtensionPosition( filename );
   if( pos == String::npos ) {
      return {};
   }
   return filename.substr( pos + 1 );
}

/// \brief Returns true if the file has the given extension.
inline bool FileCompareExtension(
      String const& filename,
      String const& extension
) {
   return StringCompareCaseInsensitive( FileGetExtension( filename ), extension );
}

/// \brief Adds the given extension to the file name, replacing any existing extension.
inline String FileAddExtension(
      String const& filename,
      String const& extension
) {
   auto const pos = FileGetExtensionPosition( filename );
   return filename.substr( 0, pos ) + String{ '.' } + extension;
}


/// \endgroup

} // namespace dip

#endif // DIP_FILE_IO_H
