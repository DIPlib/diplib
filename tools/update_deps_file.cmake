# Utility for automatically updating targets when a file is added to the source directories
# from: https://stackoverflow.com/a/39971448/7328782, but modified
# Creates a file called ${name} with the list of dependencies in it.
# The file is updated when the list of dependencies changes.
# If the file is updated, cmake will automatically reload.
function(update_deps_file name deps)
   # Normalize the list so it's the same on every machine
   list(REMOVE_DUPLICATES deps)
   foreach(dep IN LISTS deps)
      file(RELATIVE_PATH rel_dep "${CMAKE_CURRENT_SOURCE_DIR}" ${dep})
      list(APPEND rel_deps ${rel_dep})
   endforeach(dep)
   list(SORT rel_deps)
   # Split the list into lines, and add some CMake-valid syntax so it's ignored
   string(REPLACE ";" "\n" new_deps "${rel_deps}")
   set(new_deps "# Automatically generated, don't edit!\nset(${name}_bogus\n${new_deps}\n)\n")
   # Compare with the old file
   set(old_deps "")
   if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake")
      file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" old_deps)
   endif()
   if(NOT old_deps STREQUAL new_deps)
      file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake" "${new_deps}")
   endif()
   # Include the file so it's tracked as a generation dependency (we don't need the content).
   include("${CMAKE_CURRENT_SOURCE_DIR}/${name}.cmake")
endfunction(update_deps_file)
