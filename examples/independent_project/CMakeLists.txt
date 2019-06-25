# This CMake script shows how to link to DIPlib and DIPviewer from a project that does not build
# those libraries directly, but instead uses the libraries as installed on the system.
#
# Note that it is necessary to use the same compiler as was used to build the libraries, otherwise
# linking will likely fail. The ABI (binary interface) for C++ is not standardized across compilers
# (or even different versions of the same compiler).
#
# If DIPlib and DIPviewer were installed to a non-standard location, provide this location to
# CMake using the `CMAKE_PREFIX_PATH` variable:
#
#    cmake ~/src/diplib/examples/independent_project/ -DCMAKE_PREFIX_PATH=~/.local/

cmake_minimum_required(VERSION 3.5)

project(fileviewer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(DIPlib 3.0 REQUIRED DIP DIPviewer)
# find_package finds the file ${...}/lib/cmake/DIPlib/DIPlibConfig.cmake

add_executable(fileviewer ../cpp/fileviewer.cpp)
target_link_libraries(fileviewer PRIVATE DIPlib::DIP DIPlib::DIPviewer)
