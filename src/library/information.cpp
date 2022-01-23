/*
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

#include "diplib.h"

namespace dip {

DIP_EXPORT extern LibraryInformation const libraryInformation{
      "DIPlib",
      "a quantitative image analysis library",
      "(c)2014-" DIP_COPYRIGHT_YEAR ", Cris Luengo and contributors\n(c)1995-2014, Delft University of Technology",
      "https://diplib.org",
      DIP_VERSION_STRING,
      __DATE__,
#if DIP_DEBUG_VERSION
      "Debug"
#else
      "Release"
#endif
#ifdef _OPENMP
            ", with OpenMP"
#endif
#ifdef DIP_CONFIG_ENABLE_STACK_TRACE
            ", recording stack traces"
#endif
#ifdef DIP_CONFIG_ENABLE_ASSERT
            ", asserts enabled"
#endif
#ifdef DIP_CONFIG_ENABLE_UNICODE
            ", Unicode support"
#endif
#ifdef DIP_CONFIG_HAS_ICS
            ", ICS support"
#endif
#ifdef DIP_CONFIG_HAS_TIFF
            ", TIFF support"
#endif
#ifdef DIP_CONFIG_HAS_JPEG
            ", JPEG support"
#endif
};

} // namespace dip
