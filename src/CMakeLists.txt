# (c)2015-2022, Cris Luengo, Wouter Caarls
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


file(GLOB DIPLIB_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/*/*.h"
     "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp"
     "${PROJECT_SOURCE_DIR}/include/diplib.h"
     "${PROJECT_SOURCE_DIR}/include/diplib/*.h"
     "${PROJECT_SOURCE_DIR}/include/diplib/library/*.h"
     "${PROJECT_SOURCE_DIR}/include/diplib/private/*.h")
update_deps_file("DIPlib_sources" "${DIPLIB_SOURCES}")
add_library(DIP "${CMAKE_CURRENT_LIST_DIR}/support/matrix.cpp" ${DIPLIB_SOURCES}) # The matrix.cpp file takes annoyingly long to compile, let's compile it first!
target_include_directories(DIP PUBLIC
                           "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                           $<INSTALL_INTERFACE:include>)
target_compile_definitions(DIP PRIVATE DIP_VERSION_STRING="${PROJECT_VERSION}")
target_compile_definitions(DIP PRIVATE DIP_COPYRIGHT_YEAR="${DIP_COPYRIGHT_YEAR}")
if(DIP_SHARED_LIBRARY)
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_DIP_BUILD_SHARED)
else()
   target_compile_definitions(DIP PUBLIC DIP_CONFIG_DIP_IS_STATIC)
endif()

# Debugging mode
target_compile_definitions(DIP PRIVATE DIP_DEBUG_VERSION=$<CONFIG:Debug>)

# C++ version, etc.
set_target_properties(DIP PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
set_target_properties(DIP PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   if(${MSVC_VERSION} VERSION_LESS 1900)
      message(SEND_ERROR "You need MSVC 2015 or later to compile DIPlib.")
   endif()
else()
   target_compile_features(DIP PUBLIC cxx_generic_lambdas cxx_relaxed_constexpr) # This should force dependent targets to also use C++14.
endif()

# Multithreading
if(DIP_ENABLE_MULTITHREADING)
   if(TARGET OpenMP::OpenMP_CXX)
      target_link_libraries(DIP PRIVATE OpenMP::OpenMP_CXX)
   else()
      target_compile_options(DIP PRIVATE ${OpenMP_CXX_FLAGS})
      if(OpenMP_CXX_LIB_NAMES)
         target_link_libraries(DIP PRIVATE ${OpenMP_CXX_LIB_NAMES}) # OpenMP_CXX_LIBRARIES is empty?!
      else()
         target_link_libraries(DIP PRIVATE ${OpenMP_CXX_FLAGS}) # This is the case for CMake <= 3.8
      endif()
   endif()
endif()

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { char const* name = __PRETTY_FUNCTION__; }" HAS_PRETTY_FUNCTION)
if(HAS_PRETTY_FUNCTION)
   target_compile_definitions(DIP PUBLIC DIP_CONFIG_HAS_PRETTY_FUNCTION)
endif()

# Control exceptions
set(DIP_ENABLE_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
if(DIP_ENABLE_STACK_TRACE)
   target_compile_definitions(DIP PUBLIC DIP_CONFIG_ENABLE_STACK_TRACE)
endif()
set(DIP_ENABLE_ASSERT OFF CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")
if(DIP_ENABLE_ASSERT)
   target_compile_definitions(DIP PUBLIC DIP_CONFIG_ENABLE_ASSERT)
endif()

# Enable testing
# It's possible to disable DocTest using `DOCTEST_CONFIG_DISABLE`, but that would also disable any tests in code
# that uses DIPlib, hence we define a variable here that removes all of DocTest from the DIPlib sources.
set(DIP_ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")
if(DIP_ENABLE_DOCTEST)
   target_include_directories(DIP PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/doctest")
   target_compile_definitions(DIP PRIVATE
                              DIP_CONFIG_ENABLE_DOCTEST
                              DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
                              DIP_EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples")
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(DIP PRIVATE DIP_CONFIG_DOCTEST_IN_SHARED_LIB)
   endif()
endif()

# UFT-8 or plain old ASCII?
set(DIP_ENABLE_UNICODE ON CACHE BOOL "Enable UTF-8 encoded strings, if disabled, some text output will look more 'primitive'")
if(DIP_ENABLE_UNICODE)
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_ENABLE_UNICODE)
endif()

# Force 128-bit PRNG?
set(DIP_ALWAYS_128_PRNG OFF CACHE BOOL "Use the 128-bit PRNG code even if 128-bit integers are not natively supported by your platform")
if(DIP_ALWAYS_128_PRNG)
   target_compile_definitions(DIP PUBLIC DIP_CONFIG_ALWAYS_128_PRNG)
   set(HAS_128_INT TRUE)
else()
   check_cxx_source_compiles("int main() { int v = __SIZEOF_INT128__; return 0; }" HAS_128_INT)
endif()
set(HAS_128_INT ${HAS_128_INT} PARENT_SCOPE)

# Eigen
target_include_directories(DIP PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/eigen3")
target_compile_definitions(DIP PRIVATE
                           EIGEN_MPL2_ONLY # This makes sure we only use parts of the Eigen library that use the MPL2 license or more permissive ones.
                           EIGEN_DONT_PARALLELIZE) # This to prevent Eigen algorithms trying to run in parallel -- we parallelize at a larger scale.

# zlib (for use in libics and libtiff)
set(DIP_ENABLE_ZLIB ON CACHE BOOL "Enable zlib compression in ICS and TIFF (deflate), required for PNG")
if(DIP_ENABLE_ZLIB)
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/zlib" "${PROJECT_BINARY_DIR}/zlib" EXCLUDE_FROM_ALL)
endif()

# libjpeg (also for use in libtiff)
set(DIP_ENABLE_JPEG ON CACHE BOOL "Enable JPEG file support, and compression in TIFF")
if(DIP_ENABLE_JPEG)
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/libjpeg" "${PROJECT_BINARY_DIR}/libjpeg" EXCLUDE_FROM_ALL)
   target_link_libraries(DIP PRIVATE jpeg)
   target_include_directories(DIP PRIVATE $<TARGET_PROPERTY:jpeg,INTERFACE_INCLUDE_DIRECTORIES>) # these need to come before system include directories
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_JPEG)
endif()

# libics
set(DIP_ENABLE_ICS ON CACHE BOOL "Enable ICS file support")
if(DIP_ENABLE_ICS)
   set(LIBICS_INCLUDE_CPP Off) # TODO: we should start using the C++ interface
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/libics" "${PROJECT_BINARY_DIR}/libics" EXCLUDE_FROM_ALL)
   target_link_libraries(DIP PRIVATE libics)
   target_include_directories(DIP PRIVATE $<TARGET_PROPERTY:libics,INTERFACE_INCLUDE_DIRECTORIES>) # these need to come before system include directories
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_ICS)
endif()

# libtiff
set(DIP_ENABLE_TIFF ON CACHE BOOL "Enable TIFF file support")
if(DIP_ENABLE_TIFF)
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/libtiff" "${PROJECT_BINARY_DIR}/libtiff" EXCLUDE_FROM_ALL)
   target_link_libraries(DIP PRIVATE tiff)
   target_include_directories(DIP PRIVATE $<TARGET_PROPERTY:tiff,INTERFACE_INCLUDE_DIRECTORIES>) # these need to come before system include directories
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_TIFF)
endif()

# libspng
set(DIP_ENABLE_PNG ON CACHE BOOL "Enable PNG file support")
if(DIP_ENABLE_PNG AND NOT DIP_ENABLE_ZLIB)
   message(WARNING "PNG cannot be enabled without also enabling zlib -- disabling PNG.")
   set(DIP_ENABLE_PNG OFF CACHE BOOL "Enable PNG file support" FORCE)
endif()
if(DIP_ENABLE_PNG)
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/libspng" "${PROJECT_BINARY_DIR}/libspng" EXCLUDE_FROM_ALL)
   target_link_libraries(DIP PRIVATE spng_static)
   target_include_directories(DIP PRIVATE $<TARGET_PROPERTY:spng_static,INTERFACE_INCLUDE_DIRECTORIES>) # these need to come before system include directories
   target_include_directories(DIP PRIVATE $<TARGET_PROPERTY:zlibstatic,INTERFACE_INCLUDE_DIRECTORIES>)  # we're using the zlib header in png.cpp
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_PNG)
endif()

# FFTW or PocketFFT for Fourier transforms
find_package(FFTW3)
if(FFTW3_FOUND)
   set(DIP_ENABLE_FFTW OFF CACHE BOOL "Enable linking against the FFTW library (GPL license) for Fourier transforms")
endif()
if(DIP_ENABLE_FFTW)
   target_include_directories(DIP PRIVATE ${FFTW3_INCLUDE_DIRS})
   target_link_libraries(DIP PRIVATE ${FFTW3_LIBRARIES})
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_FFTW)
else()
   target_include_directories(DIP PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/pocketfft")
endif()

# Use FreeType
find_package(Freetype)
if(FREETYPE_FOUND)
   set(DIP_ENABLE_FREETYPE OFF CACHE BOOL "Enable linking against FreeType, for better text rendering in images")
endif()
if(DIP_ENABLE_FREETYPE)
   target_include_directories(DIP PRIVATE ${FREETYPE_INCLUDE_DIRS})
   target_link_libraries(DIP PRIVATE ${FREETYPE_LIBRARIES})
   target_compile_definitions(DIP PRIVATE DIP_CONFIG_HAS_FREETYPE)
endif()


# Install
install(TARGETS DIP EXPORT DIPlibTargets ${DIP_DESTINATIONS})
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/" DESTINATION include
        PATTERN "diplib/viewer" EXCLUDE PATTERN "dipviewer.h" EXCLUDE
        PATTERN "diplib/javaio" EXCLUDE PATTERN "diplib/javaio.h" EXCLUDE)
# To examine symbol table of the shared library on MacOSX: nm -g -p -U libDIP.dylib

# Uncomment next line to see compile times for each source file (make without -j).
#set_target_properties(DIP PROPERTIES RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")


# DIPlib unit tests
if(DIP_ENABLE_DOCTEST)
   add_executable(unit_tests EXCLUDE_FROM_ALL "${CMAKE_CURRENT_LIST_DIR}/library/unit_tests.cpp")
   target_include_directories(unit_tests PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/doctest")
   target_link_libraries(unit_tests PRIVATE DIP)
   target_compile_definitions(unit_tests PRIVATE
                              DIP_IMPLEMENT_UNIT_TESTS
                              DIP_CONFIG_ENABLE_DOCTEST
                              DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(unit_tests PRIVATE DIP_CONFIG_DOCTEST_IN_SHARED_LIB)
      if(APPLE)
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   else()
      include("${PROJECT_SOURCE_DIR}/dependencies/doctest/doctest_force_link_static_lib_in_target.cmake")
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
   add_custom_target(DIP_check COMMAND unit_tests --force-colors=true DEPENDS unit_tests)
   add_custom_target(check DEPENDS DIP_check)
   add_custom_target(check_memory COMMAND valgrind ./unit_tests DEPENDS unit_tests)
endif()
