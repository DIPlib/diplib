/*
 * (c)2026, Tolga Ciftcicelik.
 * Based on dip_opencv_interface.h: (c)2018-2021, Flagship Biosciences. Written by Cris Luengo.
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


#ifndef DIP_QIMAGE_INTERFACE_H
#define DIP_QIMAGE_INTERFACE_H

#include <map>

#include "diplib.h"

#include <QImage>


/// \file
/// \brief This file defines the \ref dip_qimage namespace, functionality to interface *Qt*'s `QImage` and *DIPlib*.


/// \group dip_qimage_interface *DIPlib*-*Qt* interface
/// \ingroup interfaces
/// \brief Functions to convert images to and from *Qt*'s `QImage`.
///
/// The \ref dip_qimage namespace defines the functions needed to convert between *Qt* `QImage` objects and *DIPlib*
/// \ref dip::Image objects.
///
/// We define a class \ref dip_qimage::ExternalInterface so that output images from *DIPlib* can yield a `QImage`,
/// for example to display the result of a processing pipeline in a *Qt* UI. The function \ref dip_qimage::QImageToDip
/// encapsulates (maps) a `QImage` in a *DIPlib* image; there currently is no copyless way to go the other direction
/// for an arbitrary \ref dip::Image, see \ref dip_qimage::ExternalInterface instead.
///
/// \section dip_qimage_limitations Limitations
///
/// `QImage` is much more limited in how the pixel data is stored than a \ref dip::Image, and consequently only a
/// small subset of *DIPlib* images can be mapped to (or from) a `QImage`. These are the limitations:
///
/// - Only 8-bit unsigned integer samples (\ref dip::DT_UINT8) are supported, because these are the only depth that
///   `QImage` natively supports for the formats we use. If your image has any other data type, convert it first,
///   for example with \ref dip::Convert. Note that this conversion clips or truncates values, it does not rescale
///   them: an image with a wider range (e.g. 16-bit unsigned data read from a PNG or TIFF file) must be scaled to
///   the 0-255 range before conversion, or the result will be clipped to white.
///
/// - Only 1 (grayscale), 3 (RGB) or 4 (RGBA) channels are supported, mapping to `QImage::Format_Grayscale8`,
///   `QImage::Format_RGB888` and `QImage::Format_RGBA8888`, respectively. The tensor stride must be 1 (interleaved
///   channels), matching *DIPlib*'s default.
///
/// - Only 2D images can be mapped.
///
/// !!! attention
///     When a \ref dip::Image reads a file with an alpha channel, the alpha channel is the last tensor element
///     (2nd or 4th), matching the channel order expected by `QImage::Format_RGBA8888`. No channel reordering is
///     needed.
/// \addtogroup


/// \brief The `dip_qimage` namespace contains the interface between *Qt*'s `QImage` and *DIPlib*.
namespace dip_qimage {


namespace detail {

inline QImage::Format GetQImageFormat( dip::uint channels ) {
   switch( channels ) {
      case 1: return QImage::Format_Grayscale8;
      case 3: return QImage::Format_RGB888;
      case 4: return QImage::Format_RGBA8888;
      default: DIP_THROW( "QImage interface only supports 1 (gray), 3 (RGB) or 4 (RGBA) channels" );
   }
}

} // namespace detail


/// \brief Creates a *DIPlib* image around a *Qt* `QImage`, without taking ownership of the data.
///
/// This function maps a `QImage` object to a \ref dip::Image object. The `dip::Image` object will point to the
/// data in the `QImage`, which must continue existing until the `dip::Image` is deleted or stripped. The output
/// \ref dip::Image is protected to prevent accidental reforging, unprotect it using \ref dip::Image::Protect.
///
/// `img` must be in one of the formats `QImage::Format_Grayscale8`, `QImage::Format_RGB888` or
/// `QImage::Format_RGBA8888`; convert `img` first (e.g. with `QImage::convertToFormat`) if it isn't. An empty
/// `QImage` produces a non-forged \ref dip::Image.
///
/// Note that `QImage::bits()` is non-const and detaches the `QImage` from any shared (implicitly copied) data,
/// this function calls it on a `const&`, requiring a `const_cast`; be aware that this can trigger a deep copy
/// inside `QImage` itself if its data was shared with another `QImage` object.
inline dip::Image QImageToDip( QImage const& img ) {
   if( img.isNull() ) {
      return {};
   }
   dip::uint channels;
   switch( img.format() ) {
      case QImage::Format_Grayscale8: channels = 1; break;
      case QImage::Format_RGB888: channels = 3; break;
      case QImage::Format_RGBA8888: channels = 4; break;
      default: DIP_THROW( "Unsupported QImage format, convert to Grayscale8, RGB888 or RGBA8888 first" );
   }
   dip::UnsignedArray sizes = { static_cast< dip::uint >( img.width() ), static_cast< dip::uint >( img.height() ) };
   dip::Tensor tensor( channels );
   // QImage's scanlines are `bytesPerLine()` bytes long, which can include padding for alignment; since our
   // sample type is always 1 byte (UINT8), this is directly the y-stride, in samples.
   dip::IntegerArray strides = {
         static_cast< dip::sint >( channels ),
         static_cast< dip::sint >( img.bytesPerLine() )
   };
   dip::sint tensorStride = 1;
   uchar* data = const_cast< uchar* >( img.bits() );
   dip::Image out( dip::NonOwnedRefToDataSegment( data ), data, dip::DT_UINT8, sizes, strides, tensor, tensorStride );
   out.Protect();
   return out;
}


/// \brief This class is the \ref dip::ExternalInterface for the *Qt* interface.
///
/// Use the following code when declaring images to be used as the output to a *DIPlib* function:
///
/// ```cpp
/// dip_qimage::ExternalInterface qtei;
/// dip::Image img_out = qtei.NewImage();
/// ```
///
/// This configures the image `img_out` such that, when it is forged later on, a `QImage` object will be created
/// to hold the pixel data. Only images with 1, 3 or 4 channels of type \ref dip::DT_UINT8 can be forged this way,
/// see \ref dip_qimage_limitations for details. For other images, the allocator will fail, prompting *DIPlib* to
/// use its own, default allocator instead. The resulting \ref dip::Image object cannot be converted to a `QImage`.
///
/// The \ref dip::ExternalInterface object owns the `QImage` objects. You need to keep it around as long as you use
/// the image objects returned by its \ref NewImage method, otherwise the data segments will be freed and the
/// \ref dip::Image objects will point to non-existing data segments.
///
/// To retrieve the `QImage` object inside such a \ref dip::Image, use the \ref GetQImage method:
///
/// ```cpp
/// QImage img = qtei.GetQImage( img_out );
/// ```
///
/// Remember to not assign a result into the image created with \ref NewImage, as the pixel data will be copied in
/// the assignment. Instead, use the *DIPlib* functions that take output images as function arguments:
///
/// ```cpp
/// img_out = in1 + in2;           // Bad! Incurs an unnecessary copy
/// dip::Add( in1, in2, img_out ); // Correct, the operation writes directly in the output data segment
/// ```
class ExternalInterface : public dip::ExternalInterface {
   private:
      // This map holds `QImage`s, we can find the right one if we have the data pointer.
      std::map< void const*, QImage > images_;

      class StripHandler {
         private:
            ExternalInterface& interface_;
         public:
            explicit StripHandler( ExternalInterface& ei ) : interface_( ei ) {}
            void operator()( void const* p ) const { interface_.images_.erase( p ); }
      };

   public:
      // This function overrides `dip::ExternalInterface::AllocateData`. It is called when an image with this
      // `ExternalInterface` is forged. It allocates a `QImage` and returns a `dip::DataSegment` to the data
      // pointer, with a custom deleter functor.
      //
      // A user will never call this function directly.
      dip::DataSegment AllocateData(
            void*& origin,
            dip::DataType datatype,
            dip::UnsignedArray const& sizes,
            dip::IntegerArray& strides,
            dip::Tensor const& tensor,
            dip::sint& tensorStride
      ) override {
         DIP_THROW_IF( datatype != dip::DT_UINT8, "QImage interface only supports UINT8 images" );
         DIP_THROW_IF( sizes.size() != 2, "QImage interface only supports 2D images" );
         dip::uint channels = tensor.Elements();
         QImage::Format format;
         DIP_STACK_TRACE_THIS( format = detail::GetQImageFormat( channels ) );
         QImage img( static_cast< int >( sizes[ 0 ] ), static_cast< int >( sizes[ 1 ] ), format );
         origin = img.bits();
         strides.resize( 2 );
         strides[ 0 ] = static_cast< dip::sint >( channels );
         strides[ 1 ] = static_cast< dip::sint >( img.bytesPerLine() );
         tensorStride = 1;
         images_.emplace( origin, std::move( img ) );
         return dip::DataSegment{ origin, StripHandler( *this ) };
      }

      /// \brief Returns the `QImage` that holds the data for the \ref dip::Image `img`.
      ///
      /// The `QImage` returned is the one allocated to hold the pixel data in the input `img`. If `img` is a view
      /// of another image, or has been manipulated such that it no longer matches the `QImage` it was allocated
      /// in, this function throws an exception; use \ref dip::Image::ForceNormalStrides beforehand if needed.
      QImage GetQImage( dip::Image const& img ) {
         DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
         auto it = images_.find( img.Data() );
         DIP_THROW_IF( it == images_.end(), "Image was not allocated by this interface" );
         QImage out = std::move( it->second );
         images_.erase( it );
         return out;
      }

      /// \brief Constructs a \ref dip::Image object with the external interface set so that, when forged, a
      /// `QImage` will be allocated to hold the samples.
      dip::Image NewImage() {
         dip::Image out;
         out.SetExternalInterface( this );
         return out;
      }
};


/// \endgroup

} // namespace dip_qimage

#endif // DIP_QIMAGE_INTERFACE_H
