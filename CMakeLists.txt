# CMakeLists file for DIPlib 3
cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0068)
   cmake_policy(SET CMP0068 NEW)
endif()

project(DIPlib)

# The version number and latest copyright year. Update these values here, they're used all throughout the project.
set(PROJECT_VERSION_MAJOR "3")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "alpha")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
message(STATUS "PROJECT_VERSION = ${PROJECT_VERSION}")
add_definitions(-DDIP_VERSION_STRING="${PROJECT_VERSION}")
add_definitions(-DDIP_COPYRIGHT_YEAR="2018")

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

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # Hiding all functions, to avoid settings mismatch with the MEX files
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   if(${MSVC_VERSION} VERSION_LESS 1900)
      message(SEND_ERROR "You need MSVC 2015 or later to compile DIPlib.")
   endif()
else()
   if(NOT "cxx_generic_lambdas" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
      message(SEND_ERROR "Your compiler does not support generic lambdas. You need a compiler with full C++14 support to build DIPlib.")
   endif()
   if(NOT "cxx_relaxed_constexpr" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
      message(SEND_ERROR "Your compiler does not support relaxed constexpr functions. You need a compiler with full C++14 support to build DIPlib.")
   endif()
endif()
# TODO: We should move to target-specific compiler mode:
#       target_compile_features(DIP PUBLIC cxx_generic_lambdas cxx_relaxed_constexpr)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   # using Clang C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion -pedantic")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage.
   set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   # using GNU C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -pedantic")
   # "#pragma omp" causes a warning if OpenMP is not enabled.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
   # "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   # "DIP_EXPORT" in forward class declaration sometimes causes a warning in GCC 6.0 and 7.0.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage; to see which flags are enabled: gcc -march=native -Q --help=target
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og") # Does some optimization that doesn't impact debugging.
   set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
   # using Intel C++
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   # using Visual Studio C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS) # Disable unchecked iterator warnings and unsafe (string manipulation) function warnings
   add_definitions(/wd4180) # Disable "qualifier applied to function type has no meaning; ignored" that happens in union_find.h as used in maxima.cpp
endif()

# RPATH
#set(CMAKE_MACOSX_RPATH 1) # This is the default
#set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# Extend CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/tools")

# OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND) # OPENMP_FOUND for CMake <= 3.8
   set(DIP_ENABLE_MULTITHREADING ON CACHE BOOL "Enable multithreading support")
endif()

include(tools/update_deps_file.cmake)


### DIPlib library

add_subdirectory(src)


### DIPviewer

# OpenGL, FreeGLUT, GLFW
find_package(OpenGL)
if(NOT APPLE)
   find_package(FreeGLUT)
endif()
if(FREEGLUT_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "FreeGLUT")
endif()
find_package(GLFW)
if(GLFW_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "GLFW")
endif()

if(OPENGL_FOUND AND (FREEGLUT_FOUND OR GLFW_FOUND))
   set(DIP_BUILD_DIPVIEWER ON CACHE BOOL "Build the viewer module")
endif()

if(DIP_BUILD_DIPVIEWER)

   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)

   if(WIN32)
      # On Windows, GLEW is required for OpenGL extensions
      find_package(GLEW REQUIRED)
   endif()

   add_subdirectory(viewer)

endif()


### PyDIP module for Python

# Some code modified from the Pybind11 cmake file, to determine if Python is installed.
# Pybind11 cmake file calls `find_package` for Python with `REQUIRED`, which causes the
# build to fail if Python is not installed. Here we disable the `DIP_BUILD_PYDIP` setting
# if there is no Python libraries.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/pybind11/tools")
set(PYBIND11_PYTHON_VERSION ${PYBIND11_PYTHON_VERSION} CACHE STRING "Python version to use for compiling modules")
set(Python_ADDITIONAL_VERSIONS 3.8 3.7 3.6 3.5 3.4 3.3) # TODO: This line will need to be updated when new versions of Python are released
   # CMake 3.5 knows about Python 3.6 and older, but by adding these version numbers here we make sure Python3 is found over Python2.
find_package(PythonLibsNew ${PYBIND11_PYTHON_VERSION})

if(PYTHONLIBS_FOUND)
   set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
   set(PYDIP_INSTALL_PATH ${PYTHON_SITE_PACKAGES} CACHE PATH "PyDIP install path")
endif()

if(DIP_BUILD_PYDIP)
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


### Examples

add_subdirectory(examples EXCLUDE_FROM_ALL)


### Packaging

# Write CMake configuration import scripts (but only when DIPlib is a shared library)
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

# Package stuff
if(CMAKE_HOST_WIN32)
   # On Windows we use the NSIS generator
   # TODO: This must be tested and cofigured properly
   set(CPACK_GENERATOR "NSIS")
#elseif(CMAKE_HOST_APPLE)
   # On MacOS we use the Bundle generator
   #set(CPACK_GENERATOR "Bundle")
   #CPACK_BUNDLE_NAME
   #CPACK_BUNDLE_ICON
   #CPACK_BUNDLE_PLIST
   #CPACK_BUNDLE_STARTUP_COMMAND # What would the startup command be???
else()
   # On Linux (or any other platform) we use a tar file
   set(CPACK_GENERATOR "TBZ2")
endif()
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_VENDOR "The DIPlib Project")
set(CPACK_PACKAGE_CONTACT "https://github.com/DIPlib/DIPlib")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
string(TIMESTAMP TODAYS_DATE "%Y%m%d" UTC)
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}-${TODAYS_DATE}")
#CPACK_PACKAGE_ICON
include(CPack)
# TODO: The package doesn't contain any of the dependencies: libtiff, libz, OpenMP, C++ stdlib, etc.
#       Including these is highly dependent on the configuration of the system used for compiling.
#       Posible solutions:
#        - `install(PROGRAMS ${TIFF_LIBRARIES} DESTINATION lib)`
#          We'd then need to modify the library names and the search locations using `install_name_tool`,
#          see http://doc.qt.io/qt-4.8/deployment-mac.html#linking-the-application-to-qt-as-frameworks
#        - `set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})`
#          to link against static libraries where possible, but this requires we also link against
#          libjpeg libz, and who knows what else. And it doesn't fix the problems with OpenMP, stdlib, etc.
#        - Include libtiff, libjpeg and libz in the repository, just like libics.
#          (And if we include libjpeg any way, we might as well port the JPEG reader over from the old dipIO.)
#        See also: https://cmake.org/cmake/help/v3.5/manual/cmake-packages.7.html#creating-relocatable-packages


