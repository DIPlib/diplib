# CMake build for libtiff
# Run "cmake" to generate the build files for your platform
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

cmake_minimum_required(VERSION 3.5...3.28)

# Read version information from configure.ac.
set(LIBTIFF_MAJOR_VERSION 4)
set(LIBTIFF_MINOR_VERSION 0)
set(LIBTIFF_MICRO_VERSION 10)
set(LIBTIFF_ALPHA_VERSION "")
set(LIBTIFF_CURRENT 9)
set(LIBTIFF_REVISION 0)
set(LIBTIFF_AGE 4)

math(EXPR SO_MAJOR "${LIBTIFF_CURRENT} - ${LIBTIFF_AGE}")
set(SO_MINOR "${LIBTIFF_AGE}")
set(SO_REVISION "${LIBTIFF_REVISION}")

#message(STATUS "Building tiff version ${LIBTIFF_MAJOR_VERSION}.${LIBTIFF_MINOR_VERSION}.${LIBTIFF_MICRO_VERSION}${LIBTIFF_ALPHA_VERSION}")
#message(STATUS "libtiff library version ${SO_MAJOR}.${SO_MINOR}.${SO_REVISION}")

set(BUILD_SHARED_LIBS OFF)

# Project version
project(tiff LANGUAGES C VERSION "${LIBTIFF_MAJOR_VERSION}.${LIBTIFF_MINOR_VERSION}.${LIBTIFF_MICRO_VERSION}")
set(tiff_VERSION "${VERSION}")
set(tiff_VERSION_MAJOR "${LIBTIFF_MAJOR_VERSION}")
set(tiff_VERSION_MINOR "${LIBTIFF_MINOR_VERSION}")
set(tiff_VERSION_PATCH "${LIBTIFF_MICRO_VERSION}")

# the other tiff_VERSION_* variables are set automatically
set(tiff_VERSION_ALPHA "${LIBTIFF_ALPHA_VERSION}")
# Library version (unlike libtool's baroque scheme, WYSIWYG here)
set(SO_COMPATVERSION "${SO_MAJOR}")
set(SO_VERSION "${SO_MAJOR}.${SO_MINOR}.${SO_REVISION}")

# For autotools header compatibility
set(PACKAGE_NAME "LibTIFF Software")
set(PACKAGE_TARNAME "${PROJECT_NAME}")
set(PACKAGE_VERSION "${PROJECT_VERSION}${tiff_VERSION_ALPHA}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "tiff@lists.maptools.org")

include(GNUInstallDirs)
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckSymbolExists)
enable_testing()

macro(current_date var)
  if(UNIX)
    execute_process(COMMAND "date" +"%Y%m%d" OUTPUT_VARIABLE ${var})
  endif()
endmacro()

current_date(RELEASE_DATE)

# These are annoyingly verbose, produce false positives or don't work
# nicely with all supported compiler versions, so are disabled unless
# explicitly enabled.
option(extra-warnings "Enable extra compiler warnings" OFF)

# This will cause the compiler to fail when an error occurs.
option(fatal-warnings "Compiler warnings are errors" OFF)

# Check if the compiler supports each of the following additional
# flags, and enable them if supported.  This greatly improves the
# quality of the build by checking for a number of common problems,
# some of which are quite serious.
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(test_flags
      -Wall
      -Winline
      -W
      -Wformat-security
      -Wpointer-arith
      -Wdisabled-optimization
      -Wno-unknown-pragmas
      -Wdeclaration-after-statement
      -Wno-unused-parameter
      -Wno-unused-but-set-variable
      -fstrict-aliasing)
  if(extra-warnings)
    list(APPEND test_flags
        -Wfloat-equal
        -Wmissing-prototypes
        -Wunreachable-code)
  endif()
  if(fatal-warnings)
    list(APPEND test_flags
         -Werror)
  endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(test_flags)
  if(extra-warnings)
    list(APPEND test_flags
         /W4)
  else()
    list(APPEND test_flags
         /W3)
  endif()
  if (fatal-warnings)
    list(APPEND test_flags
         /WX)
  endif()
