/*
 * (c)2016-2025, Cris Luengo.
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

#include <array>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "diplib.h"
#include "diplib/iterators.h"


/// \file
/// \brief Color space conversions
/// See \ref colorspaces.


namespace dip {


/// \group colorspaces Color spaces
/// Management and conversion between color spaces
/// \addtogroup

/// \brief An XYZ triplet, used to specify a white point for color spaces.
using XYZ = std::array< dfloat, 3 >;

/// \brief A color, as (x,y) chromaticity coordinates, used to specify a white point for color spaces.
using xy = std::array< dfloat, 2 >;

/// \brief XYZ matrix (3x3 matrix, column-major order) for conversion between RGB and XYZ. Computed from a \ref XYZ triplet.
using XYZMatrix = std::array< dfloat, 9 >;


/// \brief Abstract base class for conversion between two color spaces.
///
/// Classes that convert between color spaces must derive from this and overload all the pure virtual functions.
/// See \ref dip::ColorSpaceManager for how to use these converters.
class DIP_CLASS_EXPORT ColorSpaceConverter {
   public:
      /// \brief Returns the source color space name.
      virtual String InputColorSpace() const = 0;

      /// \brief Returns the destination color space name.
      virtual String OutputColorSpace() const = 0;

      /// \brief Returns the cost of the conversion. This cost includes computational cost as well as precision loss.
      ///
      /// Called by \ref dip::ColorSpaceManager::Convert.
      ///
      /// The cost is used to avoid pathways such as "RGB" &rarr; "grey" &rarr; "Lab" instead of "RGB" &rarr; "XYZ" &rarr; "Yxy" &rarr; "Lab".
      /// Conversion to grey therefore must always have a high cost. It is not necessary to define this method,
      /// the default implementation returns a cost of 1.
      virtual dip::uint Cost() const { return 1; }

      /// \brief This is the method that performs the conversion for one image line.
      ///
      /// Called by \ref dip::ColorSpaceManager::Convert.
      ///
      /// `input` and `output` point to buffers with the number of tensor elements expected for the two color
      /// spaces, as determined by the `InputColorSpace` and `OutputColorSpace` method.
      virtual void Convert( ConstLineIterator< dfloat >& input, LineIterator< dfloat >& output ) const = 0;

      /// \brief This method is called to set the white point used by the converter. Does nothing by default.
      ///
      /// Called by \ref dip::ColorSpaceManager::SetWhitePoint.
      ///
      /// `matrix` and `inverseMatrix` are computed from `whitePoint` by the caller, to avoid multiple converter
      /// functions doing the same computations.
      virtual void SetWhitePoint( XYZ const& whitePoint, XYZMatrix const& matrix, XYZMatrix const& inverseMatrix ) {
         ( void )whitePoint;
         ( void )matrix;
         ( void )inverseMatrix;
      }

      ColorSpaceConverter() = default;
      ColorSpaceConverter( ColorSpaceConverter const& ) = delete;
      ColorSpaceConverter( ColorSpaceConverter&& ) = default;
      ColorSpaceConverter& operator=( ColorSpaceConverter const& ) = delete;
      ColorSpaceConverter& operator=( ColorSpaceConverter&& ) = default;
      virtual ~ColorSpaceConverter() = default;
};


/// \brief An object of this class is used to convert images between color spaces.
///
/// By default, the object will known a set of color spaces, and be able to convert between
/// them. It is possible to define new color spaces, see below.
///
/// To convert an image into a different color space, simply call the \ref Convert function.
/// In  this code snippet, we first set the image's color space to sRGB. This causes no
/// change to the pixel values, it simply tags the image object with the color space name.
/// ```cpp
/// dip::ColorSpaceManager csm;
/// dip::Image img = ...
/// img.SetColorSpace( "sRGB" );                     // img is sRGB
/// dip::Image img_lab = csm.Convert( img, "Lab" );  // img_lab will be Lab
/// ```
///
/// These are the color spaces known by default. Color space names are case-sensitive,
/// but aliases are registered for all these names using all-lowercase.
///
/// <div markdown="1" class="m-spaced m-block m-flat">
/// Name     | Aliases    | Description
/// -------- | ---------- | ------------
/// `"grey"` | `"gray"`   | An empty string is also interpreted as grey. Defined to be in the range [0,255].
/// `"RGB"`  |            | Linear RGB, defined in the range [0,255].
/// `"sRGB"` | `"R'G'B'"` | Industry-standard non-linear, gamma-corrected RGB (average gamma is approximately 2.2, with a linear segment near 0). Values are in the range [0,255].
/// `"sRGBA"` |           | sRGB with an alpha channel, intended mostly for use when reading 4-channel PNG image files. Conversion to sRGB simply drops the alpha channel.
/// `"CMY"`  |            | Cyan-Magenta-Yellow. Subtractive colors, defined simply as 255-RGB. Values are in the range [0,255].
/// `"CMYK"` |            | Cyan-Magenta-Yellow-blacK. Subtractive colors with black added. Note that printers need a more complex mapping to CMYK to work correctly.
/// `"HSI"`  |            | Hue-Saturation-Intensity. L^1^ norm polar decomposition of the RGB cube, more suited to image analysis than HSV or HCV. S and I are in the range [0,255], H is an angle in degrees. Defined by Hanbury and Serra (2003).
/// `"ICH"`  |            | Intensity-Chroma-Hue. Rotation of the RGB cube, where I is along the black-white diagonal of the cube, and the CH-plane is perpendicular. I is in the range [0,255], H is an angle in degrees.
/// `"ISH"`  |            | Intensity-Saturation-Hue. Based in ICH, where S is the C channel normalized so that the maximum saturation for each H is 1. For each H, the largest value of C is attained for a different value of I.
/// `"HCV"`  |            | Hue-Chroma-Value. V is the max of R, G and B, and C is the difference between largest and smallest RGB intensities. C and V are in range [0,255], H is an angle in degrees.
/// `"HSV"`  |            | Hue-Saturation-Value. Based on HCV, where S is equal to C normalized by V. S is in range [0,1] and V in range [0,255], H is an angle in degrees.
/// `"Y'PbPr"` | `"YPbPr"`, `"YPP"` | Luma and two chroma components. Computed from R'G'B' (non-linear, gamma-corrected RGB). Y' is in the range [0,1], and Pb and Pr are in the range [-0.5,0.5].
/// `"Y'CbCr"` | `"YCbCr"`, `"YCC"` | Luma and two chroma components. Scaled Y'PbPr such that all three components are in the range [0,255]. Sometimes incorrectly referred to as YUV.
/// `"XYZ"`  |            | CIE 1931 XYZ, standard observer tristimulus values. A rotation of the (linear) RGB cube that aligns Y with the luminance axis. X, Y and Z are in the range [0,1].
/// `"Yxy"`  |            | CIE Yxy, where x and y are normalized X and Y. They are the chromaticity coordinates.
/// `"Lab"`  | `"L*a*b*"`, `"CIELAB"` | Lightness and two chromaticity coordinates. One of two color spaces proposed by CIE in 1976 in an attempt for perceptual uniformity. L is in the range [0,100], and a and b are approximately in the range [-100,100].
/// `"Luv"`  | `"L*u*v*"`, `"CIELUV"` | Lightness and two chromaticity coordinates. One of two color spaces proposed by CIE in 1976 in an attempt for perceptual uniformity. L is in the range [0,100], and u and v are in a range significantly wider than [-100,100].
/// `"LCH"`  | `"L*C*H*"` | Lightness-Chroma-Hue. Computed from CIE Lab, where C and H are the polar coordinates to a and b. H is an angle in degrees.
/// `"Oklab"` |           | An "OK Lab colorspace", a better approximation to perceptual uniformity than CIE Lab. Oklab was designed to better predict CAM16-UCS results than other existing color spaces, while being numerically stable. Assumes a D65 white point. L is in the range [0,1], a and b are approximately in the range [-1,1]. Defined by Ottosson (2020).
/// `"Oklch"` |           | Lightness-Chroma-Hue derived from Oklab, where C and H are the polar coordinates to a and b. H is an angle in degrees.
/// `"wavelength"` |      | Can only be converted from, not to. Yields an approximate color representation for the given wavelength in nanometers, in the range 380 through 780 nanometers. For values outside the range, produces black. The conversion to XYZ is according to CIE rec. 709, but most of these colors lie outside of the RGB gamut. The conversion to RGB produces colors within the gamut, computed according to Young (2012).
/// </div>
///
/// Note that most color images are stored to file as (nonlinear) sRGB. After loading a color image,
/// it is therefore often advantageous to convert the image to (linear) RGB for computation (or some
/// other desired color space).
///
/// When converting to/from gray, it is assumed that gray is linear (i.e. a weighted addition of the linear R, G and B
/// values, the weights depending on the white point). But in the case of HSI and ISH color spaces, the I channel is the
/// gray channel; I is a non-weighted mean of linear RGB, the conversion does not take the white point into account.
///
/// # Defining a new color space
///
/// It is possible to define new color spaces, and register conversion functions that
/// translate from one color space to another. The `ColorSpaceManager` object is capable of finding optimal
/// paths trough the graph defined by these conversion functions, to convert between color spaces. Thus,
/// it is not necessary to create functions that translate from your new color space to
/// all known color spaces, it is sufficient to register two function that translate to
/// and from your new color space from/to any existing color space.
///
/// In this example, we define a conversion from a new color space (Frank) to XYZ, and from Yxy to Frank.
/// The \ref Convert function then is able to convert an image from any color space to Frank, and from Frank
/// to any color space.
/// ```cpp
/// dip::ColorSpaceManager csm;
/// csm.Define( "Frank", 4 );                        // A new color space with 4 channels
/// csm.DefineAlias( "f", "Frank" );                 // "f" is an alias for "Frank"
/// csm.Register( std::make_shared< frank2xyz >() ); // an object that converts from Frank to XYZ
/// csm.Register( std::make_shared< yxy2frank >() ); // an object that converts from Yxy to Frank
///
/// dip::Image img = ...                             // assume img is sRGB
/// csm.Convert( img, img, "f" );                    // img will be converted from sRGB to Frank
/// ```
/// In the example above, `frank2xyz` and `yxy2frank` are objects derived from \ref dip::ColorSpaceConverter.
///
/// Note that one could add conversion functions to and from more color spaces as deemed appropriate,
/// for example to save computational time. And it is not necessary for a new color space to have a conversion
/// path to or from it. For example, by default the "wavelength" color space has only a conversion function
/// from it to XYZ and to RGB, there are no functions that can convert to the "wavelength" color space.
///
/// !!! literature
///     - C. Poynton, "Color FAQ", 1997. <https://poynton.ca/PDFs/ColorFAQ.pdf> (last retrieved February 3, 2021).
///     - A. Hanbury and J. Serra, "Colour image analysis in 3D-polar coordinates", Joint Pattern Recognition Symposium, 2003.
///     - A.T. Young, "Rendering Spectra", 2012. <https://aty.sdsu.edu/explain/optics/rendering.html> (last retrieved August 1, 2020).
///     - B. Ottosson, "A perceptual color space for image processing", 2020. <https://bottosson.github.io/posts/oklab/> (last retrieved October 23, 2023).
class DIP_NO_EXPORT ColorSpaceManager {
   public:
      /// \brief A shared pointer to a \ref ColorSpaceConverter object.
      using ColorSpaceConverterPointer = std::shared_ptr< ColorSpaceConverter >; // TODO: MSVC does not like us using a unique_ptr here, which is really what we want to do.

      /// \brief Constructor, registers the default color spaces.
      DIP_EXPORT ColorSpaceManager();

      /// \brief Defines a new color space, that requires `nChannels` channels.
      void Define( String colorSpaceName, dip::uint nChannels ) {
         DIP_THROW_IF( IsDefined( colorSpaceName ), "Color space name already defined" );
         names_[ colorSpaceName ] = colorSpaces_.size();
         colorSpaces_.emplace_back( std::move( colorSpaceName ), nChannels );
      }

      /// \brief Defines an alias for a defined color space name.
      void DefineAlias( String const& alias, String const& colorSpaceName ) {
         DIP_THROW_IF( IsDefined( alias ), "Alias name already defined" );
         names_[ alias ] = Index( colorSpaceName );
      }

      /// \brief Registers a function object to translate from one color space to another. The
      /// `dip::ColorSpaceManager` object takes ownership of the converter.
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

      /// \brief Overload for the previous function, for backwards compatibility. Not recommended.
      void Register( ColorSpaceConverter* converter ) {
         Register( ColorSpaceConverterPointer( converter ));
      }

      /// \brief Check to see if a color space name is defined.
      bool IsDefined( String const& colorSpaceName ) const {
         return names_.count( colorSpaceName ) != 0;
      }

      /// \brief Gets a pointer to a color space converter object registered with this `ColorSpaceManager`.
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
      String const& CanonicalName( String const& colorSpaceName ) const {
         return colorSpaces_[ Index( colorSpaceName ) ].name;
      }

      /// \brief Converts an image to a different color space.
      ///
      /// Both the source (`in.ColorSpace()`) and destination (`colorSpaceName`) color spaces must be known,
      /// and a path of registered conversion functions must exist between the two.
      ///
      /// Note that it is possible to assign an arbitrary string as a color space name in an image. Setting
      /// an image's color space property is always possible, and gives no guarantee that the image has the
      /// right number of tensor elements (color channels).
      ///
      /// When converting from one color channel to another, the input image is checked for number of color
      /// channels. If it doesn't match the number expected for its color space, an exception will be thrown.
      ///
      /// If `in.ColorSpace()` is an empty string:
      ///
      /// - If the image has the same number of color channels as expected for `colorSpaceName`, it will
      ///     be assumed that the image already is in the `colorSpaceName` color space, and no conversion is done.
      /// - Else, if the image is scalar, it will be assumed that its color space is "grey".
      /// - Otherwise, an exception will be thrown.
      ///
      /// If `colorSpaceName` is an empty string, "grey" is assumed.
      ///
      /// The output image is of a floating-point type. If you want the output to be written in, for example,
      /// an 8-bit unsigned integer, you can use the "protect" flag on the output image:
      ///
      /// ```cpp
      /// dip::Image in = ...; // read in a color image
      /// dip::Image out;
      /// out.SetDataType( dip::DT::UINT8 );
      /// out.Protect();
      /// dip::ColorSpaceManager csm;
      /// cms.Convert( in, out, "HSV" );
      /// ```
      ///
      /// In this case, all computations are still performed as double-precision floating-point computations,
      /// but the result is cast to 8-bit unsigned integers when written to the output image. Some color spaces,
      /// such as RGB and CMYK are defined to use the [0,255] range of 8-bit unsigned integers. Other color spaces
      /// such as Lab and XYZ are not. For those color spaces, casting to an integer will destroy the data.
      DIP_EXPORT void Convert( Image const& in, Image& out, String const& colorSpaceName = "" ) const;
      DIP_NODISCARD Image Convert( Image const& in, String const& colorSpaceName = "" ) const {
         Image out;
         Convert( in, out, colorSpaceName );
         return out;
      }

      // for backwards compatibility
      using XYZ [[ deprecated( "Use dip::XYZ" ) ]] = dip::XYZ;

      /// \brief The CIE Standard Illuminant A (typical, domestic, tungsten-filament lighting).
      DIP_EXPORT static constexpr dip::XYZ IlluminantA{{ 1.0985, 1.0000, 0.3558 }};

      /// \brief The CIE Standard Illuminant D50 (mid-morning or mid-afternoon daylight, color temperature is about 5000 K).
      DIP_EXPORT static constexpr dip::XYZ IlluminantD50{{ 0.9642, 1.0000, 0.8252 }};

      /// \brief The CIE Standard Illuminant D55 (morning or evening daylight, color temperature is about 5500 K).
      DIP_EXPORT static constexpr dip::XYZ IlluminantD55{{ 0.9568, 1.0000, 0.9215 }};

      /// \brief The CIE Standard Illuminant D65 (noon daylight, color temperature is about 6500 K). This is also used in the sRGB standard.
      DIP_EXPORT static constexpr dip::XYZ IlluminantD65{{ 0.9504, 1.0000, 1.0889 }};

      /// \brief The CIE Standard Illuminant E (synthetic, equal energy illuminant).
      DIP_EXPORT static constexpr dip::XYZ IlluminantE{{ 1.0000, 1.0000, 1.0000 }};

      /// \brief Configure the conversion functions to use the given white point.
      ///
      /// This will configure each of the converter functions that use the white point information
      /// (grey &harr; RGB &harr; XYZ &harr; Lab/Luv). The default white point is the Standard Illuminant D65
      /// (\ref dip::ColorSpaceManager::IlluminantD65).
      ///
      /// The white point is given as an XYZ triplet or (x,y) chromaticity coordinates.
      DIP_EXPORT void SetWhitePoint( dip::XYZ whitePoint );

      /// \brief Overload of the function above that takes a (x,y) chromaticity coordinate.
      void SetWhitePoint( xy const& whitePoint ) {
         dip::XYZ triplet;
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
         // Note that we use std::map here because we expect few edges
         ColorSpace( String name, dip::uint chans ) :
               name( std::move( name )), nChannels( chans ) {}
      };

      std::map< String, dip::uint > names_;
      // Note that we use std::map here because we expect a relatively small set of color spaces
      std::vector< ColorSpace > colorSpaces_;

      dip::uint Index( String const& name ) const {
         auto it = names_.find( name );
         DIP_THROW_IF( it == names_.end(), "Color space name not defined" );
         return it->second;
      }

      // The map `names_` translates known color space names to an index into the `colorSpaces_` array.
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

/// \brief Apply the alpha channel in the sRGBA image `in`, using the background color `background`.
///
/// The alpha channel is expected to be in the range [0, `scaling`]. The output image is of the
/// same data type as `in`, but will have 3 channels and be in he sRGB color space.
///
/// If `in` has two channels, it is assumed to be a gray-scale image with an alpha channel,
/// and the output will be scalar. Otherwise, if `in` is not sRGBA, it is returned as-is.
///
/// The alpha channel is assumed to not be pre-multiplied.
///
/// \see AlphaMask
// TODO: correctly handle pre-multiplied alpha (as an option) and do computations in linear RGB.
DIP_EXPORT void ApplyAlphaChannel(
      Image const& in,
      Image& out,
      Image::Pixel const& background = { 0 },
      dfloat scaling = 255
);
DIP_NODISCARD inline Image ApplyAlphaChannel(
      Image const& in,
      Image::Pixel const& background = { 0 },
      dfloat scaling = 255
) {
   Image out;
   ApplyAlphaChannel( in, out, background, scaling );
   return out;
}

/// \endgroup

} // namespace dip

#endif // DIP_COLOR_H
