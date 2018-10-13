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
/// \brief Declares the `DIP_EXPORT` and `DIP_NO_EXPORT` macros.

/// \def DIP_EXPORT
/// \brief Indicates that the function or class is exported from the shared/dynamic-load library.

/// \def DIP_NO_EXPORT
/// \brief Indicates that the function or class is not exported from the shared/dynamic-load library.

/// \def DIP_CLASS_EXPORT
/// \brief Specifically for classes in a inheritance hierarchy and that must be passed across the
/// executable/shared library interface.
///
/// Equal to `DIP_NO_EXPORT` on Windows and `DIP_EXPORT` elsewhere.
///
/// On Linux and other Unix-like systems, classes in a hierarchy, as well as classes thrown as
/// exceptions, must be exported from the shared library if they are to be used across the interface
/// between the library and whatever is using the library. This is the only way that the run-time
/// linker is able to merge the virtual function tables, and identify the type of the thrown object.
///
/// However, on Windows, trying to export a class derived from a class in the standard library (such
/// as `std::exception`), or trying to export a class that contains members of a non-exported class
/// (such as a `dip::String`, a.k.a. `std::string`), causes compiler warnings. It seems that currently
/// it is considered best practice to not export such classes. The Windows run-time linker always
/// maps local classes to those in the shared library, even when they are not exported.

#ifdef DIP__IS_STATIC
#   define DIP_EXPORT
#   define DIP_NO_EXPORT
#   define DIP_CLASS_EXPORT
#else
#   ifdef _WIN32 // TODO: do we need to test for __CYGWIN__ here also?
#      ifdef DIP__BUILD_SHARED
#         define DIP_EXPORT __declspec(dllexport)
#      else // We are using the library
#         define DIP_EXPORT __declspec(dllimport)
#      endif
#      define DIP_NO_EXPORT
       // On the Windows platform, exception classes derived from `std::exception` should not exported
#      define DIP_CLASS_EXPORT DIP_NO_EXPORT
#   else
#      define DIP_EXPORT __attribute__((visibility("default")))
#      define DIP_NO_EXPORT __attribute__((visibility("hidden")))
       // On other platforms, exception classes derived from `std::exception` must be exported
#      define DIP_CLASS_EXPORT DIP_EXPORT
#   endif
#endif

#endif // DIP_EXPORT_H
