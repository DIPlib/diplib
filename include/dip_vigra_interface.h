/*
 * (c)2019, Cris Luengo.
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


#ifndef DIP_VIGRA_INTERFACE_H
#define DIP_VIGRA_INTERFACE_H

#include "diplib.h"

#include <vigra/multi_array.hxx>


/// \file
/// \brief This file defines the \ref dip_vigra namespace, functionality to interface *Vigra* and *DIPlib*.


/// \group dip_vigra_interface *DIPlib*-*Vigra* interface
/// \ingroup interfaces
/// \brief Functions to convert images to and from *Vigra*.
///
/// The \ref dip_vigra namespace defines the functions needed to convert between *Vigra* `vigra::MultiArray` and
/// `vigra::MultiArrayView` objects and *DIPlib* \ref dip::Image objects.
///
/// The function \ref dip_vigra::VigraToDip encapsulates (maps) a *Vigra* image in a *DIPlib* image;
/// \ref dip_vigra::DipToVigra does the opposite, mapping a *DIPlib* image as a *Vigra* image.
///
/// *Vigra* supports anything as pixel type, but in order for the code written here to be manageable, these types
/// must be limited. Pixel types are therefore limited to the numeric types supported by *DIPlib* as well as vectors
/// of arbitrary length of these numeric types. *DIPlib* tensor images are mapped to *Vigra* vector images, but
/// the tensor shape is lost. Note that the tensor stride must always be 1. Use \ref dip::Image::ForceNormalStrides to
/// fix the tensor stride for mapping. Alternatively, use \ref dip_vigra::CopyDipToVigra.
///
/// *Vigra* seems to prefer to use the 8-bit unsigned integer type for binary images. These are always converted to
/// \ref dip::DT_UINT8 images, as the code here cannot distinguish between binary and non-binary images.
/// Use \ref dip::Convert to cast the resulting image to binary.
///
/// Because *Vigra* defines image properties through template parameters (data type and dimensionality), it is
/// not possible to write a non-templated function that creates a `vigra::MultiArray` object. Consequently, a
/// \ref dip::ExternalInterface would be very limited in usefulness, so we don't define one. The *DIPlib-Vigra*
/// interface is therefore less easy to use than, for example, the \ref dip_opencv_interface.
/// \addtogroup


/// \brief The `dip_vigra` namespace contains the interface between *Vigra* and *DIPlib*.
namespace dip_vigra {


namespace detail {

struct TemplateParams {
   dip::DataType dataType;
   dip::uint tensorElements = 1;
};

template< typename PixelType, typename = std::enable_if_t< dip::IsSampleType< PixelType >::value >>
inline TemplateParams GetTemplateParams( PixelType /**/ ) {
   return { dip::DataType( PixelType{} ), 1 };
}

template< typename PixelType, int tensorElements >
inline TemplateParams GetTemplateParams( vigra::TinyVector< PixelType, tensorElements > ) {
   return { dip::DataType( PixelType{} ), tensorElements };
}

} // namespace detail

/// \brief Creates a *DIPlib* image around a *Vigra* `vigra::MultiArrayView`, without taking ownership of the data.
///
/// This function maps a `vigra::MultiArrayView` object to a \ref dip::Image object.
/// The `dip::Image` object will point to the data in the `vigra::MultiArrayView`, which must continue existing until
/// the `dip::Image` is deleted or stripped. The output `dip::Image` is protected to prevent accidental reforging,
/// unprotect it using \ref dip::Image::Protect.
///
/// An invalid `vigra::MultiArrayView` produces a non-forged \ref dip::Image.
///
/// The template parameters do not need to be explicitly given, as the `input` object defines them.
template< unsigned int Dimensionality, class PixelType, class StrideTag >
inline dip::Image VigraToDip( vigra::MultiArrayView< Dimensionality, PixelType, StrideTag > const& input ) {
   if( !input.hasData() ) {
      return {};
   }
   detail::TemplateParams templateParams = detail::GetTemplateParams( PixelType{} );
   dip::DataType dt( PixelType );
   dip::UnsignedArray sizes( Dimensionality );
   dip::IntegerArray strides( Dimensionality );
   for( dip::uint ii = 0; ii < Dimensionality; ++ii ) {
      sizes[ ii ] = input.size( ii );
      strides[ ii ] = input.stride( ii ) * static_cast< dip::sint >( templateParams.tensorElements );
   }
   dip::Image img( dip::NonOwnedRefToDataSegment( input.data() ), input.data(), templateParams.dataType, sizes, strides, dip::Tensor( templateParams.tensorElements ), 1 );
   img.Protect();
   return img;
}


