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
set(PYDIP_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib/PyDIP)

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # Hiding all functions, to avoid settings mismatch with the MEX files
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") # also matchs "AppleClang"
   # Safe Clang compiler and architecture flags across all Intel and AMD processors (circa 2015)
   set(CMAKE_CXX_FLAGS "-Wall -Wconversion -Wsign-conversion")
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx -mavx2 -msse2 -mfma -mbmi")
   set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsign-conversion -Wno-attributes") # Wattributes: "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native") # To see which flags are enabled: gcc -march=native -Q --help=target
   set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
   # using Intel C++
   # TODO: compiler flags for Intel compiler
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   # using Visual Studio C++
   # TODO: compiler flags for MSVC compiler
endif()
set(CMAKE_INCLUDE_CURRENT_DIR 1)

# RPATH
#set(CMAKE_MACOSX_RPATH 1) # This is the default
#set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

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

# OpenGL and FreeGLUT
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/viewer/cmake)
find_package(OpenGL)
find_package(FreeGLUT)
if (OPENGL_FOUND AND FREEGLUT_FOUND)
   set(DIP_BUILD_DIPVIEWER ON CACHE BOOL "Build the viewer module")
endif()

# pybind11
set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
if(DIP_BUILD_PYDIP)
   add_subdirectory(dependencies/pybind11)
endif()

# MATLAB
set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY)
if (Matlab_FOUND)
   set(DIP_BUILD_DIPIMAGE ON CACHE BOOL "Build the DIPimage toolbox")
endif()

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
      "include/diplib/private/*.h"
)
add_library(DIP ${SOURCES} ${HEADERS})
include(GenerateExportHeader)
generate_export_header(DIP BASE_NAME dip)
install(TARGETS DIP DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/dip_export.h DESTINATION include)
# To examine symbol table of the shared library on MacOSX: nm -g -p -U libDIP.dylib

### DIPlib unit tests

if(DIP_ENABLE_DOCTEST)
   add_custom_target(check COMMAND ./unit_tests)
   enable_testing()
   add_executable (unit_tests EXCLUDE_FROM_ALL test/unit_tests.cpp)
   target_link_libraries(unit_tests DIP)
   if(DIP_SHARED_LIBRARY)
      if(APPLE)
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   else()
      include(dependencies/doctest/doctest_force_link_static_lib_in_target.cmake)
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
   add_test(NAME unit_tests COMMAND unit_tests)
   add_dependencies(check unit_tests)
endif()

### DIPviewer

if(DIP_BUILD_DIPVIEWER)
   file(GLOB DIP_VIEWER_SRC "viewer/src/*.cpp")
   file(GLOB DIP_VIEWER_H "viewer/include/diplib/viewer/*.h")
   add_library(DIPviewer ${DIP_VIEWER_SRC} ${DIP_VIEWER_H})
   target_include_directories(DIPviewer PRIVATE ${OPENGL_INCLUDE_DIR} ${FREEGLUT_INCLUDE_DIR} PUBLIC viewer/include)
   target_link_libraries(DIPviewer DIP ${FREEGLUT_LIBRARIES} ${OPENGL_LIBRARIES} -lpthread)
   install(TARGETS DIPviewer DESTINATION lib)
   install(DIRECTORY viewer/include/ DESTINATION include)
endif()

### PyDIP module for Python

if(DIP_BUILD_PYDIP)
   file(GLOB DIP_PYTHON_SRC "pydip/*.cpp")
   file(GLOB DIP_PYTHON_H "pydip/*.h")
   pybind11_add_module(PyDIP_bin ${DIP_PYTHON_SRC} ${DIP_PYTHON_H})
   #target_compile_options(PyDIP_bin PRIVATE -Wno-deprecated-declarations)
   target_link_libraries(PyDIP_bin PUBLIC DIP)
   if(APPLE)
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "@loader_path/..")
   else()
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "$ORIGIN/..")
   endif()
   install(TARGETS PyDIP_bin DESTINATION ${PYDIP_INSTALL_PATH})
   # Install .py files
   file(GLOB DIP_PYTHON_PY "pydip/*.py")
   install(FILES ${DIP_PYTHON_PY} DESTINATION ${PYDIP_INSTALL_PATH})
