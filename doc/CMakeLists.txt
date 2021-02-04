# DIPlib documentation
find_program(
      DOXPP_PARSE_EXECUTABLE
      NAMES dox++parse
      PATHS
         /Users/cris/src/dox++/
      DOC "dox++ documentation generation tool, parser (https://crisluengo.github.io/doxpp/)"
)
mark_as_advanced(DOXPP_PARSE_EXECUTABLE)

find_program(
      DOXPP_HTML_EXECUTABLE
      NAMES dox++html
      PATHS
         /Users/cris/src/dox++/
      DOC "dox++ documentation generation tool, HTML generator (https://crisluengo.github.io/doxpp/)"
)
mark_as_advanced(DOXPP_HTML_EXECUTABLE)

if(DOXPP_PARSE_EXECUTABLE AND DOXPP_HTML_EXECUTABLE)
   message(STATUS "Found dox++parse: ${DOXPP_PARSE_EXECUTABLE}")
   message(STATUS "Found dox++html: ${DOXPP_HTML_EXECUTABLE}")
   set(DOXPP_FOUND on)
else()
   set(DOXPP_FOUND off)
endif()
set(DOXPP_FOUND ${DOXPP_FOUND} PARENT_SCOPE)

if(DOXPP_FOUND)
   configure_file("${CMAKE_CURRENT_LIST_DIR}/dox++config.in" "${CMAKE_CURRENT_BINARY_DIR}/dox++config" @ONLY)
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
         COMMAND "${DOXPP_PARSE_EXECUTABLE}"
         COMMAND "${DOXPP_HTML_EXECUTABLE}"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   )
endif()