/// \brief Creates a *Vigra* `vigra::MultiArrayView` object around a *DIPlib* image, without taking ownership of the data.
///
/// This function maps a \ref dip::Image object to a `vigra::MultiArrayView` object.
/// The `vigra::MultiArrayView` object will point to the data in the `dip::Image`, which must continue existing until the
/// `vigra::MultiArrayView` is deleted.
///
/// A non-forged \ref dip::Image produces an invalid `vigra::MultiArrayView`.
///
/// Note that it is required to set the two template parameters, and that these template parameters must match the
/// the `dip::Image` object at runtime.
template< unsigned int Dimensionality, class PixelType >
inline vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag > DipToVigra( dip::Image const& img ) {
   using VigraView = typename vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag >;
   if( !img.IsForged() ) {
      return {};
   }
   DIP_THROW_IF( img.Dimensionality() != Dimensionality, dip::E::DIMENSIONALITIES_DONT_MATCH );
   detail::TemplateParams templateParams = detail::GetTemplateParams( PixelType{} );
   DIP_THROW_IF( img.TensorElements() != templateParams.tensorElements, dip::E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( img.DataType() != templateParams.dataType, dip::E::DATA_TYPES_DONT_MATCH );
   DIP_THROW_IF(( img.TensorElements() > 1 ) && ( img.TensorStride() != 1 ), "Vigra requires a tensor stride of 1" );
   typename VigraView::difference_type shape;
   typename VigraView::difference_type stride;
   for( dip::uint ii = 0; ii < Dimensionality; ++ii ) {
      shape[ ii ] = img.Size( ii );
      stride[ ii ] = img.Stride( ii ) / static_cast< dip::sint >( templateParams.tensorElements );
   }
   auto ptr = static_cast< typename VigraView::const_pointer >( img.Origin() );
   return VigraView( shape, stride, ptr );
}

/// \brief Creates a *Vigra* `vigra::MultiArrayView` object from a *DIPlib* image by copy.
///
/// A non-forged \ref dip::Image produces an invalid `vigra::MultiArrayView`.
///
/// Use this function if the \ref dip::Image cannot be mapped with \ref dip_vigra::DipToVigra, for example
/// if the data type doesn't match (or you don't know in advance what data type the *DIPlib* image
/// will have) or if the tensor stride is not 1.
///
/// Note that it is required to set the two template parameters, and that the `Dimensionality` must match
/// the `dip::Image` object at runtime. If the `PixelType` doesn't match, the sample values will be converted
/// in the same way that \ref dip::Image::Copy does.
template< unsigned int Dimensionality, class PixelType >
inline vigra::MultiArray< Dimensionality, PixelType > CopyDipToVigra( dip::Image const& img ) {
   if( !img.IsForged() ) {
      return {};
   }
   DIP_THROW_IF( img.Dimensionality() != Dimensionality, dip::E::DIMENSIONALITIES_DONT_MATCH );
   detail::TemplateParams templateParams = detail::GetTemplateParams( PixelType{} );
   DIP_THROW_IF( img.TensorElements() != templateParams.tensorElements, dip::E::NTENSORELEM_DONT_MATCH );
   typename vigra::MultiArray< Dimensionality, PixelType >::difference_type shape;
   for( dip::uint ii = 0; ii < Dimensionality; ++ii ) {
      shape[ ii ] = img.Size( ii );
   }
   vigra::MultiArray< Dimensionality, PixelType > output( shape );
   dip::Image tmp;
   DIP_STACK_TRACE_THIS( tmp = VigraToDip( output ));
   tmp.Copy( img );
   return output;
}

/// \endgroup

} // namespace dip_vigra

#endif // DIP_VIGRA_INTERFACE_H