endif()

foreach(flag ${test_flags})
  string(REGEX REPLACE "[^A-Za-z0-9]" "_" flag_var "${flag}")
  set(test_c_flag "C_FLAG${flag_var}")
  CHECK_C_COMPILER_FLAG(${flag} "${test_c_flag}")
  if (${test_c_flag})
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
  endif (${test_c_flag})
endforeach(flag ${test_flags})

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Find libm, if available
find_library(M_LIBRARY m)

check_include_file(assert.h    HAVE_ASSERT_H)
check_include_file(dlfcn.h     HAVE_DLFCN_H)
check_include_file(fcntl.h     HAVE_FCNTL_H)
check_include_file(inttypes.h  HAVE_INTTYPES_H)
check_include_file(io.h        HAVE_IO_H)
check_include_file(search.h    HAVE_SEARCH_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(string.h    HAVE_STRING_H)
check_include_file(strings.h   HAVE_STRINGS_H)
check_include_file(sys/time.h  HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h    HAVE_UNISTD_H)

# Inspired from /usr/share/autoconf/autoconf/c.m4
foreach(inline_keyword "inline" "__inline__" "__inline")
  if(NOT DEFINED C_INLINE)
    set(CMAKE_REQUIRED_DEFINITIONS_SAVE ${CMAKE_REQUIRED_DEFINITIONS})
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
        "-Dinline=${inline_keyword}")
    check_c_source_compiles("
        typedef int foo_t;
        static inline foo_t static_foo() {return 0;}
        foo_t foo(){return 0;}
        int main(int argc, char *argv[]) {return 0;}"
      C_HAS_${inline_keyword})
    set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS_SAVE})
    if(C_HAS_${inline_keyword})
      set(C_INLINE TRUE)
      set(INLINE_KEYWORD "${inline_keyword}")
    endif()
 endif()
endforeach()
if(NOT DEFINED C_INLINE)
  set(INLINE_KEYWORD)
endif()

# off_t and size_t checks omitted; not clear they are used at all
# Are off_t and size_t checks strictly necessary?

# Check if sys/time.h and time.h allow use together
check_c_source_compiles("
#include <sys/time.h>
#include <time.h>
int main(void){return 0;}"
  TIME_WITH_SYS_TIME)

# Check if struct tm is in sys/time.h
check_c_source_compiles("
#include <sys/types.h>
#include <time.h>

int main(void){
  struct tm tm;
  int *p = &tm.tm_sec;
  return !p;
}"
  TM_IN_SYS_TIME)

# Check type sizes
# NOTE: Could be replaced with C99 <stdint.h>
check_type_size("signed int" SIZEOF_SIGNED_INT)
check_type_size("unsigned int" SIZEOF_UNSIGNED_INT)
check_type_size("signed long" SIZEOF_SIGNED_LONG)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size("signed long long" SIZEOF_SIGNED_LONG_LONG)
check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)
check_type_size("unsigned char *" SIZEOF_UNSIGNED_CHAR_P)

set(CMAKE_EXTRA_INCLUDE_FILES_SAVE ${CMAKE_EXTRA_INCLUDE_FILES})
set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES} "stddef.h")
check_type_size("size_t" SIZEOF_SIZE_T)
check_type_size("ptrdiff_t" SIZEOF_PTRDIFF_T)
set(CMAKE_EXTRA_INCLUDE_FILES ${CMAKE_EXTRA_INCLUDE_FILES_SAVE})

macro(report_values)
  foreach(val ${ARGV})
    message(STATUS "${val} set to ${${val}}")
  endforeach()
endmacro()

set(TIFF_INT8_T "signed char")
set(TIFF_UINT8_T "unsigned char")

set(TIFF_INT16_T "signed short")
set(TIFF_UINT16_T "unsigned short")