endif()

### DIPimage toolbox for MATLAB

if(DIP_BUILD_DIPIMAGE)
   # Install M-files
   install(DIRECTORY dipimage/ DESTINATION ${DIPIMAGE_INSTALL_PATH} FILES_MATCHING PATTERN "*.m") # could also do PATTERN "*.cpp" EXCLUDE

   # Compile and install MEX-files in dipimage/@dip_image/private
   file(GLOB DIP_PRIVATE "dipimage/@dip_image/private/*.cpp")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../../lib/") # doesn't do anything on Windows, so this should be OK
   endif()
   set(DIP_PRIVATE_TARGETS "")
   foreach(file ${DIP_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      set(DIP_PRIVATE_TARGETS ${DIP_PRIVATE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_PRIVATE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image/private)

   # Compile and install MEX-files in dipimage/@dip_image
   file(GLOB DIP_IMAGE "dipimage/@dip_image/*.cpp")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../lib/")
   endif()
   set(DIP_IMAGE_TARGETS "")
   foreach(file ${DIP_IMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      set(DIP_IMAGE_TARGETS ${DIP_IMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image)

   # Compile and install MEX-files in dipimage/private
   file(GLOB DIPIMAGE "dipimage/private/*.cpp")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../lib/")
   endif()
   set(DIPIMAGE_TARGETS "")
   foreach(file ${DIPIMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      set(DIPIMAGE_TARGETS ${DIPIMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/private)

   # Compile and install MEX-files in dipimage/
   file(GLOB DIPIMAGE "dipimage/*.cpp")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../lib/")
   endif()
   set(DIPIMAGE_TARGETS "")
   foreach(file ${DIPIMAGE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      set(DIPIMAGE_TARGETS ${DIPIMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH})
endif()


### DIPlib tests ### TEMPORARY! -- This should become an 'examples' directory

file(GLOB DIP_TESTS "test/test_*.cpp")
foreach(file ${DIP_TESTS})
   get_filename_component(target ${file} NAME_WE)
   add_executable(${target} EXCLUDE_FROM_ALL ${file})
   target_link_libraries(${target} DIP)
   if(DIP_SHARED_LIBRARY)
      if(APPLE)
         set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   endif()
endforeach()
target_link_libraries(test_viewer DIPviewer)

add_custom_target(
      tests DEPENDS
      test_error
      test_physdims
      test_image
      test_framework_scan test_lambdafun test_framework_separable test_framework_separable2 test_framework_full
      test_boundary
      test_measurement test_measuretool
      test_viewer
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
      DEPENDS DIPthoughts.md
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

if(DIP_BUILD_DIPVIEWER)
   message(" * DIPviewer module added to 'all' target")
else()
   message(" * DIPviewer module not configured")
endif()

if(DIP_BUILD_PYDIP)
   message(" * PyDIP module added to 'all' target")
   message(" * PyDIP installed to: ${PYDIP_INSTALL_PATH}")
else()
   message(" * PyDIP module not configured")
endif()

if(DIP_BUILD_DIPIMAGE)
   message(" * DIPimage toolbox added to 'all' target")
   message(" * DIPimage toolbox installed to: ${DIPIMAGE_INSTALL_PATH}")
else()
   message(" * DIPimage toolbox cannot be compiled")
endif()

if (DOXYGEN_FOUND)
   message(" * Doxygen found, 'apidoc' target available")
   message(" * Documentation installed to: ${DOCUMENTATION_OUTPUT}")
else()
   message(" * Doxygen not found, 'apidoc' target not available")
endif()

message("")
message("--------------------------------------")
message("")