### Status report

message("")
message("--------------------------------------")
message("   DIPlib CONFIGURATION REPORT")
message("")

message(" * C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
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
message(" * Library installed to: ${CMAKE_INSTALL_PREFIX}/lib")
message(" * Header files installed to: ${CMAKE_INSTALL_PREFIX}/include")

if(DOXYGEN_FOUND)
   message(" * Documentation installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
else()
   message(" * Doxygen documentation not configured")
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
   message(" * ICS file IO enabled")
else()
   message(" * ICS file IO disabled")
endif()
if(DIP_ENABLE_TIFF)
   message(" * TIFF file IO enabled")
else()
   message(" * TIFF file IO disabled")
endif()

if(DIP_ENABLE_FFTW)
   message(" * Using FFT from FFTW3 library (GPL)")
else()
   message(" * Using built-in FFT")
endif()

if(DIP_ENABLE_DOCTEST)
   message(" * Unit test code included")
else()
   message(" * Unit test code excluded")
endif()

if(DIP_BUILD_DIPVIEWER)
   message(" * DIPviewer module added to 'all' with window managers: " ${DIP_VIEWER_MANAGERS})
else()
   message(" * DIPviewer module not configured")
endif()

if(DIP_BUILD_DIPVIEWER_JAVA)
   message(" * DIPviewer Java bindings added to 'all' target")
else()
   message(" * DIPviewer Java bindings not configured")
endif()

if(DIP_BUILD_PYDIP)
   message(" * PyDIP module added to 'all' target")
   message(" * PyDIP compiled for Python version ${PYTHON_VERSION_STRING}")
   message(" * PyDIP installed to: ${PYDIP_INSTALL_PATH}/PyDIP")
else()
   message(" * PyDIP module not configured")
endif()

if(DIP_BUILD_DIPIMAGE)
   message(" * DIPimage toolbox added to 'all' target")
   matlab_get_release_name_from_version(${Matlab_VERSION_STRING} Matlab_RELEASE_NAME)
   message(" * DIPimage toolbox compiled for MATLAB ${Matlab_RELEASE_NAME}")
   message(" * DIPimage toolbox installed to: ${CMAKE_INSTALL_PREFIX}/${DIPIMAGE_INSTALL_PATH}")
   if(TARGET dum)
      message(" * DIPimage User Manual installed to: ${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}")
   else()
      message(" * DIPimage User Manual cannot be created")
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
   if(DOXYGEN_FOUND)
      message("Type 'make apidoc' to build the documentation")
   endif()
endif()

message("")
message("--------------------------------------")
message("")
