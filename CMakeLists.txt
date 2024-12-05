# (c)2015-2024, Cris Luengo, Wouter Caarls
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# CMakeLists file for DIPlib 3
cmake_minimum_required(VERSION 3.12...3.28)

project(DIPlib)

# The version number and latest copyright year. Update these values here, they're used all throughout the project.
set(PROJECT_VERSION_MAJOR "3")
set(PROJECT_VERSION_MINOR "5")
set(PROJECT_VERSION_PATCH "1")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(DIP_COPYRIGHT_YEAR "2024")

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

# Static or shared?
set(DIP_SHARED_LIBRARY ON CACHE BOOL "Build a shared library (off for static library)")
set(BUILD_SHARED_LIBS ${DIP_SHARED_LIBRARY})

# Installation path
set(CMAKE_INSTALL_PREFIX "${CMAKE_BUILD_TYPE}" CACHE PATH "Installation directory")
set(DOCUMENTATION_OUTPUT share/doc/DIPlib)
set(DIPIMAGE_INSTALL_PATH share/DIPimage)
set(RUNTIME_DESTINATION bin)
if(WIN32)
   set(LIBRARY_DESTINATION bin)
else()
   set(LIBRARY_DESTINATION lib)
endif()
set(ARCHIVE_DESTINATION lib)
set(DIP_DESTINATIONS
    RUNTIME DESTINATION ${RUNTIME_DESTINATION}
    LIBRARY DESTINATION ${LIBRARY_DESTINATION}
    ARCHIVE DESTINATION ${ARCHIVE_DESTINATION})

# RPATH
#set(CMAKE_MACOSX_RPATH 1) # This is the default
#set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# Extend CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/tools")

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND) # OPENMP_FOUND for CMake <= 3.8
   set(DIP_ENABLE_MULTITHREADING ON CACHE BOOL "Enable multithreading support")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/tools/compiler_flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/tools/update_deps_file.cmake)


### DIPlib library

add_subdirectory(src)


### DIPjavaio

find_package(Java COMPONENTS Development)
unset(DIP_JAVAIO_INTERFACES)
if(JAVA_FOUND)
   # Set JAVA_HOME so that the JNI found is from the same JDK as the Java package
   get_filename_component(JAVA_HOME "${Java_JAVAC_EXECUTABLE}" DIRECTORY) # strip off /javac
   get_filename_component(JAVA_HOME "${JAVA_HOME}" DIRECTORY)             # strip off /bin
   find_package(JNI)
   if(JNI_FOUND)
      set(DIP_BUILD_JAVAIO ON CACHE BOOL "Build the javaio module")
      include(UseJava)
      list(APPEND DIP_JAVAIO_INTERFACES "Bio-Formats")
   endif()
   set(DIP_JAVA_VERSION "1.8" CACHE STRING "Java version to target when building DIPviewer Java bindings and DIPjavaio.")
   # MATLAB R2013b--R2017a use Java SDK 1.7, R2017b-- use SDK 1.8.
   set(CMAKE_JAVA_COMPILE_FLAGS -target ${DIP_JAVA_VERSION} -source ${DIP_JAVA_VERSION})
endif()

if(DIP_BUILD_JAVAIO)
   add_subdirectory(javaio)
endif()


### DIPviewer

# OpenGL, FreeGLUT, GLFW
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL)
if(APPLE)
    set(DIP_FIND_FREEGLUT OFF CACHE BOOL "Try to find FreeGLUT library")
else()
    set(DIP_FIND_FREEGLUT ON CACHE BOOL "Try to find FreeGLUT library")
endif()
set(DIP_FIND_GLFW ON CACHE BOOL "Try to find GLFW library")

if (DIP_FIND_FREEGLUT)
   find_package(FreeGLUT)
endif()
if(FREEGLUT_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "FreeGLUT")
   set(FREEGLUT_STATIC OFF CACHE BOOL "Link to static FreeGLUT library")
endif()
if (DIP_FIND_GLFW)
    find_package(GLFW)
endif()
if(GLFW_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "GLFW")
endif()

if(OPENGL_FOUND AND (FREEGLUT_FOUND OR GLFW_FOUND))
   set(DIP_BUILD_DIPVIEWER ON CACHE BOOL "Build the viewer module")
else()
   set(DIP_BUILD_DIPVIEWER OFF CACHE BOOL "Build the viewer module")
endif()

