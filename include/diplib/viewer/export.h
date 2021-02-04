/*
 * DIPlib 3.0 viewer
 * This file defines the DIPVIEWER_EXPORT and DIPVIEWER_NO_EXPORT macros.
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

#ifndef DIP_VIEWER_EXPORT_H
#define DIP_VIEWER_EXPORT_H

/// \addtogroup dipviewer

/// \file
/// \brief Declares the \ref DIPVIEWER_EXPORT and \ref DIPVIEWER_NO_EXPORT macros.

/// \def DIPVIEWER_EXPORT
/// \brief Indicates that the function or class is exported from the shared library.

/// \def DIPVIEWER_NO_EXPORT
/// \brief Indicates that the function or class is not exported from the shared library.

/// \def DIPVIEWER_CLASS_EXPORT
/// \brief Specifically for classes in a inheritance hierarchy and that must be passed across the
/// executable/shared library interface. See \ref DIP_CLASS_EXPORT for more details.

#ifdef DIP_CONFIG_DIPVIEWER_IS_STATIC
#   define DIPVIEWER_EXPORT
#   define DIPVIEWER_NO_EXPORT
#   define DIPVIEWER_CLASS_EXPORT
#else
#   ifdef _WIN32 // TODO: do we need to test for __CYGWIN__ here also?
#      ifdef DIP_CONFIG_DIPVIEWER_BUILD_SHARED
#         define DIPVIEWER_EXPORT __declspec(dllexport)
#      else // We are using the library
#         define DIPVIEWER_EXPORT __declspec(dllimport)
#      endif
#      define DIPVIEWER_NO_EXPORT
#      define DIPVIEWER_CLASS_EXPORT DIP_NO_EXPORT
#   else
#      define DIPVIEWER_EXPORT __attribute__((visibility("default")))
#      define DIPVIEWER_NO_EXPORT __attribute__((visibility("hidden")))
#      define DIPVIEWER_CLASS_EXPORT DIPVIEWER_EXPORT
#   endif
#endif

/// \endgroup

#endif // DIP_VIEWER_EXPORT_H
