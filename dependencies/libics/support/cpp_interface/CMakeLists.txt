##############################################################################
#
# libics: Image Cytometry Standard file reading and writing.
#
# C++ interface
# Copyright 2018 Cris Luengo
#
##############################################################################

# Compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion -pedantic")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -pedantic")
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
   # TODO: compiler flags for Intel compiler
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
   # TODO: compiler flags for MSVC compiler
endif()

set(SOURCES ${CMAKE_CURRENT_LIST_DIR}/libics.cpp)
set(HEADERS ${CMAKE_CURRENT_LIST_DIR}/libics.hpp)

add_library(libics_cpp ${SOURCES} ${HEADERS})
target_link_libraries(libics_cpp PRIVATE libics)

if(BUILD_SHARED_LIBS)
   target_compile_definitions(libics_cpp PRIVATE BUILD_ICSCPP) # When compiling DLL/SO
   target_compile_definitions(libics_cpp INTERFACE USE_ICSCPP_DLL) # When linking against DLL/SO
endif()

target_include_directories(libics_cpp PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if(UNIX)
   set_target_properties(libics_cpp PROPERTIES OUTPUT_NAME "ics_cpp")
endif(UNIX)

# Install
export(TARGETS libics_cpp FILE cmake/libics_cppTargets.cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
      cmake/libics_cppConfigVersion.cmake
      VERSION ${PACKAGE_VERSION}
      COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libicsConfig.cmake.in
      cmake/libics_cppConfig.cmake
      INSTALL_DESTINATION cmake/)

install(TARGETS libics_cpp
      EXPORT libics_cppTargets
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
      INCLUDES DESTINATION include)

install(FILES ${HEADERS} DESTINATION include)

install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/cmake/libics_cppConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/cmake/libics_cppConfigVersion.cmake
      DESTINATION cmake/)

install(EXPORT libics_cppTargets DESTINATION cmake)


# Unit tests
add_executable(test_ics2a_cpp EXCLUDE_FROM_ALL ${CMAKE_CURRENT_LIST_DIR}/test_ics2a.cpp)
target_link_libraries(test_ics2a_cpp libics_cpp)
add_executable(test_ics2b_cpp EXCLUDE_FROM_ALL ${CMAKE_CURRENT_LIST_DIR}/test_ics2b.cpp)
target_link_libraries(test_ics2b_cpp libics_cpp)
add_executable(test_metadata_cpp EXCLUDE_FROM_ALL ${CMAKE_CURRENT_LIST_DIR}/test_metadata.cpp)
target_link_libraries(test_metadata_cpp libics_cpp)
add_executable(test_history_cpp EXCLUDE_FROM_ALL ${CMAKE_CURRENT_LIST_DIR}/test_history.cpp)
target_link_libraries(test_history_cpp libics_cpp)

set(TEST_PROGRAMS ${TEST_PROGRAMS} test_ics2a_cpp test_ics2b_cpp test_metadata_cpp test_history_cpp)
add_dependencies(all_tests ${TEST_PROGRAMS})

add_test(NAME test_ics2a_cpp COMMAND test_ics2a_cpp "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v2a_cpp.ics)
set_tests_properties(test_ics2a_cpp PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_ics2b_cpp COMMAND test_ics2b_cpp "${CMAKE_CURRENT_SOURCE_DIR}/test/testim.ics" result_v2b_cpp.ics)
set_tests_properties(test_ics2b_cpp PROPERTIES DEPENDS ctest_build_test_code)
add_test(NAME test_metadata1_cpp COMMAND test_metadata_cpp result_v1.ics)
set_tests_properties(test_metadata1_cpp PROPERTIES DEPENDS test_ics1)
add_test(NAME test_metadata2_cpp COMMAND test_metadata_cpp result_v2a.ics)
set_tests_properties(test_metadata2_cpp PROPERTIES DEPENDS test_ics2a)
add_test(NAME test_metadata3_cpp COMMAND test_metadata_cpp result_v2b.ics)
set_tests_properties(test_metadata3_cpp PROPERTIES DEPENDS test_ics2b)
if(LIBICS_USE_ZLIB)
   add_test(NAME test_metadata4_cpp COMMAND test_metadata_cpp result_v2z.ics)
   set_tests_properties(test_metadata4_cpp PROPERTIES DEPENDS test_gzip)
endif()
add_test(NAME test_history_cpp COMMAND test_history_cpp result_v1.ics)
set_tests_properties(test_history_cpp PROPERTIES DEPENDS test_ics1)
