/*
 * DIPlib 3.0
 * This file contains definitions for framework functions and support classes.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_FRAMEWORK_H
#define DIP_FRAMEWORK_H

#include "diplib.h"


/// \file
/// Declares the dip::Framework namespace.


namespace dip {

/// Frameworks are the basis of most pixel-based processing in DIPlib.
///
/// The various frameworks implement iterating over image pixels, giving
/// access to a single pixel, a whole image line, or a pixel's neighborhood.
/// The programmer needs to define a function that loops over one dimension.
/// The framework will call this function repeatedly to process all the image's
/// lines, thereby freeing the programmer from implementing loops over multiple
/// dimensions. This process allows most of DIPlib's filters to be dimensionality
/// independent, with little effort from the programmer.
namespace Framework {

// Maximum number of pixels in a buffer for the framework functions
constexpr dip::uint MAX_BUFFER_SIZE = 256*1024;

//
// Support functions
//

/// Determines if images can be singleton-expanded to the same size, and what
/// that size would be. Singleton dimensions (size==1) can be expanded to a
/// larger size by setting their stride to 0. This change can be performed
/// without modifying the data segment. If image dimensions differ such that
/// singleton expansion cannot make them all the same size, an exception is
/// thrown. Use dip::Framework::SingletonExpansion to apply the transform
/// to one image.
UnsignedArray SingletonExpandedSize(
      const ImageConstRefArray& in
);
/// Determines if images can be singleton-expanded to the same size, and what
/// that size would be. Singleton dimensions (size==1) can be expanded to a
/// larger size by setting their stride to 0. This change can be performed
/// without modifying the data segment. If image dimensions differ such that
/// singleton expansion cannot make them all the same size, an exception is
/// thrown. Use dip::Framework::SingletonExpansion to apply the transform
/// to one image.
UnsignedArray SingletonExpandedSize(
      const ImageArray& in
);

/// Performs singleton expansion. The image is modified so that it has `size`
/// as dimensions. It must be forged and singleton-expandable to `size`,
/// otherwise an exception is thrown. See dip::Image::ExpandSingletonDimension.
/// `size` is the array as returned by dip::Framework::SingletonExpandedSize.
void SingletonExpansion(
      Image& in,
      const UnsignedArray& size
);

/// Determines the best processing dimension, which is the one with the
/// smallest stride, except if that dimension is very small and there's a
/// longer dimension.
dip::uint OptimalProcessingDim(
      const Image& in
);


//
// Scan Framework:
// Process an image pixel by pixel
//

DIP_DECLARE_OPTIONS(ScanOptions, 4);
DIP_DEFINE_OPTION(ScanOptions, Scan_NoMultiThreading, 0);
DIP_DEFINE_OPTION(ScanOptions, Scan_NeedCoordinates, 1);
DIP_DEFINE_OPTION(ScanOptions, Scan_TensorAsSpatialDim, 2);
DIP_DEFINE_OPTION(ScanOptions, Scan_NoSingletonExpansion, 3);

/// Structure that holds information about input or output pixel buffers
/// for the dip::Framework::Scan callback function. The length of the buffer
/// is given in a separate argument to the callback function. Depending on the
/// arguments given to the framework function, you might assume that `tensorLength`
/// is always 1, and consequently ignore also `tensorStride`.
struct ScanBuffer {
   void*     buffer;       ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

//
typedef void (*ScanFilter) (
      const std::vector<ScanBuffer>&   inBuffer,
      std::vector<ScanBuffer>&         outBuffer,
      dip::uint            bufferLength,
      dip::uint            dimension,
      UnsignedArray        position,
      const void*          functionParameters,
      void*                functionVariables
);

/// Framework for pixel-based processing of images.
///
/// The function `lineFilter` is called for each image line, with input and
/// output buffers either pointing directly to the input and output images,
/// or pointing to temporary buffers that are handled by the framework and
/// serve to prevent the `lineFilter` function to have to deal with too many
/// different data types. The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. Output buffers are not initialized,
/// the `lineFilter` function is responsible for setting all its values.
///
/// Output images (unless protected) will be resized to match the
/// (singleton-expanded) input, and their type will be set to that specified
/// by `outImage`. Protected output images must have the correct size and type,
/// otherwise an exception will be thrown.
/// The scan function can be called without input images. In this case, at least
/// one output image must be given. The dimensions of the first output image
/// will be used to direct the scanning, and the remaining output images (if
/// any) will be adjusted to the same size.
/// It is also possible to give no output images, as would be the case for a
/// reduction operation such as computing the average pixel value. However, it
/// makes no sense to call the scan function without input nor output images.
///
/// Tensors are passed to the `lineFilter` function as vectors, if the shape is
/// important, pass this information through `functionParameters`. `nTensorElements`
/// gives the number of tensor elements for each output image. These are created
/// as standard vectors. The calling function can reshape the tensors after the
/// call to dip::Framework::Scan. It is not necessary nor enforced that the
/// tensors for each image (both input and output) are the same, the calling
/// function is to make sure the tensors satisfy whatever constraints. However,
/// if the option `dip::FrameWork::Scan_TensorAsSpatialDim` is given, then the
/// tensor is cast to a spatial dimension, and singleton expansion is applied.
/// Thus, `lineFilter` does not need to check `inTensorLength` or
/// `outTensorLength` (they will be 1), and the output tensor size is guaranteed
/// to match the largest input tensor. `nTensorElements` is ignored.
///
/// The framework function also does not set the physical dimensions or color
/// space information, the caller is expected to do so when the framework
/// function is finished.
///
/// The buffers are not guaranteed to be contiguous, please use the `inStride`
/// and `outStride` values to access pixels. All buffers contain `bufferLength`
/// pixels. `position` gives the coordinates for the first pixel in the buffers,
/// subsequent pixels occur along dimension `dimension`. `position[dimension]`
/// is not necessarily zero. However, when `dip::FrameWork::Scan_NeedCoordinates`
/// is not given, `dimension` and `position` are meaningless. The framework is
/// allowed to treat all pixels in the image as a single image line in this case.
///
/// If `in` and `out` share an image, then it is possible that the corresponding
/// input and output buffers point to the same memory. The input image will be
/// overwritten with the processing result. That is, all processing can be
/// performed in place. The scan framework is intended for pixel-wise processing,
/// not neighborhood-based processing, so there is never a reason not to work
/// in place.
///
/// dip::Framework::Scan() will process the image using multiple threads, so
/// `lineFilter` is expected to be re-entrant. If this is not the case, specify
/// `dip::FrameWork::Scan_NoMultiThreading` as an option.
///
/// `functionParameters` is a `void*` pointing to user data. This pointer will
/// be passed unaltered to the `lineFilter`. `functionVariables` is an array
/// of `void*`, which should have as many (identical) elements as threads can
/// be generated by the framework. `lineFilter` can write output data into
/// the memory referenced by these pointers. The calling function can use these
/// to do reduction. However, the calling function should not presume that all
/// elements were used, as the framework is free to choose a suitable number
/// of threads. The length of the `functionVariables` array determines the
/// maximum number of threads created, but if the array is of zero length, then
/// it is presumed that this feature is not used, no additional limits are
/// imposed on the number of threads, and a `nullptr` pointer will be passed to
/// the `lineFilter` function.
///
/// The `dip::FrameWork::ScanOptions` parameter can contain the following values
/// (can be added together):
/// * `dip::FrameWork::Scan_NoMultiThreading`: Do not call the line scan filter
///     simultaneouly from multiple threads (it is not re-entrant).
/// * `dip::FrameWork::Scan_NeedCoordinates`: The line scan filter needs the
///     coordinates to the first pixel in the buffer.
/// * `dip::FrameWork::Scan_TensorAsSpatialDim`: Tensor dimensions are treated
///     as a spatial dimension for scanning, ensuring that the line scan filter
///     always gets scalar pixels.
/// * `dip::Framework::Scan_NoSingletonExpansion`: Inhibits singleton expansion
///     of input images.
///
/// The `lineFilter` function signature is as follows:
///
///     void ScanFilter (
///         const std::vector<const ScanBuffer>& inBuffer,  // Input buffers (1D)
///         std::vector<ScanBuffer>&   outBuffer,           // Output buffers (1D)
///         dip::uint                  bufferLength,        // Number of pixels in each buffer
///         dip::uint                  dimension,           // Dimension along which the line filter is applied
///         UnsignedArray              position,            // Coordinates of first pixel in line
///         const void*                functionParameters,  // A pointer to user-defined input data
///         void*                      functionVariables);  // A pointer to user-defined output data
/// See the definition of the dip::Framework::ScanBuffer structure.
void Scan(
      const ImageConstRefArray& in,             ///< Input images
      ImageRefArray&       out,                 ///< Output images
      const DataTypeArray& inBufferTypes,       ///< Data types for input buffers
      const DataTypeArray& outBufferTypes,      ///< Data types for output buffers
      const DataTypeArray& outImageTypes,       ///< Data types for output images
      const UnsignedArray& nTensorElements,     ///< Number of tensor elements in output images
      ScanFilter           lineFilter,          ///< Function to call for each image line
      const void*          functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector<void*>&  functionVariables,   ///< Variables to pass to `lineFilter`
      ScanOptions          opts                 ///< Options to control how `lineFilter` is called
);

} // namespace Framework
} // namespace dip

#endif // DIP_FRAMEWORK_H
