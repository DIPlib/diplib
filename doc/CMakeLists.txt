# DIPlib documentation
set(DOXYGEN_SKIP_DOT ON)
find_package(Doxygen)
if(DOXYGEN_FOUND)
   configure_file("${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
   configure_file("${CMAKE_CURRENT_LIST_DIR}/Doxyfile-mcss.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-mcss" @ONLY)
   add_custom_target(
         apidoc_old
         COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
         COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names and other things..."
         COMMAND "${CMAKE_CURRENT_LIST_DIR}/tools/fix_doc_includes.sh" "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}/html"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   )
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
         COMMAND "./doxygen.py" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-mcss"
         WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/m.css"
   )
endif(DOXYGEN_FOUND)
set(DOXYGEN_FOUND ${DOXYGEN_FOUND} PARENT_SCOPE)
