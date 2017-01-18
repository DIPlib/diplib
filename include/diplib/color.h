/*
 * DIPlib 3.0
 * This file contains definitions for color image support.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#ifndef DIP_COLOR_H
#define DIP_COLOR_H

#include <array>
#include <vector>
#include <map>

#include "diplib.h"


/// \file
/// \brief Defines `dip::ColorSpaceManager`, providing support for color images.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief An object to encapsulate the white point array.
///
/// It defines how R, G and B are to be combined to form X, Y and Z:
///
/// ```cpp
///     XYZ = WhitePoint * RGB
///     RGB = inv(WhitePoint) * XYZ
/// ```
class WhitePoint {

   public:

      /// \brief The default white point is the Standard Illuminant D65.
      WhitePoint() :
            matrix_{ { 0.412453, 0.212671, 0.019334, 0.357580, 0.715160, 0.119193, 0.180423, 0.072169, 0.950227 } } {};
      /// \brief Any 3x3 array can be used as white point (column-major).
      WhitePoint( std::array< double, 9 > m ) {
         std::copy( m.begin(), m.end(), matrix_.begin() );
      }

      /// \brief Get the 3x3 white point array, for conversion from RGB to XYZ.
      std::array< double, 9 > const& Matrix() { return matrix_; }

      /// \brief Get the inverse of the 3x3 white point array, for conversion from XYZ to RGB.
      std::array< double, 9 > InverseMatrix();

   private:

      std::array< double, 9 > matrix_;
};

// Prototype function for conversion between two color spaces.
// TODO: This should probably be an abstract base class where converters derive from, with a Convert() function
// (and maybe also a Cost() function?)
using ColorSpaceConverter = void ( * )(
      double const* input,    // pointer to input data, holding a known number of elements
      double* output,         // pointer to output data, holding a known number of elements
      double const* matrix    // pointer to the whitepoint array, its inverse, or any other relevant array
);

/// \brief An object of this class is used to convert images between color spaces.
///
/// By default, the object will known a set of color spaces, and be able to convert between
/// them. It is possible to define new color spaces, and register conversion functions that
/// translate from one color space to another. The object is capable of finding optimal
/// paths, defined by these conversion functions, to convert between color spaces. Thus,
/// it is not necessary to create functions that translate from your new color space to
/// all known color spaces, it is sufficient to register two function that translate to
/// and from your new color space to any existing color space.
///
/// ```cpp
///     dip::ColorSpaceManager csm;
///     dip::Image img = ...
///     csm.Set( img, "RGB" );                      // img is RGB
///     csm.Convert( img, img, "Lab" );             // img will be converted to Lab
///
///     csm.Define( "Frank", 4 );                   // A new color space with 4 channels
///     csm.DefineAlias( "f", "Frank" );            // "f" is an alias for "Frank"
///     csm.Register( frank2xyz, "f", "XYZ", 2 );   // a function that converts from Frank to XYZ
///     csm.Register( yxy2frank, "Yxy", "f", 3 );   // a function that converts from Yxy to Frank
///     csm.Convert( img, "f" );                    // img will be converted from Lab to Frank
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
// Also known: Piet's color spaces: art and LCh (or L*C*h*). What to do with those? Are they even published?
class ColorSpaceManager {

   public:

      /// \brief Constructor, registers the default color spaces.
      ColorSpaceManager();

      /// \brief Defines a new color space, that requires `chans` channels.
      void Define( String const& name, dip::uint chans ) {
         DIP_THROW_IF( IsDefined( name ), "Color space name already defined." );
         nodes_.emplace_back( name, chans );
         names_[ name ] = nodes_.size() - 1;
      }

      /// \brief Defines an alias for a defined color space name.
      void DefineAlias( String const& alias, String const& name ) {
         DIP_THROW_IF( IsDefined( alias ), "Alias name already defined." );
         names_[ alias ] = Index( name );
      }

      /// \brief Registers a function to translate from one color space to another.
      ///
      /// The conversion function converts a single pixel, and has the following
      /// signature:
      ///
      /// ```cpp
      ///     void ColorSpaceConverter(
      ///         double const*  input,
      ///         double*        output,
      ///         double const*  matrix);
      /// ```
      ///
      /// `input` is a pointer to a set of sample values composing the pixel, and
      /// `output` is where the result of the conversion is to be placed. Both
      /// arrays have a number of values corresponding to the channels used by
      /// the corresponding color space.
      void Register( ColorSpaceConverter func, String const& source, String const& destination, dip::uint cost = 1 ) {
         nodes_[ Index( source ) ].edges[ Index( destination ) ] = { func, cost }; // updates the edge if it was already there
      }

      /// \brief Returns the number of channels used by the given color space.
      dip::uint NumberOfChannels( String const& name ) const {
         return nodes_[ Index( name ) ].chans;
      }

      /// \brief Returns the cannonical name for the given color space (i.e. looks up name aliases).
      const String& CannonicalName( String const& name ) const {
         return nodes_[ Index( name ) ].name;
      }

      /// \brief Sets the color space of a non-color image.
      ///
      /// The image must have the right
      /// number of channels for the given color space, and the color space name
      /// must be one of the color spaces known by the ColorSpaceManager object.
      void Set( Image& in, String const& name ) const;

      /// \brief Converts an image to a different color space.
      ///
      /// Both the source and destination
      /// color spaces must be known, and a path of registered conversion functions
      /// must exist between the two.
      void Convert( Image const& in, Image const& out, String const& name, WhitePoint const& whitepoint ) const;

      Image Convert( Image const& in, String const& name, WhitePoint const& whitepoint ) const {
         Image out;
         Convert( in, out, name, whitepoint );
         return out;
      }

   private:

      struct Edge {
         ColorSpaceConverter func;
         dip::uint cost;
      };

      struct Node {
         String name;
         dip::uint chans;
         std::map< dip::uint, Edge > edges;  // The key is the target color space index
         Node( String const& name, dip::uint chans ) :
               name( name ), chans( chans ) {}
      };

      std::map< String, dip::uint > names_;
      std::vector< Node > nodes_;

      dip::uint Index( String const& name ) const {
         auto it = names_.find( name );
         DIP_THROW_IF( it == names_.end(), "Color space name not defined." );
         return it->second;
      }

      bool IsDefined( String const& name ) const {
         return names_.count( name ) != 0;
      }

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
      std::vector< dip::uint > FindPath( dip::uint start, dip::uint stop ) const;
};

/// \}

} // namespace dip

#endif // DIP_COLOR_H
