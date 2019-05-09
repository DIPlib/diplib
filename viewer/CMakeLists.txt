file(GLOB DIP_VIEWER_SRC
     "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_LIST_DIR}/src/*.h"
     "${CMAKE_CURRENT_LIST_DIR}/src/manager/*.cpp"
     "${CMAKE_CURRENT_LIST_DIR}/src/manager/*.h"
     "${PROJECT_SOURCE_DIR}/include/dipviewer.h"
     "${PROJECT_SOURCE_DIR}/include/diplib/viewer/*.h")
update_deps_file("DIPviewer_sources" "${DIP_VIEWER_SRC}")
add_library(DIPviewer ${DIP_VIEWER_SRC})
if(DIP_SHARED_LIBRARY)
   target_compile_definitions(DIPviewer PRIVATE DIP__VIEWER_BUILD_SHARED)
else()
   target_compile_definitions(DIPviewer PUBLIC DIP__VIEWER_IS_STATIC)
endif()
target_link_libraries(DIPviewer PRIVATE DIP Threads::Threads)
if(FREEGLUT_FOUND)
   target_compile_definitions(DIPviewer PUBLIC DIP__HAS_FREEGLUT)
   target_link_libraries(DIPviewer PRIVATE FREEGLUT::FREEGLUT)
endif()
if(GLFW_FOUND)
   target_compile_definitions(DIPviewer PUBLIC DIP__HAS_GLFW)
   target_link_libraries(DIPviewer PRIVATE GLFW::GLFW)
endif()
if(WIN32)
   target_link_libraries(DIPviewer PRIVATE GLEW::GLEW)
endif()
if(TARGET OpenGL::GL)
   target_link_libraries(DIPviewer PRIVATE OpenGL::GL)
else() # For older CMakes:
   target_link_libraries(DIPviewer PRIVATE ${OPENGL_LIBRARIES})
   target_include_directories(DIPviewer PRIVATE ${OPENGL_INCLUDE_DIR})
endif()
if(APPLE)
   set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "@loader_path")
   target_compile_definitions(DIPviewer PRIVATE GL_SILENCE_DEPRECATION) # MacOS 10.14 deprecates OpenGL
else()
   set_target_properties(DIPviewer PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()
install(TARGETS DIPviewer DESTINATION lib EXPORT DIPlibTargets)
install(FILES "${PROJECT_SOURCE_DIR}/include/dipviewer.h" DESTINATION include)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/diplib/viewer" DESTINATION include/diplib)
