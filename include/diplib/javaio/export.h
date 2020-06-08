/*
 * DIPlib 3.0
 * This file defines the DIPJAVAIO_EXPORT and DIPJAVAIO_NO_EXPORT macros.
 *
 * (c)2019, Cris Luengo.
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

#ifndef DIP_JAVAIO_EXPORT_H
#define DIP_JAVAIO_EXPORT_H

/// \file
/// \brief Declares the `DIPJAVAIO_EXPORT` and `DIPJAVAIO_NO_EXPORT` macros.

/// \def DIPJAVAIO_EXPORT
/// \brief Indicates that the function or class is exported from the shared library.

/// \def DIPJAVAIO_NO_EXPORT
/// \brief Indicates that the function or class is not exported from the shared library.

/// \def DIPJAVAIO_CLASS_EXPORT
/// \brief Specifically for classes in a inheritance hierarchy and that must be passed across the
/// executable/shared library interface. See `DIP_CLASS_EXPORT` for more details.

#ifdef DIP_CONFIG_DIPJAVAIO_IS_STATIC
#   define DIPJAVAIO_EXPORT
#   define DIPJAVAIO_NO_EXPORT
#   define DIPJAVAIO_CLASS_EXPORT
#else
#   ifdef _WIN32 // TODO: do we need to test for __CYGWIN__ here also?
#      ifdef DIP_CONFIG_DIPJAVAIO_BUILD_SHARED
#         define DIPJAVAIO_EXPORT __declspec(dllexport)
#      else // We are using the library
#         define DIPJAVAIO_EXPORT __declspec(dllimport)
#      endif
#      define DIPJAVAIO_NO_EXPORT
#      define DIPJAVAIO_CLASS_EXPORT DIP_NO_EXPORT
#   else
#      define DIPJAVAIO_EXPORT __attribute__((visibility("default")))
#      define DIPJAVAIO_NO_EXPORT __attribute__((visibility("hidden")))
#      define DIPJAVAIO_CLASS_EXPORT DIPJAVAIO_EXPORT
#   endif
#endif

#endif // DIP_JAVAIO_EXPORT_H
