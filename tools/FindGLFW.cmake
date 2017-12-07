# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Modified by Cris Luengo to look for GLFW instead of FreeGLUT, and set a target.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#  Note:
#     Searching headers and libraries is very simple and is NOT as powerful as scripts
#     distributed with CMake, because LuaDist defines directories to search for.
#     Everyone is encouraged to contact the author with improvements. Maybe this file
#     becomes part of CMake distribution sometime.

# - Find GLFW
# Find the native GLFW headers and libraries.
#
#  GLFW_INCLUDE_DIRS - where to find glfw3.h
#  GLFW_LIBRARIES    - List of libraries when using GLFW.
#  GLFW_FOUND        - True if GLFW found.
#  GLFW::GLFW        - Target to link to (automatically sets include directories and linked libraries)

# Look for the header file.
find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h)

# Look for the library.
find_library(GLFW_LIBRARY NAMES glfw)

# Handle the QUIETLY and REQUIRED arguments and set GLFW_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)

# Copy the results to the output variables.
if(GLFW_FOUND)
   set(GLFW_LIBRARIES ${GLFW_LIBRARY})
   set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
   if (NOT TARGET GLFW::GLFW)
      add_library(GLFW::GLFW UNKNOWN IMPORTED)
      set_target_properties(GLFW::GLFW PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIRS}"
                            IMPORTED_LOCATION "${GLFW_LIBRARY}")
   endif()
   message(STATUS "GLFW found: ${GLFW_LIBRARY} -- ${GLFW_INCLUDE_DIR}")
else()
   set(GLFW_LIBRARIES)
   set(GLFW_INCLUDE_DIRS)
   message(STATUS "GLFW not found")
endif()

mark_as_advanced(GLFW_INCLUDE_DIRS GLFW_LIBRARIES)
