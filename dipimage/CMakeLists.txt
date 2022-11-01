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


# A function that compiles a series of MEX-files
function(compile_mex_files FILE_LIST LIB_REL_PATH OUTPUT_VAR)
   if(APPLE)
      set(LIB_REL_PATH "@loader_path/${LIB_REL_PATH}")
   else()
      set(LIB_REL_PATH "$ORIGIN/${LIB_REL_PATH}") # doesn't do anything on Windows, so this should be OK
   endif()
   set(TARGETS "")
   foreach(file ${FILE_LIST})
      get_filename_component(target ${file} NAME_WE)
      matlab_add_mex(NAME MEX_${target} SRC ${file} ${MEX_VERSION_FILE} OUTPUT_NAME ${target} LINK_TO DIP)
      set_target_properties(MEX_${target} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED On CXX_EXTENSIONS Off)
      set_target_properties(MEX_${target} PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN 1)
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      target_compile_definitions(MEX_${target} PRIVATE ${MEX_API_MACRO})
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP_CONFIG_ENABLE_UNICODE)
      endif()
      set(TARGETS ${TARGETS} MEX_${target})
   endforeach()
   set(${OUTPUT_VAR} ${TARGETS} PARENT_SCOPE)
endfunction(compile_mex_files)

# Install M-files
file(GLOB DIPIMAGE_MFILES "${CMAKE_CURRENT_LIST_DIR}/*.m")
install(FILES ${DIPIMAGE_MFILES} DESTINATION "${DIPIMAGE_INSTALL_PATH}")

file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/private/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/private")

file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/@dip_image/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image")

file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/alias/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/alias")

# Keep a file with all M-files to be written to the Contents.m file
update_deps_file("DIPimage_Mfiles" "${DIPIMAGE_MFILES}")

# Compile and install MEX-files in dipimage/@dip_image/private
file(GLOB DIP_IMAGE_PRIVATE "${CMAKE_CURRENT_LIST_DIR}/@dip_image/private/*.cpp")
set(DIPIMAGE_DEPS "${DIP_IMAGE_PRIVATE}")
compile_mex_files("${DIP_IMAGE_PRIVATE}" ../../../../${LIBRARY_DESTINATION}/ DIP_IMAGE_PRIVATE_TARGETS)
install(TARGETS ${DIP_IMAGE_PRIVATE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image/private"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image/private")
# By specifying LIBRARY and RUNTIME, but not ARCHIVE, we manage to install
# the .so/.dylib (LIBRARY) and the .dll (RUNTIME), but not the .lib (ARCHIVE)
# that comes with the .dll but we don't need.

# Compile and install MEX-files in dipimage/@dip_image (this set is currently empty)
#file(GLOB DIP_IMAGE "${CMAKE_CURRENT_LIST_DIR}/@dip_image/*.cpp")
#list(APPEND DIPIMAGE_DEPS "${DIP_IMAGE}")
#compile_mex_files("${DIP_IMAGE}" ../../../${LIBRARY_DESTINATION}/ DIP_IMAGE_TARGETS)
#install(TARGETS ${DIP_IMAGE_TARGETS}
#        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image"
#        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image")

# Compile and install MEX-files in dipimage/private
file(GLOB DIPIMAGE_PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp")
list(APPEND DIPIMAGE_DEPS "${DIPIMAGE_PRIVATE}")
compile_mex_files("${DIPIMAGE_PRIVATE}" ../../../${LIBRARY_DESTINATION}/ DIPIMAGE_PRIVATE_TARGETS)
install(TARGETS ${DIPIMAGE_PRIVATE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/private"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/private")

# Compile and install MEX-files in dipimage/
file(GLOB DIPIMAGE "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
list(APPEND DIPIMAGE_DEPS "${DIPIMAGE}")
compile_mex_files("${DIPIMAGE}" ../../${LIBRARY_DESTINATION}/ DIPIMAGE_TARGETS)
install(TARGETS ${DIPIMAGE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}")

# Keep a file with all dependencies
update_deps_file("DIPimage_sources" "${DIPIMAGE_DEPS}")

# The DIPviewer tool in MATLAB
if(DIP_BUILD_DIPVIEWER)

   # The tool requires DIPviewer Java bindings
   if(JAVA_FOUND)
      set(DIP_BUILD_DIPVIEWER_JAVA ON CACHE BOOL "Build the viewer Java bindings (required for DIPviewer in MATLAB)")
   endif()
   if(DIP_BUILD_DIPVIEWER_JAVA)

      # Install M-files in dipviewer/dipimage
      file(GLOB DIPVIEWER_MFILES "${PROJECT_SOURCE_DIR}/viewer/dipimage/*.m")
      install(FILES ${DIPVIEWER_MFILES} DESTINATION "${DIPIMAGE_INSTALL_PATH}")

      # Keep a file with all M-files to be written to the Contents.m file
      update_deps_file("DIPimageViewer_Mfiles" "${DIPVIEWER_MFILES}")

      # Compile and install MEX-files in dipviewer/dipimage/
      file(GLOB DIPVIEWER_DIPIMAGE "${PROJECT_SOURCE_DIR}/viewer/dipimage/*.cpp")
      update_deps_file("DIPimageViewer_sources" "${DIPVIEWER_DIPIMAGE}")
      compile_mex_files("${DIPVIEWER_DIPIMAGE}" ../../${LIBRARY_DESTINATION}/ DIPVIEWER_DIPIMAGE_TARGETS)
      target_link_libraries(${DIPVIEWER_DIPIMAGE_TARGETS} DIPviewer) # TODO: this only works because there's a single target here. Will need to write a loop if we add targets.
      install(TARGETS ${DIPVIEWER_DIPIMAGE_TARGETS}
              LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}"
              RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}")

      # DIPviewer Java binding
      include(UseJava)
      add_jar(ViewerJava "${PROJECT_SOURCE_DIR}/viewer/java/Viewer.java"
              INCLUDE_JARS "${Matlab_ROOT_DIR}/java/jarext/svnkit/jna.jar"
                           "${Matlab_ROOT_DIR}/java/jarext/jogl-all.jar"
                           "${Matlab_ROOT_DIR}/java/jarext/gluegen-rt.jar"
              OUTPUT_NAME Viewer)
      install_jar(ViewerJava ${DIPIMAGE_INSTALL_PATH})

   endif()
endif()

# Create the Contents.m file
# TODO: Starting with R2019a we can use the -batch command line option instead.
if(WIN32)
	add_custom_command(
		  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Contents.m
		  COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -wait -nosplash -r "try, makecontents('${CMAKE_CURRENT_BINARY_DIR}'), catch ME, disp(ME.message), exit(1), end, exit(0)"
		  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tools"
		  DEPENDS "${DIPIMAGE_MFILES}" "${DIPVIEWER_MFILES}")
else()
	add_custom_command(
		  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Contents.m
		  COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -nosplash -r "try, makecontents\\(\\'${CMAKE_CURRENT_BINARY_DIR}\\'\\), catch ME, disp\\(ME.message\\), exit\\(1\\), end, exit\\(0\\)"
		  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tools"
		  DEPENDS "${DIPIMAGE_MFILES}" "${DIPVIEWER_MFILES}")
endif()
add_custom_target(contents ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Contents.m)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Contents.m DESTINATION ${DIPIMAGE_INSTALL_PATH})
