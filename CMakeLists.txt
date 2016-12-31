# CMakeLists file for DIPlib 3.0

# USAGE:
#    mkdir target
#    cd target
#    cmake /path/to/dip/root/directory
#    make -j         # builds the library (builds the default DIP target)
#    make -j DIP     # builds the library
#    make -j check   # builds the unit_tests program and runs it
#    make -j tests   # deprecated -- old test functions
#    make -j mex     # deprecated -- mex test functions
#    make docs       # makes some PDF project documentation files
#    make apidoc     # Doxygen HTML documentation for the library API

cmake_minimum_required (VERSION 3.5.2) # It might work with older versions, but this is the version I'm testing with.

project (DIPlib)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # Hiding all functions, to avoid settings mismatch with the MEX files
set(CMAKE_VISIBILITY_INLINES_HIDDEN hidden)

# Safe gcc compiler and architecture flags across all Intel and AMD processors (circa 2015)
# This optimizes things for GCC and CLANG, but presumably shouldn't be here for MSVC++.
set(CMAKE_CXX_FLAGS "-Wall -Wno-c++14-extensions")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mavx -mavx2 -msse2 -mfma -mbmi")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

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
set(EXCEPTIONS_RECORD_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
if(EXCEPTIONS_RECORD_STACK_TRACE)
   add_definitions(-DDIP__EXCEPTIONS_RECORD_STACK_TRACE)
endif()
set(ENABLE_ASSERT ON CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")
if(ENABLE_ASSERT)
   add_definitions(-DDIP__ENABLE_ASSERT)
endif()

# Enable testing
set(ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")
if(ENABLE_DOCTEST)
   include_directories(dependencies/doctest)
   add_definitions(-DDIP__ENABLE_DOCTEST -DDOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
endif()

# MATLAB
set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY)

# Doxygen
find_package(Doxygen)
set(DOCUMENTATION_OUTPUT doc)


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
add_library(DIP STATIC ${SOURCES} ${HEADERS})

#add_library(DIP DYNAMIC ${SOURCES} ${HEADERS})
# If you want to compile a dynamic library, you need to set export properties for all public functions.
# See CMake command GenerateExportHeader()


### DIPlib unit tests

if(ENABLE_DOCTEST)
   add_custom_target(check COMMAND ./unit_tests)
   enable_testing()
   add_executable (unit_tests EXCLUDE_FROM_ALL test/test.cpp)
   target_link_libraries(unit_tests DIP)
   add_test(NAME unit_tests COMMAND unit_tests)
   add_dependencies(check unit_tests)
   include(dependencies/doctest/doctest_force_link_static_lib_in_target.cmake)
   doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
endif()


### DIPlib tests

add_executable(test_error EXCLUDE_FROM_ALL test/test_error.cpp)
add_executable(test_physdims EXCLUDE_FROM_ALL test/test_physdims.cpp)
add_executable(test_image EXCLUDE_FROM_ALL test/test_image.cpp)
target_link_libraries(test_image DIP)
add_executable( test_framework_scan EXCLUDE_FROM_ALL test/test_framework_scan.cpp )
target_link_libraries(test_framework_scan DIP)
add_executable( test_framework_separable EXCLUDE_FROM_ALL test/test_framework_separable.cpp )
target_link_libraries(test_framework_separable DIP)
add_executable( test_boundary EXCLUDE_FROM_ALL test/test_boundary.cpp )
target_link_libraries(test_boundary DIP)
add_executable( test_measurement EXCLUDE_FROM_ALL test/test_measurement.cpp )
target_link_libraries(test_measurement DIP)

add_custom_target(tests DEPENDS test_error test_physdims test_image test_framework_scan test_framework_separable test_boundary test_measurement)

if(Matlab_FOUND)

   matlab_add_mex(NAME mextest SRC test/mextest.cpp LINK_TO DIP)
   set_target_properties(mextest PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_function SRC test/mextest_function.cpp LINK_TO DIP)
   set_target_properties(mextest_function PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_iterators SRC test/mextest_iterators.cpp LINK_TO DIP)
   set_target_properties(mextest_iterators PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_measure SRC test/mextest_measure.cpp LINK_TO DIP)
   set_target_properties(mextest_measure PROPERTIES EXCLUDE_FROM_ALL 1)

   matlab_add_mex(NAME mextest_maxmin SRC test/mextest_maxmin.cpp LINK_TO DIP)
   set_target_properties(mextest_maxmin PROPERTIES EXCLUDE_FROM_ALL 1)

   add_custom_target(mex DEPENDS mextest mextest_function mextest_iterators mextest_measure mextest_maxmin)

endif(Matlab_FOUND)


### DIPlib documentation

if (DOXYGEN_FOUND)

   set(DOXYGEN_INPUT Doxyfile)

   add_custom_command(
         OUTPUT ${DOCUMENTATION_OUTPUT}
         COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation..."
         COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names..."
         COMMAND tools/fix_doc_includes.sh ${DOCUMENTATION_OUTPUT}/html
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

add_custom_command(
      OUTPUT ${DOCUMENTATION_OUTPUT}/DIPproposal.pdf
      COMMAND pandoc -s DIPproposal.md -o ${DOCUMENTATION_OUTPUT}/DIPproposal.pdf
      --variable colorlinks=true --variable mainfont="Palatino" --latex-engine=xelatex
      --toc --listings -H pandoc/listings_setup.tex --number-sections
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${DIPproposal.md}
)

add_custom_target(docs DEPENDS
                  ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
                  ${DOCUMENTATION_OUTPUT}/DIPworkplan.pdf
                  ${DOCUMENTATION_OUTPUT}/DIPproposal.pdf)
