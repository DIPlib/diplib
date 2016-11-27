# CMakeLists file for DIPlib 3.0

cmake_minimum_required (VERSION 3.5.2) # It might work with older versions, but this is the version I'm testing with.

project (DIPlib)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

# Safe gcc compiler and architecture flags across all Intel and AMD processors (circa 2015)
# This optimizes things for GCC and CLANG, but presumably shouldn't be here for MSVC++.
set(CMAKE_CXX_FLAGS "-Wall -Wno-c++14-extensions")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx -mavx2 -msse2 -mfma -mbmi")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
      "int main() { char const* name = __PRETTY_FUNCTION__; return 0; }"
      HAS_PRETTY_FUNCTION)
if(HAS_PRETTY_FUNCTION)
   add_definitions(-DHAS_PRETTY_FUNCTION)
endif()

# Control exceptions
set(EXCEPTIONS_RECORD_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
if(EXCEPTIONS_RECORD_STACK_TRACE)
   add_definitions(-DEXCEPTIONS_RECORD_STACK_TRACE)
endif()
set(ENABLE_ASSERT ON CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")
if(ENABLE_ASSERT)
   add_definitions(-DENABLE_ASSERT)
endif()

# MATLAB
set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY)

# Doxygen
find_package(Doxygen)
set(DOCUMENTATION_OUTPUT doc)


# DIPlib library

include_directories(include)
file(GLOB SOURCES
   "src/*/*.h"
   "src/*/*.cpp"
   )
file(GLOB HEADERS
   "include/*.h"
   "include/diplib/*.h"
   )
#add_library(DIP DYNAMIC ${SOURCES} ${HEADERS}) # Why does this not work any more? What changed?
add_library(DIP STATIC ${SOURCES} ${HEADERS})


# DIPlib tests

add_executable(test_array EXCLUDE_FROM_ALL test/test_array.cpp)
add_executable(test_bin EXCLUDE_FROM_ALL test/test_bin.cpp)
add_executable(test_error EXCLUDE_FROM_ALL test/test_error.cpp)
add_executable(test_saturated_arithmetic EXCLUDE_FROM_ALL test/test_saturated_arithmetic.cpp)
add_executable(test_options EXCLUDE_FROM_ALL test/test_options.cpp)
add_executable(test_physdims EXCLUDE_FROM_ALL test/test_physdims.cpp)
add_executable(test_image EXCLUDE_FROM_ALL test/test_image.cpp)
target_link_libraries(test_image DIP)
add_executable( test_framework_scan EXCLUDE_FROM_ALL test/test_framework_scan.cpp )
target_link_libraries(test_framework_scan DIP)
add_executable( test_framework_separable EXCLUDE_FROM_ALL test/test_framework_separable.cpp )
target_link_libraries(test_framework_separable DIP)
add_executable( test_boundary EXCLUDE_FROM_ALL test/test_boundary.cpp )
target_link_libraries(test_boundary DIP)

add_custom_target(tests DEPENDS test_array test_bin test_error test_saturated_arithmetic test_options test_physdims test_image test_framework_scan test_framework_separable test_boundary)

if(Matlab_FOUND)

   matlab_add_mex(NAME mextest SRC test/mextest.cpp LINK_TO DIP)
   set_target_properties(mextest PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_function SRC test/mextest_function.cpp LINK_TO DIP)
   set_target_properties(mextest_function PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_iterators SRC test/mextest_iterators.cpp LINK_TO DIP)
   set_target_properties(mextest_iterators PROPERTIES EXCLUDE_FROM_ALL 1)

   add_custom_target(mex DEPENDS mextest mextest_function mextest_iterators)

endif(Matlab_FOUND)


# DIPlib documentation

if (DOXYGEN_FOUND)

   set(DOXYGEN_INPUT Doxyfile)

   add_custom_command(
      OUTPUT ${DOCUMENTATION_OUTPUT}
      COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
      COMMAND ${CMAKE_COMMAND} -E echo "Done."
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${DOXYGEN_INPUT}
      )

   add_custom_target(apidoc DEPENDS ${DOCUMENTATION_OUTPUT})

endif (DOXYGEN_FOUND)

add_custom_command(
      OUTPUT ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
      COMMAND pandoc -s DIPthoughts.md -o ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
      --variable colorlinks=true --variable mainfont="Palatino" --latex-engine=xelatex
      --toc --listings -H pandoc/listings_setup.tex --number-sections
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${DIPthoughts.md}
)

add_custom_command(
      OUTPUT ${DOCUMENTATION_OUTPUT}/DIPworkplan.pdf
      COMMAND pandoc -s DIPworkplan.md -o ${DOCUMENTATION_OUTPUT}/DIPworkplan.pdf
      --variable colorlinks=true --variable mainfont="Palatino" --latex-engine=xelatex
      --toc --listings -H pandoc/listings_setup.tex --number-sections
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${DIPworkplan.md}
)

add_custom_target(docs DEPENDS ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf ${DOCUMENTATION_OUTPUT}/DIPworkplan.pdf)
