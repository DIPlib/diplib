/*
 * (c)2016-2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 *
 * Credit to Wouter Caarls for suggesting what is now `VariadicScanLineFilter`.
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

#ifndef DIP_FRAMEWORK_H
#define DIP_FRAMEWORK_H

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "diplib.h"
#include "diplib/boundary.h"
#include "diplib/kernel.h"


/// \file
/// \brief Frameworks are the basis of most pixel-based processing in *DIPlib*.
/// See \ref frameworks, \ref infrastructure.


namespace dip {

// Forward declaration
class DIP_NO_EXPORT PixelTableOffsets;


/// \group frameworks Frameworks
/// \ingroup infrastructure
/// \brief Functions that form the basis of most pixel-based processing in *DIPlib*.
///
/// The various frameworks implement iterating over image pixels, giving access to a single pixel, a whole image line,
/// or a pixel's neighborhood. The programmer needs to define a function that loops over one dimension. The framework
/// will call this function repeatedly to process all the image's lines, thereby freeing the programmer from
/// implementing loops over multiple dimensions. This process allows most of *DIPlib*'s filters to be dimensionality
/// independent, with little effort from the programmer. See \ref design_frameworks.
///
/// There are three frameworks that represent three different types of image processing functions:
///
/// - The Scan framework, to process individual pixels across multiple input and output images: \ref dip::Framework::Scan.
/// - The Separable framework, to apply separable filters: \ref dip::Framework::Separable.
/// - The Full framework, to apply non-separable filters: \ref dip::Framework::Full.
/// \addtogroup


/// \brief Frameworks are the basis of most pixel-based processing in *DIPlib*.
namespace Framework {


// Maximum number of pixels in a buffer for the scan framework
constexpr dip::uint MAX_BUFFER_SIZE = 256 * 1024; // NOLINT(*-implicit-widening-of-multiplication-result)


//
// Support functions
//

/// \brief Determines the singleton-expanded size as a combination of the two sizes.
///
/// Singleton dimensions (size==1) can be expanded to match another image's size. This function can be used to check
/// if such expansion is possible, and what the resulting sizes would be. `size1` is adjusted. An exception is thrown
/// if the singleton expansion is not possible.
DIP_EXPORT void SingletonExpandedSize( UnsignedArray& size1, UnsignedArray const& size2 );

/// \brief Determines if images can be singleton-expanded to the same size, and what that size would be.
///
/// Singleton dimensions (size==1) can be expanded to a larger size by setting their stride to 0. This change can be
/// performed without modifying the data segment. If image dimensions differ such that singleton expansion cannot make
/// them all the same size, an exception is thrown. Use \ref dip::Image::ExpandSingletonDimensions to apply the
/// transform to one image.
DIP_EXPORT UnsignedArray SingletonExpandedSize( ImageConstRefArray const& in );

/// \brief Determines if images can be singleton-expanded to the same size, and what that size would be.
///
/// Singleton dimensions (size==1) can be expanded to a larger size by setting their stride to 0. This change can be
/// performed without modifying the data segment. If image dimensions differ such that singleton expansion cannot make
/// them all the same size, an exception is thrown. Use \ref dip::Image::ExpandSingletonDimensions to apply the
/// transform to one image.
DIP_EXPORT UnsignedArray SingletonExpandedSize( ImageArray const& in );

/// \brief Determines if tensors in images can be singleton-expanded to the same size, and what that size would be.
///
/// The tensors must all be of the same size, or of size 1. The tensors with size 1 are singletons, and can be
/// expended to the size of the others by setting their stride to 0. This change can be performed without modifying
/// the data segment. If singleton expansion cannot make them all the same size, an exception is thrown.
/// Use \ref dip::Image::ExpandSingletonTensor to apply the transform to one image.
DIP_EXPORT dip::uint SingletonExpendedTensorElements( ImageArray const& in );

/// \brief Determines the best processing dimension, which is the one with the smallest stride, except if that
/// dimension is very small and there's a longer dimension.
DIP_EXPORT dip::uint OptimalProcessingDim( Image const& in );

/// \brief Determines the best processing dimension as above, but giving preference to a dimension where
/// `kernelSizes` is large also.
DIP_EXPORT dip::uint OptimalProcessingDim( Image const& in, UnsignedArray const& kernelSizes );


//
// Scan Framework:
// Process one or more images pixel by pixel
//


/// \brief Defines options to the \ref dip::Framework::Scan function.
///
/// Implicitly casts to \ref dip::Framework::ScanOptions. Combine constants together with the `+` operator.
enum class ScanOption : uint8 {
      NoMultiThreading,       ///< Do not call the line filter simultaneously from multiple threads (it is not thread safe).
      NeedCoordinates,        ///< The line filter needs the coordinates to the first pixel in the buffer.
      TensorAsSpatialDim,     ///< Tensor dimensions are treated as a spatial dimension for scanning, ensuring that the line scan filter always gets scalar pixels.
      ExpandTensorInBuffer,   ///< The line filter always gets input tensor elements as a standard, column-major matrix.
      NoSingletonExpansion,   ///< Inhibits singleton expansion of input images.
      NotInPlace              ///< The line filter can write to the output buffers without affecting the input buffers.
};
/// \class dip::Framework::ScanOptions
/// \brief Combines any number of \ref dip::Framework::ScanOption constants together.
DIP_DECLARE_OPTIONS( ScanOption, ScanOptions )

/// \brief Structure that holds information about input or output pixel buffers for the \ref dip::Framework::Scan
/// callback function object.
///
/// The length of the buffer is given in a separate argument to the line filter. Depending on the arguments given to the
/// framework function, you might assume that `tensorLength` is always 1, and consequently ignore also `tensorStride`.
struct DIP_NO_EXPORT ScanBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

/// \brief Parameters to the line filter for \ref dip::Framework::Scan.
///
/// We have put all the parameters to the line filter \ref dip::Framework::ScanLineFilter::Filter into
/// a single struct to simplify writing those functions.
///
/// Note that `dimension` and `position` are within the images that have had their tensor dimension
/// converted to spatial dimension, if \ref dip::Framework::ScanOption::TensorAsSpatialDim was given and at least
/// one input or output image is not scalar. In this case, `tensorToSpatial` is `true`, and the last dimension
/// corresponds to the tensor dimension. `dimension` will never be equal to the last dimension in this case.
/// That is, `position` will have one more element than the original image(s) we're iterating over, but
/// `position[ dimension ]` will always correspond to a position in the original image(s).
struct DIP_NO_EXPORT ScanLineFilterParameters {
   std::vector< ScanBuffer > const& inBuffer;   ///< Input buffers (1D)
   std::vector< ScanBuffer >& outBuffer;        ///< Output buffers (1D)
   dip::uint bufferLength;                      ///< Number of pixels in each buffer
   dip::uint dimension;                         ///< Dimension along which the line filter is applied
   UnsignedArray const& position;               ///< Coordinates of first pixel in line
   bool tensorToSpatial;                        ///< `true` if the tensor dimension was converted to spatial dimension
   dip::uint thread;                            ///< Thread number
};

/// \brief Prototype line filter for \ref dip::Framework::Scan.
///
/// An object of a class derived from `ScanLineFilter` must be passed to the scan framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for output values, such
/// that each threads has its own output variables that the calling function can later combine (reduce). Note
/// that this function is called even if \ref dip::Framework::ScanOption::NoMultiThreading is given, or if the library
/// is compiled without multi-threading.
///
/// The `GetNumberOfOperations` method is called to determine if it is worthwhile to start worker threads and
/// perform the computation in parallel. This function should not perform any other tasks, as it is not
/// guaranteed to be called. It is not important that the function be very precise, see \ref design_multithreading.
class DIP_CLASS_EXPORT ScanLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( ScanLineFilterParameters const& params ) = 0;
      /// \brief The derived class can define this function for setting up the processing.
      virtual void SetNumberOfThreads( dip::uint threads ) { ( void )threads; }
      /// \brief The derived class can define this function for helping to determine whether to compute
      /// in parallel or not. It must return the number of clock cycles per input pixel. The default is valid for
      /// an arithmetic-like operation.
      virtual dip::uint GetNumberOfOperations( dip::uint nInput, dip::uint nOutput, dip::uint nTensorElements ) {
         return std::max( nInput, nOutput ) * nTensorElements;
      }
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~ScanLineFilter() = default;
};

/// \brief Framework for pixel-based processing of images.
///
/// The function object `lineFilter` is called for each image line, with input and output buffers either pointing
/// directly to the input and output images, or pointing to temporary buffers that are handled by the framework and
/// serve to prevent `lineFilter` to have to deal with too many different data types. The buffers are always of the
/// type requested by the `inBufferTypes` and `outBufferTypes` parameters, but are passed as `void*`.
/// `lineFilter` should cast these pointers to the right types.
/// Output buffers are not initialized, `lineFilter` is responsible for setting all its values.
///
/// Output images (unless protected) will be resized to match the (singleton-expanded) input, but have a number of
/// tensor elements specified by `nTensorElements`, and their type will be set to that specified by `outImageTypes`.
/// Protected output images must have the correct size and type, otherwise an exception will be thrown.
/// The scan function can be called without input images. In this case, at least one output image must be given.
/// The dimensions of the first output image will be used to direct the scanning, and the remaining output images (if
/// any) will be adjusted to the same size. It is also possible to give no output images, as would be the case for a
/// reduction operation such as computing the average pixel value. However, it makes no sense to call the scan
/// function without input nor output images.
///
/// Tensors are passed to `lineFilter` as vectors, if the shape is important, store this information in `lineFilter`.
/// `nTensorElements` gives the number of tensor elements for each output image. These are created as standard vectors.
/// The calling function can reshape the tensors after the call to `dip::Framework::Scan`. It is not necessary nor
/// enforced that the tensors for each image (both input and output) are the same, the calling function is to make
/// sure the tensors satisfy whatever constraints.
///
/// However, if the option \ref dip::Framework::ScanOption::TensorAsSpatialDim is given, then the tensor is cast to a
/// spatial dimension, and singleton expansion is applied. Thus, `lineFilter` does not need to check `inTensorLength`
/// or `outTensorLength` (they will be 1), and the output tensor size is guaranteed to match the largest input tensor.
/// `nTensorElements` is ignored. Even with a single input image, where no singleton expansion can happen, it is
/// beneficial to use the \ref dip::Framework::ScanOption::TensorAsSpatialDim option, as `lineFilter` can be simpler
/// and faster. Additionally, the output tensor shape is identical to the input image's. In case of multiple inputs,
/// the first input image that has as many tensor elements as the (singleton-expanded) output will model the output
/// tensor shape.
///
/// If the option \ref dip::Framework::ScanOption::ExpandTensorInBuffer is given, then the input buffers passed to
/// `lineFilter` will contain the tensor elements as a standard, column-major matrix. If the image has tensors stored
/// differently, buffers will be used. This option is not used when \ref dip::Framework::ScanOption::TensorAsSpatialDim
/// is set, as that forces the tensor to be a single sample. Use this option if you need to do computations with the
/// tensors, but do not want to bother with all the different tensor shapes, which are meant only to save memory.
/// Note, however, that this option does not apply to the output images. When expanding the input tensors in this way,
/// it makes sense to set the output tensor to a full matrix. Don't forget to specify the right size in `nTensorElements`.
///
/// The framework function sets the output pixel size to that of the first input image with a defined pixel size,
/// and it sets the color space to that of the first input image with matching number of tensor elements.
/// The calling function is expected to "correct" these values if necessary.
///
/// The buffers are not guaranteed to be contiguous, please use the `stride` and `tensorStride` values to access samples.
/// All buffers contain `bufferLength` pixels. `position` gives the coordinates for the first pixel in the buffers,
/// subsequent pixels occur along dimension `dimension`. `position[dimension]` is not necessarily zero.
/// However, when \ref dip::Framework::ScanOption::NeedCoordinates is not given, `dimension` and `position` are
/// meaningless. The framework is allowed to treat all pixels in the image as a single image line in this case.
///
/// If `in` and `out` share an image, then it is possible that the corresponding input and output buffers point to the
/// same memory. The input image will be overwritten with the processing result. That is, all processing can be
/// performed in place. The scan framework is intended for pixel-wise processing, not neighborhood-based processing,
/// so there is never a reason not to work in place. However, some types of tensor processing might want to write to
/// the output without invalidating the input for that same pixel. In this case, give the option
/// \ref dip::Framework::ScanOption::NotInPlace. It will make sure that the output buffers given to the line filter
/// do not alias the input buffers.
///
/// `dip::Framework::Scan` will process the image using multiple threads, so `lineFilter` will be called from multiple
/// threads simultaneously. If it is not thread safe, specify \ref dip::Framework::ScanOption::NoMultiThreading as an
/// option. The `SetNumberOfThreads` method to `lineFilter` will be called once before the processing starts, when
/// `dip::Framework::Scan` has determined how many threads will be used in the scan, even if
/// \ref dip::Framework::ScanOption::NoMultiThreading was specified.
DIP_EXPORT void Scan(
      ImageConstRefArray const& in,
      ImageRefArray& out,
      DataTypeArray const& inBufferTypes,
      DataTypeArray const& outBufferTypes,
      DataTypeArray const& outImageTypes,
      UnsignedArray const& nTensorElements,
      ScanLineFilter& lineFilter,
      ScanOptions opts = {}
);

/// \brief Calls \ref dip::Framework::Scan with one output image, which is already forged.
/// The `lineFilter` will be called with an output buffer of type `bufferType`.
inline void ScanSingleOutput(
      Image& out,
      DataType bufferType,
      ScanLineFilter& lineFilter,
      ScanOptions opts = {}
) {
   ImageRefArray outar{ out };
   Scan( {}, outar, {}, { bufferType }, { out.DataType() }, { out.TensorElements() }, lineFilter, opts );
}

/// \brief Calls \ref dip::Framework::Scan with one input image and a mask image, and no output image.
///
/// If `mask` is forged, it is expected to be a scalar image of type \ref dip::DT_BIN, and of size compatible with `in`.
/// `mask` is singleton-expanded to the size of `in`, but not the other way around. Its pointer will be passed to
/// `lineFilter` directly, without copies to change its data type. Thus, `inBuffer[ 1 ].buffer` is of type `bin*`,
/// not of type `bufferType`.
inline void ScanSingleInput(
      Image const& in,
      Image const& c_mask,
      DataType bufferType,
      ScanLineFilter& lineFilter,
      ScanOptions opts = {}
) {
   ImageConstRefArray inar;
   inar.reserve( 2 );
   inar.emplace_back( in );
   DataTypeArray inBufT{ bufferType };
   Image mask;
   if( c_mask.IsForged() ) {
      // If we have a mask, add it to the input array.
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
      inar.emplace_back( mask );
      inBufT.push_back( mask.DataType() );
   }
   ImageRefArray outar{};
   DIP_STACK_TRACE_THIS( Scan( inar, outar, inBufT, {}, {}, {}, lineFilter, opts ));
}

/// \brief Calls \ref dip::Framework::Scan with one input image and one output image.
///
/// `bufferTypes` is the type for both the input and output buffer. The output image will be reforged to have the
/// same sizes as the input image, and `nTensorElements` and `outImageType`.
inline void ScanMonadic(
      Image const& in,
      Image& out,
      DataType bufferTypes,
      DataType outImageType,
      dip::uint nTensorElements,
      ScanLineFilter& lineFilter,
      ScanOptions opts = {}
) {
   ImageRefArray outar{ out };
   Scan( { in }, outar, { bufferTypes }, { bufferTypes }, { outImageType }, { nTensorElements }, lineFilter, opts );
}

/// \brief Calls \ref dip::Framework::Scan with two input images and one output image.
///
/// It handles some of the work for dyadic (binary) operators related to matching up tensor dimensions in the input image.
///
/// Input tensors are expected to match, but a scalar is expanded to the size of the other tensor.
/// The output tensor will be of the same size as the input tensors, its shape will match the input shape if one image
/// is a scalar, or if both images have matching tensor shapes. Otherwise the output tensor will be a column-major
/// matrix (or vector or scalar, as appropriate).
///
/// This function adds \ref dip::Framework::ScanOption::TensorAsSpatialDim or \ref dip::Framework::ScanOption::ExpandTensorInBuffer
/// to `opts`, so don't set these values yourself. This means that the tensors passed to `lineFilter` is either all
/// scalars (the tensor can be converted to a spatial dimension) or full, column-major tensors of equal size.
/// Do not specify \ref dip::Framework::ScanOption::NoSingletonExpansion in `opts`.
inline void ScanDyadic(
      Image const& in1,
      Image const& in2,
      Image& out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      ScanLineFilter& lineFilter,
      ScanOptions opts = {}
) {
   Tensor outTensor;
   if( in1.IsScalar() ) {
      outTensor = in2.Tensor();
      opts += ScanOption::TensorAsSpatialDim;
   } else if( in2.IsScalar() || ( in1.Tensor() == in2.Tensor() )) {
      outTensor = in1.Tensor();
      opts += ScanOption::TensorAsSpatialDim;
   } else if( in1.TensorSizes() == in2.TensorSizes() ) {
      outTensor = Tensor( in1.TensorRows(), in1.TensorColumns() );
      opts += ScanOption::ExpandTensorInBuffer;
   } else {
      DIP_THROW( E::NTENSORELEM_DONT_MATCH );
   }
   ImageConstRefArray inar{ in1, in2 };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ inBufferType, inBufferType };
   DataTypeArray outBufT{ outBufferType };
   DataTypeArray outImT{ outImageType };
   UnsignedArray nElem{ outTensor.Elements() };
   Scan( inar, outar, inBufT, outBufT, outImT, nElem, lineFilter, opts );
   out.ReshapeTensor( outTensor );
}

/// An implementation of the ScanLinefilter for N input images and 1 output image.
///
/// Here, all buffers are of the same data type, and the scalar operation applied to each sample is the lambda
/// function of type F, passed to the constructor. All input and output images must have the same number of tensor
/// elements, and in the same order.
///
/// When `N` = 1, the resulting object can be passed to the \ref dip::Framework::ScanMonadic function. When `N` = 2,
/// you can use the \ref dip::Framework::ScanDyadic function. For any other `N`, or when \ref dip::Framework::ScanDyadic
/// does not do the right thing, use \ref dip::Framework::Scan.
///
/// The following example shows how to make a dyadic operator that performs computations in `sfloat` and generates
/// an output image of that same type.
///
/// ```cpp
/// dip::Image lhs = ...;
/// dip::Image rhs = ...;
/// dip::Image out;
/// dip::dfloat offset = 40;
/// auto sampleOperator = [ = ]( std::array< dip::sfloat const*, 2 > its ) { return ( *its[ 0 ] * 100 ) / ( *its[ 1 ] * 10 ) + offset; };
/// dip::Framework::VariadicScanLineFilter< 2, dip::sfloat, decltype( sampleOperator ) > scanLineFilter( sampleOperator );
/// dip::Framework::ScanDyadic( lhs, rhs, out, dip::DT_SFLOAT, dip::DT_SFLOAT, scanLineFilter );
/// ```
///
/// `sampleOperator` is a lambda function, which captures `offset` by value (note that capturing by reference will
/// slow down the execution significantly). It has a single input argument, a `std::array` of 2 float pointers. The
/// first pointer will point at a sample in the `lhs` input image, and the second to the corresponding sample in the
/// `rhs` input image. The return value of the lambda will be assigned to the corresponding sample in the output image.
/// Note that to access the sample values, you need to use the syntax `*its[ 0 ]`, where the `0` is the index into
/// the array, yielding a pointer, which is dereferenced by the `*` operator to access the sample value.
///
/// To use the `VariadicScanLineFilter` with dynamic data type dispatch, it is necessary to use an auxiliary function.
/// Such an auxiliary function also simplifies the use of the class template:
///
/// ```cpp
/// template< typename TPI, typename F >
/// std::unique_ptr< dip::Framework::ScanLineFilter > NewFilter( F func ) {
///    return static_cast< std::unique_ptr< dip::Framework::ScanLineFilter >>( new dip::Framework::VariadicScanLineFilter< 1, TPI, F >( func ));
/// }
/// // ...
/// dip::Image in = ...;
/// dip::Image out;
/// dip::DataType dt = in.DataType();
/// std::unique_ptr< dip::Framework::ScanLineFilter > scanLineFilter;
/// DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, NewFilter, (
///       [ = ]( auto its ) { return ( std::cos( *its[ 0 ] ) * 100 ) + offset; }
/// ), dt );
/// dip::Framework::ScanMonadic( in, out, dt, dt, in.TensorElements(), *scanLineFilter, dip::Framework::ScanOption::TensorAsSpatialDim );
/// ```
///
/// Notice in this case we used a generic lambda, i.e. its input parameter has type `auto`. It will be compiled
/// differently for each allowed data type. The function template `NewFilter` that we defined specifies the `N` in
/// the `VariadicScanLineFilter` template, and helps pass on the type of the lambda, which is automatically determined
/// by the compiler and filled out. That is, the function can be called as `NewFilter< dip::sfloat >( sampleOperator )`.
/// The object is allocated in free memory, and its lifetime is managed by `std::unique_ptr`, meaning that there is
/// no need to explicitly delete the object. Next, we use the `DIP_OVL_CALL_ASSIGN_REAL` macro to call different
/// instantiations of our function, depending on the data type `dt`, which we determine dynamically from the input
/// image. The output is captured in a variable and passed to the scan function.
///
/// For values of `N` from 1 to 4 there are pre-defined functions just like the `NewFilter` function above:
/// \ref dip::Framework::NewMonadicScanLineFilter, \ref dip::Framework::NewDyadicScanLineFilter,
/// \ref dip::Framework::NewTriadicScanLineFilter, \ref dip::Framework::NewTetradicScanLineFilter.
/// These functions take an optional second input argument `cost`, which specifies the cost in cycles to execute
/// a single call of `func`. This cost is used to determine if it's worthwhile to parallelize the operation, see
/// \ref design_multithreading.
template< dip::uint N, typename TPI, typename F >
class DIP_NO_EXPORT VariadicScanLineFilter : public ScanLineFilter {
   // Note that N is a compile-time constant, and consequently the compiler should be able to optimize all the loops
   // over N.
   public:
      static_assert( N > 0, "VariadicScanLineFilter does not work without input images" );
      VariadicScanLineFilter( F const& func, dip::uint cost = 1 ) : func_( func ), cost_( cost ) {}
      dip::uint GetNumberOfOperations( dip::uint /**/, dip::uint /**/, dip::uint nTensorElements ) override {
         return cost_ * nTensorElements;
      }
      void Filter( ScanLineFilterParameters const& params ) override {
         DIP_ASSERT( params.inBuffer.size() == N );
         DIP_ASSERT( params.outBuffer.size() == 1 );
         std::array< TPI const*, N > in;
         std::array< dip::sint, N > inStride;
         std::array< dip::sint, N > inTensorStride;
         dip::uint const bufferLength = params.bufferLength;
         dip::uint const tensorLength = params.outBuffer[ 0 ].tensorLength; // all buffers have same number of tensor elements
         for( dip::uint ii = 0; ii < N; ++ii ) {
            in[ ii ] = static_cast< TPI const* >( params.inBuffer[ ii ].buffer );
            inStride[ ii ] = params.inBuffer[ ii ].stride;
            if( tensorLength > 1 ) {
               inTensorStride[ ii ] = params.inBuffer[ ii ].tensorStride;
            }
            DIP_ASSERT( params.inBuffer[ ii ].tensorLength == tensorLength );
         }
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint const outStride = params.outBuffer[ 0 ].stride;
         dip::sint const outTensorStride = params.outBuffer[ 0 ].tensorStride;
         if( tensorLength > 1 ) {
            for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
               std::array< TPI const*, N > inT = in;
               TPI* outT = out;
               for( dip::uint jj = 0; jj < tensorLength; ++jj ) {
                  *outT = func_( inT );
                  for( dip::uint ii = 0; ii < N; ++ii ) {
                     inT[ ii ] += inTensorStride[ ii ];
                  }
                  outT += outTensorStride;
               }
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  in[ ii ] += inStride[ ii ];
               }
               out += outStride;
            }
         } else {
            for( dip::uint kk = 0; kk < bufferLength; ++kk ) {
               *out = func_( in );
               for( dip::uint ii = 0; ii < N; ++ii ) {
                  in[ ii ] += inStride[ ii ];
               }
               out += outStride;
            }
         }
      }
   private:
      F func_; // save a copy of the lambda, in case we want to use it with a temporary-constructed lambda that captures a variable.
      dip::uint cost_ = 1;
};

