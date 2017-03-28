# CMakeLists file for DIPlib 3

cmake_minimum_required (VERSION 3.5) # It might work with older versions, but this is the version I'm testing with.

project (DIPlib VERSION 3.0)

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Release$")
   message(STATUS "Configured for release")
elseif(CMAKE_BUILD_TYPE MATCHES "^Debug$")
   message(STATUS "Configured for debugging")
else()
   message(WARNING "Not configured for release or debugging!: ${CMAKE_BUILD_TYPE}")
endif()

# Static or dynamic?
set(DIP_SHARED_LIBRARY ON CACHE BOOL "Build a shared library (off for static library)")
if(DIP_SHARED_LIBRARY)
   set(BUILD_SHARED_LIBS On)
endif()

# Installation path
set(CMAKE_INSTALL_PREFIX "${CMAKE_BUILD_TYPE}" CACHE PATH "Installation directory")
set(DOCUMENTATION_OUTPUT ${CMAKE_INSTALL_PREFIX}/share/doc/DIPlib)
set(DIPIMAGE_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/share/dipimage)

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # Hiding all functions, to avoid settings mismatch with the MEX files
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") # also matchs "AppleClang"
   # Safe Clang compiler and architecture flags across all Intel and AMD processors (circa 2015)
   set(CMAKE_CXX_FLAGS "-Wall")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx -mavx2 -msse2 -mfma -mbmi")
   set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   # Safe GCC compiler and architecture flags across all Intel and AMD processors (circa 2015)
   set(CMAKE_CXX_FLAGS "-Wall -Wno-attributes") # Wattributes: "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx -mavx2 -msse2 -mfma -mbmi")
   set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
   # using Intel C++
   # TODO: compiler flags for Intel compiler
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   # using Visual Studio C++
   # TODO: compiler flags for MSVC compiler
endif()

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
      "int main() { char const* name = __PRETTY_FUNCTION__; return 0; }"
      HAS_PRETTY_FUNCTION
)
if(HAS_PRETTY_FUNCTION)
   add_definitions(-DDIP__HAS_PRETTY_FUNCTION)
endif()

