/*
 * DIPlib 3.0
 * This file contains definitions for support classes and functions.
 *
 * (c)2014-2015, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

// This file is included through diplib.h
#ifndef DIPLIB_H
#error "Please don't include this file directly, include diplib.h instead."
#endif

#ifndef DIP_SUPPORT_H
#define DIP_SUPPORT_H

#include <string>    // std::string

/// The dip namespace contains all the library functionality.
namespace dip {

//
// String
//

typedef std::string String;               ///< A string type
typedef std::vector<String> StringArray;  ///< An array of strings


//
// Range
//

/// Used in indexing to indicate start, stop and step. Negative start
/// and stop values indicate offset from the end (-1 is the last pixel,
/// -2 the second to last, etc.). If the stop comes before the start,
/// the step is assumed to be negative. No sign is stored for the step.
/// If stop cannot be reached with the given step size, the last pixel
/// in the range will come earlier. That is, stop is never exceeded.
struct Range {
   dip::sint start;    ///< First pixel included in range
   dip::sint stop;     ///< Last pixel included in range
   dip::uint step;     ///< Step size when going from start to stop

   /// Create a range that indicates all pixels
   Range() : start{0}, stop{-1}, step{1} {}
   /// Create a range that indicates a single pixel
   Range(dip::sint i) : start{i}, stop{i}, step{1} {}
   /// Create a range that indicates all pixels between `i` and `j`
   Range(dip::sint i, dip::sint j) : start{i}, stop{j}, step{1} {}
   /// Create a range with all thee values set
   Range(dip::sint i, dip::sint j, dip::uint s) : start{i}, stop{j}, step{s} {}

   /// Modify a range so that negative values are assigned correct
   /// values according to the given size; throws if the range falls
   /// out of bounds.
   void Fix( dip::uint size ) {
      // Compute indices from end
      if( start<0 ) start += size;
      if( stop <0 ) stop  += size;
      // Check start and stop are within range
      dip_ThrowIf( (start<0)||(start>=size)||(stop<0)||(stop>=size),
               E::INDEX_OUT_OF_RANGE );
      // Compute stop given start and step
      //stop = start + ((stop-start)/step)*step;
   }

   /// Get the number of pixels addressed by the range (must be fixed first!).
   dip::uint Size() const {
      if( start > stop )
         return 1 + (start-stop)/step;
      else
         return 1 + (stop-start)/step;
   }

   /// Get the offset for the range (must be fixed first!).
   dip::uint Offset() const {
      return start;
   }

   /// Get the signed step size for the range (must be fixed first!).
   dip::sint Step() const {
      if( start > stop )
         return -step;
      else
         return step;
   }

};

typedef std::vector<Range> RangeArray;  ///< An array of ranges


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
