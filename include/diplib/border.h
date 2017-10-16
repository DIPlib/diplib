#ifndef BORDER_H_INCLUDED
#define BORDER_H_INCLUDED

#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/library/types.h"
#include "diplib/neighborlist.h"
#include <queue>

namespace dip
{

/// Generic template to process the border/edges of an image.
/// Derived classes can override ProcessBorderPixel() to modify border pixels
/// and ProcessNonBorderPixel() for non-border pixels.
/// By default, no processing is done.
/// TODO: replace 'template< typename TPI > void dip__SetBorder( Image& out, Image::Pixel& c_value, dip::uint size )' with the functionality below (see generation/coordinates.cpp)
template< typename TPI >
class BorderProcessor
{
public:

   explicit BorderProcessor( Image& out, dip::uint borderWidth = 1 ) : out_( out ), borderWidth_( borderWidth ), tensorStride_(out.TensorStride()) {}

   // Process the border
   void operator()() {
      Process();
   }

   /// Process the border
   void Process() {
      dip::uint nDim = out_.Dimensionality();
      UnsignedArray const& sizes = out_.Sizes();

      // Prepare the value set to the border pixels
      PrepareBorderValues();

      // Iterate over all image lines, in the optimal processing dimension
      dip::uint procDim = Framework::OptimalProcessingDim( out_ );
      dip::sint stride = out_.Stride( procDim );
      dip::sint lastOffset = (static_cast<dip::sint>(out_.Size( procDim )) - 1) * stride;
      dip::sint tensorStride = out_.TensorStride();
      ImageIterator< TPI > it( out_, procDim );
      do {
         // Is this image line along the image border?
         bool all = false;
         UnsignedArray const& coord = it.Coordinates();
         for( dip::uint ii = 0; ii < nDim; ++ii ) {
            if( (ii != procDim) && ((coord[ii] < borderWidth_) || (coord[ii] >= sizes[ii] - borderWidth_)) ) {
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
         }
         else {
            // No, it isn't: fill only the first `borderWidth_` and last `borderWidth_` pixels
            TPI* firstPtr = it.Pointer();
            TPI* lastPtr = firstPtr + lastOffset;
            for( dip::uint ii = 0; ii < borderWidth_; ++ii, firstPtr += stride, lastPtr -= stride ) {
               ProcessBorderPixel( firstPtr );
               ProcessBorderPixel( lastPtr );
            }
            // Process non-border pixels
            // TODO: create template parameter (bool) that can disable this code altogether
            // if no non-border pixel processing is required?
            TPI* midPtr = it.Pointer() + borderWidth_ * stride;
            for( dip::uint ii = borderWidth_; ii < out_.Size( procDim ) - borderWidth_; ++ii, midPtr += stride ) {
               ProcessNonBorderPixel( midPtr );
            }
         }
      } while( ++it );
   }

protected:
   Image& out_;
   dip::uint borderWidth_;
   dip::sint tensorStride_;   // Cached for convenience

   /// Prepare value(s) that are to be copied to the border pixels
   virtual void PrepareBorderValues() {}
   /// Set the border pixel
   virtual void ProcessBorderPixel( TPI* pPixel ) {}
   /// Set the non-border pixel
   virtual void ProcessNonBorderPixel( TPI* pPixel ) {}
};

template< typename TPI >
class BorderSetter : public BorderProcessor< TPI >
{
public:
   explicit BorderSetter( Image& out, Image::Pixel const& borderPixel, dip::uint borderWidth = 1 ) : BorderProcessor( out, borderWidth ), borderPixel_( borderPixel ) {}

protected:
   Image::Pixel const& borderPixel_;
   std::vector< TPI > borderValues_;

   virtual void PrepareBorderValues() {
      dip::uint nTensor = out.TensorElements();
      // Copy c_value into an array with the right number of elements, and of the right data type
      border_value_.resize( nTensor, border_pixel_[0].As< TPI >() );
      if( !border_pixel_.IsScalar() ) {
         for( dip::uint ii = 1; ii < nTensor; ++ii ) {
            borderValues_[ii] = borderPixel_[ii].As< TPI >();
         }
      }
   }

   virtual void ProcessBorderSample( SampleIterator< TPI > sit ) {
      std::copy( borderValues_.begin(), borderValues_.end(), sit );
   }

};

}; // namespace dip

#endif