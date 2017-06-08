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

// TODO: functions to port:
/*
   dip_PairCorrelation (dip_analysis.h)
   dip_ProbabilisticPairCorrelation (dip_analysis.h)
   dip_ChordLength (dip_analysis.h)
   dip_RadialDistribution (dip_analysis.h)

   dip_StructureAnalysis (dip_analysis.h)

   dip_SubpixelMaxima (dip_analysis.h)
   dip_SubpixelMinima (dip_analysis.h)
   dip_SubpixelLocation (dip_analysis.h)
   dip_CrossCorrelationFT (dip_findshift.h)
   dip_FindShift (dip_findshift.h)

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
