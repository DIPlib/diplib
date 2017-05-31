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


/// \file
/// \brief Declares functions for generating image data.


namespace dip {


/// \defgroup generation Image generation
/// \brief Functions for filling images with generated data, and creating test images
/// \{

inline void FillDelta( Image& out ) {
   DIP_THROW_IF( !out.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !out.IsScalar(), E::IMAGE_NOT_FORGED );
   out.Fill( 0 );
   UnsignedArray pos = out.Sizes();
   for( auto& p : pos ) {
      p /= 2;
   }
   out.At( pos ) = 1;
}

inline void CreateDelta( Image const& in, Image& out ) {
   out.ReForge( in.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   FillDelta( out );
}
inline Image CreateDelta( Image const& in ) {
   Image out;
   CreateDelta( in, out );
   return out;
}


// `mode` can contain: "frequency", "left", "right", "true" or "corner" (default); "math" (inverts y axis); "radial" (in combination with "frequency")
DIP_EXPORT void FillRamp( Image& out, dip::uint dimension, StringSet const& mode = {} );

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

inline void FillXCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 0, mode );
}
inline void CreateXCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
   FillXCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateXCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateXCoordinate( in, out, mode );
   return out;
}

inline void FillYCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 1, mode );
}
inline void CreateYCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
   FillYCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateYCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateYCoordinate( in, out, mode );
   return out;
}

inline void FillZCoordinate( Image& out, StringSet const& mode = {} ) {
   FillRamp( out, 2, mode );
}
inline void CreateZCoordinate( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in, Option::AcceptDataTypeChange::DO_ALLOW );
   FillZCoordinate( out, mode );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateZCoordinate( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateZCoordinate( in, out, mode );
   return out;
}


DIP_EXPORT void FillRadiusCoordinate( Image& out, StringSet const& mode = {} );

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


DIP_EXPORT void FillPhiCoordinate( Image& out, StringSet const& mode = {} );

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


DIP_EXPORT void FillThetaCoordinate( Image& out, StringSet const& mode = {} );

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


DIP_EXPORT void FillCoordinates( Image& out, StringSet const& mode = {} );

inline void CreateCoordinates( Image const& in, Image& out, StringSet const& mode = {} ) {
   out.ReForge( in.Sizes(), in.Dimensionality(), DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   FillCoordinates( out, mode );
   out.SetPixelSize( in.PixelSize() );
}
inline Image CreateCoordinates( Image const& in, StringSet const& mode = {} ) {
   Image out;
   CreateCoordinates( in, out, mode );
   return out;
}

// FillCoordinates fills a vector image such that each pixel has as value its coordinates.
// FillXCoordinate, YCoordinate, ZCoordinate do the same for a scalar image, with the given index of the coordinates.
// FillRamp is a generalization, where a parameter selects the index of the coordinates.
// FillRadiusCoordinate again fills a scalar image, with the distance to the origin.
// FillPhiCoordinate again fills a scalar image, with the angle to the x-axis in the x-y plane.
// FillThetaCoordinate again fills a scalar image, with the angle to the z-axis.


/// \}


} // namespace dip


#endif // DIP_GENERATION_H
