/*
 * DIPlib 3.0
 * This file contains declarations for microscopy-related functionality
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

#ifndef DIP_MICROSCOPY_H
#define DIP_MICROSCOPY_H

#include "diplib.h"


/// \file
/// \brief Microscopy-related functionality.
/// \see microscopy


namespace dip {


/// \defgroup microscopy Microscopy
/// \brief Assorted tools useful in microscopy, some presumably also in astronomy and other applications.
/// \{

// TODO: functions to port:
/*
   dip_IncoherentPSF (dip_microscopy.h)
   dip_IncoherentOTF (dip_microscopy.h)
   dip_ExponentialFitCorrection (dip_microscopy.h)
   dip_AttenuationCorrection (dip_microscopy.h)
   dip_SimulatedAttenuation (dip_microscopy.h)
   dip_RestorationTransform (dip_restoration.h)
   dip_TikhonovRegularizationParameter (dip_restoration.h)
   dip_TikhonovMiller (dip_restoration.h)
   dip_Wiener (dip_restoration.h)
   dip_PseudoInverse (dip_restoration.h)
*/

// TODO: add brightfield stain unmixing

/// \}

} // namespace dip

#endif // DIP_MICROSCOPY_H
