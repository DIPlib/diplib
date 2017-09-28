/*
 * DIPlib 3.0
 * This file contains declarations for functions that generate image data
 *
 * (c)2017, Cris Luengo.
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

#include "diplib.h"
#include "diplib/random.h"


/// \file
/// \brief Functions for generating image data.
/// \see generation


namespace dip {


/// \defgroup generation Image generation
/// \brief Filling images with generated data, and creating test images.
/// \{


/// \brief Fills an image with a delta function.
///
/// `out` must be forged, and scalar.
/// All pixels will be zero except at the origin, where it will be 1.
///
/// `origin` specifies where the origin lies:
///  - `"right"`: The origin is on the pixel right of the center (at integer division result of
///    `size/2`). This is the default.
///  - `"left"`: The origin is on the pixel left of the center (at integer division result of
///    `(size-1)/2`).
///  - `"corner"`: The origin is on the first pixel. This is the default if no other option is given.
DIP_EXPORT void FillDelta( Image& out, String const& origin = "" );

/// \brief Creates a delta function image of the same size as `in`.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// All pixels will be zero except at the origin, where it will be 1.
/// See `dip::FillDelta` for the meaning of `origin`.
inline void CreateDelta( Image const& in, Image& out, String const& origin = "" ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillDelta( out, origin );
}
inline Image CreateDelta( Image const& in, String const& origin = "" ) {
   Image out;
   CreateDelta( in, out, origin );
   return out;
}


/// \brief Sets the pixels at the border of `out` to `value`. `size` indicates how many pixels in from the
/// border are set.
DIP_EXPORT void SetBorder( Image& out, Image::Pixel value = { 0 }, dip::uint size = 1 );


/// \brief Fills an image with a ramp function.
///
/// The ramp function increases along dimension `dimension`, and is
/// equivalent to the cartesian coordinate for dimension `dimension`. `dimension` must be
/// one of the dimensions of `out`. `out` must be forged, scalar, and of a real type.
/// See `dip::FillCoordinates` for the meaning of `mode`.
DIP_EXPORT void FillRamp( Image& out, dip::uint dimension, StringSet const& mode = {} );

/// \brief Creates a ramp function image of the same size as `in`.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The ramp function increases along dimension `dimension`, and is equivalent to the cartesian
/// coordinate for dimension `dimension`. `dimension` must be one of the dimensions of `out`.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateRamp( Image const& in, Image& out, dip::uint dimension, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillRamp( out, dimension, mode );
}
inline Image CreateRamp( Image const& in, dip::uint dimension, StringSet const& mode = {} ) {
   Image out;
   CreateRamp( in, out, dimension, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the x-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the x-axis.
/// `out` must be forged, scalar, and of a real type.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void FillXCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 0, mode );
}

/// \brief Creates a ramp function image of the same size as `in`.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The ramp function increases along the x-axis, and is equivalent to the cartesian coordinate
/// for the x-axis.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateXCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   CreateRamp( in, out, 0, mode );
}
inline Image CreateXCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateXCoordinate( in, out, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the y-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the y-axis.
/// `out` must be forged, scalar, of a real type, and have at least two dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void FillYCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 1, mode );
}

/// \brief Creates a ramp function image of the same size as `in`.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The ramp function increases along the y-axis, and is equivalent to the cartesian coordinate
/// for the y-axis. `in` must have at least two dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateYCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   CreateRamp( in, out, 1, mode );
}
inline Image CreateYCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateYCoordinate( in, out, mode );
   return out;
}

/// \brief Fills an image with a ramp function that increases along the z-axis.
///
/// The ramp function is equivalent to the cartesian coordinate for the z-axis.
/// `out` must be forged, scalar, of a real type, and have at least three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void FillZCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 2, mode );
}

/// \brief Creates a ramp function image of the same size as `in`.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The ramp function increases along the z-axis, and is equivalent to the cartesian coordinate
/// for the z-axis. `in` must have at least three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateZCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   CreateRamp( in, out, 2, mode );
}
inline Image CreateZCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateZCoordinate( in, out, mode );
   return out;
}


/// \brief Fills an image with the distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, and of a real type.
/// See `dip::FillCoordinates` for the meaning of `mode`.
DIP_EXPORT void FillRadiusCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image of the same size as `in`, filled with the distance to the origin.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateRadiusCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillRadiusCoordinate( out, mode );
}
inline Image CreateRadiusCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateRadiusCoordinate( in, out, mode );
   return out;
}


/// \brief Fills an image with the square distance to the origin.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, and of a real type.
/// See `dip::FillCoordinates` for the meaning of `mode`.
DIP_EXPORT void FillRadiusSquareCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image of the same size as `in`, filled with the square distance to the origin.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The distance function is equivalent to the radius component of the polar or spherical
/// coordinate system.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateRadiusSquareCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillRadiusSquareCoordinate( out, mode );
}
inline Image CreateRadiusSquareCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateRadiusSquareCoordinate( in, out, mode );
   return out;
}


/// \brief Fills an image with the angle to the x-axis within the x-y plane.
///
/// The angle function is equivalent to the phi component of the polar or spherical
/// coordinate system.
/// `out` must be forged, scalar, of a real type, and have two or three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
DIP_EXPORT void FillPhiCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image of the same size as `in`, filled with the angle to the x-axis
/// within the x-y plane.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The angle function is equivalent to the phi component of the polar or spherical
/// coordinate system. `in` must have two or three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreatePhiCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillPhiCoordinate( out, mode );
}
inline Image CreatePhiCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreatePhiCoordinate( in, out, mode );
   return out;
}


/// \brief Fills an image with the angle to the z-axis.
///
/// The angle function is equivalent to the theta component of the spherical coordinate system.
/// `out` must be forged, scalar, of a real type, and have three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
DIP_EXPORT void FillThetaCoordinate( Image& out, StringSet const& mode = {} );

/// \brief Creates an image of the same size as `in`, filled with the angle to the z-axis.
///
/// `out` will have the same sizes as `in`, scalar, and of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// The angle function is equivalent to the theta component of the spherical coordinate
/// system. `in` must have three dimensions.
/// See `dip::FillCoordinates` for the meaning of `mode`.
inline void CreateThetaCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillThetaCoordinate( out, mode );
}
inline Image CreateThetaCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateThetaCoordinate( in, out, mode );
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
///  - `"cartesian"`: Uses cartesian coordinates.
///  - `"spherical"`: Uses polar (2D) or spherical (3D) coordinates. The image must have
///    two or three dimensions.
///
/// `mode` specifies the origin and scaling of the coordinates. If can contain one of the
/// following strings:
///  - `"right"`: The origin is on the pixel right of the center (at integer division result of
///    `size/2`). This is the default if no other option is given.
///  - `"left"`: The origin is on the pixel left of the center (at integer division result of
///    `(size-1)/2`).
///  - `"true"`: The origin is halfway the first and last pixel, in between pixels if necessary
///    (at floating-point division result of `size/2`).
///  - `"corner"`: The origin is on the first pixel.
///  - `"frequency"`: The coordinates used are as for the Fourier transform.
///    The origin is as for `"right"`, and the coordinates are in the range [0.5,0.5).
/// Additionally, `mode` can contain the following strings:
///  - `"math"`: The y axis is inverted, such that it increases upwards.
///  - `"radial"`: In combination with "frequency", changes the range to [-pi,pi), as with radial
///    frequencies.
///  - `"physical"`: The coordinate system is in phyisical units rather than providing indices.
///    That is, instead of unit increments between pixels, the pixel size magnitudes are used to
///    scale distances. Units are ignored, so if they differ, polar/spherical coordinates might
///    not make sense.
///    In combination with `"frequency"`, yields the same result as in combination with `"right"`.
/// The string `"radfreq"` is equivalent to both `"frequency"` and `"radial"`.
DIP_EXPORT void FillCoordinates( Image& out, StringSet const& mode = {}, String const& system = "cartesian" );

/// \brief Creates an image of the same size as `in`, filled with the coordinates of each pixel.
///
/// `out` will have the same sizes as `in`, will be a vector image with as many vector elements as
/// spatial dimensions, and will be of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// See `dip::FillCoordinates` for the meaning of `mode` and `system`.
inline void CreateCoordinates( Image const& in, Image& out, StringSet const& mode = {}, String const& system = "cartesian" ) {
   out.ReForge( in.Sizes(), in.Dimensionality(), DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.SetPixelSize( in.PixelSize() );
   FillCoordinates( out, mode, system );
}
inline Image CreateCoordinates( Image const& in, StringSet const& mode = {}, String const& system = "cartesian" ) {
   Image out;
   CreateCoordinates( in, out, mode, system );
   return out;
}


/// \brief Adds uniformly distributed white noise to the input image.
///
/// The uniformly distributed noise added to the image is in the range `lowerBound` to `upperBound`. That is,
/// for each pixel it does `in += uniformRandomGenerator( lowerBound, upperBound )`. The output image is of the
/// same type as the input image.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a `dip::Random` object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::UniformRandomGenerator.
DIP_EXPORT void UniformNoise( Image const& in, Image& out, Random& random, dfloat lowerBound = 0.0, dfloat upperBound = 1.0 );
inline Image UniformNoise( Image const& in, Random& random, dfloat lowerBound = 0.0, dfloat upperBound = 1.0 ) {
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
/// which is essentially a copy of `random` set to a different random stream. Given a `dip::Random` object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::GaussianRandomGenerator.
DIP_EXPORT void GaussianNoise( Image const& in, Image& out, Random& random, dfloat variance = 1.0 );
inline Image GaussianNoise( Image const& in, Random& random, dfloat variance = 1.0 ) {
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
/// of photons recorded and the pixel value.
///
/// The output image is of the same type as the input image.
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a `dip::Random` object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::PoissonRandomGenerator.
DIP_EXPORT void PoissonNoise( Image const& in, Image& out, Random& random, dfloat conversion = 1.0 );
inline Image PoissonNoise( Image const& in, Random& random, dfloat conversion = 1.0 ) {
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
/// `BinaryNoise( in, 0.05, 0.0 )` limits the noise to the foreground components, and does not add noise to
/// the background.
///
/// Note that the noise generated corresponds to a Poisson point process. The distances between changed pixels
/// have a Poisson distribution. For example, the following bit of code yields a Poisson process with a density
/// governed by `D`.
///
/// ```cpp
///     dip::Random random;
///     dip::dfloat D = 0.001;
///     dip::Image poissonPoint( { 256, 256 }, 1, dip::DT_BIN );
///     poissonPoint.Fill( 0 );
///     BinaryNoise( poissonPoint, poissonPoint, random, 0, D );
/// ```
///
/// The binary noise added to an all-zero image as in the code snippet above is equivalent to thresholding
/// uniformly-distributed noise at a fraction `D` of the noise range, from the max value:
///
/// ```cpp
///     dip::Image poissonPoint2( { 256, 256 }, 1, dip::DT_SFLOAT );
///     poissonPoint2.Fill( 0 );
///     UniformNoise( poissonPoint2, poissonPoint2, random, 0, 1 );
///     poissonPoint2 = poissonPoint2 >= ( 1 - D );
/// ```
///
/// Using blue noise it is possible to create a point process with similar density, but more equally-distributed
/// points:
///
/// ```cpp
///     dip::Image poissonPoint3( { 256, 256 }, 1, dip::DT_SFLOAT );
///     FillColoredNoise( poissonPoint3, random, 1, 1 );
///     dip::dfloat threshold = Percentile( poissonPoint3, {}, 100 * ( 1 - D )).As< dip::dfloat >();
///     poissonPoint3 = poissonPoint3 >= threshold;
/// ```
///
/// `random` is used to generate the random values needed by the first thread. If the algorithm runs in multiple
/// threads, portions of the image processed by additional threads take their random values from `random.Split()`,
/// which is essentially a copy of `random` set to a different random stream. Given a `dip::Random` object in an
/// identical state before calling this function, the output image will be different depending on the number of
/// threads used.
///
/// \see dip::BinaryRandomGenerator.
DIP_EXPORT void BinaryNoise( Image const& in, Image& out, Random& random, dfloat p10 = 0.05, dfloat p01 = 0.05 );
inline Image BinaryNoise( Image const& in, Random& random, dfloat p10 = 0.05, dfloat p01 = 0.05 ) {
   Image out;
   BinaryNoise( in, out, random, p10, p01 );
   return out;
}

/// \brief Fills `out` with colored (Brownian, pink, blue, violet) noise.
///
/// Colored noise is correlated, as opposed to white nose, which is uncorrelated.
///
/// The output image will have a variance of `variance`. `color` indicates the color of the noise (and is equal to
/// the power of the function used to modulate the frequency spectrum):
/// - -2.0: Brownian noise (a.k.a. brown or red noise), with a frequency spectrum proportional to \$1/f^2\$.
/// - -1.0: pink noise, with a frequency spectrum proportional to \$1/f\$.
/// - 0.0: white noise, equal to `dip::GaussianNoise` (but much more expensive).
/// - 1.0: blue noise, with a frequency spectrum proportional to \$f\$.
/// - 2.0: violet noise, with a frequency spectrum proportional to \$f^2\$.
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
DIP_EXPORT void FillColoredNoise( Image& out, Random& random, dfloat variance = 1.0, dfloat color = -2.0 );

/// \brief Adds colored (Brownian, pink, blue, violet) noise to `in`.
///
/// Equivalent to adding the output of `dip::FillColoredNoise` to `in`. See the reference for that function for
/// information on the input parameters. `out` will have the data type of `in`.
inline void ColoredNoise( Image const& in, Image& out, Random& random, dfloat variance = 1.0, dfloat color = -2.0 ) {
   out.ReForge( in.Sizes(), in.TensorElements(), in.DataType(), Option::AcceptDataTypeChange::DO_ALLOW );
   out.CopyNonDataProperties( in );
   FillColoredNoise( out, random, variance, color );
   out += in;
}
inline Image ColoredNoise( Image const& in, Random& random, dfloat variance = 1.0, dfloat color = -2.0 ) {
   Image out;
   ColoredNoise( in, out, random, variance, color );
   return out;
}


// TODO: functions to port:
/*
   dip_FTSphere (dip_generation.h)
   dip_FTBox (dip_generation.h)
   dip_FTCube (dip_generation.h)
   dip_FTGaussian (dip_generation.h)
   dip_FTEllipsoid (dip_generation.h)
   dip_FTCross (dip_generation.h)
   dip_EuclideanDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
   dip_EllipticDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
   dip_CityBlockDistanceToPoint (dip_generation.h) (related to dip::FillRadiusCoordinate)
   dip_TestObjectCreate (dip_generation.h)
   dip_TestObjectModulate (dip_generation.h)
   dip_TestObjectBlur (dip_generation.h)
   dip_TestObjectAddNoise (dip_generation.h)
   dip_ObjectCylinder (dip_generation.h)
   dip_ObjectEdge (dip_generation.h)
   dip_ObjectPlane (dip_generation.h)
   dip_ObjectEllipsoid (dip_generation.h)
   dip_DrawLineFloat (dip_paint.h) (merge into a single dip::DrawLine)
   dip_DrawLineComplex (dip_paint.h) (merge into a single dip::DrawLine)
   dip_DrawLinesFloat (dip_paint.h) (merge into a single dip::DrawLine)
   dip_DrawLinesComplex (dip_paint.h) (merge into a single dip::DrawLine)
   dip_PaintEllipsoid (dip_paint.h)
   dip_PaintDiamond (dip_paint.h)
   dip_PaintBox (dip_paint.h)
*/

/// \}


} // namespace dip


#endif // DIP_GENERATION_H
