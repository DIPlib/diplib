file(GLOB DIP_EXAMPLES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
set(EXAMPLES_TARGETS "")
set(EXAMPLES_REQUIRE_VIEWER
    display
    fileviewer
    viewer
    test_filters
)
foreach(file ${DIP_EXAMPLES})
   get_filename_component(target ${file} NAME_WE)
   list(FIND EXAMPLES_REQUIRE_VIEWER ${target} RES)
   if((${RES} EQUAL -1) OR DIP_BUILD_DIPVIEWER) # Don't build the examples that need DIPviewer if we haven't built DIPviewer.
      add_executable(${target} EXCLUDE_FROM_ALL ${file})
      target_compile_definitions(${target} PRIVATE DIP__EXAMPLES_DIR="${CMAKE_CURRENT_LIST_DIR}")
      target_link_libraries(${target} DIP)
      if(NOT (${RES} EQUAL -1))
         target_link_libraries(${target} DIPviewer)
      endif()
      set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_RPATH 0)
      set(EXAMPLES_TARGETS ${EXAMPLES_TARGETS} ${target})
   endif()
endforeach()
if(DIP_BUILD_DIPVIEWER)
   target_link_libraries(viewer Threads::Threads)
endif()

add_custom_target(examples DEPENDS ${EXAMPLES_TARGETS})
