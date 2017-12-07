# CMake Find script for FFTW3
#
# Prerequisites:
#  FFTW3_ROOT_DIR           - root directory
#  FFTW3_PRECISIONS         - list of precisions, which must be a combination of
#                             "single", "double" and/or "long-double",
#                             defaulting to "single;double"
#
# Will define:
#  FFTW3_FOUND              - FFTW3 was found
#  FFTW3_INCLUDE_DIRS       - include directories
#  FFTW3_LIBRARY_...        - library for a certain precision
#  FFTW3_LIBRARIES          - all libraries

# Keep track of all files to be handled
set(ALL_LIBS)

# Determine FFTW3 root
if(NOT FFTW3_ROOT_DIR AND ENV{FFTW3_ROOT_DIR})
  set(FFTW3_ROOT_DIR $ENV{FFTW3_ROOT_DIR})
endif()
#message("FFTW3_ROOT_DIR = ${FFTW3_ROOT_DIR}")

# Set default precisions to single and double
if(NOT FFTW3_PRECISIONS)
   set(FFTW3_PRECISIONS "single;double" CACHE STRING "Supported precisions, for example: single;double;long-double")
endif()

# Set lib name depending on precision
set(LIB_NAMES)
foreach(FFTW3_PRECISION ${FFTW3_PRECISIONS})
  if(FFTW3_PRECISION STREQUAL "single")
     set(LIB_NAME fftw3f)
  elseif(FFTW3_PRECISION STREQUAL "double")
     set(LIB_NAME fftw3)
  elseif(FFTW3_PRECISION STREQUAL "long-double")
     set(LIB_NAME fftw3l)
  else()
     message(FATAL_ERROR "FindFFTW3: precision `${FFTW3_PRECISION}' incorrect! Must be 'single', 'double' or 'long-double'.")
  endif()
  list(APPEND LIB_NAMES ${LIB_NAME})
  if (NOT WIN32)
    list(APPEND LIB_NAMES ${LIB_NAME}_threads)
  endif ()
endforeach()
#message("Lib names: ${LIB_NAMES}")

# Find the libraries
set(FFTW3_LIBRARIES)
foreach(LIB_NAME ${LIB_NAMES})
  string(TOUPPER ${LIB_NAME} LIB_VARNAME)
  if (WIN32)
     # Lib is named differently for Windows
     set(LIB_FILENAME lib${LIB_NAME}-3)
  else ()
     set(LIB_FILENAME ${LIB_NAME})
  endif ()
  #message("Looking for ${LIB_FILENAME}")
  find_library(FFTW3_LIBRARY_${LIB_VARNAME} ${LIB_FILENAME} HINTS ${FFTW3_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(FFTW3_LIBRARY_${LIB_VARNAME})
  #message("Found: ${FFTW3_LIBRARY_${LIB_VARNAME}}")
  list(APPEND FFTW3_LIBRARIES ${FFTW3_LIBRARY_${LIB_VARNAME}})
  list(APPEND ALL_LIBS "FFTW3_LIBRARY_${LIB_VARNAME}")
endforeach()
# TODO: add threads library?
mark_as_advanced(FFTW3_LIBRARIES)
#message(STATUS "FFTW3 libraries: ${FFTW3_LIBRARIES}")

# Find the header file
find_path(FFTW3_INCLUDE_DIR fftw3.h HINTS ${FFTW3_ROOT_DIR} PATH_SUFFIXES include)
set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_INCLUDE_DIRS)

# Handles the REQUIRED, QUIET and version-related arguments to find_package()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW3 DEFAULT_MSG FFTW3_INCLUDE_DIR ${ALL_LIBS})
