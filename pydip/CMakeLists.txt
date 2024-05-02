# (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
# (c)2018-2024, Wouter Caarls, Cris Luengo.
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


# CMakeLists file for DIPlib 3
cmake_minimum_required(VERSION 3.12...3.28)

project(PyDIP)

# Debug or Release?
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

# RPATH
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1) # Prevent relinking when installing

# Extend CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../tools")

include(${CMAKE_CURRENT_LIST_DIR}/../tools/compiler_flags.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../tools/update_deps_file.cmake)

# Python
find_package(Python 3.6 REQUIRED COMPONENTS Interpreter Development)

# Pybind11
set(PYBIND11_FINDPYTHON On)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../dependencies/pybind11" "${PROJECT_BINARY_DIR}/pybind11" EXCLUDE_FROM_ALL)

# DIPlib
if(NOT TARGET DIP)
   find_package(DIPlib REQUIRED)
   add_library(DIP ALIAS DIPlib::DIP)
   if(TARGET DIPlib::DIPviewer)
      add_library(DIPviewer ALIAS DIPlib::DIPviewer)
   endif()
   if(TARGET DIPlib::DIPjavaio)
      add_library(DIPjavaio ALIAS DIPlib::DIPjavaio)
   endif()
endif()

# Find sources
file(GLOB DIP_PYTHON_SRC "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
set(PyDIPdeps "${DIP_PYTHON_SRC}")
file(GLOB DIP_PYTHON_PY "${CMAKE_CURRENT_LIST_DIR}/src/*.py")
list(APPEND PyDIPdeps "${DIP_PYTHON_PY}")
set(DIP_PYDIP_WHEEL_INCLUDE_LIBS OFF CACHE BOOL "Include libraries in PyDIP wheel (turn on for binary distribution, keep off for personal builds)")
if(APPLE)
   set(rpath "@loader_path")
else()
   set(rpath "$ORIGIN")
endif()
if (DIP_PYDIP_WHEEL_INCLUDE_LIBS)
   set(pydip_wheel_include_libs True)
else()
   set(pydip_wheel_include_libs False)
   # Still search current directory first, for unit tests
   set(rpath "${rpath};${CMAKE_INSTALL_PREFIX}/${LIBRARY_DESTINATION}")
endif()
configure_file("${CMAKE_CURRENT_LIST_DIR}/src/__init__.py.in" "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)
list(APPEND DIP_PYTHON_PY "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")
list(APPEND PyDIPdeps "${CMAKE_CURRENT_LIST_DIR}/src/__init__.py.in")
update_deps_file("PyDIP_sources" "${PyDIPdeps}")

# Build PyDIP_bin
pybind11_add_module(PyDIP_bin ${DIP_PYTHON_SRC})
target_link_libraries(PyDIP_bin PRIVATE DIP)
set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "${rpath}")

# C++ version, etc.
set_target_properties(PyDIP_bin PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
set_target_properties(PyDIP_bin PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)

add_custom_target(PyDIP DEPENDS PyDIP_bin)

# Repeat the above for PyDIPviewer
if(TARGET DIPviewer)

   # Find sources
   file(GLOB DIP_PYTHON_VIEWER_SRC "${CMAKE_CURRENT_LIST_DIR}/src/viewer/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/viewer/*.h")
   update_deps_file("PyDIPviewer_sources" "${DIP_PYTHON_VIEWER_SRC}")

   # Build PyDIPviewer
   pybind11_add_module(PyDIPviewer ${DIP_PYTHON_VIEWER_SRC})
   target_include_directories(PyDIPviewer PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")
   target_link_libraries(PyDIPviewer PRIVATE DIP DIPviewer)
   set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "${rpath}")

   # C++ version, etc.
   set_target_properties(PyDIPviewer PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
   set_target_properties(PyDIPviewer PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)

   add_dependencies(PyDIP PyDIPviewer)

endif()

# Repeat the above for PyDIPjavaio
if(TARGET DIPjavaio)

   # Find sources
   file(GLOB DIP_PYTHON_JAVAIO_SRC "${CMAKE_CURRENT_LIST_DIR}/src/javaio/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/javaio/*.h")
   update_deps_file("PyDIPjavaio_sources" "${DIP_PYTHON_JAVAIO_SRC}")

   # Build PyDIPjavaio
   pybind11_add_module(PyDIPjavaio ${DIP_PYTHON_JAVAIO_SRC})
   target_link_libraries(PyDIPjavaio PRIVATE DIP DIPjavaio)
   set_target_properties(PyDIPjavaio PROPERTIES INSTALL_RPATH "${rpath}")

   # C++ version, etc.
   set_target_properties(PyDIPjavaio PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
   set_target_properties(PyDIPjavaio PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)

   add_dependencies(PyDIP PyDIPjavaio)

endif()

# PyDIP wheel creation
configure_file(${CMAKE_CURRENT_LIST_DIR}/setup/stage_wheel.py.in ${CMAKE_CURRENT_BINARY_DIR}/stage_wheel.py.out)
configure_file(${CMAKE_CURRENT_LIST_DIR}/setup/setup.py.in ${CMAKE_CURRENT_BINARY_DIR}/setup.py.out)

file(GENERATE
   OUTPUT "$<TARGET_FILE_DIR:PyDIP_bin>/stage_wheel.py"
   INPUT "${CMAKE_CURRENT_BINARY_DIR}/stage_wheel.py.out"
)
file(GENERATE
   OUTPUT "$<TARGET_FILE_DIR:PyDIP_bin>/setup.py"
   INPUT "${CMAKE_CURRENT_BINARY_DIR}/setup.py.out"
)
file(GENERATE
   OUTPUT "$<TARGET_FILE_DIR:PyDIP_bin>/cmake_pip_install.cmake"
   CONTENT "file(STRINGS $<TARGET_FILE_DIR:PyDIP_bin>/wheel.txt PYDIP_WHEEL_NAME)
   execute_process(COMMAND ${Python_EXECUTABLE} -m pip install --upgrade --force-reinstall \${PYDIP_WHEEL_NAME})"
)

add_custom_target(staging
                  COMMAND ${Python_EXECUTABLE} $<TARGET_FILE_DIR:PyDIP_bin>/stage_wheel.py
                  DEPENDS PyDIP)

add_custom_target(bdist_wheel
                  COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:PyDIP_bin>/staging ${Python_EXECUTABLE} -m build --wheel --no-isolation
                  DEPENDS staging)

add_custom_target(pip_install
                  COMMAND ${CMAKE_COMMAND} -P $<TARGET_FILE_DIR:PyDIP_bin>/cmake_pip_install.cmake
                  DEPENDS bdist_wheel)

add_custom_target(pip_uninstall
                  COMMAND ${Python_EXECUTABLE} -m pip uninstall diplib)

# PyDIP unit tests
# Resolve standard CMake variables
configure_file(${CMAKE_CURRENT_LIST_DIR}/test/unit_tests.py.in ${CMAKE_CURRENT_BINARY_DIR}/unit_tests.py.out @ONLY)

# Resolve generator expressions
file(GENERATE
   OUTPUT $<TARGET_FILE_DIR:PyDIP_bin>/unit_tests.py
   INPUT ${CMAKE_CURRENT_BINARY_DIR}/unit_tests.py.out
)

add_custom_target(PyDIP_check
                  COMMAND ${CMAKE_COMMAND} -E chdir $<TARGET_FILE_DIR:PyDIP_bin> ${Python_EXECUTABLE} $<TARGET_FILE_DIR:PyDIP_bin>/unit_tests.py
                  DEPENDS staging)
if(TARGET check)
   add_dependencies(check PyDIP_check)
else()
   add_custom_target(check DEPENDS PyDIP_check)
endif()
