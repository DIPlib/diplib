/*
 * DIPlib 3.0
 * This file contains functionality for the Vigra interface.
 *
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

#include <map>
#include <utility>

#include "diplib.h"
#include "diplib/iterators.h"

#include <vigra/multi_array.hxx>


/// \file
/// \brief This file defines the `dip_vigra` namespace, functionality to interface *Vigra* and *DIPlib*.


/// \brief The `%dip_vigra` namespace contains the interface between *Vigra* and *DIPlib*.
namespace dip_vigra {

/// \defgroup dip_vigra_interface DIPlib-Vigra interface
/// \ingroup interfaces
/// \brief Functions to convert images to and from *Vigra*.
///
/// The `dip_vigra` namespace defines the functions needed to convert between *Vigra* `vigra::MultiArray` and
/// `vigra::MultiArrayView` objects and *DIPlib* `dip::Image` objects.
///
/// The function `dip_vigra::VigraToDip` encapsulates (maps) a *Vigra* image in a *DIPlib* image;
/// `dip_vigra::DipToVigra` does the opposite, mapping a *DIPlib* image as a *Vigra* image.
///
/// *Vigra* supports anything as pixel type, but in order for the code written here to be manageable, these types
/// must be limited. Pixel types are therefore limited to the numeric types supported by *DIPlib* as well as vectors
/// of arbitrary length of these numeric types. *DIPlib* tensor images are mapped to *Vigra* vector images, but
/// the tensor shape is lost. Note that the tensor stride must always be 1. Use `dip::Image::ForceNormalStrides` to
/// fix the tensor stride for mapping. Alternatively, use `dip_vigra::CopyDipToVigra`.
///
/// *Vigra* seems to prefer to use the 8-bit unsigned integer type for binary images. These are always converted to
/// `dip::DT_UINT8` images, as the code here cannot distinguish between binary and non-binary images. Use `dip::Convert`
/// to cast the resulting image to binary.
///
/// Because *Vigra* defines image properties through template parameters (data type and dimensionality), it is
/// not possible to write a non-templated function that creates a `vigra::MultiArray` object. Consequently, a
/// `dip::ExternalInterface` would be very limited in usefulness, so we don't define one. The *DIPlib-Vigra*
/// interface is therefore less easy to use than, for example, the \ref dip_opencv_interface.
/// \{

namespace detail {

struct TemplateParams {
   dip::DataType dataType;
   dip::uint tensorElements;
};

template< typename PixelType, typename = std::enable_if_t< dip::IsSampleType< PixelType >::value >>
inline TemplateParams GetTemplateParams( PixelType ) {
   return { dip::DataType( PixelType{} ), 1 };
}

template< typename PixelType, int tensorElements >
inline TemplateParams GetTemplateParams( vigra::TinyVector< PixelType, tensorElements > ) {
   return { dip::DataType( PixelType{} ), tensorElements };
}

} // namespace detail

/// \brief Creates a *DIPlib* image around a *Vigra* `vigra::MultiArrayView`, without taking ownership of the data.
///
/// This function maps a `vigra::MultiArrayView` object to a `dip::Image` object.
/// The `dip::Image` object will point to the data in the `vigra::MultiArrayView`, which must continue existing until
/// the `dip::Image` is deleted or `Strip`ped. The output `dip::Image` is protected to prevent accidental reforging,
/// unprotect it using `dip::Image::Protect`.
///
/// An invalid `vigra::MultiArrayView` produces a non-forged `dip::Image`.
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
/// This function maps a `dip::Image` object to a `vigra::MultiArrayView` object.
/// The `vigra::MultiArrayView` object will point to the data in the `dip::Image`, which must continue existing until the
/// `vigra::MultiArrayView` is deleted.
///
/// A non-forged `dip::Image` produces an invalid `vigra::MultiArrayView`.
template< unsigned int Dimensionality, class PixelType >
inline vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag > DipToVigra( dip::Image const& img ) {
   if( !img.IsForged() ) {
      return {};
   }
   DIP_THROW_IF( img.Dimensionality() != Dimensionality, dip::E::DIMENSIONALITIES_DONT_MATCH );
   detail::TemplateParams templateParams = detail::GetTemplateParams( PixelType{} );
   DIP_THROW_IF( img.TensorElements() != templateParams.tensorElements, dip::E::NTENSORELEM_DONT_MATCH );
   DIP_THROW_IF( img.DataType() != templateParams.dataType, dip::E::DATA_TYPES_DONT_MATCH );
   DIP_THROW_IF(( img.TensorElements() > 1 ) && ( img.TensorStride() != 1 ), "Vigra requires a tensor stride of 1" );
   typename vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag >::difference_type shape;
   typename vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag >::difference_type stride;
   for( dip::uint ii = 0; ii < Dimensionality; ++ii ) {
      shape[ ii ] = img.Size( ii );
      stride[ ii ] = img.Stride( ii ) / static_cast< dip::sint >( templateParams.tensorElements );
   }
   auto ptr = static_cast< typename vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag >::const_pointer >( img.Origin() );
   return vigra::MultiArrayView< Dimensionality, PixelType, vigra::StridedArrayTag >( shape, stride, ptr );
}

/// \brief Creates a *Vigra* `vigra::MultiArrayView` object from a *DIPlib* image by copy.
///
/// A non-forged `dip::Image` produces an invalid `vigra::MultiArrayView`.
///
/// Use this function if the `dip::Image` cannot be mapped with `dip_vigra::DipToVigra`, for example
/// if the data type doesn't match (or you don't know in advance what data type the *DIPlib* image
/// will have) or if the tensor stride is not 1.
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

/// \}

} // namespace dip_vigra

#endif // DIP_VIGRA_INTERFACE_H
