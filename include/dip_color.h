/*
 * DIPlib 3.0
 * This file contains definitions for color image support.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_COLOR_H
#define DIP_COLOR_H

#include <array>

#include "dip_error.h"
#include "dip_types.h"


/// \file
/// Defines dip::ColorSpace and other support for color images.


namespace dip {


/// Specifies an image's color space and holds related information.
class ColorSpace {

   public:
      /// The default color space is none (i.e. grey-value image)
      ColorSpace() {}

      /// Any name can be used to create a color space.
      explicit ColorSpace( const String& name ) : name_( name ) {}

      /// Returns the color space name, or an empty string if the image is not a color image.
      const String& Name() const { return name_; }

      /// True if a color space name is set.
      bool IsColor() const { return !name_.empty(); }

      // TODO: This class needs some functionality that helps managing the `name` string and the `whitepoint` array.
      // Specifically, we need some way of associating a color space name to a number of channels, and to a
      // series of functions to convert to and from that color space.

      // TODO: We need to implement stuff to convert an image to a different color space.

   private:
      String name_;  // The color space name, if empty it's not a color image.
      //std::array< double, 9 > whitepoint_ = {{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
                     // The whitepoint XYZ array, used by some color conversion routines
};

} // namespace dip

#endif // DIP_COLOR_H