/// \brief Support for quickly defining monadic operators (1 input image, 1 output image).
/// See \ref dip::Framework::VariadicScanLineFilter.
template< typename TPI, typename F >
inline std::unique_ptr< ScanLineFilter > NewMonadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< ScanLineFilter >>( new VariadicScanLineFilter< 1, TPI, F >( func, cost ));
}

/// \brief Support for quickly defining dyadic operators (2 input images, 1 output image).
/// See \ref dip::Framework::VariadicScanLineFilter.
template< typename TPI, typename F >
inline std::unique_ptr< ScanLineFilter > NewDyadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< ScanLineFilter >>( new VariadicScanLineFilter< 2, TPI, F >( func, cost ));
}

/// \brief Support for quickly defining triadic operators (3 input images, 1 output image).
/// See \ref dip::Framework::VariadicScanLineFilter.
template< typename TPI, typename F >
inline std::unique_ptr< ScanLineFilter > NewTriadicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< ScanLineFilter >>( new VariadicScanLineFilter< 3, TPI, F >( func, cost ));
}

/// \brief Support for quickly defining tetradic operators (4 input images, 1 output image).
/// See \ref dip::Framework::VariadicScanLineFilter.
template< typename TPI, typename F >
inline std::unique_ptr< ScanLineFilter > NewTetradicScanLineFilter( F const& func, dip::uint cost = 1 ) {
   return static_cast< std::unique_ptr< ScanLineFilter >>( new VariadicScanLineFilter< 4, TPI, F >( func, cost ));
}


