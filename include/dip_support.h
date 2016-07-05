/*
 * DIPlib 3.0
 * This file contains definitions for support classes and functions.
 *
 * (c)2014-2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_SUPPORT_H
#define DIP_SUPPORT_H

#include "dip_error.h"
#include "dip_types.h"


/// \file
/// Defines several support classes. This file is always included through diplib.h.


namespace dip {


//
// Color spaces (should probably get a header of its own)
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

      bool IsColor() const {
         return false;        // TODO
      }
};

//
// Physical dimensions (should probably get a header of its own)
//

/// Specifies an image's pixel size in physical units.
class PhysicalDimensions {
   private:
      //StringArray units_;
      //FloatArray magnitude_;
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
