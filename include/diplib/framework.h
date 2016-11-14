/*
 * DIPlib 3.0
 * This file contains declarations for framework functions and support classes.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_FRAMEWORK_H
#define DIP_FRAMEWORK_H

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/pixel_table.h"

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
constexpr dip::uint MAX_BUFFER_SIZE = 256 * 1024;


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
      ImageConstRefArray const& in
);
/// Determines if images can be singleton-expanded to the same size, and what
/// that size would be. Singleton dimensions (size==1) can be expanded to a
/// larger size by setting their stride to 0. This change can be performed
/// without modifying the data segment. If image dimensions differ such that
/// singleton expansion cannot make them all the same size, an exception is
/// thrown. Use dip::Framework::SingletonExpansion to apply the transform
/// to one image.
UnsignedArray SingletonExpandedSize(
      ImageArray const& in
);

/// Performs singleton expansion. The image is modified so that it has `size`
/// as dimensions. It must be forged and singleton-expandable to `size`,
/// otherwise an exception is thrown. See dip::Image::ExpandSingletonDimension.
/// `size` is the array as returned by dip::Framework::SingletonExpandedSize.
void SingletonExpansion(
      Image& in,
      UnsignedArray const& size
);

/// Determines the best processing dimension, which is the one with the
/// smallest stride, except if that dimension is very small and there's a
/// longer dimension.
dip::uint OptimalProcessingDim(
      Image const& in
);

/// Determines which color space names to assign to each output image, by finding
/// the first input image with the same number of tensor elements as each output
/// image.
StringArray OutputColorSpaces(
      ImageConstRefArray const& c_in,
      UnsignedArray const& nTensorElements
);

/// Creates a vector of void pointers that point at the elements of `in`.
/// Use this function to create the `functionVariables` input for the framework
/// functions.
template< typename T >
std::vector< void* > CastToVoidpVector(
      std::vector< T >& in
) {
   std::vector< void* > out( in.size() );
   for( dip::uint ii = 0; ii < in.size(); ++ii ) {
      out[ ii ] = & ( in[ ii ] );
   }
   return out;
}


//
// Scan Framework:
// Process an image pixel by pixel
//


/// \class dip::Framework::ScanOptions
/// Defines options to the `dip::Framework::Scan` function. Valid values are:
///
/// `ScanOptions` constant      | Meaning
/// --------------------------- | ----------
/// `Scan_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not re-entrant).
/// `Scan_NeedCoordinates`      | The line filter needs the coordinates to the first pixel in the buffer.
/// `Scan_TensorAsSpatialDim`   | Tensor dimensions are treated as a spatial dimension for scanning, ensuring that the line scan filter always gets scalar pixels.
/// `Scan_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
/// `Scan_NoSingletonExpansion` | Inhibits singleton expansion of input images.
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( ScanOptions, 5 );
DIP_DEFINE_OPTION( ScanOptions, Scan_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( ScanOptions, Scan_NeedCoordinates, 1 );
DIP_DEFINE_OPTION( ScanOptions, Scan_TensorAsSpatialDim, 2 );
DIP_DEFINE_OPTION( ScanOptions, Scan_ExpandTensorInBuffer, 3 );
DIP_DEFINE_OPTION( ScanOptions, Scan_NoSingletonExpansion, 4 );

/// Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Scan` callback function. The length of the buffer
/// is given in a separate argument to the callback function. Depending on the
/// arguments given to the framework function, you might assume that `tensorLength`
/// is always 1, and consequently ignore also `tensorStride`.
struct ScanBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

// Prototype line filter for dip::Framework::Scan.
using ScanFilter =  void ( * )(
      std::vector< ScanBuffer > const& inBuffer,
      std::vector< ScanBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      void const* functionParameters,
      void* functionVariables
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
/// call to `dip::Framework::Scan`. It is not necessary nor enforced that the
/// tensors for each image (both input and output) are the same, the calling
/// function is to make sure the tensors satisfy whatever constraints.
///
/// However, if the option `dip::FrameWork::Scan_TensorAsSpatialDim` is given,
/// then the tensor is cast to a spatial dimension, and singleton expansion is
/// applied. Thus, `lineFilter` does not need to check `inTensorLength` or
/// `outTensorLength` (they will be 1), and the output tensor size is guaranteed
/// to match the largest input tensor. `nTensorElements` is ignored.
///
/// If the option `dip::FrameWork::Scan_ExpandTensorInBuffer` is given, then
/// the input buffers passed to `lineFilter` will contain the tensor elements as a
/// standard, column-major matrix. If the image has tensors stored differently,
/// buffers will be used. This option is not used when
/// `dip::FrameWork::Scan_TensorAsSpatialDim` is set, as that forces the tensor
/// to be a single sample. Use this option if you need to do computations with
/// the tensors, but do not want to bother with all the different tensor shapes,
/// which are meant only to save memory. Note, however, that this option does
/// not apply to the output images. When expanding the input tensors in this
/// way, it makes sense to set the output tensor to a full matrix. Don't forget
/// to specify the right size in `nTensorElements`.
///
/// The framework function sets the output pixel size to that of the first input
/// image with a defined pixel size, and it sets the color space to that of the
/// first input image with matching number of tensor elements.
/// The calling function is expected to "correct" these values if necessary.
///
/// The buffers are not guaranteed to be contiguous, please use the `stride`
/// and `tensorStride` values to access samples. All buffers contain `bufferLength`
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
/// `dip::Framework::Scan` will process the image using multiple threads, so
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
/// The `lineFilter` function signature is as follows:
///
///     void ScanFilter (
///         std::vector<ScanBuffer> const& inBuffer,        // Input buffers (1D)
///         std::vector<ScanBuffer>&   outBuffer,           // Output buffers (1D)
///         dip::uint                  bufferLength,        // Number of pixels in each buffer
///         dip::uint                  dimension,           // Dimension along which the line filter is applied
///         UnsignedArray const&       position,            // Coordinates of first pixel in line
///         void const*                functionParameters,  // A pointer to user-defined input data
///         void*                      functionVariables);  // A pointer to user-defined temporary or output data
/// See the definition of the `dip::Framework::ScanBuffer` structure.
void Scan(
      ImageConstRefArray const& in,             ///< Input images
      ImageRefArray& out,                       ///< Output images
      DataTypeArray const& inBufferTypes,       ///< Data types for input buffers
      DataTypeArray const& outBufferTypes,      ///< Data types for output buffers
      DataTypeArray const& outImageTypes,       ///< Data types for output images
      UnsignedArray const& nTensorElements,     ///< Number of tensor elements in output images
      ScanFilter lineFilter,                    ///< Function to call for each image line
      void const* functionParameters,           ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      ScanOptions opts                          ///< Options to control how `lineFilter` is called
);

/// Calls dip::Framework::Scan with one output image.
inline void ScanSingleOutput(
      Image& out,                      ///< Output image
      DataType outImageType,           ///< Data type for output image, buffer will have this type also
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      ScanFilter lineFilter,           ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{};
   ImageRefArray outar{ out };
   DataTypeArray inBufT{};
   DataTypeArray outBufT{ outImageType };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ nTensorElements };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, functionParameters, functionVariables, opts );
}

/// Calls dip::Framework::Scan with one input image.
inline void ScanSingleInput(
      Image& in,                       ///< Input image
      DataType bufferType,             ///< Data type for input buffer
      ScanFilter lineFilter,           ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{ in };
   ImageRefArray outar{};
   DataTypeArray inBufT{ bufferType };
   DataTypeArray outBufT{};
   DataTypeArray outImT{};
   UnsignedArray nElem{};
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, functionParameters, functionVariables, opts );
}

/// Calls dip::Framework::Scan with one input image and one output image.
inline void ScanMonadic(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType bufferTypes,            ///< Data type for all input and output buffers
      DataType outImageType,           ///< Data type for output image
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      ScanFilter lineFilter,           ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{ in };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ bufferTypes };
   DataTypeArray outBufT{ bufferTypes };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ nTensorElements };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, functionParameters, functionVariables, opts );
}

/// Calls `dip::Framework::Scan` with two input images and one output image. It handles
/// some of the work for dyadic (binary) operators related to matching up tensor
/// dimensions in the input image.
///
/// Input tensors are expected to match, but a scalar is expanded to the size of the
/// other tensor. The output tensor will be of the same size as the input tensors,
/// its shape will match the input shape if one image is a scalar, or if both images
/// have matching tensor shapes. Otherwise the output tensor will be a column-major
/// matrix (or vector or scalar, as appropriate).
///
/// This function adds `dip::Framework::Scan_TensorAsSpatialDim` or
/// `dip::Framework::Scan_ExpandTensorInBuffer` to `opts`, so don't set these
/// values. This means that the tensors passed to `lineFilter` is either all scalars
/// (the tensor can be converted to a spatial dimension) or full, column-major
/// tensors of equal size. Do not specify `dip::Framework::Scan_NoSingletonExpansion`
/// in `opts`.
inline void ScanDyadic(
      Image const& in1,                ///< Input image 1
      Image const& in2,                ///< Input image 2
      Image& out,                      ///< Output image
      DataType inType,                 ///< Data type for all input buffers
      DataType outType,                ///< Data type for output image and output buffer
      ScanFilter lineFilter,           ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   Tensor outTensor;
   if( in1.IsScalar() ) {
      outTensor = in2.Tensor();
      opts += Framework::Scan_TensorAsSpatialDim;
   } else if( in2.IsScalar() ) {
      outTensor = in1.Tensor();
      opts += Framework::Scan_TensorAsSpatialDim;
   } else if( in1.Tensor() == in2.Tensor() ) {
      outTensor = in1.Tensor();
      opts += Framework::Scan_TensorAsSpatialDim;
   } else if( in1.TensorSizes() == in2.TensorSizes() ) {
      outTensor = Tensor( in1.TensorRows(), in1.TensorColumns() );
      opts += Framework::Scan_ExpandTensorInBuffer;
   } else {
      dip_Throw( E::NTENSORELEM_DONT_MATCH );
   }
   ImageConstRefArray inar{ in1, in2 };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ inType, inType };
   DataTypeArray outBufT{ outType };
   DataTypeArray outImT{ outType };
   UnsignedArray nElem{ outTensor.Elements() };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, functionParameters, functionVariables, opts );
   out.ReshapeTensor( outTensor );
}

// TODO: ScanDyadic, ScanMonadic and ScanSingleInput are used a lot, I don't know how much overhead comes from converting these problems to the generic problem of N input images and M output images.


//
// Separable Framework:
// Process an image line by line, once for each dimension
//


/// \class dip::Framework::SeparableOptions
/// Defines options to the `dip::Framework::Separable` function. Valid values are:
///
/// `SeparableOptions` constant      | Meaning
/// -------------------------------- | ----------
/// `Separable_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not re-entrant).
/// `Separable_AsScalarImage`        | The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
/// `Separable_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
/// `Separable_UseOutBorder`         | The output line buffer also has space allocated for a border.
/// `Separable_DontResizeOutput`     | The output image has the right size; it can differ from the input size
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( SeparableOptions, 5 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_AsScalarImage, 1 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_ExpandTensorInBuffer, 2 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_UseOutBorder, 3 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_DontResizeOutput, 4 );

/// Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Separable` callback function. The length of the buffer
/// is given in a separate argument to the callback function. Depending on the
/// arguments given to the framework function, you might assume that `tensorLength`
/// is always 1, and consequently ignore also `tensorStride`.
struct SeparableBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::uint length;       ///< Length of the buffer, not counting the expanded boundary
   dip::uint border;       ///< Length of the expanded boundary at each side of the buffer.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

// Prototype line filter for dip::Framework::Separable.
using SeparableFilter = void ( * )(
      SeparableBuffer const& inBuffer,
      SeparableBuffer& outBuffer,
      dip::uint dimension,
      UnsignedArray const& position,
      void const* functionParameters,
      void* functionVariables
);

/// Framework for separable filtering of images.
///
/// The function `lineFilter` is called for each image line, and along each
/// dimension, with input and output buffers either pointing directly to the
/// input and output images, or pointing to temporary buffers that are handled
/// by the framework and present the line's pixel data with a different data type,
/// with expanded borders, etc. The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. The output buffer is not initialized,
/// the `lineFilter` function is responsible for setting all its values.
///
/// The `process` array specifies along which dimensions the filtering is applied.
/// If it is an empty array, all dimensions will be processed.
///
/// Output images (unless protected) will be resized to match the input,
/// and their type will be set to that specified by `outImage`.
/// Protected output images must have the correct size and type,
/// otherwise an exception will be thrown.
/// The separable filter function always has one input and one output image.
///
/// If the option `dip::FrameWork::Separable_DontResizeOutput` is given, then
/// the sizes of the output image will be kept (but it could still be reforged
/// to change the data type). In this case, the length of the input and output
/// buffers can differ, causing the intermediate result image to change size one
/// dimension at the time, as each dimension is processed. For example, if the
/// input image is of size 256x256, and the output is 1x1, then in a first step
/// 256 lines are processed, each with 256 pixels as input and a single pixel as
/// output. In a second step, a single line of 256 pixels is processed yielding
/// the final single-pixel result. In the same case, but with an output of 64x512,
/// 256 lines are processed, each with 256 pixels as input and 64 pixels as output.
/// In the second step, 64 lines are processed, each iwth 256 pixels as input and
/// 512 pixels as output. This option is useful for functions that scale and do other
/// geometric transformations, as well as functions that compute projections.
///
/// Tensors are passed to the `lineFilter` function as vectors, if the shape is
/// important, pass this information through `functionParameters`. The output image
/// will have the same tensor shape as the input except if the option
/// `dip::FrameWork::Separable_ExpandTensorInBuffer` is given. In this case,
/// the input buffers passed to `lineFilter` will contain the tensor elements as a
/// standard, column-major matrix, and the output image will be a full matrix of
/// that size. If the input image has tensors stored differently, buffers will be
/// used when processing the first dimension; for subsequent dimensions, the
/// intermetidate result will already contain the full matrix. Use this option if
/// you need to do computations with the tensors, but do not want to bother with
/// all the different tensor shapes, which are meant only to save memory.
///
/// However, if the option `dip::FrameWork::Separable_AsScalarImage` is given,
/// then the line filter is called for each tensor element, effectively causing
/// the filter to process a sequence of scalar images, one for each tensor element.
/// This is accomplished by converting the tensor into a spatial dimension for
/// both the input and output image, and setting the `process` array for the new
/// dimension to false. For example, given an input image `in` with 3 tensor
/// elements, `filter(in,out)` will result in an output image `out` with 3 tensor
/// elements, and computed as if the filter function were called 3 times:
/// `filter(in[0],out[0])`, `filter(in[1],out[1])`, and `filter(in[2],out[2])`.
///
/// The framework function sets the output pixel size to that of the input
/// image, and it sets the color space to that of the
/// input image if the two images have matching number of tensor elements.
/// The calling function is expected to "correct" these values if necessary.
///
/// The buffers are not guaranteed to be contiguous, please use the `stride`
/// and `tensorStride` values to access samples. The input buffer contains `bufferLength + 2 * border`
/// pixels. The pixel pointed to by the `buffer` pointer is the first pixel on
/// that line in the input image. The `lineFilter` function can read up to `border`
/// pixels before that pixel, and up to `border` pixels after the last pixel on the
/// line. These pixels are filled by the framework using the `boundaryCondition`
/// value for the given dimension. The `boundaryCondition` array can be empty, in which
/// case the default boundary condition value is used. If the option
/// `dip::FrameWork::Separable_UseOutBorder` is given, then the output buffer also has `border`
/// extra samples at each end. These extra samples are meant to help in the
/// computation for some filters, and are not copied back to the output image.
/// `position` gives the coordinates for the first pixel in the buffers,
/// subsequent pixels occur along dimension `dimension`. `position[dimension]`
/// is always zero.
///
/// If `in` and `out` share their data segments, then the input image might be
/// overwritten with the processing result. However, the input and output buffers
/// will never share memory. That is, the line filter can freely write in the
/// output buffer without invalidating the input buffer, even when the filter is
/// being applied in-place.
///
/// `dip::Framework::Separable` will process the image using multiple threads, so
/// `lineFilter` is expected to be re-entrant. If this is not the case, specify
/// `dip::FrameWork::Separable_NoMultiThreading` as an option.
///
/// `functionParameters` is a `void*` pointing to user data. This pointer will
/// be passed unaltered to the `lineFilter`. `functionVariables` is an array
/// of `void*`, which should have as many (identical) elements as threads can
/// be generated by the framework. `lineFilter` can e.g. write intermediate data into
/// the memory referenced by these pointers. The length of the `functionVariables`
/// array determines the maximum number of threads created, but if the array is of
/// zero length, then it is presumed that this feature is not used, no additional
/// limits are imposed on the number of threads, and a `nullptr` pointer will
/// be passed to the `lineFilter` function.
///
/// The `lineFilter` function signature is as follows:
///
///     void SeparableFilter (
///         SeparableBuffer const& inBuffer,            // Input buffers (1D)
///         SeparableBuffer&       outBuffer,           // Output buffers (1D)
///         dip::uint              bufferLength,        // Number of pixels in each buffer
///         dip::uint              dimension,           // Dimension along which the line filter is applied
///         UnsignedArray const&   position,            // Coordinates of first pixel in line
///         void const*            functionParameters,  // A pointer to user-defined input data
///         void*                  functionVariables);  // A pointer to user-defined temporary or output data
/// See the definition of the `dip::Framework::SeparableBuffer` structure.
void Separable(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType bufferType,             ///< Data type for input and output buffer
      DataType outImageType,           ///< Data type for output image
      BooleanArray process,            ///< Determines along which dimensions to apply the filter
      UnsignedArray border,            ///< Number of pixels to add to the beginning and end of each line, for each dimension
      BoundaryConditionArray boundaryConditions, ///< Filling method for the border
      SeparableFilter lineFilter,      ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      SeparableOptions opts            ///< Options to control how `lineFilter` is called
);
// Examples of usage:
// - The Gaussian filter can make an array of arrays, where each array contains the Gaussian weights
//   to be used along one dimension. The `lineFilter` function takes array number `dimension` for weights.
// - The dilation with a square support has two std::vector<uint8> for each element of the `functionVariables`
//   array. The `lineFilter` checks to see if they have the right number of elements, and resizes them if not.
//   These are then the temporary buffers it needs to do the computation efficiently. The `std::vector`s will be
//   resized for processing the first line along each dimension, and freed automatically at the end of the caller's
//   scope.


//
// Full Framework:
// Process an image line by line, with access to a full neighborhood given by a PixelTable
//


/// \class dip::Framework::FullOptions
/// Defines options to the `dip::Framework::Full` function. Valid values are:
///
/// `FullOptions` constant      | Meaning
/// --------------------------- | ----------
/// `Full_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not re-entrant).
/// `Full_AsScalarImage`        | The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
/// `Full_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( FullOptions, 3 );
DIP_DEFINE_OPTION( FullOptions, Full_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( FullOptions, Full_AsScalarImage, 1 );
DIP_DEFINE_OPTION( FullOptions, Full_ExpandTensorInBuffer, 2 );

/// Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Full` callback function. Depending on the
/// arguments given to the framework function, you might assume that `tensorLength`
/// is always 1, and consequently ignore also `tensorStride`.
struct FullBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

// Prototype line filter for dip::Framework::Full.
using FullFilter = void ( * )(
      std::vector< FullBuffer > const& inBuffer,
      std::vector< FullBuffer >& outBuffer,
      dip::uint bufferLength,
      dip::uint dimension,
      UnsignedArray const& position,
      PixelTable const& pixelTable,
      void const* functionParameters,
      void* functionVariables
);

/// Framework for filtering of images with an arbitrary shape neighborhood.
///
/// The function `lineFilter` is called for each image line,
/// with input and output buffers either pointing directly to the
/// input and output images, or pointing to temporary buffers that are handled
/// by the framework and present the line's pixel data with a different data type,
/// with expanded borders, etc.  The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. The output buffer is not initialized,
/// the `lineFilter` function is responsible for setting all its values.
///
/// The `lineFilter` function can access the pixels on the given line for all
/// input and output images, as well as all pixels within the neighborhood for
/// all input images.
///
/// Output images (unless protected) will be resized to match the input,
/// and their type will be set to that specified by `outImage`.
/// Protected output images must have the correct size and type,
/// otherwise an exception will be thrown.
///
/// Tensors are passed to the `lineFilter` function as vectors, if the shape is
/// important, pass this information through `functionParameters`. `nTensorElements`
/// gives the number of tensor elements for the output image. These are created
/// as standard vectors, unless the input image has the same number of tensor elements,
/// in which case that tensor shape is copied. The calling function can reshape the tensors after the
/// call to `dip::Framework::Full`. It is not necessary nor enforced that the
/// tensors for each image (both input and output) are the same, the calling
/// function is to make sure the tensors satisfy whatever constraints.
///
/// However, if the option `dip::FrameWork::Full_AsScalarImage` is given,
/// then the line filter is called for each tensor element, effectively causing
/// the filter to process a sequence of scalar images, one for each tensor element.
/// `nTensorElements` is ignored, and set to the number of tensor
/// elements of the input. For example, given an input image `in` with 3 tensor
/// elements, `filter(in,out)` will result in an output image `out` with 3 tensor
/// elements, and computed as if the filter function were called 3 times:
/// `filter(in[0],out[0])`, `filter(in[1],out[1])`, and `filter(in[2],out[2])`.
///
/// If the option `dip::FrameWork::Full_ExpandTensorInBuffer` is given, then
/// the input buffers passed to `lineFilter` will contain the tensor elements as a
/// standard, column-major matrix. If the image has tensors stored differently,
/// buffers will be used. This option is not used when
/// `dip::FrameWork::Full_AsScalarImage` is set, as that forces the tensor
/// to be a single sample. Use this option if you need to do computations with
/// the tensors, but do not want to bother with all the different tensor shapes,
/// which are meant only to save memory. Note, however, that this option does
/// not apply to the output images. When expanding the input tensors in this
/// way, it makes sense to set the output tensor to a full matrix. Don't forget
/// to specify the right size in `nTensorElements`.
///
/// The framework function sets the output pixel size to that of the input
/// image, and it sets the color space to that of the
/// input image if the two images have matching number of tensor elements.
/// The calling function is expected to "correct" these values if necessary.
///
/// The buffers are not guaranteed to be contiguous, please use the `stride`
/// and `tensorStride` values to access samples. The pixel pointed to by the
/// `buffer` pointer is the first pixel on that line in the input image.
/// The `lineFilter` function can read any pixel within the neighborhood of
/// all the pixels on the line. These pixels are filled by the framework using
/// the `boundaryCondition` values. The `boundaryCondition` vector can be empty,
/// in which case the default boundary condition value is used.
/// `position` gives the coordinates for the first pixel in the buffers,
/// subsequent pixels occur along dimension `dimension`. `position[dimension]`
/// is always zero.
///
/// The input and output buffers will never share memory. That is, the line
/// filter can freely write in the output buffers without invalidating input
/// buffers, even when the filter is being applied in-place.
///
/// dip::Framework::Full() will process the image using multiple threads, so
/// `lineFilter` is expected to be re-entrant. If this is not the case, specify
/// `dip::FrameWork::Full_NoMultiThreading` as an option.
///
/// `functionParameters` is a `void*` pointing to user data. This pointer will
/// be passed unaltered to the `lineFilter`. `functionVariables` is an array
/// of `void*`, which should have as many (identical) elements as threads can
/// be generated by the framework. `lineFilter` can e.g. write intermediate data into
/// the memory referenced by these pointers. The length of the `functionVariables`
/// array determines the maximum number of threads created, but if the array is of
/// zero length, then it is presumed that this feature is not used, no additional
/// limits are imposed on the number of threads, and a `nullptr` pointer will
/// be passed to the `lineFilter` function.
///
/// The `lineFilter` function signature is as follows:
///
///     void FullFilter (
///         std::vector<FullBuffer> const& inBuffer,     // Input buffers (1D)
///         std::vector<FullBuffer>& outBuffer,          // Output buffers (1D)
///         dip::uint                bufferLength,       // Number of pixels in each buffer
///         dip::uint                dimension,          // Dimension along which the line filter is applied
///         UnsignedArray const&     position,           // Coordinates of first pixel in line
///         PixelTable const&        pixelTable,         // The pixel table object describing the neighborhood
///         void const*              functionParameters, // A pointer to user-defined input data
///         void*                    functionVariables); // A pointer to user-defined temporary or output data
/// See the definition of the `dip::Framework::FullBuffer` structure.
void Full(
      ImageConstRefArray const& in,          ///< Input images
      ImageRefArray& out,                    ///< Output images
      DataTypeArray const& inBufferTypes,    ///< Data types for input buffers
      DataTypeArray const& outBufferTypes,   ///< Data types for output buffers
      DataTypeArray const& outImageTypes,    ///< Data types for output images
      UnsignedArray const& nTensorElements,  ///< Number of tensor elements in output images
      BoundaryConditionArray boundaryConditions, ///< Filling method for the border
      PixelTable const& pixelTable,          ///< Object describing the neighborhood
      FullFilter lineFilter,                 ///< Function to call for each image line
      void const* functionParameters,        ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      FullOptions opts                       ///< Options to control how `lineFilter` is called
);

/// Calls dip::Framework::Full with one input image and one output image.
inline void FullMonadic(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType bufferTypes,            ///< Data type for all input and output buffers
      DataType outImageType,           ///< Data type for output image
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      BoundaryConditionArray boundaryConditions, ///< Filling method for the border
      PixelTable const& pixelTable,    ///< Object describing the neighborhood
      FullFilter lineFilter,           ///< Function to call for each image line
      void const* functionParameters,  ///< Parameters to pass to `lineFilter`
      std::vector< void* > const& functionVariables, ///< Variables to pass to `lineFilter`
      FullOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{ in };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ bufferTypes };
   DataTypeArray outBufT{ bufferTypes };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ nTensorElements };
   Full( inar, outar, inBufT, outBufT, outImT, nElem, boundaryConditions, pixelTable, lineFilter, functionParameters, functionVariables, opts );
}


} // namespace Framework
} // namespace dip

#endif // DIP_FRAMEWORK_H