if(SIZEOF_SIGNED_INT EQUAL 4)
  set(TIFF_INT32_T "signed int")
  set(TIFF_INT32_FORMAT "%d")
elseif(SIZEOF_SIGNED_LONG EQUAL 4)
  set(TIFF_INT32_T "signed long")
  set(TIFF_INT32_FORMAT "%ld")
endif()

if(SIZEOF_UNSIGNED_INT EQUAL 4)
  set(TIFF_UINT32_T "unsigned int")
  set(TIFF_UINT32_FORMAT "%u")
elseif(SIZEOF_UNSIGNED_LONG EQUAL 4)
  set(TIFF_UINT32_T "unsigned long")
  set(TIFF_UINT32_FORMAT "%lu")
endif()

if(SIZEOF_SIGNED_LONG EQUAL 8)
  set(TIFF_INT64_T "signed long")
  set(TIFF_INT64_FORMAT "%ld")
elseif(SIZEOF_SIGNED_LONG_LONG EQUAL 8)
  set(TIFF_INT64_T "signed long long")
  if (MINGW)
    set(TIFF_INT64_FORMAT "%I64d")
  else()
    set(TIFF_INT64_FORMAT "%lld")
  endif()
endif()

if(SIZEOF_UNSIGNED_LONG EQUAL 8)
  set(TIFF_UINT64_T "unsigned long")
  set(TIFF_UINT64_FORMAT "%lu")
elseif(SIZEOF_UNSIGNED_LONG_LONG EQUAL 8)
  set(TIFF_UINT64_T "unsigned long long")
  if (MINGW)
    set(TIFF_UINT64_FORMAT "%I64u")
  else()
    set(TIFF_UINT64_FORMAT "%llu")
  endif()
endif()

if(SIZEOF_UNSIGNED_INT EQUAL SIZEOF_SIZE_T)
  set(TIFF_SIZE_T "unsigned int")
  set(TIFF_SIZE_FORMAT "%u")
elseif(SIZEOF_UNSIGNED_LONG EQUAL SIZEOF_SIZE_T)
  set(TIFF_SIZE_T "unsigned long")
  set(TIFF_SIZE_FORMAT "%lu")
elseif(SIZEOF_UNSIGNED_LONG_LONG EQUAL SIZEOF_SIZE_T)
  set(TIFF_SIZE_T "unsigned long")
  if (MINGW)
    set(TIFF_SIZE_FORMAT "%I64u")
  else()
    set(TIFF_SIZE_FORMAT "%llu")
  endif()
endif()

if(SIZEOF_SIGNED_INT EQUAL SIZEOF_UNSIGNED_CHAR_P)
  set(TIFF_SSIZE_T "signed int")
  set(TIFF_SSIZE_FORMAT "%d")
elseif(SIZEOF_SIGNED_LONG EQUAL SIZEOF_UNSIGNED_CHAR_P)
  set(TIFF_SSIZE_T "signed long")
  set(TIFF_SSIZE_FORMAT "%ld")
elseif(SIZEOF_SIGNED_LONG_LONG EQUAL SIZEOF_UNSIGNED_CHAR_P)
  set(TIFF_SSIZE_T "signed long long")
  if (MINGW)
    set(TIFF_SSIZE_FORMAT "%I64d")
  else()
    set(TIFF_SSIZE_FORMAT "%lld")
  endif()
endif()

if(NOT SIZEOF_PTRDIFF_T)
  set(TIFF_PTRDIFF_T "${TIFF_SSIZE_T}")
  set(TIFF_PTRDIFF_FORMAT "${SSIZE_FORMAT}")
else()
  set(TIFF_PTRDIFF_T "ptrdiff_t")
  set(TIFF_PTRDIFF_FORMAT "%ld")
endif()

