# CMakeLists file for DIPlib 3
cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0068)
   cmake_policy(SET CMP0068 NEW)
endif()

project(DIPlib)

# The version number and latest copyright year. Update these values here, they're used all throughout the project.
set(PROJECT_VERSION_MAJOR "3")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(DIP_COPYRIGHT_YEAR "2021")

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

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   # Compiler flags for Clang C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion -pedantic -Wno-c++17-extensions")
   #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # This is optimal for local usage.
   set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   # Compiler flags for GNU C++
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
   # Compiler flags for Intel C++
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   # Compiler flags for Visual Studio C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS) # Disable unchecked iterator warnings and unsafe (string manipulation) function warnings
   add_definitions(/wd4180) # Disable "qualifier applied to function type has no meaning; ignored" that happens in union_find.h as used in maxima.cpp
   add_definitions(/EHa) # This exception handling model apparently is what MATLAB uses (default is /EHsc)
   add_definitions(/MP) # Enable Multi-Processor compilation
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


### DIPjavaio

find_package(Java 1.7 COMPONENTS Development)
if(JAVA_FOUND)
   # Set JAVA_HOME so that the JNI found is from the same JDK as the Java package
   get_filename_component(JAVA_HOME "${Java_JAVAC_EXECUTABLE}" DIRECTORY) # strip off /javac
   get_filename_component(JAVA_HOME "${JAVA_HOME}" DIRECTORY)             # strip off /bin
   find_package(JNI)
   if(JNI_FOUND)
      set(DIP_BUILD_JAVAIO ON CACHE BOOL "Build the javaio module")
      include(UseJava)
      find_jar(BIOFORMATS_JAR bioformats_package)
      if(BIOFORMATS_JAR)
         list(APPEND DIP_JAVAIO_INTERFACES "Bio-Formats")
      endif()
   endif()
   set(CMAKE_JAVA_COMPILE_FLAGS -target 1.7 -source 1.7) # MATLAB R2013b--R2017a use Java SDK 1.7, R2017b-- use SDK 1.8.
   # TODO: Make target Java version configurable?
endif()

if(DIP_BUILD_JAVAIO)
   add_subdirectory(javaio)
endif()


### DIPviewer

# OpenGL, FreeGLUT, GLFW
find_package(OpenGL)
if(NOT APPLE)
   find_package(FreeGLUT)
endif()
if(FREEGLUT_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "FreeGLUT")
   set(FREEGLUT_STATIC OFF CACHE BOOL "Link to static FreeGLUT library")
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
   add_subdirectory(viewer)
endif()


### PyDIP module for Python

# Some code modified from the Pybind11 cmake file, to determine if Python is installed.
# Pybind11 cmake file calls `find_package` for Python with `REQUIRED`, which causes the
# build to fail if Python is not installed. Here we disable the `DIP_BUILD_PYDIP` setting
# if there is no Python libraries.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/pybind11/tools")
set(Python_ADDITIONAL_VERSIONS ${PYBIND11_PYTHON_VERSION} 3.10 3.9 3.8 3.7 3.6 3.5) # TODO: This line will need to be updated when new versions of Python are released
   # CMake 3.5 knows about Python 3.6 and older, but by adding these version numbers here we make sure Python3 is found over Python2.
find_package(PythonLibsNew)

if(PYTHONLIBS_FOUND)
   set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
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


### Documentation

add_subdirectory(doc)

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
   # TODO: This must be tested and configured properly
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
# TODO: The package doesn't contain any of the dependencies: OpenMP, C++ stdlib, etc.
#       Including these is highly dependent on the configuration of the system used for compiling.
#       Posible solutions:
#        - `install(PROGRAMS ${libname} DESTINATION lib)`
#          We'd then need to modify the library names and the search locations using `install_name_tool`,
#          see http://doc.qt.io/qt-4.8/deployment-mac.html#linking-the-application-to-qt-as-frameworks
#        See also: https://cmake.org/cmake/help/v3.5/manual/cmake-packages.7.html#creating-relocatable-packages


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
   message(" * DIPviewer module added to 'all' with window managers: ${DIP_VIEWER_MANAGERS}")
   if (FREEGLUT_STATIC)
      message(" * Using static FreeGLUT library")
   endif()
else()
   message(" * DIPviewer module not configured")
endif()

if(DIP_BUILD_DIPVIEWER_JAVA)
   message(" * DIPviewer Java bindings added to 'all' target")
else()
   message(" * DIPviewer Java bindings not configured")
endif()

if(DIP_BUILD_JAVAIO)
   message(" * DIPjavaio module added to 'all' target with interfaces: ${DIP_JAVAIO_INTERFACES}")
   message(" * DIPjavaio compiled using Java version ${Java_VERSION_STRING}")
else()
   message(" * DIPJavaio module not configured")
endif()

if(DIP_BUILD_PYDIP)
   message(" * PyDIP module added to 'all' target")
   message(" * PyDIP compiled for Python version ${PYTHON_VERSION_STRING}")
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
      message("Type 'make apidoc' to build the documentation")
   endif()
endif()

message("")
message("--------------------------------------")
message("")
