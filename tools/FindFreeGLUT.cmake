# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Modified by Cris Luengo to set a target.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#  Note:
#     Searching headers and libraries is very simple and is NOT as powerful as scripts
#     distributed with CMake, because LuaDist defines directories to search for.
#     Everyone is encouraged to contact the author with improvements. Maybe this file
#     becomes part of CMake distribution sometime.

# - Find FreeGLUT
# Find the native FreeGLUT headers and libraries.
#
#  FREEGLUT_INCLUDE_DIRS - where to find freeglut.h, etc.
#  FREEGLUT_LIBRARIES    - List of libraries when using FreeGLUT.
#  FREEGLUT_FOUND        - True if FreeGLUT found.
#  FREEGLUT::FREEGLUT    - Target to link to (automatically sets include directories and linked libraries)

# Look for the header file.
find_path(FREEGLUT_INCLUDE_DIR NAMES GL/freeglut.h)

# Look for the library.
find_library(FREEGLUT_LIBRARY NAMES freeglut glut)

# Handle the QUIETLY and REQUIRED arguments and set FreeGLUT_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeGLUT DEFAULT_MSG FREEGLUT_LIBRARY FREEGLUT_INCLUDE_DIR)

# Copy the results to the output variables.
if(FREEGLUT_FOUND)
   set(FREEGLUT_LIBRARIES ${FREEGLUT_LIBRARY})
   set(FREEGLUT_INCLUDE_DIRS ${FREEGLUT_INCLUDE_DIR})
   if (NOT TARGET FREEGLUT::FREEGLUT)
      add_library(FREEGLUT::FREEGLUT UNKNOWN IMPORTED)
      set_target_properties(FREEGLUT::FREEGLUT PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${FREEGLUT_INCLUDE_DIRS}"
                            IMPORTED_LOCATION "${FREEGLUT_LIBRARY}")
   endif()
   message(STATUS "FreeGLUT found: ${FREEGLUT_LIBRARY} -- ${FREEGLUT_INCLUDE_DIR}")
else()
   set(FREEGLUT_LIBRARIES)
   set(FREEGLUT_INCLUDE_DIRS)
   message(STATUS "FreeGLUT not found")
endif()

mark_as_advanced(FREEGLUT_INCLUDE_DIRS FREEGLUT_LIBRARIES)
