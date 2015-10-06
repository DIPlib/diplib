/*
 * DIPlib 3.0
 * This file contains definitions for support classes and functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#include "diplib.h"
#endif

#ifndef DIP_SUPPORT_H
#define DIP_SUPPORT_H

#include <string>    // std::string

namespace dip {

//
// Strings
//

typedef std::string String;               ///< A string type
typedef std::vector<String> StringArray;  ///< An array of strings

//
// Color spaces
//

/// Specifies an image's color space and holds related information.
class ColorSpace {
   public:
      String name;               // We use strings to specify color space.
      dfloat whitepoint[3][3];   // This will hold the whitepoint XYZ array.

      // constructors & destructor
      ColorSpace() {}
      explicit ColorSpace( const String& );
      explicit ColorSpace( const String&, const dfloat (&a) [3][3] );
};

//
// Physical dimensions
//

/// Specifies an image's pixel size in physical units.
class PhysicalDimensions {
   private:
      //StringArray spatial_value;
      //FloatArray spatial_size;
      //String intensity_unit;
      //dfloat intensity_value = 0;
   public:
      // constructors & destructor
      // getters
      // setters
      // other
            // Some static? methods to multiply and divide units
            // Some knowledge about standard units and unit conversions
};

} // namespace dip

#endif // DIP_SUPPORT_H
