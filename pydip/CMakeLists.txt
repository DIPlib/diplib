add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/pybind11" "${PROJECT_BINARY_DIR}/pybind11" EXCLUDE_FROM_ALL)

# Find sources
file(GLOB DIP_PYTHON_SRC "${CMAKE_CURRENT_LIST_DIR}/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/*.h")
set(PyDIPdeps "${DIP_PYTHON_SRC}")
file(GLOB DIP_PYTHON_PY "${CMAKE_CURRENT_LIST_DIR}/*.py")
list(APPEND PyDIPdeps "${DIP_PYTHON_PY}")
# Insert DIP.dll location into __init__.py
configure_file("${CMAKE_CURRENT_LIST_DIR}/__init__.py.in" "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" @ONLY)
list(APPEND DIP_PYTHON_PY "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")
list(APPEND PyDIPdeps "${CMAKE_CURRENT_LIST_DIR}/__init__.py.in")
update_deps_file("PyDIP_sources" "${PyDIPdeps}")

# Build PyDIP_bin
pybind11_add_module(PyDIP_bin ${DIP_PYTHON_SRC})
target_link_libraries(PyDIP_bin PRIVATE DIP)
file(RELATIVE_PATH diplib_dir "${PYDIP_INSTALL_PATH}/PyDIP" "${CMAKE_INSTALL_PREFIX}/lib")
if(APPLE)
   set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "@loader_path/${diplib_dir}")
else()
   set_target_properties(PyDIP_bin PROPERTIES INSTALL_RPATH "$ORIGIN/${diplib_dir}")
endif()

# Install PyDIP_bin
install(TARGETS PyDIP_bin DESTINATION "${PYDIP_INSTALL_PATH}/PyDIP")

# Install .py files
install(FILES ${DIP_PYTHON_PY} DESTINATION "${PYDIP_INSTALL_PATH}/PyDIP")

add_custom_target(PyDIP DEPENDS PyDIP_bin)

# Repeat the above for PyDIPviewer
if(DIP_BUILD_DIPVIEWER)
   # Find sources
   file(GLOB DIP_PYTHON_VIEWER_SRC "${CMAKE_CURRENT_LIST_DIR}/viewer/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/viewer/*.h")
   update_deps_file("PyDIPviewer_sources" "${DIP_PYTHON_VIEWER_SRC}")
   # Build PyDIPviewer
   pybind11_add_module(PyDIPviewer ${DIP_PYTHON_VIEWER_SRC})
   target_link_libraries(PyDIPviewer PRIVATE DIP DIPviewer)
   if(APPLE)
      set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "@loader_path/${diplib_dir}")
   else()
      set_target_properties(PyDIPviewer PROPERTIES INSTALL_RPATH "$ORIGIN/${diplib_dir}")
   endif()
   # Install PyDIPviewer
   install(TARGETS PyDIPviewer DESTINATION ${PYDIP_INSTALL_PATH}/PyDIP)
   add_dependencies(PyDIP PyDIPviewer)
endif()
