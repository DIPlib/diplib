file(GLOB DIP_JAVAIO_SRC             "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
                                     "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
                                     "${PROJECT_SOURCE_DIR}/include/diplib/javaio.h"
                                     "${PROJECT_SOURCE_DIR}/include/diplib/javaio/*.h")
file(GLOB DIP_JAVAIO_JAVA            "${CMAKE_CURRENT_LIST_DIR}/java/*.java")
file(GLOB DIP_JAVAIO_BIOFORMATS_JAVA "${CMAKE_CURRENT_LIST_DIR}/java/bioformats/*.java")

update_deps_file("DIPjavaio_sources" "${DIP_JAVAIO_SRC};${DIP_JAVAIO_JAVA};${DIP_JAVAIO_BIOFORMATS_JAVA};${CMAKE_CURRENT_LIST_DIR}/Manifest.txt.in")


### CPP part

add_library(DIPjavaio ${DIP_JAVAIO_SRC})

target_link_libraries(DIPjavaio PRIVATE DIP ${JNI_LIBRARIES})
target_include_directories(DIPjavaio PRIVATE ${JNI_INCLUDE_DIRS})
foreach (lib ${JNI_LIBRARIES})
   get_filename_component(path ${lib} DIRECTORY)
   set(JNI_LIBRARY_PATHS ${JNI_LIBRARY_PATHS};${path})
endforeach()
set_target_properties(DIPjavaio PROPERTIES INSTALL_RPATH "${JNI_LIBRARY_PATHS}")

target_compile_definitions(DIPjavaio PUBLIC DIP__HAS_JAVAIO)

if(DIP_SHARED_LIBRARY)
   target_compile_definitions(DIPjavaio PRIVATE DIP__JAVAIO_BUILD_SHARED)
else()
   target_compile_definitions(DIPjavaio PUBLIC DIP__JAVAIO_IS_STATIC)
endif()

install(TARGETS DIPjavaio DESTINATION lib EXPORT DIPlibTargets)
install(FILES "${PROJECT_SOURCE_DIR}/include/diplib/javaio.h" DESTINATION include/diplib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/diplib/javaio" DESTINATION include/diplib)


### Java part

if (BIOFORMATS_JAR)
   set(DIP_JAVAIO_JAVA ${DIP_JAVAIO_JAVA} ${DIP_JAVAIO_BIOFORMATS_JAVA})
   set(DIP_JAVAIO_CLASSPATH ${DIP_JAVAIO_CLASSPATH} ${BIOFORMATS_JAR})
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/Manifest.txt.in" "${CMAKE_CURRENT_BINARY_DIR}/Manifest.txt" @ONLY)

add_jar(DIPjavaio_java "${DIP_JAVAIO_JAVA}"
        INCLUDE_JARS "${DIP_JAVAIO_CLASSPATH}"
        OUTPUT_NAME DIPjavaio
        MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/Manifest.txt")
install_jar(DIPjavaio_java lib)

add_dependencies(DIPjavaio DIPjavaio_java)
