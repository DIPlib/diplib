/*
 * (c)2017-2025, Cris Luengo.
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

LibraryInformation const libraryInformation{
      // name
      "DIPlib",
      // description
      "a quantitative image analysis library",
      // copyright
      "(c)2014-" DIP_COPYRIGHT_YEAR ", Cris Luengo and contributors\n(c)1995-2014, Delft University of Technology",
      // URL
      "https://diplib.org",
      // version
      DIP_VERSION_STRING,
      // date
      __DATE__,
      // type
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
#ifdef DIP_CONFIG_HAS_PNG
            ", PNG support"
#endif
#ifdef DIP_CONFIG_HAS_FFTW
            ", using FFTW"
#endif
#ifdef DIP_CONFIG_HAS_FREETYPE
            ", FreeType support"
#endif
   ,
      // isReleaseBuild
#if DIP_DEBUG_VERSION
      false,
#else
      true,
#endif
      // usingOpenMP
#ifdef _OPENMP
      true,
#else
      false,
#endif
      // stackTracesEnabled
#ifdef DIP_CONFIG_ENABLE_STACK_TRACE
      true,
#else
      false,
#endif
      // assertsEnabled
#ifdef DIP_CONFIG_ENABLE_ASSERT
      true,
#else
      false,
#endif
      // usingUnicode
#ifdef DIP_CONFIG_ENABLE_UNICODE
      true,
#else
      false,
#endif
      // hasICS
#ifdef DIP_CONFIG_HAS_ICS
      true,
#else
      false,
#endif
      // hasTIFF
#ifdef DIP_CONFIG_HAS_TIFF
      true,
#else
      false,
#endif
      // hasJPEG
#ifdef DIP_CONFIG_HAS_JPEG
      true,
#else
      false,
#endif
      // hasPNG
#ifdef DIP_CONFIG_HAS_PNG
      true,
#else
      false,
#endif
      // usingFFTW
#ifdef DIP_CONFIG_HAS_FFTW
      true,
#else
      false,
#endif
      // usingFreeType
#ifdef DIP_CONFIG_HAS_FREETYPE
      true
#else
      false
#endif
};

} // namespace dip
