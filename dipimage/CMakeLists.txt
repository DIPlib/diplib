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
      set_target_properties(MEX_${target} PROPERTIES INSTALL_RPATH "${LIB_REL_PATH}")
      target_compile_definitions(MEX_${target} PRIVATE ${MEX_API_MACRO})
      if(DIP_ENABLE_UNICODE)
         target_compile_definitions(MEX_${target} PRIVATE DIP__ENABLE_UNICODE)
      endif()
      set(TARGETS ${TARGETS} MEX_${target})
   endforeach()
   set(${OUTPUT_VAR} ${TARGETS} PARENT_SCOPE)
endfunction(compile_mex_files)

# Install M-files
file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}")
file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/private/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/private")
file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/@dip_image/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image")
file(GLOB FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/alias/*.m")
install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}/alias")

# Compile and install MEX-files in dipimage/@dip_image/private
file(GLOB DIP_IMAGE_PRIVATE "${CMAKE_CURRENT_LIST_DIR}/@dip_image/private/*.cpp")
set(DIPIMAGE_DEPS "${DIP_IMAGE_PRIVATE}")
compile_mex_files("${DIP_IMAGE_PRIVATE}" ../../../../lib/ DIP_IMAGE_PRIVATE_TARGETS)
install(TARGETS ${DIP_IMAGE_PRIVATE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image/private"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image/private")
# By specifying LIBRARY and RUNTIME, but not ARCHIVE, we manage to install
# the .so/.dylib (LIBRARY) and the .dll (RUNTIME), but not the .lib (ARCHIVE)
# that comes with the .dll but we don't need.

# Compile and install MEX-files in dipimage/@dip_image (this set is currently empty)
#file(GLOB DIP_IMAGE "${CMAKE_CURRENT_LIST_DIR}/@dip_image/*.cpp")
#list(APPEND DIPIMAGE_DEPS "${DIP_IMAGE}")
#compile_mex_files("${DIP_IMAGE}" ../../../lib/ DIP_IMAGE_TARGETS)
#install(TARGETS ${DIP_IMAGE_TARGETS}
#        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image"
#        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/@dip_image")

# Compile and install MEX-files in dipimage/private
file(GLOB DIPIMAGE_PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp")
list(APPEND DIPIMAGE_DEPS "${DIPIMAGE_PRIVATE}")
compile_mex_files("${DIPIMAGE_PRIVATE}" ../../../lib/ DIPIMAGE_PRIVATE_TARGETS)
install(TARGETS ${DIPIMAGE_PRIVATE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}/private"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}/private")

# Compile and install MEX-files in dipimage/
file(GLOB DIPIMAGE "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
list(APPEND DIPIMAGE_DEPS "${DIPIMAGE}")
compile_mex_files("${DIPIMAGE}" ../../lib/ DIPIMAGE_TARGETS)
install(TARGETS ${DIPIMAGE_TARGETS}
        LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}"
        RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}")

# Keep a file with all dependencies
update_deps_file("DIPimage_sources" "${DIPIMAGE_DEPS}")

# The DIPviewer tool in MATLAB
if(DIP_BUILD_DIPVIEWER)

   # The tool requires DIPviewer Java bindings
   find_package(Java)
   if (JAVA_FOUND)
      set(DIP_BUILD_DIPVIEWER_JAVA ON CACHE BOOL "Build the viewer Java bindings (required for DIPviewer in MATLAB)")
   endif()
   if(DIP_BUILD_DIPVIEWER_JAVA)

      # Install M-files in dipviewer/dipimage
      file(GLOB FILE_LIST "${PROJECT_SOURCE_DIR}/viewer/dipimage/*.m")
      install(FILES ${FILE_LIST} DESTINATION "${DIPIMAGE_INSTALL_PATH}")

      # Compile and install MEX-files in dipviewer/dipimage/
      file(GLOB DIPVIEWER_DIPIMAGE "${PROJECT_SOURCE_DIR}/viewer/dipimage/*.cpp")
      update_deps_file("DIPimageViewer_sources" "${DIPVIEWER_DIPIMAGE}")
      compile_mex_files("${DIPVIEWER_DIPIMAGE}" ../../lib/ DIPVIEWER_DIPIMAGE_TARGETS)
      target_link_libraries(${DIPVIEWER_DIPIMAGE_TARGETS} DIPviewer) # TODO: this only works because there's a single target here. Will need to write a loop if we add targets.
      install(TARGETS ${DIPVIEWER_DIPIMAGE_TARGETS}
              LIBRARY DESTINATION "${DIPIMAGE_INSTALL_PATH}"
              RUNTIME DESTINATION "${DIPIMAGE_INSTALL_PATH}")

      # DIPviewer Java binding
      include(UseJava)
      set(CMAKE_JAVA_COMPILE_FLAGS -target 1.7 -source 1.7) # MATLAB R2013b--R2017a use Java SDK 1.7, R2017b-- use SDK 1.8.
      add_jar(ViewerJava "${PROJECT_SOURCE_DIR}/viewer/java/Viewer.java"
              INCLUDE_JARS "${Matlab_ROOT_DIR}/java/jarext/svnkit/jna.jar"
                           "${Matlab_ROOT_DIR}/java/jarext/jogl-all.jar"
                           "${Matlab_ROOT_DIR}/java/jarext/gluegen-rt.jar"
              OUTPUT_NAME Viewer)
      install_jar(ViewerJava ${DIPIMAGE_INSTALL_PATH})

   endif()
endif()

# Tell DIPlib library code that it might link against MATLAB (needed to fix a bug with MATLAB integration)
target_compile_definitions(DIP PRIVATE DIP__DUILDING_DIPIMAGE)

# Create the Contents.m file
install(CODE "execute_process(COMMAND ${Matlab_MAIN_PROGRAM} -nojvm -nodisplay -r \"makecontents '${CMAKE_INSTALL_PREFIX}/${DIPIMAGE_INSTALL_PATH}'; exit\" WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/tools\")")

# DIPimage user manual
find_program(PANDOC_EXECUTABLE pandoc)
find_program(PANDOC_CROSSREF_EXECUTABLE pandoc-crossref)
if(PANDOC_EXECUTABLE AND PANDOC_CROSSREF_EXECUTABLE)
   execute_process(
         COMMAND ${PANDOC_EXECUTABLE} --version
         OUTPUT_VARIABLE pandoc_version
         ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
   if(pandoc_version MATCHES "^pandoc ([0-9]*)\\.([0-9]*)")
      set(PANDOC_MAJOR "${CMAKE_MATCH_1}")
      set(PANDOC_MINOR "${CMAKE_MATCH_2}")
      message(STATUS "Found pandoc version ${PANDOC_MAJOR}.${PANDOC_MINOR}")
   else()
      message(STATUS "Found pandoc but could not determine version")
   endif()
   unset(pandoc_version)

   if(PANDOC_MAJOR LESS "2")
      # pandoc 1 has a --smart option, which was removed in pandoc 2.
      set(PANDOC_SMART_OPTION "--smart")
   else()
      # pandoc 2 defines +smart extension to markdown instead of --smart.
      set(PANDOC_SMART_OPTION "--from=markdown+smart")
   endif()

   set(DUM_PDF_NAME "${CMAKE_CURRENT_BINARY_DIR}/DIPimageUserManual.pdf")
   add_custom_command(
         OUTPUT ${DUM_PDF_NAME}
         COMMAND ${PANDOC_EXECUTABLE} *.md --output=${DUM_PDF_NAME}
         --template=template.tex ${PANDOC_SMART_OPTION}
         --number-sections --toc --toc-depth=2
         --filter=pandoc-crossref
         --highlight-style=kate
         WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/user_manual"
         DEPENDS "${CMAKE_CURRENT_LIST_DIR}/user_manual/*.md" "${CMAKE_CURRENT_LIST_DIR}/user_manual/template.tex"
   )
   # Code hightlight style alternative: tango -- light background color, but colors are poorly chosen
   add_custom_target(dum ALL DEPENDS ${DUM_PDF_NAME})
   install(FILES ${DUM_PDF_NAME} DESTINATION ${DOCUMENTATION_OUTPUT})
endif()
