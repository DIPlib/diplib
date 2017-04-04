# DIPlib 3.0

*DIPlib* is a library for quantitative image analysis. It has been in development
at Delft University of Technology in The Netherlands since 1995. The 3.0 release
represents a complete rewrite in C++ of the library infrastructure, with most of
the image processing and analysis algorithms ported unmodified from the previous
version.

*DIPlib* is distributed under the Apache 2.0 license. See the LICENCE.txt file.

See the [documentation](https://diplib.github.io/diplib-docs/) for more information.

## Building the library

To build the library under Unix or MacOS, you will need a C++ compiler and CMake.
Use the following commands to build:

    mkdir target
    cd target
    cmake /path/to/dip/root/directory
    make -j install

To change which compilers CMake uses:

    CC=gcc-5 CXX=g++-5 cmake /path/to/dip/root/directory

Available `make` targets:

    <default>   # builds DIPlib and DIPimage MEX-files
    DIP         # builds the DIPlib library only
    PyDIP       # builds the PyDIP Python module only (plus the DIPlib library)
    check       # builds the unit_tests program and runs it
    apidoc      # Doxygen HTML documentation for the library API
    install     # builds and installs DIPlib, PyDIP and DIPimage
    docs        # makes some PDF project documentation files
    tests       # deprecated -- old test functions

Important `cmake` command-line arguments:

    -DCMAKE_INSTALL_PREFIX=$HOME/dip        # choose an instal location
    -DCMAKE_BUILD_TYPE=Debug                # by default it is release
    -DDIP_EXCEPTIONS_RECORD_STACK_TRACE=Off # to disable stack trace generation on exception
    -DDIP_ENABLE_ASSERT=Off                 # to disable asserts
    -DDIP_ENABLE_DOCTEST=Off                # to disable doctest within DIPlib
    -DDIP_ENABLE_UNICODE=Off                # to disable UFT-8 strings within DIPlib
    -DDIP_BUILD_PYTHON_MODULE=Off           # to not build the PyDIP Python module
    -DPYBIND11_PYTHON_VERSION=3             # to compile PyDIP agains Python 3

Under Windows you can follow a similar process, but I have never used CMake under
Windows, so I'll let someone else write this bit.
