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
#include "random.h"


/// \file
/// \brief Declares functions for generating image data.


namespace dip {


/// \defgroup generation Image generation
/// \brief Functions for filling images with generated data, and creating test images
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
   FillDelta( out, origin );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateDelta( Image const& in, String const& origin = "" ) {
   Image out;
   CreateDelta( in, out, origin );
   return out;
}


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
   FillRamp( out, dimension, mode );
   out.SetPixelSize( in.PixelSize() );
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
   FillRadiusCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateRadiusCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateRadiusCoordinate( in, out, mode );
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
   FillPhiCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
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
   FillThetaCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
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
DIP_EXPORT void FillCoordinates( Image& out, StringSet const& mode = {}, String const& system = "" );

/// \brief Creates an image of the same size as `in`, filled with the coordinates of each pixel.
///
/// `out` will have the same sizes as `in`, will be a vector image with as many vector elements as
/// spatial dimensions, and will be of type `dip::DT_SFLOAT`.
/// The pixel size of `in` will be copied over.
///
/// See `dip::FillCoordinates` for the meaning of `mode` and `system`.
inline void CreateCoordinates( Image const& in, Image& out, StringSet const& mode = {}, String const& system = "" ) {
   out.ReForge( in.Sizes(), in.Dimensionality(), DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   FillCoordinates( out, mode, system );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateCoordinates( Image const& in, StringSet const& mode = {}, String const& system = "" ) {
   Image out;
   CreateCoordinates( in, out, mode, system );
   return out;
}


/// \}


} // namespace dip


#endif // DIP_GENERATION_H
