/*
 * DIPlib 3.0
 * This file contains definitions for color image support.
 *
 * (c)2016, Cris Luengo.
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

/*

/// \brief An object to encapsulate the white point array.
///
/// It defines how R, G and B are to be combined to form X, Y and Z:
///
/// ```
///     XYZ = WhitePoint * RGB
///     RGB = inv(WhitePoint) * XYZ
/// ```
class WhitePoint {

   public:

      using MatrixValues = std::array< dfloat, 9 >;

      /// \brief The default white point is the Standard Illuminant D65.
      WhitePoint() :
            matrix_{{ 0.412453, 0.212671, 0.019334, 0.357580, 0.715160, 0.119193, 0.180423, 0.072169, 0.950227 }} {};
      /// \brief Any 3x3 array can be used as white point (column-major).
      WhitePoint( MatrixValues m ) {
         std::copy( m.begin(), m.end(), matrix_.begin() );
      }

      /// \brief Get the 3x3 white point array, for conversion from RGB to XYZ.
      MatrixValues const& Matrix() { return matrix_; }

      /// \brief Get the inverse of the 3x3 white point array, for conversion from XYZ to RGB.
      MatrixValues InverseMatrix();

   private:

      MatrixValues matrix_;
};

*/

/// \brief Base class for conversion between two color spaces. Classes that convert between color spaces must
/// derive from this and overload all the pure virtual functions.
class ColorSpaceConverter {
   public:
      /// \brief Returns the source color space name.
      virtual String InputColorSpace() const = 0;

      /// \brief Returns the destination color space name.
      virtual String OutputColorSpace() const = 0;

      /// \brief Returns the cost of the conversion. This cost includes computational cost as well as precision loss.
      ///
      /// The cost is used to avoid pathways such as "RGB"->"grey"->"Lab" instead of "RGB"->"XYZ"->"Yxy"->"Lab".
      /// Conversion to grey therefor must always have a high cost. It is not necessary to define this method,
      /// the default implementation returns a cost of 1.
      virtual dip::uint Cost() const { return 1; }

      /// \brief This is the method that performs the conversion for one pixel.
      ///
      /// `input` and `output` point to buffers with the number of samples corresponding to the two color
      /// spaces, as determined by the `InputColorSpace` and `OutputColorSpace` method.
      // TODO: This will be a lot more efficient if we pass a whole image line at a time.
      virtual void Convert(
            dfloat const* input,    // pointer to input data, holding a known number of elements
            dfloat* output
      ) = 0;

      /// \brief The user can access this method through `dip::ColorSpaceManager::Configure`.
      /// It is not necessary to define this method.
      virtual void Configure( String const& parameter, dfloat value ) {
         DIP_THROW( "ColorSpaceConverter not configurable" );
      }

      virtual ~ColorSpaceConverter() {}
};

/// \brief A pointer to a color space conversion object
using ColorSpaceConverterPointer = std::unique_ptr< ColorSpaceConverter >;


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
///     csm.Register( frank2xyz );                  // an object that converts from Frank to XYZ
///     csm.Register( yxy2frank );                  // an object that converts from Yxy to Frank
///     csm.Convert( img, "f" );                    // img will be converted from Lab to Frank
/// ```
///
/// The color spaces known by default are:
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
// TODO: Also known: Piet's color spaces: art and LCh (or L*C*h*). What to do with those? Are they even published?
// TODO: Add Serra's HSI.
class ColorSpaceManager {

   public:

      /// \brief Constructor, registers the default color spaces.
      ColorSpaceManager();

      /// \brief Defines a new color space, that requires `nChannels` channels.
      void Define( String const& colorSpaceName, dip::uint nChannels ) {
         DIP_THROW_IF( IsDefined( colorSpaceName ), "Color space name already defined" );
         colorSpaces_.emplace_back( colorSpaceName, nChannels );
         names_[ colorSpaceName ] = colorSpaces_.size() - 1;
      }

      /// \brief Defines an alias for a defined color space name.
      void DefineAlias( String const& alias, String const& colorSpaceName ) {
         DIP_THROW_IF( IsDefined( alias ), "Alias name already defined" );
         names_[ alias ] = Index( colorSpaceName );
      }

      /// \brief Registers a function object to translate from one color space to another.
      void Register( ColorSpaceConverterPointer converter ) {
         dip::uint source = Index( converter->InputColorSpace() );
         dip::uint destination = Index( converter->OutputColorSpace() );
         auto& edges = colorSpaces_[ source ].edges;
         auto it = edges.find( destination );
         if( it == edges.end() ) {
            // emplace
            edges.emplace( destination, std::move( converter ));
         } else {
            // replace
            it->second = std::move( converter );
         }
      }

      /// \brief Returns the number of channels used by the given color space.
      dip::uint NumberOfChannels( String const& colorSpaceName ) const {
         return colorSpaces_[ Index( colorSpaceName ) ].chans;
      }

      /// \brief Returns the canonical name for the given color space (i.e. looks up name aliases).
      const String& CanonicalName( String const& colorSpaceName ) const {
         return colorSpaces_[ Index( colorSpaceName ) ].name;
      }

      /// \brief Converts an image to a different color space.
      ///
      /// Both the source (`in.ColorSpace`) and destination (`colorSpaceName`) color spaces must be known,
      /// and a path of registered conversion functions must exist between the two.
      ///
      /// Note that it is possible to assign an arbitrary string as a color space name in an image. Setting
      /// an image's color space property is always possible, and gives no guarantee that the image has the
      /// right number of tensor elements (color channels).
      ///
      /// When converting from one color channel to another, the input image is checked for number of color
      /// channels. If it doesn't match the number expected for its color space, an exception will be thrown.
      ///
      /// If the input color space name is an empty string, and the image is scalar, it will be assumed that
      /// the color space is "grey". If the image has the same number of color channels as expected for
      /// `colorSpaceName`, then the color space will be set to that name. Otherwise, an exception will be
      /// thrown.
      ///
      /// If `colorSpaceName` is an empty string, "grey" is assumed.
      void Convert( Image const& in, Image const& out, String const& colorSpaceName = "" ) const;
      Image Convert( Image const& in, String const& colorSpaceName = "" ) const {
         Image out;
         Convert( in, out, colorSpaceName );
         return out;
      }

   private:

      struct ColorSpace {
         String name;
         dip::uint chans;
         std::map< dip::uint, ColorSpaceConverterPointer > edges;  // The key is the target color space index
         ColorSpace( String const& name, dip::uint chans ) :
               name( name ), chans( chans ) {}
      };

      std::map< String, dip::uint > names_;
      std::vector< ColorSpace > colorSpaces_;

      dip::uint Index( String const& name ) const {
         auto it = names_.find( name );
         DIP_THROW_IF( it == names_.end(), "Color space name not defined" );
         return it->second;
      }

      bool IsDefined( String const& name ) const {
         return names_.count( name ) != 0;
      }

      // The std::map `names_` translates known color space names to an index into the `colorSpaces_` array.
      // This array index is how we refer to color spaces internally. Externally, we always use
      // names. This way, different `ColorSpaceManager` objects can be used interchangeably (as long
      // as they contain the given color space name).
      //
      // We construct a graph among the color spaces. Elements of `colorSpaces_` as the nodes, and their
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