#report_values(TIFF_INT8_T TIFF_INT8_FORMAT
#              TIFF_UINT8_T TIFF_UINT8_FORMAT
#              TIFF_INT16_T TIFF_INT16_FORMAT
#              TIFF_UINT16_T TIFF_UINT16_FORMAT
#              TIFF_INT32_T TIFF_INT32_FORMAT
#              TIFF_UINT32_T TIFF_UINT32_FORMAT
#              TIFF_INT64_T TIFF_INT64_FORMAT
#              TIFF_UINT64_T TIFF_UINT64_FORMAT
#              TIFF_SSIZE_T TIFF_SSIZE_FORMAT
#              TIFF_PTRDIFF_T TIFF_PTRDIFF_FORMAT)

check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(setmode "unistd.h" HAVE_SETMODE)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
check_symbol_exists(strtol "stdlib.h" HAVE_STRTOL)
check_symbol_exists(strtoll "stdlib.h" HAVE_STRTOLL)
check_symbol_exists(strtoul "stdlib.h" HAVE_STRTOUL)
check_symbol_exists(strtoull "stdlib.h" HAVE_STRTOULL)
check_symbol_exists(getopt "unistd.h" HAVE_GETOPT)
check_symbol_exists(lfind "search.h" HAVE_LFIND)

if(NOT HAVE_SNPRINTF)
  add_definitions(-DNEED_LIBPORT)
endif()

# CPU bit order
set(HOST_FILLORDER FILLORDER_MSB2LSB)
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i.*86.*" OR
   CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "amd64.*" OR
   CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64.*")
  set(HOST_FILLORDER FILLORDER_LSB2MSB)
endif()

# CPU endianness
include(TestBigEndian)
test_big_endian(HOST_BIG_ENDIAN)

# IEEE floating point
set(HAVE_IEEEFP 1)

#report_values(CMAKE_HOST_SYSTEM_PROCESSOR HOST_FILLORDER HOST_BIG_ENDIAN HAVE_IEEEFP)

# Large file support
if (UNIX OR MINGW)
  # This might not catch every possibility catered for by
  # AC_SYS_LARGEFILE.
  add_definitions(-D_FILE_OFFSET_BITS=64)
  set(FILE_OFFSET_BITS 64)
endif()

# Documentation install directory (default to cmake project docdir)
set(LIBTIFF_DOCDIR "${CMAKE_INSTALL_FULL_DOCDIR}")

# Options to enable and disable internal codecs
set(CCITT_SUPPORT TRUE)
set(PACKBITS_SUPPORT TRUE)
set(LZW_SUPPORT TRUE)
set(THUNDER_SUPPORT TRUE)
set(NEXT_SUPPORT TRUE)
set(LOGLUV_SUPPORT TRUE)
set(MDI_SUPPORT TRUE)

# ZLIB
set(ZLIB_SUPPORT FALSE)
if(TARGET zlibstatic)
  set(ZLIB_SUPPORT TRUE)
endif()
set(ZIP_SUPPORT ${ZLIB_SUPPORT})

# Option for Pixar log-format algorithm
# Pixar log format
set(PIXARLOG_SUPPORT ${ZLIB_SUPPORT})

# JPEG
set(JPEG_SUPPORT FALSE)
if(TARGET jpeg)
  set(JPEG_SUPPORT TRUE)
endif()
set(OJPEG_SUPPORT ${JPEG_SUPPORT})

# TODO: 8/12-bit jpeg mode -- We currently cannot support the 12-bit mode too, don't let the user turn this on
#option(jpeg12 "enable libjpeg 8/12-bit dual mode (requires separate 12-bit libjpeg build)" ON)
#set(JPEG12_INCLUDE_DIR JPEG12_INCLUDE_DIR-NOTFOUND CACHE PATH "Include directory for 12-bit libjpeg")
#set(JPEG12_LIBRARY JPEG12_LIBRARY-NOTFOUND CACHE FILEPATH "12-bit libjpeg library")
#set(JPEG12_FOUND FALSE)
#if (JPEG12_INCLUDE_DIR AND JPEG12_LIBRARY)
#  set(JPEG12_LIBRARIES ${JPEG12_LIBRARY})
#  set(JPEG12_FOUND TRUE)
#endif()
#if (JPEG12_FOUND)
#  set(JPEG_DUAL_MODE_8_12 1)
#  set(LIBJPEG_12_PATH "${JPEG12_INCLUDE_DIR}/jpeglib.h")
#endif()

