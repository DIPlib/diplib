##############################################################################
#
# libics: Image Cytometry Standard file reading and writing.
#
# Copyright (C) 2000-2013, 2016 Cris Luengo and others
# Copyright 2015, 2016:
#   Scientific Volume Imaging Holding B.V.
#   Laapersveld 63, 1213 VB Hilversum, The Netherlands
#   https://www.svi.nl
#
#   CMakeLists.txt
#   Created by Paul Barber <paul.barber@oncology.ox.ac.uk>, Feb 2017
#   Heavily modified by Cris Luengo, July 2017
#
#   To aid building on Windows 10 and with Visual Studio 2015 (VC14).
#   Requires libics.def
#
##############################################################################
cmake_minimum_required(VERSION 3.0)

project(libics VERSION 1.6.1)

# Compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED on)
# TODO: These flags work for GCC and CLang, not sure about other compilers
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

# Zlib
find_package(ZLIB)
if(ZLIB_FOUND)
   set(USE_ZLIB TRUE CACHE BOOL "Use Zlib")
endif()
if(USE_ZLIB)
   include_directories(${ZLIB_INCLUDE_DIRS})
   add_definitions(-DICS_ZLIB)
endif()

# ICS
set(SOURCES
      libics_binary.c
      libics_compress.c
      libics_data.c
      libics_gzip.c
      libics_history.c
      libics_preview.c
      libics_read.c
      libics_sensor.c
      libics_test.c
      libics_top.c
      libics_util.c
      libics_write.c
      libics_conf.h
      )

set(HEADERS
      libics.h
      libics_intern.h
      libics_ll.h
      libics_sensor.h
      libics_test.h
      )

include_directories(${CMAKE_SOURCE_DIR})

include(CheckSymbolExists)
check_symbol_exists(strcasecmp string.h HAVE_STRING_STRCASECMP)
if(HAVE_STRING_STRCASECMP)
   add_definitions(-DHAVE_STRING_STRCASECMP)
   message(STATUS "strcasecmp found in string.h")
else()
   check_symbol_exists(strcasecmp strings.h HAVE_STRINGS_STRCASECMP)
   if(HAVE_STRINGS_STRCASECMP)
      add_definitions(-DHAVE_STRINGS_STRCASECMP)
      message(STATUS "strcasecmp found in strings.h")
   else()
      message(STATUS "strcasecmp not found")
   endif()
endif()

# TODO: rewrite the header file with this version number, so we don't duplicate effort.
#add_definitions(-DICSLIB_VERSION="${libics_VERSION}")

# Build a dll
add_library(libics SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(libics PRIVATE BUILD_ICSLIB) # For Windows, when compiling DLL
target_compile_definitions(libics INTERFACE USE_ICSLIB_DLL) # For Windows, when linking against DLL

# Build a static library
add_library(libics_static STATIC ${SOURCES} ${HEADERS})

if(USE_ZLIB)
    target_link_libraries(libics ${ZLIB_LIBRARIES})
    target_link_libraries(libics_static ${ZLIB_LIBRARIES})
endif()

# Install
install(TARGETS libics libics_static DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)

# Unit tests
enable_testing()
add_executable(test_ics1 EXCLUDE_FROM_ALL test_ics1.c)
target_link_libraries(test_ics1 libics)
add_executable(test_ics2a EXCLUDE_FROM_ALL test_ics2a.c)
target_link_libraries(test_ics2a libics)
add_executable(test_ics2b EXCLUDE_FROM_ALL test_ics2b.c)
target_link_libraries(test_ics2b libics)
add_executable(test_gzip EXCLUDE_FROM_ALL test_gzip.c)
target_link_libraries(test_gzip libics)
add_executable(test_compress EXCLUDE_FROM_ALL test_compress.c)
target_link_libraries(test_compress libics)
add_executable(test_strides EXCLUDE_FROM_ALL test_strides.c)
target_link_libraries(test_strides libics)
add_executable(test_strides2 EXCLUDE_FROM_ALL test_strides2.c)
target_link_libraries(test_strides2 libics)
add_executable(test_metadata EXCLUDE_FROM_ALL test_metadata.c)
target_link_libraries(test_metadata libics)
add_executable(test_history EXCLUDE_FROM_ALL test_history.c)
target_link_libraries(test_history libics)
add_custom_target(all_tests DEPENDS
      test_ics1
      test_ics2a
      test_ics2b
      test_gzip
      test_compress
      test_strides
      test_strides2
      test_metadata
      test_history
      )
add_test(ctest_build_test_code "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target all_tests)
add_test(NAME test_ics1 COMMAND test_ics1 ${CMAKE_SOURCE_DIR}/test/testim.ics result_v1.ics)
set_tests_properties(test_ics1 PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_ics2a COMMAND test_ics2a ${CMAKE_SOURCE_DIR}/test/testim.ics result_v2a.ics)
set_tests_properties(test_ics2a PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_ics2b COMMAND test_ics2b ${CMAKE_SOURCE_DIR}/test/testim.ics result_v2b.ics)
set_tests_properties(test_ics2b PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_gzip COMMAND test_gzip ${CMAKE_SOURCE_DIR}/test/testim.ics result_v2z.ics)
set_tests_properties(test_gzip PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_compress COMMAND test_compress ${CMAKE_SOURCE_DIR}/test/testim.ics ${CMAKE_SOURCE_DIR}/test/testim_c.ics)
set_tests_properties(test_compress PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_strides COMMAND test_strides ${CMAKE_SOURCE_DIR}/test/testim.ics result_s.ics)
set_tests_properties(test_strides PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_strides2 COMMAND test_strides2 ${CMAKE_SOURCE_DIR}/test/testim.ics result_s.ics)
set_tests_properties(test_strides2 PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_metadata1 COMMAND test_metadata result_v1.ics)
set_tests_properties(test_metadata1 PROPERTIES DEPENDS test_ics1)
add_test(NAME test_metadata2 COMMAND test_metadata result_v2a.ics)
set_tests_properties(test_metadata2 PROPERTIES DEPENDS test_ics2a)
add_test(NAME test_metadata3 COMMAND test_metadata result_v2b.ics)
set_tests_properties(test_metadata3 PROPERTIES DEPENDS test_ics2b)
add_test(NAME test_metadata4 COMMAND test_metadata result_v2z.ics)
set_tests_properties(test_metadata4 PROPERTIES DEPENDS test_gzip)
add_test(NAME test_history COMMAND test_history result_v1.ics)
set_tests_properties(test_history PROPERTIES DEPENDS test_ics1)