//
// Separable Framework:
// Process an image line by line, once for each dimension
//


/// \brief Defines options to the \ref dip::Framework::Separable function.
///
/// Implicitly casts to \ref dip::Framework::SeparableOptions. Combine constants together with the `+` operator.
enum class SeparableOption : uint8 {
      NoMultiThreading,          ///< Do not call the line filter simultaneously from multiple threads (it is not thread safe).
      AsScalarImage,             ///< The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
      ExpandTensorInBuffer,      ///< The line filter always gets input tensor elements as a standard, column-major matrix.
      UseOutputBorder,           ///< The output line buffer also has space allocated for a border.
      DontResizeOutput,          ///< The output image has the right size; it can differ from the input size.
      UseInputBuffer,            ///< The line filter can modify the input data without affecting the input image; samples are guaranteed to be contiguous.
      UseOutputBuffer,           ///< The output buffer is guaranteed to have contiguous samples.
      CanWorkInPlace,            ///< The input and output buffer are allowed to both point to the same memory.
      UseRealComponentOfOutput   ///< If the buffer type is complex, and the output type is not, cast by taking the real component of the complex data, rather than the modulus.
};
/// \class dip::Framework::SeparableOptions
/// \brief Combines any number of \ref dip::Framework::SeparableOption constants together.
DIP_DECLARE_OPTIONS( SeparableOption, SeparableOptions )

