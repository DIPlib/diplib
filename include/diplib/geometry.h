/*
 * DIPlib 3.0
 * This file contains declarations for geometric transformations
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

#ifndef DIP_GEOMETRY_H
#define DIP_GEOMETRY_H

#include "diplib.h"


/// \file
/// \brief Declares functions for geometric transformations


namespace dip {


/// \defgroup geometry Geometric transformations
/// \brief Geometric image transformations.
/// \{

// TODO: functions to port:
/*
   dip_Resampling (dip_interpolation.h)
   dip_ResampleAt (dip_interpolation.h)
   dip_Subsampling (dip_interpolation.h)
   dip_Skewing (dip_interpolation.h)
   dip_SkewingWithBgval (dip_interpolation.h) (merge into dip_Skewing)
   dip_Rotation (dip_interpolation.h)
   dip_RotationWithBgval (dip_interpolation.h) (merge into dip_Rotation)
   dip_Rotation3d_Axis (dip_interpolation.h)
   dip_Rotation3d (dip_interpolation.h)
   dip_Rotation2d90 (dip_interpolation.h) (as method to dip::Image)
   dip_Rotation3d90 (dip_interpolation.h) (as method to dip::Image, generalize to nD)
   dip_AffineTransform (dip_interpolation.h)
   dip_Shift (dip_manipulation.h)
   dip_Wrap (dip_manipulation.h)
   dip_ResamplingFT (dip_manipulation.h)
*/

// TODO: port also rot_euler_low.c and affine_trans_low.c from DIPimage

/// \}

} // namespace dip

#endif // DIP_GEOMETRY_H