if(DIP_BUILD_DIPVIEWER)
   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)
   add_subdirectory(viewer)
endif()


### PyDIP module for Python
if(DEFINED PYTHON_EXECUTABLE AND NOT DEFINED Python_EXECUTABLE)
   message(WARNING "PYTHON_EXECUTABLE is deprecated, use Python_EXECUTABLE instead.")
   set(Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
endif()
find_package(Python 3.6 COMPONENTS Interpreter Development.Module)
if(Python_Development.Module_FOUND)
   set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
endif()

if(DIP_BUILD_PYDIP)
   set(DIPlib_VERSION ${PROJECT_VERSION})
   add_subdirectory(pydip)
endif()


### DIPimage toolbox for MATLAB

if(DIP_SHARED_LIBRARY)
   # We will not build DIPimage without a shared DIPlib -- technically possible, but not desireable.
   # (This would also require solving linker warnings because of Doctest.)

   #set(MATLAB_ADDITIONAL_VERSIONS "R2018b=9.5") # You might need to add a line like this if your version of MATLAB is not recognized
   find_package(Matlab COMPONENTS MAIN_PROGRAM) # NOTE! Here we use the local copy of the FindMatlab.cmake script
   if(Matlab_FOUND)
      set(DIP_BUILD_DIPIMAGE ON CACHE BOOL "Build the DIPimage toolbox")
   endif()

endif()

if(DIP_BUILD_DIPIMAGE)
   add_subdirectory(dipimage)
endif()


### Documentation

add_subdirectory(doc)


### Examples

add_subdirectory(examples EXCLUDE_FROM_ALL)


### CMake import scripts

if(DIP_SHARED_LIBRARY)

   include(CMakePackageConfigHelpers)
   set(ConfigPackageLocation lib/cmake/DIPlib)

   export(EXPORT DIPlibTargets
          FILE "${CMAKE_CURRENT_BINARY_DIR}/DIPlibTargets.cmake"
          NAMESPACE DIPlib::)
   install(EXPORT DIPlibTargets
           FILE DIPlibTargets.cmake
           NAMESPACE DIPlib::
           DESTINATION ${ConfigPackageLocation})

   configure_package_config_file(
         tools/DIPlibConfig.cmake.in
         "${CMAKE_CURRENT_BINARY_DIR}/DIPlibConfig.cmake"
         INSTALL_DESTINATION ${ConfigPackageLocation})
   write_basic_package_version_file(
         "${CMAKE_CURRENT_BINARY_DIR}/DIPlibConfigVersion.cmake"
         VERSION ${DIPlib_VERSION}
         COMPATIBILITY SameMajorVersion)
   install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DIPlibConfigVersion.cmake"
                 "${CMAKE_CURRENT_BINARY_DIR}/DIPlibConfig.cmake"
           DESTINATION ${ConfigPackageLocation})

endif()


### Status report

message("")
message("--------------------------------------")
message("   DIPlib CONFIGURATION REPORT")
message("")

message(" * PROJECT_VERSION: ${PROJECT_VERSION}")
message(" * DIP_COPYRIGHT_YEAR: ${DIP_COPYRIGHT_YEAR}")