/// \brief Structure that holds information about input or output pixel buffers
/// for the \ref dip::Framework::Separable callback function object.
///
/// The length of the buffer is given in a separate argument to the line filter. Depending on the arguments given to the
/// framework function, you might assume that `tensorLength` is always 1, and consequently ignore also `tensorStride`.
struct DIP_NO_EXPORT SeparableBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::uint length;       ///< Length of the buffer, not counting the expanded boundary
   dip::uint border;       ///< Length of the expanded boundary at each side of the buffer.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

/// \brief Parameters to the line filter for \ref dip::Framework::Separable.
///
/// We have put all the parameters to the line filter \ref dip::Framework::SeparableLineFilter::Filter into
/// a single struct to simplify writing those functions.
///
/// Note that `dimension` and `position` are within the images that have had their tensor dimension converted to
/// spatial dimension, if \ref dip::Framework::SeparableOption::AsScalarImage was given and the input is not scalar.
/// In this case, `tensorToSpatial` is `true`, and the last dimension corresponds to the tensor dimension.
/// `dimension` will never be equal to the last dimension in this case. That is, `position` will have one more element
/// than the original image(s) we're iterating over, but `position[ dimension ]` will always correspond to a position
/// in the original image(s).
struct DIP_NO_EXPORT SeparableLineFilterParameters {
   SeparableBuffer const& inBuffer;   ///< Input buffer (1D)
   SeparableBuffer& outBuffer;        ///< Output buffer (1D)
   dip::uint dimension;               ///< Dimension along which the line filter is applied
   dip::uint pass;                    ///< Pass number (0..nPasses-1)
   dip::uint nPasses;                 ///< Number of passes (typically nDims)
   UnsignedArray const& position;     ///< Coordinates of first pixel in line
   bool tensorToSpatial;              ///< `true` if the tensor dimension was converted to spatial dimension
   dip::uint thread;                  ///< Thread number
};

