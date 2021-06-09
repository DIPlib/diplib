# Copyright (C) 2021 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Modified by Cris Luengo to look for GLFW instead of FreeGLUT, and set a target.
# Modified by Milian Wolff to look for ICS instead of GLFW, and set a target.
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#  Note:
#     Searching headers and libraries is very simple and is NOT as powerful as scripts
#     distributed with CMake, because LuaDist defines directories to search for.
#     Everyone is encouraged to contact the author with improvements. Maybe this file
#     becomes part of CMake distribution sometime.

# - Find ICS
# Find the native ICS headers and libraries.
#
#  ICS_INCLUDE_DIRS - where to find libics.h
#  ICS_LIBRARIES    - List of libraries when using ICS.
#  ICS_FOUND        - True if ICS found.
#  ICS::ICS        - Target to link to (automatically sets include directories and linked libraries)

# Look for the header file.
find_path(ICS_INCLUDE_DIR NAMES libics.h)

# Look for the library.
find_library(ICS_LIBRARY NAMES ics)

# Handle the QUIETLY and REQUIRED arguments and set ICS_FOUND to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICS DEFAULT_MSG ICS_LIBRARY ICS_INCLUDE_DIR)

# Copy the results to the output variables.
if(ICS_FOUND)
   set(ICS_LIBRARIES ${ICS_LIBRARY})
   set(ICS_INCLUDE_DIRS ${ICS_INCLUDE_DIR})
   if (NOT TARGET ICS::ICS)
      add_library(ICS::ICS UNKNOWN IMPORTED)
      set_target_properties(ICS::ICS PROPERTIES
                            INTERFACE_INCLUDE_DIRECTORIES "${ICS_INCLUDE_DIRS}"
                            IMPORTED_LOCATION "${ICS_LIBRARY}")
   endif()
   message(STATUS "ICS found: ${ICS_LIBRARY} -- ${ICS_INCLUDE_DIR}")
else()
   set(ICS_LIBRARIES)
   set(ICS_INCLUDE_DIRS)
   message(STATUS "ICS not found")
endif()

mark_as_advanced(ICS_INCLUDE_DIRS ICS_LIBRARIES)
