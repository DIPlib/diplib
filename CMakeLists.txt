# CMakeLists file for DIPlib 3
cmake_minimum_required (VERSION 3.5) # It might work with older versions, but this is the version I'm testing with.
if(POLICY CMP0068)
   cmake_policy(SET CMP0068 NEW)
endif()

project (DIPlib VERSION 3.0)
add_definitions(-DDIP_VERSION_STRING="${DIPlib_VERSION}")

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
   # using Clang C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Wsign-conversion")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   # using GNU C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wsign-conversion -Wno-attributes -Wno-unknown-pragmas")
   # -Wno-attributes: "enum class DIP_EXPORT" causes a warning in GCC 5.4, fixed in 6.0.
   # -Wno-unknown-pragmas: "#pragma omp" causes a warning if OpenMP is not enabled.
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native") # To see which flags are enabled: gcc -march=native -Q --help=target
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
   # using Intel C++
   # TODO: compiler flags for Intel compiler
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   # using Visual Studio C++
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS) # Disable unchecked iterator warnings and unsafe (string manipulation) function warnings
   add_compile_options(/wd4251) # Disable "needs to have dll-interface" warning
endif()
set(CMAKE_INCLUDE_CURRENT_DIR 1)

# RPATH
#set(CMAKE_MACOSX_RPATH 1) # This is the default
#set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# OpenMP
find_package(OpenMP)
if (OpenMP_CXX_FOUND OR OPENMP_FOUND) # OPENMP_FOUND for CMake <= 3.8
   set(DIP_ENABLE_MULTITHREADING ON CACHE BOOL "Enable multithreading support")
endif()

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { char const* name = __PRETTY_FUNCTION__; return 0; }" HAS_PRETTY_FUNCTION)