/// \brief Prototype line filter for \ref dip::Framework::Separable.
///
/// An object of a class derived from `SeparableLineFilter` must be passed to the separable framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for temporary buffers, such
/// that each threads has its own buffers to write in. Note that this function is called even if
/// \ref dip::Framework::SeparableOption::NoMultiThreading is given, or if the library is compiled without multi-threading.
///
/// The `GetNumberOfOperations` method is called to determine if it is worthwhile to start worker threads and
/// perform the computation in parallel. This function should not perform any other tasks, as it is not
/// guaranteed to be called. It is not important that the function be very precise, see \ref design_multithreading.
class DIP_CLASS_EXPORT SeparableLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( SeparableLineFilterParameters const& params ) = 0;
      /// \brief The derived class can define this function for setting up the processing.
      virtual void SetNumberOfThreads( dip::uint threads ) { ( void )threads; }
      /// \brief The derived class can define this function for helping to determine whether to whether to compute
      /// in parallel or not. It must return the number of clock cycles per image line. The default is valid for
      /// a convolution-like operation.
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint nTensorElements, dip::uint border, dip::uint procDim ) {
         ( void )procDim; // not used in this function, but useful for some line filters.
         return lineLength * nTensorElements * 2 * ( 2 * border + 1 ); // 2*border+1 is filter size, double that for the number of multiply-adds.
      }
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~SeparableLineFilter() = default;
};

