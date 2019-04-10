/*
 * DIPlib 3.0
 * This file contains functionality for the OpenCV 2 interface.
 *
 * (c)2018, Flagship Biosciences. Written by Cris Luengo.
 * Based on dip_matlab_interface.h: (c)2015-2017, Cris Luengo.
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


#ifndef DIP_OPENCV_INTERFACE_H
#define DIP_OPENCV_INTERFACE_H

#include <map>
#include <utility>

#include "diplib.h"
#include "diplib/iterators.h"

#include <opencv2/opencv.hpp>
#include <diplib/library/error.h>


/// \file
/// \brief This file defines the `dip_opencv` namespace, functionality to interface *OpenCV 2* (or later) and *DIPlib*.


/// \brief The `%dip_opencv` namespace contains the interface between *OpenCV 2* (or later) and *DIPlib*.
namespace dip_opencv {


/// \defgroup dip_opencv_interface DIPlib--OpenCV interface
/// \brief Functions to convert images to and from *OpenCV*.
///
/// The `dip_opencv` namespace defines the functions needed to convert between *OpenCV* `cv::Mat` objects and *DIPlib*
/// `dip::Image` objects.
///
/// We define a class `dip_opencv::ExternalInterface` so that output images from *DIPlib* can yield an *OpenCV* image.
/// The function `dip_opencv::MatToDip` encapsulates (maps) an *OpenCV* image in a *DIPlib* image;
/// `dip_opencv::DipToMat` does the opposite, mapping a *DIPlib* image as an *OpenCV* image.
///
/// **Note:** *OpenCV* is more limited in how the pixel data is stored, and consequently not all *DIPlib* images can
/// be mapped as an *OpenCV* image. These are the limitations:
///  - The maximum number of channels in *OpenCV* is `CV_CN_MAX` (equal to 512 in my copy). *DIPlib* tensor elements are
///    mapped to channels, but the tensor shape is lost. The tensor stride must be 1.
///  - *OpenCV* recognizes the following types (depths): 8-bit and 16-bit signed and unsigned ints, 32-bit signed ints,
///    and 32-bit and 64-bit floats. Thus, `dip::DT_UINT32` cannot be mapped. We choose to map it to 32-bit signed
///    ints, with the potential problem that the upper half of the unsigned range is mapped to negative values
///    (all modern systems use two's complement). `dip::DT_BIN` is mapped to an 8-bit unsigned integer, see the note
///    below. 64-bit integer images cannot be mapped and throw an exception.
///  - Complex pixel values are mapped to `CV_32FC2` or `CV_64FC2` -- a 2-channel float `cv::Mat` array. Consequently,
///    complex-valued tensor images cannot be mapped.
///  - `cv::Mat` objects can store arrays of any dimensionality larger than 2, but *OpenCV* functionality
///    is mostly limited to 2D images. Therefore, we only map 2D `dip::Image` objects in a `cv::Mat` array.
///    0D or 1D images will have singleton dimensions appended to force them to be 2D.
///  - In *OpenCV*, the image rows must be contiguous (i.e. the x-stride must be equal to the number of tensor
///    elements), and the y-stride must be positive. This matches *DIPlib*'s default, but if the `dip::Image` object
///    has strides that don't match *OpenCV*'s requirement (e.g. after extracting a non-contiguous subset of pixels,
///    or calling `dip::Image::Mirror` or `dip::Image::Rotation90`), an exception will be thrown. Use
///    `dip::Image::ForceNormalStrides` to copy the image into a suitable order for mapping. Alternatively, use
///    `dip_opencv::CopyDipToMat`.
///
/// **Note:** *OpenCV* does not know a binary image type, and uses an 8-bit unsigned integer with values 0 and 255
/// where a binary image is intended. This clashes with *DIPlib*'s binary type, which is of the same size but is
/// expected to contain only 0 and 1 values. The functions `dip_opencv::FixBinaryImageForDip` and
/// `dip_opencv::FixBinaryImageForOpenCv` fix up binary images for processing in either library.
/// \{


/// \brief Creates a *DIPlib* image around an *OpenCV* `cv::Mat`, without taking ownership of the data.
///
/// This function maps a `cv::Mat` object to a `dip::Image` object.
/// The `dip::Image` object will point to the data in the `cv::Mat`, which must continue existing until the
/// `dip::Image` is deleted or `Strip`ped. The output `dip::Image` is protected to prevent accidental reforging,
/// unprotect it using `dip::Image::Protect`.
///
/// An empty `cv::Mat` produces a non-forged `dip::Image`.
///
/// If the *OpenCV* image `mat` has depth `CV_32S` (32-bit signed integer), and `forceUnsigned` is `true`, then
/// the output `dip::Image` will be of type `dip::DT_UINT32`, instead of `dip::DT_SINT32`.
inline dip::Image MatToDip( cv::Mat const& mat, bool forceUnsigned = false ) {
   if( mat.empty() ) {
      return {};
   }

   // Find data type
   dip::DataType dt;
   switch( mat.depth() ) {
      case CV_8S:
         dt = dip::DT_SINT8;
         break;
      case CV_8U:
         dt = dip::DT_UINT8;
         break;
      case CV_16S:
         dt = dip::DT_SINT16;
         break;
      case CV_16U:
         dt = dip::DT_UINT16;
         break;
      case CV_32S:
         dt = forceUnsigned ? dip::DT_UINT32 : dip::DT_SINT32;
         break;
      case CV_32F:
         dt = dip::DT_SFLOAT;
         break;
      case CV_64F:
         dt = dip::DT_DFLOAT;
         break;
      default:
         DIP_THROW( "OpenCV image with non-standard depth" );
   }

   // Find sizes and number of channels
   DIP_ASSERT( mat.dims >= 2 ); // This is supposed to be a cv::Mat invariant
   dip::UnsignedArray sizes( static_cast< dip::uint >( mat.dims ));
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      DIP_ASSERT( mat.size.p[ ii ] >= 1 );
      sizes[ ii ] = static_cast< dip::uint >( mat.size.p[ ii ] );
   }
   DIP_ASSERT( mat.channels() >= 1 );
   dip::Tensor tensor( static_cast< dip::uint >( mat.channels() ));

   // Find strides
   dip::IntegerArray strides( sizes.size() );
   for( dip::uint ii = 0; ii < sizes.size(); ++ii ) {
      strides[ ii ] = static_cast< dip::sint >( mat.step1( static_cast< int >( ii )));
   }
   dip::sint tstride = 1;

   // OpenCV swaps the first two dimensions:
   std::swap( sizes[ 0 ], sizes[ 1 ] );
   std::swap( strides[ 0 ], strides[ 1 ] );

   // Create Image object
   dip::Image img( dip::NonOwnedRefToDataSegment( mat.ptr() ), const_cast< uchar* >( mat.ptr() ), dt, sizes, strides, tensor, tstride );
   img.Protect();
   return img;
}


namespace detail {

inline int GetOpenMatType( dip::DataType dt, dip::uint nTensor ) {
   // Get OpenCV channels
   int channels = static_cast< int >( nTensor );
   DIP_THROW_IF( channels > CV_CN_MAX, "Image has too many channels for OpenCV" );

   // Handle complex images
   bool isComplex = dt.IsComplex();
   if( isComplex ) {
      DIP_THROW_IF( channels > 1, "Cannot map a complex-valued image with more than one channel" );
      channels = 2;
   }

   // Get OpenCV depth
   int depth;
   switch( dt ) {
      case dip::DT_BIN:
      case dip::DT_UINT8:
         depth = CV_8U;
         break;
      case dip::DT_SINT8:
         depth = CV_8S;
         break;
      case dip::DT_UINT16:
         depth = CV_16U;
         break;
      case dip::DT_SINT16:
         depth = CV_16S;
         break;
      case dip::DT_UINT32:
         // We're pretending this is a signed 32-bit integer -- caution!
      case dip::DT_SINT32:
         depth = CV_32S;
         break;
      case dip::DT_SFLOAT:
      case dip::DT_SCOMPLEX:
         depth = CV_32F;
         break;
      case dip::DT_DFLOAT:
      case dip::DT_DCOMPLEX:
         depth = CV_64F;
         break;
      default:
         DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }

   return CV_MAKETYPE(depth, channels );
}


inline cv::Size GetOpenMatSizes(
      dip::UnsignedArray const& sizes
) {
   // Get OpenCV sizes
   dip::uint nDims = sizes.size();
   DIP_THROW_IF( nDims > 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   cv::Size matSizes{ 1, 1 };
   if( nDims > 0 ) {
      DIP_THROW_IF( sizes[ 0 ] > static_cast< dip::uint >( std::numeric_limits< int >::max() ), dip::E::SIZE_EXCEEDS_LIMIT );
      matSizes.width = static_cast< int >( sizes[ 0 ]);
   }
   if( nDims > 1 ) {
      DIP_THROW_IF( sizes[ 1 ] > static_cast< dip::uint >( std::numeric_limits< int >::max() ), dip::E::SIZE_EXCEEDS_LIMIT );
      matSizes.height = static_cast< int >( sizes[ 1 ]);
   }
   return matSizes;
}

inline size_t GetOpenMatStep(
      dip::IntegerArray const& strides,
      dip::DataType dt,
      dip::uint nTensor
) {
   // Get OpenCV sizes and strides
   dip::uint nDims = strides.size();
   DIP_THROW_IF( nDims > 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   size_t matStep = nTensor;
   if( nDims > 0 ) {
      DIP_THROW_IF( strides[ 0 ] != static_cast< dip::sint >( nTensor ), "Cannot map an image with non-contiguous rows" );
   }
   if( nDims > 1 ) {
      DIP_THROW_IF( strides[ 1 ] < strides[ 0 ], "Cannot map an image with a non-positive stride" );
      matStep = static_cast< dip::uint >( strides[ 1 ]);
   }

   // Convert steps to number of bytes
   dip::uint elsz = dt.SizeOf();
   matStep *= elsz;
   return matStep;
}

} // namespace detail

/// \brief Creates an *OpenCV* `cv::Mat` object around a *DIPlib* image, without taking ownership of the data.
///
/// This function maps a `dip::Image` object to a `cv::Mat` object.
/// The `cv::Mat` object will point to the data in the `dip::Image`, which must continue existing until the
/// `cv::Mat` is deleted.
///
/// A non-forged `dip::Image` produces an empty `cv::Mat`.
///
/// There are many limitations to the images that can be mapped by a `cv::Mat`, see the description in the
/// documentation to the module: \ref dip_opencv_interface. You can also use `dip_opencv::CopyDipToMat` instead.
inline cv::Mat DipToMat( dip::Image const& img ) {
   if( !img.IsForged() ) {
      return {};
   }
   DIP_THROW_IF(( img.TensorElements() > 1 ) && ( img.TensorStride() != 1 ), "OpenCV requires the tensor stride to be 1" );
   int type;
   DIP_STACK_TRACE_THIS( type = detail::GetOpenMatType( img.DataType(), img.TensorElements() ));
   cv::Size matSizes;
   DIP_STACK_TRACE_THIS( matSizes = detail::GetOpenMatSizes( img.Sizes() ));
   size_t matSteps;
   DIP_STACK_TRACE_THIS( matSteps = detail::GetOpenMatStep( img.Strides(), img.DataType(), img.TensorElements() ));
   cv::Mat mat( matSizes, type, img.Origin(), matSteps );
   return mat;
}


/// \brief Creates an *OpenCV* `cv::Mat` object from a *DIPlib* image by copy.
///
/// A non-forged `dip::Image` produces an empty `cv::Mat`.
///
/// If the image has more than two dimensions, or is a complex-valued tensor image, no copy can be made;
/// an exception will be thrown.
inline cv::Mat CopyDipToMat( dip::Image const& img ) {
   //std::cout << "-- Making a copy of a DIPlib image to an OpenCV matrix --\n"; // Enable this line for debugging
   if( !img.IsForged() ) {
      return {};
   }
   int type;
   DIP_STACK_TRACE_THIS( type = detail::GetOpenMatType( img.DataType(), img.TensorElements() ));
   cv::Size matSizes;
   DIP_STACK_TRACE_THIS( matSizes = detail::GetOpenMatSizes( img.Sizes() ));
   cv::Mat mat( matSizes, type );
   dip::Image tmp;
   DIP_STACK_TRACE_THIS( tmp = MatToDip( mat, img.DataType() == dip::DT_UINT32 ));
   if( img.DataType().IsComplex() ) {
      DIP_STACK_TRACE_THIS( tmp.MergeTensorToComplex() );
   }
   tmp.Copy( img );
   return mat;
}


/// \brief This class is the `dip::ExternalInterface` for the *OpenCV* interface.
///
/// Use the following code when declaring images to be used as the output to a *DIPlib* function:
/// ```cpp
///     dip_opencv::ExternalInterface cvei;
///     dip::Image img_out0 = cvei.NewImage();
///     dip::Image img_out1 = cvei.NewImage();
/// ```
/// This configures the images `img_out0` and `img_out1` such that, when they are forged later on, an `cv::Mat`
/// object will be created to hold the pixel data.
///
/// However, there are many limitations to the images that can be mapped by a `cv::Mat`, see the description in the
/// documentation to the module: \ref dip_opencv_interface. For these images, the allocator will fail, prompting
/// *DIPlib* to use its own, default allocator instead. The resulting `dip::Image` object cannot be converted back
/// to an *OpenCV* object, though it might be possible to convert parts of it (for example each 2D plane separately).
///
/// The `%ExternalInterface` object owns the `cv::Mat` objects. You need to keep it around as long as you use the
/// image objects returned by its `NewImage` method, otherwise the data segments will be freed and the `dip::Image`
/// objects will point to non-existing data segments.
///
/// To retrieve the `cv::Mat` object inside such a `dip::Image`, use the `dip_opencv::ExternalInterface::GetMat`
/// method:
/// ```cpp
///     cv::Mat img0 = cvei.GetMat( img_out0 );
///     cv::Mat img1 = cvei.GetMat( img_out1 );
/// ```
/// If you don't use the `GetMat` method, the `cv::Mat` that contains the pixel data will be destroyed when the
/// `dip::Image` object goes out of scope. The `GetMat` method returns a `cv::Mat` object that owns the data segment
/// used by the `dip::Image` object. In this case, the `dip::Image` object is still valid, and shares the data segment
/// with the extracted `cv::Mat`. If the `cv::Mat` is destroyed, the data segment will be freed and the `dip::Image`
/// object will point to a non-existing data segment.
///
/// Remember to not assign a result into the images created with `NewImage`, as the pixel data will be copied in the
/// assignment. Instead, use the *DIPlib* functions that take output images as function arguments:
/// ```cpp
///     img_out0 = in1 + in2;           // Bad! Incurs an unnecessary copy
///     dip::Add( in1, in2, img_out0 ); // Correct, the operation writes directly in the output data segment
/// ```
/// In the first case, `in1 + in2` is computed into a temporary image, whose pixels are then copied into the
/// `cv::Mat` created for `img_out0`. In the second case, the result of the operation is directly written into the
/// `cv::Mat`, no copies are necessary.
class ExternalInterface : public dip::ExternalInterface {
   private:
      // This map holds `cv::Mat`s, we can find the right one if we have the data pointer.
      std::map< void const*, cv::Mat > images_;
      // This is the deleter functor we'll associate to the dip::DataSegment.
      class StripHandler {
         private:
            ExternalInterface& interface;
         public:
            StripHandler( ExternalInterface& ei ) : interface{ ei } {};
            void operator()( void const* p ) {
               interface.images_.erase( p );
            };
      };

   public:
      // This function overrides `dip::ExternalInterface::AllocateData`. It is called when an image with this
      // `ExternalInterface` is forged. It allocates an *OpenCV* `cv::Mat` and returns a `dip::DataSegment` to the
      // data pointer, with a custom deleter functor. Strides are set to normal, and an exception is thrown if
      // the dimensionality is not supported.
      //
      // A user will never call this function directly.
      virtual dip::DataSegment AllocateData(
            void*& origin,
            dip::DataType datatype,
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tstride
      ) override {
         int type;
         DIP_STACK_TRACE_THIS( type = detail::GetOpenMatType( datatype, tensor.Elements() ));
         cv::Size matSizes;
         DIP_STACK_TRACE_THIS( matSizes = detail::GetOpenMatSizes( sizes )); // Will throw if nDims > 2
         dip::uint nDims = sizes.size();
         strides.resize( nDims, static_cast< dip::sint >( tensor.Elements() ));
         if( nDims > 1 ) {
            strides[ 1 ] = static_cast< dip::sint >( sizes[ 0 ] ) * strides[ 0 ];
         }
         tstride = 1;
         cv::Mat mat( matSizes, type );
         origin = mat.ptr();
         images_.emplace( origin, std::move( mat ));
         return dip::DataSegment{ origin, StripHandler( *this ) };
      }

      /// \brief Returns the *OpenCV* `cv::Mat` that holds the data for the `dip::Image` `img`.
      ///
      /// The *OpenCV* `cv::Mat` returned is the one allocated to hold the pixel data in the input
      /// `img`. If `img` is a view of another image, or has been manipulated through `dip::Image::Mirror` or
      /// `dip::Image::Rotation90`, the pixel data will be copied into a new `cv::Mat` object.
      ///
      /// If the `dip::Image` object does not point to data in a `cv::Mat` object, the pixel data will be copied
      /// into a new `cv::Mat` object.
      cv::Mat GetMat( dip::Image const& img ) {
         DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
         void const* ptr = img.Data();
         auto it = images_.find( ptr );
         if( it == images_.end() ) {
            // This image was not forged by this interface. Let's copy instead.
            DIP_STACK_TRACE_THIS( return CopyDipToMat( img ));
         }
         cv::Mat mat{ std::move( it->second ) }; // In OpenCV 2.9 there is no move constructor nor move assignment... The docs to 4.0.0-pre also don't show these
         images_.erase( ptr );
         // Check to make sure `mat` and `img` provide the same view
         dip::uint nDims = img.Dimensionality();
         int type;
         DIP_STACK_TRACE_THIS( type = detail::GetOpenMatType( img.DataType(), img.TensorElements() ));
         if(   (( nDims > 0 ) && ( img.Stride( 0 ) != static_cast< dip::sint >( img.TensorElements() )))
            || (( nDims > 1 ) && ( img.Stride( 1 ) <= img.Stride( 0 ) ))
            || (( img.TensorElements() > 1 ) && ( img.TensorStride() != 1 ))
            || ( type != mat.type() )) {
            // This image does not fit in a cv::Mat, we must have modified it.
            DIP_STACK_TRACE_THIS( return CopyDipToMat( img ));
         }
         cv::Size matSizes;
         DIP_STACK_TRACE_THIS( matSizes = detail::GetOpenMatSizes( img.Sizes() ));
         if(( matSizes.height != mat.rows ) || ( matSizes.width != mat.cols )) {
            // It looks like a cropped view, let's try to recreate it:
            dip::sint offset = ( static_cast< dip::uint8* >( img.Origin() ) - static_cast< dip::uint8* >( img.Data() ))
                               / static_cast< dip::sint >( img.DataType().SizeOf() * img.TensorElements() );
            int col = static_cast< int >( offset % img.Stride( 1 ));
            int row = static_cast< int >( offset / img.Stride( 1 ));
            mat = mat( cv::Range{ row, row + matSizes.height }, cv::Range{ col, col + matSizes.width } );
         }
         return mat;
      }

      /// \brief Constructs a `dip::Image` object with the external interface set so that,
      /// when forged, a *OpenCV* `cv::Mat` will be allocated to hold the samples.
      dip::Image NewImage() {
         dip::Image out;
         out.SetExternalInterface( this );
         return out;
      }
};


/// \brief Fixes the binary image `img` to match expectations of *DIPlib* (i.e. only the bottom bit is used).
inline void FixBinaryImageForDip( dip::Image& img ) {
   dip::ImageIterator< dip::bin >it( img ); // throws if input image is not binary, or not forged.
   do {
      if( *it ) {
         *it = true; // Note that `*it = *it` does not do this. `*it = !!*it` might work.
      }
   } while( ++it );
}

/// \brief Fixes the binary image `img` to match expectations of *OpenCV* (i.e. all bits have the same value).
inline void FixBinaryImageForOpenCv( dip::Image& img ) {
   dip::ImageIterator< dip::bin >it( img ); // throws if input image is not binary, or not forged.
   do {
      if( *it ) {
         static_cast< dip::uint8& >( *it ) = 255;
      }
   } while( ++it );
}


/// \}

} // namespace dip_opencv

#endif // DIP_OPENCV_INTERFACE_H
