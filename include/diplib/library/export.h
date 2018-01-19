/*
 * DIPlib 3.0
 * This file defines the DIP_EXPORT and DIP_NO_EXPORT macros.
 *
 * (c)2018, Cris Luengo.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_EXPORT_H
#define DIP_EXPORT_H

/// \file
/// \brief Declares the DIP_EXPORT and DIP_NO_EXPORT macros.

/// \def DIP_EXPORT
/// \brief Indicates that the function or class is exported from the shared/dynamic-load library.

/// \def DIP_NO_EXPORT
/// \brief Indicates that the function or class is not exported from the shared/dynamic-load library.

#ifdef DIP__IS_STATIC
#   define DIP_EXPORT
#   define DIP_NO_EXPORT
#else
#   ifdef _WIN32 // TODO: do we need to test for __CYGWIN__ here also?
#      ifdef DIP__BUILD_SHARED
#         define DIP_EXPORT __declspec(dllexport)
#      else // We are using the library
#         define DIP_EXPORT __declspec(dllimport)
#      endif
#      define DIP_NO_EXPORT
#   else
#      define DIP_EXPORT __attribute__((visibility("default")))
#      define DIP_NO_EXPORT __attribute__((visibility("hidden")))
#   endif
#endif

#endif // DIP_EXPORT_H