/// \brief Framework for separable filtering of images.
///
/// The function object `lineFilter` is called for each image line, and along each dimension, with input and output
/// buffers either pointing directly to the input and output images, or pointing to temporary buffers that are handled
/// by the framework and present the line's pixel data with a different data type, with expanded borders, etc.
/// The buffers are always of the type specified in `bufferType`, but are passed as `void*`.
/// `lineFilter` should cast these pointers to the right types. The output buffer is not initialized, `lineFilter`
/// is responsible for setting all its values.
///
/// The `process` array specifies along which dimensions the filtering is applied. If it is an empty array, all
/// dimensions will be processed. Otherwise, it must have one element per image dimension.
///
/// The output image (unless protected) will be resized to match the input, and its type will be set to that specified
/// by `outImage`. A protected output image must have the correct size and type, otherwise an exception will be thrown.
/// The separable filter always has one input and one output image.
///
/// If the option \ref dip::Framework::SeparableOption::DontResizeOutput is given, then the sizes of the output image
/// will be kept (but it could still be reforged to change the data type). In this case, the length of the input and
/// output buffers can differ, causing the intermediate result image to change size one dimension at the time, as each
/// dimension is processed. For example, if the input image is of size 256x256, and the output is 1x1, then in a first
/// step 256 lines are processed, each with 256 pixels as input and a single pixel as output. In a second step, a single
/// line of 256 pixels is processed yielding the final single-pixel result. In the same case, but with an output of
/// 64x512, 256 lines are processed, each with 256 pixels as input and 64 pixels as output. In the second step,
/// 64 lines are processed, each with 256 pixels as input and 512 pixels as output. This option is useful for functions
/// that scale and do other geometric transformations, as well as functions that compute projections.
///
/// Tensors are passed to `lineFilter` as vectors, if the shape is important, store this information in `lineFilter`.
/// The output image will have the same tensor shape as the input except if the option
/// \ref dip::Framework::SeparableOption::ExpandTensorInBuffer is given. In this case, the input buffers passed to
/// `lineFilter` will contain the tensor elements as a standard, column-major matrix, and the output image will be
/// a full matrix of that size. If the input image has tensors stored differently, buffers will be used when processing
/// the first dimension; for subsequent dimensions, the intermediate result will already contain the full matrix.
/// Use this option if you need to do computations with the tensors, but do not want to bother with all the different
/// tensor shapes, which are meant only to save memory.
///
/// However, if the option \ref dip::Framework::SeparableOption::AsScalarImage is given, then the line filter is called
/// for each tensor element, effectively causing the filter to process a sequence of scalar images, one for each tensor
/// element. This is accomplished by converting the tensor into a spatial dimension for both the input and output image,
/// and setting the `process` array for the new dimension to false. For example, given an input image `in` with 3 tensor
/// elements, `filter(in,out)` will result in an output image `out` with 3 tensor elements, and computed as if `filter`
/// were called 3 times: `filter(in[0],out[0])`, `filter(in[1],out[1])`, and `filter(in[2],out[2])`.
///
/// The framework function sets the output tensor size to that of the input image, and it sets the color space to that
/// of the input image if the two images have matching number of tensor elements (these can differ if
/// \ref dip::Framework::SeparableOption::ExpandTensorInBuffer is given). The calling function is expected to "correct"
/// these values if necessary. Note the difference here with the `Scan` and `Full` frameworks: it is not possible to
/// apply a separate filter to a tensor image and obtain an output with a different tensor representation
/// (because the question arises: in which image pass does this change occur?).
///
/// The buffers are not guaranteed to be contiguous, please use the `stride` and `tensorStride` values to access samples.
/// The \ref dip::Framework::SeparableOption::UseInputBuffer and \ref dip::Framework::SeparableOption::UseOutputBuffer
/// options force the use of temporary buffers to store each image line. These temporary buffers always have contiguous
/// samples, with the tensor stride equal to 1 and the spatial stride equal to the number of tensor elements.
/// That is, the tensor elements for each pixel are contiguous, and the pixels are contiguous. This is useful when
/// calling external code to process the buffers, and that external code expects input data to be contiguous.
/// These buffers will also be aligned to a 32-byte boundary.
/// Forcing the use of an input buffer is also useful when the algorithm needs to write temporary data to its input,
/// for example, to compute the median of the input data by sorting. If the input has a stride of 0 in the dimension
/// being processed (this happens when expanding singleton dimensions), it means that a single pixel is repeated across
/// the whole line. This property is preserved in the buffer. Thus, even when these two flags are used, you need to
/// check the `stride` value and deal with the singleton dimension appropriately.
///
/// The input buffer contains `bufferLength + 2 * border` pixels. The pixel pointed to by the `buffer` pointer is the
/// first pixel on that line in the input image. The `lineFilter` function object can read up to `border` pixels before
/// that pixel, and up to `border` pixels after the last pixel on the line. These pixels are filled by the framework
/// using the `boundaryCondition` value for the given dimension. The `boundaryCondition` array can be empty, in which
/// case the default boundary condition value is used. If the option \ref dip::Framework::SeparableOption::UseOutputBorder
/// is given, then the output buffer also has `border` extra samples at each end. These extra samples are meant to help
/// in the computation for some filters, and are not copied back to the output image. `position` gives the coordinates
/// for the first pixel in the buffers, subsequent pixels occur along dimension `dimension`.
/// `position[dimension]` is always zero.
///
/// If `in` and `out` share their data segments, then the input image might be overwritten with the processing result.
/// However, the input and output buffers will not share memory. That is, the line filter can freely write in the output
/// buffer without invalidating the input buffer, even when the filter is being applied in-place.
/// The \ref dip::Framework::SeparableOption::CanWorkInPlace option causes the input and output buffer to potentially
/// both point to the same image data, if input and output images are the same and everything else falls into place
/// as well. It is meant to save some copy work for those algorithms that can work in-place, but does not guarantee
/// that the output buffer points to the input data.
///
/// If `in` and `out` share their data segments (e.g. they are the same image), then the filtering operation can be
/// applied completely in place, without any temporary images. For this to be possible, `outImageType`, `bufferType`
/// and the input image data type must all be the same.
///
/// `dip::Framework::Separable` will process the image using multiple threads, so `lineFilter` will be called from
/// multiple threads simultaneously. If it is not thread safe, specify \ref dip::Framework::SeparableOption::NoMultiThreading
/// as an option. The `SetNumberOfThreads` method to `lineFilter` will be called once before the processing starts,
/// when `dip::Framework::Separable` has determined how many threads will be used in the processing, even if
/// \ref dip::Framework::SeparableOption::NoMultiThreading was specified.
DIP_EXPORT void Separable(
      Image const& in,
      Image& out,
      DataType bufferType,
      DataType outImageType,
      BooleanArray process,
      UnsignedArray border,
      BoundaryConditionArray boundaryCondition,
      SeparableLineFilter& lineFilter,
      SeparableOptions opts = {}
);


/// \brief Framework for filtering of image lines. This is a version of \ref Separable that works along one
/// dimension only.
///
/// Here we describe only the differences with `dip::Framework::Separable`. If it is not described here, refer to
/// \ref Separable.
///
/// The input and output buffers can be of different types, `inBufferType` and  `outBufferType` determine these
/// two types. Note that this would not be possible in the separable framework function: the output of one
/// pass is the input to the next pass, so the data types of input and output must be the same.
///
/// Instead of a `process` array, there is a `processingDimension` parameter, which specifies which dimension
/// the filter will be applied along. Both `border` and `boundaryCondition` are scalars instead of arrays, and
/// apply to `processingDimension`.
DIP_EXPORT void OneDimensionalLineFilter(
      Image const& in,
      Image& out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint processingDimension,
      dip::uint border,
      BoundaryCondition boundaryCondition,
      SeparableLineFilter& lineFilter,
      SeparableOptions opts = {}
);


//
// Full Framework:
// Process an image line by line, with access to a full neighborhood given by a PixelTable
//


/// \brief Defines options to the \ref dip::Framework::Full function.
///
/// Implicitly casts to \ref dip::Framework::FullOptions. Combine constants together with the `+` operator.
enum class FullOption : uint8 {
      NoMultiThreading,       ///< Do not call the line filter simultaneously from multiple threads (it is not thread safe).
      AsScalarImage,          ///< The line filter is called for each tensor element separately, and thus always sees pixels as scalar values.
      ExpandTensorInBuffer,   ///< The line filter always gets input tensor elements as a standard, column-major matrix.
      BorderAlreadyExpanded   ///< The input image already has expanded boundaries (see \ref dip::ExtendImage, use `"masked"` option).
};
/// \class dip::Framework::FullOptions
/// \brief Combines any number of \ref dip::Framework::FullOption constants together.
DIP_DECLARE_OPTIONS( FullOption, FullOptions )

