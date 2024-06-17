# (c)2019-2022, Wouter Caarls
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


file(GLOB DIP_JAVAIO_SRC             "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
                                     "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
                                     "${PROJECT_SOURCE_DIR}/include/diplib/javaio.h"
                                     "${PROJECT_SOURCE_DIR}/include/diplib/javaio/*.h")
file(GLOB DIP_JAVAIO_JAVA            "${CMAKE_CURRENT_LIST_DIR}/java/*.java")
file(GLOB DIP_JAVAIO_BIOFORMATS_JAVA "${CMAKE_CURRENT_LIST_DIR}/java/bioformats/*.java")

update_deps_file("DIPjavaio_sources" "${DIP_JAVAIO_SRC};${DIP_JAVAIO_JAVA};${DIP_JAVAIO_BIOFORMATS_JAVA};${CMAKE_CURRENT_LIST_DIR}/Manifest.txt.in")


### CPP part

add_library(DIPjavaio ${DIP_JAVAIO_SRC})

target_link_libraries(DIPjavaio PRIVATE DIP ${JAVA_JVM_LIBRARY})
target_include_directories(DIPjavaio PRIVATE ${JNI_INCLUDE_DIRS})
foreach (lib ${JNI_LIBRARIES})
   get_filename_component(path ${lib} DIRECTORY)
   set(JNI_LIBRARY_PATHS ${JNI_LIBRARY_PATHS};${path})
endforeach()
set_target_properties(DIPjavaio PROPERTIES INSTALL_RPATH "${JNI_LIBRARY_PATHS}")

target_compile_definitions(DIPjavaio PUBLIC DIP_CONFIG_HAS_DIPJAVAIO)

if(DIP_SHARED_LIBRARY)
   target_compile_definitions(DIPjavaio PRIVATE DIP_CONFIG_DIPJAVAIO_BUILD_SHARED)
else()
   target_compile_definitions(DIPjavaio PUBLIC DIP_CONFIG_DIPJAVAIO_IS_STATIC)
endif()

# rpath
set_target_properties(DIPjavaio PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)
if(APPLE)
   set_target_properties(DIPjavaio PROPERTIES INSTALL_RPATH "@loader_path")
else()
   set_target_properties(DIPjavaio PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

install(TARGETS DIPjavaio EXPORT DIPlibTargets ${DIP_DESTINATIONS})
install(FILES "${PROJECT_SOURCE_DIR}/include/diplib/javaio.h" DESTINATION include/diplib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/diplib/javaio" DESTINATION include/diplib)


### Java part

# Bio-Formats interface
set(DIP_JAVAIO_JAVA ${DIP_JAVAIO_JAVA} ${DIP_JAVAIO_BIOFORMATS_JAVA})
set(DIP_JAVAIO_CLASSPATH ${DIP_JAVAIO_CLASSPATH} "${CMAKE_CURRENT_LIST_DIR}/java/bioformats/bio-formats-stripped.jar")

configure_file("${CMAKE_CURRENT_LIST_DIR}/Manifest.txt.in" "${CMAKE_CURRENT_BINARY_DIR}/Manifest.txt" @ONLY)

add_jar(DIPjavaio_java "${DIP_JAVAIO_JAVA}"
        INCLUDE_JARS "${DIP_JAVAIO_CLASSPATH}"
        OUTPUT_NAME DIPjavaio
        MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/Manifest.txt")
install_jar(DIPjavaio_java ${LIBRARY_DESTINATION})

add_dependencies(DIPjavaio DIPjavaio_java)
