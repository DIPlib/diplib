# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Modified by Cris Luengo to look for GLFW instead of FreeGLUT.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find GLFW
# Find the native GLFW headers and libraries.
#
#  GLFW_INCLUDE_DIRS - where to find glfw3.h
#  GLFW_LIBRARIES    - List of libraries when using GLFW.
#  GLFW_FOUND        - True if GLFW found.

# Look for the header file.
FIND_PATH(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h)

# Look for the library.
FIND_LIBRARY(GLFW_LIBRARY NAMES glfw)

# Handle the QUIETLY and REQUIRED arguments and set GLFW_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)

# Copy the results to the output variables.
IF(GLFW_FOUND)
	SET(GLFW_LIBRARIES ${GLFW_LIBRARY})
	SET(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
   MESSAGE(STATUS "GLFW found: ${GLFW_LIBRARY} -- ${GLFW_INCLUDE_DIR}")
ELSE()
	SET(GLFW_LIBRARIES)
	SET(GLFW_INCLUDE_DIRS)
   MESSAGE(STATUS "GLFW not found")
ENDIF()

MARK_AS_ADVANCED(GLFW_INCLUDE_DIRS GLFW_LIBRARIES)
