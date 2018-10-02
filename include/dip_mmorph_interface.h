/*
 * DIPlib 3.0
 * This file contains functionality for the MMorph interface.
 *
 * (c)2017, Flagship Biosciences. Written by Cris Luengo.
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


#ifndef DIP_MMORPH_INTERFACE_H
#define DIP_MMORPH_INTERFACE_H

#include <map>
#include <utility>

#include "diplib.h"
#include "diplib/iterators.h"

#include <morph4cpp.h>


/// \file
/// \brief This file defines the `dip_mmorph` namespace, functionality to interface *MMorph* and *DIPlib*.


/// \brief The `%dip_mmorph` namespace contains the interface between *MMorph* and *DIPlib*.
///
/// *MMorph* is the *[SDC Morphology Toolbox for C++](http://www.mmorph.com/)* (**note:** broken link, this library
/// has been discontinued).
///
/// This namespace defines the functions needed to convert between *MMorph* `::%Image` objects and *DIPlib*
/// `dip::Image` objects. Note that *MMorph* is pretty old code that does not use namespaces.
///
/// We define a class `dip_mmorph::ExternalInterface` so that output images from *DIPlib* can yield an *MMorph* image,
/// and we define a function `dip_mmorph::MmToDip` that encapsulates an *MMorph* image in a *DIPlib* image.
/// `dip_mmorph::DipToMm` copies a *DIPlib* image into a new *MMorph* image.
///
/// **Note** the difference between how *DIPlib* and *MMorph* represent binary images. For *DIPlib*, any non-zero
/// value is foreground, but foreground is always stored as a 1; some functions in `diplib/binary.h` will expect
/// other bits to be 0, as they use those bit planes for intermediate data. For *MMorph*, foreground is always
/// stored as 255, and some of its functions will expect foreground to be 255. `dip_mmorph::FixBinaryImageForDip`
/// and `dip_mmorph::FixBinaryImageForMm` fix up binary images for processing in either library.
namespace dip_mmorph {


/// \brief Creates a *DIPlib* image around an *MMorph* image, without taking ownership of the data.
///
/// This function "converts" an `::%Image` object to a `dip::Image` object.
/// The `dip::Image` object will point to the data in the `::%Image`, which must continue existing until the
/// `dip::Image` is deleted or `Strip`ped. The output `dip::Image` is protected to prevent accidental reforging,
/// unprotect it using `dip::Image::Protect`.
///
/// An empty `::%Image` produces a non-forged `dip::Image`.
///
/// The optional second input argument serves to force an `MM_INT` image to be `DT_UINT32`. The pixel values
/// are simply re-interpreted as unsigned integer. This is useful for the output of `mmLabel`, which is either
/// `MM_USHORT` or `MM_INT` and always contains only non-negative integers, given that *DIPlib* expects
/// labeled images to be unsigned.
inline dip::Image MmToDip( ::Image const& mm, bool forceUnsigned = false ) {
   // Find image properties
   if( mm.isnull() ) {
      return {};
   }
   dip::DataType dt;
   switch( mm.typecode() ) {
      case MM_BYTE: // not supported by C++ MMorph interface
         dt = dip::DT_SINT8;
         break;
      case MM_UBYTE:
         if( mm.isbinary() ) {
            dt = dip::DT_BIN;
         } else {
            dt = dip::DT_UINT8;
         }
         break;
      case MM_SHORT: // not supported by C++ MMorph interface
         dt = dip::DT_SINT16;
         break;
      case MM_USHORT:
         dt = dip::DT_UINT16;
         break;
      case MM_INT:
         dt = forceUnsigned ? dip::DT_UINT32 : dip::DT_SINT32;
         break;
      case MM_UINT: // not supported by C++ MMorph interface
         dt = dip::DT_UINT32;
         break;
      case MM_FLOAT: // not supported by C++ MMorph interface
         dt = dip::DT_SFLOAT;
         break;
      case MM_DOUBLE: // not supported by C++ MMorph interface
         dt = dip::DT_DFLOAT;
         break;
      default:
         DIP_THROW( "MMorph image with unknown type code" );
   }
   dip::UnsignedArray sizes = { static_cast< dip::uint >( mm.width() ), static_cast< dip::uint >( mm.height() ) };
   dip::Tensor tensor; // scalar by default
   // Define proper strides
   dip::IntegerArray strides = { 1, mm.width() };
   dip::sint tstride = 1;
   if( mm.depth() > 3 ) { // NOTE! This is an arbitrary threshold
      sizes.push_back( static_cast< dip::uint >( mm.depth() ));
      strides.push_back( mm.width() * mm.height() );
   } else if( mm.depth() > 1 ) {
      tensor = dip::Tensor( static_cast< dip::uint >( mm.depth() ));
      tstride = mm.width() * mm.height();
   }
   // Create Image object
   dip::Image out( dip::NonOwnedRefToDataSegment( &mm ), mm.raster(), dt, sizes, strides, tensor, tstride );
   out.Protect();
   return out;
}

/// \brief Fixes the binary image `img` to match expectations of *DIPlib* (i.e. only the bottom bit is used).
inline void FixBinaryImageForDip( dip::Image& img ) {
   dip::ImageIterator< dip::bin >it( img ); // throws if input image is not binary, or not forged.
   do {
      if( *it ) {
         *it = true; // Note that `*it = *it` does not do this. `*it = !!*it` might work.
      }
   } while( ++it );
}

/// \brief Fixes the binary image `img` to match expectations of *MMorph* (i.e. all bits have the same value).
inline void FixBinaryImageForMm( dip::Image& img ) {
   dip::ImageIterator< dip::bin >it( img ); // throws if input image is not binary, or not forged.
   do {
      if( *it ) {
         static_cast< dip::uint8& >( *it ) = 255;
      }
   } while( ++it );
}

inline std::pair< dip::UnsignedArray, char const* > GetMmImageProperties(
        dip::DataType datatype,
        dip::UnsignedArray const& sizes,
        dip::uint tensorElements
) {
    dip::UnsignedArray mmSizes = sizes;
    mmSizes.resize( 3 );
    dip::uint ndims = sizes.size();
   if( ndims == 3 ) {
      DIP_THROW_IF( tensorElements != 1, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
      //mmSizes[ 2 ] = sizes[ 2 ];
   } else if( ndims == 2 ) {
      mmSizes[ 2 ] = tensorElements;
   } else {
      DIP_THROW( dip::E::DIMENSIONALITY_NOT_SUPPORTED );
   }
   char const* typestr;
   switch( datatype ) {
      case dip::DT_BIN:
         typestr = "binary";
           break;
      case dip::DT_UINT8:
         typestr = "uint8";
           break;
      case dip::DT_UINT16:
         typestr = "uint16";
           break;
      case dip::DT_SINT32:
         typestr = "int32";
           break;
      default:
         DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED );
   }
   return std::make_pair( mmSizes, typestr );
}


/// \brief Copies a *DIPlib* image to an *MMorph* image.
inline ::Image DipToMm( dip::Image const& img ) {
   dip::UnsignedArray mmSizes;
   char const* typestr;
   DIP_STACK_TRACE_THIS( std::tie( mmSizes, typestr ) = GetMmImageProperties( img.DataType(), img.Sizes(), img.TensorElements() ));
   ::Image mmImg( static_cast< int >( mmSizes[ 0 ] ), static_cast< int >( mmSizes[ 1 ] ),
                  static_cast< int >( mmSizes[ 2 ] ), typestr, 0.0 );
   dip::Image ref = MmToDip( mmImg );
   ref.Copy( img );
   if( mmImg.isbinary() ) {
      FixBinaryImageForMm( ref );
   }
   return mmImg;
}


/// \brief A unique pointer to an *MMorph* image. We cannot allocate these on the stack within the interface.
using ImagePtr = std::unique_ptr< ::Image >;


/// \brief This class is the `dip::ExternalInterface` for the *MMorph* interface.
///
/// Use the following code when declaring images to be used as the output to a function:
/// ```cpp
///     dip_mmorph::ExternalInterface mmei;
///     dip::Image img_out0 = mmei.NewImage();
///     dip::Image img_out1 = mmei.NewImage();
/// ```
/// This configures the images `img_out0` and `img_out1` such that, when they are
/// forged later on, an `::%Image` object will be created to hold the pixel data.
///
/// The `%ExternalInterface` object owns the `::%Image` objects. You need to keep it
/// around as long as you use the image objects returned by its `NewImage` method,
/// otherwise the data segments will be freed and the `dip::Image` objects will point
/// to non-existing data segments.
///
/// To retrieve the `::%Image` object inside such a `dip::Image`, use the
/// `dip_mmorph::ExternalInterface::DipToMm` method:
/// ```cpp
///     dip_mmorph::ImagePtr img0 = mmei.DipToMm( img_out0 );
///     dip_mmorph::ImagePtr img1 = mmei.DipToMm( img_out1 );
///     mmEro( *img0 ); // you need to dereference the pointer to the image...
/// ```
/// If you don't use the `DipToMm` method, the `::%Image` that contains the pixel data
/// will be destroyed when the dip::Image object goes out of scope. The `DipToMm` method
/// changes ownership of the `::%Image` object from the `%ExternalInterface` to the
/// `dip_mmorph::ImagePtr` object returned. In this case, the `dip::Image` object is still
/// valid, and shares the data segment with the extracted `::%Image`. If the
/// `dip_mmorph::ImagePtr` is destroyed, the data segment will be freed and the `dip::Image`
/// object will point to a non-existing data segment.
///
/// Remember to not assign a result into the images created with `NewImage`,
/// as the pixel data will be copied in the assignment.
/// Instead, use the *DIPlib* functions that take output images as function
/// arguments:
/// ```cpp
///     img_out0 = in1 + in2;           // Bad! Incurs an unnecessary copy
///     dip::Add( in1, in2, img_out0 ); // Correct, the operation writes directly in the output data segment
/// ```
/// In the first case, `in1 + in2` is computed into a temporary image, whose
/// pixels are then copied into the `::%Image` created for `img_out0`. In the
/// second case, the result of the operation is directly written into the
/// `::%Image`, no copies are necessary.
class ExternalInterface : public dip::ExternalInterface {
   private:
      // This map holds ::Images, we can find the right one if we have the data pointer.
      std::map< void const*, ImagePtr > images_;
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
      // This function overrides `dip::ExternalInterface::AllocateData`.
      // It is called when an image with this `ExternalInterface` is forged.
      // It allocates a *MMorph* `::Image` and returns a `dip::DataSegment` to the
      // data pointer, with a custom deleter functor. Strides are forced to
      // the only option available in *MMorph*, and an exception is thrown if
      // the data type or dimensionality is not supported.
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
         dip::UnsignedArray mmSizes;
         char const* typestr;
         DIP_STACK_TRACE_THIS( std::tie( mmSizes, typestr ) = GetMmImageProperties( datatype, sizes, tensor.Elements() ));
         dip::uint ndims = sizes.size();
         strides.resize( ndims );
         strides[ 0 ] = 1;
         strides[ 1 ] = sizes[ 0 ];
         if( ndims == 3 ) {
            strides[ 2 ] = sizes[ 0 ] * sizes[ 1 ];
            tstride = 1;
         } else {
            tstride = sizes[ 0 ] * sizes[ 1 ]; // tensor dimension is last ('depth')
         }
         // Create ::Image
         ImagePtr mm = std::make_unique< ::Image >( static_cast< int >( mmSizes[ 0 ] ), static_cast< int >( mmSizes[ 1 ] ),
                                                    static_cast< int >( mmSizes[ 2 ] ), typestr, 0.0 );
         origin = mm->raster();
         images_.emplace( origin, std::move( mm ));
         return dip::DataSegment{ origin, StripHandler( *this ) };
      }

      /// \brief Returns the MMorph `::%Image` that holds the data for the `dip::Image` `img`.
      ///
      /// This function transfers ownership of the `::%Image` object to the `dip_mmorph::ImagePtr`
      /// output object. Dereference the output object to access the *MMorph* image.
      ///
      /// The *MMorph* `::%Image` returned is the one allocated to hold the pixel data in the input
      /// `img`. If `img` is a view of another image, the output will be the full image, not only
      /// the view. If `img` was obtained by indexing, or contains permuted and/or mirrored dimensions,
      /// then you need to make a copy first:
      /// ```cpp
      ///     dip_mmorph::ExternalInterface mm;
      ///     dip::Image img = mm.NewImage();
      ///     img.ReForge( { 256, 256 }, 1, dip::DT_UINT8 );
      ///     img = img.At( { 0, 128 }, { 0, 128 } ); // `img` is a view of the original image
      ///     img.Mirror( { true, false } );          // `img` has a mirrored dimension
      ///     // Make a copy, then extract the *MMorph* image:
      ///     dip::Image tmp = mm.NewImage();
      ///     tmp.Copy( img );                        // `tmp` now is a new image where the pixels have been copied from `img`
      ///     dip_mmorph::ImagePtr pImgMm = mm.DipToMm( tmp );
      /// ```
      ImagePtr DipToMm( dip::Image const& img ) {
         DIP_THROW_IF( !img.IsForged(), dip::E::IMAGE_NOT_FORGED );
         // TODO: We should check for strides exactly matching image dimensions, and make a copy if not a match.
         void const* ptr = img.Data();
         ImagePtr out = std::move( images_[ ptr ]);
         DIP_THROW_IF( !out, "The image was not present in the dip_mmorph::ExternalInterface" );
         images_.erase( ptr );
         // If the image is binary, we need to convert non-zero values to 255, as they are expected in MMorph.
         // DIPlib always writes 1 for true.
         if( out->isbinary() ) {
            long N = out->width() * out->height() * out->depth();
            dip::uint8* mptr = (dip::uint8*) out->raster();
            for( long ii = 0; ii < N; ++ii, ++mptr ) {
               if( *mptr ) {
                  *mptr = 255;
               }
            }
         }
         return out;
      }

      /// \brief Constructs a `dip::Image` object with the external interface set so that,
      /// when forged, a *MMorph* `::%Image` will be allocated to hold the samples.
      dip::Image NewImage() {
         dip::Image out;
         out.SetExternalInterface( this );
         return out;
      }
};


} // namespace dip_mmorph

#endif // DIP_MMORPH_INTERFACE_H
