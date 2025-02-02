# (c)2015-2022, Cris Luengo
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


# The dox++ program
set(DIP_DOXPP_PATH "/Users/cris/src/dox++/" CACHE PATH "Directory to the dox++ program  (https://crisluengo.github.io/doxpp/)")

find_program(
   DOXPP_PARSE_EXECUTABLE
   NAMES dox++parse
   PATHS ${DIP_DOXPP_PATH}
   DOC "dox++ documentation generation tool, parser"
)
mark_as_advanced(FORCE DOXPP_PARSE_EXECUTABLE)

find_program(
   DOXPP_HTML_EXECUTABLE
   NAMES dox++html
   PATHS ${DIP_DOXPP_PATH}
   DOC "dox++ documentation generation tool, HTML generator"
)
mark_as_advanced(FORCE DOXPP_HTML_EXECUTABLE)

find_program(
   DOXPP_BRIEF_EXECUTABLE
   NAMES dox++brief
   PATHS ${DIP_DOXPP_PATH}
   DOC "dox++ documentation generation tool, brief generator"
)
mark_as_advanced(FORCE DOXPP_BRIEF_EXECUTABLE)

find_program(
   DOXPP_URLS_EXECUTABLE
   NAMES dox++urls
   PATHS ${DIP_DOXPP_PATH}
   DOC "dox++ documentation generation tool, URLs generator"
)
mark_as_advanced(FORCE DOXPP_URLS_EXECUTABLE)

if(DOXPP_PARSE_EXECUTABLE AND DOXPP_HTML_EXECUTABLE AND DOXPP_BRIEF_EXECUTABLE AND DOXPP_URLS_EXECUTABLE)
   message(STATUS "Found dox++parse: ${DOXPP_PARSE_EXECUTABLE}")
   message(STATUS "Found dox++html: ${DOXPP_HTML_EXECUTABLE}")
   message(STATUS "Found dox++brief: ${DOXPP_BRIEF_EXECUTABLE}")
   message(STATUS "Found dox++urls: ${DOXPP_URLS_EXECUTABLE}")
   set(DOXPP_FOUND on)
else()
   set(DOXPP_FOUND off)
endif()
set(DOXPP_FOUND ${DOXPP_FOUND} PARENT_SCOPE)

if(DOXPP_FOUND)
   find_package(OpenCV QUIET COMPONENTS core)
   if(NOT OpenCV_FOUND)
      set(OpenCV_INCLUDE_DIRS "")
   endif()
   find_package(Vigra QUIET)
   if(NOT Vigra_FOUND)
      set(Vigra_INCLUDE_DIRS "")
   endif()
   configure_file("${CMAKE_CURRENT_LIST_DIR}/dox++config.in" "${CMAKE_CURRENT_BINARY_DIR}/dox++config" @ONLY)
   add_custom_target(
      doc
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
      COMMAND "${DOXPP_PARSE_EXECUTABLE}"
      COMMAND "${DOXPP_BRIEF_EXECUTABLE}"
      COMMAND "${DOXPP_URLS_EXECUTABLE}"
      COMMAND "${DOXPP_HTML_EXECUTABLE}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   )
endif()