# Control exceptions
set(DIP_EXCEPTIONS_RECORD_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
if(DIP_EXCEPTIONS_RECORD_STACK_TRACE)
   add_definitions(-DDIP__EXCEPTIONS_RECORD_STACK_TRACE)
endif()
set(DIP_ENABLE_ASSERT ON CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")
if(DIP_ENABLE_ASSERT)
   add_definitions(-DDIP__ENABLE_ASSERT)
endif()

# Enable testing
set(DIP_ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")
if(DIP_ENABLE_DOCTEST)
   include_directories(dependencies/doctest)
   add_definitions(-DDIP__ENABLE_DOCTEST -DDOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      add_definitions(-DDIP__DOCTEST_IN_SHARED_LIB)
   endif()
endif()

# UFT-8 or plain old ASCII?
set(DIP_ENABLE_UNICODE ON CACHE BOOL "Enable UTF-8 encoded strings, if disabled, some text output will look more 'primitive'")
if(DIP_ENABLE_UNICODE)
   add_definitions(-DDIP__ENABLE_UNICODE)
endif()

# Eigen
include_directories(dependencies/eigen3)
add_definitions(-DEIGEN_MPL2_ONLY) # This makes sure we only use parts of the Eigen library that use the MPL2 license or more permissive ones.
add_definitions(-DEIGEN_DONT_PARALLELIZE) # This to prevent Eigen algorithms trying to run in parallel -- we parallelize at a larger scale.

# MATLAB
set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY)

# Doxygen
find_package(Doxygen)


### DIPlib library

include_directories(include)
file(
      GLOB SOURCES
      "src/*/*.h"
      "src/*/*.cpp"
)
file(
      GLOB HEADERS
      "include/*.h"
      "include/diplib/*.h"
      "include/diplib/library/*.h"
)
add_library(DIP ${SOURCES} ${HEADERS})
include(GenerateExportHeader)
include_directories(${PROJECT_BINARY_DIR})
generate_export_header(DIP BASE_NAME dip)
install(TARGETS DIP DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/dip_export.h DESTINATION include)


### DIPlib unit tests

if(DIP_ENABLE_DOCTEST)
   add_custom_target(check COMMAND ./unit_tests)
   enable_testing()
   add_executable (unit_tests EXCLUDE_FROM_ALL test/unit_tests.cpp)
   target_link_libraries(unit_tests DIP)
   add_test(NAME unit_tests COMMAND unit_tests)
   add_dependencies(check unit_tests)
   if(NOT DIP_SHARED_LIBRARY)
      include(dependencies/doctest/doctest_force_link_static_lib_in_target.cmake)
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
endif()


### DIPimage toolbox for MATLAB

if(Matlab_FOUND)
   # Install M-files
   install(DIRECTORY dipimage/ DESTINATION ${DIPIMAGE_INSTALL_PATH} FILES_MATCHING PATTERN "*.m") # could also do PATTERN "*.cpp" EXCLUDE

   # Compile and install MEX-files in @dip_image/private
   file(GLOB DIP_PRIVATE "dipimage/@dip_image/private/*.cpp")
   set(DIP_PRIVATE_TARGETS "")
   foreach(file ${DIP_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME ${target} SRC ${file} LINK_TO DIP)
      set(DIP_PRIVATE_TARGETS ${DIP_PRIVATE_TARGETS} ${target})
   endforeach()
   install(TARGETS ${DIP_PRIVATE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image/private)

   # Compile and install MEX-files in @dip_image
   file(GLOB DIP_IMAGE "dipimage/@dip_image/*.cpp")
   set(DIP_IMAGE_TARGETS "")
   foreach(file ${DIP_IMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME ${target} SRC ${file} LINK_TO DIP)
      set(DIP_IMAGE_TARGETS ${DIP_IMAGE_TARGETS} ${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image)

   # Compile and install MEX-files in toolbox directory
   file(GLOB DIPIMAGE "dipimage/*.cpp")
   set(DIPIMAGE_TARGETS "")
   foreach(file ${DIPIMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME ${target} SRC ${file} LINK_TO DIP)
      set(DIPIMAGE_TARGETS ${DIPIMAGE_TARGETS} ${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH})
endif()


### DIPlib tests ### TEMPORARY!

add_executable(test_error EXCLUDE_FROM_ALL test/test_error.cpp)
add_executable(test_physdims EXCLUDE_FROM_ALL test/test_physdims.cpp)
target_link_libraries(test_physdims DIP)
add_executable(test_image EXCLUDE_FROM_ALL test/test_image.cpp)
target_link_libraries(test_image DIP)
add_executable( test_framework_scan EXCLUDE_FROM_ALL test/test_framework_scan.cpp )
target_link_libraries(test_framework_scan DIP)
add_executable( test_lambdafun EXCLUDE_FROM_ALL test/test_lambdafun.cpp )
target_link_libraries(test_lambdafun DIP)
add_executable( test_framework_separable EXCLUDE_FROM_ALL test/test_framework_separable.cpp )
target_link_libraries(test_framework_separable DIP)
add_executable( test_framework_separable2 EXCLUDE_FROM_ALL test/test_framework_separable2.cpp )
target_link_libraries(test_framework_separable2 DIP)
add_executable( test_framework_full EXCLUDE_FROM_ALL test/test_framework_full.cpp )
target_link_libraries(test_framework_full DIP)
add_executable( test_boundary EXCLUDE_FROM_ALL test/test_boundary.cpp )
target_link_libraries(test_boundary DIP)
add_executable( test_measurement EXCLUDE_FROM_ALL test/test_measurement.cpp )
target_link_libraries(test_measurement DIP)
add_executable( test_measuretool EXCLUDE_FROM_ALL test/test_measuretool.cpp )
target_link_libraries(test_measuretool DIP)

add_executable(test_fft_kissfft EXCLUDE_FROM_ALL test/test_fft_kissfft.cpp test/kissfft.h)
add_executable(test_fft_opencv EXCLUDE_FROM_ALL test/test_fft_opencv.cpp src/transform/opencv_dxt.h)

add_custom_target(
      tests DEPENDS
      test_error
      test_physdims
      test_image
      test_framework_scan test_lambdafun test_framework_separable test_framework_separable2 test_framework_full
      test_boundary
      test_measurement test_measuretool
      test_fft_kissfft test_fft_opencv
)


### DIPlib documentation

if (DOXYGEN_FOUND)
   configure_file(src/documentation/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCUMENTATION_OUTPUT}
         COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names and other things..."
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/fix_doc_includes.sh ${DOCUMENTATION_OUTPUT}/html
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         DEPENDS ${DOXYGEN_INPUT}
   )
endif (DOXYGEN_FOUND)

add_custom_command(
      OUTPUT ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
      COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCUMENTATION_OUTPUT}
      COMMAND pandoc -s DIPthoughts.md -o ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
      --variable colorlinks=true --variable mainfont="Palatino" --latex-engine=xelatex
      --toc --listings -H pandoc/listings_setup.tex --number-sections
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${DIPthoughts.md}
)

add_custom_target(
      docs DEPENDS
      ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
)


### Status report

message("")
message("--------------------------------------")
message("   CONFIGURATION REPORT")
message("")

if(CMAKE_BUILD_TYPE MATCHES "^Release$")
   message(" * Configured for release")
elseif(CMAKE_BUILD_TYPE MATCHES "^Debug$")
   message(" * Configured for debugging")
endif()

if(DIP_SHARED_LIBRARY)
   message(" * Building shared library")
else()
   message(" * Building static library")
endif()
message(" * Library installed to: ${CMAKE_INSTALL_PREFIX}/lib")
message(" * Header files installed to: ${CMAKE_INSTALL_PREFIX}/include")

if(DIP_EXCEPTIONS_RECORD_STACK_TRACE)
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

if(DIP_ENABLE_DOCTEST)
   message(" * Unit test code included")
else()
   message(" * Unit test code excluded")
endif()

if (DOXYGEN_FOUND)
   message(" * Doxygen found, 'apidoc' target available")
   message(" * Documentation installed to: ${DOCUMENTATION_OUTPUT}")
else()
   message(" * Doxygen not found, 'apidoc' target not available")
endif()

if(Matlab_FOUND)
   message(" * MATLAB found, DIPimage added to ALL target")
   message(" * DIPimage installed to: ${DIPIMAGE_INSTALL_PATH}")
else()
   message(" * MATLAB not found, DIPimage cannot be compiled")
endif()

message("")
message("--------------------------------------")
message("")
