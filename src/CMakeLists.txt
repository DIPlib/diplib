file(GLOB DIPLIB_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*/*.h"
                         "${CMAKE_CURRENT_LIST_DIR}/*/*.cpp"
                         "${PROJECT_SOURCE_DIR}/include/*.h"
                         "${PROJECT_SOURCE_DIR}/include/diplib/*.h"
                         "${PROJECT_SOURCE_DIR}/include/diplib/library/*.h"
                         "${PROJECT_SOURCE_DIR}/include/diplib/private/*.h")
update_deps_file("DIPlib_sources" "${DIPLIB_SOURCES}")
add_library(DIP "${CMAKE_CURRENT_LIST_DIR}/support/matrix.cpp" ${DIPLIB_SOURCES}) # The matrix.cpp file takes annoyingly long to compile, let's compile it first!
target_include_directories(DIP PUBLIC
                           "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                           $<INSTALL_INTERFACE:include>)
if(DIP_SHARED_LIBRARY)
   target_compile_definitions(DIP PRIVATE DIP__BUILD_SHARED)
else()
   target_compile_definitions(DIP PUBLIC DIP__IS_STATIC)
endif()

# Debugging mode
target_compile_definitions(DIP PRIVATE DIP_DEBUG_VERSION=$<CONFIG:Debug>)

# Multithreading
if(DIP_ENABLE_MULTITHREADING)
   target_compile_options(DIP PRIVATE ${OpenMP_CXX_FLAGS})
   if(OpenMP_CXX_LIB_NAMES)
      target_link_libraries(DIP PRIVATE ${OpenMP_CXX_LIB_NAMES}) # OpenMP_CXX_LIBRARIES is empty?!
   else()
      target_link_libraries(DIP PRIVATE ${OpenMP_CXX_FLAGS}) # This is the case for CMake <= 3.8
   endif()
endif()

# Do we have __PRETTY_FUNCTION__ ?
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { char const* name = __PRETTY_FUNCTION__; return 0; }" HAS_PRETTY_FUNCTION)
if(HAS_PRETTY_FUNCTION)
   target_compile_definitions(DIP PUBLIC DIP__HAS_PRETTY_FUNCTION)
endif()

# Control exceptions
set(DIP_ENABLE_STACK_TRACE ON CACHE BOOL "Stack trace information is added to exceptions (see the documentation for DIP_CATCH)")
if(DIP_ENABLE_STACK_TRACE)
   target_compile_definitions(DIP PUBLIC DIP__EXCEPTIONS_RECORD_STACK_TRACE)
endif()
set(DIP_ENABLE_ASSERT OFF CACHE BOOL "Enable assertions (see the documentation for DIP_ASSERT)")
if(DIP_ENABLE_ASSERT)
   target_compile_definitions(DIP PUBLIC DIP__ENABLE_ASSERT)
endif()

# Enable testing
# It's possible to disable DocTest using `DOCTEST_CONFIG_DISABLE`, but that would also disable any tests in code
# that uses DIPlib, hence we define a variable here that removes all of DocTest from the DIPlib sources.
set(DIP_ENABLE_DOCTEST ON CACHE BOOL "Turn off to not include doctest.h in the library headers")
if(DIP_ENABLE_DOCTEST)
   target_include_directories(DIP PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/doctest")
   target_compile_definitions(DIP PRIVATE
                              DIP__ENABLE_DOCTEST
                              DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
                              DIP__EXAMPLES_DIR="${PROJECT_SOURCE_DIR}/examples")
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(DIP PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
   endif()
endif()

# UFT-8 or plain old ASCII?
set(DIP_ENABLE_UNICODE ON CACHE BOOL "Enable UTF-8 encoded strings, if disabled, some text output will look more 'primitive'")
if(DIP_ENABLE_UNICODE)
   target_compile_definitions(DIP PRIVATE DIP__ENABLE_UNICODE)
endif()

# Force 128-bit PRNG?
set(DIP_ALWAYS_128_PRNG OFF CACHE BOOL "Use the 128-bit PRNG code even if 128-bit integers are not natively supported by your platform")
if(DIP_ALWAYS_128_PRNG)
   target_compile_definitions(DIP PUBLIC DIP__ALWAYS_128_PRNG)
   set(HAS_128_INT TRUE)
else()
   check_cxx_source_compiles("int main() { int v = __SIZEOF_INT128__; return 0; }" HAS_128_INT)
endif()
set(HAS_128_INT ${HAS_128_INT} PARENT_SCOPE)

# Eigen
target_include_directories(DIP PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/eigen3")
target_compile_definitions(DIP PRIVATE
                           EIGEN_MPL2_ONLY # This makes sure we only use parts of the Eigen library that use the MPL2 license or more permissive ones.
                           EIGEN_DONT_PARALLELIZE) # This to prevent Eigen algorithms trying to run in parallel -- we parallelize at a larger scale.

# libics
set(DIP_ENABLE_ICS ON CACHE BOOL "Enable ICS file support")
if(DIP_ENABLE_ICS)
   add_subdirectory("${PROJECT_SOURCE_DIR}/dependencies/libics" "${PROJECT_BINARY_DIR}/libics" EXCLUDE_FROM_ALL)
   target_link_libraries(DIP PRIVATE libics_static)
   target_compile_definitions(DIP PRIVATE DIP__HAS_ICS)
endif()

# libtiff
find_package(TIFF)
if(TIFF_FOUND)
   set(DIP_ENABLE_TIFF ON CACHE BOOL "Enable TIFF file support")
endif()
if(DIP_ENABLE_TIFF)
   target_link_libraries(DIP PRIVATE TIFF::TIFF)
   target_compile_definitions(DIP PRIVATE DIP__HAS_TIFF)
endif()

# Switch to link against the FFTW library for Fourier transforms
find_package(FFTW3)
if(FFTW3_FOUND)
   set(DIP_ENABLE_FFTW OFF CACHE BOOL "Enable linking against the FFTW library (GPL license) for Fourier transforms")
endif()
if(DIP_ENABLE_FFTW)
   target_include_directories(DIP PRIVATE ${FFTW3_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/dependencies/fftw")
   target_link_libraries(DIP PRIVATE ${FFTW3_LIBRARIES})
   target_compile_definitions(DIP PRIVATE DIP__HAS_FFTW)
endif()

# Install
install(TARGETS DIP DESTINATION lib EXPORT DIPlibTargets)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/" DESTINATION include)
# To examine symbol table of the shared library on MacOSX: nm -g -p -U libDIP.dylib

# Uncomment next line to see compile times for each source file (make without -j).
#set_target_properties(DIP PROPERTIES RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")


# DIPlib unit tests
if(DIP_ENABLE_DOCTEST)
   add_executable(unit_tests EXCLUDE_FROM_ALL "${CMAKE_CURRENT_LIST_DIR}/library/unit_tests.cpp")
   target_include_directories(unit_tests PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/doctest")
   target_link_libraries(unit_tests PRIVATE DIP)
   target_compile_definitions(unit_tests PRIVATE
                              DIP__IMPLEMENT_UNIT_TESTS
                              DIP__ENABLE_DOCTEST
                              DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES)
   if(DIP_SHARED_LIBRARY)
      target_compile_definitions(unit_tests PRIVATE DIP__DOCTEST_IN_SHARED_LIB)
      if(APPLE)
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "@loader_path")
      else()
         set_target_properties(unit_tests PROPERTIES INSTALL_RPATH "$ORIGIN")
      endif()
   else()
      include("${PROJECT_SOURCE_DIR}/dependencies/doctest/doctest_force_link_static_lib_in_target.cmake")
      doctest_force_link_static_lib_in_target(unit_tests DIP) # This pulls in all object files from the static DIP library
   endif()
   add_custom_target(check COMMAND unit_tests)
   add_dependencies(check unit_tests)
   add_custom_target(check_memory COMMAND valgrind ./unit_tests)
   add_dependencies(check_memory unit_tests)
endif()

# DIPlib documentation
set(DOXYGEN_SKIP_DOT ON)
find_package(Doxygen)
if(DOXYGEN_FOUND)
   configure_file("${CMAKE_CURRENT_LIST_DIR}/documentation/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
   add_custom_target(
         apidoc
         COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}"
         COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
         COMMAND ${CMAKE_COMMAND} -E echo "Fixing header names and other things..."
         COMMAND "${PROJECT_SOURCE_DIR}/tools/fix_doc_includes.sh" "${CMAKE_INSTALL_PREFIX}/${DOCUMENTATION_OUTPUT}/html"
         WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
   )
endif(DOXYGEN_FOUND)