# Control exceptions
set(DIP_EXCEPTIONS_RECORD_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
set(DIP_ENABLE_ASSERT ON CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")

# Enable testing
# It's possible to disable DocTest using `DOCTEST_CONFIG_DISABLE`, but that would also disable any tests in code
# that uses DIPlib, hence we define a variable here that removes all of DocTest from the DIPlib sources.
set(DIP_ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")

# UFT-8 or plain old ASCII?
set(DIP_ENABLE_UNICODE ON CACHE BOOL "Enable UTF-8 encoded strings, if disabled, some text output will look more 'primitive'")

# Force 128-bit PRNG?
set(DIP_ALWAYS_128_PRNG OFF CACHE BOOL "Use the 128-bit PRNG code even if 128-bit integers are not natively supported by your platform")
if(DIP_ALWAYS_128_PRNG)
   set(FORCE_128_INT TRUE)
   set(HAS_128_INT TRUE)
else()
   check_cxx_source_compiles("int main() { int v = __SIZEOF_INT128__; return 0; }" HAS_128_INT)
endif()

# Image file format libraries: libics, libtiff
set(DIP_ENABLE_ICS ON CACHE BOOL "Enable ICS file support")
if(DIP_ENABLE_ICS)
   add_subdirectory(dependencies/libics EXCLUDE_FROM_ALL)
endif()
find_package(TIFF)
if (TIFF_FOUND)
   set(DIP_ENABLE_TIFF ON CACHE BOOL "Enable TIFF file support")
endif()

# Utility for automatically updating targets when a file is added to the source directories
# from: https://stackoverflow.com/a/39971448/7328782, but modified
# Creates a file called ${name} with the list of dependencies in it.
# The file is updated when the list of dependencies changes.
# If the file is updated, cmake will automatically reload.
function(update_deps_file name deps)
   # Normalize the list so it's the same on every machine
   list(REMOVE_DUPLICATES deps)
   foreach(dep IN LISTS deps)
      file(RELATIVE_PATH rel_dep ${CMAKE_CURRENT_SOURCE_DIR} ${dep})
      list(APPEND rel_deps ${rel_dep})
   endforeach(dep)
   list(SORT rel_deps)
   # Split the list into lines, and add some CMake-valid syntax so it's ignored
   string(REPLACE ";" "\n" new_deps "${rel_deps}")
   set(new_deps "# Automatically generated, don't edit!\nset(${name}_bogus\n${new_deps}\n)\n")
   # Compare with the old file
   set(old_deps "")
   if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake")
      file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" old_deps)
   endif()
   if(NOT old_deps STREQUAL new_deps)
      file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" "${new_deps}")
   endif()
   # Include the file so it's tracked as a generation dependency (we don't need the content).
   include(${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake)
endfunction(update_deps_file)


### DIPlib library

file(GLOB DIPLIB_SOURCES src/*/*.h src/*/*.cpp include/*.h include/diplib/*.h include/diplib/library/*.h include/diplib/private/*.h)
update_deps_file("DIPlib_sources" "${DIPLIB_SOURCES}")
add_library(DIP src/support/matrix.cpp ${DIPLIB_SOURCES}) # The matrix.cpp file takes annoyingly long to compile, let's compile it first!
target_include_directories(DIP PUBLIC include)
include(GenerateExportHeader)
generate_export_header(DIP BASE_NAME dip)
# configuration
if(DIP_ENABLE_MULTITHREADING)
   target_compile_options(DIP PRIVATE ${OpenMP_CXX_FLAGS})
   if(OpenMP_CXX_LIB_NAMES)
      target_link_libraries(DIP ${OpenMP_CXX_LIB_NAMES}) # OpenMP_CXX_LIBRARIES is empty?!
   else()
      target_link_libraries(DIP ${OpenMP_CXX_FLAGS}) # This is the case for CMake <= 3.8
   endif()
endif()
if(HAS_PRETTY_FUNCTION)
   target_compile_definitions(DIP PUBLIC DIP__HAS_PRETTY_FUNCTION)
endif()
if(DIP_EXCEPTIONS_RECORD_STACK_TRACE)
   target_compile_definitions(DIP PUBLIC DIP__EXCEPTIONS_RECORD_STACK_TRACE)
endif()
if(DIP_ENABLE_ASSERT)
   target_compile_definitions(DIP PUBLIC DIP__ENABLE_ASSERT)
endif()
if(DIP_ENABLE_DOCTEST)
   target_include_directories(DIP PRIVATE dependencies/doctest)
   target_compile_definitions(DIP PRIVATE DIP__ENABLE_DOCTEST DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(DIP PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
   endif()
endif()
if(DIP_ENABLE_UNICODE)
   target_compile_definitions(DIP PRIVATE DIP__ENABLE_UNICODE)
endif()
if(FORCE_128_INT)
   target_compile_definitions(DIP PUBLIC DIP__ALWAYS_128_PRNG)
endif()
# Eigen
target_include_directories(DIP PRIVATE dependencies/eigen3)
target_compile_definitions(DIP PRIVATE EIGEN_MPL2_ONLY # This makes sure we only use parts of the Eigen library that use the MPL2 license or more permissive ones.
                                       EIGEN_DONT_PARALLELIZE) # This to prevent Eigen algorithms trying to run in parallel -- we parallelize at a larger scale.
# libics
if(DIP_ENABLE_ICS)
   target_include_directories(DIP PRIVATE dependencies/libics)
   target_link_libraries(DIP libics_static)
   target_compile_definitions(DIP PRIVATE DIP__HAS_ICS)
endif()
# libtiff
if(DIP_ENABLE_TIFF)
   target_include_directories(DIP PRIVATE ${TIFF_INCLUDE_DIR})
   target_link_libraries(DIP ${TIFF_LIBRARIES})
   target_compile_definitions(DIP PRIVATE DIP__HAS_TIFF)
endif()
#install
install(TARGETS DIP DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(FILES ${PROJECT_BINARY_DIR}/dip_export.h DESTINATION include)
# To examine symbol table of the shared library on MacOSX: nm -g -p -U libDIP.dylib

# Uncomment next line to see compile times for each source file (make without -j).
#set_target_properties(DIP PROPERTIES RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")


### DIPlib unit tests

if(DIP_ENABLE_DOCTEST)
   add_executable (unit_tests EXCLUDE_FROM_ALL test/unit_tests.cpp)
   target_include_directories(unit_tests PRIVATE dependencies/doctest)
   target_link_libraries(unit_tests DIP)
   target_compile_definitions(unit_tests PRIVATE DIP__ENABLE_DOCTEST DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(unit_tests PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
      if(APPLE)
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   else()
      include(dependencies/doctest/doctest_force_link_static_lib_in_target.cmake)
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
   add_custom_target(check COMMAND ./unit_tests)
   add_dependencies(check unit_tests)
   add_custom_target(check_memory COMMAND valgrind ./unit_tests)
   add_dependencies(check_memory unit_tests)
endif()

### DIPviewer

# OpenGL, FreeGLUT, GLFW
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/viewer/cmake)
find_package(OpenGL)
if(NOT APPLE)
   find_package(FreeGLUT)
endif()
if (FREEGLUT_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "FreeGLUT")
endif()
find_package(GLFW)
if (GLFW_FOUND)
   list(APPEND DIP_VIEWER_MANAGERS "GLFW")
endif()
if (OPENGL_FOUND AND (FREEGLUT_FOUND OR GLFW_FOUND))
   set(DIP_BUILD_DIPVIEWER ON CACHE BOOL "Build the viewer module")
endif()
if(WIN32 AND DIP_BUILD_DIPVIEWER)
   # On Windows, GLEW is required for OpenGL extensions
   find_package(GLEW REQUIRED)
endif()

if(DIP_BUILD_DIPVIEWER)
   file(GLOB DIP_VIEWER_SRC viewer/src/*.cpp viewer/src/manager/* viewer/include/diplib/viewer/*.h)
   update_deps_file("DIPviewer_sources" "${DIP_VIEWER_SRC}")
   add_library(DIPviewer ${DIP_VIEWER_SRC})
   target_include_directories(DIPviewer PUBLIC viewer/include)
   generate_export_header(DIPviewer BASE_NAME dipviewer)
   if (FREEGLUT_FOUND)
      target_compile_definitions(DIPviewer PUBLIC DIP__HAS_FREEGLUT)
      target_include_directories(DIPviewer PRIVATE ${OPENGL_INCLUDE_DIR} ${FREEGLUT_INCLUDE_DIR})
      target_link_libraries(DIPviewer ${FREEGLUT_LIBRARIES})
   endif()
   if (GLFW_FOUND)
      target_compile_definitions(DIPviewer PUBLIC DIP__HAS_GLFW)
      target_include_directories(DIPviewer PRIVATE ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS})
      target_link_libraries(DIPviewer ${GLFW_LIBRARIES})
   endif()
   if (WIN32)
      target_include_directories(DIPviewer PRIVATE ${GLEW_INCLUDE_DIRS})
      target_link_libraries(DIPviewer ${GLEW_LIBRARIES})
   endif()
   target_link_libraries(DIPviewer DIP ${OPENGL_LIBRARIES} -lpthread)
   if(APPLE)
      set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "@loader_path")
   else()
      set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "$ORIGIN")
   endif()
   install(TARGETS DIPviewer DESTINATION lib)
   install(DIRECTORY viewer/include/ DESTINATION include)
endif()

### PyDIP module for Python

set(DIP_BUILD_PYDIP ON CACHE BOOL "Build the PyDIP Python module")
if(DIP_BUILD_PYDIP)
   add_subdirectory(dependencies/pybind11 EXCLUDE_FROM_ALL)
   file(GLOB DIP_PYTHON_SRC pydip/*.cpp pydip/*.h)
   file(GLOB DIP_PYTHON_PY pydip/*.py)
   set(PyDIPdeps "${DIP_PYTHON_SRC}")
   list(APPEND PyDIPdeps "${DIP_PYTHON_PY}")
   update_deps_file("PyDIP_sources" "${PyDIPdeps}")
   pybind11_add_module(PyDIP_bin ${DIP_PYTHON_SRC})
   #target_compile_options(PyDIP_bin PRIVATE -Wno-deprecated-declarations)
   target_link_libraries(PyDIP_bin PUBLIC DIP)
   if(APPLE)
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "@loader_path/..")
   else()
      set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "$ORIGIN/..")
   endif()
   install(TARGETS PyDIP_bin DESTINATION ${PYDIP_INSTALL_PATH})
   # Install .py files
   install(FILES ${DIP_PYTHON_PY} DESTINATION ${PYDIP_INSTALL_PATH})
   add_custom_target(PyDIP DEPENDS PyDIP_bin)
endif()

### DIPimage toolbox for MATLAB

set(MATLAB_ADDITIONAL_VERSIONS "R2016a=9.0") # This is necessary because my version of MATLAB is very recent and my version of CMake is not. Ajust as necessary.
find_package(Matlab OPTIONAL_COMPONENTS MX_LIBRARY)
if (Matlab_FOUND)
   set(DIP_BUILD_DIPIMAGE ON CACHE BOOL "Build the DIPimage toolbox")
endif()

if(DIP_BUILD_DIPIMAGE)
   # Install M-files
   install(DIRECTORY dipimage/ DESTINATION ${DIPIMAGE_INSTALL_PATH} FILES_MATCHING PATTERN "*.m") # could also do PATTERN "*.cpp" EXCLUDE

   # Compile and install MEX-files in dipimage/@dip_image/private
   file(GLOB DIP_IMAGE_PRIVATE dipimage/@dip_image/private/*.cpp)
   set(DIPIMAGE_DEPS "${DIP_IMAGE_PRIVATE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../../lib/") # doesn't do anything on Windows, so this should be OK
   endif()
   set(DIP_IMAGE_PRIVATE_TARGETS "")
   foreach(file ${DIP_IMAGE_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIP_IMAGE_PRIVATE_TARGETS ${DIP_IMAGE_PRIVATE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_PRIVATE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image/private)

   # Compile and install MEX-files in dipimage/@dip_image
   file(GLOB DIP_IMAGE dipimage/@dip_image/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIP_IMAGE}")
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
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIP_IMAGE_TARGETS ${DIP_IMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIP_IMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/@dip_image)

   # Compile and install MEX-files in dipimage/private
   file(GLOB DIPIMAGE_PRIVATE dipimage/private/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIPIMAGE_PRIVATE}")
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/../../../lib/")
   else()
      set(LIB_REL_PATH "$ORIGIN/../../../lib/")
   endif()
   set(DIPIMAGE_PRIVATE_TARGETS "")
   foreach(file ${DIPIMAGE_PRIVATE})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIPIMAGE_PRIVATE_TARGETS ${DIPIMAGE_PRIVATE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_PRIVATE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH}/private)

   # Compile and install MEX-files in dipimage/
   file(GLOB DIPIMAGE dipimage/*.cpp)
   list(APPEND DIPIMAGE_DEPS "${DIPIMAGE}")
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
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(DIPIMAGE_TARGETS ${DIPIMAGE_TARGETS} MEX_${target})
   endforeach()
   install(TARGETS ${DIPIMAGE_TARGETS} DESTINATION ${DIPIMAGE_INSTALL_PATH})

   # Keep a file with all dependencies
   update_deps_file("DIPimage_sources" "${DIPIMAGE_DEPS}")
endif()


### DIPlib tests ### TEMPORARY! -- This should become an 'examples' directory

file(GLOB DIP_TESTS test/test_*.cpp)
set(TESTS_TARGETS "")
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
   set(TESTS_TARGETS ${TESTS_TARGETS} ${target})
endforeach()
target_link_libraries(test_viewer DIPviewer)
target_link_libraries(test_filters DIPviewer)

add_custom_target(tests DEPENDS ${TESTS_TARGETS})


### DIPlib documentation

set(DOXYGEN_SKIP_DOT On)
find_package(Doxygen)
if (DOXYGEN_FOUND)
   configure_file(src/documentation/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCUMENTATION_OUTPUT}
         COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names and other things..."
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/fix_doc_includes.sh ${DOCUMENTATION_OUTPUT}/html
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )
endif (DOXYGEN_FOUND)

#add_custom_command(
#      OUTPUT ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
#      COMMAND ${CMAKE_COMMAND} -E make_directory ${DOCUMENTATION_OUTPUT}
#      COMMAND pandoc -s DIPthoughts.md -o ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
#      --variable colorlinks=true --variable mainfont="Palatino" --latex-engine=xelatex
#      --toc --listings -H pandoc/listings_setup.tex --number-sections
#      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#      DEPENDS DIPthoughts.md
#)
#
#add_custom_target(
#      docs DEPENDS
#      ${DOCUMENTATION_OUTPUT}/DIPthoughts.pdf
#)


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

if(DIP_ENABLE_MULTITHREADING)
   message(" * Using OpenMP for multithreading")
else()
   message(" * Multithreading disabled")
endif()

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
   message(" * DIPimage toolbox not configured")
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
