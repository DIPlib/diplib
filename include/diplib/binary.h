/*
 * DIPlib 3.0
 * This file contains declarations for binary image processing
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

#ifndef DIP_BINARY_H
#define DIP_BINARY_H

#include "diplib.h"


/// \file
/// \brief Declares functions for binary image processing.


namespace dip {


/// \defgroup binary Binary image filters
/// \ingroup filtering
/// \brief Processing binary images, including binary mathematical morphology.
/// \{

// TODO: functions to port:
/*
   dip_BinaryDilation (dip_binary.h)
   dip_BinaryErosion (dip_binary.h)
   dip_BinaryClosing (dip_binary.h)
   dip_BinaryOpening (dip_binary.h)
   dip_BinaryPropagation (dip_binary.h)
   dip_EdgeObjectsRemove (dip_binary.h)
   dip_EuclideanSkeleton (dip_binary.h)
   dip_BinarySkeleton3D (dip_binary.h) (don't port this one, it's crap; instead fix the bug in EuclideanSkeleton for 3D)
*/

/// \}

} // namespace dip

#endif // DIP_BINARY_H
