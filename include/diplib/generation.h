/*
 * (c)2017-2023, Cris Luengo.
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

#ifndef DIP_GENERATION_H
#define DIP_GENERATION_H

#include <vector>
#include <utility>

#include "diplib.h"
#include "diplib/random.h"

// Forward declarations taken from the FreeType headers. This avoids making FreeType a dependency of these headers.
using FT_Library = struct FT_LibraryRec_*;
using FT_Face = struct FT_FaceRec_*;


/// \file
/// \brief Functions for generating image data.
/// See \ref generation.


namespace dip {

// Forward declaration, defined in polygon.h
struct DIP_NO_EXPORT Polygon;

/// \group generation Generation
/// \brief Filling images with generated data, and creating test images.

/// \group generation_drawing Drawing
/// \ingroup generation
/// \brief Drawing in images
/// \addtogroup

/// \brief Sets the pixels at the border of `out` to `value`.
///
/// `sizes` must contain either a single value or one value per image dimension, and indicates how many pixels
/// in from the border are set.
///
/// `out` must not be 0D.
DIP_EXPORT void SetBorder( Image& out, Image::Pixel const& value = { 0 }, UnsignedArray const& sizes = { 1 } );

/// \brief Multiplies the image with a windowing function.
///
/// `type` can be one of the following windowing functions:
///
/// - "Hamming": A cosine window. Set `parameter` to 0.5 to get a Hann window, and to 25.0/46.0 to get
///   a Hamming window. With 0.53836, a small refinement to the Hamming optimum, yields the minimum peak
///   side-lobe level.
/// - "Gaussian": A Gaussian window, this is the only one that is isotropic. `parameter` is the sigma, as a function
///   of the image half-width. Choose a value smaller or equal to 0.5. At 0.5, 4 sigmas fit in the image width.
/// - "Tukey": A rectangular window convolved with a Hann window. `parameter` is the fraction of image
///   width occupied by the cosine lobe. If `parameter` is 1.0, it is a Hann window, if it is 0.0 it is a rectangular
///   window.
/// - "GaussianTukey": A rectangular window convolved with a Gaussian window. `parameter` is the sigma in pixels,
///   a value of the order of 10 is a good choice. The rectangular window is of the size of the image minus 3 sigma
///   on each edge.
///   This is the only window where the tapering is independent of the image width, and thus equal along each image
///   dimension even if the image is not square. If the image size along one dimension is too small to accommodate
///   the window shape, a Gaussian window is created instead.
///
/// In all these cases, the window is applied to each dimension independently, meaning that the multi-dimensional
/// window is the outer product of the 1D windows.
DIP_EXPORT void ApplyWindow( Image const& in, Image& out, String const& type = "Hamming", dfloat parameter = 0.5 );
DIP_NODISCARD inline Image ApplyWindow( Image const& in, String const& type = "Hamming", dfloat parameter = 0.5 ) {
   Image out;
   ApplyWindow( in, out, type, parameter );
   return out;
}

/// \brief Draws a Bresenham line in an image.
///
/// The line goes from `start` to `end`, both points included. These points must be within the image.
/// Pixels in `out` on the line are set to `value`, other pixels are not touched.
/// `blend` can be one of the following strings:
///
/// - `"assign"`: The pixels are set to `value`.
/// - `"add"`: `value` is added to the pixels using saturated arithmetic.
DIP_EXPORT void DrawLine(
      Image& out,
      UnsignedArray const& start,
      UnsignedArray const& end,
      Image::Pixel const& value = { 1 },
      String const& blend = S::ASSIGN
);

/// \brief Draws a series of Bresenham lines in an image.
///
/// Lines are drawn from `points[0]` to `points[1]`, from `points[1]` to `points[2]`, etc, forming a continuous
/// curve composed of straight (Bresenham) line segments that hits each of the points in sequence.
/// To create a closed curve, repeat the first point at the end.
///
/// `points` must have at least two points, and all points must be within the image.
/// Pixels in `out` on the lines are set to `value`, other pixels are not touched.
/// `blend` can be one of the following strings:
///
/// - `"assign"`: The pixels are set to `value`.
/// - `"add"`: `value` is added to the pixels using saturated arithmetic.
///
/// `out` must have at least two dimensions.
DIP_EXPORT void DrawLines(
      Image& out,
      CoordinateArray const& points,
      Image::Pixel const& value = { 1 },
      String const& blend = S::ASSIGN
);

/// \brief Draws a polygon in a 2D image.
///
/// Draws a polygon going through each of the points in `polygon`. `mode` can be one of the following strings:
///
/// - `"open"`: the start and end points are not connected.
/// - `"closed"`: the start and end points are connected.
/// - `"filled"`: the polygon is filled, that is, all pixels within the polygon are painted (default).
///
/// In the `"filled"` mode, the polygon must be simple (which is guaranteed if it was obtained from a \ref ChainCode).
/// In the `"open"` or `"closed"` mode, the polygon can self-intersect. In all cases, polygon vertices can be outside
/// the image. The two different algorithms (filled and not filled) do not necessarily produce the exact same polygon
/// outline, rounding errors can be different.
///
/// Pixels in `out` on the polygon (and within the polygon for filled polygons) are set to `value`, other pixels
/// are not touched.
///
/// The \ref dip::Polygon struct is defined in \ref "diplib/polygon.h", which you'll need to include to use this function.
///
/// `out` must have two dimensions.
DIP_EXPORT void DrawPolygon2D(
      Image& out,
      Polygon const& polygon,
      Image::Pixel const& value = { 1 },
      String const& mode = S::FILLED
      // String const& blend = S::ASSIGN
);

/// \brief Draws a solid ellipsoid in an image.
///
/// The ellipsoid is centered around the coordinates given by `origin`, and has a diameter `sizes[ii]` along
/// dimension `ii`. That is, the ellipsoid is composed of all pixels within a Euclidean distance of `sizes/2`
/// from the `origin`.
///
/// The origin does not need to be within the image.
/// Pixels in `out` within the ellipsoid are set to `value`, other pixels are not touched.
///
/// `out` must have at least one dimension.
DIP_EXPORT void DrawEllipsoid(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value = { 1 }
      // String const& blend = S::ASSIGN
);

/// \brief Draws a solid diamond in an image.
///
/// The diamond is centered around the coordinates given by `origin`, and has a width `sizes[ii]` along
/// dimension `ii`. That is, the diamond is composed of all pixels within a L-1 distance of `sizes/2`
/// from the `origin`.
///
/// The origin does not need to be within the image.
/// Pixels in `out` within the diamond are set to `value`, other pixels are not touched.
///
/// `out` must have at least one dimension.
DIP_EXPORT void DrawDiamond(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value = { 1 }
      // String const& blend = S::ASSIGN
);

/// \brief Draws a solid box (rectangle) in an image.
///
/// The box is centered around the coordinates given by `origin`, and has a width `sizes[ii]` along
/// dimension `ii`. That is, the box is composed of all pixels within a L-infinity distance of `sizes/2`
/// from the `origin`.
///
/// The origin does not need to be within the image.
/// Pixels in `out` within the box are set to `value`, other pixels are not touched.
///
/// `out` must have at least one dimension.
DIP_EXPORT void DrawBox(
      Image& out,
      FloatArray const& sizes,
      FloatArray const& origin,
      Image::Pixel const& value = { 1 }
      // String const& blend = S::ASSIGN
);


/// \brief Draws an approximately bandlimited point in the image, in the form of a Gaussian blob.
///
/// The blob is centered around the coordinates given by `origin`, and `sigmas[ii]` is the parameter for the
/// Gaussian along dimension `ii`. The Gaussian is scaled such that its integral is `value`. The integral might
/// be off if `sigmas` contains a small value.
///
/// The origin does not need to be within the image. `sigmas * truncation` is the size of the box around `origin`
/// that is affected by the blob. Pixels in `out` within that box have the values of the Gaussian added to them,
/// other pixels are not touched.
///
/// `out` must not be binary, and have at least one dimension.
DIP_EXPORT void DrawBandlimitedPoint(
      Image& out,
      FloatArray origin,
      Image::Pixel const& value = { 1 },
      FloatArray sigmas = { 1.0 },
      dfloat truncation = 3.0
      // String const& blend = S::ADD
);

/// \brief Draws an approximately bandlimited line between two points in the image, using Gaussian profiles.
///
/// The two points do not need to be within the image domain.
///
/// `sigma` determines the smoothness of the line. Values are calculated up to a distance of `sigma * truncation`
/// from the line, further away values are rounded to 0. `value` is the linear integral perpendicular to the line.
/// That is, it is the weight of the Gaussian used to draw the line. The values are added to existing values in
/// the image `out`.
///
/// `out` must not be binary and have at least two dimensions.
///
/// If `start` and `end` are identical, calls \ref dip::DrawBandlimitedPoint.
DIP_EXPORT void DrawBandlimitedLine(
      Image& out,
      FloatArray start,
      FloatArray end,
      Image::Pixel const& value = { 1 },
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
      // String const& blend = S::ADD
);

/// \brief Draws an approximately bandlimited ball (disk) or an n-sphere (circle) in an image, using Gaussian profiles.
///
/// The ball is centered around the coordinates given by `origin`, and has a diameter `diameter` along
/// all dimensions. The origin does not need to be within the image.
///
/// If `mode` is `"empty"`, a circle/sphere/n-sphere is drawn as a thin shell with a Gaussian profile.
/// If `mode` is `"filled"`, a disk/ball/hyperball is drawn as a solid shape with an error function transition
/// to background values. The former is the gradient magnitude of the latter.
///
/// In both cases, `sigma` determines the smoothness of the shape, and `truncation`
/// determines how far out from the edge the smooth values are computed: at a distance of `sigma * truncation`
/// the values are rounded to 1 or 0. `value` indicates the weight of the ball: it is the value of the
/// solid shape, and the value of the integral perpendicular to the edge for the empty shape.
///
/// The ball is added to the image `out`. Pixels within `sigma * truncation` of the ball's edge have
/// their value increased, other pixels are not touched.
///
/// `out` must not be binary, and have at least one dimension.
///
/// Note: `diameter` is a scalar, unlike for similar functions, because a bandlimited ellipsoid would be very
/// expensive (and complicated) to compute in the spatial domain.
DIP_EXPORT void DrawBandlimitedBall(
      Image& out,
      dfloat diameter,
      FloatArray origin,
      Image::Pixel const& value = { 1 },
      String const& mode = S::FILLED,
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
      // String const& blend = S::ADD
);

/// \brief Draws an approximately bandlimited box (rectangle) in an image, using Gaussian profiles.
///
/// The box is centered around the coordinates given by `origin`, and has a width of `sizes[ii]` along
/// dimension `ii`. The origin does not need to be within the image.
///
/// If `mode` is `"empty"`, the edge of the rectangle or the surface of the box is drawn as a thin shell
/// with a Gaussian profile. If `mode` is `"filled"`, the rectangle/box is drawn as a solid shape with an
/// error function transition to background values. The former is the gradient magnitude of the latter.
///
/// In both cases, `sigma` determines the smoothness of the shape, and `truncation`
/// determines how far out from the edge the smooth values are computed: at a distance of `sigma * truncation`
/// the values are rounded to 1 or 0. `value` indicates the weight of the ball: it is the value of the
/// solid shape, and the value of the integral perpendicular to the edge for the empty shape.
///
/// The box is added to the image `out`. Pixels within `sigma * truncation` of the box's edge have
/// their value increased, other pixels are not touched.
///
/// `out` must not be binary, and have at least one dimension.
DIP_EXPORT void DrawBandlimitedBox(
      Image& out,
      FloatArray sizes,
      FloatArray origin,
      Image::Pixel const& value = { 1 },
      String const& mode = S::FILLED,
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
      // String const& blend = S::ADD
);

/// \brief Blends `value` into `out` at position `pos`, according to `mask`.
///
/// Computes `out = value * mask + out * (1 - mask)`, after shifting `value` and `mask` by `pos`. If
/// mask is an integer type, it will be scaled to the 0-1 range first. That is, where `mask` is maximal,
/// `out` will be assigned `value`. Where `mask` is zero, `out` will not be changed. Intermediate values
/// indicate how much of `value` to mix into the existing color.
/// `value` is cast to the data type of `out` after blending.
///
/// `out` is a forged image of any data type, dimensionality and number of tensor elements. `mask` is a
/// scalar image of the same dimensionality. If it is a floating-point image, the values should be in
/// the range 0-1; if it is an integer image, the values should be between 0 and the maximum for the data type.
/// `mask` can also be binary, but it cannot be a complex type.
///
/// `value` is an image of the same sizes as `mask`, or can be singleton-expanded to the same sizes. It has
/// either one tensor element or as many as `out` (i.e. the tensor dimension can be singleton-expanded to
/// the tensor size of `out`).
///
/// Note that `value` can be a single pixel to paint `mask` in a single color. Likewise, `mask` can be a single
/// pixel to mix in `value` at a constant level. `mask` and `value` will be singleton-expanded to the match
/// their sizes. This means that, if both are a single pixel, only a single pixel in `out` will be modified.
///
/// `pos` has one value for each dimension in `out`, and indicates the position of the top-left corner of
/// `mask` and `value` in `out`. That is, `mask` and `value` will be translated by this vector.
/// Note that `mask` can fall partially outside of `out`, it is perfectly fine to specify negative coordinates.
/// If `pos` is an empty array, no translation is applied, `mask` will coincide with the top-left corner
/// of `out`.
///
/// If `out` is binary, `mask` will be thresholded at 50%.
DIP_EXPORT void BlendBandlimitedMask(
      Image& out,
      Image const& mask,
      Image const& value = dip::Image( { 255 } ),
      IntegerArray pos = {}
);

/// \brief Class used to draw text using a specified font file (TTF, OTF, etc).
///
/// An object of this class must first be given the file name of a font on disk, which can be done either in the
/// constructor or through \ref SetFont. This font is subsequently used to render text in an image using \ref DrawText.
/// The text size can be specified by calling \ref SetSize before rendering.
///
/// This class supports the most common font file formats (TrueType, Type 1, CID-keyed and OpenType/CFF fonts are
/// supported). A default font is not provided because there is no standard for where these files are
/// to be found (on some platforms it's an easier problem than on others). A program that uses `FreeTypeTool` could
/// be distributed with a font file.
///
/// Example usage:
/// ```cpp
/// dip::FreeTypeTool freeTypeTool( "/usr/share/fonts/truetype/times.ttf" );
/// ```
///
/// !!! note "Note on thread-safety"
///     Setting a font through \ref SetFont is not thread-safe. The FreeType documentation says that the functionality
///     used in \ref DrawText is thread-safe, though I don't see how this function would work properly if called from
///     two threads at the same time. I would recommend using a mutex in a multi-threaded environment if the same object
///     is shared among threads. Ideally, each thread would create their own `FreeTypeTool` object, which is perfectly
///     safe to do.
///
/// `FreeTypeTool` objects cannot be copied, they can only be moved.
///
/// !!! warning
///     This class only works if *DIPlib* was configured to link to the FreeType library. By default this is not the
///     case, and none of the binary releases produced by the *DIPlib* project enable this functionality. If you want
///     to use this class, you need to compile *DIPlib* yourself. Otherwise, use \ref dip::DrawText, which uses a
///     hard-coded font.
class FreeTypeTool {
   public:
      /// \brief A default-constructed object cannot be used until a font is set with \ref SetFont.
      DIP_EXPORT FreeTypeTool();
      /// \brief This constructor immediately sets a font, see \ref SetFont for details.
      explicit FreeTypeTool( String const& font ) : FreeTypeTool() {
         SetFont( font );
      }

      // Destructor
      DIP_EXPORT ~FreeTypeTool();

      // Object can be moved
      FreeTypeTool( FreeTypeTool&& other ) noexcept {
         std::swap( library, other.library );
         std::swap( face, other.face );
      }
      FreeTypeTool& operator=( FreeTypeTool&& other ) noexcept {
         std::swap( library, other.library );
         std::swap( face, other.face );
         return *this;
      }

      // Object cannot be copied
      FreeTypeTool( FreeTypeTool const& ) = delete;
      FreeTypeTool& operator=( FreeTypeTool const& ) = delete;

      /// \brief Set the font to be used to render text.
      ///
      /// `font` is the full path to a file with a type face description (TrueType, Type 1, CID-keyed and OpenType/CFF
      /// fonts are supported).
      ///
      /// It is fine to switch fonts in between calls to \ref DrawText. When changing the font, the size selected
      /// through \ref SetSize is not preserved, and needs to be set anew. By default, the size is set to 12 pixels.
      ///
      /// This function is not thread-safe.
      DIP_EXPORT void SetFont( String const& font );

      /// \brief Set the font size to be used to render text.
      ///
      /// `size` is the EM square size in pixels (equivalent to the size in points at 72 dpi). It depends on the
      /// selected font how many pixels a letter actually takes up.
      DIP_EXPORT void SetSize( dfloat size );

      /// \brief Render text in an existing image.
      ///
      /// Draws text in the image `out`, at location `origin`, with a color given by `value`, and rotated according to
      /// `orientation`.
      ///
      /// `text` is any UTF-8 encoded string, even if Unicode support was not enabled when building *DIPlib*. But note
      /// that any ASCII string is also a properly UTF-8 encoded string. Characters that don't have a glyph in the given
      /// type face `font` will be rendered with the character known as "missing glyph", typically a box or a space.
      /// Note that control characters such as the newline and the backspace are not treated specially, and thus they
      /// will be drawn as a missing glyph. To draw multiple lines of text, call this function for each line in turn.
      ///
      /// `origin` is the pixel coordinates of a point on the base line. If `align` is `"left"`, `origin` is a point
      /// on the left edge of the rendered text; if `align` is `"right"`, it is a point on the right edge; and if it
      /// is `"center"`, it is the point halfway between the left and right edges.
      /// `orientation` is in radian, with 0 for horizontal text, and increasing clockwise.
      ///
      /// `out` must be a forged 2D image. If `out` is binary, the anti-aliased glyphs will be thresholded.
      /// `value` must have the same number of tensor elements as `out`. If `value` is scalar, this value will be used
      /// for all tensor elements.
      DIP_EXPORT void DrawText(
            Image& out,
            String const& text,
            FloatArray origin,
            Image::Pixel const& value = { 1 },
            dfloat orientation = 0,
            String const& align = S::LEFT
      );

      /// \brief Data structure returned by \ref DrawText(String const&, dfloat).
      struct TextInfo {
         Image image;         ///< The image with the rendered text.
         IntegerArray left;   ///< Coordinates within `image` of the point on the baseline at the left edge of the text.
         IntegerArray right;  ///< Coordinates within `image` of the point on the baseline at the right edge of the text.
      };

      /// \brief Alternate version of the function above that returns a new image tightly cropped around the rendered text.
      ///
      /// The output image is a 2D scalar image of type \ref dip::DT_UINT8, with white text on a black background.
      /// The output data structure additionally contains the two end points of the baseline, on either side of the
      /// rendered text.
      ///
      /// See \ref DrawText(Image&, String const&, FloatArray, Image::Pixel const&, dfloat, String const&)
      /// for a description of the `text` and `orientation` arguments.
      DIP_NODISCARD DIP_EXPORT TextInfo DrawText(
            String const& text,
            dfloat orientation = 0
      );

   private:
      FT_Library library = nullptr;
      FT_Face face = nullptr;
};

/// \brief Draws text with the built-in, fixed-sized glyphs.
///
/// Draws text in the image `out`, at location `origin`, with a color given by `value`, and rotated according to
/// `orientation`.
///
/// The font used is composed of glyph images rendered from the Open Sans font, at 14 px.
/// The lowercase letter 'x' is 8x8 pixels and uppercase letter 'X' is 9x10 pixels. The font uses anti-aliasing,
/// blending from the color `value` to the existing image colors.
///
/// `text` is any ASCII string; even if Unicode support was enabled when building *DIPlib*, the built-in font only
/// has glyphs for the ASCII characters 32-126. Other characters will be ignored. In particular, control characters
/// such as the newline and the backspace are ignored, to draw multiple lines of text, call this function for each
/// line in turn.
///
/// `origin` is the pixel coordinates of a point on the base line. If `align` is `"left"`, `origin` is a point
/// on the left edge of the rendered text; if `align` is `"right"`, it is a point on the right edge; and if it
/// is `"center"`, it is the point halfway between the left and right edges.
/// `orientation` is in radian, with 0 for horizontal text, and increasing clockwise.
///
/// `out` must be a forged 2D image. If `out` is binary, the anti-aliased glyphs will be thresholded.
/// `value` must have the same number of tensor elements as `out`. If `value` is scalar, this value will be used
/// for all tensor elements.
DIP_EXPORT void DrawText(
      Image& out,
      String const& text,
      FloatArray origin,
      Image::Pixel const& value = { 1 },
      dfloat orientation = 0,
      String const& align = S::LEFT
);

/// \brief Alternate version of the function above that returns a new image tightly cropped around the rendered text.
///
/// The output image is a 2D scalar image of type \ref dip::DT_UINT8, with white text on a black background.
///
/// See \ref DrawText(Image&, String const&, FloatArray, Image::Pixel const&, dfloat, String const&)
/// for a description of the functionality and arguments.
DIP_NODISCARD DIP_EXPORT Image DrawText(
      String const& text,
      dfloat orientation = 0
);

/// \endgroup


/// \group generation_test Test image generation
/// \ingroup generation
/// \brief Generating images with test objects or functions
/// \addtogroup

/// \brief Maps input values through an error function, can be used to generate arbitrary band-limited objects.
///
/// `in` is a scalar, real-valued function whose zero level set represents the edges of an object. The function
/// indicates the Euclidean distance to these edges, with positive values inside the object. `out` will have a
/// value of `value` inside the object, zero outside the object, and a Gaussian profile in the transition. If
/// `sigma` is larger or equal to about 0.8, and the input image is well formed, the output will be approximately
/// bandlimited.
///
/// The error function mapping is computed in band around the zero crossings where the input image has values
/// smaller than `sigma * truncation`.
///
/// If `value` has more than one element, the output will be a tensor image with the same number of elements.
///
/// The following example draws a band-limited cross, where the horizontal and vertical bars both have 20.5 pixels
/// width, and different sub-pixel shifts. The foreground has a value of 255, and the background of 0.
///
/// ```cpp
/// dip::UnsignedArray outSize{ 256, 256 };
/// dip::Image xx = 20.5 - dip::Abs( dip::CreateXCoordinate( outSize ) + 21.3 );
/// dip::Image yy = 20.5 - dip::Abs( dip::CreateYCoordinate( outSize ) - 7.8 );
/// dip::Image cross = dip::GaussianEdgeClip( dip::Supremum( xx, yy ), { 255 } );
/// ```
DIP_EXPORT void GaussianEdgeClip(
      Image const& in,
      Image& out,
      Image::Pixel const& value = { 1 },
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
);
DIP_NODISCARD inline Image GaussianEdgeClip(
      Image const& in,
      Image::Pixel const& value = { 1 },
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
) {
   Image out;
   GaussianEdgeClip( in, out, value, sigma, truncation );
   return out;
}

/// \brief Maps input values through a Gaussian function, can be used to generate arbitrary band-limited lines.
///
/// `in` is a scalar, real-valued function whose zero level set represents the lines to be drawn. The function
/// indicates the Euclidean distance to these edges. `out` will have lines with a Gaussian profile and a weight
/// of `value` (the integral perpendicular to the line is `value`), and a value of zero away from the lines. If
/// `sigma` is larger or equal to about 0.8, and the input image is well formed, the output will be approximately
/// bandlimited.
///
/// The Gaussian function mapping is computed in band around the zero crossings where the input image has values
/// smaller than `sigma * truncation`.
///
/// If `value` has more than one element, the output will be a tensor image with the same number of elements.
///
/// The following example draws a band-limited cross outline, where the horizontal and vertical bars both have
/// 20.5 pixels width, and different sub-pixel shifts. The lines have a weight of 1500, and the background has
/// a value of 0.
///
/// ```cpp
/// dip::UnsignedArray outSize{ 256, 256 };
/// dip::Image xx = 20.5 - dip::Abs( dip::CreateXCoordinate( outSize ) + 21.3 );
/// dip::Image yy = 20.5 - dip::Abs( dip::CreateYCoordinate( outSize ) - 7.8 );
/// dip::Image cross = dip::GaussianLineClip( dip::Supremum( xx, yy ), { 1500 } );
/// ```
DIP_EXPORT void GaussianLineClip(
      Image const& in,
      Image& out,
      Image::Pixel const& value = { 1 },
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
);
DIP_NODISCARD inline Image GaussianLineClip(
      Image const& in,
      Image::Pixel const& value = { 1 },
      dfloat sigma = 1.0,
      dfloat truncation = 3.0
) {
   Image out;
   GaussianLineClip( in, out, value, sigma, truncation );
   return out;
}

/// \brief Fills an image with a delta function.
///
/// All pixels will be zero except at the origin, where it will be 1.
/// `out` must be forged, and scalar.
///
/// `origin` specifies where the origin lies:
///
/// - `"right"`: The origin is on the pixel right of the center (at integer division result of
///   `size/2`). This is the default.
/// - `"left"`: The origin is on the pixel left of the center (at integer division result of
///   `(size-1)/2`).
/// - `"corner"`: The origin is on the first pixel. This is the default if no other option is given.
DIP_EXPORT void FillDelta( Image& out, String const& origin = "" );

/// \brief Creates a delta function image.
///
/// All pixels will be zero except at the origin, where it will be 1.
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
/// See \ref dip::FillDelta for the meaning of `origin`.
inline void CreateDelta( Image& out, UnsignedArray const& sizes, String const& origin = "" ) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillDelta( out, origin ));
}
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image CreateDelta( UnsignedArray const& sizes, String const& origin = "" ) {
   Image out;
   CreateDelta( out, sizes, origin );
   return out;
}

// Create a half Gaussian, used by MakeGaussian in linear/gauss.cpp (where it is defined)
// Not documented because not a public function. We suggest you use `CreateGauss()` instead.
// Length will be given by truncation and sigma, but limited to meaningful values.
//
// !!! warning
//     The second half of the gaussian will need to be scaled by -1.0 for odd derivative order (ex 1, 3, ...)
// Defined in src/linear/gauss.cpp
DIP_EXPORT std::vector< dfloat > MakeHalfGaussian(
      dfloat sigma,
      dip::uint derivativeOrder = 0,
      dfloat truncation = 3.0,
      DataType dt = DT_DFLOAT  // if not DT_DFLOAT, assumed to be DT_SFLOAT
);

// Create 1D Gaussian, used in linear/gauss.cpp (where it is defined) and in nonlinear/bilateral.cpp
// Not documented because not a public function. We suggest you use `CreateGauss()` instead.
// Length will be given by truncation and sigma, but limited to meaningful values.
DIP_EXPORT std::vector< dfloat > MakeGaussian(
      dfloat sigma,
      dip::uint derivativeOrder = 0,
      dfloat truncation = 3.0,
      DataType dt = DT_DFLOAT  // if not DT_DFLOAT, assumed to be DT_SFLOAT
);

/// \brief Creates a Gaussian kernel.
///
/// `out` is reforged to the required size to hold the kernel. These sizes are always odd. `sigmas` determines
/// the number of dimensions. `order` and `exponents` will be adjusted if necessary to match.
///
/// `derivativeOrder` is the derivative order, and can be a value between 0 and 3 for each dimension.
///
/// If `derivativeOrder` is 0, the size of the kernel is given by `2 * std::ceil( truncation * sigma ) + 1`.
/// The default value for `truncation` is 3, which assures a good approximation of the Gaussian kernel without
/// unnecessary expense. For derivatives, the value of `truncation` is increased by `0.5 * derivativeOrder`.
/// Truncation is limited to avoid unusefully small values.
///
/// By setting `exponents` to a positive value for each dimension, the created kernel will be multiplied by
/// the coordinates to the power of `exponents`.
///
/// `extent` defaults to `"full"`. Set it to `"half"` to generate only the first half (along each dimension)
/// of the kernel.
/// The second half of the gaussian will need to be scaled by -1.0 for odd derivative order (ex 1, 3, ...)
///
/// !!! warning
///     Convolving an image with the result of this function is much less efficient than calling \ref Gauss.
// Defined in src/linear/gauss.cpp
DIP_EXPORT void CreateGauss(
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray derivativeOrder = { 0 },
      dfloat truncation = 3.0,
      UnsignedArray exponents = { 0 },
      String const& extent = "full"
);
DIP_NODISCARD inline Image CreateGauss(
      FloatArray const& sigmas,
      UnsignedArray derivativeOrder = { 0 },
      dfloat truncation = 3.0,
      UnsignedArray exponents = { 0 },
      String const& extent = "full"
) {
   Image out;
   CreateGauss( out, sigmas, std::move( derivativeOrder ), truncation, std::move( exponents ), extent );
   return out;
}

/// \brief Creates a Gabor kernel.
///
/// `out` is reforged to the required size to hold the kernel. These sizes are always odd. `sigmas` determines
/// the number of dimensions. `frequencies` must have the same number of elements as `sigmas`.
///
/// Frequencies are in the range [0, 0.5), with 0.5 being the frequency corresponding to a period of the
/// size of the image.
///
/// The size of the kernel is given by `2 * std::ceil( truncation * sigma ) + 1`.
/// The default value for `truncation` is 3, which assures a good approximation of the kernel without
/// unnecessary expense. Truncation is limited to avoid unusefully small values.
///
/// !!! warning
///     Convolving an image with the result of this function is much less efficient than calling \ref GaborIIR.
// Defined in src/linear/gabor.cpp
DIP_EXPORT void CreateGabor(
      Image& out,
      FloatArray const& sigmas,
      FloatArray const& frequencies,
      dfloat truncation = 3.0
);
DIP_NODISCARD inline Image CreateGabor(
      FloatArray const& sigmas,
      FloatArray const& frequencies,
      dfloat truncation = 3.0
) {
   Image out;
   CreateGabor( out, sigmas, frequencies, truncation );
   return out;
}


/// \brief Generates the Fourier transform of an ellipsoid.
///
/// The length of the axes of the ellipsoid are specified through `radius`, which indicates the half-length
/// of the axes along each dimension. `amplitude` specifies the brightness of the ellipsoid.
///
/// The function is defined for images between 1 and 3 dimensions. `out` must be forged, scalar, and of a
/// floating-point type.
///
/// !!! literature
///     - L.J. van Vliet, "Grey-Scale Measurements in Multi-Dimensional Digitized Images",
///       Ph.D. thesis, Delft University of Technology, 1993.
DIP_EXPORT void FTEllipsoid(
      Image& out,
      FloatArray radius = { 1 },
      dfloat amplitude = 1
);
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image FTEllipsoid(
      UnsignedArray const& sizes,
      FloatArray radius = { 1 },
      dfloat amplitude = 1
) {
   Image out( sizes, 1, DT_SFLOAT );
   FTEllipsoid( out, std::move( radius ), amplitude );
   return out;
}

/// \brief Generates the Fourier transform of a box.
///
/// The length of the sides of the box are specified through `length`, which indicates the half-length
/// of the sides along each dimension. `amplitude` specifies the brightness of the box.
///
/// `out` must be forged, scalar, and of a floating-point type.
DIP_EXPORT void FTBox(
      Image& out,
      FloatArray length = { 1 },
      dfloat amplitude = 1
);
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image FTBox(
      UnsignedArray const& sizes,
      FloatArray length = { 1 },
      dfloat amplitude = 1
) {
   Image out( sizes, 1, DT_SFLOAT );
   FTBox( out, std::move( length ), amplitude );
   return out;
}

/// \brief Generates the Fourier transform of a cross.
///
/// The length of the sides of the cross are specified through `length`, which indicates the half-length
/// of the sides along each dimension. `amplitude` specifies the brightness of the cross.
///
/// `out` must be forged, scalar, and of a floating-point type.
DIP_EXPORT void FTCross(
      Image& out,
      FloatArray length = { 1 },
      dfloat amplitude = 1
);
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image FTCross(
      UnsignedArray const& sizes,
      FloatArray length = { 1 },
      dfloat amplitude = 1
) {
   Image out( sizes, 1, DT_SFLOAT );
   FTCross( out, std::move( length ), amplitude );
   return out;
}

/// \brief Generates the Fourier transform of a Gaussian.
///
/// The size of the Gaussian is specified with `sigma` (note that the Fourier transform of a Gaussian is also a
/// Gaussian). `volume` is the integral of the Gaussian in the spatial domain.
///
/// `out` must be forged, scalar, and of a floating-point type.
DIP_EXPORT void FTGaussian(
      Image& out,
      FloatArray sigma,
      dfloat amplitude = 1,
      dfloat truncation = 3
);
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image FTGaussian(
      UnsignedArray const& sizes,
      FloatArray sigma,
      dfloat amplitude = 1,
      dfloat truncation = 3
) {
   Image out( sizes, 1, DT_SFLOAT );
   FTGaussian( out, std::move( sigma ), amplitude, truncation );
   return out;
}


/// \brief Describes the parameters for a test object, used by \ref dip::TestObject.
struct TestObjectParams {
   // Object description
   String objectShape = S::ELLIPSOID;        ///< Can be `"ellipsoid"`, `"ellipsoid shell"`, `"box"`, `"box shell"`, or `"custom"`.
   FloatArray objectSizes = { 10 };          ///< Sizes of the object along each dimension.
   dfloat objectAmplitude = 1;               ///< Brightness of object pixels.
   bool randomShift = false;                 ///< If true, add a random sub-pixel shift in the range [-0.5,0.5].
   String generationMethod = S::GAUSSIAN;    ///< Can be `"gaussian"` (spatial domain method) or `"fourier"` (frequency domain method).
   // Optional sine modulation
   dfloat modulationDepth = 0;               ///< Strength of modulation, if 0 no modulation is applied.
   FloatArray modulationFrequency = {};      ///< Frequency of a sine modulation added to the object, units are periods/pixel.
   // Optional PDF blurring
   String pointSpreadFunction = S::NONE;     ///< PSF, can be `"gaussian"`, `"incoherent"`, or `"none"`.
   dfloat oversampling = 1;                  ///< Determines size of PSF (Gaussian PSF has sigma = 0.9*oversampling).
   // Optional noise added
   dfloat backgroundValue = 0.01;            ///< Background intensity, must be non-negative.
   dfloat signalNoiseRatio = 0;              ///< SNR = average object energy divided by average noise power. If SNR > 0, adds a mixture of Gaussian and Poisson noise.
   dfloat gaussianNoise = 1;                 ///< Relative amount of Gaussian noise.
   dfloat poissonNoise = 1;                  ///< Relative amount of Poisson noise.
};

/// \brief Generates a test object according to `params`.
///
/// Generates a test object in the center of `out`, which must be forged, scalar and of a floating-point type.
/// The test object can optionally be modulated using a sine function, blurred, and have noise added.
///
/// `params` describes how the object is generated:
///
/// - `params.generationMethod` can be one of:
///     - `"gaussian"`: creates the shape directly in the spatial domain, the shape will have Gaussian edges with
///       a sigma of 0.9.
///     - `"fourier"`: creates the shape in the frequency domain, the shape will be truly bandlimited.
/// - `params.objectShape` can be one of:
///     - `"ellipsoid"` or `"ellipsoid shell"`: the shape is drawn with \ref dip::DrawBandlimitedBall or
///       \ref dip::FTEllipsoid, depending on the generation method. In the case of `"gaussian"` (spatial-domain
///       generation), the shape must be isotropic (have same sizes in all dimensions). In the case of `"fourier"`,
///       the image cannot have more than three dimensions.
///     - `"box"` or `"box shell"`: the shape is drawn with \ref dip::DrawBandlimitedBox or
///       \ref dip::FTBox, depending on the generation method.
///     - `"custom"`: `out` already contains a shape, which is used as-is. In the case that `params.generationMethod`
///       is `"gaussian"`, `out` is taken to be in the spatial domain, and in the case of `"fourier"`, in the
///       frequency domain.
/// - `params.objectSizes` determines the extent of the object along each dimension. Must have either one element
///   or as many elements as image dimensions in `out`.
/// - `params.objectAmplitude` determines the brightness of the object.
/// - `params.randomShift`, if `true`, shifts the object with a random sub-pixel shift in the range [-0.5,0.5].
///   This sub-pixel shift can be used to avoid bias due to digitization error over a sequence of generated objects.
///
/// `params` also describes what effects are applied to the image:
///
/// Modulation is an additive sine wave along each dimension, and is controlled by:
///
/// - `params.modulationDepth` controls the strength of the modulation. If this value is zero, no modulation is applied.
/// - `params.modulationFrequency` controls the frequency along each image axis. The units are number of periods per
///   pixel, and hence values below 0.5 should be given to prevent aliasing.
///
/// Blurring is controlled by:
///
/// - `params.pointSpreadFunction` determines the point spread function (PSF) used. It can be `"gaussian"` for
///    Gaussian blurring, `"incoherent"` for a 2D, in-focus, diffraction limited incoherent PSF (applied through
///    Fourier domain filtering), or `"none"` for no blurring.
/// - `params.oversampling` determines the size of the PSF. In the case of `"gaussian"`, the sigma used for blurring
///   is `0.9 * params.oversampling`. In the case of `"incoherent"`, this is the `oversampling` parameter passed to
///   \ref dip::IncoherentOTF.
///
/// Noise is controlled by:
///
/// - `params.backgroundValue` determines the background intensity added to the image. This is relevant for the
///   Poisson noise.
/// - `params.signalNoiseRatio` determines the signal to noise ratio (SNR), which we define as the average object
///   energy divided by average noise power (i.e. not in dB). If the SNR is larger than 0, a mixture of Gaussian
///   and Poisson noise is added to the whole image.
/// - `params.gaussianNoise` determines the relative amount of Gaussian noise used.
/// - `params.poissonNoise` determines the relative amount of Poisson noise used. The magnitude of these two
///   quantities is not relevant, only their relative values are. If they are equal, the requested SNR is divided
///   equally between the Gaussian and the Poisson noise.
///
/// `random` is the random number generator used for both the sub-pixel shift and the noise added to the image.
DIP_EXPORT void TestObject(
      Image& out,
      TestObjectParams const& params,
      Random& random
);
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image TestObject(
      UnsignedArray const& sizes,
      TestObjectParams const& params,
      Random& random
) {
   Image out( sizes, 1, DT_SFLOAT );
   TestObject( out, params, random );
   return out;
}
/// \brief Calls the main \ref dip::TestObject function with a default-initialized \ref dip::Random object.
inline void TestObject(
      Image& out,
      TestObjectParams const& params = {}
) {
   Random random;
   TestObject( out, params, random );
}
/// \brief Overload for the function above, which takes image sizes instead of an image.
DIP_NODISCARD inline Image TestObject(
      UnsignedArray const& sizes = { 256, 256 },
      TestObjectParams const& params = {}
) {
   Random random;
   return TestObject( sizes, params, random );
}

/// \brief Fills the binary image `out` with a Poisson point process of `density`.
///
/// `out` must be forged, binary and scalar. On average, one of every `1/density` pixels will be set.
void FillPoissonPointProcess(
      Image& out,
      Random& random,
      dfloat density = 0.01
);

/// \brief Creates a binary image with a Poisson point process of `density`.
///
/// `out` will be of size `sizes`, binary and scalar.
inline void CreatePoissonPointProcess(
      Image& out,
      UnsignedArray const& sizes,
      Random& random,
      dfloat density = 0.01
) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_BIN, Option::AcceptDataTypeChange::DONT_ALLOW ));
   DIP_STACK_TRACE_THIS( FillPoissonPointProcess( out, random, density ));
}
DIP_NODISCARD inline Image CreatePoissonPointProcess(
      UnsignedArray const& sizes,
      Random& random,
      dfloat density = 0.01
) {
   Image out;
   CreatePoissonPointProcess( out, sizes, random, density );
   return out;
}

/// \brief Fills the binary image `out` with a grid that is randomly placed over the image.
///
/// This grid can be useful for random systematic sampling.
///
/// `type` determines the grid type. It can be `"rectangular"` in any number of dimensions, this is the default grid.
/// For 2D images it can be `"hexagonal"`. In 3D it can be `"fcc"` or `"bcc"` for face-centered cubic and body-centered
/// cubic, respectively.
///
/// `density` determines the grid density. On average, one of every `1/density` pixels will be set. The grid is
/// sampled equally densely along all dimensions. If the density doesn't lead to an integer grid spacing, the grid
/// locations will be rounded, leading to an uneven spacing. `density` must be such that the grid spacing is at
/// least 2. Therefore, `density` must be smaller than $\frac{1}{2^d}$, with $d$ the image dimensionality,
/// in the rectangular case. In the hexagonal case, this is $\frac{1}{2 \sqrt 3} \approx 0.2887$.
///
/// `mode` determines how the random grid location is determined. It can be either `"translation"` or `"rotation"`.
/// In the first case, only a random translation is applied to the grid, it will be aligned with the image axes.
/// In the second case, the grid will also be randomly rotated. This option is used only for 2D and 3D grids.
///
/// `out` must be forged, binary and scalar.
DIP_EXPORT void FillRandomGrid(
      Image& out,
      Random& random,
      dfloat density = 0.01,
      String const& type = S::RECTANGULAR,
      String const& mode = S::TRANSLATION
);

/// \brief Creates a binary image with a random grid.
///
/// `out` will be of size `sizes`, binary and scalar.
///
/// See \ref dip::FillRandomGrid for the meaning of the remainder of the parameters, which define the grid.
inline void CreateRandomGrid(
      Image& out,
      UnsignedArray const& sizes,
      Random& random,
      dfloat density = 0.01,
      String const& type = S::RECTANGULAR,
      String const& mode = S::TRANSLATION
) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_BIN, Option::AcceptDataTypeChange::DONT_ALLOW ));
   DIP_STACK_TRACE_THIS( FillRandomGrid( out, random, density, type, mode ));
}
DIP_NODISCARD inline Image CreateRandomGrid(
      UnsignedArray const& sizes,
      Random& random,
      dfloat density = 0.01,
      String const& type = S::RECTANGULAR,
      String const& mode = S::TRANSLATION
) {
   Image out;
   CreateRandomGrid( out, sizes, random, density, type, mode );
   return out;
}

/// \endgroup


/// \group generation_coordinates Coordinate generation
/// \ingroup generation
/// \brief Generating images with coordinates
/// \addtogroup

/// \brief Fills an image with a ramp function.
///
/// The ramp function increases along dimension `dimension`, and is
/// equivalent to the cartesian coordinate for dimension `dimension`. `dimension` must be
/// one of the dimensions of `out`. `out` must be forged, scalar, and of a real type.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
DIP_EXPORT void FillRamp( Image& out, dip::uint dimension, StringSet const& mode = {} );

/// \brief Creates a ramp function image.
///
/// The ramp function increases along dimension `dimension`, and is equivalent to the cartesian
/// coordinate for dimension `dimension`. `dimension` must be smaller than `sizes.size()`.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT. All dimensions except
/// for `dimension` will be \ref singleton_expansion "expanded singleton dimensions". That is, the
/// output image only stores `sizes[dimension]` pixels.
///
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateRamp(
      Image& out,
      UnsignedArray const& sizes,
      dip::uint dimension,
      StringSet const& mode = {}
) {
   UnsignedArray trueSizes( sizes.size(), 1 );
   if( dimension < sizes.size() ) {
      trueSizes[ dimension ] = sizes[ dimension ];
   }
   DIP_STACK_TRACE_THIS( out.ReForge( trueSizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillRamp( out, dimension, mode ));
   out.ExpandSingletonDimensions( sizes );
}
DIP_NODISCARD inline Image CreateRamp(
      UnsignedArray const& sizes,
      dip::uint dimension,
      StringSet const& mode = {}
) {
   Image out;
   CreateRamp( out, sizes, dimension, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the x-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the x-axis.
/// `out` must be forged, scalar, and of a real type.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void FillXCoordinate( Image& out, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( FillRamp( out, 0, mode ));
}

/// \brief Creates a ramp function image.
///
/// The ramp function increases along the x-axis, and is equivalent to the cartesian coordinate
/// for the x-axis.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT. All dimensions except
/// for `dimension` will be \ref singleton_expansion "expanded singleton dimensions". That is, the
/// output image only stores `sizes[dimension]` pixels.
///
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateXCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( CreateRamp( out, sizes, 0, mode ));
}
DIP_NODISCARD inline Image CreateXCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateXCoordinate( out, sizes, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the y-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the y-axis.
/// `out` must be forged, scalar, of a real type, and have at least two dimensions.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void FillYCoordinate( Image& out, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( FillRamp( out, 1, mode ));
}

/// \brief Creates a ramp function image.
///
/// The ramp function increases along the y-axis, and is equivalent to the cartesian coordinate
/// for the y-axis. `size` must have at least two elements.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT. All dimensions except
/// for `dimension` will be \ref singleton_expansion "expanded singleton dimensions". That is, the
/// output image only stores `sizes[dimension]` pixels.
///
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateYCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( CreateRamp( out, sizes, 1, mode ));
}
DIP_NODISCARD inline Image CreateYCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateYCoordinate( out, sizes, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the z-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the z-axis.
/// `out` must be forged, scalar, of a real type, and have at least three dimensions.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void FillZCoordinate( Image& out, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( FillRamp( out, 2, mode ));
}

/// \brief Creates a ramp function image.
///
/// The ramp function increases along the z-axis, and is equivalent to the cartesian coordinate
/// for the z-axis. `sizes` must have at least three elements.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT. All dimensions except
/// for `dimension` will be \ref singleton_expansion "expanded singleton dimensions". That is, the
/// output image only stores `sizes[dimension]` pixels.
///
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateZCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( CreateRamp( out, sizes, 2, mode ));
}
DIP_NODISCARD inline Image CreateZCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateZCoordinate( out, sizes, mode );
   return out;
}


/// \brief Fills an image with the distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, and of a real type.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
DIP_EXPORT void FillRadiusCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image filled with the distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateRadiusCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillRadiusCoordinate( out, mode ));
}
DIP_NODISCARD inline Image CreateRadiusCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateRadiusCoordinate( out, sizes, mode );
   return out;
}


/// \brief Fills an image with the square distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, and of a real type.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
DIP_EXPORT void FillRadiusSquareCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image filled with the square distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateRadiusSquareCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillRadiusSquareCoordinate( out, mode ));
}
DIP_NODISCARD inline Image CreateRadiusSquareCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateRadiusSquareCoordinate( out, sizes, mode );
   return out;
}


/// \brief Fills an image with the angle to the x-axis within the x-y plane.
///
/// The angle function is equivalent to the phi component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, of a real type, and have two or three dimensions.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
DIP_EXPORT void FillPhiCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image filled with the angle to the x-axis within the x-y plane.
///
/// The angle function is equivalent to the phi component of the polar or spherical
/// coordinate system. `size` must have two or three elements.
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreatePhiCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillPhiCoordinate( out, mode ));
}
DIP_NODISCARD inline Image CreatePhiCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreatePhiCoordinate( out, sizes, mode );
   return out;
}


/// \brief Fills an image with the angle to the z-axis.
///
/// The angle function is equivalent to the theta component of the spherical coordinate system.
/// `out` must be forged, scalar, of a real type, and have three dimensions.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
DIP_EXPORT void FillThetaCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image filled with the angle to the z-axis.
///
/// The angle function is equivalent to the theta component of the spherical coordinate
/// system. `size` must have three elements.
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
/// See \ref dip::FillCoordinates for the meaning of `mode`.
inline void CreateThetaCoordinate( Image& out, UnsignedArray const& sizes, StringSet const& mode = {} ) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillThetaCoordinate( out, mode ));
}
DIP_NODISCARD inline Image CreateThetaCoordinate( UnsignedArray const& sizes, StringSet const& mode = {} ) {
   Image out;
   CreateThetaCoordinate( out, sizes, mode );
   return out;
}


/// \brief Fills an image with the coordinates of each pixel.
///
/// `system` determines the coordinate system, and `mode` further defines the origin and
/// scaling of the coordinate system, as described below.
///
/// `out` must be forged, of a real type, and have as many tensor elements as spatial
/// dimensions.
///
/// `system` determines the coordinate system. It is one of the following strings:
///
/// - `"cartesian"`: Uses cartesian coordinates.
/// - `"spherical"`: Uses polar (2D) or spherical (3D) coordinates. The image must have
///   two or three dimensions.
///
/// `mode` specifies the origin and scaling of the coordinates. It can contain one of the
/// following strings:
///
/// - `"right"`: The origin is on the pixel right of the center (at integer division result of
///   `size/2`). This is the default if no other option is given.
/// - `"left"`: The origin is on the pixel left of the center (at integer division result of
///   `(size-1)/2`).
/// - `"true"`: The origin is halfway the first and last pixel, in between pixels if necessary
///   (at floating-point division result of `size/2`).
/// - `"corner"`: The origin is on the first pixel.
/// - `"frequency"`: The coordinates used are as for the Fourier transform.
///   The origin is as for `"right"`, and the coordinates are in the range [0.5,0.5).
///
/// Additionally, `mode` can contain the following strings:
///
/// - `"math"`: The y axis is inverted, such that it increases upwards.
/// - `"radial"`: In combination with "frequency", changes the range to [-&pi;,&pi;), as with radial
///   frequencies.
/// - `"physical"`: The coordinate system is in physical units rather than providing indices.
///   That is, instead of unit increments between pixels, the pixel size magnitudes are used to
///   scale distances. Units are ignored, so if they differ, polar/spherical coordinates might
///   not make sense.
///   In combination with `"frequency"`, yields the same result as in combination with `"right"`.
///
/// The string `"radfreq"` is equivalent to both `"frequency"` and `"radial"`.
DIP_EXPORT void FillCoordinates(
      Image& out,
      StringSet const& mode = {},
      String const& system = S::CARTESIAN
);

/// \brief Creates an image filled with the coordinates of each pixel.
///
/// `out` will be of size `sizes`, with `sizes.size()` vector elements, and of type \ref dip::DT_SFLOAT.
///
/// See \ref dip::FillCoordinates for the meaning of `mode` and `system`.
inline void CreateCoordinates(
      Image& out,
      UnsignedArray const& sizes,
      StringSet const& mode = {},
      String const& system = S::CARTESIAN
) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, sizes.size(), DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillCoordinates( out, mode, system ));
}
DIP_NODISCARD inline Image CreateCoordinates(
      UnsignedArray const& sizes,
      StringSet const& mode = {},
      String const& system = S::CARTESIAN
) {
   Image out;
   CreateCoordinates( out, sizes, mode, system );
   return out;
}


/// \brief Fills an image with the distance to a given point.
///
/// Computes the distance from each pixel in `out` to the coordinates specified through `point`, which can be
/// outside of the image. The `scaling` parameter may be used to specify the relative distance between pixels
/// in each dimension (the pixel sizes in `out` are ignored). Both `point` and `scaling` must have the same
/// number of elements as `out.Size()`, but `scaling` can also be empty (no scaling) or have a single element
/// (isotropic scaling).
///
/// `distance` indicates how the distance is computed, and can be `"Euclidean"`, `"square"` (for square Euclidean
/// distance), `"city"` (for city block or L1 distance), or `"chess"` (for chessboard or L-infinity distance).
///
/// `out` must be forged, real-valued and scalar.
DIP_EXPORT void FillDistanceToPoint(
      Image& out,
      FloatArray const& point,
      String const& distance = S::EUCLIDEAN,
      FloatArray scaling = {}
);

/// \brief Creates an image filled with the distance to a given point.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
///
/// See \ref dip::FillDistanceToPoint for the meaning of `point`, `distance` and `scaling`.
inline void DistanceToPoint(
      Image& out,
      UnsignedArray const& sizes,
      FloatArray const& point,
      String const& distance = S::EUCLIDEAN,
      FloatArray scaling = {}
) {
   DIP_STACK_TRACE_THIS( out.ReForge( sizes, 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   DIP_STACK_TRACE_THIS( FillDistanceToPoint( out, point, distance, std::move( scaling )));
}
DIP_NODISCARD inline Image DistanceToPoint(
      UnsignedArray const& sizes,
      FloatArray const& point,
      String const& distance = S::EUCLIDEAN,
      FloatArray scaling = {}
) {
   Image out;
   DistanceToPoint( out, sizes, point, distance, std::move( scaling ));
   return out;
}

/// \brief Creates an image filled with the Euclidean distance to a given point.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
///
/// See \ref dip::FillDistanceToPoint for the meaning of `point` and `scaling`.
inline void EuclideanDistanceToPoint(
      Image& out,
      UnsignedArray const& sizes,
      FloatArray const& point,
      FloatArray scaling = {}
) {
   DIP_STACK_TRACE_THIS( DistanceToPoint( out, sizes, point, S::EUCLIDEAN, std::move( scaling )));
}
DIP_NODISCARD inline Image EuclideanDistanceToPoint(
      UnsignedArray const& sizes,
      FloatArray const& point,
      FloatArray scaling = {}
) {
   Image out;
   EuclideanDistanceToPoint( out, sizes, point, std::move( scaling ));
   return out;
}

/// \brief Creates an image filled with the city block distance to a given point.
///
/// `out` will be of size `sizes`, scalar, and of type \ref dip::DT_SFLOAT.
///
/// See \ref dip::FillDistanceToPoint for the meaning of `point` and `scaling`.
inline void CityBlockDistanceToPoint(
      Image& out,
      UnsignedArray const& sizes,
      FloatArray const& point,
      FloatArray scaling = {}
) {
   DIP_STACK_TRACE_THIS( DistanceToPoint( out, sizes, point, S::CITY, std::move( scaling )));
}
DIP_NODISCARD inline Image CityBlockDistanceToPoint(
      UnsignedArray const& sizes,
      FloatArray const& point,
      FloatArray scaling = {}
) {
   Image out;
   CityBlockDistanceToPoint( out, sizes, point, std::move( scaling ));
   return out;
}

/// \endgroup


/// \group generation_noise Noise generation
/// \ingroup generation
/// \brief Adding noise to an image
/// \addtogroup

/// \brief Adds uniformly distributed white noise to the input image.
///
/// The uniformly distributed noise added to the image is taken from the half-open interval
/// [`lowerBound`, `upperBound`). That is, for each pixel it does
/// `in += uniformRandomGenerator( lowerBound, upperBound )`. The output image is of the same type as the input image.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a \ref dip::Random object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::UniformRandomGenerator
DIP_EXPORT void UniformNoise(
      Image const& in,
      Image& out,
      Random& random,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 1.0
);
DIP_NODISCARD inline Image UniformNoise(
      Image const& in,
      Random& random,
      dfloat lowerBound = 0.0,
      dfloat upperBound = 1.0
) {
   Image out;
   UniformNoise( in, out, random, lowerBound, upperBound );
   return out;
}

/// \brief Adds normally distributed white noise to the input image.
///
/// The normally distributed noise added to the image is defined by `variance`, and has a zero mean. That is,
/// for each pixel it does `in += gaussianRandomGenerator( 0, std::sqrt( variance ))`. The output image is of the
/// same type as the input image.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a \ref dip::Random object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::GaussianRandomGenerator
DIP_EXPORT void GaussianNoise( Image const& in, Image& out, Random& random, dfloat variance = 1.0 );
DIP_NODISCARD inline Image GaussianNoise( Image const& in, Random& random, dfloat variance = 1.0 ) {
   Image out;
   GaussianNoise( in, out, random, variance );
   return out;
}

/// \brief Adds Poisson-distributed white noise to the input image.
///
/// The Poisson-distributed noise is added to the image scaled by `conversion`. That is,
/// for each pixel it does `in = poissonRandomGenerator( in * conversion ) / conversion`.
/// `conversion` can be used to relate the pixel values with the number of counts. For example, the simulate a
/// photon-limited image acquired by a CCD camera, the conversion factor specifies the relation between the number
/// of photons recorded and the pixel value. Note that the input pixel values must be positive for the noise
/// to be generated. Pixels with a value of 0 or less will always result in an output value of 0.
///
/// The output image is of the same type as the input image.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a \ref dip::Random object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::PoissonRandomGenerator
DIP_EXPORT void PoissonNoise( Image const& in, Image& out, Random& random, dfloat conversion = 1.0 );
DIP_NODISCARD inline Image PoissonNoise( Image const& in, Random& random, dfloat conversion = 1.0 ) {
   Image out;
   PoissonNoise( in, out, random, conversion );
   return out;
}

/// \brief Adds noise to the binary input image.
///
/// The noise added to the binary image is described by the two probabilities `p10` and `p01`. `p10` is the
/// probability that a foreground pixel transitions to background (probability of 1 &rarr; 0 transition), and `p01`
/// is the probability that a background pixel transitions to foreground (probability to 0 &rarr; 1 transition).
/// Thus, `p10` indicates the probability for each foreground pixel in the input image to be set to background,
/// and `p01` indicates the probability that a background pixel in the input image is set to foreground. It is
/// possible to set either of these to 0, to limit the noise to only one of the phases: for example,
/// `BinaryNoise( in, random, 0.05, 0.0 )` limits the noise to the foreground components, and does not add noise to
/// the background.
///
/// Note that the noise generated corresponds to a Poisson point process. The distances between changed pixels
/// have a Poisson distribution. For example, the following bit of code yields a Poisson process with a density
/// governed by `D`.
///
/// ```cpp
/// dip::Random random;
/// dip::dfloat D = 0.001;
/// dip::Image poissonPoint( { 256, 256 }, 1, dip::DT_BIN );
/// poissonPoint.Fill( 0 );
/// BinaryNoise( poissonPoint, poissonPoint, random, 0, D );
/// ```
///
/// The binary noise added to an all-zero image as in the code snippet above is equivalent to thresholding
/// uniformly-distributed noise at a fraction `D` of the noise range, from the max value:
///
/// ```cpp
/// dip::Image poissonPoint2( { 256, 256 }, 1, dip::DT_SFLOAT );
/// poissonPoint2.Fill( 0 );
/// UniformNoise( poissonPoint2, poissonPoint2, random, 0, 1 );
/// poissonPoint2 = poissonPoint2 >= ( 1 - D );
/// ```
///
/// Using blue noise it is possible to create a point process with similar density, but more equally-distributed
/// points:
///
/// ```cpp
/// dip::Image poissonPoint3( { 256, 256 }, 1, dip::DT_SFLOAT );
/// FillColoredNoise( poissonPoint3, random, 1, 1 );
/// dip::dfloat threshold = Percentile( poissonPoint3, {}, 100 * ( 1 - D )).As< dip::dfloat >();
/// poissonPoint3 = poissonPoint3 >= threshold;
/// ```
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a \ref dip::Random object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::BinaryRandomGenerator
DIP_EXPORT void BinaryNoise(
      Image const& in,
      Image& out,
      Random& random,
      dfloat p10 = 0.05,
      dfloat p01 = 0.05
);
DIP_NODISCARD inline Image BinaryNoise(
      Image const& in,
      Random& random,
      dfloat p10 = 0.05,
      dfloat p01 = 0.05
) {
   Image out;
   BinaryNoise( in, out, random, p10, p01 );
   return out;
}

/// \brief Adds salt-and-pepper noise to the input image.
///
/// The noise added to the image is described by the two probabilities `p0` and `p1`. `p0` is the
/// probability that a pixel is set to 0 (black), and `p01` is the probability that a pixel is set to `white`.
/// It is possible to set either of these to 0, to limit the noise to only one of the phases: for example,
/// `SaltPepperNoise( in, random, 0.05, 0.0 )` adds only black pixels to the image, not white ones. `p0+p1`
/// must not be larger than 1.
///
/// Note that the noise generated corresponds to a Poisson point process. The distances between changed pixels
/// have a Poisson distribution.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a \ref dip::Random object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::UniformRandomGenerator
DIP_EXPORT void SaltPepperNoise(
      Image const& in,
      Image& out,
      Random& random,
      dfloat p0 = 0.05,
      dfloat p1 = 0.05,
      dfloat white = 1.0
);
DIP_NODISCARD inline Image SaltPepperNoise(
      Image const& in,
      Random& random,
      dfloat p0 = 0.05,
      dfloat p1 = 0.05,
      dfloat white = 1.0
) {
   Image out;
   SaltPepperNoise( in, out, random, p0, p1, white );
   return out;
}

/// \brief Fills `out` with colored (Brownian, pink, blue, violet) noise.
///
/// Colored noise is correlated, as opposed to white nose, which is uncorrelated.
///
/// The output image will have a variance of `variance`. `color` indicates the color of the noise (and is equal to
/// the power of the function used to modulate the frequency spectrum):
///
/// - -2.0: Brownian noise (a.k.a. brown or red noise), with a frequency spectrum proportional to $1/f^2$.
/// - -1.0: pink noise, with a frequency spectrum proportional to $1/f$.
/// - 0.0: white noise, equal to \ref "dip::GaussianNoise" (but much more expensive).
/// - 1.0: blue noise, with a frequency spectrum proportional to $f$.
/// - 2.0: violet noise, with a frequency spectrum proportional to $f^2$.
///
/// It is possible to specify any values in between these, to tune the color more precisely. Values larger than
/// 2.0 and smaller than -2.0 are possible also, but the results become less interesting quickly as the magnitude
/// increases.
///
/// With pink and Brownian noise, nearby pixels will be positively correlated. That is, the noise changes slowly
/// across the image. This is because it has more power in the lower frequencies, which represent slow changes.
/// These forms of noise can add texture to an image. The variance of the output image is given by `variance`, but
/// the computed population variance will differ from it more strongly than with white noise. The differences are
/// stronger for smaller images.
///
/// With blue and violet noise, nearby pixels will be negatively correlated. That is, large-scale changes across
/// the image are weaker. The resulting noise looks more uniform than white noise. Because of this, the computed
/// population variance in the output will be much closer to `variance` than with white noise.
DIP_EXPORT void FillColoredNoise(
      Image& out,
      Random& random,
      dfloat variance = 1.0,
      dfloat color = -2.0
);

/// \brief Adds colored (Brownian, pink, blue, violet) noise to `in`.
///
/// Equivalent to adding the output of \ref dip::FillColoredNoise to `in`. See the reference for that function for
/// information on the input parameters. `out` will have the data type of `in`.
inline void ColoredNoise(
      Image const& in,
      Image& out,
      Random& random,
      dfloat variance = 1.0,
      dfloat color = -2.0
) {
   DIP_STACK_TRACE_THIS( out.ReForge( in.Sizes(), in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW ));
   out.CopyNonDataProperties( in );
   DIP_STACK_TRACE_THIS( FillColoredNoise( out, random, variance, color ));
   DIP_STACK_TRACE_THIS( out += in );
}
DIP_NODISCARD inline Image ColoredNoise(
      Image const& in,
      Random& random,
      dfloat variance = 1.0,
      dfloat color = -2.0
) {
   Image out;
   ColoredNoise( in, out, random, variance, color );
   return out;
}

/// \endgroup


// Inline body of function declared above
inline void FillPoissonPointProcess(
      Image& out,
      Random& random,
      dfloat density
) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !out.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   out.Fill( 0 );
   DIP_STACK_TRACE_THIS( BinaryNoise( out, out, random, 0, density ));
}


} // namespace dip

#endif // DIP_GENERATION_H
