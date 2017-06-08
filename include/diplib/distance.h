/*
 * DIPlib 3.0
 * This file contains declarations for distance transforms
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

#ifndef DIP_DISTANCE_H
#define DIP_DISTANCE_H

#include "diplib.h"


/// \file
/// \brief Declares functions that compute distance transforms.


namespace dip {


/// \defgroup distance Distance transforms
/// \brief Various distance transforms.
/// \{

// TODO: functions to port:
/*
   dip_EuclideanDistanceTransform (dip_distance.h)
   dip_VectorDistanceTransform (dip_distance.h)
   dip_GreyWeightedDistanceTransform (dip_distance.h)
   dip_FastMarching_PlaneWave (dip_distance.h) (this function needs some input image checking!)
   dip_FastMarching_SphericalWave (dip_distance.h) (this function needs some input image checking!)
*/

/// \}

} // namespace dip

#endif // DIP_DISTANCE_H
