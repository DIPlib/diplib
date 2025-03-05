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
##############################################################################
cmake_minimum_required(VERSION 3.5...3.28)

project(libics VERSION 1.6.2)

# Note: the version number above is not yet used anywhere.
# TODO: rewrite the header file with this version number.
# The line below does not work: we need the installed header file to contain the version number also.
#add_definitions(-DICSLIB_VERSION="${libics_VERSION}")

# Compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED on)

if(CMAKE_C_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -pedantic")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -Wno-unused-parameter -pedantic")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
   # TODO: compiler flags for MSVC compiler
endif()

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

# ICS
#configure_file(libics_conf.h.in ${CMAKE_CURRENT_SOURCE_DIR}/libics_conf.h COPYONLY) # Commented out for DIPlib
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

add_library(libics STATIC ${SOURCES} ${HEADERS}) # For DIPlib: added STATIC

#if(BUILD_SHARED_LIBS)
#   target_compile_definitions(libics PRIVATE BUILD_ICSLIB) # When compiling DLL/SO
#   target_compile_definitions(libics INTERFACE USE_ICSLIB_DLL) # When linking against DLL/SO
#endif()

target_include_directories(libics PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>) # This line added for DIPlib

set_target_properties(libics PROPERTIES C_VISIBILITY_PRESET hidden POSITION_INDEPENDENT_CODE TRUE)

if(UNIX)
   set_target_properties(libics PROPERTIES OUTPUT_NAME "ics")
   target_link_libraries(libics PUBLIC m)
endif(UNIX)

# Link against zlib
#find_package(ZLIB)
#if(ZLIB_FOUND)
#   set(LIBICS_USE_ZLIB TRUE CACHE BOOL "Use Zlib in libics")
#endif()
#if(LIBICS_USE_ZLIB)
#   target_link_libraries(libics PUBLIC ${ZLIB_LIBRARIES})
#   target_include_directories(libics PRIVATE ${ZLIB_INCLUDE_DIRS})
#   target_compile_definitions(libics PUBLIC -DICS_ZLIB)
#endif()

# DIPlib-specific zlib
if(TARGET zlibstatic)
   target_link_libraries(libics PRIVATE zlibstatic)
   target_compile_definitions(libics PUBLIC -DICS_ZLIB)
endif()

# Reentrant string tokenization
include(CheckFunctionExists)
check_function_exists(strtok_r HAVE_STRTOK_R)
if (HAVE_STRTOK_R)
  target_compile_definitions(libics PRIVATE -DHAVE_STRTOK_R)
endif()

# Install -- No installing for DIPlib
#export(TARGETS libics FILE cmake/libicsTargets.cmake)

#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#      cmake/libicsConfigVersion.cmake
#      VERSION ${PACKAGE_VERSION}
#      COMPATIBILITY AnyNewerVersion)

#configure_package_config_file(
#      cmake/libicsConfig.cmake.in
#      cmake/libicsConfig.cmake
#      INSTALL_DESTINATION cmake/)

#install(TARGETS libics
#      EXPORT libicsTargets
#      ARCHIVE DESTINATION lib
#      LIBRARY DESTINATION lib
#      RUNTIME DESTINATION bin
#      INCLUDES DESTINATION include)

#install(FILES ${HEADERS} DESTINATION include)

#install(FILES
#      ${CMAKE_CURRENT_BINARY_DIR}/cmake/libicsConfig.cmake
#      ${CMAKE_CURRENT_BINARY_DIR}/cmake/libicsConfigVersion.cmake
#      DESTINATION cmake/)

#install(EXPORT libicsTargets DESTINATION cmake)


# Unit tests
#enable_testing()

