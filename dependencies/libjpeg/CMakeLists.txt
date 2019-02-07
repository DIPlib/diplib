# CMake script for libjpeg 9c
# (c)2019 Cris Luengo

cmake_minimum_required(VERSION 3.5)

project(libjpeg LANGUAGES C VERSION 9.3.0)

set(SOURCES
    jaricom.c
    jcapimin.c
    jcapistd.c
    jcarith.c
    jccoefct.c
    jccolor.c
    jcdctmgr.c
    jchuff.c
    jcinit.c
    jcmainct.c
    jcmarker.c
    jcmaster.c
    jcomapi.c
    jcparam.c
    jcprepct.c
    jcsample.c
    jctrans.c
    jdapimin.c
    jdapistd.c
    jdarith.c
    jdatadst.c
    jdatasrc.c
    jdcoefct.c
    jdcolor.c
    jddctmgr.c
    jdhuff.c
    jdinput.c
    jdmainct.c
    jdmarker.c
    jdmaster.c
    jdmerge.c
    jdpostct.c
    jdsample.c
    jdtrans.c
    jerror.c
    jfdctflt.c
    jfdctfst.c
    jfdctint.c
    jidctflt.c
    jidctfst.c
    jidctint.c
    jmemmgr.c
    jmemnobs.c  # We choose this one, assuming sufficient memory. Who doesn't?
    jquant1.c
    jquant2.c
    jutils.c
    )

set(HEADERS
    jconfig.h  # The standard configuration is OK for ANSI-compliant compilers, which is what we will only deal with.
    jpeglib.h
    jmorecfg.h
    jerror.h
    jinclude.h
    jpegint.h
    jdct.h
    jmemsys.h
    jversion.h
    )

# Build a static library
add_library(jpeg STATIC ${SOURCES} ${HEADERS})
target_include_directories(jpeg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(jpeg PROPERTIES C_VISIBILITY_PRESET hidden POSITION_INDEPENDENT_CODE TRUE)
if(CMAKE_C_COMPILER_ID MATCHES "Clang") # also matchs "AppleClang"
   target_compile_options(jpeg PRIVATE -Wno-shift-negative-value)
endif()