/// \brief Structure that holds information about input or output pixel buffers
/// for the \ref dip::Framework::Full callback function object.
///
/// Depending on the arguments given to the framework function, you might assume that
/// `tensorLength` is always 1, and consequently ignore also `tensorStride`.
struct DIP_NO_EXPORT FullBuffer {
   void* buffer;           ///< Pointer to pixel data for image line, to be cast to expected data type.
   dip::sint stride;       ///< Stride to walk along pixels.
   dip::sint tensorStride; ///< Stride to walk along tensor elements.
   dip::uint tensorLength; ///< Number of tensor elements.
};

/// \brief Parameters to the line filter for \ref dip::Framework::Full.
///
/// We have put all the parameters to the line filter \ref dip::Framework::FullLineFilter::Filter into
/// a single struct to simplify writing those functions.
struct DIP_NO_EXPORT FullLineFilterParameters {
   FullBuffer const& inBuffer;            ///< Input buffer (1D)
   FullBuffer& outBuffer;                 ///< Output buffer (1D)
   dip::uint bufferLength;                ///< Number of pixels in each buffer
   dip::uint dimension;                   ///< Dimension along which the line filter is applied
   UnsignedArray const& position;         ///< Coordinates of first pixel in line
   PixelTableOffsets const& pixelTable;   ///< The pixel table object describing the neighborhood
   dip::uint thread;                      ///< Thread number
};

/// \brief Prototype line filter for \ref dip::Framework::Full.
///
/// An object of a class derived from `FullLineFilter` must be passed to the full framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the line filter, that hold output values,
/// or that hold intermediate buffers. The `SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for temporary buffers,
/// such that each threads has its own buffers to write in. It is also the first place where the line filter
/// can see what the pixel table looks like (as it depends on the processing dimension determined by the
/// framework), and so it's a good place to determine some processing options.
/// Note that this function is called even if
/// \ref dip::Framework::FullOption::NoMultiThreading is given, or if the library is compiled without multi-threading.
///
/// The `GetNumberOfOperations` method is called to determine if it is worthwhile to start worker threads and
/// perform the computation in parallel. This function should not perform any other tasks, as it is not
/// guaranteed to be called. It is not important that the function be very precise, see \ref design_multithreading.
class DIP_CLASS_EXPORT FullLineFilter {
   public:
      /// \brief The derived class must must define this method, this is the actual line filter.
      virtual void Filter( FullLineFilterParameters const& params ) = 0;
      /// \brief The derived class can define this function for setting up the processing.
      virtual void SetNumberOfThreads( dip::uint threads, PixelTableOffsets const& pixelTable ) {
         ( void )threads; ( void )pixelTable;
      }
      /// \brief The derived class can define this function for helping to determine whether to compute
      /// in parallel or not. It must return the number of clock cycles per image line. The default is valid for
      /// a convolution-like operation.
      virtual dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint nTensorElements, dip::uint nKernelPixels, dip::uint nRuns ) {
         return lineLength * nTensorElements * nKernelPixels   // number of multiply-adds
              + lineLength * ( 2 * nKernelPixels + nRuns )     // iterating over pixel table
              + lineLength * nKernelPixels;                    // iterating over pixel table weights
      }
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~FullLineFilter() = default;
};

/// \brief Framework for filtering of images with an arbitrary shape neighborhood.
///
/// The function object `lineFilter` is called for each image line, with input and output buffers either pointing
/// directly to the input and output images, or pointing to temporary buffers that are handled by the framework and
/// present the line's pixel data with a different data type, with expanded borders, etc.  The buffers are always of
/// the type specified in `inBufferType` and `outBufferType`, but are passed as `void*`. `lineFilter` should cast
/// these pointers to the right types. The output buffer is not initialized, `lineFilter` is responsible for setting
/// all its values.
///
/// `lineFilter` can access the pixels on the given line for all input and output images, as well as all pixels within
/// the neighborhood for all input images. The neighborhood is given by `kernel`. This object defines the size of the
/// border extension in the input buffer.
///
/// The output image `out` (unless protected) will be resized to match the input `in`, but have `nTensorElements`
/// tensor elements, and its type will be set to that specified by `outImageType`. A protected output image must have
/// the correct size and type, otherwise an exception will be thrown. The full filter always has one input and one
/// output image.
///
/// Tensors are passed to `lineFilter` as vectors, if the shape is important, store this information in `lineFilter`.
/// `nTensorElements` gives the number of tensor elements for the output image. These are created as standard vectors,
/// unless the input image has the same number of tensor elements, in which case that tensor shape is copied.
/// The calling function can reshape the tensors after the call to \ref dip::Framework::Full. It is not necessary nor
/// enforced that the tensors for each image (both input and output) are the same, the calling function is to make
/// sure the tensors satisfy whatever constraints.
///
/// However, if the option \ref dip::Framework::FullOption::AsScalarImage is given, then the line filter is called for
/// each tensor element, effectively causing the filter to process a sequence of scalar images, one for each tensor
/// element. `nTensorElements` is ignored, and set to the number of tensor elements of the input. For example, given
/// an input image `in` with 3 tensor elements, `filter(in,out)` will result in an output image `out` with 3 tensor
/// elements, and computed as if `filter` were called 3 times: `filter(in[0],out[0])`, `filter(in[1],out[1])`, and
/// `filter(in[2],out[2])`.
///
/// If the option \ref dip::Framework::FullOption::ExpandTensorInBuffer is given, then the input buffer passed to
/// `lineFilter` will contain the tensor elements as a standard, column-major matrix. If the image has tensors stored
/// differently, buffers will be used. This option is not used when \ref dip::Framework::FullOption::AsScalarImage
/// is set, as that forces the tensor to be a single sample. Use this option if you need to do computations with the
/// tensors, but do not want to bother with all the different tensor shapes, which are meant only to save memory.
/// Note, however, that this option does not apply to the output image. When expanding the input tensor in this way,
/// it makes sense to set the output tensor to a full matrix. Don't forget to specify the right size in `nTensorElements`.
///
/// The framework function sets the output pixel size to that of the input image, and it sets the color space to that
/// of the input image if the two images have matching number of tensor elements. The calling function is expected
/// to "correct" these values if necessary.
///
/// The buffers are not guaranteed to be contiguous, please use the `stride` and `tensorStride` values to access samples.
/// The pixel pointed to by the `buffer` pointer is the first pixel on that line in the input image.
/// `lineFilter` can read any pixel within the neighborhood of all the pixels on the line. These pixels are filled by
/// the framework using the `boundaryCondition` values. The `boundaryCondition` vector can be empty, in which case
/// the default boundary condition value is used.
///
/// If the option \ref dip::Framework::FullOption::BorderAlreadyExpanded is given, then the input image is presumed
/// to have been expanded using the function \ref "dip::ExtendImage" (specify the option `"masked"`). That is, it is
/// possible to read outside the image bounds within an area given by the size of `kernel`. If the tensor doesn't need
/// to be expanded, and the image data type matches the buffer data type, then the input image will not be copied.
/// In this case, a new data segment will always be allocated for the output image. That is, the operation cannot
/// be performed in place. Also, `boundaryCondition` are ignored.
///
/// `position` gives the coordinates for the first pixel in the buffers, subsequent pixels occur along dimension
/// `dimension`. `position[dimension]` is always zero. If \ref dip::Framework::FullOption::AsScalarImage was given and
/// the input image has more than one tensor element, then `position` will have an additional element.
/// Use `pixelTable.Dimensionality()` to determine how many of the elements in `position` to use.
///
/// The input and output buffers will never share memory. That is, the line filter can freely write in the output
/// buffer without invalidating the input buffer, even when the filter is being applied in-place.
///
/// `dip::Framework::Full` will process the image using multiple threads, so `lineFilter` will be called from multiple
/// threads simultaneously. If it is not thread safe, specify \ref dip::Framework::FullOption::NoMultiThreading as an
/// option. The `SetNumberOfThreads` method to `lineFilter` will be called once before the processing starts, when
/// `dip::Framework::Full` has determined how many threads will be used in the scan, even if
/// \ref dip::Framework::FullOption::NoMultiThreading was specified.
DIP_EXPORT void Full(
      Image const& in,
      Image& out,
      DataType inBufferType,
      DataType outBufferType,
      DataType outImageType,
      dip::uint nTensorElements,
      BoundaryConditionArray const& boundaryCondition,
      Kernel const& kernel,
      FullLineFilter& lineFilter,
      FullOptions opts = {}
);


