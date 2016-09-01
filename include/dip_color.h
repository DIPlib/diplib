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
#include <vector>
#include <map>

#include "dip_error.h"
#include "dip_types.h"


/// \file
/// Defines dip::ColorSpaceManager and dip::ColorSpace, providing support for
/// color images.


namespace dip {


class Image; // Forward declaration

// Prototype function for conversion between two color spaces.
typedef void (*ColorSpaceConverter) (
      const double*  input,      // pointer to input data, holding a known number of elements
      double*        output,     // pointer to output data, holding a known number of elements
      const double*  whitepoint  // pointer to the whitepoint array
      // TODO: some functions will need the inverse of the whitepoint array. How do we do that?
);

/// An object of this class is used to convert images between color spaces. By default,
/// the object will known a set of color spaces, and be able to convert between them.
/// It is possible to define new color spaces, and register conversion functions that
/// translate from one color space to another. The object is capable of finding optimal
/// paths, defined by these conversion functions, to convert between color spaces. Thus,
/// it is not necessary to create functions that translate from your new color space to
/// all known color spaces, it is sufficient to register two function that translate to
/// and from your new color space to any existing color space.
///
/// ```
/// dip::ColorSpaceManager csm;
/// dip::Image img = ...
/// csm.Set( img, "RGB" );                      // img is RGB
/// csm.Convert( img, img, "Lab" );             // img will be converted to Lab
///
/// csm.Define( "Frank", 4 );                   // A new color space with 4 channels
/// csm.DefineAlias( "f", "Frank" );            // "f" is an alias for "Frank"
/// csm.Register( frank2xyz, "f", "XYZ", 2 );   // a function that converts from Frank to XYZ
/// csm.Register( yxy2frank, "Yxy", "f", 3 );   // a function that converts from Yxy to Frank
/// csm.Convert( img, "f" );                    // img will be converted from Lab to Frank
/// ```
///
/// The known color spaces are:
/// * CMY
/// * CMYK
/// * grey (or gray)
/// * HCV
/// * HSV
/// * Lab (or L*a*b*, CIELAB)
/// * Luv (or L*u*v*, CIELUV)
/// * RGB
/// * nlRGB (or R'G'B')
/// * XYZ
/// * Yxy
///
/// Also known:
/// * art
/// * LCh (or L*C*h*)
class ColorSpaceManager {

   public:

      /// Constructor, registers the default color spaces.
      ColorSpaceManager(); // TODO

      /// Defines a new color space, that requires `chans` channels.
      void Define( String name, dip::uint chans ) {
         dip_ThrowIf( names_.count( name ) != 0, "Color space name already defined." );
         nodes_.emplace_back( name, chans );
         names_[name] = nodes_.size() - 1;
      }

      /// Defines an alias for a defined color space name.
      void DefineAlias( String alias, String name ) {
         dip_ThrowIf( names_.count( alias ) != 0, "Alias name already defined." );
         dip_ThrowIf( names_.count( name ) == 0, "Color space name not defined." );
         names_[alias] = names_[name];
      }

      /// Registers a function to translate from one color space to another.
      /// The conversion function converts a single pixel, and has the following
      /// signature:
      ///     void ColorSpaceConverter(
      ///         const double*  input,
      ///         double*        output,
      ///         const double*  whitepoint);
      /// `input` is a pointer to a set of sample values composing the pixel, and
      /// `output` is where the result of the conversion is to be placed. Both
      /// arrays have a number of values corresponding to the channels used by
      /// the corresponding color space.
      void Register( ColorSpaceConverter func, String source, String destination, dip::uint cost = 1 ) {
         dip_ThrowIf( names_.count( source ) == 0, "Source color space name not defined." );
         dip::uint index = names_[source];
         dip_ThrowIf( names_.count( destination ) == 0, "Destination color space name not defined." );
         dip::uint target = names_[destination];
         nodes_[index].edges[target] = { func, cost }; // updates the edge if it was already there
      }

      /// Returns the number of channels used by the given color space.
      dip::uint NumberOfChannels( String name ) const {
         auto it = names_.find( name );
         dip_ThrowIf( it == names_.end(), "Color space name not defined." );
         return nodes_[it->second].chans;
      }

      /// Returns the cannonical name for the given color space (i.e. looks up name aliases).
      String CannonicalName( String name ) const {
         auto it = names_.find( name );
         dip_ThrowIf( it == names_.end(), "Color space name not defined." );
         return nodes_[it->second].name;
      }

      /// Sets the color space of a non-color image. The image must have the right
      /// number of channels for the given color space, and the color space name
      /// must be one of the color spaces known by the ColorSpaceManager object.
      void Set( Image& in, String name ) const;

      /// Converts an image to a different color space. Both the source and destination
      /// color spaces must be known, and a path of registered conversion functions
      /// must exist between the two.
      void Convert( const Image& in, const Image& out, String name ) const;

      Image Convert( const Image& in, String name ) const;
      // { Image out; Convert( in, out, name ); return out; }

   private:

      struct Edge {
         ColorSpaceConverter func;
         dip::uint cost;
      };
      struct Node {
         String name;
         dip::uint chans;
         std::map< dip::uint, Edge > edges;  // The key is the target color space index
         Node( const String& name, dip::uint chans ):
            name( name ), chans( chans ) {}
      };

      std::map< String, dip::uint > names_;
      std::vector< Node > nodes_;

      // The std::map `names_` translates known color space names to an index into the `nodes_` array.
      // This array index is how we refer to color spaces internally. Externally, we always use
      // names. This way, different `ColorSpaceManager` objects can be used interchangeably (as long
      // as they contain the given color space name).
      //
      // We construct a graph among the color spaces. Elements of `nodes_` as the nodes, and their
      // `edges` element are outgoing edges. Through `FindPath()` it is possible to find an optimal
      // path from any source color space to any other destination color space (assuming there are
      // conversion functions defined that allow this). This path is a string of conversion functions
      // which, when called in succession, accomplish the color space conversion.

      // Find an optimal path between two color spaces, given by their indices.
      std::vector< dip::uint > FindPath( dip::uint start, dip::uint stop );
};

/// Specifies an image's color space and holds related information. The user should
/// not need to use this class directly.
/// \see dip::Image::ColorSpace, dip::Image::IsColor, dip::ColorSpaceManager
class ColorSpace {

   public:

      /// The default color space is none (i.e. grey-value image)
      ColorSpace() {}

      /// Returns the color space name, or an empty string if the image is not a color image.
      const String& Name() const { return name_; }

      /// True if a color space name is set.
      bool IsColor() const { return !name_.empty(); }

      // By making ColorSpaceManager a friend, it can access the private constructor.
      friend class dip::ColorSpaceManager;

      // TODO: figure out how to best use the whitepoint array (given that most images won't carry one).
      // TODO: add functions to set and read the whitepoint array.
      // Maybe the whitepoint won't be in here, but will be an optional argument to ColorSpaceManager::Convert.

   private:

      // The private constructor is only accessible by dip::ColorSpaceManager.
      explicit ColorSpace( const String& name ) : name_( name ) {}

      String name_;  // The color space name, if empty it's not a color image.
      //std::array< double, 9 > whitepoint_ = {{ 0, 0, 0, 0, 0, 0, 0, 0, 0 }};
                     // The whitepoint XYZ array, used by some color conversion routines
};

} // namespace dip

#endif // DIP_COLOR_H