message(" * C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
   string(TOUPPER ${CMAKE_BUILD_TYPE} UPPERCASE_BUILD_TYPE)
   message(" * C++ compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${UPPERCASE_BUILD_TYPE}}")
   message(" * Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(DIP_SHARED_LIBRARY)
   message(" * Building shared library")
else()
   message(" * Building static library")
endif()
message(" * Library installed to: ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DESTINATION}")
message(" * Binaries installed to: ${CMAKE_INSTALL_PREFIX}/${RUNTIME_DESTINATION}")
if (WIN32)
   message(" * Import library installed to: ${CMAKE_INSTALL_PREFIX}/${ARCHIVE_DESTINATION}")
endif()
message(" * Header files installed to: ${CMAKE_INSTALL_PREFIX}/include")

if(DOXPP_FOUND)
   message(" * Documentation installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
else()
   message(" * dox++ program not configured, no documentation can be generated")
endif()


if(DIP_ENABLE_MULTITHREADING)
   message(" * Using OpenMP for multithreading")
else()
   message(" * Multithreading disabled")
endif()

if(DIP_ENABLE_STACK_TRACE)
   message(" * Stack trace recording enabled")
else()
   message(" * Stack trace recording disabled")
endif()
if(DIP_ENABLE_ASSERT)
   message(" * Asserts enabled")
else()
   message(" * Asserts disabled")
endif()
if(DIP_ENABLE_UNICODE)
   message(" * Unicode support enabled")
else()
   message(" * Unicode support disabled")
endif()
if(HAS_128_INT)
   message(" * Using 128-bit PRNG")
else()
   message(" * Using 64-bit PRNG")
endif()

if(DIP_ENABLE_ICS)
   if(DIP_ENABLE_ZLIB)
      set(features ", with zlib")
   else()
      set(features ", without zlib")
   endif()
   message(" * ICS file IO enabled${features}")
else()
   message(" * ICS file IO disabled")
endif()
if(DIP_ENABLE_TIFF)
   set(features "")
   if(DIP_ENABLE_ZLIB)
      set(features "${features}, with zlib")
   else()
      set(features "${features}, without zlib")
   endif()
   if(DIP_ENABLE_JPEG)
      set(features "${features}, with JPEG")
   else()
      set(features "${features}, without JPEG")
   endif()
   message(" * TIFF file IO enabled${features}")
else()
   message(" * TIFF file IO disabled")
endif()
if(DIP_ENABLE_JPEG)
   message(" * JPEG file IO enabled")
else()
   message(" * JPEG file IO disabled")
endif()
if(DIP_ENABLE_PNG)
   message(" * PNG file IO enabled")
else()
   message(" * PNG file IO disabled")
endif()

if(DIP_ENABLE_FFTW)
   message(" * Using FFTW3 (GPL)")
else()
   message(" * Using PocketFFT")
endif()

if(DIP_ENABLE_FREETYPE)
   message(" * Using FreeType")
else()
   message(" * Not using FreeType")
endif()

if(DIP_ENABLE_DOCTEST)
   message(" * Unit test code included")
else()
   message(" * Unit test code excluded")
endif()

if(DIP_BUILD_DIPVIEWER)
   message(" * DIPviewer module added to 'all' with window managers: ${DIP_VIEWER_MANAGERS}")
   if (FREEGLUT_STATIC)
      message(" * Using static FreeGLUT library")
   endif()
else()
   message(" * DIPviewer module not configured")
endif()

if(DIP_BUILD_JAVAIO)
   message(" * DIPjavaio module added to 'all' target with interfaces: ${DIP_JAVAIO_INTERFACES}")
   message(" * DIPjavaio compiled using Java version ${Java_VERSION_STRING} and targeting ${DIP_JAVA_VERSION}")
else()
   message(" * DIPJavaio module not configured")
endif()

if(DIP_BUILD_PYDIP)
   message(" * PyDIP module added to 'all' target")
   message(" * PyDIP compiled for Python version ${Python_VERSION}")
   if(DIP_PYDIP_WHEEL_INCLUDE_LIBS)
      message(" * PyDIP includes libraries in binary wheel")
   else()
      message(" * PyDIP does not include libraries in binary wheel")
   endif()
else()
   message(" * PyDIP module not configured")
endif()

if(DIP_BUILD_DIPIMAGE)
   message(" * DIPimage toolbox added to 'all' target")
   matlab_get_release_name_from_version(${Matlab_VERSION_STRING} Matlab_RELEASE_NAME)
   message(" * DIPimage toolbox compiled for MATLAB ${Matlab_RELEASE_NAME}")
   if(DIP_BUILD_DIPVIEWER_JAVA)
      message(" * DIPviewer Java bindings added to 'all' target, compiled using Java version ${Java_VERSION_STRING} and targeting ${DIP_JAVA_VERSION}")
   else()
      message(" * DIPviewer Java bindings not configured")
   endif()
   message(" * DIPimage toolbox installed to: ${CMAKE_INSTALL_PREFIX}/${DIPIMAGE_INSTALL_PATH}")
   if(DOXPP_FOUND)
      message(" * DIPimage User Manual installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}/html/dipimage_user_manual.html")
   else()
      message(" * DIPimage User Manual cannot be created (dox++ program not configured)")
   endif()
else()
   message(" * DIPimage toolbox not configured")
endif()

if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
   message("")
   if(DIP_ENABLE_DOCTEST)
      message("Next, type 'make; make check; make install'")
   else()
      message("Next, type 'make; make install'")
   endif()
   if(DOXPP_FOUND)
      message("Type 'make doc' to build the documentation")
   endif()
endif()

message("")
message("--------------------------------------")
message("")