#add_executable(test_ics1 EXCLUDE_FROM_ALL test_ics1.c)
#target_link_libraries(test_ics1 libics)
#add_executable(test_ics2a EXCLUDE_FROM_ALL test_ics2a.c)
#target_link_libraries(test_ics2a libics)
#add_executable(test_ics2b EXCLUDE_FROM_ALL test_ics2b.c)
#target_link_libraries(test_ics2b libics)
#if(LIBICS_USE_ZLIB)
#   add_executable(test_gzip EXCLUDE_FROM_ALL test_gzip.c)
#   target_link_libraries(test_gzip libics)
#endif()
#add_executable(test_compress EXCLUDE_FROM_ALL test_compress.c)
#target_link_libraries(test_compress libics)
#add_executable(test_strides EXCLUDE_FROM_ALL test_strides.c)
#target_link_libraries(test_strides libics)
#add_executable(test_strides2 EXCLUDE_FROM_ALL test_strides2.c)
#target_link_libraries(test_strides2 libics)
#add_executable(test_strides3 EXCLUDE_FROM_ALL test_strides3.c)
#target_link_libraries(test_strides3 libics)
#add_executable(test_metadata EXCLUDE_FROM_ALL test_metadata.c)
#target_link_libraries(test_metadata libics)
#add_executable(test_history EXCLUDE_FROM_ALL test_history.c)
#target_link_libraries(test_history libics)

#set(TEST_PROGRAMS
#      test_ics1
#      test_ics2a
#      test_ics2b
#      test_compress
#      test_strides
#      test_strides2
#      test_strides3
#      test_metadata
#      test_history
#      )
#if(LIBICS_USE_ZLIB)
#   set(TEST_PROGRAMS ${TEST_PROGRAMS} test_gzip)
#endif()
#add_custom_target(all_tests DEPENDS ${TEST_PROGRAMS})

#add_test(ctest_build_test_code "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}" --target all_tests)
#add_test(NAME test_ics1 COMMAND test_ics1 "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v1.ics)
#set_tests_properties(test_ics1 PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_ics2a COMMAND test_ics2a "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v2a.ics)
#set_tests_properties(test_ics2a PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_ics2b COMMAND test_ics2b "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v2b.ics)
#set_tests_properties(test_ics2b PROPERTIES DEPENDS ctest_build_test_code)
#if(LIBICS_USE_ZLIB)
#   add_test(NAME test_gzip COMMAND test_gzip "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v2z.ics)
#   set_tests_properties(test_gzip PROPERTIES DEPENDS ctest_build_test_code)
#endif()
#add_test(NAME test_compress COMMAND test_compress "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" "${CMAKE_CURRENT_SOURCE_DIR}/test/testim_c.ics")
#set_tests_properties(test_compress PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_strides COMMAND test_strides "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_s.ics)
#set_tests_properties(test_strides PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_strides2 COMMAND test_strides2 "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_s2.ics)
#set_tests_properties(test_strides2 PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_strides3 COMMAND test_strides3 "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_s3.ics)
#set_tests_properties(test_strides3 PROPERTIES DEPENDS ctest_build_test_code)
#add_test(NAME test_metadata1 COMMAND test_metadata result_v1.ics)
#set_tests_properties(test_metadata1 PROPERTIES DEPENDS test_ics1)
#add_test(NAME test_metadata2 COMMAND test_metadata result_v2a.ics)
#set_tests_properties(test_metadata2 PROPERTIES DEPENDS test_ics2a)
#add_test(NAME test_metadata3 COMMAND test_metadata result_v2b.ics)
#set_tests_properties(test_metadata3 PROPERTIES DEPENDS test_ics2b)
#if(LIBICS_USE_ZLIB)
#   add_test(NAME test_metadata4 COMMAND test_metadata result_v2z.ics)
#   set_tests_properties(test_metadata4 PROPERTIES DEPENDS test_gzip)
#endif()
#add_test(NAME test_history COMMAND test_history result_v1.ics)
#set_tests_properties(test_history PROPERTIES DEPENDS test_ics1)


# Include the C++ interface?
#set(LIBICS_INCLUDE_CPP TRUE CACHE BOOL "Include the C++ interface")
#if(LIBICS_INCLUDE_CPP)
#   include(support/cpp_interface/CMakeLists.txt)
#endif()