//
// Projection Framework:
// Process an image sub-image by sub-image, yielding a single output value per sub-image.
//


/// \brief Defines options to the \ref dip::Framework::Projection function.
///
/// Implicitly casts to \ref dip::Framework::ProjectionOptions. Combine constants together with the `+` operator.
enum class ProjectionOption : uint8 {
   NoMultiThreading,      ///< Do not call the projection function simultaneously from multiple threads (it is not thread safe).
};
/// \class dip::Framework::ProjectionOptions
/// \brief Combines any number of \ref dip::Framework::ProjectionOption constants together.
DIP_DECLARE_OPTIONS( ProjectionOption, ProjectionOptions )

/// \brief Prototype line filter for \ref dip::Framework::Projection.
///
/// An object of a class derived from `ProjectionFunction` must be passed to the projection framework. The derived
/// class can be a template class, such that the line filter is overloaded for each possible pixel data type.
///
/// A derived class can have data members that hold parameters to the projection function, that hold output values,
/// or that hold intermediate buffers. The `SetNumberOfThreads` method is
/// called once before any processing starts. This is a good place to allocate space for temporary buffers,
/// such that each threads has its own buffers to write in. Note that this function is called even if
/// \ref dip::Framework::ProjectionOption::NoMultiThreading is given, or if the library is compiled without multi-threading.
///
/// The `GetNumberOfOperations` method is called to determine if it is worthwhile to start worker threads and
/// perform the computation in parallel. This function should not perform any other tasks, as it is not
/// guaranteed to be called. It is not important that the function be very precise, see \ref design_multithreading.
class ProjectionFunction {
   public:
      /// \brief The filter to be applied to each sub-image, which fills out a single sample in `out`.
      /// The `out` sample is of the `outImageType` type requested in the call to `Projection`.
      virtual void Project( Image const& in, Image const& mask, Image::Sample& out, dip::uint thread ) = 0;
      /// \brief The derived class can define this function if it needs this information ahead of time.
      virtual void SetNumberOfThreads( dip::uint threads ) { ( void )threads; }
      /// \brief The derived class can define this function for helping to determine whether to compute
      /// in parallel or not. It must return the number of clock cycles per sub-image. The default is valid for
      /// a trivial projection operation such as max or mean.
      virtual dip::uint GetNumberOfOperations( dip::uint nPixels ) {
            return nPixels;
      }
      /// \brief A virtual destructor guarantees that we can destroy a derived class by a pointer to base
      virtual ~ProjectionFunction() = default;
};

/// \brief Framework for projecting one or more dimensions of an image.
///
/// `process` determines which dimensions of the input image `in` will be collapsed. `out` will have the
/// same dimensionality as `in`, but the dimensions that are `true` in `process` will have a size of 1
/// (i.e. be singleton dimensions); the remaining dimensions will be of the same size as in `in`.
///
/// The function object `projectionFunction` is called for each sub-image that projects onto a single sample.
/// Each tensor element is processed independently, and so the sub-image is always a scalar image.
/// For example, when computing the sum over the entire image, the `projectionFunction` is called once for
/// each tensor element, with a scalar image the size of the full input image as input. When computing the
/// sum over image rows, the `projectionFunction` is called once for each tensor element and each row of the
/// image, with a scalar image the size of one image row.
///
/// The projection function cannot make any assumptions about contiguous data or input dimensionality. The
/// input will be transformed such that it has as few dimensions as possible, just to make the looping inside
/// the projection function more efficient.
///
/// The output image `out` (unless protected) will be resized to match the required output size,
/// and its type will be set to that specified by `outImageType`. A protected output image must have
/// the correct size, otherwise an exception will be thrown, but can have a different data type.
///
/// The output sample in the projection function will always be of type `outImageType`, even if the output
/// image cannot be converted to that type (in which case the framework function will take care of casting
/// each output value generated by the projection function to the output type).
///
/// `dip::Framework::Projection` will process the image using multiple threads, so `projectionFunction`
/// will be called from multiple threads simultaneously. If it is not thread safe, specify
/// \ref dip::Framework::ProjectionOption::NoMultiThreading as an option. The `SetNumberOfThreads`
/// method to `projectionFunction` will be called once before the processing starts, when
/// `dip::Framework::Projection` has determined how many threads will be used in the scan, even if
/// \ref dip::Framework::FullOption::NoMultiThreading was specified.
void Projection(
      Image const& in,
      Image const& mask,
      Image& out,
      DataType outImageType,
      BooleanArray process,
      ProjectionFunction& projectionFunction,
      ProjectionOptions opts = {}
);


/// \endgroup

} // namespace Framework
} // namespace dip

#endif // DIP_FRAMEWORK_H
