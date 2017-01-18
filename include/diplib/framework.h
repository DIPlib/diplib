/*
 * DIPlib 3.0
 * This file contains declarations for framework functions and support classes.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_FRAMEWORK_H
#define DIP_FRAMEWORK_H

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/pixel_table.h"


/// \file
/// \brief Declares the `dip::Framework` namespace.
/// \see frameworks, infrastructure


namespace dip {


/// \brief Frameworks are the basis of most pixel-based processing in *DIPlib*.
///
/// The various frameworks implement iterating over image pixels, giving
/// access to a single pixel, a whole image line, or a pixel's neighborhood.
/// The programmer needs to define a function that loops over one dimension.
/// The framework will call this function repeatedly to process all the image's
/// lines, thereby freeing the programmer from implementing loops over multiple
/// dimensions. This process allows most of *DIPlib*'s filters to be dimensionality
/// independent, with little effort from the programmer.
namespace Framework {


/// \defgroup frameworks Frameworks
/// \ingroup infrastructure
/// \brief Functions that form the basis of most pixel-based processing in *DIPlib*.
///
/// \{


// Maximum number of pixels in a buffer for the framework functions
constexpr dip::uint MAX_BUFFER_SIZE = 256 * 1024;


//
// Support functions
//


/// \brief Determines if images can be singleton-expanded to the same size, and what
/// that size would be.
///
/// Singleton dimensions (size==1) can be expanded to a
/// larger size by setting their stride to 0. This change can be performed
/// without modifying the data segment. If image dimensions differ such that
/// singleton expansion cannot make them all the same size, an exception is
/// thrown. Use `dip::Image::ExpandSingletonDimensions` to apply the transform
/// to one image.
UnsignedArray SingletonExpandedSize(
      ImageConstRefArray const& in
);

/// \brief Determines if images can be singleton-expanded to the same size, and what
/// that size would be.
///
/// Singleton dimensions (size==1) can be expanded to a
/// larger size by setting their stride to 0. This change can be performed
/// without modifying the data segment. If image dimensions differ such that
/// singleton expansion cannot make them all the same size, an exception is
/// thrown. Use `dip::Image::ExpandSingletonDimensions` to apply the transform
/// to one image.
UnsignedArray SingletonExpandedSize(
      ImageArray const& in
);

/// \brief Determines the best processing dimension, which is the one with the
/// smallest stride, except if that dimension is very small and there's a
/// longer dimension.
dip::uint OptimalProcessingDim(
      Image const& in
);

/// \brief Determines which color space names to assign to each output image, by finding
/// the first input image with the same number of tensor elements as each output
/// image.
StringArray OutputColorSpaces(
      ImageConstRefArray const& c_in,
      UnsignedArray const& nTensorElements
);


//
// Scan Framework:
// Process an image pixel by pixel
//


/// \class dip::Framework::ScanOptions
/// \brief Defines options to the `dip::Framework::Scan` function.
///
/// Valid values are:
///
/// `ScanOptions` constant      | Meaning
/// --------------------------- | ----------
/// `Scan_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not thread safe).
/// `Scan_NeedCoordinates`      | The line filter needs the coordinates to the first pixel in the buffer.
/// `Scan_TensorAsSpatialDim`   | Tensor dimensions are treated as a spatial dimension for scanning, ensuring that the line scan filter always gets scalar pixels.
/// `Scan_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
/// `Scan_NoSingletonExpansion` | Inhibits singleton expansion of input images.
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( ScanOptions );
DIP_DEFINE_OPTION( ScanOptions, Scan_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( ScanOptions, Scan_NeedCoordinates, 1 );
DIP_DEFINE_OPTION( ScanOptions, Scan_TensorAsSpatialDim, 2 );
DIP_DEFINE_OPTION( ScanOptions, Scan_ExpandTensorInBuffer, 3 );
DIP_DEFINE_OPTION( ScanOptions, Scan_NoSingletonExpansion, 4 );

/// \brief Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Scan` callback function object.
///
/// The length of the buffer
/// is given in a separate argument to the line filter. Depending on the
/// arguments given to the framework function, you might assume that `tensorLength`
/// is always 1, and consequently ignore also `tensorStride`.
struct ScanBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

/// \brief Parameters to the line filter for `dip::Framework::Scan`.
///
/// We have put all the parameters to the line filter `dip::Framework::ScanLineFilter::Filter` into
/// a single struct to simplify writing those functions.
struct ScanLineFilterParameters {
   std::vector< ScanBuffer > const& inBuffer;   ///< Input buffers (1D)
   std::vector< ScanBuffer >& outBuffer;        ///< Output buffers (1D)
   dip::uint bufferLength;                      ///< Number of pixels in each buffer
   dip::uint dimension;                         ///< Dimension along which the line filter is applied
   UnsignedArray const& position;               ///< Coordinates of first pixel in line
   dip::uint thread;                            ///< Thread number
};

/// \brief Prototype line filter for `dip::Framework::Scan`.
///
/// An object of a class derived from `%ScanLineFilter` must be passed to the scan framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `dip::Framework::ScanLineFilter::SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for output values, such
/// that each threads has its own output variables that the calling function can later combine (reduce).
class ScanLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( ScanLineFilterParameters& params ) = 0;
      /// \brief The derived class can defined this function if it needs this information ahead of time.
      virtual void SetNumberOfThreads( dip::uint threads ) {}
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~ScanLineFilter() {}
};

/// \brief Framework for pixel-based processing of images.
///
/// The function object `lineFilter` is called for each image line, with input and
/// output buffers either pointing directly to the input and output images,
/// or pointing to temporary buffers that are handled by the framework and
/// serve to prevent `lineFilter` to have to deal with too many
/// different data types. The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. Output buffers are not initialized,
/// `lineFilter` is responsible for setting all its values.
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
/// Tensors are passed to `lineFilter` as vectors, if the shape is
/// important, pass this information through `functionParameters`. `nTensorElements`
/// gives the number of tensor elements for each output image. These are created
/// as standard vectors. The calling function can reshape the tensors after the
/// call to `%dip::Framework::Scan`. It is not necessary nor enforced that the
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
/// `%dip::Framework::Scan` will process the image using multiple threads, so
/// `lineFilter` will be called from multiple threads simultaneously. If it is not
/// thread safe, specify `dip::FrameWork::Scan_NoMultiThreading` as an option.
/// the `SetNumberOfThreads` method to `lineFilter` will be called once before
/// the processing starts, when `%dip::Framework::Scan` has determined how many
/// threads will be used in the scan.
void Scan(
      ImageConstRefArray const& in,             ///< Input images
      ImageRefArray& out,                       ///< Output images
      DataTypeArray const& inBufferTypes,       ///< Data types for input buffers
      DataTypeArray const& outBufferTypes,      ///< Data types for output buffers
      DataTypeArray const& outImageTypes,       ///< Data types for output images
      UnsignedArray const& nTensorElements,     ///< Number of tensor elements in output images
      ScanLineFilter* lineFilter,               ///< Pointer to function object to call for each image line
      ScanOptions opts                          ///< Options to control how `lineFilter` is called
);

/// \brief Calls `dip::Framework::Scan` with one output image.
inline void ScanSingleOutput(
      Image& out,                      ///< Output image
      DataType outImageType,           ///< Data type for output image, buffer will have this type also
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      ScanLineFilter* lineFilter,      ///< Pointer to function object to call for each image line
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{};
   ImageRefArray outar{ out };
   DataTypeArray inBufT{};
   DataTypeArray outBufT{ outImageType };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ nTensorElements };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, opts );
}

/// \brief Calls `dip::Framework::Scan` with one input image.
inline void ScanSingleInput(
      Image const& in,                 ///< Input image
      DataType bufferType,             ///< Data type for input buffer
      ScanLineFilter* lineFilter,      ///< Pointer to function object to call for each image line
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{ in };
   ImageRefArray outar{};
   DataTypeArray inBufT{ bufferType };
   DataTypeArray outBufT{};
   DataTypeArray outImT{};
   UnsignedArray nElem{};
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, opts );
}

/// \brief Calls `dip::Framework::Scan` with one input image and one output image.
inline void ScanMonadic(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType bufferTypes,            ///< Data type for all input and output buffers
      DataType outImageType,           ///< Data type for output image
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      ScanLineFilter* lineFilter,      ///< Pointer to function object to call for each image line
      ScanOptions opts                 ///< Options to control how `lineFilter` is called
) {
   ImageConstRefArray inar{ in };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ bufferTypes };
   DataTypeArray outBufT{ bufferTypes };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ nTensorElements };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, opts );
}

/// \brief Calls `dip::Framework::Scan` with two input images and one output image.
///
/// It handles some of the work for dyadic (binary) operators related to matching up
/// tensor dimensions in the input image.
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
      ScanLineFilter* lineFilter,      ///< Pointer to function object to call for each image line
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
      DIP_THROW( E::NTENSORELEM_DONT_MATCH );
   }
   ImageConstRefArray inar{ in1, in2 };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ inType, inType };
   DataTypeArray outBufT{ outType };
   DataTypeArray outImT{ outType };
   UnsignedArray nElem{ outTensor.Elements() };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, opts );
   out.ReshapeTensor( outTensor );
}

// TODO: ScanDyadic, ScanMonadic and ScanSingleInput are used a lot, I don't know how much overhead comes from converting these problems to the generic problem of N input images and M output images.


//
// Separable Framework:
// Process an image line by line, once for each dimension
//


/// \class dip::Framework::SeparableOptions
/// \brief Defines options to the `dip::Framework::Separable` function.
///
/// Valid values are:
///
/// `SeparableOptions` constant      | Meaning
/// -------------------------------- | ----------
/// `Separable_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not thread safe).
/// `Separable_AsScalarImage`        | The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
/// `Separable_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
/// `Separable_UseOutBorder`         | The output line buffer also has space allocated for a border.
/// `Separable_DontResizeOutput`     | The output image has the right size; it can differ from the input size
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( SeparableOptions );
DIP_DEFINE_OPTION( SeparableOptions, Separable_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_AsScalarImage, 1 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_ExpandTensorInBuffer, 2 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_UseOutBorder, 3 );
DIP_DEFINE_OPTION( SeparableOptions, Separable_DontResizeOutput, 4 );

/// \brief Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Separable` callback function object.
///
/// The length of the buffer
/// is given in a separate argument to the line filter. Depending on the
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

/// \brief Parameters to the line filter for `dip::Framework::Separable`.
///
/// We have put all the parameters to the line filter `dip::Framework::SeparableLineFilter::Filter` into
/// a single struct to simplify writing those functions.
struct SeparableLineFilterParameters {
   SeparableBuffer const& inBuffer;   ///< Input buffer (1D)
   SeparableBuffer& outBuffer;        ///< Output buffer (1D)
   dip::uint dimension;               ///< Dimension along which the line filter is applied
   UnsignedArray const& position;     ///< Coordinates of first pixel in line
   dip::uint thread;                  ///< Thread number
};

/// \brief Prototype line filter for `dip::Framework::Separable`.
///
/// An object of a class derived from `%SeparableLineFilter` must be passed to the separable framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `dip::Framework::SeparableLineFilter::SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for temporary buffers, such
/// that each threads has its own buffers to write in.
class SeparableLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( SeparableLineFilterParameters& params ) = 0;
      /// \brief The derived class can defined this function if it needs this information ahead of time.
      virtual void SetNumberOfThreads( dip::uint threads ) {}
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~SeparableLineFilter() {}
};

/// \brief Framework for separable filtering of images.
///
/// The function object `lineFilter` is called for each image line, and along each
/// dimension, with input and output buffers either pointing directly to the
/// input and output images, or pointing to temporary buffers that are handled
/// by the framework and present the line's pixel data with a different data type,
/// with expanded borders, etc. The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. The output buffer is not initialized,
/// `lineFilter` is responsible for setting all its values.
///
/// The `process` array specifies along which dimensions the filtering is applied.
/// If it is an empty array, all dimensions will be processed.
///
/// The output image (unless protected) will be resized to match the input,
/// and its type will be set to that specified by `outImage`.
/// A protected output image must have the correct size and type,
/// otherwise an exception will be thrown.
/// The separable filter always has one input and one output image.
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
/// Tensors are passed to lineFilter` as vectors, if the shape is
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
/// elements, and computed as if `filter` were called 3 times:
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
/// that line in the input image. The `lineFilter` function object can read up to `border`
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
/// `%dip::Framework::Separable` will process the image using multiple threads, so
/// `lineFilter` will be called from multiple threads simultaneously. If it is not
/// thread safe, specify `dip::FrameWork::Separable_NoMultiThreading` as an option.
/// the `SetNumberOfThreads` method to `lineFilter` will be called once before
/// the processing starts, when `%dip::Framework::Separable` has determined how many
/// threads will be used in the processing.
void Separable(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType bufferType,             ///< Data type for input and output buffer
      DataType outImageType,           ///< Data type for output image
      BooleanArray process,            ///< Determines along which dimensions to apply the filter
      UnsignedArray border,            ///< Number of pixels to add to the beginning and end of each line, for each dimension
      BoundaryConditionArray boundaryConditions, ///< Filling method for the border
      SeparableLineFilter* lineFilter, ///< Pointer to function object to call for each image line
      SeparableOptions opts            ///< Options to control how `lineFilter` is called
);


//
// Full Framework:
// Process an image line by line, with access to a full neighborhood given by a PixelTable
//


/// \class dip::Framework::FullOptions
/// \brief Defines options to the `dip::Framework::Full` function.
///
/// Valid values are:
///
/// `FullOptions` constant      | Meaning
/// --------------------------- | ----------
/// `Full_NoMultiThreading`     | Do not call the line filter simultaneouly from multiple threads (it is not thread safe).
/// `Full_AsScalarImage`        | The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
/// `Full_ExpandTensorInBuffer` | The line filter always gets input tensor elements as a standard, column-major matrix.
///
/// Combine options by adding constants together.
DIP_DECLARE_OPTIONS( FullOptions );
DIP_DEFINE_OPTION( FullOptions, Full_NoMultiThreading, 0 );
DIP_DEFINE_OPTION( FullOptions, Full_AsScalarImage, 1 );
DIP_DEFINE_OPTION( FullOptions, Full_ExpandTensorInBuffer, 2 );

/// \brief Structure that holds information about input or output pixel buffers
/// for the `dip::Framework::Full` callback function object.
///
/// Depending on the arguments given to the framework function, you might assume that
/// `tensorLength` is always 1, and consequently ignore also `tensorStride`.
struct FullBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

/// \brief Parameters to the line filter for `dip::Framework::Full`.
///
/// We have put all the parameters to the line filter `dip::Framework::FullLineFilter::Filter` into
/// a single struct to simplify writing those functions.
struct FullLineFilterParameters {
   FullBuffer const& inBuffer;            ///< Input buffer (1D)
   FullBuffer& outBuffer;                 ///< Output buffer (1D)
   dip::uint bufferLength;                ///< Number of pixels in each buffer
   dip::uint dimension;                   ///< Dimension along which the line filter is applied
   UnsignedArray const& position;         ///< Coordinates of first pixel in line
   PixelTableOffsets const& pixelTable;   ///< The pixel table object describing the neighborhood
   dip::uint thread;                      ///< Thread number
};

/// \brief Prototype line filter for `dip::Framework::Full`.
///
/// An object of a class derived from `%FullLineFilter` must be passed to the full framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `dip::Framework::FullLineFilter::SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for temporary buffers, such
/// that each threads has its own buffers to write in.
class FullLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( FullLineFilterParameters& params ) = 0;
      /// \brief The derived class can defined this function if it needs this information ahead of time.
      virtual void SetNumberOfThreads( dip::uint threads ) {}
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~FullLineFilter() {}
};

/// \brief Framework for filtering of images with an arbitrary shape neighborhood.
///
/// The function object `lineFilter` is called for each image line,
/// with input and output buffers either pointing directly to the
/// input and output images, or pointing to temporary buffers that are handled
/// by the framework and present the line's pixel data with a different data type,
/// with expanded borders, etc.  The buffers are always of the type specified in
/// `inBuffer` and `outBuffer`, but are passed as `void*`. `lineFilter` should
/// cast these pointers to the right types. The output buffer is not initialized,
/// `lineFilter` is responsible for setting all its values.
///
/// `lineFilter` can access the pixels on the given line for all
/// input and output images, as well as all pixels within the neighborhood for
/// all input images. The neighborhood is given by `pixelTable`. This object
/// defines the size of the border extension in the input buffer, as well as
/// the processing dimension. Use `dip::Framework::OptimalProcessingDim` to determine
/// the processing dimension to use when generating the `pixelTable`. Though under
/// some circumstances it might be beneficial to use a different dimension.
///
/// TODO: some filters benefit strongly from longer pixel runs, they'd want to
/// decrease the number of runs by picking a processing dimension where the filter
/// is wide.
///
/// The output image (unless protected) will be resized to match the input,
/// and its type will be set to that specified by `outImage`.
/// A protected output image must have the correct size and type,
/// otherwise an exception will be thrown.
/// The full filter always has one input and one output image.
///
/// Tensors are passed to `lineFilter` as vectors, if the shape is
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
/// elements, and computed as if `filter` were called 3 times:
/// `filter(in[0],out[0])`, `filter(in[1],out[1])`, and `filter(in[2],out[2])`.
///
/// If the option `dip::FrameWork::Full_ExpandTensorInBuffer` is given, then
/// the input buffer passed to `lineFilter` will contain the tensor elements as a
/// standard, column-major matrix. If the image has tensors stored differently,
/// buffers will be used. This option is not used when
/// `dip::FrameWork::Full_AsScalarImage` is set, as that forces the tensor
/// to be a single sample. Use this option if you need to do computations with
/// the tensors, but do not want to bother with all the different tensor shapes,
/// which are meant only to save memory. Note, however, that this option does
/// not apply to the output image. When expanding the input tensor in this
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
/// `lineFilter` can read any pixel within the neighborhood of
/// all the pixels on the line. These pixels are filled by the framework using
/// the `boundaryCondition` values. The `boundaryCondition` vector can be empty,
/// in which case the default boundary condition value is used.
/// `position` gives the coordinates for the first pixel in the buffers,
/// subsequent pixels occur along dimension `dimension`. `position[dimension]`
/// is always zero.
///
/// The input and output buffers will never share memory. That is, the line
/// filter can freely write in the output buffer without invalidating input
/// buffer, even when the filter is being applied in-place.
///
/// `%dip::Framework::Full` will process the image using multiple threads, so
/// `lineFilter` will be called from multiple threads simultaneously. If it is not
/// thread safe, specify `dip::FrameWork::Full_NoMultiThreading` as an option.
/// the `SetNumberOfThreads` method to `lineFilter` will be called once before
/// the processing starts, when `%dip::Framework::Full` has determined how many
/// threads will be used in the scan.
void Full(
      Image const& in,                 ///< Input image
      Image& out,                      ///< Output image
      DataType inBufferType,           ///< Data types for input buffer
      DataType outBufferType,          ///< Data types for output buffer
      DataType outImageType,           ///< Data types for output image
      dip::uint nTensorElements,       ///< Number of tensor elements in output image
      BoundaryConditionArray boundaryConditions, ///< Filling method for the border
      PixelTable const& pixelTable,    ///< Object describing the neighborhood
      FullLineFilter* lineFilter,      ///< Pointer to function object to call for each image line
      FullOptions opts                 ///< Options to control how `lineFilter` is called
);

/// \}

} // namespace Framework
} // namespace dip

#endif // DIP_FRAMEWORK_H
