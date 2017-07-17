/*
 * DIPlib 3.0
 * This file contains declarations for assorted analysis functions
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

#ifndef DIP_ANALYSIS_H
#define DIP_ANALYSIS_H

#include "diplib.h"


/// \file
/// \brief Declares functions for assorted analysis functions.


namespace dip {


/// \defgroup analysis Analysis
/// \brief Assorted analysis tools.
/// \{

/// \brief Contains the result of the function `dip::SubpixelLocation`.
struct SubpixelLocationResult {
   FloatArray coordinates;
   dfloat value;
};

/// \brief Contains the result of the functions `dip::SubpixelMaxima` and `dip::SubpixelMinima`.
using SubpixelLocationArray = std::vector< SubpixelLocationResult >;

/// \brief Gets coordinates of a local extremum with sub-pixel precision
///
/// Determines the sub-pixel location of a local maximum or minimum close to `position`. `position` should point
/// to a pixel whose value is larger than its direct neighbours' (if `polarity` is `"maximum"`) or smaller than
/// its direct neighbours' (`polarity` is `"minimum"`).
///
/// The `coordinates` member of the output struct will contain the the sub-pixel location of this local
/// extremum. The `value` member will contain the interpolated grey value at the location of the extremum.
///
/// `method` determines which method is used. These are the allowed values:
///  - `"linear"`: Computes the center of gravity of 3 pixels around the extremum, in each dimension independently.
///    The value at the extremum is that of the pixel at `position`.
///  - `"parabolic"`: Fits a parabola to a 3-pixel-wide block around the extremum (for up to 3D only).
///  - `"parabolic separable"`: Fits a parabola in each dimension independently. The value at the extremum
///    is the maximum/minimum of the values obtained in each of the 1D processes, and thus not equivalent
///    to the grey value obtained by true interpolation.
///  - `"gaussian"`: Fits a Gaussian to a 3-pixel-wide block around the extremum (for up to 3D only).
///  - `"gaussian separable"`: Fits a Gaussian in each dimension independently. The value at the extremum
///    is the maximum/minimum of the values obtained in each of the 1D processes, and thus not equivalent
///    to the grey value obtained by true interpolation.
///
/// The image `in` must be scalar and real-valued.
DIP_EXPORT SubpixelLocationResult SubpixelLocation(
      Image const& in,
      UnsignedArray const& position,
      String const& polarity = "maximum",
      String const& method = "parabolic separable"
);

/// \brief Gets coordinates of local maxima with sub-pixel precision
///
/// Detects local maxima in the image, and returns their coordinates, with sub-pixel precision.
/// Only pixels where `mask` is on will be examined. Local maxima are detected using `dip::Maxima`,
/// then their position is determined more precisely using `dip::SubpixelLocation`.
///
/// A local maximum can not touch the edge of the image. That is, its integer location must be one
/// pixel away from the edge.
///
/// See `dip::SubpixelLocation` for the definition of the `method` parameter.
DIP_EXPORT SubpixelLocationArray SubpixelMaxima(
      Image const& in,
      Image const& mask = {},
      String const& method = "parabolic separable"
);

/// \brief Gets coordinates of local minima with sub-pixel precision
///
/// Detects local minima in the image, and returns their coordinates, with sub-pixel precision.
/// Only pixels where `mask` is on will be examined. Local minima are detected using `dip::Minima`,
/// then their position is determined more precisely using `dip::SubpixelLocation`.
///
/// A local minimum can not touch the edge of the image. That is, its integer location must be one
/// pixel away from the edge.
///
/// See `dip::SubpixelLocation` for the definition of the `method` parameter.
DIP_EXPORT SubpixelLocationArray SubpixelMinima(
      Image const& in,
      Image const& mask = {},
      String const& method = "parabolic separable"
);


// TODO: functions to port:
/*
   dip_CrossCorrelationFT (dip_findshift.h)
   dip_FindShift (dip_findshift.h)

   dip_PairCorrelation (dip_analysis.h)
   dip_ProbabilisticPairCorrelation (dip_analysis.h)
   dip_ChordLength (dip_analysis.h)
   dip_RadialDistribution (dip_analysis.h)

   dip_StructureAnalysis (dip_analysis.h)

   dip_OrientationSpace (dip_structure.h)
   dip_ExtendedOrientationSpace (dip_structure.h)

   dip_StructureTensor2D (dip_structure.h) (trivial using existing functionality, generalize to nD)
   dip_StructureTensor3D (dip_structure.h) (see dip_StructureTensor2D)
   dip_CurvatureFromTilt (dip_structure.h)

   dip_OSEmphasizeLinearStructures (dip_structure.h)
   dip_DanielsonLineDetector (dip_structure.h)
   dip_Canny (dip_detection.h) (or in diplib/segmentation.h?)
*/

/// \}

} // namespace dip

#endif // DIP_ANALYSIS_H
