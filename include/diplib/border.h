/*
 * DIPlib 3.0
 * This file contains binary support functions.
 *
 * (c)2017, Erik Schuitema, Cris Luengo
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

#ifndef BORDER_H_INCLUDED
#define BORDER_H_INCLUDED

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"

namespace dip {

/// \brief Generic template to process the border/edges of an image.
///
/// Derived classes must override ProcessBorderPixel() to modify border pixels
/// and ProcessNonBorderPixel() for non-border pixels.
/// TODO: replace 'template< typename TPI > void dip__SetBorder( Image& out, Image::Pixel& c_value, dip::uint size )' with the functionality below (see generation/coordinates.cpp)
template< typename TPI, bool processInner >
class DIP_EXPORT BorderProcessor {
   public:

      explicit BorderProcessor( Image& out, dip::uint borderWidth = 1 ): out_( out ), borderWidth_( borderWidth ) {}

      // Process the border
      void operator()() {
         Process();
      }

      /// Process the border
      void Process() {
         dip::uint nDim = out_.Dimensionality();
         UnsignedArray const& sizes = out_.Sizes();

         // Iterate over all image lines, in the optimal processing dimension
         dip::uint procDim = Framework::OptimalProcessingDim( out_ );
         dip::sint stride = out_.Stride( procDim );
         dip::sint lastOffset = borderWidth_ > out_.Size( procDim )
                                ? 0
                                : static_cast< dip::sint >( out_.Size( procDim ) - borderWidth_ ) * stride;
         dip::uint innerLength = 2 * borderWidth_ > out_.Size( procDim ) ? 0 : out_.Size( procDim ) - 2 * borderWidth_;
         dip::sint tensorStride = out_.TensorStride();
         ImageIterator< TPI > it( out_, procDim );
         do {
            // Is this image line along the image border?
            bool all = false;
            UnsignedArray const& coord = it.Coordinates();
            for( dip::uint ii = 0; ii < nDim; ++ii ) {
               if(( ii != procDim ) &&
                  (( coord[ ii ] < borderWidth_ ) || ( coord[ ii ] >= sizes[ ii ] - borderWidth_ ))) {
                  all = true;
                  break;
               }
            }
            if( all ) {
               // Yes, it is: fill all pixels on the line
               LineIterator< TPI > lit = it.GetLineIterator();
               do {
                  ProcessBorderPixel( &*lit );
               } while( ++lit );
            } else {
               // No, it isn't: fill only the first `borderWidth_` and last `borderWidth_` pixels
               TPI* ptr = it.Pointer();
               for( dip::uint ii = 0; ii < borderWidth_; ++ii, ptr += stride ) {
                  ProcessBorderPixel( { ptr, tensorStride } );
               }
               // Process non-border pixels
               if( processInner ) {
                  for( dip::uint ii = 0; ii < innerLength; ++ii, ptr += stride ) {
                     ProcessNonBorderPixel( { ptr, tensorStride } );
                  }
               }
               ptr = it.Pointer() + lastOffset; // We reset the pointer here in case !processInner, and in case borderWidth_ is larger than the image size.
               for( dip::uint ii = 0; ii < borderWidth_; ++ii, ptr += stride ) {
                  ProcessBorderPixel( { ptr, tensorStride } );
               }
            }
         } while( ++it );
      }

   protected:
      Image& out_;
      dip::uint borderWidth_;

      /// Set the border pixel
      virtual void ProcessBorderPixel( SampleIterator< TPI > sit ) = 0; // Make this a pure virtual base class
      /// Set the non-border pixel
      virtual void ProcessNonBorderPixel( SampleIterator< TPI > sit ) { (void)sit; };
};

/// \brief Template to set border pixels to a given value.
template< typename TPI >
class DIP_EXPORT BorderSetter: public BorderProcessor< TPI, false > {
   public:
      explicit BorderSetter( Image& out, Image::Pixel const& borderPixel, dip::uint borderWidth = 1 ):
            BorderProcessor< TPI, false >( out, borderWidth ) {
         dip::uint nTensor = out.TensorElements();
         DIP_THROW_IF( !borderPixel.IsScalar() && borderPixel.TensorElements() != nTensor, E::NTENSORELEM_DONT_MATCH );
         // Copy c_value into an array with the right number of elements, and of the right data type
         borderValues_.resize( nTensor, borderPixel[ 0 ].As< TPI >() );
         if( !borderPixel.IsScalar() ) {
            for( dip::uint ii = 1; ii < nTensor; ++ii ) {
               borderValues_[ ii ] = borderPixel[ ii ].As< TPI >();
            }
         }
      }

   protected:
      std::vector< TPI > borderValues_;

      virtual void ProcessBorderPixel( SampleIterator< TPI > sit ) override {
         std::copy( borderValues_.begin(), borderValues_.end(), sit );
      }
};

}; // namespace dip

#endif
