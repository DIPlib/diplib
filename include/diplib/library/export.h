/*
 * DIPlib 3.0
 * This file defines the DIP_EXPORT macros and some other ones.
 *
 * (c)2018-2021, Cris Luengo.
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
/// \brief Declares the \ref DIP_EXPORT and \ref DIP_NO_EXPORT macros, and some other similar ones.
/// This file is always included through \ref "diplib.h".

/// \addtogroup infrastructure

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
/// (such as a \ref dip::String, a.k.a. `std::string`), causes compiler warnings. It seems that currently
/// it is considered best practice to not export such classes. The Windows run-time linker always
/// maps local classes to those in the shared library, even when they are not exported.

#ifdef DIP_CONFIG_DIP_IS_STATIC
#   define DIP_EXPORT
#   define DIP_NO_EXPORT
#   define DIP_CLASS_EXPORT
#else
#   ifdef _WIN32
#      ifdef DIP_CONFIG_DIP_BUILD_SHARED
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


// Define macros to optionally use C++17 or later features
#ifndef __has_cpp_attribute
#   define __has_cpp_attribute(x) 0
#endif

/// \macro DIP_NODISCARD
/// \brief If your compiler supports it, adds `[[nodiscard]]` to a function definition.
///
/// `[[nodiscard]]` will produce a compile-time warning if the function return value is discarded.
/// This should pick up some programming errors, especially in cases where a different overload is
/// picked by the compiler than expected by the programmer. For example:
/// ```cpp
/// dip::Mean( in, {}, out ); // OK, picks main function
/// out = dip::Mean( in );    // OK, picks alternate overload
/// dip::Mean( in, out );     // warns, picks alternate overload and output is discarded
/// ```
/// The third line will warn because the compiler picks the `dip::Mean( in, mask )` overload that returns the
/// result image, rather than the intended main function `dip::Mean( in, mask, out )` that puts the result in
/// the existing `out` image.
#if __has_cpp_attribute(nodiscard)
#   define DIP_NODISCARD [[nodiscard]]
#else
#   define DIP_NODISCARD
#endif

/// \endgroup

#endif // DIP_EXPORT_H
