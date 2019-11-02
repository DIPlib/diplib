add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/pybind11" "${PROJECT_BINARY_DIR}/pybind11" EXCLUDE_FROM_ALL)

# Find sources
file(GLOB DIP_PYTHON_SRC "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/*.h")
set(PyDIPdeps "${DIP_PYTHON_SRC}")
file(GLOB DIP_PYTHON_PY "${CMAKE_CURRENT_LIST_DIR}/src/*.py")
list(APPEND PyDIPdeps "${DIP_PYTHON_PY}")
# Insert DIP.dll location into __init__.py
set(DIP_PYDIP_RELATIVE_LOCATION OFF CACHE BOOL "PyDIP uses relative path to find the DIPlib library (turn on for binary distribution, keep off for personal builds)")
if(DIP_PYDIP_RELATIVE_LOCATION)
   file(RELATIVE_PATH diplib_dir "${PYDIP_INSTALL_PATH}/PyDIP" "${CMAKE_INSTALL_PREFIX}/${LIBRARY_DESTINATION}")
   configure_file("${CMAKE_CURRENT_LIST_DIR}/src/__init__.py.distrib" "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY) # uses ${diplib_dir}
   if(APPLE)
      set(rpath "@loader_path/${diplib_dir}")
   else()
      set(rpath "$ORIGIN/${diplib_dir}")
   endif()
else()
   configure_file("${CMAKE_CURRENT_LIST_DIR}/src/__init__.py.in" "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY) # uses ${CMAKE_INSTALL_PREFIX}
   set(rpath "${CMAKE_INSTALL_PREFIX}/${LIBRARY_DESTINATION}")
endif()
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

# Install PyDIP_bin
install(TARGETS PyDIP_bin DESTINATION "${PYDIP_INSTALL_PATH}/PyDIP")

# Install .py files
install(FILES ${DIP_PYTHON_PY} DESTINATION "${PYDIP_INSTALL_PATH}/PyDIP")

add_custom_target(PyDIP DEPENDS PyDIP_bin)

# Repeat the above for PyDIPviewer
if(DIP_BUILD_DIPVIEWER)

   # Find sources
   file(GLOB DIP_PYTHON_VIEWER_SRC "${CMAKE_CURRENT_LIST_DIR}/src/viewer/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/viewer/*.h")
   update_deps_file("PyDIPviewer_sources" "${DIP_PYTHON_VIEWER_SRC}")

   # Build PyDIPviewer
   pybind11_add_module(PyDIPviewer ${DIP_PYTHON_VIEWER_SRC})
   target_link_libraries(PyDIPviewer PRIVATE DIP DIPviewer)
   set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "${rpath}")

   # C++ version, etc.
   set_target_properties(PyDIPviewer PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
   set_target_properties(PyDIPviewer PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)

   # Install PyDIPviewer
   install(TARGETS PyDIPviewer DESTINATION ${PYDIP_INSTALL_PATH}/PyDIP)
   add_dependencies(PyDIP PyDIPviewer)

endif()

# Repeat the above for PyDIPjavaio
if(DIP_BUILD_JAVAIO)

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

   # Install PyDIPjavaio
   install(TARGETS PyDIPjavaio DESTINATION ${PYDIP_INSTALL_PATH}/PyDIP)
   add_dependencies(PyDIP PyDIPjavaio)

endif()