# Win32 IO
set(WIN32_IO FALSE)
if(WIN32)
  set(WIN32_IO TRUE)
endif()

set(USE_WIN32_FILEIO ${WIN32_IO})

# Orthogonal features

# Strip chopping
option(strip-chopping "strip chopping (whether or not to convert single-strip uncompressed images to mutiple strips of specified size to reduce memory usage)" ON)
set(TIFF_DEFAULT_STRIP_SIZE 8192 CACHE STRING "default size of the strip in bytes (when strip chopping is enabled)")

set(STRIPCHOP_DEFAULT)
if(strip-chopping)
  set(STRIPCHOP_DEFAULT TRUE)
  if(TIFF_DEFAULT_STRIP_SIZE)
    set(STRIP_SIZE_DEFAULT "${TIFF_DEFAULT_STRIP_SIZE}")
  endif()
endif()

# Defer loading of strip/tile offsets
option(defer-strile-load "enable deferred strip/tile offset/size loading (experimental)" OFF)
set(DEFER_STRILE_LOAD ${defer-strile-load})

# CHUNKY_STRIP_READ_SUPPORT
option(chunky-strip-read "enable reading large strips in chunks for TIFFReadScanline() (experimental)" OFF)
set(CHUNKY_STRIP_READ_SUPPORT ${chunky-strip-read})

# SUBIFD support
set(SUBIFD_SUPPORT 1)

# Default handling of ASSOCALPHA support.
option(extrasample-as-alpha "the RGBA interface will treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many packages produce RGBA files but don't mark the alpha properly" ON)
if(extrasample-as-alpha)
  set(DEFAULT_EXTRASAMPLE_AS_ALPHA 1)
endif()

# Default handling of YCbCr subsampling support.
# See Bug 168 in Bugzilla, and JPEGFixupTestSubsampling() for details.
option(check-ycbcr-subsampling "enable picking up YCbCr subsampling info from the JPEG data stream to support files lacking the tag" ON)
if (check-ycbcr-subsampling)
  set(CHECK_JPEG_YCBCR_SUBSAMPLING 1)
endif()

# Includes used by libtiff (and tests)
if(JPEG_INCLUDE_DIR)
  list(APPEND TIFF_INCLUDES ${JPEG_INCLUDE_DIR})
endif()
if(JPEG12_INCLUDE_DIR)
  list(APPEND TIFF_INCLUDES ${JPEG12_INCLUDE_DIR})
endif()

# Libraries required by libtiff
set(TIFF_LIBRARY_DEPS)
if(M_LIBRARY)
  list(APPEND TIFF_LIBRARY_DEPS ${M_LIBRARY})
endif()
if(ZLIB_SUPPORT)
  list(APPEND TIFF_LIBRARY_DEPS zlibstatic)
endif()
if(JPEG_SUPPORT)
  list(APPEND TIFF_LIBRARY_DEPS jpeg)
endif()
if(JPEG12_LIBRARIES)
  list(APPEND TIFF_LIBRARY_DEPS ${JPEG12_LIBRARIES})
endif()

#report_values(TIFF_INCLUDES TIFF_LIBRARY_DEPS)

# Process subdirectories
add_subdirectory(libtiff)

#message(STATUS "EXTRA_DIST: ${EXTRA_DIST}")

message(STATUS "LibTIFF uses win32 IO: ${USE_WIN32_FILEIO}")
#message(STATUS "LibTIFF has JPEG 8/12 bit dual mode: ${JPEG12_FOUND}")
