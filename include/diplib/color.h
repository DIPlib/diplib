/*
 * DIPlib 3.0
 * This file contains definitions for color image support.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPimage code: (c)2014, Cris Luengo;
 *                                  (c)1999-2014, Delft University of Technology.
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

#include <map>

#include "diplib.h"
#include "diplib/iterators.h"


/// \file
/// \brief Color space conversions
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{

/// \brief Base class for conversion between two color spaces.
///
/// Classes that convert between color spaces must derive from this and overload all the pure virtual functions.
/// See `dip::ColorSpaceManager` for how to use these converters.
class DIP_CLASS_EXPORT ColorSpaceConverter {
   public:
      /// \brief Returns the source color space name.
      virtual String InputColorSpace() const = 0;

      /// \brief Returns the destination color space name.
      virtual String OutputColorSpace() const = 0;

      /// \brief Returns the cost of the conversion. This cost includes computational cost as well as precision loss.
      ///
      /// The cost is used to avoid pathways such as "RGB" &rarr; "grey" &rarr; "Lab" instead of "RGB" &rarr; "XYZ" &rarr; "Yxy" &rarr; "Lab".
      /// Conversion to grey therefore must always have a high cost. It is not necessary to define this method,
      /// the default implementation returns a cost of 1.
      virtual dip::uint Cost() const { return 1; }

      /// \brief This is the method that performs the conversion for one image line.
      ///
      /// `input` and `output` point to buffers with the number of tensor elements expected for the two color
      /// spaces, as determined by the `InputColorSpace` and `OutputColorSpace` method.
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const = 0;

      virtual ~ColorSpaceConverter() = default;
};


/// \brief An object of this class is used to convert images between color spaces.
///
/// By default, the object will known a set of color spaces, and be able to convert between
/// them. It is possible to define new color spaces, and register conversion functions that
/// translate from one color space to another. The object is capable of finding optimal
/// paths, defined by these conversion functions, to convert between color spaces. Thus,
/// it is not necessary to create functions that translate from your new color space to
/// all known color spaces, it is sufficient to register two function that translate to
/// and from your new color space to any existing color space:
///
/// ```cpp
///     dip::ColorSpaceManager csm;
///     dip::Image img = ...
///     csm.Set( img, "RGB" );                      // img is RGB
///     img = csm.Convert( img, "Lab" );            // img will be converted to Lab
///
///     csm.Define( "Frank", 4 );                   // A new color space with 4 channels
///     csm.DefineAlias( "f", "Frank" );            // "f" is an alias for "Frank"
///     csm.Register( new frank2xyz );              // an object that converts from Frank to XYZ
///     csm.Register( new yxy2frank );              // an object that converts from Yxy to Frank
///     img = csm.Convert( img, "f" );              // img will be converted from Lab to Frank
/// ```
///
/// In the code snippet above, `frank2xyz` and `yxy2frank` are objects derived from `dip::ColorSpaceConverter`.
///
/// The color spaces known by default are given in the table below. Color space names are case-sensitive,
/// but aliases are registered for all these names using all-lowercase.
///
/// Name     | Aliases    | Description
/// -------- | ---------- | ------------
/// `"grey"` | `"gray"`   | An empty string is also interpreted as grey. Defined to be in the range [0,255].
/// `"RGB"`  |            | Linear RGB, defined in the range [0,255].
/// `"sRGB"` |            | Industry-standard non-linear RGB, gamma-corrected linear RGB (average gamma is approximately 2.2, with a linear segment near 0). Values in the range is [0,255].
/// `"CMY"`  |            | Cyan-Magenta-Yellow. Subtractive colors, defined simply as 255-RGB. Values in the range is [0,255].
/// `"CMYK"` |            | Cyan-Magenta-Yellow-blacK. Subtractive colors with black added. Note that printers need a more complex mapping to CMYK to work correctly.
/// `"HSI"`  |            | Hue-Saturation-Intensity. \f$L^1\f$ norm polar decomposition of the RGB cube, more suited to image analysis than HSV or HCV. S and I are in the range [0,255], H is an angle in degrees. Defined by Hanbury and Serra, "Colour image analysis in 3d-polar coordinates", Joint Pattern Recognition Symposium, 2003.
/// `"ICH"`  |            | Intensity-Chroma-Hue. Rotation of the RGB cube, where I is along the black-white diagonal of the cube, and the CH-plane is perpendicular. I is in the range [0,255], H is an angle in degrees.
/// `"ISH"`  |            | Intensity-Saturation-Hue. Based in ICH, where S is the C channel normalized so that the maximum saturation for each H is 1. For each H, the largest value of C is attained for a different value of I.
/// `"HCV"`  |            | Hue-Chroma-Value. V is the max of R, G and B, and C is the difference between largest and smallest RGB intensities. C and V are in range [0,255], H is an angle in degrees.
/// `"HSV"`  |            | Hue-Saturation-Value. Based on HCV, where S is equal to C normalized by V. S is in range [0,1] and V in range [0,255], H is an angle in degrees.
/// `"XYZ"`  |            | CIE 1931 XYZ, standard observer tristimulus values. A rotation of the RGB cube that aligns Y with the luminance axis.
/// `"Yxy"`  |            | CIE Yxy, where x and y are normalized X and Y. They are the chromacity coordinates.
/// `"Lab"`  | `"L*a*b*"`, `"CIELAB"` | Lightness and two chromacity coordinates. A color space that is much closer to being perceptually uniform than Yxy.
/// `"Luv"`  | `"L*u*v*"`, `"CIELUV"` | Lightness and two chromacity coordinates. An alternative to CIE Lab.
/// `"LCH"`  | `"L*C*H*"` | Lightness-Chroma-Hue. Computed from Lab, where C and H are the polar coordinates to a and b. H is an angle in degrees.
//
// TODO: Also known: Piet's color space: art. What to do with this? Is it even published?
class DIP_NO_EXPORT ColorSpaceManager {
      using ColorSpaceConverterPointer = std::shared_ptr< ColorSpaceConverter >; // TODO: MSVC does not like us using a unique_ptr here, which is really what we want to do.

   public:
      /// \brief Constructor, registers the default color spaces.
      DIP_EXPORT ColorSpaceManager();

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

      /// \brief Registers a function object to translate from one color space to another. The
      /// `%dip::ColorSpaceManager` object takes ownership of the converter.
      void Register( ColorSpaceConverter* converter ) {
         auto smartpointer = ColorSpaceConverterPointer( converter );
         dip::uint source = Index( converter->InputColorSpace() );
         dip::uint destination = Index( converter->OutputColorSpace() );
         auto& edges = colorSpaces_[ source ].edges;
         auto it = edges.find( destination );
         if( it == edges.end() ) {
            // emplace
            edges.emplace( destination, std::move( smartpointer ));
         } else {
            // replace
            it->second = std::move( smartpointer );
         }
      }

      /// \brief Check to see if a color space name is defined.
      bool IsDefined( String const& colorSpaceName ) const {
         return names_.count( colorSpaceName ) != 0;
      }

      /// \brief Gets a pointer to a color space converter object registered with this `%ColorSpaceManager`.
      /// Use this to access the object to modify it, for example configure a parameter.
      ColorSpaceConverter* GetColorSpaceConverter(
            String const& inputColorSpaceName,
            String const& outputColorSpaceName
      ) const {
         dip::uint source = Index( inputColorSpaceName );
         dip::uint destination = Index( outputColorSpaceName );
         auto& edges = colorSpaces_[ source ].edges;
         auto it = edges.find( destination );
         DIP_THROW_IF( it == edges.end(), "Converter function not registered" );
         return it->second.get();
      }

      /// \brief Returns the number of channels used by the given color space.
      dip::uint NumberOfChannels( String const& colorSpaceName ) const {
         return colorSpaces_[ Index( colorSpaceName ) ].nChannels;
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
      ///
      /// The output image is of a floating-point type. If you want the output to be written in, for example,
      /// an 8-bit unsigned integer, you can use the "protect" flag on the output image:
      ///
      /// ```cpp
      ///     dip::Image in = ...; // read in a color image
      ///     dip::Image out;
      ///     out.SetDataType( dip::DT::UINT8 );
      ///     out.Protect();
      ///     dip::ColorSpaceManager csm;
      ///     cms.Convert( in, out, "HSV" );
      /// ```
      ///
      /// In this case, all computations are still performed as double-precision floating-point computations,
      /// but the result is cast to 8-bit unsigned integers when written to the output image. Some color spaces,
      /// such as RGB and CMYK are defined to use the [0,255] range of 8-bit unsigned integers. Other color spaces
      /// such as Lab and XYZ are not. For those color spaces, casting to an integer will destroy the data.
      DIP_EXPORT void Convert( Image const& in, Image& out, String const& colorSpaceName = "" ) const;
      Image Convert( Image const& in, String const& colorSpaceName = "" ) const {
         Image out;
         Convert( in, out, colorSpaceName );
         return out;
      }

      /// \brief The white point, as an XYZ triplet.
      ///
      /// The default white point is the Standard Illuminant D65. Configure the `dip::ColorSpaceManager`
      /// object through its `SetWhitePoint` method.
      using XYZ = std::array< dfloat, 3 >;

      /// \brief The white point, as (x,y) chromacity coordinates.
      using xy = std::array< dfloat, 2 >;

      /// \brief The CIE Standard Illuminant A (typical, domestic, tungsten-filament lighting).
      DIP_EXPORT static constexpr XYZ IlluminantA{{ 1.0985, 1.0000, 0.3558 }};

      /// \brief The CIE Standard Illuminant D50 (mid-morning or mid-afternoon daylight, color temperature is about 5000 K).
      DIP_EXPORT static constexpr XYZ IlluminantD50{{ 0.9642, 1.0000, 0.8252 }};

      /// \brief The CIE Standard Illuminant D55 (morning or evening daylight, color temperature is about 5500 K).
      DIP_EXPORT static constexpr XYZ IlluminantD55{{ 0.9568, 1.0000, 0.9215 }};

      /// \brief The CIE Standard Illuminant D65 (noon daylight, color temperature is about 6500 K). This is also used in the sRGB standard.
      DIP_EXPORT static constexpr XYZ IlluminantD65{{ 0.9504, 1.0000, 1.0889 }};

      /// \brief The CIE Standard Illuminant E (synthetic, equal energy illuminant).
      DIP_EXPORT static constexpr XYZ IlluminantE{{ 1.0000, 1.0000, 1.0000 }};

      /// \brief Configure the conversion functions to use the given white point.
      ///
      /// This will configure each of the converter functions that use the white point information
      /// (grey &harr; RGB &harr; XYZ &harr; Lab/Luv). The default white point is the Standard Illuminant D65
      /// (`dip::ColorSpaceManager::IlluminantD65`).
      ///
      /// The white point is given as an XYZ triplet or (x,y) chromacity coordinates.
      DIP_EXPORT void SetWhitePoint( XYZ whitePoint );
      void SetWhitePoint( xy const& whitePoint ) {
         XYZ triplet;
         triplet[ 0 ] = whitePoint[ 0 ];
         triplet[ 1 ] = whitePoint[ 1 ];
         triplet[ 2 ] = 1 - whitePoint[ 0 ] - whitePoint[ 1 ];
         SetWhitePoint( triplet );
      }

   private:

      struct ColorSpace {
         String name;
         dip::uint nChannels;
         std::map< dip::uint, ColorSpaceConverterPointer > edges;  // The key is the target color space index
         ColorSpace( String const& name, dip::uint chans ) :
               name( name ), nChannels( chans ) {}
      };

      std::map< String, dip::uint > names_;
      std::vector< ColorSpace > colorSpaces_;

      dip::uint Index( String const& name ) const {
         auto it = names_.find( name );
         DIP_THROW_IF( it == names_.end(), "Color space name not defined" );
         return it->second;
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

      // Find an optimal path between two color spaces, given by their indices. Returns a list of color space
      // indices including `start` and `stop`.
      DIP_NO_EXPORT std::vector< dip::uint > FindPath( dip::uint start, dip::uint stop ) const;
};

/// \}

} // namespace dip

#endif // DIP_COLOR_H
